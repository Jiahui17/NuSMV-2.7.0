/* ---------------------------------------------------------------------------

  This file is part of the ``automata'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author
  \brief Implementation of class 'AutomataModeLoc'

  \todo: Missing description

*/

#include "nusmv/core/automata/AutomataModeLoc.h"
#include "nusmv/core/automata/AutomataModeLoc_private.h"
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
/* See 'AutomataModeLoc_private.h' for class 'AutomataModeLoc' definition. */

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

static void automata_mode_loc_finalize(Object_ptr object,
                                          void* dummy);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataModeLoc_ptr
AutomataModeLoc_create(Automaton_Label_Type label_type,
                          boolean keep_support_location)
{
  AutomataModeLoc_ptr self = ALLOC(AutomataModeLoc, 1);

  AUTOMATA_MODE_LOC_CHECK_INSTANCE(self);

  automata_mode_loc_init(self, label_type, keep_support_location);

  return self;
}

AutomataModeLoc_ptr
AutomataModeLoc_create_from_manager(AutomataManager_ptr manager)
{
  AutomataModeLoc_ptr self;

  self = AutomataModeLoc_create(
                 AutomataManager_get_label_type(manager),
                 AutomataManager_is_keep_support_set_in_location(manager));

  AUTOMATA_MODE_LOC_CHECK_INSTANCE(self);

  return self;
}

AutomataModeLoc_ptr
AutomataModeLoc_create_and_add_to_automaton(const AutomataManager_ptr manager,
                                               const Automaton_ptr automaton,
                                               const boolean is_final,
                                               const Expr_ptr location_expr)
{
  AutomataModeLoc_ptr location;

  location = AutomataModeLoc_create_from_manager(manager);
  AutomataLoc_set_final(AUTOMATA_LOC(location), is_final);
  AutomataModeLoc_set_expr(location, (Expr_ptr)location_expr);
  AbstractAutomaton_add_location(ABSTRACT_AUTOMATON(automaton), AUTOMATA_LOC(location));

  return location;
}

void AutomataModeLoc_destroy(AutomataModeLoc_ptr self)
{
  AUTOMATA_MODE_LOC_CHECK_INSTANCE(self);

  automata_mode_loc_finalize(OBJECT(self), NULL);
}

void AutomataModeLoc_set_expr(AutomataModeLoc_ptr self, Expr_ptr expr)
{
  AUTOMATA_MODE_LOC_CHECK_INSTANCE(self);

  self->expr = expr;
}

Expr_ptr AutomataModeLoc_get_expr(AutomataModeLoc_ptr self)
{
  AUTOMATA_MODE_LOC_CHECK_INSTANCE(self);

  return self->expr;
}

boolean AutomataModeLoc_have_same_expr(AutomataModeLoc_ptr first,
                                       AutomataModeLoc_ptr second)
{
  AUTOMATA_MODE_LOC_CHECK_INSTANCE(first);
  AUTOMATA_MODE_LOC_CHECK_INSTANCE(second);

  return (first->expr == second->expr);
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void automata_mode_loc_init(AutomataModeLoc_ptr self,
                             Automaton_Label_Type label_type,
                             boolean keep_support_location)
{
  /* base class initialization */
  automata_loc_init(AUTOMATA_LOC(self), label_type, keep_support_location);

  /* members initialization */
  self->expr = (Expr_ptr) NULL;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = automata_mode_loc_finalize;
}

void automata_mode_loc_deinit(AutomataModeLoc_ptr self)
{
  /* members deinitialization */
  /* base class deinitialization */
  automata_loc_deinit(AUTOMATA_LOC(self));
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataModeLoc class virtual finalizer

  Called by the class destructor
*/
static void automata_mode_loc_finalize(Object_ptr object, void* dummy)
{
  AutomataModeLoc_ptr self = AUTOMATA_MODE_LOC(object);

  automata_mode_loc_deinit(self);
  FREE(self);
}



/**AutomaticEnd***************************************************************/
