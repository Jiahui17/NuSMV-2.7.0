/**CFile*****************************************************************

   FileName    [SymbTable.c]

   PackageName [compile.symb_table]

   Synopsis    [Implementation of the system-wide SymbolTable]

   Description []

   SeeAlso     [SymbTable.h]

   Author      [Roberto Cavada, Alessandro Mariotti]

   Copyright   [
   This file is part of the ``compile.symb_table'' package of NuSMV
   version 2.  Copyright (C) 2004 by FBK-irst.

   NuSMV version 2 is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   NuSMV version 2 is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

   For more information on NuSMV see <http://nusmv.fbk.eu>
   or email to <nusmv-users@fbk.eu>.
   Please report bugs to <nusmv-users@fbk.eu>.

   To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

******************************************************************************/

#include "nusmv/core/compile/symb_table/SymbTable_.h"
#include "nusmv/core/compile/symb_table/SymbCache_.h"

#include <algorithm>

extern "C" {

#include "nusmv/core/utils/Logger.h"

#include "nusmv/core/compile/symb_table/SymbLayer.h"
#include "nusmv/core/compile/symb_table/SymbLayer_private.h"
#include "nusmv/core/compile/symb_table/ResolveSymbol.h"

#include "nusmv/core/utils/Pair.h"
#include "nusmv/core/compile/compileInt.h"
#include "nusmv/core/compile/type_checking/TypeChecker.h"

#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/utils/error.h"
#include "nusmv/core/utils/NodeList.h"
#include "nusmv/core/utils/ustring.h"
#include "nusmv/core/utils/assoc.h"
#include "nusmv/core/utils/defs.h"
#include "nusmv/core/compile/symb_table/NFunction.h"

#include "nusmv/core/utils/TimerBench.h"

#include "nusmv/core/utils/ErrorMgr.h"
#include "nusmv/core/utils/StreamMgr.h"
#include "nusmv/core/utils/UStringMgr.h"
#include "nusmv/core/node/printers/MasterPrinter.h"
#include "nusmv/core/node/NodeMgr.h"
}


namespace nusmv {


  static assoc_retval
  class_layers_hash_free(char *key, char *data, char *arg);

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

  enum SymbTableHandledType {
    ST_HANDLED_STRUCTS_HASH
  };


  SymbTable::SymbTable(const NuSMVEnv_ptr env)
    : EnvObject{env}, cache{this}
  {

    options = OPTS_HANDLER(NuSMVEnv_get_value(get_env(), ENV_OPTS_HANDLER));
    streams = STREAM_MGR(NuSMVEnv_get_value(get_env(), ENV_STREAM_MANAGER));
    nodes = NODE_MGR(NuSMVEnv_get_value(get_env(), ENV_NODE_MGR));
    printer = MASTER_PRINTER(NuSMVEnv_get_value(get_env(), ENV_WFF_PRINTER));
    strings = USTRING_MGR(NuSMVEnv_get_value(get_env(), ENV_STRING_MGR));

    categories = new_assoc();
    temp_layer_name_suffix = 0;
    class_layers = new_assoc();
    nusmv_assert(class_layers != (hash_ptr) NULL);
    class_names = NodeList_create();
    default_class_name = (const char*) NULL;

    name2layer = new_assoc();
    nusmv_assert(name2layer != (hash_ptr) NULL);

    det_counter = 0;
    tc = TypeChecker_create_with_default_checkers(NULL/*this*/);

    expr_simplify_hash = new_assoc();
    resolver = ResolveSymbol_create();
    handled_structures = new_assoc();
  }


  SymbTable::~SymbTable()
  {
    ListIter_ptr iter;
    assoc_iter aiter;
    Pair_ptr v;
    string_ptr k;

    free_assoc(expr_simplify_hash);
    expr_simplify_hash = NULL;

    TypeChecker_destroy(tc);
    tc = TYPE_CHECKER(NULL);

    /* destroys the layer classes: */
    NodeList_destroy(class_names);
    clear_assoc_and_free_entries(class_layers, class_layers_hash_free);
    free_assoc(class_layers);

    if (default_class_name != (const char*) NULL) {
      FREE(default_class_name);
    }

    /* get rid of all contained layers */
    std::for_each(layers.begin(), layers.end(), &SymbLayer_destroy_raw);

    free_assoc(categories);

    free_assoc(name2layer);

    ResolveSymbol_destroy(resolver);

    ASSOC_FOREACH(handled_structures, aiter, &k, &v) {
      SymbTableHandledType t = (SymbTableHandledType) (size_t) Pair_get_second(v);
      void* val = Pair_get_first(v);

      switch (t) {
      case ST_HANDLED_STRUCTS_HASH:
	free_assoc((hash_ptr)val);
	break;

      default:
	error_unreachable_code_msg("Unhandled structure type");
      }

#ifdef MEMOIZED_HASH_DEBUG
      FREE(v);
#else
      Pair_destroy(v);
#endif
    }

    free_assoc(handled_structures);

  }


  TypeChecker_ptr SymbTable::get_type_checker() const
  {
    return tc;
  }


  void SymbTable::gen_iter(SymbTableIter* iter, unsigned int mask) const
  {
    cache.gen_iter(iter, mask);
  }

  void SymbTable::iter_next(SymbTableIter* iter) const
  {
    cache.next_iter(iter);
  }

  boolean SymbTable::iter_is_end(const SymbTableIter* iter) const
  {
    return cache.is_iter_end(iter);
  }

  node_ptr SymbTable::iter_get_symbol(const SymbTableIter* iter) const
  {
    return cache.iter_get_symbol(iter);
  }


  static assoc_retval
  class_layers_hash_free(char *key, char *data, char *arg)
  {
    array_t* _class = (array_t*) data;

    if (_class != NULL) {
      const char* name; int i;
      arrayForEachItem(const char*, _class, i, name) { FREE(name); }
      array_free(_class);
    }
    return ASSOC_DELETE;
  }


}
