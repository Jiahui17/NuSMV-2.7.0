/* ---------------------------------------------------------------------------


   %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Michele Dorigatti
  \brief Implementation of class 'AbstractAutomaton'

  \todo: Missing description

*/


#include "nusmv/core/automata/AbstractAutomaton.h"
#include "nusmv/core/automata/AbstractAutomaton_private.h"
#include "nusmv/core/utils/utils.h"

#include "nusmv/core/automata/automataInt.h"
#include "nusmv/core/automata/AutomataAlgoChooser.h"
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
/* See 'AbstractAutomaton_private.h' for class 'AbstractAutomaton' definition. */

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

static void abstract_automaton_finalize(Object_ptr object, void* dummy);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AbstractAutomaton class constructor

  The AbstractAutomaton class constructor

  \sa AbstractAutomaton_destroy
*/

/* WARNING [MDHybrid] Consider deleting create and destroy since this class
   should not be instantiated */
AbstractAutomaton_ptr AbstractAutomaton_create(void)
{
  AbstractAutomaton_ptr self = ALLOC(AbstractAutomaton, 1);
  ABSTRACT_AUTOMATON_CHECK_INSTANCE(self);

  abstract_automaton_init(self);
  return self;
}

void AbstractAutomaton_destroy(AbstractAutomaton_ptr self)
{
  ABSTRACT_AUTOMATON_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

VIRTUAL Slist_ptr AbstractAutomaton_get_location_list(AbstractAutomaton_ptr self)
{
  ABSTRACT_AUTOMATON_CHECK_INSTANCE(self);

  return self->get_location_list(self);
}

VIRTUAL Set_t AbstractAutomaton_get_initial_locations(AbstractAutomaton_ptr self)
{
  ABSTRACT_AUTOMATON_CHECK_INSTANCE(self);

  return self->get_initial_locations(self);
}

VIRTUAL int AbstractAutomaton_get_transitions_duplicate_count(AbstractAutomaton_ptr self)
{
  ABSTRACT_AUTOMATON_CHECK_INSTANCE(self);

  return self->get_transitions_duplicate_count(self);
}

VIRTUAL unsigned int AbstractAutomaton_get_location_count(AbstractAutomaton_ptr self)
{
  ABSTRACT_AUTOMATON_CHECK_INSTANCE(self);

  return self->get_location_count(self);
}

VIRTUAL long AbstractAutomaton_get_transitions_count(AbstractAutomaton_ptr self)
{
  ABSTRACT_AUTOMATON_CHECK_INSTANCE(self);

  return self->get_transitions_count(self);
}

VIRTUAL int AbstractAutomaton_new_location(AutomataManager_ptr manager,
                                   AbstractAutomaton_ptr self,
                                   boolean is_final)
{
  ABSTRACT_AUTOMATON_CHECK_INSTANCE(self);

  return self->new_location(manager, self, is_final);
}

VIRTUAL void AbstractAutomaton_add_location(AbstractAutomaton_ptr self,
                                    AutomataLoc_ptr location)
{
  ABSTRACT_AUTOMATON_CHECK_INSTANCE(self);

  self->add_location(self, location);
}

VIRTUAL void AbstractAutomaton_print(AbstractAutomaton_ptr self,
                                     NuSMVEnv_ptr env,
                                     FILE* out_file)
{
  ABSTRACT_AUTOMATON_CHECK_INSTANCE(self);
  nusmv_assert(out_file != NULL);

  self->print(self, env, out_file);
}

VIRTUAL void AbstractAutomaton_print_dot(AbstractAutomaton_ptr self,
                                         NuSMVEnv_ptr env,
                                         FILE* out_file)
{
  ABSTRACT_AUTOMATON_CHECK_INSTANCE(self);
  nusmv_assert(out_file != NULL);

  self->print_dot(self, env, out_file);
}

VIRTUAL void
AbstractAutomaton_add_initial_location(AbstractAutomaton_ptr const self,
                                       AutomataLoc_ptr const location)
{
  ABSTRACT_AUTOMATON_CHECK_INSTANCE(self);

  self->add_initial_location(self, location);
}

VIRTUAL
void AbstractAutomaton_set_sink_location(AbstractAutomaton_ptr self,
					 AutomataLoc_ptr location)
{
  ABSTRACT_AUTOMATON_CHECK_INSTANCE(self);
  AUTOMATA_LOC_CHECK_INSTANCE(location);

  self->set_sink_location(self, location);
}

VIRTUAL
AutomataLoc_ptr AbstractAutomaton_get_sink_location(AbstractAutomaton_ptr self)
{
  ABSTRACT_AUTOMATON_CHECK_INSTANCE(self);

  return self->get_sink_location(self);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void abstract_automaton_init(AbstractAutomaton_ptr self)
{
  /* base class initialization */
  object_init(OBJECT(self));

  /* members initialization */
  self->initial_locations = Set_MakeEmpty();
  self->location_count = 0;
  self->locations = Slist_create();
  self->type = DETERMINISTIC;
  self->label_type = BDD_LABEL;
  self->sink_location = NULL;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = abstract_automaton_finalize;

  OVERRIDE(AbstractAutomaton, get_location_list)
    = abstract_automaton_get_location_list;
  OVERRIDE(AbstractAutomaton, get_initial_locations) =
    abstract_automaton_get_initial_locations;
  OVERRIDE(AbstractAutomaton, add_initial_location) =
    abstract_automaton_add_initial_location;
  OVERRIDE(AbstractAutomaton, get_transitions_duplicate_count) =
    abstract_automaton_get_transitions_duplicate_count;
  OVERRIDE(AbstractAutomaton, get_location_count) =
    abstract_automaton_get_location_count;
  OVERRIDE(AbstractAutomaton, get_transitions_count) =
    abstract_automaton_get_transitions_count;
  OVERRIDE(AbstractAutomaton, new_location) = abstract_automaton_new_loc;
  OVERRIDE(AbstractAutomaton, print) = abstract_automaton_print;
  OVERRIDE(AbstractAutomaton, print_dot) = abstract_automaton_print_dot;
  OVERRIDE(AbstractAutomaton, create_location_from_manager) =
    abstract_automaton_create_loc_from_manager;
  OVERRIDE(AbstractAutomaton, create_location) = abstract_automaton_create_loc;
  OVERRIDE(AbstractAutomaton, add_location) = abstract_automaton_add_loc;
  OVERRIDE(AbstractAutomaton, get_sink_location) =
    abstract_automaton_get_sink_location;
  OVERRIDE(AbstractAutomaton, set_sink_location) =
    abstract_automaton_set_sink_location;
}

void abstract_automaton_deinit(AbstractAutomaton_ptr self)
{
  /* members deinitialization */
  /* free all locations of abstract_automaton. This is performed popping out all
     elements from location's stack.
  */
  while(! Slist_is_empty(self->locations)) {
    AutomataLoc_ptr location = Slist_pop(self->locations);
    AutomataLoc_destroy(location);
  }
  Slist_destroy(self->locations);
  Set_ReleaseSet(self->initial_locations);

  /* base class deinitialization */
  object_deinit(OBJECT(self));
}

Slist_ptr abstract_automaton_get_location_list(AbstractAutomaton_ptr self)
{
  return self->locations;
}

Set_t abstract_automaton_get_initial_locations(AbstractAutomaton_ptr self)
{
  return self->initial_locations;
}

void abstract_automaton_add_initial_location(AbstractAutomaton_ptr const self,
                                             AutomataLoc_ptr const location)
{
  self->initial_locations = Set_AddMember(self->initial_locations,
                                          (Set_Element_t)location);
}

void abstract_automaton_print(AbstractAutomaton_ptr A, NuSMVEnv_ptr env,
                              FILE* out_file)
{
  error_unreachable_code();
}

void abstract_automaton_print_dot(AbstractAutomaton_ptr A, NuSMVEnv_ptr env,
                                  FILE* out_file)
{
  error_unreachable_code();
}

int abstract_automaton_get_transitions_duplicate_count(AbstractAutomaton_ptr A)
{
  Siter location_iter;
  int duplicates_count;

  duplicates_count = 0;

  SLIST_FOREACH(A->locations, location_iter) {
    AutomataLoc_ptr location;
    Siter trans_iter;
    Set_t destinations_locations;

    location = AUTOMATA_LOC(Siter_element(location_iter));

    destinations_locations = Set_MakeEmpty();

    SLIST_FOREACH(AutomataLoc_get_transitions(location), trans_iter) {
      AutomataTransition_ptr trans;
      AutomataLoc_ptr end_location;
      int end_location_id;

      trans = AUTOMATA_TRANSITION(Siter_element(trans_iter));
      end_location = AutomataTransition_get_end_location(trans);
      end_location_id = AutomataLoc_get_id(end_location);

      if (Set_IsMember(destinations_locations,
                       (Set_Element_t) NODE_FROM_INT(end_location_id))) {
        duplicates_count = duplicates_count + 1;
      } else {
        destinations_locations =
          Set_AddMember(destinations_locations,
                        (Set_Element_t) NODE_FROM_INT(end_location_id));
      }

    }

    Set_ReleaseSet(destinations_locations);
  }

  return duplicates_count;
}

unsigned int abstract_automaton_get_location_count(AbstractAutomaton_ptr self)
{
  return self->location_count;
}

long abstract_automaton_get_transitions_count(AbstractAutomaton_ptr A)
{
  Siter iter;
  long trans_count;

  trans_count = 0;
  SLIST_FOREACH(A->locations, iter) {
    AutomataLoc_ptr location;

    location = AUTOMATA_LOC(Siter_element(iter));
    trans_count = trans_count + AutomataLoc_get_transitions_count(location);
  }

  return trans_count;
}

int abstract_automaton_new_loc(AutomataManager_ptr manager, AbstractAutomaton_ptr A,
                               boolean is_final)
{
  AutomataLoc_ptr new_location;

  new_location = A->create_location_from_manager(A, manager);
  AutomataLoc_set_final(new_location, is_final);
  abstract_automaton_add_loc(A, new_location);

  return AutomataLoc_get_id(new_location);
}

void abstract_automaton_add_loc(AbstractAutomaton_ptr self,
                                AutomataLoc_ptr location)
{
  ABSTRACT_AUTOMATON_CHECK_INSTANCE(self);

  self->location_count = self->location_count + 1;
  AutomataLoc_set_id(location, self->location_count);

  Slist_push(self->locations, location);
}

AutomataLoc_ptr
abstract_automaton_create_loc_from_manager(AbstractAutomaton_ptr self,
                                           AutomataManager_ptr manager)
{
  return AutomataLoc_create_from_manager(manager);
}

AutomataLoc_ptr
abstract_automaton_create_loc(AbstractAutomaton_ptr self,
                              Automaton_Label_Type label_type,
                              boolean keep_support_location)
{
  return AutomataLoc_create(label_type, keep_support_location);
}

void abstract_automaton_set_sink_location(AbstractAutomaton_ptr self,
					  AutomataLoc_ptr location)
{
  ABSTRACT_AUTOMATON(self)->sink_location = AUTOMATA_LOC(location);
}

AutomataLoc_ptr abstract_automaton_get_sink_location(AbstractAutomaton_ptr self)
{
  return AUTOMATA_LOC(ABSTRACT_AUTOMATON(self)->sink_location);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AbstractAutomaton class virtual finalizer

  Called by the class destructor
*/
static void abstract_automaton_finalize(Object_ptr object, void* dummy)
{
  AbstractAutomaton_ptr self = ABSTRACT_AUTOMATON(object);

  abstract_automaton_deinit(self);
  FREE(self);
}



/**AutomaticEnd***************************************************************/

