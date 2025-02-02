/* ---------------------------------------------------------------------------

  This file is part of the ``automata'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'AutomataAlgoChooser'

  \todo: Missing description

*/

#include "nusmv/core/utils/StreamMgr.h"
#include "nusmv/core/automata/AutomataAlgoChooser.h" 
#include "nusmv/core/automata/labels/AutomataLabel.h"
#include "nusmv/core/utils/utils.h" 

#include "math.h" /* needed for pow */


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct AutomataAlgoChooser_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  boolean has_permanent_data;
  int permanent_transition_count;
  Set_t permanent_support;
  Set_t last_support;

} AutomataAlgoChooser;



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

static void automata_algo_chooser_init(AutomataAlgoChooser_ptr self);
static void automata_algo_chooser_deinit(AutomataAlgoChooser_ptr self);

static boolean 
automata_algo_chooser_use_heuristic(AutomataManager_ptr manager);

static boolean 
automata_algo_chooser_needs_support(AutomataManager_ptr manager);

static Automaton_Algorithm_Type 
automata_algo_compute_heuristic(Set_t AP, int transitions_count);

static Set_t 
automata_algo_chooser_build_set_support(AutomataLocSet_ptr location_set);

static int 
automata_algo_chooser_count_trans(AutomataLocSet_ptr location_set);

static Set_t 
automata_algo_union_permanent_support(AutomataAlgoChooser_ptr self,
                                                   Set_t support_set);

static int 
automata_algo_sum_perm_trans_count(AutomataAlgoChooser_ptr self,
                                              int transition_count);

static void 
automata_algo_get_fresh_support_set(AutomataAlgoChooser_ptr self);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataAlgoChooser_ptr AutomataAlgoChooser_create()
{
  AutomataAlgoChooser_ptr self = ALLOC(AutomataAlgoChooser, 1);
  AUTOMATA_ALGO_CHOOSER_CHECK_INSTANCE(self);

  automata_algo_chooser_init(self);
  return self;
}

void AutomataAlgoChooser_destroy(AutomataAlgoChooser_ptr self)
{
  AUTOMATA_ALGO_CHOOSER_CHECK_INSTANCE(self);

  automata_algo_chooser_deinit(self);
  FREE(self);
}

void AutomataAlgoChooser_set_permanent_location (AutomataAlgoChooser_ptr self,
                                               AutomataManager_ptr manager,
                                               AutomataLoc_ptr permanent_location)
{
  AUTOMATA_ALGO_CHOOSER_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  AUTOMATA_LOC_CHECK_INSTANCE(permanent_location);

  if(automata_algo_chooser_needs_support(manager)) {
    self->has_permanent_data = true;
    self->permanent_support = AutomataLoc_get_support_set(permanent_location);

    /* compute the number of transitions in permanent location */
    self->permanent_transition_count =  
      AutomataLoc_get_transitions_count(permanent_location);
  }
}

Automaton_Algorithm_Type
AutomataAlgoChooser_choose_from_location_couple(AutomataAlgoChooser_ptr self,
                                             AutomataManager_ptr manager,
                                             AutomataLoc_ptr s1,
                                             AutomataLoc_ptr s2)
{
  Automaton_Algorithm_Type algorithm_type;

  AUTOMATA_ALGO_CHOOSER_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  AUTOMATA_LOC_CHECK_INSTANCE(s1);
  AUTOMATA_LOC_CHECK_INSTANCE(s2);

/*   StreamMgr_print_error(streams,  "AutomataAlgoChooser_choose_from_location_couple\n"); */
/*   StreamMgr_print_error(streams,  "S1\n"); */
/*   AutomataLoc_print(s1, errstream); */
/*   StreamMgr_print_error(streams,  "S2\n"); */
/*   AutomataLoc_print(s2, errstream); */
/*   StreamMgr_print_error(streams,  "\n"); */

  /* check if support is needed */
  if(automata_algo_chooser_needs_support(manager)) {
    /* compute support set */
    automata_algo_get_fresh_support_set(self);

    self->last_support = Set_Union(self->last_support,
                                   AutomataLoc_get_support_set(s1));
    self->last_support = Set_Union(self->last_support,
                                   AutomataLoc_get_support_set(s2));
    
    self->last_support = 
      automata_algo_union_permanent_support(self, self->last_support);
  }

  /* check if heuristic has to be computed */
  if(automata_algo_chooser_use_heuristic(manager)) {
    /* compute number of transitions */
    int transitions_count;

    transitions_count = 
      automata_algo_sum_perm_trans_count(self,
                                         AutomataLoc_get_transitions_count(s1) +
                                         AutomataLoc_get_transitions_count(s2));

    algorithm_type = automata_algo_compute_heuristic(self->last_support,
                                                     transitions_count);
  }else{
    algorithm_type = AutomataManager_get_algorithm_type(manager);
  }

  return algorithm_type;
}

Automaton_Algorithm_Type 
AutomataAlgoChooser_choose_from_trans (AutomataAlgoChooser_ptr self,
                                       AutomataManager_ptr manager,
                                       AutomataTransition_ptr trans)
{
  Automaton_Algorithm_Type algorithm_type;

  AUTOMATA_ALGO_CHOOSER_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  AUTOMATA_TRANSITION_CHECK_INSTANCE(trans);

  /* check if support is needed */
  if(automata_algo_chooser_needs_support(manager)) {
    /* compute support set using transition */
    AutomataLabel_ptr label;
    
    label = AutomataTransition_get_label(trans);
    automata_algo_get_fresh_support_set(self);
    self->last_support = Set_Union(self->last_support, 
                                   AutomataLabel_get_support(label));

    self->last_support = 
      automata_algo_union_permanent_support(self, self->last_support);
  }

  /* check if heuristic has to be computed */
  if(automata_algo_chooser_use_heuristic(manager)) {
    /* compute number of transitions */
    int transitions_count;

    /* We increment by one, the current transition, the permanent transition
       count */
    transitions_count = automata_algo_sum_perm_trans_count(self, 1); 

    algorithm_type = automata_algo_compute_heuristic(self->last_support,
                                                     transitions_count);
  }else{
    algorithm_type = AutomataManager_get_algorithm_type(manager);
  }

  return algorithm_type;

}

Automaton_Algorithm_Type 
AutomataAlgoChooser_choose_from_location_set(AutomataAlgoChooser_ptr self,
                                          AutomataManager_ptr manager,
                                          AutomataLocSet_ptr location_set)
{
  Automaton_Algorithm_Type algorithm_type;

  AUTOMATA_ALGO_CHOOSER_CHECK_INSTANCE(self);
  AUTOMATA_MANAGER_CHECK_INSTANCE(manager);
  nusmv_assert(location_set != NULL);

  /* check if support is needed */
  if(automata_algo_chooser_needs_support(manager)) {    
    self->last_support = automata_algo_chooser_build_set_support(location_set);

    self->last_support = 
      automata_algo_union_permanent_support(self, self->last_support);
  }

  /* check if heuristic has to be computed */
  if(automata_algo_chooser_use_heuristic(manager)) {
    /* compute number of transitions */
    int transitions_count;

    transitions_count = automata_algo_chooser_count_trans(location_set);
    transitions_count = automata_algo_sum_perm_trans_count(self, 
                                                           transitions_count);

    algorithm_type = automata_algo_compute_heuristic(self->last_support,
                                                     transitions_count);
  }else{
    algorithm_type = AutomataManager_get_algorithm_type(manager);
  }    
  
  return algorithm_type;
}

Set_t AutomataAlgoChooser_get_last_support (AutomataAlgoChooser_ptr self)
{
  return self->last_support;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataAlgoChooser class private initializer

  The AutomataAlgoChooser class private initializer

  \sa AutomataAlgoChooser_create
*/
static void automata_algo_chooser_init(AutomataAlgoChooser_ptr self)
{
  /* members initialization */
  self->has_permanent_data = false;
  self->permanent_support = NULL;
  self->last_support = NULL;
}

/*!
  \brief The AutomataAlgoChooser class private deinitializer

  The AutomataAlgoChooser class private deinitializer

  \sa AutomataAlgoChooser_destroy
*/
static void automata_algo_chooser_deinit(AutomataAlgoChooser_ptr self)
{
  /* members deinitialization */

  /* if initialized last_support_set belongs to AutomataAlgoChooser,
     so it has to be removed */
  if(self->last_support != NULL)
    Set_ReleaseSet(self->last_support);
}

/*!
  \brief Returns true if the heuristic must be used to
  chose the algorithm type in operations, false otherwise.

  
*/
static boolean automata_algo_chooser_use_heuristic(AutomataManager_ptr manager)
{
  Automaton_Algorithm_Type algorithm_type;
  algorithm_type = AutomataManager_get_algorithm_type(manager);
  if(algorithm_type == HEURISTIC_BASED)
    return true;
  else
    return false;
}

/*!
  \brief Returns true if is needed the computation of
  support set.

  
*/
static boolean automata_algo_chooser_needs_support(AutomataManager_ptr manager)
{
  Automaton_Algorithm_Type algorithm_type;
  algorithm_type = AutomataManager_get_algorithm_type(manager);
  if(algorithm_type == HEURISTIC_BASED || 
     algorithm_type == ALPHABET_BASED)
    return true;
  else
    return false;
}

/*!
  \brief Compute the type of algorithm that an operation
  will use.

  Heuristic decides which algorithm apply for
  combining a set of locations.
  This decision is based comparing the cardinality of alphabet
  obtained from support set AP and cardinality of transitions.
*/
static Automaton_Algorithm_Type 
automata_algo_compute_heuristic(Set_t AP, int transitions_count)
{
  int ap_cardinality;
  double alphabet_size;
  double transitions_square;

  /* [SM] support set CAN be null. A null support set generates the
     alpahbet that contains the TRUE symbol */
  /* nusmv_assert(AP != NULL); */
  nusmv_assert(transitions_count >=0);

  ap_cardinality = Set_GiveCardinality(AP);

  /* Compute the heuristic */
  alphabet_size = pow(2, ap_cardinality);
  transitions_square = transitions_count * transitions_count;

  if(transitions_square < alphabet_size) {
    return TRANSITIONS_BASED;
  }else{
    return ALPHABET_BASED;
  }
}

/*!
  \brief Builds the support set from location_set

  Builds the support set from location_set.
  The location returned is the union of all the support sets of locations in
  location_set.
  The support set for a location is the union of all the support sets of
  all the output transitions of the location.
*/
static Set_t 
automata_algo_chooser_build_set_support(AutomataLocSet_ptr location_set)
{
  Set_Iterator_t location_set_iter;
  Set_t AP;

  /* computes the set AP as the union of all the support set from
     locations in location_set */

  AP = Set_MakeEmpty();

  /* Support set from location_set */
  STATE_SET_FOREACH(location_set, location_set_iter){
    AutomataLoc_ptr location;
    location = AutomataLocSet_get_member(location_set, location_set_iter);
    AUTOMATA_LOC_CHECK_INSTANCE(location);

    AP = Set_Union(AP, AutomataLoc_get_support_set(location));
  }

  return AP;
}

/*!
  \brief Count the number of output transitions from a set
  of locations.

  
*/
static int automata_algo_chooser_count_trans(AutomataLocSet_ptr location_set)
{
  Set_Iterator_t location_set_iter;
  int count;

  count = 0;

  STATE_SET_FOREACH(location_set, location_set_iter){
    AutomataLoc_ptr location;
    location = AutomataLocSet_get_member(location_set, location_set_iter);
    AUTOMATA_LOC_CHECK_INSTANCE(location);
    
    count = count + AutomataLoc_get_transitions_count(location);
  }

  return count;
}

/*!
  \brief Merge the support set with the permanent support
  set, if it exists

  
*/
static Set_t automata_algo_union_permanent_support(AutomataAlgoChooser_ptr self,
                                                   Set_t support_set)
{
  if(self->permanent_support){
    support_set = Set_Union(support_set, self->permanent_support);
  }
  
  return support_set;
}

/*!
  \brief Adds the permanent transition count to the
  transition count

  
*/
static int automata_algo_sum_perm_trans_count(AutomataAlgoChooser_ptr self,
                                              int transition_count)
{
  if(self->permanent_support){
    return (self->permanent_transition_count + 1);
  }
  
  return transition_count;
}

/*!
  \brief Free the last support set and initialize it as a
  new empty set

  
*/
static void automata_algo_get_fresh_support_set(AutomataAlgoChooser_ptr self)
{
  /* if initialized release support set */
  if(self->last_support != NULL)
    Set_ReleaseSet(self->last_support);
  self->last_support = Set_MakeEmpty();
}

/**AutomaticEnd***************************************************************/

