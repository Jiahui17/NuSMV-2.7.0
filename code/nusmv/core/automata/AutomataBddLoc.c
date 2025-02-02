/* ---------------------------------------------------------------------------


  This file is part of the ``automata'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Chun Tian
  \brief Implementation of class 'AutomataBddLoc'

  They're locations in an automaton converted from a BddFsm.
 */

#include "nusmv/core/automata/AutomataNamedLoc_private.h"
#include "nusmv/core/automata/AutomataBddLoc.h"
#include "nusmv/core/automata/AutomataBddLoc_private.h"
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
/* See 'AutomataBddLoc_private.h' for class 'AutomataBddLoc' definition. */

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

static void automata_bdd_loc_finalize(Object_ptr object, void* dummy);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataBddLoc_ptr
AutomataBddLoc_create (boolean keep_support_location,
		       BddEnc_ptr bdd_enc)
{
  Automaton_Label_Type label_type = BDD_LABEL;
  AutomataBddLoc_ptr self = ALLOC(AutomataBddLoc, 1);

  AUTOMATA_BDD_LOC_CHECK_INSTANCE(self);

  automata_bdd_loc_init(self, label_type, keep_support_location, bdd_enc);

  return self;
}

AutomataBddLoc_ptr
AutomataBddLoc_create_from_manager (AutomataManager_ptr manager)
{
  Automaton_Label_Type label_type =
    AutomataManager_get_label_type(manager);
  boolean keep_support_location =
    AutomataManager_is_keep_support_set_in_location(manager);
  BddEnc_ptr bdd_enc =
    AutomataManager_get_bdd_enc(manager);
  AutomataBddLoc_ptr self;

  nusmv_assert(BDD_LABEL == label_type);
  self = AutomataBddLoc_create(keep_support_location, bdd_enc);

  AUTOMATA_BDD_LOC_CHECK_INSTANCE(self);
  return self;
}

void AutomataBddLoc_destroy (AutomataBddLoc_ptr self)
{
  AUTOMATA_BDD_LOC_CHECK_INSTANCE(self);

  automata_bdd_loc_finalize(OBJECT(self), NULL);
}

void AutomataBddLoc_set_states (AutomataBddLoc_ptr self, BddStates bdd_states)
{
  AUTOMATA_BDD_LOC_CHECK_INSTANCE(self);

  if (BDD_STATES(NULL) != self->bdd_states) {
    DDMgr_ptr dd = BddEnc_get_dd_manager(self->bdd_enc);
    bdd_free(dd, self->bdd_states);
  }

  self->bdd_states = bdd_dup(bdd_states);
}

BddStates AutomataBddLoc_get_states (AutomataBddLoc_ptr self)
{
  AUTOMATA_BDD_LOC_CHECK_INSTANCE(self);

  return (BDD_STATES(NULL) != self->bdd_states) ?
    bdd_dup(self->bdd_states) : BDD_STATES(NULL);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void automata_bdd_loc_init (AutomataBddLoc_ptr self,
			    Automaton_Label_Type label_type,
			    boolean keep_support_location,
			    BddEnc_ptr bdd_enc)
{
  /* base class initialization */
  automata_named_loc_init(AUTOMATA_NAMED_LOC(self), label_type, keep_support_location);

  /* members initialization */
  self->bdd_enc    = bdd_enc;
  self->bdd_states = BDD_STATES(NULL);

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = automata_bdd_loc_finalize;
}

void automata_bdd_loc_deinit(AutomataBddLoc_ptr self)
{
  /* members deinitialization */
  if (BDD_STATES(NULL) != self->bdd_states) {
    DDMgr_ptr dd = BddEnc_get_dd_manager(self->bdd_enc);
    bdd_free(dd, self->bdd_states);
  }
  self->bdd_enc = BDD_ENC(NULL);

  /* base class deinitialization */
  automata_named_loc_deinit(AUTOMATA_NAMED_LOC(self));
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataBddLoc class virtual finalizer

  Called by the class destructor
*/
static void automata_bdd_loc_finalize(Object_ptr object, void* dummy)
{
  AutomataBddLoc_ptr self = AUTOMATA_BDD_LOC(object);

  automata_bdd_loc_deinit(self);
  FREE(self);
}

/**AutomaticEnd***************************************************************/
