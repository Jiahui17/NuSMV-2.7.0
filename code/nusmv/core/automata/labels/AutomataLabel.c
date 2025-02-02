/* ---------------------------------------------------------------------------

  This file is part of the ``labels'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'AutomataLabel'

  'AutomataLabel' is the base class used to represent a
  label used in automata.

  A label contains an expression of a propositional formula.

*/

#include "nusmv/core/node/printers/MasterPrinter.h"
#include "nusmv/core/compile/compile.h"
#include "nusmv/core/utils/ErrorMgr.h"
#include "nusmv/core/utils/utils.h"
#include "nusmv/core/utils/error.h"

#include "nusmv/core/automata/labels/AutomataLabel.h"
#include "nusmv/core/automata/labels/AutomataLabel_private.h"
#include "nusmv/core/automata/utils/automata_error.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'AutomataLabel_private.h' for class 'AutomataLabel' definition. */

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

static void automata_label_finalize(Object_ptr object, void* dummy);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataLabel_ptr AutomataLabel_create(void)
{
  AutomataLabel_ptr self = ALLOC(AutomataLabel, 1);
  AUTOMATA_LABEL_CHECK_INSTANCE(self);

  automata_label_init(self);
  return self;
}

AutomataLabel_ptr AutomataLabel_create_from_expr (Expr_ptr expr)
{
  AutomataLabel_ptr self = AutomataLabel_create();

  self->set_expr(self, expr);
  return self;
}

void AutomataLabel_destroy(AutomataLabel_ptr self)
{
  AUTOMATA_LABEL_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

VIRTUAL Expr_ptr AutomataLabel_get_expr(AutomataLabel_ptr self)
{
  AUTOMATA_LABEL_CHECK_INSTANCE(self);

  return self->get_expr(self);
}

VIRTUAL void
AutomataLabel_set_expr (AutomataLabel_ptr self, Expr_ptr new_expr)
{
  AUTOMATA_LABEL_CHECK_INSTANCE(self);

  self->set_expr(self, new_expr);
}

VIRTUAL Automaton_Label_Type AutomataLabel_get_label_type (AutomataLabel_ptr self)
{
  AUTOMATA_LABEL_CHECK_INSTANCE(self);

  return self->get_label_type(self);
}

VIRTUAL boolean
AutomataLabel_is_sat (AutomataLabel_ptr self)
{
  AUTOMATA_LABEL_CHECK_INSTANCE(self);

  return self->is_sat(self);
}

VIRTUAL boolean
AutomataLabel_is_model (AutomataLabel_ptr self, Expr_ptr expression)
{
  AUTOMATA_LABEL_CHECK_INSTANCE(self);

  return self->is_model(self, expression);
}

VIRTUAL boolean AutomataLabel_is_true(AutomataLabel_ptr self)
{
  AUTOMATA_LABEL_CHECK_INSTANCE(self);

  return self->is_true(self);
}

VIRTUAL boolean
AutomataLabel_is_equivalent(AutomataLabel_ptr self, AutomataLabel_ptr label2)
{
  AUTOMATA_LABEL_CHECK_INSTANCE(self);

  return self->is_equivalent(self, label2);
}

VIRTUAL boolean
AutomataLabel_is_contained(AutomataLabel_ptr self, AutomataLabel_ptr label2)
{
  AUTOMATA_LABEL_CHECK_INSTANCE(self);

  return self->is_contained(self, label2);
}

VIRTUAL AutomataLabel_ptr
AutomataLabel_intersection(AutomataLabel_ptr self, AutomataLabel_ptr label2)
{
  AUTOMATA_LABEL_CHECK_INSTANCE(self);
  AUTOMATA_LABEL_CHECK_INSTANCE(label2);

  return self->intersection(self, label2);
}

VIRTUAL AutomataLabel_ptr
AutomataLabel_union(AutomataLabel_ptr self, AutomataLabel_ptr label2)
{
  AUTOMATA_LABEL_CHECK_INSTANCE(self);

  return self->label_union(self, label2);
}

VIRTUAL AutomataLabel_ptr
AutomataLabel_complement(AutomataLabel_ptr self)
{
  AUTOMATA_LABEL_CHECK_INSTANCE(self);

  return self->complement(self);
}

VIRTUAL Set_t
AutomataLabel_get_support(AutomataLabel_ptr self)
{
  AUTOMATA_LABEL_CHECK_INSTANCE(self);

  return self->get_support(self);
}

VIRTUAL AutomataLabel_ptr
AutomataLabel_copy(AutomataLabel_ptr self)
{
  AUTOMATA_LABEL_CHECK_INSTANCE(self);

  return self->copy(self);
}

VIRTUAL void
AutomataLabel_print_label(AutomataLabel_ptr self, NuSMVEnv_ptr env, FILE* outfile)
{
  AUTOMATA_LABEL_CHECK_INSTANCE(self);

  self->print_label(self, env, outfile);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void automata_label_init(AutomataLabel_ptr self)
{
  /* base class initialization */
  object_init(OBJECT(self));

  /* members initialization */
  self->expression = NULL;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = automata_label_finalize;

  /* Ovverride virtual methods: */
  OVERRIDE(AutomataLabel, set_expr) = automata_label_set_expr;
  OVERRIDE(AutomataLabel, get_expr) = automata_label_get_expr;
  OVERRIDE(AutomataLabel, get_label_type) = automata_label_get_label_type;
  OVERRIDE(AutomataLabel, is_sat) = automata_label_is_sat;
  OVERRIDE(AutomataLabel, is_model) = automata_label_is_model;
  OVERRIDE(AutomataLabel, is_true) = automata_label_is_true;
  OVERRIDE(AutomataLabel, is_equivalent) = automata_label_is_equivalent;
  OVERRIDE(AutomataLabel, is_contained) = automata_label_is_contained;
  OVERRIDE(AutomataLabel, intersection) = automata_label_intersection;
  OVERRIDE(AutomataLabel, label_union) = automata_label_union;
  OVERRIDE(AutomataLabel, complement) = automata_label_complement;
  OVERRIDE(AutomataLabel, get_support) = automata_label_get_support;
  OVERRIDE(AutomataLabel, copy) = automata_label_copy;
  OVERRIDE(AutomataLabel, print_label) = automata_label_print_label;
}

void automata_label_deinit(AutomataLabel_ptr self)
{
  /* members deinitialization */

  /* base class deinitialization */
  object_deinit(OBJECT(self));
}

void automata_label_set_expr(AutomataLabel_ptr self, Expr_ptr new_expr)
{
  self->expression = new_expr;
}

Expr_ptr automata_label_get_expr(AutomataLabel_ptr self)
{
  return self->expression;
}

void automata_label_set_label_type(AutomataLabel_ptr self,
                                   Automaton_Label_Type label_type)
{
  self->label_type = label_type;
}

Automaton_Label_Type automata_label_get_label_type(AutomataLabel_ptr self)
{
  return self->label_type;
}

boolean automata_label_is_sat(AutomataLabel_ptr self)
{
  error_unreachable_code(); /* Pure Virtual Member Function */
}

boolean automata_label_is_model(AutomataLabel_ptr self, Expr_ptr expression)
{
  error_unreachable_code(); /* Pure Virtual Member Function */
}

boolean automata_label_is_true(AutomataLabel_ptr self)
{
  error_unreachable_code(); /* Pure Virtual Member Function */
}

boolean automata_label_is_equivalent(AutomataLabel_ptr self,
                                     AutomataLabel_ptr label2)
{
  error_unreachable_code(); /* Pure Virtual Member Function */
}

boolean automata_label_is_contained(AutomataLabel_ptr self,
                                    AutomataLabel_ptr label2)
{
  error_unreachable_code(); /* Pure Virtual Member Function */
}

AutomataLabel_ptr automata_label_intersection(AutomataLabel_ptr self,
                                              AutomataLabel_ptr label2)
{
  error_unreachable_code(); /* Pure Virtual Member Function */
}

AutomataLabel_ptr automata_label_union(AutomataLabel_ptr self,
                                       AutomataLabel_ptr label2)
{
  error_unreachable_code(); /* Pure Virtual Member Function */
}

AutomataLabel_ptr automata_label_complement(AutomataLabel_ptr self)
{
  error_unreachable_code(); /* Pure Virtual Member Function */
}

Set_t automata_label_get_support(AutomataLabel_ptr self)
{
  error_unreachable_code(); /* Pure Virtual Member Function */
}

AutomataLabel_ptr automata_label_copy(AutomataLabel_ptr self)
{
  error_unreachable_code(); /* Pure Virtual Member Function */
}

/*!
  \brief Print the label on a file

  Print the expression contained in the label to a file

  \se The file given as input is used to print the label

  \sa automata_label_get_expr
*/

/* This function has to be changed in order to take the environment */
void automata_label_print_label(AutomataLabel_ptr self,
                                NuSMVEnv_ptr env,
                                FILE* out_file)
{
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  if (out_file) {
    print_node(wffprint, out_file, automata_label_get_expr(self));
  } else {
    automata_error_null_output_file (errmgr);
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataLabel class virtual finalizer

  Called by the class destructor
*/
static void automata_label_finalize(Object_ptr object, void* dummy)
{
  AutomataLabel_ptr self = AUTOMATA_LABEL(object);

  automata_label_deinit(self);
  FREE(self);
}



/**AutomaticEnd***************************************************************/

