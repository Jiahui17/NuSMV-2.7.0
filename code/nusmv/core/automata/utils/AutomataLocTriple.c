/* ---------------------------------------------------------------------------

  This file is part of the ``utils'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'AutomataLocTriple'

  Implementation of a triple that contains locations.

*/

#include "nusmv/core/automata/utils/AutomataLocTriple.h"
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

typedef struct AutomataLocTriple_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  AutomataLoc_ptr first_element;
  AutomataLoc_ptr second_element;
  AutomataLoc_ptr third_element;

} AutomataLocTriple;

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

static void automata_loc_triple_init(AutomataLocTriple_ptr self);
static void automata_loc_triple_deinit(AutomataLocTriple_ptr self);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataLocTriple_ptr AutomataLocTriple_create()
{
  AutomataLocTriple_ptr self = ALLOC(AutomataLocTriple, 1);
  AUTOMATA_LOC_TRIPLE_CHECK_INSTANCE(self);

  automata_loc_triple_init(self);
  return self;
}

void AutomataLocTriple_destroy(AutomataLocTriple_ptr self)
{
  AUTOMATA_LOC_TRIPLE_CHECK_INSTANCE(self);

  automata_loc_triple_deinit(self);
  FREE(self);
}

AutomataLocTriple_ptr 
AutomataLocTriple_create_from_locations (AutomataLoc_ptr first,
                                        AutomataLoc_ptr second,
                                        AutomataLoc_ptr third)
{
  AutomataLocTriple_ptr self;

  AUTOMATA_LOC_CHECK_INSTANCE(first);
  AUTOMATA_LOC_CHECK_INSTANCE(second);
  AUTOMATA_LOC_CHECK_INSTANCE(third);

  self = AutomataLocTriple_create();
  self->first_element = first;
  self->second_element = second;
  self->third_element = third;

  return self;
}

void AutomataLocTriple_set_first (AutomataLocTriple_ptr triple,
                                    AutomataLoc_ptr first_element)
{
  AUTOMATA_LOC_TRIPLE_CHECK_INSTANCE(triple);
  AUTOMATA_LOC_CHECK_INSTANCE(first_element);

  triple->first_element = first_element;
}

AutomataLoc_ptr
AutomataLocTriple_get_first (AutomataLocTriple_ptr triple)
{
  AUTOMATA_LOC_TRIPLE_CHECK_INSTANCE(triple); 
  
  return triple->first_element;
}

void AutomataLocTriple_set_second(AutomataLocTriple_ptr triple,
                                    AutomataLoc_ptr second_element)
{
  AUTOMATA_LOC_TRIPLE_CHECK_INSTANCE(triple); 
  AUTOMATA_LOC_CHECK_INSTANCE(second_element); 

  triple->second_element = second_element;
}

AutomataLoc_ptr
AutomataLocTriple_get_second (AutomataLocTriple_ptr triple)
{
  AUTOMATA_LOC_TRIPLE_CHECK_INSTANCE(triple); 

  return triple->second_element;
}

void AutomataLocTriple_set_third (AutomataLocTriple_ptr triple,
                                    AutomataLoc_ptr third_element)
{
  AUTOMATA_LOC_TRIPLE_CHECK_INSTANCE(triple); 
  AUTOMATA_LOC_CHECK_INSTANCE(third_element); 

  triple->third_element = third_element;
}

AutomataLoc_ptr
AutomataLocTriple_get_third (AutomataLocTriple_ptr triple)
{
  AUTOMATA_LOC_TRIPLE_CHECK_INSTANCE(triple); 

  return triple->third_element;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataLocTriple class private initializer

  The AutomataLocTriple class private initializer

  \sa AutomataLocTriple_create
*/
static void automata_loc_triple_init(AutomataLocTriple_ptr self)
{
  /* members initialization */
  self->first_element = NULL;
  self->second_element = NULL;
  self->third_element = NULL;
}

/*!
  \brief The AutomataLocTriple class private deinitializer

  The AutomataLocTriple class private deinitializer

  \sa AutomataLocTriple_destroy
*/
static void automata_loc_triple_deinit(AutomataLocTriple_ptr self)
{
  /* members deinitialization */
  self->first_element = NULL;
  self->second_element = NULL;
  self->third_element = NULL;
}



/**AutomaticEnd***************************************************************/

