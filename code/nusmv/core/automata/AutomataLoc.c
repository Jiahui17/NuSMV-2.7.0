/* ---------------------------------------------------------------------------

   This file is part of the ``automata'' package.
   %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'AutomataLoc'

  \todo: Missing description

*/

#include "nusmv/core/automata/AutomataLoc.h"
#include "nusmv/core/automata/AutomataTransition.h"
#include "nusmv/core/automata/automata_label_creator.h"

#include "nusmv/core/automata/utils/automata_error.h"

#include "nusmv/core/utils/utils.h"
#include "nusmv/core/utils/Slist.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
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

static void automata_loc_finalize(Object_ptr object, void* dummy);

static Set_t automata_loc_compute_support_set(AutomataLoc_ptr self);

static boolean check_is_deterministic(AutomataLoc_ptr self);
static boolean check_is_complete(AutomataLoc_ptr self,
                                 AutomataLoc_ptr sink,
                                 BddEnc_ptr bdd_enc);
static Expr_ptr
automata_loc_fold_left_transitions(AutomataLoc_ptr self,
ExprMgr_ptr exprs,
Expr_ptr (*binary_function) (ExprMgr_ptr, Expr_ptr, Expr_ptr));

static void
automata_loc_copy_transition(AutomataLoc_ptr self,
                             AutomataTransition_ptr transition,
                             ErrorMgr_ptr errmgr);

static void automata_loc_reset_transitions(AutomataLoc_ptr self);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataLoc_ptr AutomataLoc_create(Automaton_Label_Type label_type,
                                   boolean keep_support_location)
{
  AutomataLoc_ptr self = ALLOC(AutomataLoc, 1);
  AUTOMATA_LOC_CHECK_INSTANCE(self);

  automata_loc_init(self, label_type, keep_support_location);
  return self;
}

AutomataLoc_ptr AutomataLoc_create_from_manager(AutomataManager_ptr manager)
{
  AutomataLoc_ptr self;

  self = AutomataLoc_create(AutomataManager_get_label_type(manager),
                            AutomataManager_is_keep_support_set_in_location(manager));
  AUTOMATA_LOC_CHECK_INSTANCE(self);

  return self;
}

void AutomataLoc_destroy(AutomataLoc_ptr self)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

int AutomataLoc_get_id (AutomataLoc_ptr self)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);

  return self->id;
}

void AutomataLoc_set_id (AutomataLoc_ptr self, int id)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);
  nusmv_assert(id >= 0);

  self->id = id;
}

boolean AutomataLoc_is_final(AutomataLoc_ptr self)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);

  return self->final;
}

void AutomataLoc_set_final(AutomataLoc_ptr self, boolean final)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);

  self->final = final;
}

Automaton_Type AutomataLoc_get_type(AutomataLoc_ptr self)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);

  return self->type;
}

void AutomataLoc_set_type(AutomataLoc_ptr self, const Automaton_Type new_type)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);

  self->type = new_type;
}

boolean AutomataLoc_is_complete(AutomataLoc_ptr self)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);

  return self->is_complete;
}

void AutomataLoc_set_is_complete(AutomataLoc_ptr self, boolean is_complete)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);

  self->is_complete = is_complete;
}

AutomataTransition_ptr AutomataLoc_create_transition(AutomataLoc_ptr self,
                                                     AutomataLoc_ptr end_location,
                                                     AutomataLabel_ptr label,
                                                     ErrorMgr_ptr errmgr)
{
  AutomataTransition_ptr transition;

  AUTOMATA_LOC_CHECK_INSTANCE(self);
  nusmv_assert(end_location != NULL);
  nusmv_assert(label != NULL);
  nusmv_assert(errmgr != NULL);

  /* If the used label type is different from the label type of the
     location then an error is raised  */
  if(self->label_type != AutomataLabel_get_label_type(label)) {
    automata_error_wrong_label_type(errmgr, self->label_type,
                                    AutomataLabel_get_label_type(label));
  }

  transition =
    AutomataTransition_create_from_location_label(end_location, label);

  /* Check if updates the support location */
  if(self->keep_support_set){
    Set_t label_support_set = AutomataLabel_get_support(label);
    self->support_set = Set_Union(self->support_set, label_support_set);
  }

  Slist_push(self->transitions_list, transition);
  self->transitions_count = self->transitions_count + 1;

  return transition;
}

AutomataLoc_ptr AutomataLoc_copy(AutomataLoc_ptr self,
                                 boolean keep_final_info,
                                 Automaton_Label_Type label_type,
                                 boolean keep_support_set)
{
  AutomataLoc_ptr copy_location;

  AUTOMATA_LOC_CHECK_INSTANCE(self);
  copy_location = AutomataLoc_create(label_type, keep_support_set);

  if (keep_final_info) {
    copy_location->final = self->final;
  }

  /* Copies the options */
  copy_location->label_type = label_type;
  copy_location->keep_support_set = keep_support_set;

  return copy_location;
}

int AutomataLoc_get_transitions_count(AutomataLoc_ptr self)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);

  return self->transitions_count;
}

Slist_ptr AutomataLoc_get_transitions(AutomataLoc_ptr self)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);

  return self->transitions_list;
}

void AutomataLoc_reset_transitions(AutomataLoc_ptr self)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);

  automata_loc_reset_transitions(self);

  return;
}

void AutomataLoc_merge_duplicates(AutomataLoc_ptr self)
{
  hash_ptr hash_map;
  Slist_ptr new_trans_list;
  int count;

  AUTOMATA_LOC_CHECK_INSTANCE(self);

  new_trans_list = Slist_create();
  count = 0;

  hash_map = new_assoc();
  while (! Slist_is_empty(self->transitions_list)) {
    AutomataLoc_ptr dest_location;
    AutomataTransition_ptr hash_trans;
    AutomataTransition_ptr trans =
      AUTOMATA_TRANSITION(Slist_pop(self->transitions_list));

    dest_location = AutomataTransition_get_end_location(trans);
    hash_trans= AUTOMATA_TRANSITION(find_assoc(hash_map,
                                               NODE_PTR(dest_location)));
    if (AUTOMATA_TRANSITION(Nil) == hash_trans) {
      Slist_push(new_trans_list, trans);
      insert_assoc(hash_map, NODE_PTR(dest_location),
                   NODE_PTR(trans));
      count++;
    }
    else {
      AutomataLabel_ptr new_label;
      AutomataLabel_ptr hash_label;

      hash_label = AutomataTransition_get_label(hash_trans);
      new_label = AutomataLabel_union(hash_label,
                                      AutomataTransition_get_label(trans));
      AutomataTransition_set_label(hash_trans, new_label);
      AutomataLabel_destroy(hash_label);
      AutomataTransition_destroy(trans);
    }
  }

  Slist_destroy(self->transitions_list);
  self->transitions_list = new_trans_list;
  self->transitions_count = count;

  free_assoc(hash_map);
}

Automaton_Type AutomataLoc_update_type (AutomataLoc_ptr self)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);

  if(check_is_deterministic(self)) {
    self->type = DETERMINISTIC;
  }else{
    self->type = NON_DETERMINISTIC;
  }

  return self->type;
}

boolean AutomataLoc_update_is_complete (AutomataLoc_ptr self,
                                        AutomataLoc_ptr sink,
                                        BddEnc_ptr bdd_enc)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);

  return check_is_complete(self, sink, bdd_enc);
}

void AutomataLoc_print(AutomataLoc_ptr self,
                       NuSMVEnv_ptr env,
                       FILE* stream)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);
  nusmv_assert(stream!=NULL);

  fprintf(stream, "STATE %d\n", self->id);

  if(self->final) {
    fprintf(stream, "FINAL STATE\n");
  }

  if(Slist_is_empty(self->transitions_list)) {
    fprintf(stream, "NO OUT TRANS\n");
  } else {
    Siter iterator;

    fprintf(stream, "START TRANS: \n");
    //  nusmv_assert(self->transitions_list != NULL);

    /* Prints all the transitions of the location */
    for(iterator = Slist_first(self->transitions_list);
        ! Siter_is_end(iterator);
        iterator = Siter_next(iterator))
      {
        AutomataTransition_ptr transition = AUTOMATA_TRANSITION(Siter_element(iterator));
        AutomataLoc_ptr end_location = AutomataTransition_get_end_location(transition);
        AutomataLabel_ptr label;

        nusmv_assert(end_location != NULL);
        fprintf(stream, "End location: %d - label: ", end_location->id);

        /* Prints label */
        label = AutomataTransition_get_label(transition);
        AutomataLabel_print_label(label, env, stream);
        fprintf(stream, "\n");
      }
    fprintf(stream, "END TRANS\n");
  }
}

void AutomataLoc_print_dot(AutomataLoc_ptr self,
                           NuSMVEnv_ptr env,
                           FILE* stream)
{
  AUTOMATA_LOC_CHECK_INSTANCE(self);
  nusmv_assert(stream!=NULL);

  if(self->final)
    fprintf(stream, "node%d [shape = doublecircle] [label = \"%d\"]\n", self->id, self->id);
  else
    fprintf(stream, "node%d [shape = circle] [label = \"%d\"]\n", self->id, self->id);

  if(!Slist_is_empty(self->transitions_list)) {
    /* Prints all the transitions of the location */
    Siter iterator;
    for(iterator = Slist_first(self->transitions_list);
        ! Siter_is_end(iterator);
        iterator = Siter_next(iterator))
      {
        AutomataTransition_ptr transition = AUTOMATA_TRANSITION(Siter_element(iterator));
        AutomataLoc_ptr end_location = AutomataTransition_get_end_location(transition);
        AutomataLabel_ptr label;

        nusmv_assert(end_location != NULL);
        fprintf(stream, "\"node%d\" -> \"node%d\" [label = \"", self->id, end_location->id);

        /* Prints label */
        label = AutomataTransition_get_label(transition);
        AutomataLabel_print_label(label, env, stream);
        fprintf(stream, "\"]\n");
      }
  }
}

Set_t AutomataLoc_get_support_set (AutomataLoc_ptr self)
{
  if(self->keep_support_set) {
    return self->support_set;
  }else{
    self->support_set = automata_loc_compute_support_set(self);
    return self->support_set;
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The Automaton class virtual finalizer

  Called by the class destructor
*/
static void automata_loc_finalize(Object_ptr object, void* dummy)
{
  AutomataLoc_ptr self = AUTOMATA_LOC(object);

  automata_loc_deinit(self);
  FREE(self);
}

void automata_loc_init(AutomataLoc_ptr self,
                       Automaton_Label_Type label_type,
                       boolean keep_support_location)
{
  /* base class initialization */
  object_init(OBJECT(self));

  /* members initialization */
  self->id = -1; /* the initial id is negative, so it is possible to
                    know if a valid id is assigned to the location. */
  self->final = false;
  self->type = DETERMINISTIC;
  self->label_type = label_type;
  self->is_complete = false;
  self->transitions_count = 0;
  self->transitions_list = Slist_create();
  self->keep_support_set = keep_support_location;
  if(self->keep_support_set)
    self->support_set = Set_MakeEmpty();
  self->support_set = NULL;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = automata_loc_finalize;
}

void automata_loc_deinit(AutomataLoc_ptr self)
{
  /* members deinitialization */

  /* destroy transitions of the location */
  automata_loc_reset_transitions(self);

  Slist_destroy(self->transitions_list);

  /* Check if it is the case to destroy the support_set */
  if(self->support_set != NULL){
    Set_ReleaseSet(self->support_set);
  }
}

/*!
  \brief Builds the support set for the current location

  The support set for a location is the union of the
   support sets of each output transition of location. The caller is
   responsible for destroying the returned set.
*/

static Set_t automata_loc_compute_support_set(AutomataLoc_ptr self) {
  Set_t support_set;
  Siter trans_iter;

  support_set = Set_MakeEmpty();
  for(trans_iter = Slist_first(self->transitions_list);
      ! Siter_is_end(trans_iter);
      trans_iter = Siter_next(trans_iter)) {

    AutomataTransition_ptr transition =
      AUTOMATA_TRANSITION(Siter_element(trans_iter));
    AutomataLabel_ptr label =
      AUTOMATA_LABEL(AutomataTransition_get_label(transition));

    support_set = Set_Union(support_set, AutomataLabel_get_support(label));
  }

  return support_set;
}

/*!
  \brief Checks if the location is deterministic

  Returns true if the location is deterministic. A
   location is deterministic if there are no transitions that exists from
   the location that have the same symbol. Having a symbolic
   representation of labels on transitions we have to check for every
   pair of transitions if the intersection of two labels is satisfiable.
*/
static boolean check_is_deterministic(AutomataLoc_ptr self)
{
  /* Checks every transition pairwise, not considering the pair that
     has as elements the same transition.
     If t1,t2,...tn are the transitions that start from the location, and
     l1,l2,...ln are the labels of the transitions, the algorithm checks
     SAT(l1 and l2), SAT(l1 and l3), ... SAT(l1 and ln), then
     at the second iteration of the external loop checks SAT(l2 and l3),
     SAT(l2 and l4), ... SAT(l2 and ln) until all the possible transitions
     have been compared.

     The iteration is interrupted if one of these conjunctions is SAT,
     so the location is non-deterministic.
  */

  Siter first_iter;
  for(first_iter = Slist_first(self->transitions_list);
      ! Siter_is_end(first_iter);
      first_iter = Siter_next(first_iter) )
    {

      AutomataTransition_ptr first_transition =
        AUTOMATA_TRANSITION(Siter_element(first_iter));

      AutomataLabel_ptr first_label = AutomataTransition_get_label(first_transition);

      Siter second_iter;
      for(second_iter = Siter_next(first_iter);
          ! Siter_is_end(second_iter);
          second_iter = Siter_next(second_iter) ) {
        AutomataTransition_ptr second_transition =
          AUTOMATA_TRANSITION(Siter_element(second_iter));

        AutomataLabel_ptr second_label =
          AutomataTransition_get_label(second_transition);

        /* builds the intersection of the two labels and checks for SAT */
        AutomataLabel_ptr intersection_label =
          AutomataLabel_intersection(first_label, second_label);
        if(AutomataLabel_is_sat(intersection_label)) {
          /* Two transitions move with the same assigment, so location is
             non-deterministic */
          return false;
        }

      }
    }

  /* No transitions move with the same assigmnet, so location is
     deterministic */
  return true;
}

/*!
  \brief Checks if a location is complete and complete it
   adding transitions with true label to sink location.

  Returns true if self is changed, so if at least
   one transition is created to the sink location. A location is
   complete if it has an out transition for every symbol of the
   alphabet. We check if the negation of the disjunction of all the
   labels is a satisfiable formula. If this happens then it means that
   there exists at least one truth assigment that is not considered by
   any transition, so the location is not complete.

   The other options is that the location is the sink location.

   The BddEnc can be null if the type of self is not BDD_LABEL
*/
static boolean check_is_complete(AutomataLoc_ptr self,
                                 AutomataLoc_ptr sink,
                                 BddEnc_ptr bdd_enc)
{
  boolean completed = false;
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(bdd_enc));
  const ExprMgr_ptr exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  /* If the location does not have transitions in output and it is not
     the sink location then it is not complete.
  */
  if(self->transitions_count<=0) {
    if(self != sink) {

      /* Creates the transition to the sink location */
      AutomataLabel_ptr app_label = NULL;
      Expr_ptr expression = NULL;

      /* Create a transition labelled with true */
      expression = ExprMgr_true(exprs);
      app_label = automata_create_label_from_expr(self->label_type,
                                                  bdd_enc,
                                                  expression);

      AutomataLoc_create_transition(self, sink, app_label, errmgr);

      completed = true;
    }
  }else{
    AutomataLabel_ptr app_label = NULL;
    Expr_ptr expression = NULL;

    /* Builds the disjunction of the labels on the output
       transitions of self.
       If li is the the label for transition i, for i=1 to
       transitions_count, then expression = l1 | l2 | ... | ln
    */
    expression = automata_loc_fold_left_transitions(self, exprs, ExprMgr_or);

    /* negates the disjunction of all the labels
       ! (l1 | l2 | ... | ln)
    */
    expression = ExprMgr_not(exprs, expression);

    app_label = automata_create_label_from_expr(self->label_type,
                                                bdd_enc,
                                                expression);
    if(AutomataLabel_is_sat(app_label)) {
      /* Creates the transition to the sink location */
      AUTOMATA_LOC_CHECK_INSTANCE(sink);
      AutomataLoc_create_transition(self, sink, app_label, errmgr);

      completed = true;
    }else{
      AutomataLabel_destroy(app_label);
    }
  }

  self->is_complete = true;
  return completed;
}

/*!
  \brief Performs the fold left on output transitions of
   location with binary_function operator.

  
*/
static Expr_ptr
automata_loc_fold_left_transitions(AutomataLoc_ptr self,
                                   ExprMgr_ptr exprs,
                                   Expr_ptr (*binary_function) (ExprMgr_ptr, Expr_ptr, Expr_ptr))
{
  Siter first_iter;
  Expr_ptr expression = NULL;

  AUTOMATA_LOC_CHECK_INSTANCE(self);

  /* Builds the conjunction of all the labels */
  for(first_iter = Slist_first(self->transitions_list);
      ! Siter_is_end(first_iter);
      first_iter = Siter_next(first_iter) ) {

    AutomataTransition_ptr transition =
      AUTOMATA_TRANSITION(Siter_element(first_iter));
    AutomataLabel_ptr label;
    Expr_ptr label_expr;

    AUTOMATA_TRANSITION_CHECK_INSTANCE(transition);
    AUTOMATA_LABEL_CHECK_INSTANCE(transition);

    label = AUTOMATA_LABEL(AutomataTransition_get_label(transition));
    label_expr = AutomataLabel_get_expr(label);
    EXPR_CHECK_INSTANCE(label_expr);

    if(expression == NULL)
      expression = label_expr;
    else
      expression = (*binary_function)(exprs, expression, label_expr);
  }

  return expression;
}

/*!
  \brief Creates a new transition from location self to the
   end location of transition, using as label a copy of the label on the
   transition

  
*/
static void automata_loc_copy_transition(AutomataLoc_ptr self,
                                         AutomataTransition_ptr transition,
                                         ErrorMgr_ptr errmgr)
{
  AutomataLabel_ptr label;
  AutomataLoc_ptr dest_location;

  AUTOMATA_LOC_CHECK_INSTANCE(self);
  AUTOMATA_TRANSITION_CHECK_INSTANCE(self);

  label = AutomataTransition_get_label(transition);
  dest_location = AutomataTransition_get_end_location(transition);
  AutomataLoc_create_transition(self, dest_location, AutomataLabel_copy(label), errmgr);
}

/*!
  \brief Reset all the out transitions of the location.

  Reset all the out transitions of the location.

  \se The internal representation of out transition is
   deleted
*/
static void automata_loc_reset_transitions(AutomataLoc_ptr self)
{
  /* Loops all the transitions destroying them.
     At the end the stack that stores transitions is destroyed
  */

  while(! Slist_is_empty(self->transitions_list)) {
    AutomataTransition_ptr transition =
      AUTOMATA_TRANSITION(Slist_pop(self->transitions_list));

    AutomataTransition_destroy(transition);
  }
}

/**AutomaticEnd***************************************************************/

