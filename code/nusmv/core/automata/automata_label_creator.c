/* ---------------------------------------------------------------------------

  This file is part of the ``automata'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Provides the implementation for the functions needed
  for creating a label given its type.

  \todo: Missing description

*/

#include "nusmv/core/automata/automata_label_creator.h"

#include "nusmv/core/automata/labels/AutomataBddLabel.h"
#include "nusmv/core/automata/labels/AutomataBddLabelCombinator.h"
#include "nusmv/core/automata/labels/AutomataHybridLabel.h"

#include "nusmv/core/automata/utils/automata_error.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


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

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

AutomataLabel_ptr automata_create_label(Automaton_Label_Type label_type,
                                        BddEnc_ptr enc)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(enc));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  switch (label_type){
  case BDD_LABEL:
    BDD_ENC_CHECK_INSTANCE(enc);

    return AUTOMATA_LABEL(AutomataBddLabel_create(enc));
    break;

  case HYBRID_LABEL:
    nusmv_assert(BDD_ENC(NULL) == enc);

    return AUTOMATA_LABEL(AutomataHybridLabel_create());
    break;

  default:
    automata_error_unknown_label_type(errmgr, label_type);
    error_unreachable_code();
    return NULL;
  }
}

AutomataLabel_ptr
automata_create_label_from_expr(Automaton_Label_Type label_type,
                                BddEnc_ptr enc, Expr_ptr expr)
{
  AutomataLabel_ptr label;

  nusmv_assert(HYBRID_LABEL != label_type);

  if (BDD_LABEL == label_type) {
    BDD_ENC_CHECK_INSTANCE(enc);
  }

  label = automata_create_label(label_type, enc);
  AutomataLabel_set_expr(label, expr);

  return label;
}

/*!
  \brief Given a type of label creates a new instance of
  combinator that manage that type of labels.

  
*/

/* WARNING [MD] This function should take the env as parameter, and not the
   enc */
AutomataLabelCombinator_ptr automata_create_combinator(Automaton_Label_Type label_type,
                                                       BddEnc_ptr enc)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(enc));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  switch (label_type){
  case BDD_LABEL:
    return AUTOMATA_LABEL_COMBINATOR(AutomataBddLabelCombinator_create(env));
    break;

  case HYBRID_LABEL:
    error_unreachable_code_msg("Feature not available.\n");

  default:
    automata_error_unknown_label_type(errmgr, label_type);
    error_unreachable_code();
    return NULL;
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/



