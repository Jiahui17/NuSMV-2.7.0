/* ---------------------------------------------------------------------------

  This file is part of the ``labels'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'AutomataBddLabelCombinator'

  Derived class from AutomataLabelCombinator for the BDD
  label implementation. No overriding of combine is provided by this
  class. The overriding allows to check for the right label type in
  the combine method.

*/

#include "nusmv/core/automata/labels/AutomataBddLabelCombinator.h"
#include "nusmv/core/automata/labels/AutomataBddLabelCombinator_private.h"
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
/* See 'AutomataBddLabelCombinator_private.h' for class 'AutomataBddLabelCombinator' definition. */

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

static void automata_bdd_label_combinator_finalize(Object_ptr object, void* dummy);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataBddLabelCombinator_ptr
AutomataBddLabelCombinator_create(const NuSMVEnv_ptr env)
{
  AutomataBddLabelCombinator_ptr self = ALLOC(AutomataBddLabelCombinator, 1);
  AUTOMATA_BDD_LABEL_COMBINATOR_CHECK_INSTANCE(self);

  automata_bdd_label_combinator_init(self, env);
  return self;
}

void AutomataBddLabelCombinator_destroy(AutomataBddLabelCombinator_ptr self)
{
  AUTOMATA_BDD_LABEL_COMBINATOR_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void automata_bdd_label_combinator_init(AutomataBddLabelCombinator_ptr self,
                                        const NuSMVEnv_ptr env)
{
  /* base class initialization */
  automata_label_combinator_init(AUTOMATA_LABEL_COMBINATOR(self), env);

  /* members initialization */
  automata_label_combinator_set_label_type(AUTOMATA_LABEL_COMBINATOR(self),
                                           BDD_LABEL);

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = automata_bdd_label_combinator_finalize;
}

void automata_bdd_label_combinator_deinit(AutomataBddLabelCombinator_ptr self)
{
  /* members deinitialization */


  /* base class deinitialization */
  automata_label_combinator_deinit(AUTOMATA_LABEL_COMBINATOR(self));
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataBddLabelCombinator class virtual finalizer

  Called by the class destructor
*/
static void automata_bdd_label_combinator_finalize(Object_ptr object, void* dummy)
{
  AutomataBddLabelCombinator_ptr self = AUTOMATA_BDD_LABEL_COMBINATOR(object);

  automata_bdd_label_combinator_deinit(self);
  FREE(self);
}



/**AutomaticEnd***************************************************************/

