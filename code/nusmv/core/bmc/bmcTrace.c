/* ---------------------------------------------------------------------------


  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2010 by FBK.

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
  \author Marco Pensallorto
  \brief This module contains functions to build traces from BE models

  \todo: Missing description

*/


#include "nusmv/core/utils/StreamMgr.h"
#include "nusmv/core/node/NodeMgr.h"
#include "nusmv/core/node/printers/MasterPrinter.h"
#include "nusmv/core/bmc/bmc.h"
#include "nusmv/core/bmc/bmcInt.h"

#include "nusmv/core/trace/pkg_trace.h"
#include "nusmv/core/parser/symbols.h"
/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define BMC_MODEL_DEBUG 0

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static enum st_retval trace_utils_release_bv(char* key, char* data,
                                             char* arg);

static void bmc_model_trace_step_print(const NuSMVEnv_ptr env,
                                       const Trace_ptr trace,
                                       const TraceIter step,
                                       TraceIteratorType it_type,
                                       const char* prefix,
                                       int count);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

Trace_ptr Bmc_create_trace_from_cnf_model(const BeEnc_ptr be_enc,
                                          const NodeList_ptr symbols,
                                          const char* desc,
                                          const TraceType type,
                                          const Slist_ptr cnf_model,
                                          int k)
{
  Trace_ptr trace = Trace_create(BaseEnc_get_symb_table(BASE_ENC(be_enc)),
                                 desc, type, symbols, false);

  return Bmc_fill_trace_from_cnf_model(be_enc, cnf_model, k, trace);
}

Trace_ptr
Bmc_fill_trace_from_cnf_model(const BeEnc_ptr be_enc,
                              const Slist_ptr cnf_model,
                              int k, Trace_ptr trace)
{
  return Bmc_fill_trace_from_partial_cnf_model(be_enc,
                                               cnf_model,
                                               k, trace,
                                               true);

}

Trace_ptr
Bmc_fill_trace_from_partial_cnf_model(const BeEnc_ptr be_enc,
                                      const Slist_ptr cnf_model,
                                      int k, Trace_ptr trace,
                                      boolean complete_trace)
{
  TraceIter first;
  /* local refs */
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(be_enc));
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));
  const BoolEnc_ptr bool_enc = \
    BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(be_enc));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

  const Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  const SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(be_enc));

  hash_ptr tvar_2_bval = new_assoc();
  hash_ptr time_2_step = new_assoc();

  Siter genLiteral;
  nusmv_ptrint cnfLiteral;
  nusmv_ptrint beLiteral;

  int i;

  TRACE_CHECK_INSTANCE(trace);
  nusmv_assert(Trace_is_empty(trace));

  first = Trace_first_iter(trace);

  /* phase 0: setup trace iterators for all times */
  insert_assoc(time_2_step, NODE_FROM_INT(0), (node_ptr)(first));
  for (i = 1; i <= k; ++ i) {
    TraceIter step = Trace_append_step(trace);
    insert_assoc(time_2_step, NODE_FROM_INT(i), (node_ptr)(step));
  }

  /* phase 1: we consider only the cnf variables corresponding to BE
     variables in the range [0, k].

     Thus we ignore the cnf variables that are not corresponding to
     the encoding of the:
     - model variables;
     - encoding variables (sub formulas, loop variables, ...)
  */
  SLIST_FOREACH(cnf_model, genLiteral) {
    int var_idx, ut_index, vtime;
    node_ptr var, key;

    cnfLiteral = (nusmv_ptrint) Siter_element(genLiteral);
    beLiteral = (nusmv_ptrint) Be_CnfLiteral2BeLiteral(be_mgr, cnfLiteral);

    /* if there is no corresponding rbc variable skip this */
    if (0 == beLiteral) continue;

    /* get timed var */
    var_idx = Be_BeLiteral2BeIndex(be_mgr, beLiteral);
    ut_index = BeEnc_index_to_untimed_index(be_enc, var_idx);
    vtime = BeEnc_index_to_time(be_enc, var_idx);
    var = BeEnc_index_to_name(be_enc, ut_index);

    /* needed to adapt to new trace timing format, input is stored
     in the next step */
    if (SymbTable_is_symbol_input_var(st, var)) { ++ vtime; }

    if (vtime > k) continue;

    /* if it's a bit get/create a BitValues structure for
       the scalar variable which this bit belongs */
    if (BoolEnc_is_var_bit(bool_enc, var)) {
      node_ptr scalar_var = BoolEnc_get_scalar_var_from_bit(bool_enc, var);
      BitValues_ptr bv;
      key = find_node(nodemgr, ATTIME, scalar_var, NODE_FROM_INT(vtime));

      bv = BIT_VALUES(find_assoc(tvar_2_bval, key));
      if (BIT_VALUES(NULL) == bv) {
        bv = BitValues_create(bool_enc, scalar_var);
        insert_assoc(tvar_2_bval, key, (node_ptr)(bv));
      }

      /* set the bit value */
      BitValues_set(bv, BoolEnc_get_index_from_bit(bool_enc, var),
                    (beLiteral >= 0) ? BIT_VALUE_TRUE : BIT_VALUE_FALSE);

    }
    else { /* boolean variables do not require any further processing */

      TraceIter timed_step = (-1 != vtime) /* frozenvars */
        ? TRACE_ITER(find_assoc(time_2_step, NODE_FROM_INT(vtime)))
        : first ;

      nusmv_assert(TRACE_END_ITER != timed_step);
      Trace_step_put_value(trace, timed_step, var, beLiteral >= 0
                           ? ExprMgr_true(exprs) : ExprMgr_false(exprs));
    }

  } /* SLIST_FOREACH (phase 1) */

  { /* phase 2: iterate over elements of the hash table (i.e. scalar
      vars) and populate the trace accordingly. */

    node_ptr ts_var;
    BitValues_ptr bitValues;
    assoc_iter aiter;

    ASSOC_FOREACH(tvar_2_bval, aiter, &ts_var, &bitValues) {
      int vtime = NODE_TO_INT(cdr(ts_var)); /* its time */
      node_ptr value = BoolEnc_get_value_from_var_bits(bool_enc, bitValues);

      TraceIter timed_step = (-1 != vtime) /* frozenvars */
        ? TRACE_ITER(find_assoc(time_2_step, NODE_FROM_INT(vtime)))
        : first ;

      nusmv_assert(TRACE_END_ITER != timed_step);
      Trace_step_put_value(trace, timed_step, car(ts_var), value);

      BitValues_destroy(bitValues);
    }
  } /* phase 2: */

  /* phase 3: some assignments may be missing, complete the trace */
  if (complete_trace) {
    bmc_trace_utils_complete_trace(trace, bool_enc);
  }

#if BMC_MODEL_DEBUG
  {
    TraceIter step;
    int i=0;

    StreamMgr_print_error(streams,  "\n--- BMC Model extraction ---\n");
    TRACE_FOREACH(trace, step) {
      ++ i;

      if (1 != i) {
        bmc_model_trace_step_print(env, trace, step, TRACE_ITER_I_VARS,"I", i);
      }

      bmc_model_trace_step_print(env, trace, step, (1 == i) ? TRACE_ITER_SF_VARS
                                 : TRACE_ITER_S_VARS, "S", i);
    }
  }
  StreamMgr_print_error(streams,  "\n\n");
#endif

  /* cleanup */
  free_assoc(tvar_2_bval);
  free_assoc(time_2_step);

  return trace;
} /* Bmc_create_trace_from_cnf_model */

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void bmc_trace_utils_complete_trace(Trace_ptr trace, const BoolEnc_ptr bool_enc)
{
  hash_ptr defaults;
  TraceIter step;

  /* local refs */
  const SymbTable_ptr st = Trace_get_symb_table(trace);
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(st));
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));

  { /* build defaults for scalar vars */
    NodeList_ptr symbols = Trace_get_symbols(trace);
    ListIter_ptr lst_iter;

    defaults = new_assoc();

    NODE_LIST_FOREACH(symbols, lst_iter) {
      node_ptr symb = NodeList_get_elem_at(symbols, lst_iter);
      if (!SymbTable_is_symbol_var(st, symb) ||  \
          SymbType_is_boolean(SymbTable_get_var_type(st, symb))) {
        continue ; /* ignore non-vars and pure-booleans */
      }

      insert_assoc(defaults, symb, (node_ptr)(BitValues_create(bool_enc, symb)));
    }
  }

  TRACE_FOREACH(trace, step) {
    TraceSymbolsIter sym_iter;
    node_ptr var;

    TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_ALL_VARS, sym_iter, var) {
      node_ptr val = Trace_step_get_value(trace, step, var);

      if (Nil == val) { /* not assigned */

        /* no inputs on first step */
        if (SymbTable_is_symbol_input_var(st, var) && \
            (Trace_first_iter(trace) == step)) { continue; }

        if (!SymbType_is_boolean(SymbTable_get_var_type(st, var))) {
          /* default scalar value */
          Trace_step_put_value(trace, step, var,
                               BoolEnc_get_value_from_var_bits(bool_enc,
                                         BIT_VALUES(find_assoc(defaults, var))));
        }
        else {
          /* default boolean value (false) */
          Trace_step_put_value(trace, step, var, ExprMgr_false(exprs));
        }
      }
    }
  } /* trace_foreach */

  { /* dispose defaults */
    clear_assoc_and_free_entries(defaults, trace_utils_release_bv);
    free_assoc(defaults);
  }

} /* bmc_trace_utils_complete_trace */

void bmc_trace_utils_append_input_state(Trace_ptr trace, BeEnc_ptr be_enc,
                                        const Slist_ptr cnf_model)
{
  TraceIter step = Trace_append_step(trace);

  /* local refs */
  const BoolEnc_ptr bool_enc = \
    BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(be_enc));

  const Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  const SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(be_enc));
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(be_enc));
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));

  hash_ptr values = new_assoc();

  Siter genLiteral;
  nusmv_ptrint cnfLiteral;
  nusmv_ptrint beLiteral;

  ListIter_ptr lst_iter;

  { /* phase 0: setup values for all input and state vars (no frozens) */
    NodeList_ptr symbols;
    symbols = Trace_get_s_vars(trace);
    NODE_LIST_FOREACH(symbols, lst_iter) {
      node_ptr symb = NodeList_get_elem_at(symbols, lst_iter);

      /* if it's a scalar var, create an empty bv and associate to it */
      if (!SymbType_is_boolean(SymbTable_get_var_type(st, symb))) {
        insert_assoc(values, symb,
                     (node_ptr)(BitValues_create(bool_enc, symb)));
      }

      /* if it's a boolean, default is false */
      else { insert_assoc(values, symb, ExprMgr_false(exprs)); }
    }
    symbols = Trace_get_i_vars(trace);
    NODE_LIST_FOREACH(symbols, lst_iter) {
      node_ptr symb = NodeList_get_elem_at(symbols, lst_iter);

      /* if it's a scalar var, create an empty bv and associate to it */
      if (!SymbType_is_boolean(SymbTable_get_var_type(st, symb))) {
        insert_assoc(values, symb,
                     (node_ptr)(BitValues_create(bool_enc, symb)));
      }

      /* if it's a boolean, default is false */
      else { insert_assoc(values, symb, ExprMgr_false(exprs)); }
    }
  } /* 0 */

  /* phase 1: we consider only the cnf variables corresponding to BE
     variables in the range at time 1.

     Thus we ignore the cnf variables that are not corresponding to
     the encoding of the:
     - model variables;
     - encoding variables (sub formulas, loop variables, ...)
  */
  SLIST_FOREACH(cnf_model, genLiteral) {
    int var_idx, ut_index, vtime;
    node_ptr var;

    cnfLiteral = (nusmv_ptrint) Siter_element(genLiteral);
    beLiteral = (nusmv_ptrint) Be_CnfLiteral2BeLiteral(be_mgr, cnfLiteral);

    /* if there is no corresponding rbc variable skip this */
    if (0 == beLiteral) continue;

    /* get timed var */
    var_idx = Be_BeLiteral2BeIndex(be_mgr, beLiteral);
    ut_index = BeEnc_index_to_untimed_index(be_enc, var_idx);
    vtime = BeEnc_index_to_time(be_enc, var_idx);
    var = BeEnc_index_to_name(be_enc, ut_index);

    /* needed to adapt to new trace timing format, input is stored
     in the next step */
    if (SymbTable_is_symbol_input_var(st, var)) { ++ vtime; }

    /* we're interested only in (input, next) both have time 1,
       skip anything else */
    if (1 != vtime) continue;

    /* if it's a bit get/create a BitValues structure for
       the scalar variable which this bit belongs to */
    if (BoolEnc_is_var_bit(bool_enc, var)) {
      node_ptr scalar_var = BoolEnc_get_scalar_var_from_bit(bool_enc, var);
      BitValues_ptr bv = BIT_VALUES(find_assoc(values, scalar_var));

      /* set the bit value */
      BitValues_set(bv, BoolEnc_get_index_from_bit(bool_enc, var),
                    (beLiteral >= 0) ? BIT_VALUE_TRUE : BIT_VALUE_FALSE);

    }
    else { /* boolean variables do not require any further processing */
      insert_assoc(values, var,
                   (beLiteral >= 0) ? ExprMgr_true(exprs) : ExprMgr_false(exprs));
    }
  } /* SLIST_FOREACH (phase 1) */

  { /* phase 2: iterate over elements of the values hash table and
       populate the trace accordingly. */
    assoc_iter aiter;
    node_ptr var;
    void* val;

    ASSOC_FOREACH(values, aiter, &var, &val) {
      boolean is_scalar = \
        (!SymbType_is_boolean(SymbTable_get_var_type(st, var)));

      /* get value from values assoc */
      node_ptr value = (is_scalar)
        ? BoolEnc_get_value_from_var_bits(bool_enc,
                                          BIT_VALUES(val))
        : (node_ptr)val;

      /* write into trace */
      Trace_step_put_value(trace, step, var, value);

      if (is_scalar) {
        BitValues_destroy(BIT_VALUES(val));
      }
    }
  } /* phase 2: */

  /* cleanup */
  free_assoc(values);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
static void bmc_model_trace_step_print(const NuSMVEnv_ptr env,
                                       const Trace_ptr trace,
                                       const TraceIter step,
                                       TraceIteratorType it_type,
                                       const char* prefix,
                                       int count)
{
  const StreamMgr_ptr streams =
    STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));

  TraceStepIter iter;
  node_ptr symb, val;

  if (0 < count) { StreamMgr_print_error(streams,  "%s%d:", prefix, count); }
  else { StreamMgr_print_error(streams,  "%s:", prefix); }

  TRACE_STEP_FOREACH(trace, step, it_type, iter, symb, val) {
    StreamMgr_nprint_error(streams, wffprint, "%N", symb); StreamMgr_print_error(streams,  "=");
    StreamMgr_nprint_error(streams, wffprint, "%N", val); StreamMgr_print_error(streams,  " ");
  }

  StreamMgr_print_error(streams,  "\n");
} /* bmc_model_trace_step_print */


static enum st_retval
trace_utils_release_bv (char* key, char* data, char* arg)
{
  BitValues_ptr bv = BIT_VALUES(data);
  BitValues_destroy(bv);

  return ST_DELETE;
}


