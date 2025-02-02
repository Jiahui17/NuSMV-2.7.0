/* ---------------------------------------------------------------------------

  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Michele Dorigatti
  \brief Implementation of class 'AutomataHybridLabel'

  \todo: Missing description

*/

#include "nusmv/core/automata/labels/AutomataHybridLabel.h"
#include "nusmv/core/automata/labels/AutomataHybridLabel_private.h"

#include "nusmv/core/utils/utils.h"
#include "nusmv/core/utils/error.h"
#include "nusmv/core/parser/symbols.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'AutomataHybridLabel_private.h' for class 'AutomataHybridLabel' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*!
  \brief Deinitialize expr

  It is called by the setters of the other fields

  \se self->expr is modified
*/
#define INVALID_EXPR(self) \
  (AUTOMATA_LABEL(self)->expression = EXPR(NULL))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void automata_hybrid_label_finalize(Object_ptr object, void* dummy);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataHybridLabel_ptr AutomataHybridLabel_create(void)
{
  AutomataHybridLabel_ptr self = ALLOC(AutomataHybridLabel, 1);
  AUTOMATA_HYBRID_LABEL_CHECK_INSTANCE(self);

  automata_hybrid_label_init(self);
  return self;
}

void AutomataHybridLabel_destroy(AutomataHybridLabel_ptr self)
{
  AUTOMATA_HYBRID_LABEL_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

Expr_ptr AutomataHybridLabel_get_event(AutomataHybridLabel_ptr const self)
{
  AUTOMATA_HYBRID_LABEL_CHECK_INSTANCE(self);

  return self->event;
}

Olist_ptr AutomataHybridLabel_get_guards(AutomataHybridLabel_ptr const self)
{
  AUTOMATA_HYBRID_LABEL_CHECK_INSTANCE(self);

  return self->guards;
}

Olist_ptr AutomataHybridLabel_get_effects(AutomataHybridLabel_ptr const self)
{
  AUTOMATA_HYBRID_LABEL_CHECK_INSTANCE(self);

  return self->effects;
}

void AutomataHybridLabel_set_event(AutomataHybridLabel_ptr self,
                                   Expr_ptr const event)
{
  AUTOMATA_HYBRID_LABEL_CHECK_INSTANCE(self);

  INVALID_EXPR(self);

  self->event = event;
}

void AutomataHybridLabel_add_guard(AutomataHybridLabel_ptr self,
                                   Expr_ptr const guard)
{
  AUTOMATA_HYBRID_LABEL_CHECK_INSTANCE(self);

  INVALID_EXPR(self);

  Olist_append(self->guards, guard);
}

void AutomataHybridLabel_add_effect(AutomataHybridLabel_ptr self,
                                    Expr_ptr const effect)
{
  AUTOMATA_HYBRID_LABEL_CHECK_INSTANCE(self);

  INVALID_EXPR(self);

  Olist_append(self->effects, effect);
}

Expr_ptr AutomataHybridLabel_compute_expr(AutomataHybridLabel_ptr self,
                                          NodeMgr_ptr nodemgr)
{
  if (EXPR(NULL) == AUTOMATA_LABEL(self)->expression) {
    node_ptr result;
    Oiter iter;

    result = self->event;

    OLIST_FOREACH(self->guards, iter) {
      result =
        find_node(nodemgr, AND, result, NODE_PTR(Oiter_element(iter)));
    }

    OLIST_FOREACH(self->effects, iter) {
      result =
        find_node(nodemgr, AND, result, NODE_PTR(Oiter_element(iter)));
    }

    AUTOMATA_LABEL(self)->expression = EXPR(result);

    return EXPR(result);
  }
  else return AUTOMATA_LABEL(self)->expression;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void automata_hybrid_label_init(AutomataHybridLabel_ptr self)
{
  /* base class initialization */
  automata_label_init(AUTOMATA_LABEL(self));
  AUTOMATA_LABEL(self)->label_type = HYBRID_LABEL;

  /* members initialization */
  self->event = EXPR(NULL);
  self->guards = Olist_create();
  self->effects = Olist_create();

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = automata_hybrid_label_finalize;
  OVERRIDE(AutomataLabel, set_expr) = automata_hybrid_label_set_expr;
}

void automata_hybrid_label_deinit(AutomataHybridLabel_ptr self)
{
  /* members deinitialization */
  self->event = NULL;
  Olist_destroy(self->guards);
  Olist_destroy(self->effects);

  /* base class deinitialization */
  automata_label_deinit(AUTOMATA_LABEL(self));
}

void automata_hybrid_label_set_expr(AutomataLabel_ptr self, Expr_ptr new_expr)
{
  error_unreachable_code_msg("expression member of an Hybrid Label must not be"
                             " set directly.\n");
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataHybridLabel class virtual finalizer

  Called by the class destructor
*/
static void automata_hybrid_label_finalize(Object_ptr object, void* dummy)
{
  AutomataHybridLabel_ptr self = AUTOMATA_HYBRID_LABEL(object);

  automata_hybrid_label_deinit(self);
  FREE(self);
}

/**AutomaticEnd***************************************************************/

