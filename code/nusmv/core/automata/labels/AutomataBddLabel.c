/* ---------------------------------------------------------------------------

  This file is part of the ``labels'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Implementation of class 'AutomataBddLabel'

  \todo: Missing description

*/

#include "nusmv/core/utils/StreamMgr.h"
#include "nusmv/core/utils/ErrorMgr.h"
#include "nusmv/core/node/printers/MasterPrinter.h"
#include "nusmv/core/enc/enc.h"
#include "nusmv/core/wff/ExprMgr.h"
#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/parser/parser.h"
#include "nusmv/core/compile/compile.h"
#include "nusmv/core/utils/utils.h"

#include "nusmv/core/automata/labels/AutomataBddLabel.h"
#include "nusmv/core/automata/labels/AutomataBddLabel_private.h"
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
/* See 'AutomataBddLabel_private.h' for class 'AutomataBddLabel' definition. */

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

static void automata_bdd_label_finalize(Object_ptr object, void* dummy);
static Set_t automata_bdd_label_get_support_from_expr(Set_t support_set,
                                                      Expr_ptr expression);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataBddLabel_ptr AutomataBddLabel_create(BddEnc_ptr bdd_enc)
{
  AutomataBddLabel_ptr self = ALLOC(AutomataBddLabel, 1);
  AUTOMATA_BDD_LABEL_CHECK_INSTANCE(self);

  automata_bdd_label_init(self, bdd_enc);
  return self;
}

AutomataBddLabel_ptr AutomataBddLabel_create_from_expr(BddEnc_ptr bdd_enc,
                                                       Expr_ptr expr)
{
  AutomataBddLabel_ptr self = AutomataBddLabel_create(bdd_enc);

  AutomataLabel_set_expr(AUTOMATA_LABEL(self), expr);
  return self;
}

AutomataBddLabel_ptr AutomataBddLabel_create_from_bdd(BddEnc_ptr bdd_enc,
						      bdd_ptr bdd_expr)
{
  AutomataBddLabel_ptr self = AutomataBddLabel_create(bdd_enc);

  /* self->expr_ptr is not created at this moment,
     it will be created on the first call of automata_bdd_label_get_expr() */
  AutomataBddLabel_set_bdd(self, bdd_expr);
  return self;
}

void AutomataBddLabel_destroy(AutomataBddLabel_ptr self)
{
  AUTOMATA_BDD_LABEL_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataBddLabel class private initializer

  The AutomataBddLabel class private initializer

  \sa AutomataBddLabel_create
*/

void automata_bdd_label_init(AutomataBddLabel_ptr self, BddEnc_ptr bdd_enc)
{
  /* base class initialization */
  automata_label_init(AUTOMATA_LABEL(self));

  /* members initialization */
  automata_label_set_label_type(AUTOMATA_LABEL(self), BDD_LABEL);
  self->bdd_expr = NULL;
  self->bdd_enc = bdd_enc;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = automata_bdd_label_finalize;

  /* Overrides  base class' virtual methods: */
  OVERRIDE(AutomataLabel, set_expr) = automata_bdd_label_set_expr;
  OVERRIDE(AutomataLabel, get_expr) = automata_bdd_label_get_expr;
  OVERRIDE(AutomataLabel, is_sat) = automata_bdd_label_is_sat;
  OVERRIDE(AutomataLabel, is_model) = automata_bdd_label_is_model;
  OVERRIDE(AutomataLabel, is_true) = automata_bdd_label_is_true;
  OVERRIDE(AutomataLabel, is_equivalent) = automata_bdd_label_is_equivalent;
  OVERRIDE(AutomataLabel, is_contained) = automata_bdd_label_is_contained;
  OVERRIDE(AutomataLabel, intersection) = automata_bdd_label_intersection,
  OVERRIDE(AutomataLabel, label_union) = automata_bdd_label_union;
  OVERRIDE(AutomataLabel, complement) = automata_bdd_label_complement;
  OVERRIDE(AutomataLabel, get_support) = automata_bdd_label_get_support;
  OVERRIDE(AutomataLabel, copy) = automata_bdd_label_copy;
  OVERRIDE(AutomataLabel, print_label) = automata_bdd_label_print_label;
}


/*!
  \brief The AutomataBddLabel class private deinitializer

  The AutomataBddLabel class private deinitializer

  \sa AutomataBddLabel_destroy
*/

void automata_bdd_label_deinit(AutomataBddLabel_ptr self)
{
  /* members deinitialization */
  if(self->bdd_expr != NULL){
    DDMgr_ptr dd = BddEnc_get_dd_manager(self->bdd_enc);

    bdd_free(dd, self->bdd_expr);
  }

  /* base class deinitialization */
  automata_label_deinit(AUTOMATA_LABEL(self));
}

/*!
  \brief Set the expression of the label

  Set the expression contained in the label. When
  an expression is set the bdd that represents the expression is built.
*/

void automata_bdd_label_set_expr(AutomataLabel_ptr automata_label, Expr_ptr expr)
{
  AutomataBddLabel_ptr self = AUTOMATA_BDD_LABEL(automata_label);
  DDMgr_ptr dd = BddEnc_get_dd_manager(self->bdd_enc);

  nusmv_assert((Expr_ptr)NULL != expr);
  automata_label_set_expr(AUTOMATA_LABEL(self), expr);

  /* sets the BDD in the label */
  if (self->bdd_expr != NULL) {
    bdd_free(dd, self->bdd_expr);
  }

  self->bdd_expr = BddEnc_expr_to_bdd(self->bdd_enc, expr, Nil);
}

Expr_ptr automata_bdd_label_get_expr(AutomataLabel_ptr automata_label)
{
  Expr_ptr expr = automata_label_get_expr(automata_label);

  if (!expr) {
    AutomataBddLabel_ptr self = AUTOMATA_BDD_LABEL(automata_label);

    /* Set expr if the corresponding BDD expr is present */
    if (self->bdd_expr) {
      expr = BddEnc_bdd_to_expr(self->bdd_enc, self->bdd_expr);
      automata_label_set_expr(automata_label, expr);
    }
  }
  return expr;
}

/* The method signature is inherited from AutomataLabel "class" in which
   there's no "bdd_enc" to retrieve back the "env", which has to be a
   parameter here.
 */
void automata_bdd_label_print_label(AutomataLabel_ptr self,
				    NuSMVEnv_ptr env,
				    FILE* out_file)
{
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  if (out_file) {
    print_node(wffprint, out_file, automata_bdd_label_get_expr(self));
  } else {
    automata_error_null_output_file (errmgr);
  }
}

/* DO NOT CALL automata_label_set_expr() here, because it's slow. Instead,
   User should call automata_bdd_label_get_expr() and it internally builds the
   Expr_ptr on the fly.
 */
void AutomataBddLabel_set_bdd(AutomataBddLabel_ptr self, bdd_ptr bdd_expr)
{
  DDMgr_ptr dd = BddEnc_get_dd_manager(self->bdd_enc);

  if ((bdd_ptr) NULL != self->bdd_expr) {
    bdd_free(dd, self->bdd_expr);
  }

  nusmv_assert(bdd_expr != NULL);
  self->bdd_expr = bdd_dup(bdd_expr);
}

bdd_ptr AutomataBddLabel_get_bdd(AutomataBddLabel_ptr self)
{
  return bdd_dup(self->bdd_expr);
}

/*!
  \brief Returns true if the expression contained in the
  label is satisfiable.

  Check if the bdd that represents the label is
  0. If the bdd is not 0 then the label expression is sat.
*/

boolean automata_bdd_label_is_sat(AutomataLabel_ptr automata_label)
{
  AutomataBddLabel_ptr self = AUTOMATA_BDD_LABEL(automata_label);
  DDMgr_ptr dd = BddEnc_get_dd_manager(self->bdd_enc);

  return (bdd_isnot_false(dd, self->bdd_expr)) ? true : false;
}

/*!
  \brief Check if the expression is a model for the label.

  Returns true if the expression model is a model
  for the label. The model checking is performed conjuncting the bdd
  created from model with the internal bdd of the label, and then
  checking if the result is satisfiable.
*/

boolean automata_bdd_label_is_model(AutomataLabel_ptr automata_label,
                                    Expr_ptr model)
{
  AutomataBddLabel_ptr self;
  BddEnc_ptr bdd_enc;
  DDMgr_ptr dd;
  bdd_ptr model_bdd;
  bdd_ptr and_bdd;
  boolean is_model;

  self = AUTOMATA_BDD_LABEL(automata_label);
  nusmv_assert(model != NULL);

  bdd_enc = self->bdd_enc;
  dd = BddEnc_get_dd_manager(bdd_enc);

  model_bdd = BddEnc_expr_to_bdd(bdd_enc, model, Nil);

  and_bdd = bdd_and(dd, self->bdd_expr, model_bdd);

  if (bdd_isnot_false(dd, and_bdd))
    is_model = true;
  else is_model = false;

  bdd_free(dd, model_bdd);

  return is_model;
}

/*!
  \brief Check if label is true.

  Returns true if the automata label is true.
*/

boolean automata_bdd_label_is_true(AutomataLabel_ptr automata_label)
{
  AutomataBddLabel_ptr self;
  BddEnc_ptr bdd_enc;
  DDMgr_ptr dd;
  boolean is_true;

  self = AUTOMATA_BDD_LABEL(automata_label);

  bdd_enc = self->bdd_enc;
  dd = BddEnc_get_dd_manager(bdd_enc);

  if (bdd_is_true(dd, self->bdd_expr))
    is_true = true;
  else is_true = false;

  return is_true;
}

/*!
  \brief Checks if two label are equivalent

  Two labels are equivalent if the bdds that
  represent them are the same.
*/

boolean automata_bdd_label_is_equivalent(AutomataLabel_ptr automata_label1,
                                         AutomataLabel_ptr automata_label2)
{
  AutomataBddLabel_ptr self;
  AutomataBddLabel_ptr label2;

  self = AUTOMATA_BDD_LABEL(automata_label1);
  label2 = AUTOMATA_BDD_LABEL(automata_label2);

  return self->bdd_expr == label2->bdd_expr;
}

/*!
  \brief Check if label1 is contained in label 2.

  Label1 is contained in label2 if label1 entails
  label2.
*/

boolean automata_bdd_label_is_contained(AutomataLabel_ptr automata_label,
                                        AutomataLabel_ptr automata_label2)
{
  AutomataBddLabel_ptr self;
  AutomataBddLabel_ptr label2;
  BddEnc_ptr bdd_enc;
  DDMgr_ptr dd;

  self = AUTOMATA_BDD_LABEL(automata_label);
  label2 = AUTOMATA_BDD_LABEL(automata_label2);

  /* Performs the and of BDDs from self and label2. */
  bdd_enc = self->bdd_enc;
  dd = BddEnc_get_dd_manager(bdd_enc);

  return bdd_entailed(dd, self->bdd_expr, label2->bdd_expr);
}

/*!
  \brief Performs the intersection of two label

  A new AutomataBddLabel is created and its
  expression and bdd are set. The expression of the new label is the
  intersection of the expressions of self and label2. The bdd of the new
  label is the and of the bdds of self and label2.
*/

AutomataLabel_ptr automata_bdd_label_intersection(AutomataLabel_ptr automata_label,
                                                  AutomataLabel_ptr automata_label2)
{
  AutomataBddLabel_ptr self;
  AutomataBddLabel_ptr label2;

  Expr_ptr new_expression;
  AutomataBddLabel_ptr intersection_label;
  BddEnc_ptr bdd_enc;
  DDMgr_ptr dd;
  NuSMVEnv_ptr env;
  ExprMgr_ptr exprs;

  self = AUTOMATA_BDD_LABEL(automata_label);
  label2 = AUTOMATA_BDD_LABEL(automata_label2);

  /* Performs the and of BDDs from self and label2. */
  bdd_enc = self->bdd_enc;
  dd = BddEnc_get_dd_manager(bdd_enc);
  env = EnvObject_get_environment(ENV_OBJECT(bdd_enc));
  exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));

  nusmv_assert(label2 != NULL);

  intersection_label = AutomataBddLabel_create(self->bdd_enc);

  /* Sets the expression of the new label as the intersection of the
     expressions of self and label2.
     Expression is set without building the BDD in the resulting label.
  */
  new_expression = ExprMgr_and(exprs, automata_label_get_expr(automata_label),
                            automata_label_get_expr(automata_label2));
  automata_label_set_expr(AUTOMATA_LABEL(intersection_label), new_expression);

  intersection_label->bdd_expr = bdd_and(dd, self->bdd_expr, label2->bdd_expr);

  return AUTOMATA_LABEL(intersection_label);
}

/*!
  \brief Creates a new label that is the union of the two
  labels in input

  A new AutomataBddLabel is created and its
  expression and bdd are set. The expression of the new label is the
  union of the expressions of self and label2. The bdd of the new
  label is the union of the bdds of self and label2.
*/

AutomataLabel_ptr automata_bdd_label_union(AutomataLabel_ptr automata_label1,
                                           AutomataLabel_ptr automata_label2)
{
  AutomataBddLabel_ptr self;
  AutomataBddLabel_ptr label2;
  Expr_ptr new_expression;
  AutomataBddLabel_ptr union_label;
  BddEnc_ptr bdd_enc;
  DDMgr_ptr dd;
  NuSMVEnv_ptr env;
  ExprMgr_ptr exprs;

  self = AUTOMATA_BDD_LABEL(automata_label1);
  label2 = AUTOMATA_BDD_LABEL(automata_label2);
  nusmv_assert(label2 != NULL);

  union_label = AutomataBddLabel_create(self->bdd_enc);

  /* Performs the or of BDDs from self and label2. */
  bdd_enc = self->bdd_enc;
  dd = BddEnc_get_dd_manager(bdd_enc);
  env = EnvObject_get_environment(ENV_OBJECT(bdd_enc));
  exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));

  /* Sets the expression of the new label as the union of the
     expressions of self and label2.
     Expression is set without building the BDD in the resulting label.
  */
  new_expression = ExprMgr_or(exprs, automata_label_get_expr(automata_label1),
                           automata_label_get_expr(automata_label2));
  automata_label_set_expr(AUTOMATA_LABEL(union_label), new_expression);


  union_label->bdd_expr = bdd_or(dd, self->bdd_expr, label2->bdd_expr);

  return AUTOMATA_LABEL(union_label);
}

/*!
  \brief Complement a label

  Creates a new label that is the complement of
  label self. The expression of the new label is obtained negating the
  expression of self and the bdd of the new label is obtained negating
  the bdd of self.
*/

AutomataLabel_ptr automata_bdd_label_complement(AutomataLabel_ptr automata_label)
{
  AutomataBddLabel_ptr self;
  Expr_ptr new_expression;
  AutomataBddLabel_ptr complemented_label;
  BddEnc_ptr bdd_enc;
  DDMgr_ptr dd;
  NuSMVEnv_ptr env;
  ExprMgr_ptr exprs;

  self = AUTOMATA_BDD_LABEL(automata_label);

  complemented_label = AutomataBddLabel_create(self->bdd_enc);

  /* Performs the complement of BDD from self label. */
  bdd_enc = self->bdd_enc;
  dd = BddEnc_get_dd_manager(bdd_enc);
  env = EnvObject_get_environment(ENV_OBJECT(bdd_enc));
  exprs = EXPR_MGR(NuSMVEnv_get_value(env, ENV_EXPR_MANAGER));


  /* Sets the expression of the new label as the complement of self label.
     Expression is set without building the BDD in the resulting label.
  */
  new_expression = ExprMgr_not(exprs, automata_label_get_expr(AUTOMATA_LABEL(self)));
  automata_label_set_expr(AUTOMATA_LABEL(complemented_label), new_expression);

  complemented_label->bdd_expr = bdd_not(dd, self->bdd_expr);

  return AUTOMATA_LABEL(complemented_label);
}

/*!
  \brief Returns a set of all the variables contained in the bdd

  Builds the set of variables contained in the
  label. First from the bdd of the label is built the support bdd,
  that contains a conjunction of all the variables. Then the support
  bdd is converted in an expression, used to extract the set of variables.

  \sa automata_bdd_label_get_support_from_expr
*/

Set_t automata_bdd_label_get_support(AutomataLabel_ptr automata_label)
{
  AutomataBddLabel_ptr self;
  BddEnc_ptr bdd_enc;
  DDMgr_ptr dd;
  bdd_ptr support;
  Expr_ptr support_expr;
  Set_t support_set;

  self = AUTOMATA_BDD_LABEL(automata_label);

  /* Performs the or of BDDs from self and label2. */
  bdd_enc = self->bdd_enc;
  dd = BddEnc_get_dd_manager(bdd_enc);
  support_set = Set_MakeEmpty();

  if(self->bdd_expr != NULL){
    /* get the support from bdd and builds the correspondent expression */
    support = bdd_support(dd, self->bdd_expr);
    support_expr = BddEnc_bdd_to_expr(bdd_enc, support);

/*     StreamMgr_print_error(streams,  "Extracting support from expr:"); */
/*     StreamMgr_nprint_error(streams, wffprint, "%N", support_expr); */
/*     StreamMgr_print_error(streams,  "\n"); */

    bdd_free(dd, support);

    /* recursively extracts variables from expression in support_set */
    support_set = automata_bdd_label_get_support_from_expr(support_set, support_expr);
  }

  return support_set;
}

/*!
  \brief Creates a copy of the label

  Creates and return a new copy of the label.
*/

AutomataLabel_ptr automata_bdd_label_copy(AutomataLabel_ptr automata_label)
{
  AutomataBddLabel_ptr self;
  Expr_ptr new_expression;
  AutomataBddLabel_ptr copy_label;

  self = AUTOMATA_BDD_LABEL(automata_label);

  copy_label = AutomataBddLabel_create(self->bdd_enc);

  /* Copy the reference to expression of the new label. */
  new_expression = automata_label_get_expr(AUTOMATA_LABEL(self));
  automata_label_set_expr(AUTOMATA_LABEL(copy_label), new_expression);

  /* Copy the BDD from self label. */
  copy_label->bdd_expr = bdd_dup(self->bdd_expr);

  return AUTOMATA_LABEL(copy_label);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The AutomataBddLabel class virtual finalizer

  Called by the class destructor
*/
static void automata_bdd_label_finalize(Object_ptr object, void* dummy)
{
  AutomataBddLabel_ptr self = AUTOMATA_BDD_LABEL(object);

  automata_bdd_label_deinit(self);
  FREE(self);
}

/*!
  \brief Returns the set of variables contained in the expression

  Given an expression that contains a conjunction
  of variables returns a set that contains these variables. The
  expression must be in the form "a & b & c..." where every conjunct
  is a variable. The function is called recursively on expression,
  adding a variable to the support_set in the base case when
  expression is an ATOM.
*/

static Set_t automata_bdd_label_get_support_from_expr(Set_t support_set,
                                                      Expr_ptr expression)
{
  switch (node_get_type(expression)) {
  case TRUEEXP:
    break;
  case FALSEEXP:
    break;
  case ATOM:
    support_set = Set_AddMember(support_set, expression);
    break;
  case DOT:
    /* right child of DOT is an ATOM */
    support_set = automata_bdd_label_get_support_from_expr(support_set, cdr(expression));
    break;
  case AND:
    support_set = automata_bdd_label_get_support_from_expr(support_set, car(expression));
    support_set = automata_bdd_label_get_support_from_expr(support_set, cdr(expression));
    break;
  default:
    /* Another base case. TO CHECK THIS BEHAVIOUR.
       When a non boolean type is found this has to be treated as an
       atom.
     */
    support_set = Set_AddMember(support_set, expression);
/*     StreamMgr_print_error(streams,  "Unexpected type: \"%d\"\n", node_get_type(expression)); */
/*     ErrorMgr_nusmv_exit(errmgr, 1); */
  }

  return support_set;
}

/**AutomaticEnd***************************************************************/

