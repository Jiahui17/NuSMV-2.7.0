/* ---------------------------------------------------------------------------

  This file is part of the ``automata'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'AutomataTransition'

  Contains the definition of the struct
  AutomataTransition_TAG and the function used to manage a
  transition. AutomataTransition_TAG represents a transition in an
  automaton.

*/

#include "nusmv/core/automata/AutomataTransition.h"
#include "nusmv/core/automata/labels/AutomataHybridLabel.h"

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

typedef struct AutomataTransition_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  AutomataLoc_ptr end_location;
  AutomataLabel_ptr label;

} AutomataTransition;



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

static void transition_init(AutomataTransition_ptr self);
static void transition_deinit(AutomataTransition_ptr self);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataTransition_ptr AutomataTransition_create()
{
  AutomataTransition_ptr self = ALLOC(AutomataTransition, 1);
  AUTOMATA_TRANSITION_CHECK_INSTANCE(self);

  transition_init(self);
  return self;
}

void AutomataTransition_destroy(AutomataTransition_ptr self)
{
  AUTOMATA_TRANSITION_CHECK_INSTANCE(self);

  transition_deinit(self);
  FREE(self);
}

AutomataTransition_ptr AutomataTransition_create_from_location_label(AutomataLoc_ptr end_location,
                                                                     AutomataLabel_ptr label)
{
  AutomataTransition_ptr self = AutomataTransition_create();

  nusmv_assert(end_location != NULL);
  nusmv_assert(label != NULL);

  AutomataTransition_set_end_location(self, end_location);
  AutomataTransition_set_label(self, label);

  return self;
}

void AutomataTransition_set_end_location(AutomataTransition_ptr self,
                                      AutomataLoc_ptr end_location)
{
  AUTOMATA_TRANSITION_CHECK_INSTANCE(self);
  nusmv_assert(end_location != NULL);

  self->end_location = end_location;
}

AutomataLoc_ptr AutomataTransition_get_end_location(AutomataTransition_ptr self)
{
  AUTOMATA_TRANSITION_CHECK_INSTANCE(self);

  return self->end_location;
}

void AutomataTransition_set_label(AutomataTransition_ptr self,
                                  AutomataLabel_ptr label)
{
  AUTOMATA_TRANSITION_CHECK_INSTANCE(self);
  nusmv_assert(label != NULL);

  self->label = label;
}

AutomataLabel_ptr AutomataTransition_get_label(AutomataTransition_ptr self)
{
  AUTOMATA_TRANSITION_CHECK_INSTANCE(self);

  return self->label;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataTransition class private initializer

  The AutomataTransition class private initializer

  \sa AutomataTransition_create
*/
static void transition_init(AutomataTransition_ptr self)
{
  /* members initialization */
  self->end_location = NULL;
  self->label = NULL;
}

/*!
  \brief The AutomataTransition class private deinitializer

  The AutomataTransition class private deinitializer

  \sa AutomataTransition_destroy
*/
static void transition_deinit(AutomataTransition_ptr self)
{
  /* members deinitialization */
  AutomataLabel_destroy(self->label);
}



/**AutomaticEnd***************************************************************/

