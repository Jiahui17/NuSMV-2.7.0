/**CFile*****************************************************************

  FileName    [SymbCache.c]

  PackageName [compile.symb_table]

  Synopsis    [The SymbCache class implementation]

  Description []

  SeeAlso     [SymbCache.h]

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

#include "nusmv/core/compile/symb_table/SymbCache_.h"
#include "nusmv/core/compile/symb_table/SymbTable_.h"

extern "C" {
#include "nusmv/core/utils/Logger.h"
#include "nusmv/core/compile/symb_table/symb_table_int.h"

#include "nusmv/core/compile/compile.h"
#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/utils/assoc.h"
#include "nusmv/core/utils/error.h"
#include "nusmv/core/compile/symb_table/NFunction.h"
#include "nusmv/core/utils/ErrorMgr.h"
#include "nusmv/core/node/printers/MasterPrinter.h"
#include "nusmv/core/node/NodeMgr.h"

#include "nusmv/core/compile/compileInt.h"

#include "nusmv/core/compile/flattening/MasterCompileFlattener.h"

}


namespace nusmv {


  /*---------------------------------------------------------------------------*/
  /* macro declarations                                                        */
  /*---------------------------------------------------------------------------*/

#define SYMBOL_INFO_CHUNK_SIZE 1024

#define INITIAL_SYMBOLS_ARRAY_SIZE 128

#define SI_SYMBOL_REMOVED (SymbolInfo*)1

#define SI_CATEGORY(si)                         \
  si->category

#define SI_GET_CONSTANT_COUNT(si)               \
  NODE_TO_INT(si->field1)

#define SI_SET_CONSTANT_COUNT(si, val)          \
  si->field1 = NODE_FROM_INT(val)

#define SI_VAR_TYPE_LVALUE(si)                  \
  si->field1

#define SI_VAR_TYPE(si)                         \
  (SymbType_ptr) si->field1

#define SI_DEFINE_CONTEXT(si)                   \
  si->field1

#define SI_DEFINE_BODY(si)                      \
  si->field2

#define SI_DEFINE_FLATTEN_BODY(si)              \
  (node_ptr) si->field3

#define SI_ARRAY_DEFINE_CONTEXT(si)             \
  si->field1

#define SI_ARRAY_DEFINE_BODY(si)                \
  si->field2

#define SI_ARRAY_DEFINE_FLAT_BODY(si)           \
  (node_ptr) si->field3

#define SI_PARAMETER_CONTEXT(si)                \
  si->field1

#define SI_PARAMETER_ACTUAL(si)                 \
  si->field2

#define SI_PARAMETER_FLATTEN_ACTUAL(si)         \
  si->field3

#define SI_FUNCTION_CONTEXT(si)                 \
  si->field1

#define SI_FUNCTION_FUN(si)                     \
  si->field2


#define GET_SYMBOL(s)				\
  ((SymbolInfo*)find_assoc(symbol_hash, s))

#define SI_IS_DECLARED(si)				\
  (((SymbolInfo*)NULL != si) && ((SymbolInfo*)1 != si))

#define SI_IS_REMOVED(si)                       \
  (SI_SYMBOL_REMOVED == si)


  /*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

    struct SymbolInfo {
      SymbTableType category;

      /*
	Constants:
	- field1 is the reference counter

	Variables:
	- field1 is the variable type

	Defines / Parameters / Array Defines
	- field1 is the context
	- field2 is the body
	- field3 is the flatten body (NULL if still not computed)

	Functions:
	- field1 is the context
	- field2 is the NFunction pointer
      */

      void* field1;
      void* field2;
      void* field3;

      /* Pointer to next free SymbolInfo slice */
      struct SymbolInfo* next;

      /* The position in the ordered list */
      size_t position;
    };


  struct SymbCacheRemoveSymbolStruct {
    SymbTableTriggerFun trigger;
    void* arg;
    boolean must_free_arg;
  };

  /*---------------------------------------------------------------------------*/
  /* Variable declarations                                                     */
  /*---------------------------------------------------------------------------*/


  /*---------------------------------------------------------------------------*/
  /* Static function prototypes                                                */
  /*---------------------------------------------------------------------------*/
  static assoc_retval symb_hash_free_vars(char *key, char *data, char *arg);
  static void symb_cache_free_triggers(NodeList_ptr triggers);

  /*---------------------------------------------------------------------------*/
  /* Definition of exported functions                                          */
  /*---------------------------------------------------------------------------*/

  SymbCache::SymbCache(const SymbTable* st)
    : EnvObject{st->get_env()}, symb_table{st}
  {
    options = OPTS_HANDLER(NuSMVEnv_get_value(get_env(), ENV_OPTS_HANDLER));
    nodes = NODE_MGR(NuSMVEnv_get_value(get_env(), ENV_NODE_MGR));
    printer = MASTER_PRINTER(NuSMVEnv_get_value(get_env(), ENV_WFF_PRINTER));
    logger = LOGGER(NuSMVEnv_get_value(get_env(), ENV_LOGGER));

    symbol_hash = new_assoc();

    chunks = Stack_create();

    symbol_info_pool = (SymbolInfo*) NULL;

    symbols_allocated = INITIAL_SYMBOLS_ARRAY_SIZE;
    symbols_index = 0;
    symbols_empty = 0;
    symbols = ALLOC(node_ptr, symbols_allocated);

    /* Counters */
    constants_num = 0;
    state_vars_num = 0;
    input_vars_num = 0;
    frozen_vars_num = 0;
    defines_num = 0;
    functions_num = 0;
    array_defines_num = 0;
    variable_arrays_num = 0;
    parameters_num = 0;

    add_triggers = NODE_LIST(NULL);
    rem_triggers = NODE_LIST(NULL);
    redef_triggers = NODE_LIST(NULL);
  }


  SymbCache::~SymbCache()
  {
    /* Free SymbType instances */
    clear_assoc_and_free_entries(symbol_hash, symb_hash_free_vars);
    free_assoc(symbol_hash);

    while (!Stack_is_empty(chunks)) {
      SymbolInfo* chunk = (SymbolInfo*) Stack_pop(chunks);
      FREE(chunk);
    }

    Stack_destroy(chunks);
    FREE(symbols);

    symb_cache_free_triggers(add_triggers);
    symb_cache_free_triggers(rem_triggers);
    symb_cache_free_triggers(redef_triggers);
  }


  SymbType_ptr SymbCache::get_var_type(const node_ptr name) const
  {
    nusmv_assert(is_symbol_var(name));

    SymbolInfo* si = GET_SYMBOL(name);
    return SI_VAR_TYPE(si);
  }


  node_ptr SymbCache::get_define_body(const node_ptr name) const
  {
    nusmv_assert(is_symbol_define(name));

    SymbolInfo* si = GET_SYMBOL(name);
    return (node_ptr) SI_DEFINE_BODY(si);
  }


  NFunction_ptr SymbCache::get_function(const node_ptr name) const
  {
    nusmv_assert(is_symbol_function(name));

    SymbolInfo* si = GET_SYMBOL(name);
    return (NFunction_ptr) SI_FUNCTION_FUN(si);
  }


  node_ptr SymbCache::get_actual_parameter(const node_ptr name) const
  {
    nusmv_assert(is_symbol_parameter(name));

    SymbolInfo* si = GET_SYMBOL(name);
    return (node_ptr) SI_PARAMETER_ACTUAL(si);
  }


  node_ptr SymbCache::get_array_define_body(const node_ptr name) const
  {
    nusmv_assert(is_symbol_array_define(name));

    SymbolInfo* si = GET_SYMBOL(name);
    return (node_ptr) SI_ARRAY_DEFINE_BODY(si);
  }


  SymbType_ptr SymbCache::get_variable_array_type(const node_ptr name) const
  {
    nusmv_assert(is_symbol_variable_array(name));

    SymbolInfo* si = GET_SYMBOL(name);
    return SI_VAR_TYPE(si);
  }



  node_ptr SymbCache::get_flatten_actual_parameter(const node_ptr name) const
  {
    nusmv_assert(is_symbol_parameter(name));

    SymbolInfo* si = GET_SYMBOL(name);
    nusmv_assert(SI_IS_DECLARED(si));

    if (Nil == SI_PARAMETER_FLATTEN_ACTUAL(si)) {
      node_ptr flat;

      flat = NodeMgr_find_node(nodes, CONTEXT,
			       (node_ptr) SI_PARAMETER_CONTEXT(si),
			       (node_ptr) SI_PARAMETER_ACTUAL(si));

      SI_PARAMETER_FLATTEN_ACTUAL(si) = flat;
    }

    return (node_ptr) SI_PARAMETER_FLATTEN_ACTUAL(si);
  }


  node_ptr SymbCache::get_define_context(const node_ptr name) const
  {
    nusmv_assert(is_symbol_define(name));

    SymbolInfo* si = GET_SYMBOL(name);
    return (node_ptr) SI_DEFINE_CONTEXT(si);
  }


  node_ptr SymbCache::get_function_context(const node_ptr name) const
  {
    nusmv_assert(is_symbol_function(name));

    SymbolInfo* si = GET_SYMBOL(name);
    return (node_ptr) SI_FUNCTION_CONTEXT(si);
  }


  node_ptr SymbCache::get_actual_parameter_context(const node_ptr name) const
  {
    nusmv_assert(is_symbol_parameter(name));

    SymbolInfo* si = GET_SYMBOL(name);
    return (node_ptr) SI_PARAMETER_CONTEXT(si);
  }


  node_ptr SymbCache::get_array_define_context(const node_ptr name) const
  {
    nusmv_assert(is_symbol_array_define(name));

    SymbolInfo* si = GET_SYMBOL(name);
    return (node_ptr) SI_ARRAY_DEFINE_CONTEXT(si);
  }


  bool SymbCache::is_symbol_state_var(const node_ptr name) const
  {
    SymbolInfo* si = GET_SYMBOL(name);
    return (SI_IS_DECLARED(si) && STT_STATE_VAR == SI_CATEGORY(si));
  }


  bool SymbCache::is_symbol_frozen_var(const node_ptr name) const
  {
    SymbolInfo* si = GET_SYMBOL(name);
    return (SI_IS_DECLARED(si) && STT_FROZEN_VAR == SI_CATEGORY(si));
  }


  bool SymbCache::is_symbol_state_frozen_var(const node_ptr name) const
  {
    SymbolInfo* si = GET_SYMBOL(name);
    return (SI_IS_DECLARED(si) &&
	    ((STT_FROZEN_VAR | STT_STATE_VAR) & SI_CATEGORY(si)));
  }


  bool SymbCache::is_symbol_input_var(const node_ptr name) const
  {
    SymbolInfo* si = GET_SYMBOL(name);
    return (SI_IS_DECLARED(si) && STT_INPUT_VAR == SI_CATEGORY(si));
  }


  bool SymbCache::is_symbol_var(const node_ptr name) const
  {
    SymbolInfo* si = GET_SYMBOL(name);
    return (SI_IS_DECLARED(si) && (STT_VAR & SI_CATEGORY(si)));
  }


  bool SymbCache::is_symbol_declared(const node_ptr name) const
  {
    SymbolInfo* si = GET_SYMBOL(name);
    return SI_IS_DECLARED(si);
  }


  bool SymbCache::is_symbol_define(const node_ptr name) const
  {
    SymbolInfo* si = GET_SYMBOL(name);
    return (SI_IS_DECLARED(si) && STT_DEFINE == SI_CATEGORY(si));
  }


  bool SymbCache::is_symbol_function(const node_ptr name) const
  {
    SymbolInfo* si = GET_SYMBOL(name);
    return (SI_IS_DECLARED(si) && STT_FUNCTION == SI_CATEGORY(si));
  }


  bool SymbCache::is_symbol_parameter(const node_ptr name) const
  {
    SymbolInfo* si = GET_SYMBOL(name);
    return (SI_IS_DECLARED(si) && STT_PARAMETER == SI_CATEGORY(si));
  }


  bool SymbCache::is_symbol_array_define(const node_ptr name) const
  {
    SymbolInfo* si = GET_SYMBOL(name);
    return (SI_IS_DECLARED(si) && STT_ARRAY_DEFINE == SI_CATEGORY(si));
  }


  bool SymbCache::is_symbol_variable_array(const node_ptr name) const
  {
    SymbolInfo* si = GET_SYMBOL(name);
    return (SI_IS_DECLARED(si) && STT_VARIABLE_ARRAY == SI_CATEGORY(si));
  }


  bool SymbCache::is_symbol_constant(const node_ptr name) const
  {
    SymbolInfo* si = GET_SYMBOL(name);
    return (SI_IS_DECLARED(si) && STT_CONSTANT == SI_CATEGORY(si));
  }


  bool SymbCache::list_contains_input_var(const NodeList_ptr var_list) const
  {
    ListIter_ptr iter;
    NODE_LIST_FOREACH(var_list, iter) {
      node_ptr sym = NodeList_get_elem_at(var_list, iter);
      if (is_symbol_input_var(sym))
	return true;
    }
    return false;
  }


  bool SymbCache::list_contains_state_frozen_var(const NodeList_ptr var_list) const
  {
    ListIter_ptr iter;
    NODE_LIST_FOREACH(var_list, iter) {
      node_ptr sym = NodeList_get_elem_at(var_list, iter);
      if (is_symbol_state_frozen_var(sym))
	return true;
    }

    return false;
  }


  bool SymbCache::list_contains_undef_var(const NodeList_ptr var_list) const
  {
    ListIter_ptr iter;
    NODE_LIST_FOREACH(var_list, iter) {
      node_ptr sym = NodeList_get_elem_at(var_list, iter);
      SymbolInfo* si = GET_SYMBOL(sym);
      if (!SI_IS_DECLARED(si))
	return true;
    }

    return false;
  }


  void SymbCache::remove_parameter(node_ptr formal)
  {
    nusmv_assert(is_symbol_parameter(formal));
    remove_symbol(formal, true);
  }


  void SymbCache::new_input_var(node_ptr var, SymbType_ptr type)
  {
    SymbolInfo* si = GET_SYMBOL(var);

    /* The symbol must not exist */
    nusmv_assert(!SI_IS_DECLARED(si));

    si = alloc_symb_info();

    SI_CATEGORY(si) = STT_INPUT_VAR;
    SI_VAR_TYPE_LVALUE(si) = type;

    new_symbol(var, si);
  }


  void SymbCache::new_state_var(node_ptr var, SymbType_ptr type)
  {
    SymbolInfo* si = GET_SYMBOL(var);

    /* The symbol must not exist */
    nusmv_assert(!SI_IS_DECLARED(si));

    si = alloc_symb_info();

    SI_CATEGORY(si) = STT_STATE_VAR;
    SI_VAR_TYPE_LVALUE(si) = type;

    new_symbol(var, si);
  }


  void SymbCache::new_frozen_var(node_ptr var, SymbType_ptr type)
  {
    SymbolInfo* si = GET_SYMBOL(var);

    /* The symbol must not exist */
    nusmv_assert(!SI_IS_DECLARED(si));

    si = alloc_symb_info();

    SI_CATEGORY(si) = STT_FROZEN_VAR;
    SI_VAR_TYPE_LVALUE(si) = type;

    new_symbol(var, si);
  }


  void SymbCache::redeclare_state_as_frozen_var(node_ptr name)
  {
    SymbolInfo* si = GET_SYMBOL(name);

    nusmv_assert(is_symbol_state_var(name));

    SI_CATEGORY(si) = STT_FROZEN_VAR;

    /* Since new_symbol or remove_symbol are not called, we need to
       update the counters manually */
    frozen_vars_num += 1;
    state_vars_num -= 1;
  }


  void SymbCache::remove_var(node_ptr var)
  {
    nusmv_assert(is_symbol_var(var));
    remove_symbol(var, true);
  }


  void SymbCache::new_define(node_ptr name, node_ptr ctx, node_ptr definition)
  {
    SymbolInfo* si = GET_SYMBOL(name);

    /* The symbol must not exist */
    nusmv_assert(!SI_IS_DECLARED(si));

    si = alloc_symb_info();

    SI_CATEGORY(si) = STT_DEFINE;
    SI_DEFINE_CONTEXT(si) = ctx;
    SI_DEFINE_BODY(si) = definition;

    new_symbol(name, si);
  }


  void SymbCache::new_function(node_ptr name, node_ptr ctx, NFunction_ptr fun)
  {
    SymbolInfo* si = GET_SYMBOL(name);

    /* The symbol must not exist */
    nusmv_assert(!SI_IS_DECLARED(si));

    si = alloc_symb_info();

    SI_CATEGORY(si) = STT_FUNCTION;
    SI_FUNCTION_CONTEXT(si) = ctx;
    SI_FUNCTION_FUN(si) = fun;

    new_symbol(name, si);
  }


  void SymbCache::new_parameter(node_ptr formal, node_ptr ctx, node_ptr actual)
  {
    SymbolInfo* si = GET_SYMBOL(formal);
    /* The symbol must not exist */
    nusmv_assert(!SI_IS_DECLARED(si));

    si = alloc_symb_info();

    SI_CATEGORY(si) = STT_PARAMETER;
    SI_PARAMETER_CONTEXT(si) = ctx;
    SI_PARAMETER_ACTUAL(si) = actual;

    new_symbol(formal, si);
  }


  void SymbCache::new_array_define(node_ptr name, node_ptr ctx, node_ptr definition)
  {
    SymbolInfo* si = GET_SYMBOL(name);
    /* The symbol must not exist */
    nusmv_assert(!SI_IS_DECLARED(si));

    si = alloc_symb_info();

    SI_CATEGORY(si) = STT_ARRAY_DEFINE;
    SI_ARRAY_DEFINE_CONTEXT(si) = ctx;
    SI_ARRAY_DEFINE_BODY(si) = definition;

    new_symbol(name, si);
  }


  void SymbCache::new_variable_array(node_ptr name, SymbType_ptr type)
  {
    SymbolInfo* si = GET_SYMBOL(name);
    /* The symbol must not exist */
    nusmv_assert(!SI_IS_DECLARED(si));

    si = alloc_symb_info();

    SI_CATEGORY(si) = STT_VARIABLE_ARRAY;
    SI_VAR_TYPE_LVALUE(si) = type;

    new_symbol(name, si);
  }


  void SymbCache::remove_define(node_ptr define)
  {
    nusmv_assert(is_symbol_define(define));
    remove_symbol(define, true);
  }


  void SymbCache::remove_function(node_ptr fun)
  {
    nusmv_assert(is_symbol_function(fun));
    remove_symbol(fun, true);
  }


  void SymbCache::new_constant(node_ptr name)
  {
    SymbolInfo* si = GET_SYMBOL(name);

    /* Constant never defines */
    if (!SI_IS_DECLARED(si)) {
      si = alloc_symb_info();

      SI_CATEGORY(si) = STT_CONSTANT;
      SI_SET_CONSTANT_COUNT(si, 1);

      new_symbol(name, si);
    }
    else {
      SI_SET_CONSTANT_COUNT(si, SI_GET_CONSTANT_COUNT(si) + 1);
    }
  }


  void SymbCache::remove_constant(node_ptr constant)
  {
    nusmv_assert(is_symbol_constant(constant));
    remove_symbol(constant, true);
  }


  void SymbCache::remove_symbols(const node_ptr* symbols,
				 size_t size)
  {
    size_t i = 0;

    for (i = 0; i < size; ++i) {
      /* Shrink only at last if needed */
      if (Nil != symbols[i]) {
	remove_symbol(symbols[i], false);
      }
    }

    check_and_shrink_symbols();
  }


  SymbTableType SymbCache::get_symbol_type(node_ptr symbol) const
  {
    SymbolInfo* si = GET_SYMBOL(symbol);
    nusmv_assert(SI_IS_DECLARED(si));
    return SI_CATEGORY(si);
  }


  void SymbCache::gen_iter(SymbTableIter* iter, unsigned int mask) const
  {
    SymbolInfo* si;
    node_ptr sym;
    size_t index = 0;

    iter->mask = mask;
    iter->filter = (SymbTableIterFilterFun)NULL;
    iter->st = SYMB_TABLE(NULL);
    iter->arg = NULL;

    /* The list is not empty */
    if (index != symbols_index) {
      sym = symbols[index];

      si = (Nil != sym) ? GET_SYMBOL(sym) : (SymbolInfo*) NULL;

      /* Current symbol does not match */
      while ((Nil == sym) || (SI_CATEGORY(si) & mask) == 0) {
	++index;

	/* The end.. */
	if (index == symbols_index)
	  break;

	sym = symbols[index];

	/* Empty cell, continue with next */
	if (Nil == sym)
	  continue;

	si = GET_SYMBOL(sym);
	nusmv_assert(SI_IS_DECLARED(si));
      }
    }

    iter->index = index;
  }


  void SymbCache::next_iter(SymbTableIter* iter) const
  {
    node_ptr sym;
    SymbolInfo* si;
    boolean valid = true;

    /* Iterator not at it's end */
    nusmv_assert(iter->index != symbols_index);

    do {
      valid = true;
      ++iter->index;

      /* The end.. */
      if (iter->index == symbols_index) {
	break;
      }

      sym = symbols[iter->index];

      /* Empty cell, continue with next */
      if (Nil == sym) {
	valid = false;
	continue;
      }

      /* Filter is not valid for this symbol */
      if (((SymbTableIterFilterFun) NULL != (iter->filter)) &&
	  !((iter->filter)((iter->st), sym, (iter->arg)))) {
	valid = false;
	continue;
      }

      si = GET_SYMBOL(sym);
      nusmv_assert(SI_IS_DECLARED(si));

    } while (!valid || ((SI_CATEGORY(si) & iter->mask) == 0)); /* Not a match */
  }


  bool SymbCache::is_iter_end(const SymbTableIter* iter) const
  {
    return symbols_index == iter->index;
  }


  node_ptr SymbCache::iter_get_symbol(const SymbTableIter* iter) const
  {
    nusmv_assert(!is_iter_end(iter));
    return symbols[iter->index];
  }


  void SymbCache::iter_set_filter(SymbTableIter* iter,
				  SymbTableIterFilterFun filter,
				  void* arg) const
  {
    size_t index = iter->index;
    node_ptr sym;
    SymbolInfo* si;

    iter->filter = filter;
    iter->arg = arg;
    nusmv_assert(SYMB_TABLE(NULL) != iter->st);

    /* The list is not empty */
    if (index != symbols_index) {
      sym = symbols[index];
      si = GET_SYMBOL(sym);
      nusmv_assert(SI_IS_DECLARED(si));

      /* Current symbol does not match */
      while ((SI_CATEGORY(si) & iter->mask) == 0 ||
	     !(filter(iter->st, sym, arg))) {
	++index;

	/* The end.. */
	if (index == symbols_index) { break; }

	sym = symbols[index];

	/* Empty cell, continue with next */
	if (Nil == sym)
	  continue;

	si = GET_SYMBOL(sym);
	nusmv_assert(SI_IS_DECLARED(si));
      }
    }

    iter->index = index;
  }


  int SymbCache::get_constants_num() const
  {
    return constants_num;
  }


  int SymbCache::get_state_vars_num() const
  {
    return state_vars_num;
  }


  int SymbCache::get_frozen_vars_num() const
  {
    return frozen_vars_num;
  }


  int SymbCache::get_input_vars_num() const
  {
    return input_vars_num;
  }


  int SymbCache::get_defines_num() const
  {
    return defines_num;
  }


  int SymbCache::get_functions_num() const
  {
    return functions_num;
  }


  int SymbCache::get_parameters_num() const
  {
    return parameters_num;
  }


  int SymbCache::get_array_defines_num() const
  {
    return array_defines_num;
  }


  int SymbCache::get_variable_arrays_num() const
  {
    return variable_arrays_num;
  }



  void SymbCache::add_trigger(SymbTableTriggerFun trigger,
			      SymbTableTriggerAction action,
			      void* arg, bool must_free_arg)
  {
    SymbCacheRemoveSymbolStruct* srss = ALLOC(SymbCacheRemoveSymbolStruct, 1);

    srss->trigger = trigger;
    srss->arg = arg;
    srss->must_free_arg = must_free_arg;

    switch (action) {
    case ST_TRIGGER_SYMBOL_ADD:
      if (NODE_LIST(NULL) == add_triggers) {
	add_triggers = NodeList_create();
      }

      if (!NodeList_belongs_to(add_triggers, NODE_PTR(srss))) {
	NodeList_append(add_triggers, NODE_PTR(srss));
      }
      break;

    case ST_TRIGGER_SYMBOL_REMOVE:
      if (NODE_LIST(NULL) == rem_triggers) {
	rem_triggers = NodeList_create();
      }

      if (!NodeList_belongs_to(rem_triggers, NODE_PTR(srss))) {
	NodeList_append(rem_triggers, NODE_PTR(srss));
      }
      break;

    case ST_TRIGGER_SYMBOL_REDECLARE:
      if (NODE_LIST(NULL) == redef_triggers) {
	redef_triggers = NodeList_create();
      }

      if (!NodeList_belongs_to(redef_triggers, NODE_PTR(srss))) {
	NodeList_append(redef_triggers, NODE_PTR(srss));
      }
      break;

    default:
      error_unreachable_code_msg("Invalid trigger action");
    }
  }



  void SymbCache::remove_trigger(SymbTableTriggerFun trigger,
				 SymbTableTriggerAction action)
  {
    /* NodeList_ptr triggers = remove_symbol_triggers; */
    ListIter_ptr iter;
    NodeList_ptr triggers = NODE_LIST(NULL);

    switch (action) {
    case ST_TRIGGER_SYMBOL_ADD:
      triggers = add_triggers;
      break;

    case ST_TRIGGER_SYMBOL_REMOVE:
      triggers = rem_triggers;
      break;

    case ST_TRIGGER_SYMBOL_REDECLARE:
      triggers = redef_triggers;
      break;

    default:
      error_unreachable_code_msg("Invalid trigger action");
    }

    /* We found the list, but it may be empty */
    if (NODE_LIST(NULL) != triggers) {
      NODE_LIST_FOREACH(triggers, iter) {
	SymbCacheRemoveSymbolStruct* srss =
	  (SymbCacheRemoveSymbolStruct*)NodeList_get_elem_at(triggers, iter);

	if (srss->trigger == trigger) {
	  NodeList_remove_elem_at(triggers, iter);

	  if (srss->must_free_arg) {
	    FREE(srss->arg);
	  }
	  FREE(srss);
	  break;
	}
      }

      /* The list is now useless */
      if (NodeList_get_length(triggers) == 0) {
	switch (action) {
	case ST_TRIGGER_SYMBOL_ADD: add_triggers = NODE_LIST(NULL); break;
	case ST_TRIGGER_SYMBOL_REMOVE: rem_triggers = NODE_LIST(NULL);  break;
	case ST_TRIGGER_SYMBOL_REDECLARE: redef_triggers = NODE_LIST(NULL); break;
	default: error_unreachable_code_msg("Invalid trigger action");
	}
	NodeList_destroy(triggers);
      }
    }
  }

  void SymbCache::new_symbol(node_ptr sym, SymbolInfo* symbol_info)
  {
    size_t index = symbols_index;
    SymbolInfo* old_si = GET_SYMBOL(sym);
    boolean redefined = SI_IS_REMOVED(old_si);

    insert_assoc(symbol_hash, sym, (node_ptr)symbol_info);

    if (index == symbols_allocated) {
      symbols_allocated *= 2;
      symbols = REALLOC(node_ptr, symbols, symbols_allocated);
    }

    symbols[index] = sym;
    symbol_info->position = index;

    symbols_index++;

    switch (SI_CATEGORY(symbol_info)) {

    case STT_INPUT_VAR: ++input_vars_num; break;
    case STT_FROZEN_VAR: ++frozen_vars_num; break;
    case STT_STATE_VAR: ++state_vars_num; break;
    case STT_VARIABLE_ARRAY: ++variable_arrays_num; break;
    case STT_CONSTANT: ++constants_num; break;
    case STT_DEFINE: ++defines_num; break;
    case STT_ARRAY_DEFINE: ++array_defines_num; break;
    case STT_PARAMETER: ++parameters_num; break;
    case STT_FUNCTION: ++functions_num; break;

    default:
      error_unreachable_code();
      break;
    }

    /* Call triggers */
    if (NODE_LIST(NULL) != add_triggers) {
      ListIter_ptr iter;
      if (opt_verbose_level_gt(options, 4)) {
	Logger_nlog(logger, printer,
		    "SymbCache: Calling add symbol "
		    "triggers for symbol '%N'\n", sym);
      }

      NODE_LIST_FOREACH(add_triggers, iter) {
	SymbCacheRemoveSymbolStruct* srss =
	  (SymbCacheRemoveSymbolStruct*)NodeList_get_elem_at(add_triggers,
							     iter);
	srss->trigger(symb_table, sym, ST_TRIGGER_SYMBOL_ADD, srss->arg);
      }
    }

    if (redefined && NODE_LIST(NULL) != redef_triggers) {
      ListIter_ptr iter;
      if (opt_verbose_level_gt(options, 4)) {
	Logger_nlog(logger, printer,
		    "SymbCache: Calling redeclare symbol "
		    "triggers for symbol '%N'\n", sym);
      }

      NODE_LIST_FOREACH(redef_triggers, iter) {
	SymbCacheRemoveSymbolStruct* srss =
	  (SymbCacheRemoveSymbolStruct*)NodeList_get_elem_at(
							     redef_triggers,
							     iter);
	srss->trigger(symb_table, sym,
		      ST_TRIGGER_SYMBOL_REDECLARE, srss->arg);
      }
    }
  }


  void SymbCache::remove_symbol(const node_ptr sym,
				const boolean shrink_if_needed)
  {
    SymbolInfo* si = (SymbolInfo*)find_assoc(symbol_hash, sym);
    ListIter_ptr iter;

    nusmv_assert(SI_IS_DECLARED(si));

    /* Call triggers */
    if (NODE_LIST(NULL) != rem_triggers) {
      if (opt_verbose_level_gt(options, 4)) {
	Logger_nlog(logger, printer,
		    "SymbCache: Calling remove symbol "
		    "triggers for symbol '%N'\n", sym);
      }

      NODE_LIST_FOREACH(rem_triggers, iter) {
	SymbCacheRemoveSymbolStruct* srss =
	  (SymbCacheRemoveSymbolStruct*)NodeList_get_elem_at(rem_triggers,
							     iter);
	srss->trigger(symb_table, sym,
		      ST_TRIGGER_SYMBOL_REMOVE, srss->arg);
      }
    }

    switch (SI_CATEGORY(si)) {
    case STT_INPUT_VAR:
      --input_vars_num;
      SymbType_destroy(SI_VAR_TYPE(si));
      break;

    case STT_FROZEN_VAR:
      --frozen_vars_num;
      SymbType_destroy(SI_VAR_TYPE(si));
      break;

    case STT_STATE_VAR:
      --state_vars_num;
      SymbType_destroy(SI_VAR_TYPE(si));
      break;

    case STT_VARIABLE_ARRAY:
      --variable_arrays_num;
      SymbType_destroy(SI_VAR_TYPE(si));
      break;

      /* Constants have their own reference counter.. */
    case STT_CONSTANT:
      SI_SET_CONSTANT_COUNT(si, SI_GET_CONSTANT_COUNT(si) - 1);

      /* It is still not the moment for removing this constant */
      if (SI_GET_CONSTANT_COUNT(si) > 0) { return; }
      else { --constants_num; }
      break;

    case STT_DEFINE: --defines_num; break;
    case STT_ARRAY_DEFINE: --array_defines_num; break;
    case STT_PARAMETER: --parameters_num; break;

    case STT_FUNCTION:
      --functions_num;
      NFunction_destroy((NFunction_ptr) SI_FUNCTION_FUN(si));
      break;

    default:
      error_unreachable_code();
      break;
    }

    insert_assoc(symbol_hash, sym, NODE_PTR(SI_SYMBOL_REMOVED));

    /* Remove from the list */
    symbols[si->position] = Nil;
    symbols_empty++;

    free_symb_info(si);

    if (shrink_if_needed) {
      check_and_shrink_symbols();
    }
  }


  SymbolInfo* SymbCache::alloc_symb_info()
  {
    SymbolInfo* res = (SymbolInfo*) NULL;

    /* Pool does not contain any slices */
    if ((SymbolInfo*)NULL == symbol_info_pool) {
      SymbolInfo* chunk = ALLOC(SymbolInfo, SYMBOL_INFO_CHUNK_SIZE);
      int i;

      memset(chunk, 0, sizeof(SymbolInfo) * SYMBOL_INFO_CHUNK_SIZE);

      Stack_push(chunks, chunk);
      symbol_info_pool = (SymbolInfo*) chunk;

      for (i = 0; i < (SYMBOL_INFO_CHUNK_SIZE - 1); i++) {
	SymbolInfo* c = (chunk + i);
	SymbolInfo* cn = (chunk + i + 1);
	c->next = cn;
      }
    }

    res = symbol_info_pool;
    symbol_info_pool = res->next;

    nusmv_assert((SymbolInfo*)NULL != res);

    return res;
  }


  void SymbCache::free_symb_info(SymbolInfo* si)
  {
    SymbolInfo* tmp = symbol_info_pool;
    symbol_info_pool = si;

    memset(si, 0, sizeof(SymbolInfo));
    si->next = tmp;
  }


  /**Function********************************************************************

     Synopsis           [Shrinks the symbols array if needed]

     Description        [Shrinks the symbols array if needed]

     SideEffects        []

     SeeAlso            []

  ******************************************************************************/
  void SymbCache::check_and_shrink_symbols()
  {
    /* ~75% of the list is empty.. shrink */
    if ((symbols_allocated > INITIAL_SYMBOLS_ARRAY_SIZE) &&
	(((double) symbols_empty /
	  (double) symbols_allocated) > 0.75)) {
      node_ptr* old_symbols = symbols;
      node_ptr* new_symbols;

      symbols_allocated /= 2;
      new_symbols = ALLOC(node_ptr, symbols_allocated);
      symbols = new_symbols;

      size_t i, j;
      for (i = 0, j = 0; j < symbols_index; ++j) {
	node_ptr sym = old_symbols[j];

	/* The symbol has not been removed.. */
	if (sym != Nil) {
	  SymbolInfo* si = (SymbolInfo*)find_assoc(symbol_hash, sym);
	  nusmv_assert(SI_IS_DECLARED(si));

	  nusmv_assert(i < symbols_allocated);
	  new_symbols[i] = sym;

	  /* Update the position */
	  si->position = i;

	  ++i;
	}
      }

      symbols_index = i;

      /* After shrinking, there are no empty cells */
      symbols_empty = 0;

      FREE(old_symbols);
    }
  }


  /**Function********************************************************************

     Synopsis           [Private destructor used by class destroyer]

     Description        []

     SideEffects        []

     SeeAlso            [symb_table_deinit]

  ******************************************************************************/
  static assoc_retval symb_hash_free_vars(char *key, char *data, char *arg)
  {
    SymbolInfo* si = (SymbolInfo*)data;

    /* This is marked as removed, no memory to be freed */
    if (SI_IS_REMOVED(si)) return ASSOC_DELETE;

    nusmv_assert(SI_IS_DECLARED(si));

    /* If the symbols has a symbol_type instance associated, free it. */
    if (STT_VAR & SI_CATEGORY(si) ||
	STT_VARIABLE_ARRAY & SI_CATEGORY(si)) {
      SymbType_destroy(SI_VAR_TYPE(si));
    }
    /* The same for symbols that are functions! */
    else if (STT_FUNCTION & SI_CATEGORY(si)) {
      NFunction_destroy((NFunction_ptr) SI_FUNCTION_FUN(si));
    }

    return ASSOC_DELETE;
  }


  /**Function********************************************************************

     Synopsis           [Aux function for symb_cache_deinit]

     Description        [Clears the given list of triggers]

     SideEffects        []

     SeeAlso            [symb_cache_init]

  ******************************************************************************/
  static void symb_cache_free_triggers(NodeList_ptr triggers)
  {
    ListIter_ptr iter;

    if (NODE_LIST(NULL) != triggers) {

      NODE_LIST_FOREACH(triggers, iter) {
	SymbCacheRemoveSymbolStruct* srss =
	  (SymbCacheRemoveSymbolStruct*)NodeList_get_elem_at(triggers, iter);

	if (srss->must_free_arg) {
	  FREE(srss->arg);
	}
	FREE(srss);
      }

      NodeList_destroy(triggers);
    }
  }

}
