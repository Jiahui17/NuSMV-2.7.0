/* ---------------------------------------------------------------------------

  This file is part of the ``automata'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Michele Dorigatti
  \brief Implementation of class 'AutomataNamedLoc'

  \todo: Missing description

*/

#include "nusmv/core/automata/AutomataNamedLoc.h"
#include "nusmv/core/automata/AutomataNamedLoc_private.h"
#include "nusmv/core/automata/Automaton.h"

#include "nusmv/core/utils/utils.h"
#include "nusmv/core/utils/object.h"
#include "nusmv/core/utils/object_private.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'AutomataNamedLoc_private.h' for class 'AutomataNamedLoc' definition. */

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

static void automata_named_loc_finalize(Object_ptr object,
                                          void* dummy);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataNamedLoc_ptr
AutomataNamedLoc_create(Automaton_Label_Type label_type,
                          boolean keep_support_location)
{
  AutomataNamedLoc_ptr self = ALLOC(AutomataNamedLoc, 1);

  AUTOMATA_NAMED_LOC_CHECK_INSTANCE(self);

  automata_named_loc_init(self, label_type, keep_support_location);

  return self;
}

AutomataNamedLoc_ptr
AutomataNamedLoc_create_from_manager(AutomataManager_ptr manager)
{
  AutomataNamedLoc_ptr self;

  self = AutomataNamedLoc_create(
                 AutomataManager_get_label_type(manager),
                 AutomataManager_is_keep_support_set_in_location(manager));

  AUTOMATA_NAMED_LOC_CHECK_INSTANCE(self);

  return self;
}

AutomataNamedLoc_ptr
AutomataNamedLoc_create_and_add_to_automaton(const AutomataManager_ptr manager,
                                               const Automaton_ptr automaton,
                                               const boolean is_final,
                                               const char* location_str)
{
  AutomataNamedLoc_ptr location;

  location = AutomataNamedLoc_create_from_manager(manager);
  AutomataLoc_set_final(AUTOMATA_LOC(location), is_final);
  AutomataNamedLoc_set_name(location, (char*)location_str);
  AbstractAutomaton_add_location(ABSTRACT_AUTOMATON(automaton), AUTOMATA_LOC(location));

  return location;
}

void AutomataNamedLoc_destroy(AutomataNamedLoc_ptr self)
{
  AUTOMATA_NAMED_LOC_CHECK_INSTANCE(self);

  automata_named_loc_finalize(OBJECT(self), NULL);
}

void AutomataNamedLoc_set_name(AutomataNamedLoc_ptr self, char* name)
{
  AUTOMATA_NAMED_LOC_CHECK_INSTANCE(self);

  if ((char*)NULL != self->name) FREE(self->name);

  self->name = util_strsav(name);
}

char* AutomataNamedLoc_get_name(AutomataNamedLoc_ptr self)
{
  AUTOMATA_NAMED_LOC_CHECK_INSTANCE(self);

  return self->name;
}

boolean AutomataNamedLoc_have_same_name(AutomataNamedLoc_ptr first,
                                          AutomataNamedLoc_ptr second)
{
  AUTOMATA_NAMED_LOC_CHECK_INSTANCE(first);
  AUTOMATA_NAMED_LOC_CHECK_INSTANCE(second);

  return (! strcmp(first->name, second->name));
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void automata_named_loc_init(AutomataNamedLoc_ptr self,
                             Automaton_Label_Type label_type,
                             boolean keep_support_location)
{
  /* base class initialization */
  automata_loc_init(AUTOMATA_LOC(self), label_type, keep_support_location);

  /* members initialization */
  self->name = (char*)NULL;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = automata_named_loc_finalize;
}

void automata_named_loc_deinit(AutomataNamedLoc_ptr self)
{
  /* members deinitialization */
  FREE(self->name);

  /* base class deinitialization */
  automata_loc_deinit(AUTOMATA_LOC(self));
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataNamedLoc class virtual finalizer

  Called by the class destructor
*/
static void automata_named_loc_finalize(Object_ptr object, void* dummy)
{
  AutomataNamedLoc_ptr self = AUTOMATA_NAMED_LOC(object);

  automata_named_loc_deinit(self);
  FREE(self);
}



/**AutomaticEnd***************************************************************/

