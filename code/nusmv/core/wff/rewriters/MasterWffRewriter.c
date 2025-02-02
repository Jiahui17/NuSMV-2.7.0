/* ---------------------------------------------------------------------------

  This file is part of the ``wff.rewriters'' package of NuSMV version 2.
  Copyright (C) 2014 by FBK-irst.

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
   \author Michele Dorigatti, Marco Roveri, Enrico Magnago (refactoring)

   \brief Implementation of class 'MasterWffRewriter'

*/

#include "nusmv/core/wff/rewriters/MasterWffRewriter.h"
#include "nusmv/core/wff/rewriters/MasterWffRewriter_private.h"

#include "nusmv/core/wff/wff.h"
#include "nusmv/core/compile/compile.h"
#include "nusmv/core/utils/Slist.h"

#include "nusmv/core/utils/ErrorMgr.h"
#include "nusmv/core/utils/error.h"
#include "nusmv/core/parser/symbols.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

typedef enum {
  WFF_REWRITER_PROPOSITIONAL,
  WFF_REWRITER_PURE_INVAR,
  WFF_REWRITER_INIT_INVAR,
  WFF_REWRITER_PROP_IMP_INVAR,
  WFF_REWRITER_INVAR_IMP_INVAR,
  WFF_REWRITER_INIT_INVAR_TRANS_IMP_INVAR,
  WFF_REWRITER_NONE
} WffRewriter_InvariantKind;

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'MasterWffRewriter_private.h' for class 'MasterWffRewriter' definition. */


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void master_wff_rewriter_finalize(Object_ptr object, void* dummy);

static Pair_ptr
master_wff_rewriter_rewrite_generalized_property(const MasterWffRewriter_ptr self,
                                                 const WffRewriteMethod method,
                                                 const SymbLayer_ptr layer,
                                                 const FlatHierarchy_ptr outfh,
                                                 const node_ptr spec,
                                                 const short int spec_type,
                                                 const boolean initialize_monitor_to_true);

static Pair_ptr
master_wff_rewriter_extract_next_input_predicates(const MasterWffRewriter_ptr self,
                                                  const WffRewriteMethod method,
                                                  const SymbLayer_ptr layer,
                                                  const FlatHierarchy_ptr outfh,
                                                  node_ptr spec,
                                                  const short int spec_type);

static void
master_wff_rewriter_fill_layer_and_hierarchy(const MasterWffRewriter_ptr self,
                                             const WffRewriteMethod method,
                                             const SymbLayer_ptr layer,
                                             const FlatHierarchy_ptr outfh,
                                             const NodeList_ptr new_var_exprs,
                                             const boolean initialize_monitor_to_true);

static Pair_ptr
master_wff_rewriter_rewrite_ltl2invar(const MasterWffRewriter_ptr self,
                                      const WffRewriteMethod method,
                                      const WffRewriterExpectedProperty eproptype,
                                      const SymbLayer_ptr layer,
                                      const FlatHierarchy_ptr outfh,
                                      const node_ptr spec,
                                      const short int spec_type,
                                      const boolean initialize_monitor_to_true,
                                      const boolean ltl2invar_negate_property);

static WffRewriter_InvariantKind
master_wff_rewriter_invariant_kind(const SymbTable_ptr st,
                                   const node_ptr spec,
                                   const node_ptr context,
                                   const WffRewriter_InvariantKind outer);

static void master_wff_rewriter_init_invariant_process_members(const SymbTable_ptr st,
                                                               const node_ptr spec,
                                                               const node_ptr context,
                                                               FlatHierarchy_ptr fh,
                                                               const boolean is_outer_invar);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

MasterWffRewriter_ptr
MasterWffRewriter_create(const NuSMVEnv_ptr env)
{
  const MasterWffRewriter_ptr self = ALLOC(MasterWffRewriter, 1);
  MASTER_WFF_REWRITER_CHECK_INSTANCE(self);

  master_wff_rewriter_init(self, env);
  return self;
}

void MasterWffRewriter_destroy(MasterWffRewriter_ptr self)
{
  MASTER_WFF_REWRITER_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

Pair_ptr MasterWffRewriter_rewrite_formula(const MasterWffRewriter_ptr self,
                                           const WffRewriteMethod method,
                                           const WffRewriterExpectedProperty eproptype,
                                           const SymbLayer_ptr layer,
                                           const FlatHierarchy_ptr outfh,
                                           const node_ptr spec,
                                           const short int spec_type,
                                           const boolean initialize_monitor_to_true,
                                           const boolean ltl2invar_negate_property)
{


  Pair_ptr retval = NULL;
  SymbTable_ptr st = NULL;

  FLAT_HIERARCHY_CHECK_INSTANCE(outfh);
  SYMB_LAYER_CHECK_INSTANCE(layer);

  switch(spec_type) {
  case SPEC: break;
  case LTLSPEC: break;
  case INVARSPEC: break;
  case PSLSPEC: break;
  case COMPUTE: break;
  default:
    error_unreachable_code_msg("Unrecognized specification type");
  }

  if (Nil == spec) return PAIR(NULL);

  st = FlatHierarchy_get_symb_table(outfh);

  /* Maybe there is nothing to do */
  if (MasterWffRewriter_is_rewriting_needed(self, st, eproptype, spec, Nil)) {
    switch(eproptype) {
    case WFF_REWRITER_REWRITE_INPUT_NEXT:

      retval =
        master_wff_rewriter_rewrite_generalized_property(self, method,
                                                         layer, outfh,
                                                         spec, spec_type,
                                                         initialize_monitor_to_true);
      break;
    case WFF_REWRITER_LTL_2_INVAR:
      retval =
        master_wff_rewriter_rewrite_ltl2invar(self, method, eproptype,
                                              layer, outfh,
                                              spec, spec_type,
                                              initialize_monitor_to_true,
                                              ltl2invar_negate_property);
      break;
    default:
      error_unreachable_code_msg("Unrecognized rewriting of expected input property");
    }
  }
  else {
    /* No rewriting is needed */
    retval = Pair_create(spec, VOIDPTR_FROM_INT(spec_type));
  }

  nusmv_assert(!MasterWffRewriter_is_rewriting_needed(self, st,
                                                      WFF_REWRITER_REWRITE_INPUT_NEXT,
                                                      NODE_PTR(Pair_get_first(retval)),
                                                      Nil));
#ifndef NDEBUG
  {
    const node_ptr rewritten = NODE_PTR(Pair_get_first(retval));
    const TypeChecker_ptr type_checker =
      SymbTable_get_type_checker(st);
    /* computed expression should be well-formed */
    nusmv_assert(TypeChecker_is_expression_wellformed(type_checker,
                                                      rewritten, Nil));

  }
#endif
  return retval;
}


boolean MasterWffRewriter_is_rewriting_needed(MasterWffRewriter_ptr self,
                                              const SymbTable_ptr st,
                                              const WffRewriterExpectedProperty eproptype,
                                              const node_ptr wff,
                                              const node_ptr context)
{
  const NodeList_ptr walkers = MASTER_NODE_WALKER(self)->walkers;
  ListIter_ptr iter;

  if (eproptype == WFF_REWRITER_LTL_2_INVAR)
    return true;

  NODE_LIST_CHECK_INSTANCE(walkers);

  NODE_LIST_FOREACH(walkers, iter) {
    const WffRewriterBase_ptr rewriter =
      WFF_REWRITER_BASE(NodeList_get_elem_at(walkers, iter));

    WFF_REWRITER_BASE_CHECK_INSTANCE(rewriter);

    if (NodeWalker_can_handle(NODE_WALKER(rewriter), wff)) {
      return WffRewriterBase_is_rewriting_needed(rewriter, st, eproptype,
                                                 wff, context);
    }
  }

  {
    /* Fall back */
    const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
    const MasterPrinter_ptr wffprint =
      MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));
    const MasterPrinter_ptr sexpprint =
      MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_SEXP_PRINTER));
    const ErrorMgr_ptr errmgr =
      ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
    const StreamMgr_ptr streams =
      STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));

    StreamMgr_print_error(streams,
                          "Error: no compatible wff rewriter "
                          "found for expression:\n");
    StreamMgr_nprint_error(streams, wffprint, "%N", wff);
    StreamMgr_print_error(streams,  "\n");
    StreamMgr_nprint_error(streams, sexpprint, "%N", wff);
    ErrorMgr_nusmv_exit(errmgr, 1);
  }
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


void master_wff_rewriter_init(MasterWffRewriter_ptr self,
                              const NuSMVEnv_ptr env)
{
  const Slist_ptr walker_factories =
    SLIST(NuSMVEnv_get_value(env, ENV_WFF_REWRITER_LIST));
  Siter iter;

  SLIST_CHECK_INSTANCE(walker_factories);

  /* base class initialization */
  master_node_walker_init(MASTER_NODE_WALKER(self), env);

  /* members initialization */
  SLIST_FOREACH(walker_factories, iter) {

    const typeWffRewriterCreate factory =
      (typeWffRewriterCreate)Siter_element(iter);
    const WffRewriterBase_ptr rewriter = factory(env);

    WFF_REWRITER_BASE_CHECK_INSTANCE(rewriter);

    MasterNodeWalker_register_walker(MASTER_NODE_WALKER(self),
                                     NODE_WALKER(rewriter));
  }

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = master_wff_rewriter_finalize;
  OVERRIDE(Object, copy) = NULL; /* forbid copy */
}


void master_wff_rewriter_deinit(MasterWffRewriter_ptr self) {

  /* base class deinitialization */
  master_node_walker_deinit(MASTER_NODE_WALKER(self));

  /* members deinitialization */

}

WffRewriteFormulaKind
master_wff_rewriter_rewrite(const MasterWffRewriter_ptr self,
                            const WffRewriteMethod method,
                            const SymbLayer_ptr layer,
                            const FlatHierarchy_ptr outfh,
                            node_ptr* const expr,
                            const short int spec_type,
                            const NodeList_ptr new_var_exprs)
{
  const NodeList_ptr walkers = MASTER_NODE_WALKER(self)->walkers;
  const node_ptr wff = *expr;
  ListIter_ptr iter;

  NODE_LIST_CHECK_INSTANCE(walkers);

  NODE_LIST_FOREACH(walkers, iter) {
    const WffRewriterBase_ptr rewriter =
      WFF_REWRITER_BASE(NodeList_get_elem_at(walkers, iter));

    WFF_REWRITER_BASE_CHECK_INSTANCE(rewriter);

    if (NodeWalker_can_handle(NODE_WALKER(rewriter), wff)) {
      return WffRewriterBase_rewrite_formula(rewriter, method,
                                             layer, outfh,
                                             expr, spec_type,
                                             new_var_exprs);
    }
  }

  {
    /* Fall back */
    const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
    const MasterPrinter_ptr wffprint =
      MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));
    const MasterPrinter_ptr sexpprint =
      MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_SEXP_PRINTER));
    const ErrorMgr_ptr errmgr =
      ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
    const StreamMgr_ptr streams =
      STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));

    StreamMgr_print_error(streams,
                          "Error: no compatible wff rewriter "
                          "found for expression:\n");
    StreamMgr_nprint_error(streams, wffprint, "%N", wff);
    StreamMgr_print_error(streams,  "\n");
    StreamMgr_nprint_error(streams, sexpprint, "%N", wff);
    ErrorMgr_nusmv_exit(errmgr, 1);
  }
}


node_ptr
master_wff_rewriter_create_substitution(const MasterWffRewriter_ptr self,
                                        const WffRewriteMethod method,
                                        const FlatHierarchy_ptr outfh,
                                        const node_ptr spec,
                                        const short int spec_type,
                                        const NodeList_ptr new_var_exprs)
{
  const NuSMVEnv_ptr env = ENV_OBJECT_GET_ENV(self);
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

  node_ptr result = Nil;
  const SymbTable_ptr symb_table = FlatHierarchy_get_symb_table(outfh);
  /* declare a new variable */
  const node_ptr new_var =
    SymbTable_get_fresh_symbol_name(symb_table, MASTER_WFF_REWRITER_MONITOR_PREFIX);
  const Pair_ptr pair = Pair_create(new_var, spec);

  NodeList_append(new_var_exprs, NODE_PTR(pair));

  switch(method) {
  case WFF_REWRITE_METHOD_STANDARD:
    result = new_var;
    break;

  case WFF_REWRITE_METHOD_DEADLOCK_FREE:
    if (INVARSPEC == spec_type) {
      result = new_var;
    }
    else {
      result = Wff_make_opnext(nodemgr, new_var);
    }

    break;

  default: error_unreachable_code();
  }

  return result;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The MasterWffRewriter class virtual finalizer

  Called by the class destructor
*/
static void master_wff_rewriter_finalize(Object_ptr object, void* dummy)
{
  MasterWffRewriter_ptr self = MASTER_WFF_REWRITER(object);

  master_wff_rewriter_deinit(self);
  FREE(self);
}

static Pair_ptr
master_wff_rewriter_rewrite_generalized_property(const MasterWffRewriter_ptr self,
                                                 const WffRewriteMethod method,
                                                 const SymbLayer_ptr layer,
                                                 const FlatHierarchy_ptr outfh,
                                                 const node_ptr spec,
                                                 const short int spec_type,
                                                 const boolean initialize_monitor_to_true)
{
  Pair_ptr retval = NULL;
  const SymbTable_ptr symb_table = FlatHierarchy_get_symb_table(outfh);
  const Pair_ptr pair =
    master_wff_rewriter_extract_next_input_predicates(self, method, layer, outfh,
                                                      spec, spec_type);

  const NodeList_ptr new_var_exprs = NODE_LIST(Pair_get_second(pair));

  nusmv_assert(SymbTable_has_layer(symb_table, SymbLayer_get_name(layer)));


  master_wff_rewriter_fill_layer_and_hierarchy(self, method, layer, outfh,
                                               new_var_exprs,
                                               initialize_monitor_to_true);


  /* free list of new vars exprs */
  if(new_var_exprs != NODE_LIST(NULL)) {
    ListIter_ptr iter;
    NODE_LIST_FOREACH(new_var_exprs, iter) {
      const Pair_ptr elem =
        PAIR(NodeList_get_elem_at(new_var_exprs, iter));
      Pair_destroy(elem);
    }

    NodeList_destroy(new_var_exprs);
  }

  retval = Pair_create(Pair_get_first(pair), VOIDPTR_FROM_INT(spec_type));

  Pair_destroy(pair);

  return retval;
}

/*!
  \brief Rewrites the formula and collects the map monitor ->
  predicate

  next/input predicates are substituted and collected
*/
static Pair_ptr
master_wff_rewriter_extract_next_input_predicates(const MasterWffRewriter_ptr self,
                                                  const WffRewriteMethod method,
                                                  const SymbLayer_ptr layer,
                                                  const FlatHierarchy_ptr outfh,
                                                  node_ptr spec,
                                                  const short int spec_type)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  const NodeList_ptr new_var_exprs = NodeList_create();
  WffRewriteFormulaKind formula_kind = WFF_REWRITE_FORMULA_KIND_FIRST;
  Pair_ptr retval = NULL;
  SymbTable_ptr symb_table = NULL;

  symb_table = FlatHierarchy_get_symb_table(outfh);
  spec = Compile_FlattenSexpExpandDefine(symb_table, spec, Nil);

  if (INVARSPEC == spec_type) {
    node_ptr formula_to_free = Nil;
    /* convert temporary to ltl to process it with the same function */
    spec = find_node(nodemgr, OP_GLOBAL, spec, Nil);
    formula_kind =
      master_wff_rewriter_rewrite(self, method, layer, outfh,
                                  &spec, spec_type, new_var_exprs);
    formula_to_free = spec;
    spec = car(spec);
    free_node(nodemgr, formula_to_free);
  }
  else {
    nusmv_assert(LTLSPEC == spec_type);

    formula_kind =
      master_wff_rewriter_rewrite(self, method, layer, outfh,
                                  &spec, spec_type, new_var_exprs);

  }

  /* It is still possible to have next or input vars if they were at top
     level. Example:
     LTLSPEC next(a) */

  /* the bitwise operator is wanted. We have do something if the formula has
     still input vars and/or next operators */
  if (WFF_REWRITE_FORMULA_KIND_INPUT_NEXT & formula_kind) {
    spec = master_wff_rewriter_create_substitution(self, method, outfh,
                                                   spec, spec_type,
                                                   new_var_exprs);
  }

  retval = Pair_create(spec, new_var_exprs);

  return retval;
}


/*!
  \brief Given the map monitor -> predicate, builds the
  corresponding flat hierarchy

  Given the map monitor -> predicate, builds the
  corresponding flat hierarchy
*/
static void
master_wff_rewriter_fill_layer_and_hierarchy(const MasterWffRewriter_ptr self,
                                             const WffRewriteMethod method,
                                             const SymbLayer_ptr layer,
                                             const FlatHierarchy_ptr outfh,
                                             const NodeList_ptr new_var_exprs,
                                             const boolean initialize_monitor_to_true)
{
  const NuSMVEnv_ptr env = ENV_OBJECT_GET_ENV(self);
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));
  const SymbTable_ptr symb_table = FlatHierarchy_get_symb_table(outfh);
  ListIter_ptr iter;

  /* declare the new variables and create hierarchy expressions */
  NODE_LIST_FOREACH(new_var_exprs, iter) {
    const SymbType_ptr bool_type = SymbType_create(env, SYMB_TYPE_BOOLEAN, Nil);
    const Pair_ptr tmp = PAIR(NodeList_get_elem_at(new_var_exprs, iter));
    const node_ptr var = NODE_PTR(Pair_get_first(tmp));
    const node_ptr expr = NODE_PTR(Pair_get_second(tmp));

    FlatHierarchy_add_var(outfh, var);
    SymbLayer_declare_state_var(layer, var, bool_type);

    switch(method) {
    case WFF_REWRITE_METHOD_STANDARD:
      FlatHierarchy_add_trans(outfh,
                              ExprMgr_iff(exprs, var, expr));
      break;

    case WFF_REWRITE_METHOD_DEADLOCK_FREE:
      if (initialize_monitor_to_true) {
        FlatHierarchy_add_init(outfh, var);
      }
      else {
        FlatHierarchy_add_init(outfh, ExprMgr_not(exprs, var));
      }
      FlatHierarchy_add_trans(outfh,
                              ExprMgr_iff(exprs,
                                          ExprMgr_next(exprs, var, symb_table),
                                          expr));
      break;

    default: error_unreachable_code();
    }
  }
}

/*!
  \methodof WffRewriter
  \brief WffRewriter private ltl2invar rewriting method.

*/
static Pair_ptr
master_wff_rewriter_rewrite_ltl2invar(const MasterWffRewriter_ptr self,
                                      const WffRewriteMethod method,
                                      const WffRewriterExpectedProperty eproptype,
                                      const SymbLayer_ptr layer,
                                      const FlatHierarchy_ptr outfh,
                                      const node_ptr spec,
                                      const short int spec_type,
                                      const boolean initialize_monitor_to_true,
                                      const boolean ltl2invar_negate_property)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  Pair_ptr retval = PAIR(NULL);
  if (INVARSPEC == spec_type) {
    retval =
      master_wff_rewriter_rewrite_generalized_property(self, method,
                                                       layer, outfh,
                                                       spec, spec_type,
                                                       initialize_monitor_to_true);
  }
  else {
    const SymbTable_ptr st = FlatHierarchy_get_symb_table(outfh);
    const WffRewriter_InvariantKind kind =
      master_wff_rewriter_invariant_kind(st, spec, Nil, WFF_REWRITER_NONE);


    nusmv_assert(LTLSPEC == spec_type);

    if (WFF_REWRITER_NONE == kind || WFF_REWRITER_PROPOSITIONAL == kind) {
      /* No rewriting is needed */
      const NodeMgr_ptr nodemgr =
        NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

      if (MasterWffRewriter_is_rewriting_needed(self, st,
                                                WFF_REWRITER_REWRITE_INPUT_NEXT,
                                                spec, Nil)) {
        retval =
          master_wff_rewriter_rewrite_generalized_property(self, method,
                                                           layer, outfh,
                                                           spec, spec_type,
                                                           initialize_monitor_to_true);
      }
      else {
        retval = Pair_create(find_node(nodemgr, node_get_type(spec),
                                       car(spec), cdr(spec)),
                             VOIDPTR_FROM_INT(spec_type));
      }
    }
    else { /* It is needed to perform some transformation */
      const node_ptr flatten_spec =
        Compile_FlattenSexpExpandDefine(st, spec, Nil);


      switch(kind) {

      case WFF_REWRITER_PURE_INVAR:
      {
        const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));
        node_ptr invariant = car(flatten_spec);

        nusmv_assert(Wff_is_propositional(st, invariant, Nil, true));

        if (ltl2invar_negate_property) {
          invariant = ExprMgr_not(exprs, invariant);
        }

        retval =
           master_wff_rewriter_rewrite_generalized_property(self, method,
                                                            layer, outfh,
                                                            invariant, INVARSPEC,
                                                            initialize_monitor_to_true);
      }
      break;

      case WFF_REWRITER_PROP_IMP_INVAR:
      case WFF_REWRITER_INVAR_IMP_INVAR:
      case WFF_REWRITER_INIT_INVAR_TRANS_IMP_INVAR:
      {
        const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));
        const FlatHierarchy_ptr premises_fh = FlatHierarchy_create(st);
        node_ptr premises;
        node_ptr invariant;

        nusmv_assert(IMPLIES == node_get_type(flatten_spec));
        nusmv_assert(OP_GLOBAL == node_get_type(cdr(flatten_spec)));

        premises = car(flatten_spec);
        invariant = car(cdr(flatten_spec));

        master_wff_rewriter_init_invariant_process_members(st, premises, Nil, premises_fh, false);

        if (ltl2invar_negate_property) {
          invariant = ExprMgr_not(exprs, invariant);
        }

        if (MasterWffRewriter_is_rewriting_needed(self, st,
                                                  WFF_REWRITER_REWRITE_INPUT_NEXT,
                                                  invariant, Nil)) {
          retval =
            master_wff_rewriter_rewrite_generalized_property(self, method,
                                                             layer, outfh,
                                                             invariant, INVARSPEC,
                                                             initialize_monitor_to_true);
        }
        else {
          retval = Pair_create(invariant, VOIDPTR_FROM_INT(INVARSPEC));
        }
        FlatHierarchy_mergeinto(outfh, premises_fh);
        FlatHierarchy_destroy(premises_fh);
     }
      break;

      default:
        error_unreachable_code_msg("Unexpected expression kind");
      }

    }
  }

  nusmv_assert(retval != PAIR(NULL));
  return retval;
}

static WffRewriter_InvariantKind
master_wff_rewriter_invariant_kind(const SymbTable_ptr st,
                                   const node_ptr spec,
                                   const node_ptr context,
                                   const WffRewriter_InvariantKind outer)
{
  if (Wff_is_propositional(st, spec, context, true))
    return WFF_REWRITER_PROPOSITIONAL;

  switch (node_get_type(spec)) {
    case CONTEXT:
      return master_wff_rewriter_invariant_kind(st, cdr(spec), car(spec), outer);
      break;

    case OP_GLOBAL:
      if (WFF_REWRITER_NONE == outer || WFF_REWRITER_PURE_INVAR == outer) {
        const WffRewriter_InvariantKind result =
            master_wff_rewriter_invariant_kind(st, car(spec), context,
                                               WFF_REWRITER_PURE_INVAR);
        if (WFF_REWRITER_PROPOSITIONAL == result ||
            WFF_REWRITER_PURE_INVAR == result) {
          return WFF_REWRITER_PURE_INVAR;
        }
      }
      return WFF_REWRITER_NONE;
      break;

    case AND:
      if (WFF_REWRITER_NONE == outer || WFF_REWRITER_PURE_INVAR == outer) {
        const WffRewriter_InvariantKind left =
          master_wff_rewriter_invariant_kind(st, car(spec), context, outer);

        if (WFF_REWRITER_PROPOSITIONAL == left ||
            WFF_REWRITER_PURE_INVAR == left ||
            WFF_REWRITER_INIT_INVAR == left) {
          const WffRewriter_InvariantKind right =
            master_wff_rewriter_invariant_kind(st, cdr(spec), context, outer);

          if (WFF_REWRITER_PROPOSITIONAL == left &&
              WFF_REWRITER_PROPOSITIONAL == right &&
              WFF_REWRITER_PURE_INVAR == outer)
            return WFF_REWRITER_PURE_INVAR;

          if (WFF_REWRITER_PROPOSITIONAL == left &&
              WFF_REWRITER_PURE_INVAR == right &&
              WFF_REWRITER_PURE_INVAR == outer)
            return WFF_REWRITER_PURE_INVAR;

          if (WFF_REWRITER_PURE_INVAR == left &&
              WFF_REWRITER_PROPOSITIONAL == right &&
              WFF_REWRITER_PURE_INVAR == outer)
            return WFF_REWRITER_PURE_INVAR;

          if (WFF_REWRITER_PURE_INVAR == left &&
              WFF_REWRITER_PURE_INVAR == right)
            return WFF_REWRITER_PURE_INVAR;

          if ((WFF_REWRITER_PROPOSITIONAL == left ||
               WFF_REWRITER_PURE_INVAR == left ||
               WFF_REWRITER_INIT_INVAR == left) &&
              (WFF_REWRITER_PROPOSITIONAL == right ||
               WFF_REWRITER_PURE_INVAR == right ||
               WFF_REWRITER_INIT_INVAR == right) &&
              WFF_REWRITER_NONE == outer)
            return WFF_REWRITER_INIT_INVAR;

        }
      }

      return WFF_REWRITER_NONE;
      break;

    case IMPLIES:
      if (WFF_REWRITER_NONE == outer) {
        WffRewriter_InvariantKind right;
        const WffRewriter_InvariantKind left =
          master_wff_rewriter_invariant_kind(st, car(spec), context, outer);

        if (WFF_REWRITER_PROPOSITIONAL == left ||
            WFF_REWRITER_PURE_INVAR == left ||
            WFF_REWRITER_INIT_INVAR == left) {
          right =
            master_wff_rewriter_invariant_kind(st, cdr(spec), context, outer);

          if (WFF_REWRITER_PROPOSITIONAL == left &&
              WFF_REWRITER_PROPOSITIONAL == right)
            return WFF_REWRITER_PROPOSITIONAL;

          if (WFF_REWRITER_PROPOSITIONAL == left &&
              WFF_REWRITER_PURE_INVAR == right)
            return WFF_REWRITER_PROP_IMP_INVAR;

          /* We disregard (G p) -> q */
          if (WFF_REWRITER_PURE_INVAR == left &&
              WFF_REWRITER_PROPOSITIONAL == right)
            return WFF_REWRITER_NONE;

          if (WFF_REWRITER_PURE_INVAR == left &&
              WFF_REWRITER_PURE_INVAR == right)
            return WFF_REWRITER_INVAR_IMP_INVAR;

          if (WFF_REWRITER_INIT_INVAR == left &&
              WFF_REWRITER_PURE_INVAR == right)
            return WFF_REWRITER_INIT_INVAR_TRANS_IMP_INVAR;
        }
      }

      return WFF_REWRITER_NONE;
      break;

    case OR:
      /* handles implication equivalences:
         (!p | G q), (!G p | G q), (G q | !p), (G q | !G p)

         (![G] p | q) is disregarded as done for implication.
         (This was done to fix issue 5178)
      */
      if (WFF_REWRITER_NONE == outer) {
        WffRewriter_InvariantKind left, right;

        if (NOT == node_get_type(car(spec))) {
          left =
            master_wff_rewriter_invariant_kind(st,  car(car(spec)), context, outer);
          if (WFF_REWRITER_PROPOSITIONAL == left ||
              WFF_REWRITER_PURE_INVAR == left) {
            right =
              master_wff_rewriter_invariant_kind(st,  cdr(spec), context, outer);

            if (WFF_REWRITER_PROPOSITIONAL == left &&
                WFF_REWRITER_PROPOSITIONAL == right)
              return WFF_REWRITER_PROPOSITIONAL;

            if (WFF_REWRITER_PROPOSITIONAL == left &&
                WFF_REWRITER_PURE_INVAR == right)
              return WFF_REWRITER_PROP_IMP_INVAR;

            /* We disregard !(G p) | q */
            if (WFF_REWRITER_PURE_INVAR == left &&
                WFF_REWRITER_PROPOSITIONAL == right)
              return WFF_REWRITER_NONE;

            if (WFF_REWRITER_PURE_INVAR == left &&
                WFF_REWRITER_PURE_INVAR == right)
              return WFF_REWRITER_INVAR_IMP_INVAR;
          }
        }
        else if (NOT == node_get_type(cdr(spec))) {
          right =
            master_wff_rewriter_invariant_kind(st,  car(cdr(spec)), context, outer);
          if (WFF_REWRITER_PROPOSITIONAL == right ||
              WFF_REWRITER_PURE_INVAR == right) {
            left =
              master_wff_rewriter_invariant_kind(st,  car(spec), context, outer);

            if (WFF_REWRITER_PROPOSITIONAL == right &&
                WFF_REWRITER_PROPOSITIONAL == left)
              return WFF_REWRITER_PROPOSITIONAL;

            if (WFF_REWRITER_PROPOSITIONAL == right &&
                WFF_REWRITER_PURE_INVAR == left)
              return WFF_REWRITER_PROP_IMP_INVAR;

            /* We disregard q | !(G p) */
            if (WFF_REWRITER_PURE_INVAR == right &&
                WFF_REWRITER_PROPOSITIONAL == left)
              return WFF_REWRITER_NONE;

            if (WFF_REWRITER_PURE_INVAR == right &&
                WFF_REWRITER_PURE_INVAR == left)
              return WFF_REWRITER_INVAR_IMP_INVAR;
          }
        }
      }
      return WFF_REWRITER_NONE;
      break;

    default:
      return WFF_REWRITER_NONE;
      break;
  }

  return WFF_REWRITER_NONE;
}

static void master_wff_rewriter_init_invariant_process_members(const SymbTable_ptr st,
                                                               const node_ptr spec,
                                                               const node_ptr context,
                                                               FlatHierarchy_ptr fh,
                                                               const boolean is_outer_invar)
{
  if (Wff_is_propositional(st, spec, context, true)) {

    if (is_outer_invar) {
      const Set_t premises_cone =
        Formula_GetDependenciesByType(st, spec, Nil, VFT_NEXT | VFT_INPUT,
                                      true);

      if (!Set_IsEmpty(premises_cone)) {
        /* The property contains input/next thus it must be added as TRANS */
        FlatHierarchy_add_trans(fh, spec);
      }
      else {
        /* The property does NOT contains input/next thus it must be
           added as an INVAR, this ensures it holds initially and
           within the transition relation. */
        FlatHierarchy_add_invar(fh, spec);
      }

      Set_ReleaseSet(premises_cone);
    } else
      FlatHierarchy_add_init(fh, spec);
    return;
  }

  switch (node_get_type(spec)) {
    case CONTEXT:
      error_unreachable_code_msg("Expression expected to be already flattened");
      return;

    case OP_GLOBAL:
      master_wff_rewriter_init_invariant_process_members(st, car(spec), context, fh, true);
      return;

    case AND:
      {
        const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(st));
        const NodeMgr_ptr nodemgr =
          NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
        master_wff_rewriter_init_invariant_process_members (st, car(spec), context, fh, is_outer_invar);
        master_wff_rewriter_init_invariant_process_members (st, cdr(spec), context, fh, is_outer_invar);
        return;
      }

    case IMPLIES:
      {
        master_wff_rewriter_init_invariant_process_members (st, car(spec), context, fh, is_outer_invar);
        master_wff_rewriter_init_invariant_process_members (st, cdr(spec), context, fh, is_outer_invar);
        return;
      }

    case OR:
      {
        master_wff_rewriter_init_invariant_process_members (st, car(spec), context, fh, is_outer_invar);
        master_wff_rewriter_init_invariant_process_members (st, cdr(spec), context, fh, is_outer_invar);
        return;
      }

    default:
      error_unreachable_code_msg("Unexpected expression structure");
      break;
  }

  error_unreachable_code_msg("Unexpected expression structure");
}
