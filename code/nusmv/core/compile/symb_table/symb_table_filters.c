/* ---------------------------------------------------------------------------

   This file is part of the ``compile.symb_table'' package of NuSMV
   version 2.  Copyright (C) 2016 by FBK-irst.

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

   To contact the NuSMV development board, email to <nusmv@fbk.eu>.

-----------------------------------------------------------------------------*/

/*!
  \author Roberto Cavada, Alessandro Mariotti

  \brief Implementation of some utility functions to be used as
  filters for the symbol table.

*/

#include "nusmv/core/compile/symb_table/symb_table_filters.h"
#include "nusmv/core/compile/symb_table/SymbTable.h"
#include "nusmv/core/compile/symb_table/SymbCache.h"
#include "nusmv/core/parser/symbols.h"


boolean SymbTableIterFilterFun_take_sf_i_symbols(const SymbTable_ptr st,
                                                 const node_ptr sym,
                                                 void* arg)
{
  if (SymbTable_is_symbol_define(st, sym)) {
    SymbCategory cat = SymbTable_get_symbol_category(st, sym);
    return (SYMBOL_STATE_INPUT_NEXT_DEFINE == cat ||
            SYMBOL_STATE_INPUT_DEFINE == cat);
  }

  return SymbTable_is_symbol_var(st, sym);
}

boolean SymbTableIterFilterFun_take_sf_symbols(const SymbTable_ptr st,
                                               const node_ptr sym,
                                               void* arg)
{
  if (SymbTable_is_symbol_define(st, sym)) {
    SymbCategory cat = SymbTable_get_symbol_category(st, sym);
    return (SYMBOL_STATE_NEXT_DEFINE == cat ||
            SYMBOL_STATE_DEFINE == cat ||
            SYMBOL_CONSTANT == cat);
  }

  return (SymbTable_is_symbol_state_var(st, sym) ||
          SymbTable_is_symbol_frozen_var(st, sym));
}

boolean SymbTableIterFilterFun_take_i_symbols(const SymbTable_ptr st,
                                              const node_ptr sym,
                                              void* arg)
{
  if (SymbTable_is_symbol_define(st, sym)) {
    SymbCategory cat = SymbTable_get_symbol_category(st, sym);
    return (SYMBOL_INPUT_NEXT_DEFINE == cat ||
            SYMBOL_INPUT_DEFINE == cat);
  }

  return SymbTable_is_symbol_input_var(st, sym);
}

boolean SymbTableIterFilterFun_skip_var_array_elems(const SymbTable_ptr st,
                                                    const node_ptr sym,
                                                    void* arg)
{
  return !(SymbTable_is_symbol_array_var_element(st, sym));
}

boolean SymbLayerIterFilterFun_skip_var_array_elems(const SymbLayer_ptr st,
                                                    const node_ptr sym,
                                                    void* arg_st)
{
  nusmv_assert(NULL != arg_st);

  return
    SymbTableIterFilterFun_skip_var_array_elems(SYMB_TABLE(arg_st), sym, NULL);
}

boolean SymbTableIterFilterFun_take_bool_vars(const SymbTable_ptr st,
                                              const node_ptr sym,
                                              void* arg)
{
  return (SymbTable_is_symbol_var(st, sym) &&
          SymbType_is_boolean(SymbTable_get_var_type(st, sym)));
}

boolean SymbLayerIterFilterFun_take_bool_vars(const SymbLayer_ptr layer,
                                              const node_ptr sym,
                                              void* arg_st)
{
  nusmv_assert(NULL != arg_st);

  return SymbTableIterFilterFun_take_bool_vars(
      SYMB_TABLE(arg_st), sym, NULL);
}
