/**CHeaderFile*****************************************************************

  FileName    [SymbCache.h]

  PackageName [compile.symb_table]

  Synopsis    [The public interface of class SymbCache]

  Description []

  SeeAlso     [SymbCache.c]

  Author      [Roberto Cavada]

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

#ifndef __SYMB_CACHE_HH__
#define __SYMB_CACHE_HH__

#include "nusmv/core/utils/EnvObject_.h"

extern "C" {
#include "nusmv/core/compile/symb_table/symb_table.h"
#include "nusmv/core/compile/symb_table/SymbType.h"
#include "nusmv/core/node/node.h"
#include "nusmv/core/utils/utils.h"
#include "nusmv/core/utils/NodeList.h"
#include "nusmv/core/compile/symb_table/NFunction.h"
#include "nusmv/core/opt/OptsHandler.h"
#include "nusmv/core/utils/Logger.h"
#include "nusmv/core/utils/assoc.h"
#include "nusmv/core/utils/Stack.h"

}


namespace nusmv {

  struct SymbTableIter;
  struct SymbolInfo;
  class SymbTable;

  typedef boolean (*SymbTableIterFilterFun)(const SymbTable* st,
					    const node_ptr sym, void* arg);
  /** Trigger called by the symbol table when a symbol change status
   */
  typedef void (*SymbTableTriggerFun)(const SymbTable* st,
				      const node_ptr sym,
				      SymbTableTriggerAction action,
				      void* arg);


  class SymbCache : public EnvObject
  {
  public:

    SymbType_ptr get_var_type(const node_ptr name) const;
    node_ptr get_define_body(const node_ptr name) const;
    NFunction_ptr get_function(const node_ptr name) const;
    node_ptr get_actual_parameter(const node_ptr name) const;
    node_ptr get_array_define_body(const node_ptr name) const;
    SymbType_ptr get_variable_array_type(const node_ptr name) const;
    node_ptr get_define_flatten_body(const node_ptr name) const;
    node_ptr get_flatten_actual_parameter(const node_ptr name) const;
    node_ptr get_define_context(const node_ptr name) const;
    node_ptr get_function_context(const node_ptr name) const;
    node_ptr get_actual_parameter_context(const node_ptr name) const;
    node_ptr get_array_define_context(const node_ptr name) const;

    bool is_symbol_state_var(const node_ptr name) const;
    bool is_symbol_frozen_var(const node_ptr name) const;
    bool is_symbol_state_frozen_var(const node_ptr name) const;
    bool is_symbol_input_var(const node_ptr name) const;
    bool is_symbol_var(const node_ptr name) const;
    bool is_symbol_declared(const node_ptr name) const;
    bool is_symbol_define(const node_ptr name) const;
    bool is_symbol_function(const node_ptr name) const;
    bool is_symbol_parameter(const node_ptr name) const;
    bool is_symbol_array_define(const node_ptr name) const;
    bool is_symbol_variable_array(const node_ptr name) const;
    bool is_symbol_constant(const node_ptr name) const;

    bool list_contains_input_var(const NodeList_ptr var_list) const;
    bool list_contains_state_frozen_var(const NodeList_ptr var_list) const;
    bool list_contains_undef_var(const NodeList_ptr var_list) const;

    void remove_parameter(node_ptr formal);


  private:

    SymbCache(const SymbTable* st);
    ~SymbCache();

    void new_input_var(node_ptr var, SymbType_ptr type);
    void new_state_var(node_ptr var, SymbType_ptr type);
    void new_frozen_var(node_ptr var, SymbType_ptr type);
    void redeclare_state_as_frozen_var(node_ptr var);
    void remove_var(node_ptr var);
    void new_define(node_ptr name, node_ptr context, node_ptr definition);
    void new_function(node_ptr name, node_ptr context, NFunction_ptr fun);
    void new_parameter(node_ptr formal, node_ptr context, node_ptr actual);
    void new_array_define(node_ptr name, node_ptr ctx, node_ptr definition);
    void new_variable_array(node_ptr name, SymbType_ptr type);
    void remove_define(node_ptr define);
    void remove_function(node_ptr fun);
    void new_constant(node_ptr name);
    void remove_constant(node_ptr constant);
    void remove_symbols(const node_ptr* symbols, size_t size);

    SymbTableType get_symbol_type(node_ptr symbol) const;
    void gen_iter(SymbTableIter* iter, unsigned int mask) const;
    void next_iter(SymbTableIter* iter) const;
    bool is_iter_end(const SymbTableIter* iter) const;
    node_ptr iter_get_symbol(const SymbTableIter* iter) const;

    void iter_set_filter(SymbTableIter* iter,
			 SymbTableIterFilterFun filter,
			 void* arg) const;

    int get_constants_num() const;
    int get_state_vars_num() const;
    int get_frozen_vars_num() const;
    int get_input_vars_num() const;
    int get_defines_num() const;
    int get_functions_num() const;
    int get_parameters_num() const;
    int get_array_defines_num() const;
    int get_variable_arrays_num() const;

    void add_trigger(SymbTableTriggerFun trigger,
		     SymbTableTriggerAction action,
		     void* arg1, bool must_free_arg);

    void remove_trigger(SymbTableTriggerFun trigger,
			SymbTableTriggerAction action);

    void new_symbol(node_ptr sym, SymbolInfo* symbol_info);
    void remove_symbol(const node_ptr sym, const boolean shrink_if_needed);

    SymbolInfo* alloc_symb_info();
    void free_symb_info(SymbolInfo* si);
    void check_and_shrink_symbols();

    // ----------------------------------------------------------------------
    // Fields
    // ----------------------------------------------------------------------
    const SymbTable* symb_table;
    OptsHandler_ptr options;
    NodeMgr_ptr nodes;
    MasterPrinter_ptr printer;
    Logger_ptr logger;

    hash_ptr symbol_hash;

    SymbolInfo* symbol_info_pool;
    Stack_ptr chunks;

    /* Insertion ordered list */
    node_ptr* symbols;
    size_t symbols_allocated;
    size_t symbols_index;
    size_t symbols_empty;

    /* Counters */
    int constants_num;
    int state_vars_num;
    int input_vars_num;
    int frozen_vars_num;
    int defines_num;
    int functions_num;
    int array_defines_num;
    int variable_arrays_num;
    int parameters_num;

    NodeList_ptr add_triggers;
    NodeList_ptr rem_triggers;
    NodeList_ptr redef_triggers;


    friend class SymbTable;
  };


}

#endif /* __SYMB_CACHE_H__ */
