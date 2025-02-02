/* ---------------------------------------------------------------------------

  This file is part of the ``labels'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'AutomataLabelCombinator'

  Base class for the combinator hierarchy. Every type of
  label can declare his own type of combinator, that is used to build
  all the possibile combinations of transitions.

  This is bind to type of label because it allows to exploit the
  specific solver functionalities (for example the incrementality of
  the solver).

*/

#include "nusmv/core/node/NodeMgr.h"
#include "nusmv/core/automata/labels/AutomataLabelCombinator.h"
#include "nusmv/core/automata/labels/AutomataLabelCombinator_private.h"
#include "nusmv/core/automata/labels/AutomataLabel.h"

#include "nusmv/core/automata/AutomataLoc.h"
#include "nusmv/core/automata/utils/AutomataLocSet.h"

#include "nusmv/core/node/node.h"
#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/utils/utils.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'AutomataLabelCombinator_private.h' for class 'AutomataLabelCombinator' definition. */

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

static void automata_label_combinator_finalize(Object_ptr object, void* dummy);

static Set_t
automata_label_combinator_combine_from_iter(AutomataLabelCombinator_ptr self,
                                            Set_t label_location_set,
                                            Set_Iterator_t iter);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataLabelCombinator_ptr AutomataLabelCombinator_create(const NuSMVEnv_ptr env)
{
  AutomataLabelCombinator_ptr self = ALLOC(AutomataLabelCombinator, 1);
  AUTOMATA_LABEL_COMBINATOR_CHECK_INSTANCE(self);

  automata_label_combinator_init(self, env);
  return self;
}

void AutomataLabelCombinator_destroy(AutomataLabelCombinator_ptr self)
{
  AUTOMATA_LABEL_COMBINATOR_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

VIRTUAL Set_t AutomataLabelCombinator_combine (AutomataLabelCombinator_ptr self,
                                               Set_t label_location_set)
{
  AUTOMATA_LABEL_COMBINATOR_CHECK_INSTANCE(self);

  return self->combine(self, label_location_set);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void automata_label_combinator_init(AutomataLabelCombinator_ptr self,
                                    const NuSMVEnv_ptr env)
{
  /* base class initialization */
  env_object_init(ENV_OBJECT(self), env);

  /* members initialization */
  self->label_type = BASE_LABEL;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = automata_label_combinator_finalize;

  OVERRIDE(AutomataLabelCombinator, combine) =
    automata_label_combinator_combine;
}

void automata_label_combinator_deinit(AutomataLabelCombinator_ptr self)
{
  /* members deinitialization */


  /* base class deinitialization */
  env_object_deinit(ENV_OBJECT(self));
}

void automata_label_combinator_set_label_type(AutomataLabelCombinator_ptr self,
                                              Automaton_Label_Type type)
{
  self->label_type = type;
}

Set_t automata_label_combinator_combine (AutomataLabelCombinator_ptr self,
                                         Set_t label_location_set)
{
  Set_Iterator_t iter;

  iter = Set_GetFirstIter(label_location_set);
  return automata_label_combinator_combine_from_iter(self,
                                                     label_location_set,
                                                     iter);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataLabelCombinator class virtual finalizer

  Called by the class destructor
*/
static void automata_label_combinator_finalize(Object_ptr object, void* dummy)
{
  AutomataLabelCombinator_ptr self = AUTOMATA_LABEL_COMBINATOR(object);

  automata_label_combinator_deinit(self);
  FREE(self);
}

/*!
  \brief Recursive step of
  automata_label_combinator_combine.

  Creates all the possible combinations of labels
  contained in the subset of label_location_set that starts from the
  element pointed by iter.

  label_location_set is a set of couples (label, location). Couples are
  CONS node_ptr where car is the label and cdr is location.

  iter is an iterator on label_location_set and points to an element in
  the set. This function builds the combination of the subset of
  label_location_set formed by the elements that will be visited by iter.

  The function returns the set of couples (label, location_set), where
  label is a possible combination of the subset of label considered and
  location_set is the set of locations that are reachable with label.

  The function prunes the UNSAT combinations of labels.
*/

static Set_t
automata_label_combinator_combine_from_iter (AutomataLabelCombinator_ptr self,
                                             Set_t label_location_set,
                                             Set_Iterator_t iter)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

  Set_t result;

  if(Set_IsEndIter(iter)) {
    /* BASE CASE: the considered set is empty */
    result = Set_MakeEmpty();
  }else if(Set_IsEndIter(Set_GetNextIter(iter))) {
    /* BASE CASE: the considered subset has only one element */
    node_ptr element, first_element, second_element;
    AutomataLabel_ptr label, first_label, second_label;
    AutomataLocSet_ptr first_set, second_set;
    AutomataLoc_ptr dest_location;

    element = (node_ptr) Set_GetMember(label_location_set, iter);
    label = AUTOMATA_LABEL(car(element));
    AUTOMATA_LABEL_CHECK_TYPE(label, self->label_type);
    dest_location = AUTOMATA_LOC(cdr(element));

    /* creates new label
       Every time label is copied, so no side effects are performed
       on the label_location_set */
    first_label = AutomataLabel_copy(label);
    second_label = AutomataLabel_complement(label);

    /* creates new set */
    first_set = AutomataLocSet_create(env);
    second_set = AutomataLocSet_create(env); /* Empty set */

    AutomataLocSet_add_element(first_set, dest_location);

    /* creates new elements */
    first_element = new_node(nodemgr, CONS,
                             NODE_PTR(first_label),
                             NODE_PTR(first_set));
    second_element = new_node(nodemgr, CONS,
                              NODE_PTR(second_label),
                              NODE_PTR(second_set));

    result = Set_MakeEmpty();
    result = Set_AddMember(result, first_element);
    result = Set_AddMember(result, second_element);

  }else{
    /* Builds the combination for smaller subsets and combine the
       results with element */
    node_ptr element;
    AutomataLabel_ptr label;
    AutomataLoc_ptr dest_location;
    Set_t prev_res;
    Set_Iterator_t prev_res_iter;

    element = (node_ptr) Set_GetMember(label_location_set, iter);
    label = AUTOMATA_LABEL(car(element));
    AUTOMATA_LABEL_CHECK_TYPE(label, self->label_type);
    dest_location = AUTOMATA_LOC(cdr(element));

    /* Compute the result of location_set\{element}*/
    prev_res =
      automata_label_combinator_combine_from_iter(self,
                                                  label_location_set,
                                                  Set_GetNextIter(iter));

    result = Set_MakeEmpty();
    SET_FOREACH(prev_res, prev_res_iter) {
      node_ptr prev_element, first_element, second_element;
      AutomataLabel_ptr prev_label, first_label, second_label;
      AutomataLocSet_ptr prev_location_set, new_location_set;

      /* Get the current element in the iterator */
      prev_element = (node_ptr) Set_GetMember(prev_res, prev_res_iter);
      prev_label = AUTOMATA_LABEL(car(prev_element));
      prev_location_set = AUTOMATA_LOC_SET(cdr(prev_element));


      /* Builds the first new element in the result set. The new element is
         the pair (first_label, new_location_set):
           - first_label = prev_label ^ label
           - new_location_set = prev_location_set U dest_location
      */
      first_label = AutomataLabel_intersection(prev_label, label);

      /* Check if adding the element or not to the set, checking the SAT of the
         label.
       */
      if(AutomataLabel_is_sat(first_label)) {
        new_location_set = AutomataLocSet_copy(prev_location_set);
        AutomataLocSet_add_element(new_location_set, dest_location);
        first_element = new_node(nodemgr, CONS,
                                 NODE_PTR(first_label),
                                 NODE_PTR(new_location_set));
        result = Set_AddMember(result, (Set_Element_t) first_element);
      }

      /* Builds the second new element in the result set. The new
         element is  the pair (first_label, new_location_set):
         - first_label = prev_label ^ NOT(label)
           - new_location_set = prev_location_set
      */
      second_label = AutomataLabel_intersection(prev_label,
                                                AutomataLabel_complement(label));

      /* Check if adding the element or not to the set, checking the SAT of the
         label.
       */
      if(AutomataLabel_is_sat(second_label)) {
        /* Reuse  prev_location_set. This is important for
           prev_location_set, so it allows to reuse memory and not waste
           time  */
        second_element = new_node(nodemgr, CONS,
                                  NODE_PTR(second_label),
                                  NODE_PTR(prev_location_set));

        result = Set_AddMember(result, (Set_Element_t) second_element);
      }else{
        /* if it was not used free prev_location_set */
        AutomataLocSet_destroy(prev_location_set);
      }

      /* free previous_label (NOT prev_location_set, that is reused in
         second_element) */
      AutomataLabel_destroy(prev_label);

      /* free prev_element node_ptr - is it correct to use it in the iteration? */
      free_node(nodemgr, prev_element);
    }

    /* Free prev_res */
    Set_ReleaseSet(prev_res);
  }

  return result;
}


/**AutomaticEnd***************************************************************/

