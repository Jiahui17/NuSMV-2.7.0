/* ---------------------------------------------------------------------------

   This file is part of the ``automata'' package.
   %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'Automaton'

  \todo: Missing description

*/

#include "nusmv/core/automata/automataInt.h"
#include "nusmv/core/automata/Automaton_private.h"
#include "nusmv/core/automata/Automaton.h"
#include "nusmv/core/automata/AutomataAlgoChooser.h"
#include "nusmv/core/automata/AutomataNamedLoc.h"
#include "nusmv/core/automata/automata_label_creator.h"
#include "nusmv/core/automata/labels/AutomataLabelCombinator.h"
#include "nusmv/core/automata/utils/AutomataLocHash.h"
#include "nusmv/core/automata/utils/AutomataLocSetHash.h"
#include "nusmv/core/automata/utils/AutomataLocCoupleHash.h"
#include "nusmv/core/automata/utils/AutomataLocTriple.h"
#include "nusmv/core/automata/utils/AutomataPredecessorsList.h"
#include "nusmv/core/automata/utils/AutomataSimRel.h"

/* core */
#include "nusmv/core/utils/StreamMgr.h"
#include "nusmv/core/utils/utils.h"

#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/parser/psl/pslExpr.h"
#include "nusmv/core/parser/psl/psl_symbols.h"

#include "nusmv/core/wff/ExprMgr.h"
#include "nusmv/core/cinit/cinit.h"
#include "nusmv/core/node/NodeMgr.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'Automaton_private.h' for class 'Automaton' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*!
  \brief If 1 enables debug printing on errstream.

  
*/
#define AUTOMATON_DEBUG 0

/*!
  \brief If 1 performs reduction only after "costly" automata
   operations.

  Now costly automata operations are & and &&.
*/
#define LAZY_NFA_REDUCTION 1

/*!
  \brief If 1 uses kleene plus function that creates
   transitions starting from final locations.

  
*/
#define KLEENE_PLUS_FROM_END_STATE 1

/*!
  \brief Updates final representant

  
*/
#define AUTOMATON_UPDATE_FINAL_REP(rep, s1)                     \
  if (AUTOMATA_LOC(Nil) == rep)                                 \
    rep = s1;                                                   \
  else                                                          \
    if (AutomataLoc_get_id(rep) > AutomataLoc_get_id(s1))       \
      rep = s1

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void automaton_finalize(Object_ptr self, void* dummy);

static void automaton_set_location_final(Automaton_ptr self,
                                         AutomataLoc_ptr location,
                                         boolean is_final);

static int automaton_remove_loc(Automaton_ptr self,
                                AutomataLoc_ptr location);

static void
automaton_copy_reachable_from_loc(Automaton_ptr self,
                                  AutomataLocHash_ptr hash,
                                  Automaton_ptr A1,
                                  AutomataLoc_ptr start_location,
                                  ErrorMgr_ptr errmgr);

static Automaton_ptr
automaton_copy_accepting_loc(AutomataManager_ptr manager,
                             Automaton_ptr self,
                             boolean* reach_accepting);

static AutomataLoc_ptr
automaton_hash_find_or_create_loc(Automaton_ptr self,
                                  AutomataLocHash_ptr hash,
                                  Automaton_ptr source_aut,
                                  AutomataLoc_ptr key);

static void
automaton_copy_from_initial_loc(Automaton_ptr result_automaton,
                                Automaton_ptr A1,
                                AutomataLocHash_ptr hash,
                                boolean exclude_initial_locations,
                                ErrorMgr_ptr errmgr);

static void
automaton_alphabet_based_step_intersection(AutomataManager_ptr manager,
                                           Automaton_ptr res_automaton,
                                           Automaton_ptr A1,
                                           Automaton_ptr A2,
                                           AutomataLocCoupleHash_ptr location_map,
                                           Slist_ptr stack,
                                           AutomataLocTriple_ptr triple,
                                           Set_t support_set);

static void
automaton_transitions_based_step_intersection(AutomataManager_ptr manager,
                                              Automaton_ptr res_automaton,
                                              Automaton_ptr A1,
                                              Automaton_ptr A2,
                                              AutomataLocCoupleHash_ptr location_map,
                                              Slist_ptr stack,
                                              AutomataLocTriple_ptr triple);

static void
automaton_alphabet_based_step_fusion(AutomataManager_ptr manager,
                                     Automaton_ptr res_automaton,
                                     AutomataLocHash_ptr location_map,
                                     AutomataLoc_ptr res_location,
                                     AutomataTransition_ptr trans,
                                     Set_t trans_from_init_2_set,
                                     Set_t support_set);
static void
automaton_transitions_based_step_fusion(AutomataManager_ptr manager,
                                        Automaton_ptr res_automaton,
                                        AutomataLocHash_ptr location_map,
                                        AutomataLoc_ptr res_location,
                                        AutomataTransition_ptr trans,
                                        Slist_ptr trans_from_init_2);
static void
automaton_alphabet_based_step_determinization(AutomataManager_ptr manager,
                                              Automaton_ptr res_automaton,
                                              AutomataLocSetHash_ptr location_map,
                                              Slist_ptr stack,
                                              AutomataLoc_ptr res_location,
                                              AutomataLocSet_ptr location_set,
                                              Set_t location_set_support);
static void
automaton_transitions_based_step_determinization(AutomataManager_ptr manager,
                                                 Automaton_ptr res_automaton,
                                                 AutomataLocSetHash_ptr location_map,
                                                 Slist_ptr stack,
                                                 AutomataLoc_ptr res_location,
                                                 AutomataLocSet_ptr location_set);

static AutomataLoc_ptr
automaton_intersection_find_or_create_loc(Automaton_ptr res_automaton,
                                          AutomataLocCoupleHash_ptr hash,
                                          Slist_ptr stack,
                                          Automaton_ptr A1,
                                          Automaton_ptr A2,
                                          AutomataLoc_ptr t1_dest_location,
                                          AutomataLoc_ptr t2_dest_location);
static AutomataLoc_ptr
automaton_determinization_find_or_create_loc(NodeMgr_ptr nodemgr,
                                             Automaton_ptr res_automaton,
                                             AutomataLocSetHash_ptr hash,
                                             Slist_ptr stack,
                                             AutomataLocSet_ptr dest_location_set);

static AutomataTransition_ptr
automaton_copy_transition_to_loc(Automaton_ptr res_automaton,
                                 AutomataLoc_ptr res_location,
                                 Automaton_ptr A1,
                                 AutomataTransition_ptr a1_trans,
                                 AutomataLocHash_ptr hash,
                                 Slist_ptr stack,
                                 ErrorMgr_ptr errmgr);

static Automaton_ptr
automaton_propositional_kleene_star(AutomataManager_ptr manager,
                                    PslNode_ptr prop_expr);

static Automaton_ptr automaton_true_star(AutomataManager_ptr manager);

static void automaton_update_type(Automaton_ptr self,
                                  AutomataManager_ptr manager,
                                  AutomataLoc_ptr s);

static void automaton_update_complete(Automaton_ptr self,
                                      AutomataManager_ptr manager,
                                      AutomataLoc_ptr s);

static void automaton_update_type_and_complete(Automaton_ptr self,
                                               AutomataManager_ptr manager,
                                               AutomataLoc_ptr s);

static void
automaton_init_type_and_complete_from_aut(Automaton_ptr self,
                                          AutomataManager_ptr manager,
                                          Automaton_ptr A1,
                                          Automaton_ptr A2);

static AutomataLoc_ptr automaton_create_sink_loc(Automaton_ptr self,
                                                 AutomataManager_ptr manager);

static void automaton_add_sink_loc(Automaton_ptr self,
                                   AutomataLoc_ptr sink_location);

static PslNode_ptr
automaton_get_wrapped_sere_propositional(PslNode_ptr sere);

static boolean
automaton_is_wrapped_sere_propositional(PslNode_ptr sere);

static void
automaton_mark_predecessors(AutomataLoc_ptr location,
                            AutomataPredecessorsList_ptr predecessors_lists,
                            boolean* reach_accepting);

static void automaton_merge_transitions(Automaton_ptr automaton);

static Automaton_ptr
automaton_bisimulation_reduction(AutomataManager_ptr manager,
                                 Automaton_ptr self);

static AutomataSimRel_ptr
automaton_compute_simrel(AutomataManager_ptr manager,
                         Automaton_ptr self);

static boolean automaton_compute_simrel_lists(AutomataSimRel_ptr sim_rel,
                                              AutomataPredecessorsList_ptr A_pred,
                                              Slist_ptr locations_list);

static void automaton_compute_simrel_preprocess(Automaton_ptr self,
                                                AutomataSimRel_ptr sim_rel,
                                                AutomataPredecessorsList_ptr A_pred);

static Automaton_ptr
automaton_reduce_using_sim_rel(AutomataManager_ptr manager,
                               Automaton_ptr self,
                               AutomataSimRel_ptr sim_rel);

static unsigned int
automaton_start_tag(FILE* file,
                    char* tag_name,
                    boolean is_empty_tag,
                    unsigned int indent_level,
                    int num_attrs,
                    ...);

static unsigned int
automaton_end_tag(FILE* file,
                  char* tag_name,
                  unsigned int indent_level);

static enum st_retval print_all_trans(char* trans,
                                      char* id,
                                      char* trans2source);

static void print_indent(FILE* file, unsigned int indent_level);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

Automaton_ptr Automaton_create(void)
{
  Automaton_ptr self = ALLOC(Automaton, 1);
  AUTOMATON_CHECK_INSTANCE(self);

  automaton_init(self);
  return self;
}

void Automaton_destroy(Automaton_ptr self)
{
  AUTOMATON_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

AutomataPredecessorsList_ptr
Automaton_build_predecessors_list(Automaton_ptr self)
{
  AutomataPredecessorsList_ptr predecessors;
  Siter location_siter;

  AUTOMATON_CHECK_INSTANCE(self);

  predecessors = AutomataPredecessorsList_create(ABSTRACT_AUTOMATON(self)->location_count);

  SLIST_FOREACH(AbstractAutomaton_get_location_list(ABSTRACT_AUTOMATON(self)), location_siter) {
    Siter trans_siter;
    AutomataLoc_ptr location;

    location = AUTOMATA_LOC(Siter_element(location_siter));
    SLIST_FOREACH(AutomataLoc_get_transitions(location), trans_siter) {
      AutomataTransition_ptr trans;
      AutomataLoc_ptr end_location;

      trans = AUTOMATA_TRANSITION(Siter_element(trans_siter));
      end_location = AutomataTransition_get_end_location(trans);

      AutomataPredecessorsList_insert(predecessors, end_location, location);
    }
  }

  return predecessors;
}

VIRTUAL Automaton_ptr
Automaton_create_from_expr(AutomataManager_ptr manager,
                           Expr_ptr expr)
{
  AutomataLoc_ptr end_location;
  AutomataLoc_ptr sink_location;
  AutomataLabel_ptr label;
  Automaton_ptr self;
  BddEnc_ptr bdd_enc;
  NuSMVEnv_ptr env;
  ExprMgr_ptr exprs;
  ErrorMgr_ptr errmgr;

  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  EXPR_CHECK_INSTANCE(expr);

  bdd_enc = AutomataManager_get_bdd_enc(manager);
  env = EnvObject_get_environment(ENV_OBJECT(manager));
  exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));
  errmgr = ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  self = Automaton_create_empty(manager);

  label = automata_create_label_from_expr(ABSTRACT_AUTOMATON(self)->label_type, bdd_enc, expr);
  /* creates the automaton only if label is sat */
  if(AutomataLabel_is_sat(label)){
    /* creates the new end location, the label from expr and the new
       transition from initial_locations to end_location with the newly
       created label.
    */
    end_location = AutomataLoc_create_from_manager(manager);
    /* the end location is accepting */
    AutomataLoc_set_final(end_location, true);
    automaton_add_loc(ABSTRACT_AUTOMATON(self), end_location);

    Slist_push(self->final_locations, end_location);

    AutomataLoc_create_transition(Automaton_get_unique_init_loc(self),
                                  end_location,
                                  label, errmgr);

    if (AutomataManager_is_keep_complete(manager)) {
      /* Preserve completeness adding a sink location */
      sink_location = automaton_create_sink_loc(self, manager);
      automaton_add_sink_loc(self, sink_location);

      /* init_location to sink with !expr, if !expr is sat */
      label = AutomataLabel_complement(label);
      if(AutomataLabel_is_sat(label))
        AutomataLoc_create_transition(Automaton_get_unique_init_loc(self),
                                      sink_location,
                                      label,
                                      errmgr);

      /* final location to sink with true */
      label = automata_create_label_from_expr(ABSTRACT_AUTOMATON(self)->label_type, bdd_enc, ExprMgr_true(exprs));
      AutomataLoc_create_transition(end_location, sink_location,
                                    label, errmgr);

      /* resulting locations are all complete! */
      AutomataLoc_set_is_complete(Automaton_get_unique_init_loc(self), true);
      AutomataLoc_set_is_complete(ABSTRACT_AUTOMATON(self)->sink_location, true);
      AutomataLoc_set_is_complete(end_location, true);

      /* resulting locations are deterministic */
      AutomataLoc_set_type(Automaton_get_unique_init_loc(self), DETERMINISTIC);
      AutomataLoc_set_type(ABSTRACT_AUTOMATON(self)->sink_location, DETERMINISTIC);
      AutomataLoc_set_type(end_location, DETERMINISTIC);

      /* resutling automaton is complete */
      self->is_complete = true;
    }

    /* resutling automaton is deterministic */
    ABSTRACT_AUTOMATON(self)->type = DETERMINISTIC;
  } else {
    /* label is not sat, it could be destroyed */
    AutomataLabel_destroy(label);
  }

  return self;
}

VIRTUAL Automaton_ptr
Automaton_create_empty(AutomataManager_ptr manager)
{
  AutomataLoc_ptr initial_loc;
  Automaton_ptr self = Automaton_create();

  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  /* Sets the type of the labels */
  ABSTRACT_AUTOMATON(self)->label_type = AutomataManager_get_label_type(manager);
  self->keep_support_set_in_location =
    AutomataManager_is_keep_support_set_in_location(manager);
  /* Creates the only location of automaton and set it as initial location */
  initial_loc = ABSTRACT_AUTOMATON(self)->create_location_from_manager(ABSTRACT_AUTOMATON(self), manager);
  automaton_add_loc(ABSTRACT_AUTOMATON(self), initial_loc);
  ABSTRACT_AUTOMATON(self)->initial_locations = Set_AddMember(ABSTRACT_AUTOMATON(self)->initial_locations,
                                                              (Set_Element_t)initial_loc);

  /* updates the type of automaton */
  automaton_init_type_complete(self, manager);

  return self;
}

VIRTUAL Automaton_ptr
Automaton_create_empty_word(AutomataManager_ptr manager)
{
  Automaton_ptr self;

  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  /* creates the automaton with only one location and makes that location
     final. */
  self = Automaton_create_empty(manager);
  Slist_push(self->final_locations, Automaton_get_unique_init_loc(self));

  return self;
}

VIRTUAL Automaton_ptr Automaton_union(AutomataManager_ptr manager,
                                      Automaton_ptr A1,
                                      Automaton_ptr A2)
{
  AUTOMATON_CHECK_INSTANCE(A1);
  AUTOMATON_CHECK_INSTANCE(A2);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return A1->automaton_union(manager, A1, A2);
}

VIRTUAL Automaton_ptr Automaton_intersection(AutomataManager_ptr manager,
                                             Automaton_ptr A1,
                                             Automaton_ptr A2)
{
  AUTOMATON_CHECK_INSTANCE(A1);
  AUTOMATON_CHECK_INSTANCE(A2);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return A1->intersection(manager, A1, A2);
}

VIRTUAL Automaton_ptr Automaton_and(AutomataManager_ptr manager,
                                    Automaton_ptr A1,
                                    Automaton_ptr A2)
{
  AUTOMATON_CHECK_INSTANCE(A1);
  AUTOMATON_CHECK_INSTANCE(A2);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return A1->and(manager, A1, A2);
}

VIRTUAL Automaton_ptr Automaton_concatenation(AutomataManager_ptr manager,
                                              Automaton_ptr A1,
                                              Automaton_ptr A2)
{
  AUTOMATON_CHECK_INSTANCE(A1);
  AUTOMATON_CHECK_INSTANCE(A2);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return A1->concatenation(manager, A1, A2);
}

VIRTUAL Automaton_ptr Automaton_concatenation_n_times(AutomataManager_ptr manager,
                                                      Automaton_ptr self,
                                                      int n)
{
  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return self->concatenation_n_times(manager, self, n);
}

VIRTUAL Automaton_ptr Automaton_kleene_star(AutomataManager_ptr manager,
                                            Automaton_ptr self)
{
  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return self->kleene_star(manager, self);
}

VIRTUAL Automaton_ptr Automaton_kleene_plus(AutomataManager_ptr manager,
                                            Automaton_ptr self)
{
  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return self->kleene_plus(manager, self);
}

VIRTUAL Automaton_ptr Automaton_fusion(AutomataManager_ptr manager,
                                       Automaton_ptr A1,
                                       Automaton_ptr A2)
{
  AUTOMATON_CHECK_INSTANCE(A1);
  AUTOMATON_CHECK_INSTANCE(A2);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return A1->fusion(manager, A1, A2);
}

VIRTUAL Automaton_ptr Automaton_reverse(AutomataManager_ptr manager,
                                        Automaton_ptr self)
{
  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return self->reverse(manager, self);
}

VIRTUAL Automaton_ptr Automaton_complement(AutomataManager_ptr manager,
                                           Automaton_ptr self)
{
  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return self->complement(manager, self);
}

VIRTUAL Automaton_ptr Automaton_difference(AutomataManager_ptr manager,
                                           Automaton_ptr A1,
                                           Automaton_ptr A2)
{
  AUTOMATON_CHECK_INSTANCE(A1);
  AUTOMATON_CHECK_INSTANCE(A2);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return A1->difference(manager, A1, A2);
}

VIRTUAL boolean Automaton_containment(AutomataManager_ptr manager,
                                      Automaton_ptr A1,
                                      Automaton_ptr A2)
{
  AUTOMATON_CHECK_INSTANCE(A1);
  AUTOMATON_CHECK_INSTANCE(A2);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return A1->containment(manager, A1, A2);
}

VIRTUAL Automaton_ptr Automaton_determinize(AutomataManager_ptr manager,
                                            Automaton_ptr self)
{
  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return self->determinize(manager, self);
}

VIRTUAL Automaton_ptr Automaton_minimize(AutomataManager_ptr manager,
                                         Automaton_ptr self)
{
  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return self->minimize(manager, self);
}

VIRTUAL Automaton_ptr Automaton_nfa_reduction(AutomataManager_ptr manager,
                                              Automaton_ptr self)
{
  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return self->nfa_reduction(manager, self);
}

VIRTUAL boolean Automaton_equals(AutomataManager_ptr manager,
                                 Automaton_ptr A1,
                                 Automaton_ptr A2)
{
  AUTOMATON_CHECK_INSTANCE(A1);
  AUTOMATON_CHECK_INSTANCE(A2);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return A1->equals(manager, A1, A2);
}

VIRTUAL boolean Automaton_emptyness_check(AutomataManager_ptr manager,
                                          Automaton_ptr self)
{
  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return self->emptyness_check(manager, self);
}

VIRTUAL Automaton_ptr Automaton_prune(AutomataManager_ptr manager,
                                      Automaton_ptr self)
{
  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return self->prune(manager, self);
}

VIRTUAL Automaton_ptr
Automaton_prune_non_accepting_paths(AutomataManager_ptr manager,
                                    Automaton_ptr self)
{
  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return self->prune_non_accepting(manager, self);
}

VIRTUAL Automaton_ptr Automaton_create_from_sere(AutomataManager_ptr manager,
                                                 PslNode_ptr sere)
{
  Automaton_ptr res_automaton;
  Automaton_ptr A1;
  Automaton_ptr A2;
  PslOp psl_op;
  PslNode_ptr inner_sere;
  PslOp inner_psl_op;
  boolean need_minimization;
  NuSMVEnv_ptr env;
  ExprMgr_ptr exprs;
  ErrorMgr_ptr errmgr;

  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  nusmv_assert(sere != NULL);

  env = EnvObject_get_environment(ENV_OBJECT(manager));
  exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));
  errmgr = ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  psl_op = psl_node_get_op(sere);

  /* this flag is set to true after each operation when needed,
     avoiding unuseful minimization as in the case of propositional
     expressions or PSL_SERE operator */
  need_minimization = false;

  /* True and false expressions */
  if ((TRUEEXP == psl_op) || (FALSEEXP == psl_op)) {
    if (TRUEEXP == psl_op) {
      res_automaton = Automaton_create_from_expr(manager,
                                                 ExprMgr_true(exprs));
    } else {
      /* FALSEEXP */
      res_automaton = Automaton_create_empty(manager);
    }

    return res_automaton;
  }

  /* identifier */
  if (psl_node_is_id(sere)) {
    /* propositional atoms of SERE are treated as labels of automata */
    res_automaton = Automaton_create_from_expr(manager,
                                               sere);

    return res_automaton;
  }

  /* if we have a propositional SERE, i.e. a {prop_expr}, we build the
     singleton NFA for it */
  if (PslNode_is_propositional(sere)) {
    res_automaton = Automaton_create_from_expr(manager, sere);
  }
  else if ((psl_op == PSL_SERE) &&
           PslNode_is_propositional(psl_node_get_left(sere))) {
    res_automaton = Automaton_create_from_expr(manager,
                                               psl_node_get_left(sere));
  }
  else {
    /* Recursively convert the SERE */
    switch(psl_op) {
    case PSL_SERECONCAT:
      A1 = Automaton_create_from_sere(manager, psl_node_get_left(sere));
      A2 = Automaton_create_from_sere(manager, psl_node_get_right(sere));
      res_automaton =  Automaton_concatenation(manager, A1, A2);

      Automaton_destroy(A1);
      Automaton_destroy(A2);

#if ! LAZY_NFA_REDUCTION
      need_minimization = true;
#endif
      break;
    case PSL_SEREFUSION:
      A1 = Automaton_create_from_sere(manager, psl_node_get_left(sere));
      A2 = Automaton_create_from_sere(manager, psl_node_get_right(sere));
      res_automaton =  Automaton_fusion(manager, A1, A2);

      Automaton_destroy(A1);
      Automaton_destroy(A2);

#if ! LAZY_NFA_REDUCTION
      need_minimization = true;
#endif
      break;
    case PSL_SERECOMPOUND:
      /*
        TKSERECOMPOUND
        /    \
        op      Nil
        /  \
        seq1   seq2

        gets rid of TKSERECOMPOUND node and goes directly to the real
        SERE
      */

      inner_sere = psl_node_get_left(sere);
      inner_psl_op = psl_node_get_op(inner_sere);

      A1 = Automaton_create_from_sere(manager, psl_node_get_left(inner_sere));
      A2 = Automaton_create_from_sere(manager, psl_node_get_right(inner_sere));

      switch(inner_psl_op) {
      case OR:
        res_automaton = Automaton_union(manager, A1, A2);

        need_minimization = true;
        break;
      case AND:
        res_automaton = Automaton_and(manager, A1, A2);

        need_minimization = true;
        break;
      case PSL_AMPERSANDAMPERSAND:
        res_automaton = Automaton_intersection(manager, A1, A2);

        need_minimization = true;
        break;
      default:
        /* Generates an error and quit NuSMV */
        automata_error_unknown_sere_operator(errmgr, psl_op);
      } /* end of internal switch in compound */

      Automaton_destroy(A1);
      Automaton_destroy(A2);

      break;
    case PSL_SEREREPEATED:
      {
        PslNode_ptr r;

        /* Handle expression like r[*] abd r[+] */
        inner_sere = psl_node_get_left(sere);

        inner_psl_op = psl_node_sere_repeated_get_op(sere);
        r = psl_node_sere_repeated_get_expr(sere);

        switch(inner_psl_op) {
        case PSL_LBSPLAT:
          if (psl_node_sere_is_star_count(inner_sere)) {
            /* Generates an error and quit NuSMV

               Count is still not managed by the library.
               This is because the expression here should habe already
               been processed, removing count expressions.
            */
            automata_error_unknown_sere_operator(errmgr, psl_op);

          }else{
            /* [*] */
            if (psl_node_sere_is_standalone_star(sere)) {
              /* treats [*] as TRUE[*] */
              res_automaton = automaton_true_star(manager);
            } else {
              /* in the case of r[*], builds the automaton for r */
              /* r is the body of the repeated expression */
              r = psl_node_sere_repeated_get_expr(sere);

              /* check if the left node of inner sere is propositional */
              if(automaton_is_wrapped_sere_propositional(r)) {
                /* directly build prop[*] */
                PslNode_ptr prop_expr;
                prop_expr = automaton_get_wrapped_sere_propositional(r);

                res_automaton =
                  automaton_propositional_kleene_star(manager, prop_expr);

              }else{
                A1 = Automaton_create_from_sere(manager, r);
                res_automaton = Automaton_kleene_star(manager, A1);
                Automaton_destroy(A1);
#if ! LAZY_NFA_REDUCTION
                need_minimization = true;
#endif
              }
            }
          }

          break;
        case PSL_LBPLUSRB:
          /* [+] */
          if (psl_node_sere_is_standalone_plus(sere)) {
            /* treats [+] as TRUE[+] */
            A1 = Automaton_create_from_expr(manager, ExprMgr_true(exprs));
            res_automaton = Automaton_kleene_plus(manager, A1);
            Automaton_destroy(A1);
          }else{
            /* if (psl_node_sere_is_standalone_plus(inner_sere)) { */
            /*   A1 = Automaton_create_from_expr(manager, psl_node_make_true(nodemgr)); */
            /* }else{ */
            /* PslNode_ptr r = psl_node_get_left(psl_node_get_left(inner_sere)); */
            A1 = Automaton_create_from_sere(manager, r);
            /* } */

            res_automaton = Automaton_kleene_plus(manager, A1);
            Automaton_destroy(A1);
#if ! LAZY_NFA_REDUCTION
            need_minimization = true;
#endif
          }
          break;
        }
      } /* end of internal switch for PSL_SEREREPEATED */
      break;
    case NOT:
      /* This is a case that can arise during the test of the
         library. In the test files a SERE can be negated. */
      inner_sere = psl_node_get_left(sere);
      A1 = Automaton_create_from_sere(manager, inner_sere);
      res_automaton = Automaton_complement(manager, A1);
      Automaton_destroy(A1);
      break;
    case PSL_SERE:
      /* since a SERE can be bracketed by an arbitrary number of {}, we
         need to get rid of extra {} by getting read of wrapping TKSERE
         nodes (each TKSERE models a {}) */
      inner_sere = psl_node_get_left(sere);
      res_automaton =  Automaton_create_from_sere(manager, inner_sere);
      break;
    default:
      /* Generates an error and quit NuSMV */
      automata_error_unknown_sere_operator(errmgr, psl_op);
    }
  }

  if (need_minimization &&
      AutomataManager_is_minimize_automaton(manager)) {
    /* minimize the automaton */
    Automaton_ptr temp_auto = res_automaton;
    res_automaton = Automaton_nfa_reduction(manager, res_automaton);
    Automaton_destroy(temp_auto);
  }

  return res_automaton;
}

VIRTUAL PslNode_ptr Automaton_automa2sere(AutomataManager_ptr manager,
                                          Automaton_ptr self)
{
  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return self->automa2sere(manager, self);
}

VIRTUAL int Automaton_automaton2smv(AutomataManager_ptr manager,
                                    Automaton_ptr self,
                                    FILE* out_file)
{
  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  return self->automaton2smv(manager, self, out_file);
}

void Automaton_print_xmi(const Automaton_ptr self,
                         const NuSMVEnv_ptr env,
                         FILE* out_file,
                         AutomataXmiFormat format)
{
  AutomataPredecessorsList_ptr predecessors;
  hash_ptr trans2source;
  Siter siter_locations;
  char* type;
  char* trans_str;
  char* location_str;
  unsigned int trans_id;
  unsigned int trans_counter = 0;
  int location_id;
  AutomataLoc_ptr location_predecessor;
  int buffer_size;
  char* outgoing_buffer;
  char* incoming_buffer;
  AutomataLoc_ptr location;
  char* name;
  AutomataTransition_ptr trans;
  AutomataLoc_ptr trans_end_location;
  hash_ptr trans_ptr2id_hash;
  void* data[4];
  unsigned int indent_level;
  boolean is_enterprise_format = false;

  AUTOMATON_CHECK_INSTANCE(self);
  nusmv_assert(NULL != out_file);

  is_enterprise_format = (AUTOMATA_XMI_FORMAT_EA == format);
  buffer_size = 0;
  indent_level = 0;
  trans_str = ALLOC(char, ID_SIZE);
  location_str = ALLOC(char, ID_SIZE);
  trans2source = new_assoc();
  trans_ptr2id_hash = new_assoc();

  /* Here we use fprinf because <?xml has to end with ?> */
  fprintf(out_file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");

  if (is_enterprise_format) {
    indent_level =
      automaton_start_tag(out_file, "xmi:XMI", START_TAG,
                          indent_level, 3,
                          "xmi:version", "2.1",
                          "xmlns:uml", "http://schema.omg.org/spec/UML/2.1",
                          "xmlns:xmi", "http://schema.omg.org/spec/XMI/2.1");
  }
  else {
    indent_level =
      automaton_start_tag(out_file, "xmi:XMI", START_TAG,
                          indent_level, 3,
                          "xmi:version", "2.1",
                          "xmlns:uml", "http://www.eclipse.org/uml2/2.1/UML",
                          "xmlns:xmi", "http://www.w3.org/2001/XMLSchema-instance",
                          "xmlns:ecore", "http://www.eclipse.org/emf/2002/Ecore");
  }

  if (is_enterprise_format) {
    indent_level =
      automaton_start_tag(out_file, "xmi:Documentation", EMPTY_TAG,
                          indent_level, 2,
                          "exporter", "Enterprise Architect",
                          "exporterVersion", "6.5");
  }

  indent_level =
    automaton_start_tag(out_file, "uml:Model", START_TAG,
                        indent_level, 3,
                        "xmi:type", "uml:Model",
                        "xmi:id", "model",
                        "name", "_Modello");

  if (is_enterprise_format) {
    indent_level =
      automaton_start_tag(out_file, "uml:Package", START_TAG,
                          indent_level, 3,
                          "xmi:type", "uml:Package",
                          "xmi:id", "pack",
                          "name", "_Package");
  }

  indent_level =
    automaton_start_tag(out_file, "uml:StateMachine", START_TAG,
                        indent_level, 3,
                        "xmi:type", "uml:StateMachine",
                        "xmi:id", "fsm",
                        "name", "_StateMachine");


  /* Compute preliminary information */
  predecessors = Automaton_build_predecessors_list(self);

  Automaton_compute_all_trans_source_location(self, trans2source);

  /* Print the locations */
  SLIST_FOREACH(AbstractAutomaton_get_location_list(ABSTRACT_AUTOMATON(self)), siter_locations) {
    location = Siter_element(siter_locations);

    /* compute name */
    name = AutomataNamedLoc_get_name(AUTOMATA_NAMED_LOC(location));

    /* compute type */
    if (0 == strcmp(name, "Initial")) type = "uml:Pseudolocation";
    else type = "uml:State";

    /* compute location_str */
    location_id = AutomataLoc_get_id(location);

    {
      int c = snprintf(location_str, ID_SIZE, "_s%d", location_id);
      SNPRINTF_CHECK(c, ID_SIZE);
    }

    /* compute outgoing_buffer */
    {
      Siter siter;
      outgoing_buffer = (char*)NULL;
      buffer_size = 0;

      SLIST_FOREACH(AutomataLoc_get_transitions(location), siter) {
        trans = Siter_element(siter);

        trans_id = NODE_TO_INT(find_assoc(trans_ptr2id_hash, NODE_PTR(trans)));

        if (0 == trans_id) {
          trans_id = ++trans_counter;
          insert_assoc(trans_ptr2id_hash, NODE_PTR(trans), NODE_FROM_INT(trans_id));
        }

        {
          int c = snprintf(trans_str, ID_SIZE, "_t%d", trans_id);
          SNPRINTF_CHECK(c, ID_SIZE);
        }

        buffer_size = buffer_size + strlen(trans_str) + 1;
        outgoing_buffer = REALLOC(char, outgoing_buffer, buffer_size);

        if ((size_t)buffer_size == strlen(trans_str) + 1) { /* first time in loop */
          strncpy(outgoing_buffer, trans_str, strlen(trans_str) + 1);
        }
        else {
          char* tmp = ALLOC(char, buffer_size);
          int c = snprintf(tmp, buffer_size, "%s %s", outgoing_buffer,
                           trans_str);
          SNPRINTF_CHECK(c, buffer_size);
          strcpy(outgoing_buffer, tmp);
          FREE(tmp);
        }
      }
    }

    /* compute incoming_buffer */
    {
      Siter siter;
      incoming_buffer = (char*)NULL;
      buffer_size = 0;

      SLIST_FOREACH(AutomataPredecessorsList_get_predecessor(predecessors,
                                                             location),
                    siter) {
        location_predecessor = Siter_element(siter);

        {
          /* [MD] this declaration was shadowing a local variable */
          Siter siter_shadowing;
          Slist_ptr list;

          list = AutomataLoc_get_transitions(location_predecessor);

          for (siter_shadowing = Slist_first(list);
               ! Siter_is_end(siter_shadowing);
               siter_shadowing = Siter_next(siter_shadowing)) {
            trans = Siter_element(siter_shadowing);
            trans_end_location = AutomataTransition_get_end_location(trans);

            if (trans_end_location == location) {
              trans_id = NODE_TO_INT(find_assoc(trans_ptr2id_hash, NODE_PTR(trans)));

              if (0 == trans_id) {
                trans_id = ++trans_counter;
                insert_assoc(trans_ptr2id_hash, NODE_PTR(trans), NODE_FROM_INT(trans_id));
              }

              {
                int c = snprintf(trans_str, ID_SIZE, "_t%d", trans_id);
                SNPRINTF_CHECK(c, ID_SIZE);
              }

              buffer_size = buffer_size + strlen(trans_str) + 1;
              incoming_buffer = REALLOC(char, incoming_buffer, buffer_size);

              /* first time in loop */
              if ((size_t)buffer_size == strlen(trans_str) + 1) {
                strncpy(incoming_buffer, trans_str, strlen(trans_str) + 1);
              }
              else {
                char* tmp = ALLOC(char, buffer_size);
                int c = snprintf(tmp, buffer_size, "%s %s",
                                 incoming_buffer, trans_str);
                SNPRINTF_CHECK(c, buffer_size);
                strcpy(incoming_buffer, tmp);
                FREE(tmp);
              }

              break;
            }
          }
          nusmv_assert(! Siter_is_end(siter_shadowing));
        } /* end of inner foreach */
      } /* end of external foreach */

      if (0 == strcmp(name, "Initial")) {
        indent_level =
          automaton_start_tag(out_file, "subvertex", EMPTY_TAG,
                              indent_level, 6,
                              "xmi:type", type,
                              "xmi:id", location_str,
                              "name", name,
                              "kind", "initial",
                              "outgoing", outgoing_buffer,
                              "incoming", incoming_buffer);
      }
      else {
        indent_level =
          automaton_start_tag(out_file, "subvertex", EMPTY_TAG,
                              indent_level, 5,
                              "xmi:type", type,
                              "xmi:id", location_str,
                              "name", name,
                              "outgoing", outgoing_buffer,
                              "incoming", incoming_buffer);
      }
    }
    FREE(outgoing_buffer);
    FREE(incoming_buffer);
  }


  /* Print the transitions */
  data[0] = env;
  data[1] = out_file;
  data[2] = trans2source;
  data[3] = PTR_FROM_INT(void*, indent_level);

  assoc_foreach(trans_ptr2id_hash, print_all_trans, (char*)data);

  /* close the tags */
  indent_level =
    automaton_end_tag(out_file, "uml:StateMachine", indent_level);

  if (is_enterprise_format) {
    indent_level =
      automaton_end_tag(out_file, "uml:Package", indent_level);
  }

  indent_level =
    automaton_end_tag(out_file, "uml:Model", indent_level);

  if (is_enterprise_format) {
    int i;

    indent_level =
      automaton_start_tag(out_file, "xmi:Extension", START_TAG,
                          indent_level, 2,
                          "extender", "Enterprise Architect",
                          "extenderID", "6.5");

    indent_level =
      automaton_start_tag(out_file, "diagrams", START_TAG,
                          indent_level, 0);

    indent_level =
      automaton_start_tag(out_file, "diagram", START_TAG,
                          indent_level, 1,
                          "xmi:id", "diagram");

    indent_level =
      automaton_start_tag(out_file, "model", EMPTY_TAG,
                          indent_level, 2,
                          "package", "pack",
                          "owner", "pack");

    for (i = 1; i <= AbstractAutomaton_get_location_count(ABSTRACT_AUTOMATON(self)); i++) {
      char str[ID_SIZE];
      int c;

      c = snprintf(str, ID_SIZE, "_s%d", i);
      SNPRINTF_CHECK(c, ID_SIZE);
      indent_level =
        automaton_start_tag(out_file, "element", EMPTY_TAG,
                            indent_level, 1,
                            "subject", str);
    }

    indent_level =
      automaton_end_tag(out_file, "diagram", indent_level);
    indent_level =
      automaton_end_tag(out_file, "diagrams", indent_level);
    indent_level =
      automaton_end_tag(out_file, "xmi:Extension", indent_level);
  }

  indent_level =
    automaton_end_tag(out_file, "xmi:XMI", indent_level);
  fprintf(out_file, "\n");

  /* deinit */
  AutomataPredecessorsList_destroy(predecessors);
  FREE(location_str);
  FREE(trans_str);
  free_assoc(trans2source);
  free_assoc(trans_ptr2id_hash);
}

void
Automaton_compute_all_trans_source_location(Automaton_ptr self,
                                            hash_ptr trans2source)
{
  Siter location_siter;

  SLIST_FOREACH(AbstractAutomaton_get_location_list(ABSTRACT_AUTOMATON(self)), location_siter) {
    Siter trans_siter;
    AutomataLoc_ptr location;

    location = AUTOMATA_LOC(Siter_element(location_siter));
    SLIST_FOREACH(AutomataLoc_get_transitions(location), trans_siter) {
      AutomataTransition_ptr trans;

      trans = AUTOMATA_TRANSITION(Siter_element(trans_siter));

      insert_assoc(trans2source, NODE_PTR(trans), NODE_PTR(location));
    }
  }
}

/*!
  \brief Returns the initial location

  The set of initial locations must be a singleton
*/

AutomataLoc_ptr Automaton_get_unique_init_loc(Automaton_ptr const self)
{
  Set_t initial_locations;
  Set_Iterator_t iter;
  AutomataLoc_ptr result;

  AUTOMATON_CHECK_INSTANCE(self);

  initial_locations = automaton_get_initial_locations(ABSTRACT_AUTOMATON(self));

  iter = Set_GetFirstIter(initial_locations);
  result = AUTOMATA_LOC(Set_GetMember(initial_locations, iter));

  nusmv_assert(AUTOMATA_LOC(NULL) != result);

  return result;
}

VIRTUAL void Automaton_new_transition(AutomataManager_ptr manager,
                                      Automaton_ptr self,
                                      int location_1_id,
                                      int location_2_id,
                                      Expr_ptr label_expr)
{
  AUTOMATON_CHECK_INSTANCE(self);

  self->new_transition(manager, self, location_1_id, location_2_id, label_expr);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void automaton_init(Automaton_ptr self)
{
  /* base class initialization */
  abstract_automaton_init(ABSTRACT_AUTOMATON(self));

  /* members initialization */
  self->final_locations = Slist_create();
  self->is_complete = false;
  self->keep_support_set_in_location = false;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = automaton_finalize;

  OVERRIDE(AbstractAutomaton, print) = automaton_print;
  OVERRIDE(AbstractAutomaton, print_dot) = automaton_print_dot;
  OVERRIDE(AbstractAutomaton, add_location) = automaton_add_loc;
  OVERRIDE(AbstractAutomaton, get_initial_locations) =
    automaton_get_initial_locations;
  OVERRIDE(AbstractAutomaton, add_initial_location) =
    automaton_add_initial_location;

  OVERRIDE(Automaton, new_transition) = automaton_new_transition;
  OVERRIDE(Automaton, automaton_union) = automaton_union;
  OVERRIDE(Automaton, intersection) = automaton_intersection;
  OVERRIDE(Automaton, and) = automaton_and;
  OVERRIDE(Automaton, concatenation) = automaton_concatenation;
  OVERRIDE(Automaton, concatenation_n_times) = automaton_concatenation_n_times;
  OVERRIDE(Automaton, kleene_star) = automaton_kleene_star;
  OVERRIDE(Automaton, kleene_plus) = automaton_kleene_plus;
  OVERRIDE(Automaton, fusion) = automaton_fusion;
  OVERRIDE(Automaton, reverse) = automaton_reverse;
  OVERRIDE(Automaton, complement) = automaton_complement;
  OVERRIDE(Automaton, difference) = automaton_difference;
  OVERRIDE(Automaton, containment) = automaton_containment;
  OVERRIDE(Automaton, determinize) = automaton_determinize;
  OVERRIDE(Automaton, minimize) = automaton_minimize;
  OVERRIDE(Automaton, nfa_reduction) = automaton_nfa_reduction;
  OVERRIDE(Automaton, equals) = automaton_equals;
  OVERRIDE(Automaton, emptyness_check) = automaton_emptyness_check;
  OVERRIDE(Automaton, prune) = automaton_prune;
  OVERRIDE(Automaton, prune_non_accepting) =
    automaton_prune_non_accepting_paths;
  OVERRIDE(Automaton, automa2sere) = automaton_automa2sere;
  OVERRIDE(Automaton, automaton2smv) = automaton_automaton2smv;
}

void automaton_deinit(Automaton_ptr self)
{
  /* members deinitialization */
  Slist_destroy(self->final_locations);

  /* base class deinitialization */
  abstract_automaton_deinit(ABSTRACT_AUTOMATON(self));
}

Set_t automaton_get_initial_locations(AbstractAutomaton_ptr self)
{
  nusmv_assert(Set_IsSingleton(self->initial_locations));

  return self->initial_locations;
}

Automaton_ptr automaton_union(AutomataManager_ptr manager,
                              Automaton_ptr A1,
                              Automaton_ptr A2)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  Automaton_ptr result_automaton = Automaton_create_empty(manager);
  AutomataLocHash_ptr hash;

  /* If at least one of initial locations are final, then also
     Automaton_get_unique_init_loc(res_automaton) has to be final */
  if (AutomataLoc_is_final(Automaton_get_unique_init_loc(A1)) ||
      AutomataLoc_is_final(Automaton_get_unique_init_loc(A2)))
    automaton_set_location_final(result_automaton,
                                 Automaton_get_unique_init_loc(result_automaton),
                                 true);

  automaton_init_type_and_complete_from_aut(result_automaton,
                                            manager,
                                            A1,
                                            A2);

  /* Map from location to location */
  hash = AutomataLocHash_create();

  /* Copy all locations reachable from initial location of A1 */
  automaton_copy_from_initial_loc(result_automaton, A1, hash, false, errmgr);

  /* Copy all locations reachable from initial location of A2 */
  automaton_copy_from_initial_loc(result_automaton, A2, hash, false, errmgr);

  /* Manage the deterministic flag */
  automaton_update_type_and_complete(result_automaton,
                                     manager,
                                     Automaton_get_unique_init_loc(result_automaton));

  return result_automaton;
}

Automaton_ptr automaton_intersection(AutomataManager_ptr manager,
                                     Automaton_ptr A1,
                                     Automaton_ptr A2)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  Automaton_ptr app_automaton;
  Automaton_ptr res_automaton = Automaton_create_empty(manager);
  Slist_ptr stack;
  AutomataLocCoupleHash_ptr location_map;
  AutomataLocTriple_ptr triple;

  automaton_init_type_and_complete_from_aut(res_automaton,
                                            manager,
                                            A1,
                                            A2);

  /* If both initial locations are final, then also
     Automaton_get_unique_init_loc(res_automaton) has to be final */
  if (AutomataLoc_is_final(Automaton_get_unique_init_loc(A1)) &&
      AutomataLoc_is_final(Automaton_get_unique_init_loc(A2)))
    automaton_set_location_final(res_automaton,
                                 Automaton_get_unique_init_loc(res_automaton),
                                 true);

  /* stack that contains triple of locations */
  stack = Slist_create();

  /* Map from couples of locations to a single location */
  location_map = AutomataLocCoupleHash_create(env);

  /* Adds the mapping from the couple of initial locations of A1 and A2
     to the initial location of the res_automaton */
  AutomataLocCoupleHash_insert(location_map,
                               Automaton_get_unique_init_loc(A1),
                               Automaton_get_unique_init_loc(A2),
                               Automaton_get_unique_init_loc(res_automaton));

  /* Puts the triple of locations on top of stack */
  triple =
    AutomataLocTriple_create_from_locations(Automaton_get_unique_init_loc(A1),
                                            Automaton_get_unique_init_loc(A2),
                                            Automaton_get_unique_init_loc(res_automaton));
  Slist_push(stack, triple);

  while(!Slist_is_empty(stack)) {
    AutomataLoc_ptr result_location;
    AutomataLoc_ptr s1;
    AutomataLoc_ptr s2;
    AutomataAlgoChooser_ptr algo_chooser;

    triple = AUTOMATA_LOC_TRIPLE(Slist_pop(stack));
    s1 = AutomataLocTriple_get_first(triple);
    s2 = AutomataLocTriple_get_second(triple);
    result_location = AutomataLocTriple_get_third(triple);

    /* If s1 and s2 have out transitions combine them together */
    if(AutomataLoc_get_transitions_count(s1)>0 &&
       AutomataLoc_get_transitions_count(s2)>0) {
      Automaton_Algorithm_Type method_type;

      algo_chooser = AutomataAlgoChooser_create();
      method_type =
        AutomataAlgoChooser_choose_from_location_couple(algo_chooser,
                                                        manager,
                                                        s1,
                                                        s2);

      switch(method_type) {
      case ALPHABET_BASED:
        {
          Set_t support_set;

          support_set = AutomataAlgoChooser_get_last_support(algo_chooser);

          /* Possible optimization: compute type and is complete flag
             of result_location inside the alphabet step. This can be
             done in the loop of the alphabet, checking if every
             symbol is used (for completness) and if the same symbol
             is used for more than one transition (type).
          */
          automaton_alphabet_based_step_intersection(manager,
                                                     res_automaton,
                                                     A1,
                                                     A2,
                                                     location_map,
                                                     stack,
                                                     triple,
                                                     support_set);
        }

        break;
      case TRANSITIONS_BASED:
        automaton_transitions_based_step_intersection(manager,
                                                      res_automaton,
                                                      A1,
                                                      A2,
                                                      location_map,
                                                      stack,
                                                      triple);

        break;
      default:
        /* Generates an error */
        automata_error_unknown_algorithm_type(errmgr, method_type);
      }
    }

    /* Manage the deterministic flag */
    automaton_update_type_and_complete(res_automaton,
                                       manager,
                                       result_location);

    AutomataLocTriple_destroy(triple);
  }

  AutomataLocCoupleHash_destroy(location_map);
  Slist_destroy(stack);

  /* Prune unreachable locations to keep result automaton smaller */
  app_automaton = res_automaton;
  res_automaton = Automaton_prune_non_accepting_paths(manager, res_automaton);
  Automaton_destroy(app_automaton);

  return res_automaton;
}

Automaton_ptr automaton_and(AutomataManager_ptr manager,
                            Automaton_ptr A1,
                            Automaton_ptr A2)
{
  Automaton_ptr res_automaton, true_star, and1, and2, r1, r2;

  true_star = automaton_true_star(manager);

  /* A1; TRUE[*]*/
  r1 = Automaton_concatenation(manager, A1, true_star);
  /* {A1; TRUE[*]} && A2 */
  and1 = Automaton_intersection(manager, r1, A2);

  Automaton_destroy(r1);

  /* A2; TRUE[*] */
  r2 = Automaton_concatenation(manager, A2, true_star);
  /* A1 && {A2; TRUE[*]} */
  and2 = Automaton_intersection(manager, A1, r2);

  Automaton_destroy(r2);
  Automaton_destroy(true_star);

  /* {{A1; TRUE[*]} && A2}  | A1 && {TRUE[*]; A2} */
  res_automaton = Automaton_union(manager, and1, and2);

  Automaton_destroy(and1);
  Automaton_destroy(and2);


  {
    Automaton_ptr app_automaton;
    app_automaton = res_automaton;
    res_automaton = Automaton_prune_non_accepting_paths(manager, res_automaton);
    Automaton_destroy(app_automaton);
  }

  return res_automaton;
}

Automaton_ptr automaton_concatenation(AutomataManager_ptr manager,
                                      Automaton_ptr A1,
                                      Automaton_ptr A2)
{
  Automaton_ptr res_automaton;
  AutomataLocHash_ptr location_map;
  Slist_ptr stack;

  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  res_automaton = Automaton_create_empty(manager);
  /* Report final flag on initial location */
  automaton_set_location_final(res_automaton,
                               Automaton_get_unique_init_loc(res_automaton),
                               AutomataLoc_is_final(Automaton_get_unique_init_loc(A1)));

  automaton_init_type_and_complete_from_aut(res_automaton,
                                            manager,
                                            A1,
                                            A2);

  location_map = AutomataLocHash_create();
  stack = Slist_create();

  /* TODO: CHECK IF A1 AND A2 are empty  */
  /* init the type of automaton */
  AutomataLocHash_insert(location_map,
                         Automaton_get_unique_init_loc(A1),
                         Automaton_get_unique_init_loc(res_automaton));

  /* Copy all the automaton A2, from the initial one, but not including it.
     Takes all locations that Automaton_get_unique_init_loc(A2) reaches directly with a
     transition. Then copy all the reachable locations from this set. */
  automaton_copy_from_initial_loc(res_automaton, A2, location_map, true, errmgr);

  /* Copy locations from A1 to res_automaton.
     When in the copy a final location is visited, then out transitions
     from Automaton_get_unique_init_loc(A2) are copied.
  */
  Slist_push(stack, Automaton_get_unique_init_loc(A1));
  while(! Slist_is_empty(stack)) {
    AutomataLoc_ptr s1;
    AutomataLoc_ptr res_location;
    Slist_ptr a1_trans_list;
    Siter trans_iter;
    boolean s1_is_final;

    s1 = AUTOMATA_LOC(Slist_pop(stack));
    s1_is_final = AutomataLoc_is_final(s1);

    res_location = AutomataLocHash_find(location_map, s1);
    nusmv_assert(res_location != NULL); /* res_location is created before */

    /* if the location is a sink then do not copy out transition. This
       lead to an explosion of TRUE transition! */
    if (ABSTRACT_AUTOMATON(res_automaton)->sink_location != res_location) {

      /* final locations of A1:
         - if initial location of A2 is final, then final locations of A1
         must be final (A2 accept empty word, so A1 concatenated to
         A2 must accept words in A1 concatenated with empty).
         - if initial location of A2 is not final, then final locations
         of A1 are not final.
      */
      automaton_set_location_final(res_automaton,
                                   res_location,
                                   (s1_is_final &&
                                    AutomataLoc_is_final(Automaton_get_unique_init_loc(A2))));

      a1_trans_list = AutomataLoc_get_transitions(s1);
      for(trans_iter = Slist_first(a1_trans_list);
          ! Siter_is_end(trans_iter);
          trans_iter = Siter_next(trans_iter)) {
        AutomataTransition_ptr res_trans;
        AutomataTransition_ptr a1_trans;

        a1_trans = AUTOMATA_TRANSITION(Siter_element(trans_iter));

        /* If s1 is final and keeps complete option is enabled, then
           skip transitions to sink location. These transitions will be
           build after when complete information on location is recovered.
        */
        if (s1_is_final &&
            AutomataManager_is_keep_complete(manager) &&
            ABSTRACT_AUTOMATON(A1)->sink_location == AutomataTransition_get_end_location(a1_trans)) {
        } else {
          res_trans = automaton_copy_transition_to_loc(res_automaton,
                                                       res_location,
                                                       A1,
                                                       a1_trans,
                                                       location_map,
                                                       stack,
                                                       errmgr);
        }
      } /* end of transitions from s1 loop */

      /* If s1 is final, then all transitions from
         Automaton_get_unique_init_loc(A2) are copied to res_location */
      if (s1_is_final) {
        Slist_ptr transitions_init_2;
        Siter trans_iter_2;

        transitions_init_2 = AutomataLoc_get_transitions(Automaton_get_unique_init_loc(A2));

        for(trans_iter_2 = Slist_first(transitions_init_2);
            ! Siter_is_end(trans_iter_2);
            trans_iter_2 = Siter_next(trans_iter_2)) {
          AutomataTransition_ptr trans_2;
          AutomataLabel_ptr label_2;
          AutomataLoc_ptr a2_destination_location;
          AutomataLoc_ptr res_dest_location;

          trans_2 = AUTOMATA_TRANSITION(Siter_element(trans_iter_2));
          label_2 = AutomataTransition_get_label(trans_2);

          a2_destination_location = AutomataTransition_get_end_location(trans_2);

          res_dest_location = AutomataLocHash_find(location_map,
                                                   a2_destination_location);
          nusmv_assert(res_dest_location != NULL);

          AutomataLoc_create_transition(res_location,
                                        res_dest_location,
                                        AutomataLabel_copy(label_2),
                                        errmgr);
        }

        /* Manage the deterministic flag */
        automaton_update_type_and_complete(res_automaton,
                                           manager,
                                           res_location);
      }
    } /* end of sink != res_location */
  } /* end of DFS loop */

  AutomataLocHash_destroy(location_map);
  Slist_destroy(stack);

  return res_automaton;
}

Automaton_ptr automaton_concatenation_n_times(AutomataManager_ptr manager,
                                              Automaton_ptr A1,
                                              int n)
{
  Automaton_ptr res_automaton;
  int i;

  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  AUTOMATON_CHECK_INSTANCE(A1);
  nusmv_assert(n >= 0);

  res_automaton = A1;

  for(i=0; i<n; i++) {
    Automaton_ptr app_automaton;
    app_automaton = res_automaton;
    res_automaton = Automaton_concatenation(manager, res_automaton, A1);
    Automaton_destroy(app_automaton);
  }

  return res_automaton;
}

Automaton_ptr automaton_kleene_star(AutomataManager_ptr manager,
                                    Automaton_ptr self)
{
  Automaton_ptr res_automaton;

  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  AUTOMATON_CHECK_INSTANCE(self);

  /* TODO: check for empty self */
  res_automaton = automaton_kleene_plus(manager, self);

  /* makes final the initial location! */
  automaton_set_location_final(res_automaton, Automaton_get_unique_init_loc(res_automaton), true);

  return res_automaton;
}

/*!
  \brief Creates the automaton that recognize the Kleene
   plus language of language self

  self new automaton that recognize the language of self
   one or more time is created. The new automaton is created copying self
   and copying every transitions that reach a final location on the
   initial location.
*/

#if KLEENE_PLUS_FROM_END_STATE
Automaton_ptr automaton_kleene_plus(AutomataManager_ptr manager,
                                    Automaton_ptr self)
{
  Automaton_ptr res_automaton;
  AutomataLocHash_ptr location_map;
  Slist_ptr stack;
  ErrorMgr_ptr errmgr;
  NuSMVEnv_ptr env;

  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  AUTOMATON_CHECK_INSTANCE(self);

  env = EnvObject_get_environment(ENV_OBJECT(manager));
  errmgr = ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  res_automaton = Automaton_create_empty(manager);
  /* Report final flag on initial location */
  automaton_set_location_final(res_automaton,
                               Automaton_get_unique_init_loc(res_automaton),
                               AutomataLoc_is_final(Automaton_get_unique_init_loc(self)));

  /* initialize type and complete of automaton */
  ABSTRACT_AUTOMATON(res_automaton)->type = ABSTRACT_AUTOMATON(self)->type;
  res_automaton->is_complete = self->is_complete;

  location_map = AutomataLocHash_create();
  stack = Slist_create();

  /* TODO ADD CHECK FOR EMPTY self */

  AutomataLocHash_insert(location_map, Automaton_get_unique_init_loc(self), Automaton_get_unique_init_loc(res_automaton));
  Slist_push(stack, Automaton_get_unique_init_loc(self));

  /* DFS on self */
  while(! Slist_is_empty(stack)) {
    AutomataLoc_ptr s1;
    AutomataLoc_ptr res_location;
    Slist_ptr s1_transitions;
    Siter s1_trans_iter;

    s1 = AUTOMATA_LOC(Slist_pop(stack));
    res_location = AutomataLocHash_find(location_map, s1);
    AUTOMATA_LOC_CHECK_INSTANCE(res_location);

    /* Copy all transitions in out from s1 to res_location */
    s1_transitions = AutomataLoc_get_transitions(s1);
    for(s1_trans_iter = Slist_first(s1_transitions);
        ! Siter_is_end(s1_trans_iter);
        s1_trans_iter = Siter_next(s1_trans_iter)) {
      AutomataTransition_ptr s1_trans;
      AutomataLoc_ptr s1_end_location;

      s1_trans = AUTOMATA_TRANSITION(Siter_element(s1_trans_iter));
      (void)automaton_copy_transition_to_loc(res_automaton,
                                             res_location,
                                             self,
                                             s1_trans,
                                             location_map,
                                             stack,
                                             errmgr);

      /* if end location of s1_trans is final, then copy the transition
         on initial location of res_automaton */
      s1_end_location = AutomataTransition_get_end_location(s1_trans);

      if(AutomataLoc_is_final(s1_end_location)) {
        AutomataLabel_ptr trans_label;

        trans_label = AutomataTransition_get_label(s1_trans);
        AutomataLoc_create_transition(res_location,
                                      Automaton_get_unique_init_loc(res_automaton),
                                      AutomataLabel_copy(trans_label),
                                      errmgr);

        ABSTRACT_AUTOMATON(res_automaton)->type = NON_DETERMINISTIC;
        AutomataLoc_set_type(res_location, NON_DETERMINISTIC);
      }
    }
  }

  AutomataLocHash_destroy(location_map);
  Slist_destroy(stack);

  return res_automaton;
}
#else
/* [SM] Different version that creates a transition from end locations to
   locations reached by initial locations (as graz) */
Automaton_ptr automaton_kleene_plus(AutomataManager_ptr manager,
                                    Automaton_ptr self)
{
  Automaton_ptr res_automaton;
  AutomataLocHash_ptr location_map;
  Slist_ptr stack;

  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  AUTOMATON_CHECK_INSTANCE(self);

  res_automaton = Automaton_create_empty(manager);
  /* Report final flag on initial location */
  automaton_set_location_final(res_automaton,
                               Automaton_get_unique_init_loc(res_automaton),
                               AutomataLoc_is_final(Automaton_get_unique_init_loc(self)));

  /* initialize type and complete of automaton */
  ABSTRACT_AUTOMATON(res_automaton)->type = ABSTRACT_AUTOMATON(self)->type;
  res_automaton->is_complete = self->is_complete;

  location_map = AutomataLocHash_create();
  stack = Slist_create();

  AutomataLocHash_insert(location_map, Automaton_get_unique_init_loc(self), Automaton_get_unique_init_loc(res_automaton));
  Slist_push(stack, Automaton_get_unique_init_loc(self));

  while(! Slist_is_empty(stack)) {
    AutomataLoc_ptr s1;
    AutomataLoc_ptr res_location;
    Slist_ptr s1_transitions;
    Siter s1_trans_iter;

    s1 = AUTOMATA_LOC(Slist_pop(stack));
    res_location = AutomataLocHash_find(location_map, s1);
    AUTOMATA_LOC_CHECK_INSTANCE(res_location);

    /* Copy all transitions in out from s1 to res_location */
    s1_transitions = AutomataLoc_get_transitions(s1);
    for(s1_trans_iter = Slist_first(s1_transitions);
        ! Siter_is_end(s1_trans_iter);
        s1_trans_iter = Siter_next(s1_trans_iter)) {
      AutomataTransition_ptr s1_trans;
      AutomataTransition_ptr res_trans;

      s1_trans = AUTOMATA_TRANSITION(Siter_element(s1_trans_iter));
#warning [MD2SM] set but unused var
      res_trans = automaton_copy_transition_to_loc(res_automaton,
                                                   res_location,
                                                   self,
                                                   s1_trans,
                                                   location_map,
                                                   stack,
                                                   errmgr);
      UNUSED_VAR(res_trans);
    }

    /* if res_location is final then creates a transition from it to all
       locations that are reached by the initial location of self.

       Note that initial location in the result automaton can be used.
    */
    if (AutomataLoc_is_final(res_location)) {
      /* res location is connected to next of res_location! */
      if (Automaton_get_unique_init_loc(res_automaton) != res_location) {
        Slist_ptr init_out_trans;
        Siter init_trans_iter;

        init_out_trans = AutomataLoc_get_transitions(Automaton_get_unique_init_loc(res_automaton));

        SLIST_FOREACH(init_out_trans, init_trans_iter) {
          AutomataTransition_ptr out_trans;
          AutomataLoc_ptr dest_location;
          AutomataLabel_ptr trans_label;

          out_trans = AUTOMATA_TRANSITION(Siter_element(init_trans_iter));
          dest_location = AUTOMATA_LOC(AutomataTransition_get_end_location(out_trans));

          /* create a new transition */
          trans_label = AutomataTransition_get_label(out_trans);
          AutomataLoc_create_transition(res_location,
                                        dest_location,
                                        AutomataLabel_copy(trans_label),
                                        errmgr);

          /* These conditions can be refined! */
          ABSTRACT_AUTOMATON(res_automaton)->type = NON_DETERMINISTIC;
          AutomataLoc_set_type(res_location, NON_DETERMINISTIC);
        }
      }
    }
  }

  AutomataLocHash_destroy(location_map);
  Slist_destroy(stack);

  return res_automaton;
}
#endif

Automaton_ptr automaton_fusion(AutomataManager_ptr manager,
                               Automaton_ptr A1,
                               Automaton_ptr A2)
{
  NuSMVEnv_ptr env;
  ErrorMgr_ptr errmgr;
  Automaton_ptr res_automaton;
  Slist_ptr stack;
  AutomataLocHash_ptr location_map;
  Set_t trans_set_from_init_2;
  Siter trans_iter_2;
  Slist_ptr trans_from_init_2;
  AutomataAlgoChooser_ptr algo_chooser;

  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  AUTOMATON_CHECK_INSTANCE(A1);
  AUTOMATON_CHECK_INSTANCE(A2);

  env = EnvObject_get_environment(ENV_OBJECT(manager));
  errmgr = ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  res_automaton = Automaton_create_empty(manager);
  /* Report final flag on initial location */
  automaton_set_location_final(res_automaton,
                               Automaton_get_unique_init_loc(res_automaton),
                               AutomataLoc_is_final(Automaton_get_unique_init_loc(A1)));

  automaton_init_type_and_complete_from_aut(res_automaton,
                                            manager,
                                            A1,
                                            A2);

  stack = Slist_create();
  location_map = AutomataLocHash_create();

  AutomataLocHash_insert(location_map,
                         Automaton_get_unique_init_loc(A1),
                         Automaton_get_unique_init_loc(res_automaton));

  /* Copy all the automaton A2, from the initial one, but not including it.
     Takes all locations that Automaton_get_unique_init_loc(A2) reaches directly with a
     transition. Then copy all the reachable locations from this set. */
  automaton_copy_from_initial_loc(res_automaton, A2, location_map, true, errmgr);

  /* Builds the set of transitions in out from init_2 */
  trans_from_init_2 = AutomataLoc_get_transitions(Automaton_get_unique_init_loc(A2));
  trans_set_from_init_2 = Set_MakeEmpty();
  for(trans_iter_2 = Slist_first(trans_from_init_2);
      ! Siter_is_end(trans_iter_2);
      trans_iter_2 = Siter_next(trans_iter_2)) {
    trans_set_from_init_2 = Set_AddMember(trans_set_from_init_2,
                                          (Set_Element_t)Siter_element(trans_iter_2));
  }

  /* Creates the algorithm chooser instance, telling to always consider in the */
  algo_chooser = AutomataAlgoChooser_create();
  AutomataAlgoChooser_set_permanent_location(algo_chooser, manager, Automaton_get_unique_init_loc(A2));

  Slist_push(stack, Automaton_get_unique_init_loc(A1));

  /* DFS of A1 */
  while(!Slist_is_empty(stack)) {
    AutomataLoc_ptr s1;
    AutomataLoc_ptr res_location;
    Slist_ptr s1_transitions;
    Siter s1_trans_iter;

    s1 = AUTOMATA_LOC(Slist_pop(stack));
    res_location = AutomataLocHash_find(location_map, s1);
    AUTOMATA_LOC_CHECK_INSTANCE(res_location);

    /* set non final !*/
    automaton_set_location_final(res_automaton,
                                 res_location,
                                 false);


    /* Copy all transitions in out from s1 to res_location */
    s1_transitions = AutomataLoc_get_transitions(s1);
    for(s1_trans_iter = Slist_first(s1_transitions);
        ! Siter_is_end(s1_trans_iter);
        s1_trans_iter = Siter_next(s1_trans_iter)) {
      AutomataTransition_ptr s1_trans;
      AutomataLoc_ptr s1_end_location;

      s1_trans = AUTOMATA_TRANSITION(Siter_element(s1_trans_iter));
      (void)automaton_copy_transition_to_loc(res_automaton,
                                             res_location,
                                             A1,
                                             s1_trans,
                                             location_map,
                                             stack,
                                             errmgr);

      /* If the end_location is a final location perform the intersection of
         transition with all the transitions that leaves A2.initial_location
      */
      s1_end_location = AutomataTransition_get_end_location(s1_trans);
      if(AutomataLoc_is_final(s1_end_location)) {
        Set_t transitions_set;
        Automaton_Algorithm_Type method_type;

        transitions_set = Set_MakeSingleton((Set_Element_t)s1_trans);
        transitions_set = Set_Union(transitions_set,
                                    trans_set_from_init_2);

        method_type =
          AutomataAlgoChooser_choose_from_trans(algo_chooser, manager, s1_trans);

        switch(method_type) {
        case ALPHABET_BASED:
          {
            Set_t support_set;

            support_set = AutomataAlgoChooser_get_last_support(algo_chooser);

            automaton_alphabet_based_step_fusion(manager,
                                                 res_automaton,
                                                 location_map,
                                                 res_location,
                                                 s1_trans,
                                                 trans_set_from_init_2,
                                                 support_set);
          }
          break;
        case TRANSITIONS_BASED:
          automaton_transitions_based_step_fusion(manager,
                                                  res_automaton,
                                                  location_map,
                                                  res_location,
                                                  s1_trans,
                                                  trans_from_init_2);
          break;
        default:
          /* Generates an error */
          automata_error_unknown_algorithm_type(errmgr, method_type);
        }
      }
    } /* end of for on transitions */

    /* Manage the deterministic flag */
    automaton_update_type_and_complete(res_automaton,
                                       manager,
                                       res_location);
  } /* end of DFS */

  AutomataAlgoChooser_destroy(algo_chooser);
  Set_ReleaseSet(trans_set_from_init_2);
  AutomataLocHash_destroy(location_map);
  Slist_destroy(stack);

  {
    /* Prune non accepting paths.

       Non accepting paths can be introduced by fusion when final
       locations from first automaton cannot reach (in n steps) a location
       in the second automaton.
    */
    Automaton_ptr app_automaton;
    app_automaton = res_automaton;
    res_automaton = Automaton_prune_non_accepting_paths(manager,
                                                        res_automaton);
    Automaton_destroy(app_automaton);
  }

  return res_automaton;
}

Automaton_ptr automaton_reverse(AutomataManager_ptr manager, Automaton_ptr self)
{
  NuSMVEnv_ptr env;
  ErrorMgr_ptr errmgr;
  Automaton_ptr res_automaton;
  Automaton_ptr unpruned_automaton;
  AutomataLocHash_ptr location_map;
  Slist_ptr stack;
  AutomataLoc_ptr final_location;

  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  AUTOMATON_CHECK_INSTANCE(self);

  env = EnvObject_get_environment(ENV_OBJECT(manager));
  errmgr = ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  res_automaton = Automaton_create_empty(manager);
  automaton_set_location_final(res_automaton,
                               Automaton_get_unique_init_loc(res_automaton),
                               AutomataLoc_is_final(Automaton_get_unique_init_loc(self)));

  location_map = AutomataLocHash_create();
  stack = Slist_create();

  /* TODO: check for self empty */

  /* Creates one final location for result_automaton.
     The initial location of self is mapped with this final_location of
     res_automaton.
  */
  final_location = ABSTRACT_AUTOMATON(res_automaton)->
    create_location_from_manager(ABSTRACT_AUTOMATON(res_automaton), manager);
  AutomataLoc_set_final(final_location, true);
  automaton_add_loc(ABSTRACT_AUTOMATON(res_automaton), final_location);

  AutomataLocHash_insert(location_map, Automaton_get_unique_init_loc(self), final_location);

  /* performs a DFS on self.
     self is copied as follow:
     - Every location is copied: if the location final then it becomes
     initial, if it is initial then it becomes final (this is done in
     the initialization part of algorithm)
     - Every transition is copied reversing its direction (source
     and destination of transition are swapped)

     At the end we need to prune the automaton, deleting the
     non-reachable locations: this situation arises if in self there are
     paths that don't reach a final location.
  */
  Slist_push(stack, Automaton_get_unique_init_loc(self));
  while(! Slist_is_empty(stack)) {
    AutomataLoc_ptr a_location;
    AutomataLoc_ptr res_location;
    Slist_ptr a_trans_list;
    Siter a_trans_iter;

    a_location = AUTOMATA_LOC(Slist_pop(stack));
    a_trans_list = AutomataLoc_get_transitions(a_location);

    res_location = AutomataLocHash_find(location_map, a_location);
    /* res_location has been created previously */
    nusmv_assert(res_location != NULL);

    /* loops all output transitions of a_location */
    for(a_trans_iter = Slist_first(a_trans_list);
        ! Siter_is_end(a_trans_iter);
        a_trans_iter = Siter_next(a_trans_iter)) {
      AutomataTransition_ptr a_trans;
      AutomataLabel_ptr label;
      AutomataLoc_ptr source_location;
      AutomataLoc_ptr a_dest_location;

      a_trans = AUTOMATA_TRANSITION(Siter_element(a_trans_iter));

      a_dest_location = AutomataTransition_get_end_location(a_trans);

      source_location = AutomataLocHash_find(location_map, a_dest_location);
      if(source_location == NULL) {
        /* create a new location */
        source_location = automaton_hash_find_or_create_loc(res_automaton,
                                                            location_map,
                                                            self,
                                                            a_dest_location);
        /* visit dest_location at the next iteration */
        Slist_push(stack, a_dest_location);
      }

      /* Creates a new transition INVERTING its direction in result
         automaton */
      label = AutomataLabel_copy(AutomataTransition_get_label(a_trans));
      AutomataLoc_create_transition(source_location, res_location,
                                    label, errmgr);

      /* If the transition in self arrives in a final locations performs
         two tasks:
         - Create a transition from Automaton_get_unique_init_loc(res_automaton) to
         res_location
         - Make source_location non-final in the result automaton
      */
      if(AutomataLoc_is_final(a_dest_location)) {
        AutomataLabel_ptr label_from_init;
        label_from_init = AutomataLabel_copy(label);
        AutomataLoc_create_transition(Automaton_get_unique_init_loc(res_automaton),
                                      res_location,
                                      label_from_init,
                                      errmgr);

        automaton_set_location_final(res_automaton, source_location, false);
      }
    } /* end loops of trans from a_location */
  } /* end of DFS */

  /* Prune the obtained automaton.
     Pruning is needed for locations that have no out transitions.
     Reversing makes these locations non reachable by a search that
     starts from the initial locations of res_automaton.
     Example: self,B,C locations, self->B, B->C tranistions with self initial and
     C final.
     res_automaton until here has locations self,B,C,NI, with NI new initial
     location and self final locations and NI->B, C->B, B->self transitions. So
     the location C is non reachable.
  */
  unpruned_automaton = res_automaton;
  res_automaton = Automaton_prune(manager, unpruned_automaton);
  Automaton_destroy(unpruned_automaton);

  /* TODO: check if mantain complete information and automaton type
     info */

  AutomataLocHash_destroy(location_map);
  Slist_destroy(stack);

  return res_automaton;
}

Automaton_ptr automaton_complement(AutomataManager_ptr manager,
                                   Automaton_ptr self)
{
  Automaton_ptr res_automaton;
  Slist_ptr new_final_locations;
  Siter location_iter;
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  /* Creates the new automaton, determinizing and completing if
     necessary */
  if((ABSTRACT_AUTOMATON(self)->type != DETERMINISTIC) || (! self->is_complete)) {
    /* determinize and complete automaton */
    res_automaton = automaton_determinize(manager, self);
  }else{
    /* Just copy the automaton */
    AutomataLocHash_ptr location_map;

    location_map = AutomataLocHash_create();

    res_automaton = Automaton_create_empty(manager);
    AutomataLocHash_insert(location_map,
                           Automaton_get_unique_init_loc(self),
                           Automaton_get_unique_init_loc(res_automaton));
    automaton_set_location_final(res_automaton,
                                 Automaton_get_unique_init_loc(res_automaton),
                                 AutomataLoc_is_final(Automaton_get_unique_init_loc(self)));

    automaton_copy_reachable_from_loc(res_automaton,
                                      location_map,
                                      self,
                                      Automaton_get_unique_init_loc(self),
                                      errmgr);

    AutomataLocHash_destroy(location_map);
  }

  /* swap final with non-final locations. */
  new_final_locations = Slist_create();

  for(location_iter = Slist_first(ABSTRACT_AUTOMATON(res_automaton)->locations);
      ! Siter_is_end(location_iter);
      location_iter = Siter_next(location_iter)) {
    AutomataLoc_ptr location;
    location = AUTOMATA_LOC(Siter_element(location_iter));

    if(AutomataLoc_is_final(location)) {
      AutomataLoc_set_final(location, false);
    }else{
      AutomataLoc_set_final(location, true);
      Slist_push(new_final_locations, location);
    }
  }

  Slist_destroy(res_automaton->final_locations);
  res_automaton->final_locations = new_final_locations;

  /* manage sink location. The sink
     location become final, so it cannot be the sink location anymore!
  */
  ABSTRACT_AUTOMATON(res_automaton)->sink_location = NULL;

  return res_automaton;
}

Automaton_ptr automaton_difference(AutomataManager_ptr manager,
                                   Automaton_ptr A1,
                                   Automaton_ptr A2)
{
  Automaton_ptr res_automaton;
  Automaton_ptr neg_A2;

  neg_A2 = Automaton_complement(manager, A2);

  res_automaton = Automaton_intersection(manager, A1, neg_A2);

  Automaton_destroy(neg_A2);

  return res_automaton;
}

boolean automaton_containment(AutomataManager_ptr manager,
                              Automaton_ptr A1,
                              Automaton_ptr A2)
{
  Automaton_ptr difference_automaton;
  boolean is_empty;

  difference_automaton = automaton_difference(manager, A1, A2);

  is_empty = Automaton_emptyness_check(manager, difference_automaton);

  Automaton_destroy(difference_automaton);

  return is_empty;
}

Automaton_ptr automaton_determinize(AutomataManager_ptr manager,
                                    Automaton_ptr self)
{
  Automaton_ptr res_automaton;
  AutomataLocSetHash_ptr location_map;
  Slist_ptr stack;
  AutomataLocSet_ptr location_set;
  node_ptr node;
  BoolEnc_ptr bool_enc;
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

  res_automaton = Automaton_create_empty(manager);
  automaton_set_location_final(res_automaton,
                               Automaton_get_unique_init_loc(res_automaton),
                               AutomataLoc_is_final(Automaton_get_unique_init_loc(self)));

  bool_enc = AutomataManager_get_bool_enc(manager);

  location_map = AutomataLocSetHash_create(bool_enc, ABSTRACT_AUTOMATON(self)->location_count);
  stack = Slist_create();

  location_set = AutomataLocSet_create(env);
  AutomataLocSet_add_element(location_set, Automaton_get_unique_init_loc(self));

  node = new_node(nodemgr, CONS, NODE_PTR(location_set), NODE_PTR(Automaton_get_unique_init_loc(res_automaton)));

  Slist_push(stack, node);
  while(! Slist_is_empty(stack)) {
    AutomataLoc_ptr res_location;
    AutomataAlgoChooser_ptr algo_chooser;
    Automaton_Algorithm_Type method_type;

    node = Slist_pop(stack);
    location_set = AUTOMATA_LOC_SET(car(node));
    res_location = AUTOMATA_LOC(cdr(node));

    algo_chooser = AutomataAlgoChooser_create();
    method_type =
      AutomataAlgoChooser_choose_from_location_set(algo_chooser,
                                                   manager,
                                                   location_set);

    switch(method_type) {
    case ALPHABET_BASED:
      {
        Set_t support_set;

        support_set = AutomataAlgoChooser_get_last_support(algo_chooser);

        automaton_alphabet_based_step_determinization(manager,
                                                      res_automaton,
                                                      location_map,
                                                      stack,
                                                      res_location,
                                                      location_set,
                                                      support_set);
      }
      break;
    case TRANSITIONS_BASED:
      automaton_transitions_based_step_determinization(manager,
                                                       res_automaton,
                                                       location_map,
                                                       stack,
                                                       res_location,
                                                       location_set);
      break;
    default:
      /* Generates an error */
      automata_error_unknown_algorithm_type(errmgr, method_type);
    }

    AutomataAlgoChooser_destroy(algo_chooser);
    free_node(nodemgr, node);

    AutomataLocSet_destroy(location_set);
  } /* End of DFS loop */

  Slist_destroy(stack);
  AutomataLocSetHash_destroy(location_map);

  return res_automaton;
}

Automaton_ptr automaton_minimize(AutomataManager_ptr manager, Automaton_ptr A1)
{
  /* Implements brzozowski algorithm for minimization

     det(rev(det(rev(A1))))
  */

  Automaton_ptr determinized;
  Automaton_ptr reversed;

  reversed = Automaton_reverse(manager, A1);
  determinized = Automaton_determinize(manager, reversed);
  Automaton_destroy(reversed);
  reversed = Automaton_reverse(manager, determinized);
  Automaton_destroy(determinized);
  determinized = Automaton_determinize(manager, reversed);
  Automaton_destroy(reversed);

  return determinized;
}

Automaton_ptr automaton_nfa_reduction(AutomataManager_ptr manager, Automaton_ptr A1)
{
  Automaton_ptr minimized_aut = AUTOMATON(Nil);
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  switch (AutomataManager_get_reduction_algo_type(manager)) {
  case AUT_RED_ALG_BISIMULATION:
    minimized_aut = automaton_bisimulation_reduction(manager, A1);
    break;
  case AUT_RED_ALG_BRZOZOWSKI:
    minimized_aut = automaton_minimize(manager, A1);
    break;
  default:
    automata_error_unknown_reduction_algo_type(errmgr,
                                               AutomataManager_get_reduction_algo_type(manager));
  }

  return minimized_aut;
}

boolean automaton_equals(AutomataManager_ptr manager,
                         Automaton_ptr A1,
                         Automaton_ptr A2)
{
  return (automaton_containment(manager, A1, A2) &&
          automaton_containment(manager, A2, A1));
}

boolean automaton_emptyness_check(AutomataManager_ptr manager, Automaton_ptr self)
{
  boolean is_empty = true;

  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  AUTOMATON_CHECK_INSTANCE(self);

  if(Automaton_get_unique_init_loc(self) == NULL) {
    /* automaton has no initial location */
    is_empty = true;
  }else if (AutomataLoc_is_final(Automaton_get_unique_init_loc(self))) {
    /* initial location is a final location */
    is_empty = false;
  }else if (Slist_is_empty(self->final_locations)) {
    is_empty = true;
  }else{
    /* check if a final location is reachable */
    Slist_ptr stack;
    AutomataLocHash_ptr location_map;

    stack = Slist_create();
    location_map = AutomataLocHash_create();

    Slist_push(stack, Automaton_get_unique_init_loc(self));
    AutomataLocHash_insert(location_map, Automaton_get_unique_init_loc(self), Automaton_get_unique_init_loc(self));
    while((! Slist_is_empty(stack)) && is_empty) {
      AutomataLoc_ptr location;
      Slist_ptr transitions;
      Siter trans_iter;

      location = AUTOMATA_LOC(Slist_pop(stack));

      /* loops every out transitions from location */
      transitions = AutomataLoc_get_transitions(location);
      for(trans_iter = Slist_first(transitions);
          (! Siter_is_end(trans_iter)) && is_empty;
          trans_iter = Siter_next(trans_iter)) {
        /* Check if destination location is new and then check if it is final */
        AutomataTransition_ptr transition;
        AutomataLoc_ptr dest_location;

        transition = AUTOMATA_TRANSITION(Siter_element(trans_iter));
        dest_location = AutomataTransition_get_end_location(transition);

        if(! AutomataLocHash_has_key(location_map, dest_location)) {

          if(AutomataLoc_is_final(dest_location)) {
            /* location is final, so language is not empty  */
            is_empty = false; /* force to break transition loop and DFS
                                 loop */
          }else{
            /* location is not final, push the location in stack */
            Slist_push(stack, dest_location);
            AutomataLocHash_insert(location_map, dest_location, dest_location);
          }
        }
      } /* End of transitions loop */
    } /* End of DFS */

    /* destroy the stack and the map */
    Slist_destroy(stack);
    AutomataLocHash_destroy(location_map);
  }

  return is_empty;
}

Automaton_ptr automaton_prune(AutomataManager_ptr manager, Automaton_ptr self)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  Automaton_ptr res_automaton;
  AutomataLocHash_ptr location_map;

  res_automaton = Automaton_create_empty(manager);

  location_map = AutomataLocHash_create();
  AutomataLocHash_insert(location_map,
                         Automaton_get_unique_init_loc(self),
                         Automaton_get_unique_init_loc(res_automaton));
  /* Set initial location */
  automaton_set_location_final(res_automaton,
                               Automaton_get_unique_init_loc(res_automaton),
                               AutomataLoc_is_final(Automaton_get_unique_init_loc(self)));

  automaton_copy_reachable_from_loc(res_automaton,
                                    location_map,
                                    self,
                                    Automaton_get_unique_init_loc(self),
                                    errmgr);
  /* destroy the the map */
  AutomataLocHash_destroy(location_map);

  return res_automaton;
}

Automaton_ptr automaton_prune_non_accepting_paths(AutomataManager_ptr manager,
                                                  Automaton_ptr self)
{
  int i;
  boolean* reach_accepting;
  boolean* A_visited;
  Slist_ptr stack;
  AutomataPredecessorsList_ptr predecessors;
  Automaton_ptr res_automaton;

  /*
    Steps:
    1. Visit self and determine all the locations that can reach a final
    location.
    2. Visit self and copy only the location that can reach a final location.

    This method allows to keep the sink location of automaton.
  */


  /* Initialize reachable information for every location.
     The index to the location is the location_id - 1.
  */
  reach_accepting = ALLOC(boolean, ABSTRACT_AUTOMATON(self)->location_count);
  A_visited = ALLOC(boolean, ABSTRACT_AUTOMATON(self)->location_count);

  for (i=0; i < ABSTRACT_AUTOMATON(self)->location_count; i++) {
    reach_accepting[i] = false;
    A_visited[i] = false;
  }

  predecessors = AutomataPredecessorsList_create(ABSTRACT_AUTOMATON(self)->location_count);

  /* WARNING [SM] This can be done more efficiently, visiting the automaton
     backward starting from final locations. */

  stack = Slist_create();
  Slist_push(stack, Automaton_get_unique_init_loc(self));
  while (! Slist_is_empty(stack)) {
    AutomataLoc_ptr s1;
    Siter trans_iter;

    s1 = Slist_pop(stack);
    A_visited[AutomataLoc_get_id(s1) - 1] = true;

    /* check if current_location is final. In this case all locations that
       reach current_location have to be set accepting location. */
    if(AutomataLoc_is_final(s1)) {
      reach_accepting[AutomataLoc_get_id(s1) - 1] = true;

      /* propagate information to predecessors  */
      automaton_mark_predecessors(s1, predecessors, reach_accepting);
    }

    SLIST_FOREACH(AutomataLoc_get_transitions(s1), trans_iter) {
      AutomataTransition_ptr s1_trans;
      AutomataLoc_ptr s1_dest;
      int s1_dest_id;

      s1_trans = AUTOMATA_TRANSITION(Siter_element(trans_iter));
      s1_dest = AutomataTransition_get_end_location(s1_trans);
      s1_dest_id = AutomataLoc_get_id(s1_dest);

      if (! A_visited[s1_dest_id - 1] ) {
        /* s1_dest has not been visited before, so we cannot know if
           it can reach an accepting location */
        Slist_push(stack, s1_dest);
      }else{
        /* Check if s1_dest can reach an accepting location.
           If it happens than all its predesessor can reach an
           accepting location.
        */
        if (reach_accepting[s1_dest_id - 1]) {
          reach_accepting[AutomataLoc_get_id(s1) - 1] = true;

          /* propagate information to predecessor */
          automaton_mark_predecessors(s1, predecessors, reach_accepting);
        }
      }

      /* updates predecessor list with s1_dest */
      AutomataPredecessorsList_insert(predecessors, s1_dest, s1);
    }
  }

  Slist_destroy(stack);
  FREE(A_visited);
  AutomataPredecessorsList_destroy(predecessors);


#if AUTOMATON_DEBUG
  StreamMgr_print_error(streams,  "Reachable locations\n");
  for (i=0; i<ABSTRACT_AUTOMATON(self)->location_count; i++) {
    if (reach_accepting[i])
      StreamMgr_print_error(streams,  "Reachable location: %d\n", i+1);
  }
#endif

  /* STEP 2 - Creates resulting automaton only with selected locations */
  res_automaton = automaton_copy_accepting_loc(manager, self, reach_accepting);

  FREE(reach_accepting);

  return res_automaton;
}

PslNode_ptr automaton_automa2sere(AutomataManager_ptr manager,
                                  Automaton_ptr self)
{
  error_unreachable_code();
}

int automaton_automaton2smv(AutomataManager_ptr manager,
                            Automaton_ptr self,
                            FILE* out_file)
{
  error_unreachable_code();
}

void automaton_print(AbstractAutomaton_ptr self, NuSMVEnv_ptr env, FILE* out_file)
{
  /* Prints general information of automaton:
     - type of auto
     - if it is complete
     - the type of the label
     - location_count
     - initial_locations
     - sink location
  */
  fprintf(out_file, "Print of automaton:\n");
  fprintf(out_file, "Type of automaton = %d\n", ABSTRACT_AUTOMATON(self)->type);
  fprintf(out_file, "Is complete = %d\n", AUTOMATON(self)->is_complete);
  fprintf(out_file, "Type of labels = %d\n", ABSTRACT_AUTOMATON(self)->label_type);
  fprintf(out_file, "Number of locations %d\n", ABSTRACT_AUTOMATON(self)->location_count);

  if(ABSTRACT_AUTOMATON(self)->location_count > 0){
    Siter location_iter;
    /* Initial location has the id 1 */
    fprintf(out_file,
            "Initial location: %d\n",
            AutomataLoc_get_id(Automaton_get_unique_init_loc(AUTOMATON(self))));

    /* Sink location */
    if(self->sink_location != NULL){
      fprintf(out_file,
              "Sink location: %d\n",
              AutomataLoc_get_id(self->sink_location));
    }

    /* Loops all locations of automaton and prints them */
    for(location_iter = Slist_first(ABSTRACT_AUTOMATON(self)->locations);
        ! Siter_is_end(location_iter);
        location_iter = Siter_next(location_iter)) {
      AutomataLoc_ptr current_location = AUTOMATA_LOC(Siter_element(location_iter));
      AUTOMATA_LOC_CHECK_INSTANCE(current_location);

      AutomataLoc_print(current_location, env, out_file);
    }
  }
}

void automaton_print_dot(AbstractAutomaton_ptr self, NuSMVEnv_ptr env, FILE* out_file)
{
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  if (!out_file) {
    automata_error_null_output_file (errmgr);
  }

  /* NOTE: this is now inserted for debugging, it can be improved */
  fprintf(out_file, "digraph {\n");
  fprintf(out_file, "  center = true;\n");
  fprintf(out_file, "  edge [fontname=\"Courier\", fontsize=10];\n");

  /* adds the fake location used to mark the initial location in the automaton */
  if(Automaton_get_unique_init_loc(AUTOMATON(self)) != NULL){
    fprintf(out_file,"init [shape=plaintext,label=\"\"]\n");
    fprintf(out_file,"init -> \"node%d\"\n", AutomataLoc_get_id(Automaton_get_unique_init_loc(AUTOMATON(self))));
  }

  if(self->location_count > 0){
    Siter location_iter;
    /* Loops all locations of automaton and prints them */
    for(location_iter = Slist_first(self->locations);
        ! Siter_is_end(location_iter);
        location_iter = Siter_next(location_iter)) {
      AutomataLoc_ptr current_location = AUTOMATA_LOC(Siter_element(location_iter));
      AUTOMATA_LOC_CHECK_INSTANCE(current_location);

      AutomataLoc_print_dot(current_location, env, out_file);
    }
  }

  fprintf(out_file, "}\n");
}

void automaton_add_loc(AbstractAutomaton_ptr self,
                       AutomataLoc_ptr location)
{
  AUTOMATON_CHECK_INSTANCE(self);

  abstract_automaton_add_loc(ABSTRACT_AUTOMATON(self), location);

  /* If the location is final, add it to the final location list */
  if(AutomataLoc_is_final(location)){
    Slist_push(AUTOMATON(self)->final_locations, location);
  }
}

void automaton_init_type_complete(Automaton_ptr self,
                                  AutomataManager_ptr manager)
{
  boolean keep_type;
  boolean keep_complete;

  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  keep_type = AutomataManager_is_keep_automaton_type(manager);
  keep_complete = AutomataManager_is_keep_complete(manager);

  if(keep_type) {
    ABSTRACT_AUTOMATON(self)->type = DETERMINISTIC;
  }else{
    ABSTRACT_AUTOMATON(self)->type = UNKNOWN;
  }

  if(keep_complete) {
    self->is_complete = true;
  }else{
    self->is_complete = false;
  }
}

void automaton_add_initial_location(AbstractAutomaton_ptr const self,
                                    AutomataLoc_ptr const location)
{
  nusmv_assert(Set_IsEmpty(self->initial_locations));

  abstract_automaton_add_initial_location(self, location);
}

void automaton_new_transition(AutomataManager_ptr manager,
                              Automaton_ptr A,
                              int src_location_id,
                              int dst_location_id,
                              Expr_ptr label_expr)
{
  AutomataLoc_ptr src_location, dst_location;
  AutomataLabel_ptr label;
  Siter iter;
  BddEnc_ptr bdd_enc = AutomataManager_get_bdd_enc(manager);
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  /* Find src and dst location by id */
  src_location = AUTOMATA_LOC(Nil);
  dst_location = AUTOMATA_LOC(Nil);
  SLIST_FOREACH(ABSTRACT_AUTOMATON(A)->locations, iter) {
    AutomataLoc_ptr location;

    location = AUTOMATA_LOC(Siter_element(iter));

    if (src_location_id == AutomataLoc_get_id(location)) {
      src_location = location;
    }
    if (dst_location_id == AutomataLoc_get_id(location)) {
      dst_location = location;
    }
  }

  nusmv_assert(AUTOMATA_LOC(Nil) != src_location);
  nusmv_assert(AUTOMATA_LOC(Nil) != dst_location);

  /* creates the label */
  label = automata_create_label_from_expr(ABSTRACT_AUTOMATON(A)->label_type,
                                          bdd_enc, label_expr);
  AutomataLoc_create_transition(src_location, dst_location, label, errmgr);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The Automaton class virtual finalizer

  Called by the class destructor
*/
static void automaton_finalize(Object_ptr self, void* dummy)
{
  Automaton_ptr _self = AUTOMATON(self);

  automaton_deinit(_self);
  FREE(_self);
}

/*!
  \brief Set final/non-final a location in the automaton

  Set a location as final or not-final and
   adds/removes it from the final list of locations of automaton.
   State must already be in the automaton, this condition is not
   checked in this function.
   If the location is set as non-final it must not be already present in
   the list. This condition is not checked.
*/
static void automaton_set_location_final(Automaton_ptr self,
                                         AutomataLoc_ptr location,
                                         boolean is_final)
{
  AutomataLoc_set_final(location, is_final);

  if(is_final) {
    /* add the location to the list */
    Slist_push(self->final_locations, location);
  }else{
    /* Copy all the final locations skipping location */
    Slist_ptr new_final_list;

    new_final_list = Slist_create();

    while(! Slist_is_empty(self->final_locations)) {
      AutomataLoc_ptr app_location;

      app_location = Slist_pop(self->final_locations);
      if(app_location != location) {
        Slist_push(new_final_list, app_location);
      }
    }

    Slist_destroy(self->final_locations);
    self->final_locations = new_final_list;
  }
}

/*!
  \brief Removes location from the automaton

  Removes location from the list of locations of
   automaton. This functino is  useful to keep count of the number of
   locations of the automaton. If location is not found in automaton 0 is
   returned, otherwise 1 is returned.

   This operation has a linear cost in the number of locations, because
   all the stack must be scanned
*/
static int automaton_remove_loc(Automaton_ptr self,
                                AutomataLoc_ptr location)
{
  /* Copy all the elements from self->locations to an auxiliary list,
     skipping location if found.
     The auxiliary list will become the locations list of automaton.
  */
  Slist_ptr app_list = Slist_create();
  boolean found = false;

  AUTOMATON_CHECK_INSTANCE(self);

  while (! Slist_is_empty(ABSTRACT_AUTOMATON(self)->locations)) {
    AutomataLoc_ptr app_location = AUTOMATA_LOC(Slist_pop(ABSTRACT_AUTOMATON(self)->locations));

    if(app_location != location) {
      Slist_push(app_list, app_location);

      /* updates the count of locations*/
      ABSTRACT_AUTOMATON(self)->location_count = ABSTRACT_AUTOMATON(self)->location_count - 1;

      found = true;
    }
  }

  /* swap locations stacks */
  Slist_destroy(ABSTRACT_AUTOMATON(self)->locations);
  ABSTRACT_AUTOMATON(self)->locations = app_list;

  /* Decreases the id of locations that are greater than id of the
     removed locations, if location was found in the previous
     iteration. This allow to use the count of the locations as
     id when a new location is added. */
  if(found){
    Siter location_iter;

    for(location_iter = Slist_first(ABSTRACT_AUTOMATON(self)->locations);
        ! Siter_is_end(location_iter);
        location_iter = Siter_next(location_iter)){
      AutomataLoc_ptr app_location;

      app_location = AUTOMATA_LOC(Siter_element(location_iter));

      if(AutomataLoc_get_id(app_location)>AutomataLoc_get_id(location)) {
        AutomataLoc_set_id(app_location, AutomataLoc_get_id(location)-1);
      }
    }
  }

  return found;
}

/*!
  \brief Copies all reachable locations from location
   start_location of automaton A1 to automaton self performing a DFS.

  Performs a DFS on automaton A1 starting from
   start_location. In the DFS all encountered locations and transitions are
   copied in the automaton self. The hash table can contain association
   from locations in A1 to locations in self, so if a location of A1 that was
   previously copied is found in the search this is not created again.

   The automaton self must be yet initialized.

  \se The hash table is used in the function becaues
   every association created from locations in A1 to locations in self is added
*/
static void automaton_copy_reachable_from_loc(Automaton_ptr self,
                                              AutomataLocHash_ptr hash,
                                              Automaton_ptr A1,
                                              AutomataLoc_ptr start_location,
                                              ErrorMgr_ptr errmgr)
{
  Slist_ptr stack;

  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATON_CHECK_INSTANCE(A1);
  AUTOMATA_LOC_HASH_CHECK_INSTANCE(hash);
  AUTOMATA_LOC_CHECK_INSTANCE(start_location);

  /* checks in self there is a corresponding location for start_location */
  (void)automaton_hash_find_or_create_loc(self,
                                          hash,
                                          A1,
                                          start_location);

  /* DFS on A1 starting at start_location */
  stack = Slist_create();
  Slist_push(stack, start_location);

  while(! Slist_is_empty(stack)) {
    AutomataLoc_ptr a1_location = Slist_pop(stack);
    AutomataLoc_ptr current_location_shadowing = AutomataLocHash_find(hash,
                                                                      a1_location);

    {
      /* loops all transitions leaving a1_location and copy to
         current_location_shadowing */
      Siter trans_iter;
      Slist_ptr transitions;

      transitions = AutomataLoc_get_transitions(a1_location);
      for(trans_iter = Slist_first(transitions);
          ! Siter_is_end(trans_iter);
          trans_iter = Siter_next(trans_iter)) {
        AutomataTransition_ptr transition;

        transition = AUTOMATA_TRANSITION(Siter_element(trans_iter));
        automaton_copy_transition_to_loc(self,
                                         current_location_shadowing,
                                         A1,
                                         transition,
                                         hash,
                                         stack,
                                         errmgr);
      }
    }
  }

}

/*!
  \brief Creates a copy of self where locations that cannot
   reach an accepting location are pruned.

  Creates a copy of self, where locations that cannot
   reach an accepting locations are pruned.

   reach_accepting is a boolean array that contains, for each location of
   self, a flag that is true only if the location can reach an accepting
   location.

   The array is indicized be location_id -1.
*/
static Automaton_ptr automaton_copy_accepting_loc(AutomataManager_ptr manager,
                                                  Automaton_ptr self,
                                                  boolean* reach_accepting)
{
  Slist_ptr stack;
  AutomataLocHash_ptr location_map;
  Automaton_ptr res_automaton;
  boolean keep_complete;
  BddEnc_ptr bdd_enc = AutomataManager_get_bdd_enc(manager);
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  res_automaton = Automaton_create_empty(manager);
  automaton_set_location_final(res_automaton,
                               Automaton_get_unique_init_loc(res_automaton),
                               AutomataLoc_is_final(Automaton_get_unique_init_loc(self)));

  ABSTRACT_AUTOMATON(res_automaton)->type = ABSTRACT_AUTOMATON(self)->type;
  keep_complete = AutomataManager_is_keep_complete(manager);
  if (keep_complete) {
    res_automaton->is_complete = self->is_complete;

    /* creates sink location */
    automaton_add_sink_loc(res_automaton,
                           automaton_create_sink_loc(res_automaton, manager));

  }else{
    /* When no check is performed to keep the automaton complete then
       the pruning cannot ensure that completeness is preserved.

       Example: a transition can be removed from an edge, making no
       connection to the sink location. In this way a complete location can
       become not complete.
    */
    res_automaton->is_complete = false;
  }

  location_map = AutomataLocHash_create();
  AutomataLocHash_insert(location_map,
                         Automaton_get_unique_init_loc(self),
                         Automaton_get_unique_init_loc(res_automaton));

  if (! reach_accepting[AutomataLoc_get_id(Automaton_get_unique_init_loc(self)) - 1]) {
    /* empty automaton */
  }else{
    stack = Slist_create();
    Slist_push(stack, Automaton_get_unique_init_loc(self));
    while (! Slist_is_empty(stack)) {
      AutomataLoc_ptr s1;
      AutomataLoc_ptr res_location;
      Siter trans_iter;
      AutomataLabel_ptr accepting_label;

      s1 = Slist_pop(stack);

      res_location = AutomataLocHash_find(location_map, s1);
      AUTOMATA_LOC_CHECK_INSTANCE(res_location);

      if (keep_complete) {
        /* accepting label contains the OR of all labels from
           transitions that goes in a location that can reach an accepting
           location.

           Initialized to false.
        */
        accepting_label = automata_create_label_from_expr(ABSTRACT_AUTOMATON(res_automaton)->label_type,
                                                          bdd_enc,
                                                          ExprMgr_false(exprs));
      }

      SLIST_FOREACH(AutomataLoc_get_transitions(s1), trans_iter) {
        AutomataTransition_ptr s1_trans;
        AutomataLoc_ptr s1_dest;
        int s1_dest_id;

        s1_trans = AUTOMATA_TRANSITION(Siter_element(trans_iter));
        s1_dest = AutomataTransition_get_end_location(s1_trans);
        s1_dest_id = AutomataLoc_get_id(s1_dest);

        if (reach_accepting[s1_dest_id - 1]) {
          AutomataLabel_ptr s1_trans_label;
          AutomataLabel_ptr app_label;
          /* creates destination location and transition
             Note that copying transition CHANGES the stack, pushing
             destination location of a1_trans on its top.
             This allows to keep searching in automaton self.
          */
          automaton_copy_transition_to_loc(res_automaton,
                                           res_location,
                                           self,
                                           s1_trans,
                                           location_map,
                                           stack,
                                           errmgr);

          if (keep_complete) {
            /* updates accepting label */
            s1_trans_label = AutomataTransition_get_label(s1_trans);
            app_label = accepting_label;
            accepting_label = AutomataLabel_union(accepting_label, s1_trans_label);
            AutomataLabel_destroy(app_label);
          }
        }

      }

      if (keep_complete) {
        /* create transition to sink with !accepting_label */
        AutomataLabel_ptr to_sink_label;

        to_sink_label = AutomataLabel_complement(accepting_label);

        if (AutomataLabel_is_sat(to_sink_label)) {
          AutomataLoc_create_transition(res_location,
                                        ABSTRACT_AUTOMATON(res_automaton)->sink_location,
                                        to_sink_label,
                                        errmgr);
        } else {
          AutomataLabel_destroy(to_sink_label);
        }
        AutomataLabel_destroy(accepting_label);
      }

    }
    Slist_destroy(stack);
  }

  return res_automaton;
}

/*!
  \brief Returns the association in hash with the given
   key. It the association is not present creates a new location s in self
   and an association from key to s and returns location

  First looks un key in hash. If a location is found
   then it is returned. If this does not happen then a new location is
   created in the automaton, it is inserted in the hash and is returned
   as result.

  \se When key is not found in hash a new location is
   created in self and a new assoc is created in hash.
*/
static AutomataLoc_ptr
automaton_hash_find_or_create_loc(Automaton_ptr self,
                                  AutomataLocHash_ptr hash,
                                  Automaton_ptr source_aut,
                                  AutomataLoc_ptr key)
{
  AutomataLoc_ptr found_location;
  boolean is_sink;

  is_sink = ABSTRACT_AUTOMATON(source_aut)->sink_location == key;
  if(is_sink){
    found_location = ABSTRACT_AUTOMATON(self)->sink_location;

    if(found_location != NULL) {
      /* if a sink location is found, then we need to check if a map from
         key to the sink location is present */
      if(AutomataLocHash_find(hash, key) == NULL)
        AutomataLocHash_insert(hash, key, found_location);
    }
  }
  else
    found_location = AutomataLocHash_find(hash, key);

  if (found_location != NULL) {
    return found_location;
  }
  else {
    AutomataLoc_ptr location;

    location = AutomataLoc_copy(key,
                                true,
                                ABSTRACT_AUTOMATON(self)->label_type,
                                self->keep_support_set_in_location);

    automaton_add_loc(ABSTRACT_AUTOMATON(self), location);

    /* update sink location */
    if(is_sink) ABSTRACT_AUTOMATON(self)->sink_location = location;

    AutomataLocHash_insert(hash, key, location);
    return location;
  }
}

/*!
  \brief All the locations of A1 reachable from it's initial
   location are copied to result_automaton

  The part of A1 reachable from its initial location
   is copied to result_automaton.

   The flag exclude_initial_locations determine if the transitions in
   output from initial location of A1 are copied to the initial location of
   result automaton.

   For every output transitions of the initial location of automaton
   A1:
   - if exlude_initial_locations is false, the transition is copied to
   result_automaton, creating a correspondent location in result_automaton
   and updating the hash if necessary.
   - copy to result_automaton all the reachable locations from the location
   reached by transition with a DFS.

   This function is used in automaton_union.

  \se result_automaton will contains all location from A1
   reachable from the initial location of A1, and the transition from the
   initial location if exclude_initial_locations is false.

   The hash table will contain the association from locations in A1 to
   location in result_automaton created during the copy

  \sa automaton_union, automaton_concatenation,
   automaton_fusion
*/
static void
automaton_copy_from_initial_loc(Automaton_ptr result_automaton,
                                Automaton_ptr A1,
                                AutomataLocHash_ptr hash,
                                boolean exclude_initial_locations,
                                ErrorMgr_ptr errmgr)
{
  AUTOMATON_CHECK_INSTANCE(result_automaton);
  AUTOMATON_CHECK_INSTANCE(A1);
  AUTOMATA_LOC_HASH_CHECK_INSTANCE(hash);

  nusmv_assert(Automaton_get_unique_init_loc(result_automaton) != NULL);

  if(Automaton_get_unique_init_loc(A1) != NULL) {

    Slist_ptr A1_init_trans =
      AutomataLoc_get_transitions(Automaton_get_unique_init_loc(A1));

    /* Copy reachable locations from initial location of A1 to initial
       location of result_automaton, then visit all reachable locations by
       these locations, copying from A1 to result_automaton all the
       visited locations.
    */

    Siter trans_iter;
    for(trans_iter = Slist_first(A1_init_trans);
        ! Siter_is_end(trans_iter);
        trans_iter = Siter_next(trans_iter) )
      {
        AutomataTransition_ptr transition;
        AutomataLabel_ptr label;
        AutomataLoc_ptr a1_dest_location;
        AutomataLoc_ptr dest_location;
        boolean visited;

        transition = AUTOMATA_TRANSITION(Siter_element(trans_iter));
        label = AutomataTransition_get_label(transition);
        a1_dest_location = AutomataTransition_get_end_location(transition);

        visited = AutomataLocHash_has_key(hash, a1_dest_location);
        dest_location = automaton_hash_find_or_create_loc(result_automaton,
                                                          hash,
                                                          A1,
                                                          a1_dest_location);

        /* Copy the transition from Automaton_get_unique_init_loc(A1) to
           Automaton_get_unique_init_loc(result_automaton) */
        if(! exclude_initial_locations) {
          AutomataLoc_create_transition(Automaton_get_unique_init_loc(result_automaton),
                                        dest_location,
                                        AutomataLabel_copy(label),
                                        errmgr);
        }

        /* Check if a1_dest_location has been visited before in the
           copy_reachable_from_location function. This can happen if
           a1_dest_location is reached by a location of A1 that is visited
           before a1_dest_location and that can visit a1_dest_location. */
        if (! visited) {
          automaton_copy_reachable_from_loc(result_automaton,
                                            hash,
                                            A1,
                                            a1_dest_location,
                                            errmgr);
        }

      }
  }
}

/*!
  \brief Performs a single step of the intersection
   operation using the alphabet method.

  Performs a single step of the intersection
   operation, performed on automata A1 and A2, using the alphabet method.
   res_automaton: the result automaton of intersection operation.
   location_map: the map from couples (s1, s2), with s1 from the A1 and s2
   from A2, to a location in res_automaton.
   stack: stack that contains triples of locations and is used in the main
   intersection algorithm, automaton_intersection.
   triple: the triple (s1, s2, res_location) with s1 from A1, s2 from A2
   and res_location from res_automaton.

  \se location_map could be changed: new associations from
   couples of locations to a location in the result automaton can be created.
   stack: elements can be added to stack to allow these new elements to
   be visited by the DFS performed in the main algorithm.

  \sa automaton_intersection
*/
static void
automaton_alphabet_based_step_intersection(AutomataManager_ptr manager,
                                           Automaton_ptr res_automaton,
                                           Automaton_ptr A1,
                                           Automaton_ptr A2,
                                           AutomataLocCoupleHash_ptr location_map,
                                           Slist_ptr stack,
                                           AutomataLocTriple_ptr triple,
                                           Set_t support_set)
{
  AutomataLoc_ptr s1;
  AutomataLoc_ptr s2;
  AutomataLoc_ptr res_location;
  BddEnc_ptr bdd_enc = AutomataManager_get_bdd_enc(manager);
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  Set_t alphabet;
  Set_Iterator_t alphabet_iter;

  /* get locations from the triple */
  s1 = AutomataLocTriple_get_first(triple);
  s2 = AutomataLocTriple_get_second(triple);
  res_location = AutomataLocTriple_get_third(triple);

  /* builds the alphabet */
  alphabet = AutomataManager_get_alphabet(manager, support_set);

  /* Checks with which symbol s1 and s2 moves together. In that case
     creates a new transition from res_location to a new location in the
     resulting automaton. The new location is mapped with the couple of
     destination locations reached by s1 and s2 with the transition.
  */
  SET_FOREACH(alphabet, alphabet_iter) {
    Expr_ptr symbol;
    Slist_ptr s1_trans;
    Siter s1_trans_iter;

    symbol = EXPR(Set_GetMember(alphabet, alphabet_iter));

    /* Loops all transitions in out from s1 */
    s1_trans = AutomataLoc_get_transitions(s1);
    for(s1_trans_iter = Slist_first(s1_trans);
        ! Siter_is_end(s1_trans_iter);
        s1_trans_iter = Siter_next(s1_trans_iter)) {
      Slist_ptr s2_trans;
      Siter s2_trans_iter;
      AutomataTransition_ptr t1;
      AutomataLabel_ptr l1;

      t1 = AUTOMATA_TRANSITION(Siter_element(s1_trans_iter));
      l1 = AUTOMATA_LABEL(AutomataTransition_get_label(t1));

      if(AutomataLabel_is_model(l1, symbol)) {
        /* Loops all transitions in out from s2 */
        s2_trans = AutomataLoc_get_transitions(s2);
        for(s2_trans_iter = Slist_first(s2_trans);
            ! Siter_is_end(s2_trans_iter);
            s2_trans_iter = Siter_next(s2_trans_iter)) {
          AutomataTransition_ptr t2;
          AutomataLabel_ptr l2;
          t2 = AUTOMATA_TRANSITION(Siter_element(s2_trans_iter));
          l2 = AUTOMATA_LABEL(AutomataTransition_get_label(t2));

          if(AutomataLabel_is_model(l2, symbol)) {
            /* symbol is a model for labels on t1 and t2. This means
               that the two locations can move together*/
            AutomataLoc_ptr t1_dest_location;
            AutomataLoc_ptr t2_dest_location;
            AutomataLoc_ptr res_dest_location;

            AutomataLabel_ptr new_label;

            t1_dest_location = AutomataTransition_get_end_location(t1);
            t2_dest_location = AutomataTransition_get_end_location(t2);

            res_dest_location =
              automaton_intersection_find_or_create_loc(res_automaton,
                                                        location_map,
                                                        stack,
                                                        A1,
                                                        A2,
                                                        t1_dest_location,
                                                        t2_dest_location);

            /* Creates a new transition from res_location to
               res_dest_location, with symbol as label. */
            new_label =
              automata_create_label_from_expr(ABSTRACT_AUTOMATON(res_automaton)->label_type,
                                              bdd_enc,
                                              symbol);
            AutomataLoc_create_transition(res_location,
                                          res_dest_location,
                                          new_label,
                                          errmgr);
          }
        } /* end loop of s2 transitions */
      }
    } /* end loop of s1 transitions */
  } /* end loop of symbols in alphabet  */

  /* Manage automaton type and complete information */
}

/*!
  \brief Performs a single step of the intersection
   operation using the transitions method.

  Performs a single step of the intersection
   operation, performed on automata A1 and A2, using the transitions method.
   res_automaton: the result automaton of intersection operation.
   location_map: the map from couples (s1, s2), with s1 from the A1 and s2
   from A2, to a location in res_automaton.
   stack: stack that contains triples of locations and is used in the main
   intersection algorithm, automaton_intersection.
   triple: the triple (s1, s2, res_location) with s1 from A1, s2 from A2
   and res_location from res_automaton.

  \se location_map could be changed: new associations from
   couples of locations to a location in the result automaton can be created.
   stack: elements can be added to stack to allow these new elements to
   be visited by the DFS performed in the main algorithm.

  \sa automaton_intersection
*/
static void
automaton_transitions_based_step_intersection(AutomataManager_ptr manager,
                                              Automaton_ptr res_automaton,
                                              Automaton_ptr A1,
                                              Automaton_ptr A2,
                                              AutomataLocCoupleHash_ptr location_map,
                                              Slist_ptr stack,
                                              AutomataLocTriple_ptr triple)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  AutomataLoc_ptr s1;
  AutomataLoc_ptr s2;
  AutomataLoc_ptr res_location;

  Slist_ptr s1_trans;
  Siter s1_trans_iter;

  /* Test to decrease the number of transitions .
     This is a map from a couple of locations to a transition.
     This behaviour should be used in all the other opearations.
  */
  AutomataLocCoupleHash_ptr couple_hash;
  couple_hash = AutomataLocCoupleHash_create(env);
  /* test code */

  s1 = AutomataLocTriple_get_first(triple);
  s2 = AutomataLocTriple_get_second(triple);
  res_location = AutomataLocTriple_get_third(triple);

  /* Loops all transitions in out from s1 */
  s1_trans = AutomataLoc_get_transitions(s1);
  for(s1_trans_iter = Slist_first(s1_trans);
      ! Siter_is_end(s1_trans_iter);
      s1_trans_iter = Siter_next(s1_trans_iter)) {
    AutomataTransition_ptr t1;
    AutomataLabel_ptr l1;
    Slist_ptr s2_trans;
    Siter s2_trans_iter;

    t1 = AUTOMATA_TRANSITION(Siter_element(s1_trans_iter));
    l1 = AutomataTransition_get_label(t1);

    /* Loops all transitions in out from s2 */
    s2_trans = AutomataLoc_get_transitions(s2);
    for(s2_trans_iter = Slist_first(s2_trans);
        ! Siter_is_end(s2_trans_iter);
        s2_trans_iter = Siter_next(s2_trans_iter)) {
      AutomataTransition_ptr t2;
      AutomataLabel_ptr l2;
      AutomataLabel_ptr and_label;

      t2 = AUTOMATA_TRANSITION(Siter_element(s2_trans_iter));
      l2 = AutomataTransition_get_label(t2);

      and_label = AutomataLabel_intersection(l1, l2);
      if(AutomataLabel_is_sat(and_label)) {
        AutomataLoc_ptr t1_dest_location;
        AutomataLoc_ptr t2_dest_location;
        AutomataLoc_ptr res_dest_location;

        /* test code */
        AutomataTransition_ptr new_trans;
        /* test code */

        t1_dest_location = AutomataTransition_get_end_location(t1);
        t2_dest_location = AutomataTransition_get_end_location(t2);

        res_dest_location =
          automaton_intersection_find_or_create_loc(res_automaton,
                                                    location_map,
                                                    stack,
                                                    A1,
                                                    A2,
                                                    t1_dest_location,
                                                    t2_dest_location);
        /* Test code */
        new_trans = AUTOMATA_TRANSITION(AutomataLocCoupleHash_find(couple_hash,
                                                                   res_location,
                                                                   res_dest_location));
        if(new_trans == NULL) {
          /* Creates a new transition from res_location to
             res_dest_location, with and_label as label. */
          new_trans = AutomataLoc_create_transition(res_location,
                                                    res_dest_location,
                                                    and_label,
                                                    errmgr);
          AutomataLocCoupleHash_insert(couple_hash,
                                       res_location,
                                       res_dest_location,
                                       new_trans);

        }else{
          AutomataLabel_ptr new_label;
          AutomataLabel_ptr old_label;
          /* creates an OR of labels */
          old_label = AutomataTransition_get_label(new_trans);
          new_label = AutomataLabel_union(old_label, and_label);
          AutomataTransition_set_label(new_trans, new_label);
          AutomataLabel_destroy(old_label);
        }


        /* Test code */


        /*             /\* Creates a new transition from res_location to */
        /*                res_dest_location, with and_label as label. *\/ */
        /*             AutomataLoc_create_transition(res_location,  */
        /*                                             res_dest_location, */
        /*                                             and_label);           */
      }else{
        AutomataLabel_destroy(and_label);
      }

    } /* End loop for transitions of s2 */
  } /* End loop for transitions of s1 */

  /* test code */
  AutomataLocCoupleHash_destroy(couple_hash);
  /* test code */
}

/*!
  \brief Performs a single step of the fusion
   operation using the alphabet method.

  Performs a single step of the fusion
   operation, performed on automata A1 and A2, using the alphabet method.
   res_automaton: the result automaton of fusion operation.
   location_map: the map from location s1 from the A1 to a location in res_automaton.
   res_location: location in result automaton where create the transitions.
   trans: the transition from A1 that goes in a final location.
   trans_from_init_2: stack that contains all the transitions in output
   from Automaton_get_unique_init_loc(A2)

  \se location_map could be changed: new associations from
   couples of locations to a location in the result automaton can be created.

  \sa automaton_intersection
*/
static void
automaton_alphabet_based_step_fusion(AutomataManager_ptr manager,
                                     Automaton_ptr res_automaton,
                                     AutomataLocHash_ptr location_map,
                                     AutomataLoc_ptr res_location,
                                     AutomataTransition_ptr trans,
                                     Set_t trans_from_init_2_set,
                                     Set_t support_set)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  Set_t alphabet;
  Set_Iterator_t alphabet_iter;
  AutomataLabel_ptr label_1;
  BddEnc_ptr bdd_enc = AutomataManager_get_bdd_enc(manager);

  label_1 = AutomataTransition_get_label(trans);

  /* builds the alphabet from support set */
  alphabet = AutomataManager_get_alphabet(manager, support_set);

  /* loops all alphabet symbols */
  SET_FOREACH(alphabet, alphabet_iter) {
    Expr_ptr symbol;

    symbol = EXPR(Set_GetMember(alphabet, alphabet_iter));

    /* check if symbol is a model for trans */
    if(AutomataLabel_is_model(label_1, symbol)) {
      Set_Iterator_t trans_iter;

      SET_FOREACH(trans_from_init_2_set, trans_iter) {
        AutomataTransition_ptr trans_2;
        AutomataLabel_ptr label_2;

        trans_2 = AUTOMATA_TRANSITION(Set_GetMember(trans_from_init_2_set, trans_iter));
        nusmv_assert(trans_2 != NULL);
        label_2 = AUTOMATA_LABEL(AutomataTransition_get_label(trans_2));
        nusmv_assert(label_2 != NULL);

        /* Check is symbol is a model for label_2. In that case
           creates a transition from res_location to the end location of
           trans_2. The label for this transition is symbol. */
        if(AutomataLabel_is_model(label_2, symbol)) {
          AutomataLoc_ptr t2_dest_location;
          AutomataLoc_ptr dest_location;
          AutomataLabel_ptr new_label;

          t2_dest_location = AutomataTransition_get_end_location(trans_2);
          new_label = automata_create_label_from_expr(ABSTRACT_AUTOMATON(res_automaton)->label_type,
                                                      bdd_enc, symbol);

          dest_location = AutomataLocHash_find(location_map, t2_dest_location);
          /* dest_location must be create in the main function of fusion */
          nusmv_assert(dest_location != NULL);

          AutomataLoc_create_transition(res_location, dest_location, new_label, errmgr);

        }
      } /* end of loops for all trans_from_init_2_set */
    }

  } /* end for each on symbols */
}

/*!
  \brief Performs a single step of the fusion
   operation using the transitions method.

  Performs a single step of the fusion
   operation, performed on automata A1 and A2, using the transitions method.
   res_automaton: the result automaton of fusion operation.
   location_map: the map from location s1 from the A1 to a location in res_automaton.
   res_location: location in result automaton where create the transitions.
   trans: the transition from A1 that goes in a final location.
   trans_from_init_2: stack that contains all the transitions in output
   from Automaton_get_unique_init_loc(A2)

  \se location_map could be changed: new associations from
   couples of locations to a location in the result automaton can be created.

  \sa automaton_intersection
*/
static void
automaton_transitions_based_step_fusion(AutomataManager_ptr manager,
                                        Automaton_ptr res_automaton,
                                        AutomataLocHash_ptr location_map,
                                        AutomataLoc_ptr res_location,
                                        AutomataTransition_ptr trans,
                                        Slist_ptr trans_from_init_2)
{
  NuSMVEnv_ptr env;
  ErrorMgr_ptr errmgr;
  AutomataLabel_ptr label;
  Siter trans_iter;

  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  AUTOMATON_CHECK_INSTANCE(res_automaton);
  AUTOMATA_LOC_HASH_CHECK_INSTANCE(location_map);
  AUTOMATA_LOC_CHECK_INSTANCE(res_location);
  AUTOMATA_TRANSITION_CHECK_INSTANCE(trans);
  SLIST_CHECK_INSTANCE(trans_from_init_2);

  env = EnvObject_get_environment(ENV_OBJECT(manager));
  errmgr = ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  label = AutomataTransition_get_label(trans);

  /* loops all transitions in output from initial location of A2.
     Combines every transition with the transition given in input
     and, if the intersection is satisfiable creates a new transition
     from res_location.
  */
  for(trans_iter = Slist_first(trans_from_init_2);
      ! Siter_is_end(trans_iter);
      trans_iter = Siter_next(trans_iter)) {
    AutomataTransition_ptr trans_2;
    AutomataLabel_ptr label_2;
    AutomataLabel_ptr new_label;

    trans_2 = AUTOMATA_TRANSITION(Siter_element(trans_iter));
    label_2 = AUTOMATA_LABEL(AutomataTransition_get_label(trans_2));

    new_label = AutomataLabel_intersection(label, label_2);

    if(AutomataLabel_is_sat(new_label)) {
      AutomataLoc_ptr dest_location_2;
      AutomataLoc_ptr dest_location;

      dest_location_2 = AutomataTransition_get_end_location(trans_2);
      dest_location = AutomataLocHash_find(location_map, dest_location_2);
      /* dest_location must be create in the main function of fusion */
      nusmv_assert(dest_location != NULL);

      AutomataLoc_create_transition(res_location, dest_location, new_label, errmgr);
    }else{
      AutomataLabel_destroy(new_label);
    }
  }
}

/*!
  \brief Performs a single step of the determinization
   operation using the alphabet method.

  Performs a single step of the determinization
   operation, performed on automata A1 using the alphabet method.
   res_automaton: the result automaton of determinization operation.
   location_map: the map from set of locations of A1 to a location in
   res_automaton.
   stack: stack used in the main determinization algorithm. It contains
   a node_ptr, where car is location set a and cdr is a location.
   res_location: location in result automaton where create the transitions.
   location_set: the set of location represented by res_location.
   trans_set_support: the support set extracted from the output
   transitions. This is done outside, so if memoization of support is
   done in locations it can be used.

  \se location_map could be changed: new associations from
   a set of location in A1 to a location in the result automaton can be created.
   stack could be changed: if a new location is created in the resulting
   automaton then a new node that contains the new location and the location
   set of A1 is pushed on top of the stack

  \sa automaton_determinization
*/
static void
automaton_alphabet_based_step_determinization(AutomataManager_ptr manager,
                                              Automaton_ptr res_automaton,
                                              AutomataLocSetHash_ptr location_map,
                                              Slist_ptr stack,
                                              AutomataLoc_ptr res_location,
                                              AutomataLocSet_ptr location_set,
                                              Set_t location_set_support)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

  Set_t alphabet;
  Set_Iterator_t alphabet_iter;
  BddEnc_ptr bdd_enc = AutomataManager_get_bdd_enc(manager);

  /* builds the alphabet */
  alphabet = AutomataManager_get_alphabet(manager, location_set_support);

  /* loops all alphabet symbols.
     - Builds the set of locations that can be reached with symbol from
     res_location.
     - find the location in result_automaton correspondent to the
     computed destination location set
     - creates a transition with symbol as label from res_location to
     the new destination location
  */
  SET_FOREACH(alphabet, alphabet_iter) {
    Expr_ptr symbol;
    Set_Iterator_t location_set_iter;
    AutomataLocSet_ptr dest_location_set;
    AutomataLoc_ptr res_dest_location;
    AutomataLabel_ptr label;

    symbol = EXPR(Set_GetMember(alphabet, alphabet_iter));
    dest_location_set = AutomataLocSet_create(env);

    /* Builds the set of reachable locations with symbol */
    STATE_SET_FOREACH(location_set, location_set_iter){
      AutomataLoc_ptr location;
      Siter trans_iter;

      location = AutomataLocSet_get_member(location_set,location_set_iter);

      for(trans_iter = Slist_first(AutomataLoc_get_transitions(location));
          ! Siter_is_end(trans_iter);
          trans_iter = Siter_next(trans_iter)) {
        AutomataTransition_ptr trans;
        AutomataLabel_ptr label_shadowing;

        trans = AUTOMATA_TRANSITION(Siter_element(trans_iter));
        label_shadowing = AutomataTransition_get_label(trans);

        /* If automaton can move with trans with symbol add a new
           destination set */
        if(AutomataLabel_is_model(label_shadowing, symbol)) {
          AutomataLoc_ptr app_location;

          app_location = AutomataTransition_get_end_location(trans);
          AutomataLocSet_add_element(dest_location_set, app_location);
        }
      } /* end of transitions loop */
    }  /* end of loop on location set */

    /* create the transition on symbol from res_location to the
       destination location */
    res_dest_location = automaton_determinization_find_or_create_loc(nodemgr,
                                                                     res_automaton,
                                                                     location_map,
                                                                     stack,
                                                                     dest_location_set);

    label = automata_create_label_from_expr(ABSTRACT_AUTOMATON(res_automaton)->label_type,
                                            bdd_enc, symbol);
    AutomataLoc_create_transition(res_location, res_dest_location, label, errmgr);

  } /* end loop on alphabet */

  /* The location is deterministic and complete */
  AutomataLoc_set_type(res_location, DETERMINISTIC);
  AutomataLoc_set_is_complete(res_location, true);
}

/*!
  \brief Performs a single step of the determinization
   operation using the transitions method.

  Performs a single step of the determinization
   operation, performed on automata A1, using the transition method.
   res_automaton: the result automaton of determinization operation.
   location_map: the map from set of locations from A1 to a location
   in res_automaton.
   stack: stack used in the main determinization algorithm. It contains
   a node_ptr, where car is location set a and cdr is a location.
   res_location: location in result automaton where create the transitions.
   location_set: set of locations of A1 that are represented by res_location in
   the resulting automata

  \se location_map could be changed: new associations from
   a set of location in A1 to a location in the result automaton can be created.
   stack could be changed: if a new location is created in the resulting
   automaton then a new node that contains the new location and the location
   set of A1 is pushed on top of the stack

  \sa automaton_determinization
*/
static void
automaton_transitions_based_step_determinization(AutomataManager_ptr manager,
                                                 Automaton_ptr res_automaton,
                                                 AutomataLocSetHash_ptr location_map,
                                                 Slist_ptr stack,
                                                 AutomataLoc_ptr res_location,
                                                 AutomataLocSet_ptr location_set)
{
  AutomataLabelCombinator_ptr combinator;
  Set_t label_location_set;
  Set_t combinations;
  Set_Iterator_t set_iter, comb_iter;
  boolean needs_completion;
  BddEnc_ptr bdd_enc = AutomataManager_get_bdd_enc(manager);
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

  /* Builds the set made of (label, destination_location) from the
     transition set of res_location */
  label_location_set = Set_MakeEmpty();
  STATE_SET_FOREACH(location_set, set_iter) {
    AutomataLoc_ptr app_location;
    Siter trans_iter;

    app_location = AUTOMATA_LOC(AutomataLocSet_get_member(location_set, set_iter));

    for(trans_iter = Slist_first(AutomataLoc_get_transitions(app_location));
        ! Siter_is_end(trans_iter);
        trans_iter = Siter_next(trans_iter)) {
      AutomataTransition_ptr trans;
      AutomataLabel_ptr label;
      AutomataLoc_ptr dest_location;
      node_ptr node;

      trans = AUTOMATA_TRANSITION(Siter_element(trans_iter));
      label = AutomataTransition_get_label(trans);
      dest_location = AutomataTransition_get_end_location(trans);

      node = new_node(nodemgr, CONS,NODE_PTR(label), NODE_PTR(dest_location));

      label_location_set = Set_AddMember(label_location_set, (Set_Element_t) node);
    }
  }

  /* Builds the set made of pairs (label_i, S_i) such that label_i is
   * satisfiable.
   * There is a pair for each possible label build from the 2^L set,
   * where L is the set of all labels found in T.
   * For each pair, label_1 is one of the possibile label in 2^L set
   * and S is a set of locations that contain all the locations that can
   * be reached with label_i from location set S.
   */
  combinator = automata_create_combinator(ABSTRACT_AUTOMATON(res_automaton)->label_type, bdd_enc);
  combinations = AutomataLabelCombinator_combine(combinator, label_location_set);

  /* needs_completion is set false when at least an output transition
     from res_location is created.
     If needs_completion is true at the end of the algorithm then a
     transition is created to the sink location.
  */
  needs_completion = true;

  /* creates a transition from res_location for every combination. */
  SET_FOREACH(combinations, comb_iter) {
    node_ptr element;
    AutomataLabel_ptr label;
    AutomataLocSet_ptr dest_location_set;
    AutomataLoc_ptr res_dest_location;

    element = NODE_PTR(Set_GetMember(combinations, comb_iter));
    label = AUTOMATA_LABEL(car(element));
    dest_location_set = AUTOMATA_LOC_SET(cdr(element));

    /* lookup or create the destination location */
    res_dest_location = automaton_determinization_find_or_create_loc(nodemgr,
                                                                     res_automaton,
                                                                     location_map,
                                                                     stack,
                                                                     dest_location_set);

    /* creates the transition from res_location to res_dest_location */
    AutomataLoc_create_transition(res_location,
                                  res_dest_location,
                                  AutomataLabel_copy(label),
                                  errmgr);

    /* destroy label and element
       res_dest_location is not destroyed, because it is used in the stack!  */
    AutomataLabel_destroy(label);

    needs_completion = false;

    free_node(nodemgr, element);
  }

  if(needs_completion) {
    /* no output transitions were created. An output transition
       labelled with true is created from res_location to the sink location
       of res_automaton.
    */
    AutomataLabel_ptr label;

    /* lookup or create the destination location, the sink that is the
       empty location */
    AutomataLocSet_ptr dest_location_set;
    AutomataLoc_ptr res_dest_location;

    label = automata_create_label_from_expr(ABSTRACT_AUTOMATON(res_automaton)->label_type,
                                            bdd_enc, ExprMgr_true(exprs));

    dest_location_set = AutomataLocSet_create(env);

    /* Finds the location that is associated with dest_location_set.
       If the association is not present, then it is the first time
       that the sink location in res_automaton is being created.

       Note that the sink location must be visited itself, so that the
       transition from sink to sink with true is created.
    */
    res_dest_location = AutomataLocSetHash_find(location_map, dest_location_set);
    if(res_dest_location == NULL) {
      res_dest_location = automaton_determinization_find_or_create_loc(nodemgr,
                                                                       res_automaton,
                                                                       location_map,
                                                                       stack,
                                                                       dest_location_set);
    }else{
      /* If dest_location_set is already in the map, then destroy the
         instance that was just created.
         If dest_location_set is new then it will be pushed into the
         stack, so it will be freed in the DFS loop of determinization.
      */
      AutomataLocSet_destroy(dest_location_set);
    }

    AutomataLoc_create_transition(res_location, res_dest_location,
                                  label, errmgr);
  }

  /* Destroy the combination set */
  Set_ReleaseSet(combinations);

  /* Destroy the label_location_set */
  SET_FOREACH(label_location_set, set_iter) {
    node_ptr element = NODE_PTR(Set_GetMember(label_location_set, set_iter));
    free_node(nodemgr, element);
  }
  Set_ReleaseSet(label_location_set);

  AutomataLabelCombinator_destroy(combinator);
}

/*!
  \brief Finds or create a location in the
   AutomataLocCoupleHash using as keys t1_dest_location and
   t2_dest_location. The function cares of adding a new location in the
   stack in order to be visited. The location found or created is returned.

  

  \se hash: if a location is added a new mapping from
   (t1_dest_location, t2_dest_location) to the newly created location.
   stack: if a location is added then the triple of (t1_dest_location,
   t2_dest_location, res_dest_location) is added to the stack.
*/
static AutomataLoc_ptr
automaton_intersection_find_or_create_loc(Automaton_ptr res_automaton,
                                          AutomataLocCoupleHash_ptr hash,
                                          Slist_ptr stack,
                                          Automaton_ptr A1,
                                          Automaton_ptr A2,
                                          AutomataLoc_ptr t1_dest_location,
                                          AutomataLoc_ptr t2_dest_location)
{
  AutomataLoc_ptr res_dest_location;
  boolean is_sink;

  /*   is_sink = A1->sink_location == t1_dest_location &&  */
  /*     A2->sink_location == t2_dest_location; */

  is_sink = ABSTRACT_AUTOMATON(A1)->sink_location == t1_dest_location ||
      ABSTRACT_AUTOMATON(A2)->sink_location == t2_dest_location;

  if(is_sink)
    res_dest_location = ABSTRACT_AUTOMATON(res_automaton)->sink_location;
  else
    res_dest_location =
      AUTOMATA_LOC(AutomataLocCoupleHash_find(hash,
                                              t1_dest_location,
                                              t2_dest_location));

  if(res_dest_location == NULL) {
    AutomataLocTriple_ptr dest_triple;

    /* location not visited before. It needs to be created. */
    res_dest_location = AutomataLoc_create(ABSTRACT_AUTOMATON(res_automaton)->label_type,
                                           res_automaton->keep_support_set_in_location);

    AutomataLoc_set_final(res_dest_location,
                          AutomataLoc_is_final(t1_dest_location) &&
                          AutomataLoc_is_final(t2_dest_location));

    automaton_add_loc(ABSTRACT_AUTOMATON(res_automaton), res_dest_location);

    /* update sink location */
    if(is_sink) ABSTRACT_AUTOMATON(res_automaton)->sink_location = res_dest_location;

    /* add new location to the map */
    AutomataLocCoupleHash_insert(hash,
                                 t1_dest_location,
                                 t2_dest_location,
                                 res_dest_location);
    /* Force to visit (s1_dest_location, s2_dest_location, res_dest_location)
       at the next iteration in the main algorithm */
    dest_triple =
      AutomataLocTriple_create_from_locations(t1_dest_location,
                                              t2_dest_location,
                                              res_dest_location);
    Slist_push(stack, dest_triple);
  }

  return res_dest_location;
}

/*!
  \brief Finds or create a location in the
   AutomataLocSetHash using as keys dest_location_set. The function
   cares of adding a new node_ptr that contains pointer to the
   destination location set and to the new location in the stack in order to
   be visited. The location found or created is returned.

  

  \se hash: if a location is added a new mapping from
   dest_location_set to the newly created location.
   stack: if a location is added then the node_ptr that has as children
   the set of destination locations and the new location is added to the stack.
*/
static AutomataLoc_ptr
automaton_determinization_find_or_create_loc(NodeMgr_ptr nodemgr,
                                             Automaton_ptr res_automaton,
                                             AutomataLocSetHash_ptr hash,
                                             Slist_ptr stack,
                                             AutomataLocSet_ptr dest_location_set)
{
  AutomataLoc_ptr res_dest_location;
  boolean is_sink;

  /* In determinization a sink location is the empty location set */
  is_sink = AutomataLocSet_is_empty(dest_location_set);

  if(is_sink)
    res_dest_location = ABSTRACT_AUTOMATON(res_automaton)->sink_location;
  else
    res_dest_location = AutomataLocSetHash_find(hash,
                                                dest_location_set);
  if(res_dest_location == NULL) {
    Set_Iterator_t dest_location_iter;
    node_ptr couple;
    boolean is_final;

    /* location not visited before. It needs to be created. */
    res_dest_location = AutomataLoc_create(ABSTRACT_AUTOMATON(res_automaton)->label_type,
                                           res_automaton->keep_support_set_in_location);

    /* Check if at least one location is final */
    is_final = false;
    STATE_SET_FOREACH(dest_location_set, dest_location_iter){
      AutomataLoc_ptr app_location;

      app_location = AUTOMATA_LOC(AutomataLocSet_get_member(dest_location_set,
                                                            dest_location_iter));

      if(AutomataLoc_is_final(app_location)) {
        is_final = true;
        break;
      }
    }

    AutomataLoc_set_final(res_dest_location, is_final);
    automaton_add_loc(ABSTRACT_AUTOMATON(res_automaton), res_dest_location);

    /* update sink location */
    if(is_sink) ABSTRACT_AUTOMATON(res_automaton)->sink_location = res_dest_location;

    /* add new location to the map */
    AutomataLocSetHash_insert(hash,
                              dest_location_set,
                              res_dest_location);

    /* Force to visit the node that has contains res_dest_location and dest_location_set
       at the next iteration in the main algorithm */
    couple = new_node(nodemgr, CONS, NODE_PTR(dest_location_set), NODE_PTR(res_dest_location));

    Slist_push(stack, couple);
  }

  return res_dest_location;
}

/*!
  \brief Copy transition a1_trans to res_location. Returns
   the new transition.

  The transition a1_trans is copied as output
   transition for res_location.

   self map from the destination location of a1_trans to a location of resulting
   automaton is searched in the hash table. If the location is not found
   then a new location is created and a new association is inserted in the
   hash. In this case the destination location of the transition is added
   in the stack.
   Then a new transition with the same label as a1_trans is created
   from res_location to the looked up/created destination location.

  \se hash: a new association can be added to hash.
   stack: stack can be modified pushing a new location on its top.
*/
static AutomataTransition_ptr
automaton_copy_transition_to_loc(Automaton_ptr res_automaton,
                                 AutomataLoc_ptr res_location,
                                 Automaton_ptr A1,
                                 AutomataTransition_ptr a1_trans,
                                 AutomataLocHash_ptr hash,
                                 Slist_ptr stack, 
                                 ErrorMgr_ptr errmgr)
{
  AutomataLoc_ptr a1_dest_location;
  AutomataLoc_ptr res_dest_location;
  AutomataLabel_ptr a1_label;

  /* Check if a1_dest_location was visited before  */
  a1_dest_location = AutomataTransition_get_end_location(a1_trans);

  res_dest_location = AutomataLocHash_find(hash, a1_dest_location);
  if(res_dest_location == NULL) {
    /* the location has never been encountered, so we have to create it */
    res_dest_location = automaton_hash_find_or_create_loc(res_automaton,
                                                          hash,
                                                          A1,
                                                          a1_dest_location);

    /* copy dfa/nfa and complete information from the location  */
    AutomataLoc_set_type(res_dest_location,
                         AutomataLoc_get_type(a1_dest_location));
    AutomataLoc_set_is_complete(res_dest_location,
                                AutomataLoc_is_complete(a1_dest_location));

    /* the location of a1 has to be visited at the next iteration */
    Slist_push(stack, a1_dest_location);
  }

#if AUTOMATON_DEBUG
  if ((res_automaton->sink_location == res_location) &&
      (res_automaton->sink_location == res_dest_location) &&
      (AutomataLoc_get_transitions_count(res_location)>0) ) {
    StreamMgr_print_error(streams, 
                          "automaton_copy_transition_to_loc: creating unuseful transition from sink to sink!\n");
  }
#endif

  /* creates the transition */
  a1_label = AutomataTransition_get_label(a1_trans);
  return AutomataLoc_create_transition(res_location,
                                       res_dest_location,
                                       AutomataLabel_copy(a1_label),
                                       errmgr);
}

/*!
  \brief Builds the automaton that recognize the language
   made from the repetition of prop_expr 0 or n times.

  Builds the automaton that recognize the language
   made from the repetition of prop_expr 0 or n times.

   This is a more efficient implementation of kleene star for
   propositional expressions, since it does not create unuseful locations,
   needed for the general cases of kleene_star.

   prop_expr must by a propositional expression.
*/
static Automaton_ptr
automaton_propositional_kleene_star(AutomataManager_ptr manager,
                                    PslNode_ptr prop_expr)
{
  Automaton_ptr res_automaton;
  AutomataLabel_ptr label;
  BddEnc_ptr bdd_enc = AutomataManager_get_bdd_enc(manager);
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  nusmv_assert(NULL != prop_expr);
  /* must be propositional */
  nusmv_assert(PslNode_is_propositional(prop_expr));

  res_automaton = Automaton_create_empty(manager);
  label = automata_create_label_from_expr(ABSTRACT_AUTOMATON(res_automaton)->label_type,
                                          bdd_enc,
                                          prop_expr);
  if (AutomataLabel_is_sat(label)) {
    AutomataLoc_create_transition(Automaton_get_unique_init_loc(res_automaton),
                                  Automaton_get_unique_init_loc(res_automaton),
                                  label,
                                  errmgr);
  } else {
    AutomataLabel_destroy(label);
  }

  automaton_set_location_final(res_automaton,
                               Automaton_get_unique_init_loc(res_automaton),
                               true);

  return res_automaton;
}

/*!
  \brief Builds the automaton that recognize the
   universal language.

  
*/
static Automaton_ptr automaton_true_star(AutomataManager_ptr manager)
{
  Automaton_ptr true_star_aut;
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));


  true_star_aut = automaton_propositional_kleene_star(manager, ExprMgr_true(exprs));

  return true_star_aut;
}

/*!
  \brief If the flag keep_automaton_type in manager is
   true update the type of self and s

  If the flag keep_automaton_type in manager is
   true then the type in s is updated and then the type of automaton is
   set according to the type of s.

  \se Type of self and s are changed
*/
static void automaton_update_type(Automaton_ptr self,
                                  AutomataManager_ptr manager,
                                  AutomataLoc_ptr s)
{
  boolean keep_type;

  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  AUTOMATA_LOC_CHECK_INSTANCE(s);

  keep_type = AutomataManager_is_keep_automaton_type(manager);

  if(keep_type){
    /* compute the type of automaton and location */
    Automaton_Type location_type;

    /* Check the type of location */
    location_type = AutomataLoc_update_type(s);

    /* if the location is non deterministic then all the automaton
       becomes non deterministic */
    if(ABSTRACT_AUTOMATON(self)->type == DETERMINISTIC && location_type == NON_DETERMINISTIC)
      ABSTRACT_AUTOMATON(self)->type = NON_DETERMINISTIC;

  }else{
    /* set the type of automaton and location to unknown */
    ABSTRACT_AUTOMATON(self)->type = UNKNOWN;
    AutomataLoc_set_type(s, UNKNOWN);
  }
}

/*!
  \brief If the flag is_complete in manager is
   true update the is_complete flag of self and s.

  If the flag is_complete in manager is
   true then the is_complete flag in s is updated and then the
   is_complete flag of automaton self is
   set according to the flag of s

  \se is_complete flag value of self and s are changed
*/
static void automaton_update_complete(Automaton_ptr self,
                                      AutomataManager_ptr manager,
                                      AutomataLoc_ptr s)
{
  boolean keep_complete;
  BddEnc_ptr bdd_enc;

  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  AUTOMATA_LOC_CHECK_INSTANCE(s);

  bdd_enc = AutomataManager_get_bdd_enc(manager);
  keep_complete = AutomataManager_is_keep_complete(manager);

  if(keep_complete) {
    /* update the complete location of automaton */
    boolean location_changed, new_sink;
    AutomataLoc_ptr sink_location;

    /* makes location complete  */
    new_sink = ABSTRACT_AUTOMATON(self)->sink_location == NULL;
    if(new_sink) {
      sink_location = automaton_create_sink_loc(self, manager);
    }else{
      sink_location = ABSTRACT_AUTOMATON(self)->sink_location;
    }

    location_changed = AutomataLoc_update_is_complete(s,
                                                      sink_location,
                                                      bdd_enc);

    /* check if the new sink location was used. In this case it will be
       added to the automaton
    */
    if(new_sink && location_changed) {
      automaton_add_sink_loc(self, sink_location);
    }

    /* We cannot location that automaton is complete!
       But s is complete, so it does not change the automaton complete
       information to false.
    */
  }else{
    AutomataLoc_set_is_complete(s, false);
    self->is_complete = false;
  }
}

/*!
  \brief Updates the type and the is_complete flag of
   self and s.

  

  \sa automaton_update_type, automaton_update_complete
*/
static void automaton_update_type_and_complete(Automaton_ptr self,
                                               AutomataManager_ptr manager,
                                               AutomataLoc_ptr s)
{
  automaton_update_type(self, manager, s);
  automaton_update_complete(self, manager, s);
}

/*!
  \brief Updates the type and the is_complete flag of
   self according to A1 and A2.

  
*/
static void automaton_init_type_and_complete_from_aut(Automaton_ptr self,
                                                      AutomataManager_ptr manager,
                                                      Automaton_ptr A1,
                                                      Automaton_ptr A2)
{
  boolean keep_type;

  AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATON_CHECK_INSTANCE(A1);
  AUTOMATON_CHECK_INSTANCE(A2);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);

  keep_type = AutomataManager_is_keep_automaton_type(manager);

  if(keep_type) {
    /* if A1 or A2 are NON_DETERMINISTIC then either self is NON_DETERMINISTIC
       This is not precise for fusion (if the non-determinism is given
       by transitions from initial staes of  A2, and these transitions
       are not combined by A1, then self can be deterministic.
    */
    if(ABSTRACT_AUTOMATON(A1)->type == NON_DETERMINISTIC ||
       ABSTRACT_AUTOMATON(A2)->type == NON_DETERMINISTIC)
      ABSTRACT_AUTOMATON(self)->type = NON_DETERMINISTIC;
    else
      ABSTRACT_AUTOMATON(self)->type = DETERMINISTIC;
  }else{
    /* If A1 or A2 are not complete then res_auto is not complete.
       This large for union
    */
    if((!A1->is_complete) || (!A2->is_complete))
      self->is_complete = false;
    else
      self->is_complete = true;
  }
}

/*!
  \brief Creates a sink location for self.

  Creates a sink location to the automaton self. If the
   automaton already has a sink location then an assertion is violated.
*/
static AutomataLoc_ptr automaton_create_sink_loc(Automaton_ptr self,
                                                 AutomataManager_ptr manager)
{
  AutomataLoc_ptr sink_location;
  AutomataLabel_ptr label;
  BddEnc_ptr bdd_enc = AutomataManager_get_bdd_enc(manager);
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  nusmv_assert(ABSTRACT_AUTOMATON(self)->sink_location == NULL);
  sink_location = ABSTRACT_AUTOMATON(self)->create_location_from_manager(ABSTRACT_AUTOMATON(self), manager);

  label = automata_create_label_from_expr(ABSTRACT_AUTOMATON(self)->label_type,
                                          bdd_enc,
                                          ExprMgr_true(exprs));

  /* creates the self transition labelled with true on sink */
  AutomataLoc_create_transition(sink_location,
                                sink_location,
                                label,
                                errmgr);

  return sink_location;
}

/*!
  \brief Add sink_location as a the sink location for self.

  
*/
static void automaton_add_sink_loc(Automaton_ptr self,
                                   AutomataLoc_ptr sink_location)
{
  nusmv_assert(ABSTRACT_AUTOMATON(self)->sink_location == NULL);

  ABSTRACT_AUTOMATON(self)->sink_location = sink_location;
  automaton_add_loc(ABSTRACT_AUTOMATON(self), sink_location);
}

/*!
  \brief Check if multiple PSL_SERE token wraps a
   propositional expression.

  Return true if a propositional expression is
   wrapped inside multiple PSL_SERE token. This happens in expressions
   like "{{{p}}}".

   This check is important for optimization of kleene_star operator.
*/
static boolean automaton_is_wrapped_sere_propositional(PslNode_ptr sere)
{
  PslOp op;
  boolean is_propositional;

  nusmv_assert(NULL != sere);
  /* if (PSL_SERE != psl_node_get_op(sere)) { */
  /*   return false; */
  /* } */

  is_propositional = false;

  is_propositional = PslNode_is_propositional(sere);

  op = psl_node_get_op(sere);

  /* Iterates on sere expression.

     If we have the PSL_SERE then check if the left expression is
     propositional.

     Loops stop if:
     - expression is null (it should not happen)
     - we have no more a PSL_SERE token
     - we found a propositional sere
  */
  while(sere != NULL &&
        PSL_SERE == op &&
        ! is_propositional) {
    sere = psl_node_get_left(sere);

    if (sere != NULL) {
      is_propositional = PslNode_is_propositional(sere);
      op = psl_node_get_op(sere);
    }
  }

  return is_propositional;
}

/*!
  \brief Returns the wrapped propositional expression in
   a sere.

  Returns the propositional expression wrapped
   inside PSL_SERE token.

   Returns NULL if sere does not wrap a propositional expression.

  \sa automaton_is_wrapped_sere_propositional
*/
static PslNode_ptr automaton_get_wrapped_sere_propositional(PslNode_ptr sere)
{
  PslOp op;
  PslNode_ptr prop_expr;

  nusmv_assert(NULL != sere);

  prop_expr = NULL;
  op = psl_node_get_op(sere);

  if (PslNode_is_propositional(sere)) {
    prop_expr = sere;
  }
  else {
    nusmv_assert(PSL_SERE == psl_node_get_op(sere));

    /* Iterates on sere expression.

       If we have the PSL_SERE then check if the left expression is
       propositional, setting prop_expr.

       Loops stop if:
       - expression is NULL (it should not happen)
       - we have no more a PSL_SERE token
       - we found a propositional sere
    */
    while (sere != NULL &&
           PSL_SERE == op &&
           NULL == prop_expr) {

      if (sere != NULL) {
        if (PslNode_is_propositional(psl_node_get_left(sere))) {
          prop_expr = psl_node_get_left(sere);
        }
      }

      sere = psl_node_get_left(sere);
    }
  }

  return prop_expr;
}

/*!
  \brief Starting from location mark as 'reach_accepting'
   all its predecessors

  Starting from location mark as 'reach_accepting'
   all its predecessors.

   Predecessors contains all the known predecessors of a location.
   Starting from location all its predecessors are visited, marking them
   as 'can reach an accepting location'.

   The marking is then performed recursively on the predecessors
   locations.
   The search stops if a location does not have a predecessors that was
   not marked before.
   

  \se Values in reach_accepting array are changed.
*/
static void
automaton_mark_predecessors(AutomataLoc_ptr location,
                            AutomataPredecessorsList_ptr predecessors_lists,
                            boolean* reach_accepting)
{
  Slist_ptr predecessors;
  Siter iterator;


  predecessors = AutomataPredecessorsList_get_predecessor(predecessors_lists,
                                                          location);

  SLIST_FOREACH(predecessors, iterator) {
    AutomataLoc_ptr predecessor;

    predecessor = Siter_element(iterator);
    if (! reach_accepting[AutomataLoc_get_id(predecessor) - 1]) {
      /* recursive step is performed only if precesessor is not yet
         marked */
      reach_accepting[AutomataLoc_get_id(predecessor) - 1] = true;
      automaton_mark_predecessors(predecessor,
                                  predecessors_lists,
                                  reach_accepting);
    }
  }

  return;
}

/*!
  \brief Merges common transitions of automaton.

  Merges common transitions of automaton
   (ie. transitions that have the same start end end location). This
   operation should reduce the number of total transition of automaton.

  \se This operation performs side effects on
   automaton, changing internal transitions representations.
*/
static void automaton_merge_transitions(Automaton_ptr automaton)
{
  Siter iter;

  SLIST_FOREACH(ABSTRACT_AUTOMATON(automaton)->locations, iter) {
    AutomataLoc_ptr location;

    location = AUTOMATA_LOC(Siter_element(iter));
    AutomataLoc_merge_duplicates(location);
  }

  return;
}

/*!
  \brief Applies bisimulation reduction to automaton self
   returning the corresponded reduced automaton.

  Applies bisimulation reduction to automaton self
   returning the corresponded reduced automaton.
*/
static Automaton_ptr automaton_bisimulation_reduction(AutomataManager_ptr manager,
                                                      Automaton_ptr self)
{
  Automaton_ptr reduced_auto;
  AutomataSimRel_ptr sim_rel;

#if AUTOMATON_DEBUG
  StreamMgr_print_error(streams,  "Reducing automaton:\n");
  AbstractAutomaton_print_dot(self, errstream);
  StreamMgr_print_error(streams,  "\n");
#endif

  /* Compute simulation relation */
  sim_rel = automaton_compute_simrel(manager, self);

#if AUTOMATON_DEBUG
  AutomataSimRel_print(sim_rel, errstream);
#endif

  /* Reduce self using simulation relation */
  reduced_auto = automaton_reduce_using_sim_rel(manager, self, sim_rel);

  AutomataSimRel_destroy(sim_rel);

#if AUTOMATON_DEBUG
  StreamMgr_print_error(streams,  "Reduced automaton:\n");
  AbstractAutomaton_print_dot(reduced_auto, errstream);
  StreamMgr_print_error(streams,  "\n");
#endif

  return reduced_auto;
}

/*!
  \brief Computes simulation relation for self.

  Computes simulation relation for self.

   An hash map containing couples of locations of the simluation relation
   is returned.
*/
static AutomataSimRel_ptr
automaton_compute_simrel(AutomataManager_ptr manager,
                         Automaton_ptr self)
{

  AutomataSimRel_ptr sim_rel;
  boolean is_sim_rel_changed;
  AutomataPredecessorsList_ptr A_pred;

  /*
    insert (a,a) in simrel
    process locations with 0 out transitions

    while (simrel is changed)
    for b in Q
    for a in Q
    if (a != b ^ (b, a) in simrel) then
    changed = chech_bisim_couple(a,b, sim_rel, pred)


    chech_bisim_couple(a,b, sim_rel, pred)
    for t_b = (b, b')
    for t_a = (a, a')
    if (a', b') in R then
    \BigVee_a = \BigVee \or t_a

    if t_b -> \BigVee_a then
    add (a,b) to simrel
    Check (b,a)
    pred_a_b = pred_b U pred_a
    check_bisim(pred)

    sim_rel_changed = true
  */

  sim_rel = AutomataSimRel_create(ABSTRACT_AUTOMATON(self)->location_count);
  A_pred = AutomataPredecessorsList_create(ABSTRACT_AUTOMATON(self)->location_count);

  /* inserts all couples (a,a) in simrel and also creates
     bisimulation for locations with no outgoing edges */
  automaton_compute_simrel_preprocess(self, sim_rel, A_pred);

  is_sim_rel_changed = true;

  /* Fixed point until sim_rel is not stable */
  while (is_sim_rel_changed) {
    is_sim_rel_changed = automaton_compute_simrel_lists(sim_rel,
                                                        A_pred,
                                                        ABSTRACT_AUTOMATON(self)->locations);
  }

#if AUTOMATON_DEBUG
  AutomataSimRel_check_consistency(sim_rel);
#endif

  AutomataPredecessorsList_destroy(A_pred);

  return sim_rel;
}

/*!
  \brief 

  

  \se sim_rel
*/
static boolean automaton_compute_simrel_lists(AutomataSimRel_ptr sim_rel,
                                              AutomataPredecessorsList_ptr A_pred,
                                              Slist_ptr locations_list)
{
  boolean is_sim_rel_changed;
  Siter iter_b;

  is_sim_rel_changed = false;

  SLIST_FOREACH(locations_list, iter_b) {
    Siter iter_a;
    AutomataLoc_ptr location_b;
    location_b = AUTOMATA_LOC(Siter_element(iter_b));

    SLIST_FOREACH(locations_list, iter_a) {
      boolean a_sim_b;
      AutomataLoc_ptr location_a;

      location_a = AUTOMATA_LOC(Siter_element(iter_a));

      a_sim_b = true; /* suppose a simulates b */

      /* Skip the computation if:
         - location_a != location_b
         - is_final(location_a) == is_final(location_b)
         - (a,b) is not in simulation relation
      */
      if ( (location_a != location_b) |
           (AutomataLoc_is_final(location_a) == AutomataLoc_is_final(location_b)) |
           AutomataSimRel_has_key(sim_rel, location_a, location_b)) {
        Siter trans_iter_b;

        SLIST_FOREACH(AutomataLoc_get_transitions(location_b), trans_iter_b) {
          Siter trans_iter_a;
          AutomataTransition_ptr trans_b;
          AutomataLoc_ptr dest_location_b;
          AutomataLabel_ptr big_or_a;
          boolean found_true_label_a;

          trans_b = AUTOMATA_TRANSITION(Siter_element(trans_iter_b));
          dest_location_b = AutomataTransition_get_end_location(trans_b);
          big_or_a = AUTOMATA_LABEL(Nil);

          found_true_label_a = false;

          SLIST_FOREACH(AutomataLoc_get_transitions(location_a),
                        trans_iter_a) {
            AutomataTransition_ptr trans_a;
            AutomataLoc_ptr dest_location_a;
            AutomataLabel_ptr label_a;

            trans_a = AUTOMATA_TRANSITION(Siter_element(trans_iter_a));
            dest_location_a = AutomataTransition_get_end_location(trans_a);
            label_a = AutomataTransition_get_label(trans_a);

            if (AutomataSimRel_has_key(sim_rel,
                                       dest_location_a,
                                       dest_location_b)) {
              /* Optimizations: collect a transitions and perform
                 bdd or in a balanced way, instead of incremental.
              */

              if (AutomataLabel_is_true(label_a)) {
                found_true_label_a = true;
                break;
              }
              else {
                if (AUTOMATA_LABEL(Nil) == big_or_a) {
                  big_or_a = AutomataLabel_copy(label_a);
                }
                else {
                  big_or_a = AutomataLabel_union(big_or_a, label_a);
                }
              }
            }
          } /* end of a_trans loop */

          if (AUTOMATA_LABEL(Nil) != big_or_a) {
            a_sim_b = AutomataLabel_is_contained(AutomataTransition_get_label(trans_b),
                                                 big_or_a);
            AutomataLabel_destroy(big_or_a);
          } else {
            if (found_true_label_a) {
              a_sim_b = true;
            }
            else {
              a_sim_b = false;
              break; /* do not loop on remaining b trans, since location
                        a cannot simulate location_b anymore! */
            }
          }
        } /* end of b_trans loop */

        if (a_sim_b) {
          Slist_ptr pred_list;

          /* location_a simulates location_b */
          AutomataSimRel_insert(sim_rel, location_a, location_b);

          /* Try to guess if Pre(location_a) and Pre(location_b)
             simulates each other */
          pred_list = Slist_create();
          AutomataPredecessorsList_add_pred_to_list(A_pred,
                                                    location_a,
                                                    pred_list);
          AutomataPredecessorsList_add_pred_to_list(A_pred,
                                                    location_b,
                                                    pred_list);

          /* sim_rel changed, keep on building partition relation */
          is_sim_rel_changed = automaton_compute_simrel_lists(sim_rel,
                                                              A_pred,
                                                              pred_list);
          Slist_destroy(pred_list);
        }

      } /* end of early pruning if */
    } /* end of loop on a locations */
  } /* end of loop on b locations */

#if AUTOMATON_DEBUG
  StreamMgr_print_error(streams,  "Compute sim_rel\n");
  AutomataSimRel_print(sim_rel, errstream);
#endif


  return is_sim_rel_changed;
}

/*!
  \brief Preprocessing for computing simulation relation.

  Preprocessing for computing simulation relation.
   This procedure performs two step:
   - Inserts the couples (a,a) in the simulation relation.
   - Identifies and inserts in the simulation relation in the same
   equivalence class all locations that do not have outgoing transitions
   (final and non-final locations are distinguished).
*/
static void automaton_compute_simrel_preprocess(Automaton_ptr self,
                                                AutomataSimRel_ptr sim_rel,
                                                AutomataPredecessorsList_ptr A_pred)
{
  Slist_ptr locations_no_trans;
  Slist_ptr final_locations_no_trans;
  AutomataLoc_ptr non_final_rep;
  AutomataLoc_ptr final_rep;
  Siter iter;

  locations_no_trans = Slist_create();
  final_locations_no_trans = Slist_create();

  final_rep = AUTOMATA_LOC(Nil);
  non_final_rep= AUTOMATA_LOC(Nil);

  /* Finds location with no outgoing transitions */
  SLIST_FOREACH(ABSTRACT_AUTOMATON(self)->locations, iter) {
    AutomataLoc_ptr location;

    location = AUTOMATA_LOC(Siter_element(iter));
    if (Slist_is_empty(AutomataLoc_get_transitions(location))) {
      if (AutomataLoc_is_final(location)) {
        Slist_push(final_locations_no_trans, location);
        AUTOMATON_UPDATE_FINAL_REP(final_rep, location);
      }
      else {
        Slist_push(locations_no_trans, location);
        AUTOMATON_UPDATE_FINAL_REP(non_final_rep, location);
      }
    }
    else {
      AutomataSimRel_insert(sim_rel, location, location);
    }
  }

  /* Creates equivalence classes for locations with no outgoing transitions */
  if (! Slist_is_empty(locations_no_trans)) {
    AutomataSimRel_insert_bisimilar_list(sim_rel,
                                         locations_no_trans,
                                         non_final_rep);

#if defined AUTOMATON_NEW_SIM_REL && AUTOMATON_NEW_SIM_REL
    if (Slist_get_size(locations_no_trans) > 1)
      automaton_compute_simrel_lists(sim_rel,
                                     A_pred,
                                     locations_no_trans);
#endif

  }
  if (! Slist_is_empty(final_locations_no_trans)) {
    AutomataSimRel_insert_bisimilar_list(sim_rel,
                                         final_locations_no_trans,
                                         final_rep);
#if defined AUTOMATON_NEW_SIM_REL && AUTOMATON_NEW_SIM_REL
    if (Slist_get_size(final_locations_no_trans) > 1)
      automaton_compute_simrel_lists(sim_rel,
                                     A_pred,
                                     final_locations_no_trans);
#endif
  }

  Slist_destroy(locations_no_trans);
  Slist_destroy(final_locations_no_trans);
}

/*!
  \brief Reduces the automaton self using simulation relation
   sim_rel.

  Reduces the automaton self using simulation
   relation sim_rel.

   The reduction performs two steps:
   - Merge locations a and b if (a,b) \in sim_rel and (b,a) \in
   sim_rel.
   - Remove a transition t1 from a to b if there exist a transition
   t2 from a to c such that t1 -> t2 and b <= c (c simulates b).
   
*/
static Automaton_ptr
automaton_reduce_using_sim_rel(AutomataManager_ptr manager,
                               Automaton_ptr self,
                               AutomataSimRel_ptr sim_rel)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(manager));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  Automaton_ptr res_automaton;
  AutomataLocHash_ptr location_map;

  Set_t rep_set;
  Set_Iterator_t iter;

  res_automaton = Automaton_create_empty(manager);
  rep_set = AutomataSimRel_get_all_rep(sim_rel);
  location_map = AutomataLocHash_create();

#if AUTOMATON_DEBUG
  StreamMgr_print_error(streams,  "Found %d equivalence classes.\n",
                        Set_GiveCardinality(rep_set));
#endif

  /* set initial location */
  {
    AutomataLoc_ptr init_rep;

    init_rep = AutomataSimRel_get_representative(sim_rel,
                                                 Automaton_get_unique_init_loc(self));
    AutomataLocHash_insert(location_map,
                           init_rep,
                           Automaton_get_unique_init_loc(res_automaton));
  }


  SET_FOREACH(rep_set, iter) {
    AutomataLoc_ptr representative;
    AutomataLoc_ptr current_location;
    Siter trans_iter;

    representative = AUTOMATA_LOC(Set_GetMember(rep_set, iter));
    current_location =
      automaton_hash_find_or_create_loc(res_automaton,
                                        location_map,
                                        self,
                                        representative);

    SLIST_FOREACH(AutomataLoc_get_transitions(representative),
                  trans_iter) {
      AutomataLoc_ptr dest_location;
      AutomataLoc_ptr res_dest_location;
      AutomataTransition_ptr trans;

      trans = AUTOMATA_TRANSITION(Siter_element(trans_iter));
      dest_location =
        AutomataSimRel_get_representative(sim_rel,
                                          AutomataTransition_get_end_location(trans));
      res_dest_location = AutomataLocHash_find(location_map, dest_location);

      if(res_dest_location == NULL) {
        /* the location has never been encountered, so we have to create it */
        res_dest_location =
          automaton_hash_find_or_create_loc(res_automaton,
                                            location_map,
                                            self,
                                            dest_location);

        /* copy dfa/nfa and complete information from the location  */
        AutomataLoc_set_type(res_dest_location,
                             AutomataLoc_get_type(dest_location));
        AutomataLoc_set_is_complete(res_dest_location,
                                    AutomataLoc_is_complete(dest_location));
      }
      AutomataLoc_create_transition(current_location,
                                    res_dest_location,
                                    AutomataLabel_copy(AutomataTransition_get_label(trans)),
                                    errmgr);


    } /* end for each transition of representative */

    /* Check to avoid duplicate transitions */
    AutomataLoc_merge_duplicates(current_location);

  } /* end for each representative */

#if AUTOMATON_DEBUG
  nusmv_assert(Set_GiveCardinality(rep_set) == ABSTRACT_AUTOMATON(res_automaton)->location_count);
#endif


  /* Free location map and stack */
  AutomataLocHash_destroy(location_map);
  Set_ReleaseSet(rep_set);

  return res_automaton;
}

/*!
  \brief Print in xmi format all the transitions and their labels

  This is an auxiliary function of
   automaton_print_xmi
*/
static enum st_retval print_all_trans(char* trans,
                                      char* id,
                                      char* arg)
{
  NuSMVEnv_ptr env;
  ExprMgr_ptr exprs;
  AutomataLoc_ptr end_location;
  AutomataLoc_ptr src_location;
  int end_location_id;
  int src_location_id;
  char* end_location_str;
  char* src_location_str;
  char* id_str;
  FILE* out_file;
  hash_ptr trans2source;
  Expr_ptr expr;
  unsigned int indent_level;
  void** data = (void**)arg;
  MasterPrinter_ptr wffprint;
  int c;

  end_location_str = ALLOC(char, ID_SIZE);
  src_location_str = ALLOC(char, ID_SIZE);
  id_str = ALLOC(char, ID_SIZE);

  c = snprintf(id_str, ID_SIZE, "_t%d", PTR_TO_INT(id));
  SNPRINTF_CHECK(c, ID_SIZE);

  /* retrieve arguments */

  env = data[0];
  out_file = (FILE*)data[1];
  trans2source = (hash_ptr)data[2];
  indent_level = (unsigned int)PTR_TO_INT(data[3]);

  exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));
  wffprint = MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));

  /* compute end_location_id */
  end_location = AutomataTransition_get_end_location(AUTOMATA_TRANSITION(trans));
  end_location_id = AutomataLoc_get_id(end_location);
  c = snprintf(end_location_str, ID_SIZE, "_s%d", end_location_id);
  SNPRINTF_CHECK(c, ID_SIZE);


  /* compute src_location_id */
  src_location = AUTOMATA_LOC(find_assoc(trans2source,
                                         NODE_PTR(trans)));
  src_location_id = AutomataLoc_get_id(src_location);
  c = snprintf(src_location_str, ID_SIZE, "_s%d", src_location_id);
  SNPRINTF_CHECK(c, ID_SIZE);

  expr = AutomataLabel_get_expr(AutomataTransition_get_label(
                                                             AUTOMATA_TRANSITION(trans)));

  if (ExprMgr_is_true(exprs, expr)) {
    indent_level =
      automaton_start_tag(out_file, "transition", EMPTY_TAG,
                          indent_level, 5,
                          "xmi:type", "uml:Transition",
                          "xmi:id", id_str,
                          "kind", "local",
                          "target", end_location_str,
                          "source", src_location_str);
  }
  else {
    char* expr_str;
    char* trigger_id;

    trigger_id = ALLOC(char, ID_SIZE);

    expr_str = sprint_node(wffprint, NODE_PTR(expr));
    c = snprintf(trigger_id, ID_SIZE, "_T%d", PTR_TO_INT(id));
    SNPRINTF_CHECK(c, ID_SIZE);

    indent_level =
      automaton_start_tag(out_file, "transition", START_TAG,
                          indent_level, 5,
                          "xmi:type", "uml:Transition",
                          "xmi:id", id_str,
                          "kind", "local",
                          "target", end_location_str,
                          "source", src_location_str);

    indent_level =
      automaton_start_tag(out_file, "trigger", EMPTY_TAG,
                          indent_level, 3,
                          "xmi:type", "uml:Trigger",
                          "xmi:id", trigger_id,
                          "name", expr_str);

    indent_level =
      automaton_end_tag(out_file, "transition", indent_level);

    FREE(expr_str);
    FREE(trigger_id);
  }

  FREE(end_location_str);
  FREE(src_location_str);
  FREE(id_str);

  return ASSOC_CONTINUE;
}

/*!
  \brief Print indented space

  Print indented space

  \sa utils_io.indent
*/
static void print_indent(FILE* file, unsigned int indent_level)
{
  unsigned int i;

  for(i=0; i < indent_level; i++) fprintf(file, "%s", INDENT_STRING);
}

/*!
  \brief Print start or empty tag and its attributes

  Print start or empty tag and its attributes.  A boolean
   flag is included to distiguish start and empty tags.  The number
   of attributes should define the number attribute-value pairs
   passed as a variable argument list.  Every attribute must
   have a value.
*/
static unsigned int automaton_start_tag(FILE* file,
                                        char* tag_name,
                                        boolean is_empty_tag,
                                        unsigned int indent_level,
                                        int num_attrs,
                                        ...)
{
  int i;
  va_list attrs;

  fprintf(file, "\n");
  print_indent(file, indent_level);
  fprintf(file, "<%s", tag_name);

  va_start(attrs, num_attrs);
  for(i = 0; i < num_attrs; i++) {
    fprintf(file, " %s=\"", va_arg(attrs, char*));
    Utils_str_escape_xml_file(va_arg(attrs, char*), file);
    fprintf(file, "\"");
  }
  va_end(attrs);
  if(is_empty_tag) {
    fprintf(file, " /");
  }
  fprintf(file, ">");

  if (! is_empty_tag) ++indent_level;

  return indent_level;
}

/*!
  \brief Print end tag

  Print end tag
*/
static unsigned int automaton_end_tag(FILE* file, char* tag_name,
                                      unsigned int indent_level)
{
  fprintf(file, "\n");
  print_indent(file, --indent_level);
  fprintf(file, "</%s>", tag_name);

  return indent_level;
}

/**AutomaticEnd***************************************************************/

