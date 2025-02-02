/* ---------------------------------------------------------------------------


  This file is part of the ``utils'' package.
  %COPYRIGHT%

-----------------------------------------------------------------------------*/

/*!
  \author Sergio Mover
  \brief Error routines for the automata package

  \todo: Missing description

*/

#include "nusmv/core/utils/ErrorMgr.h"
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
static int get_char_representation_dim(int int_var);


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

void automata_error_unknown_label_type (ErrorMgr_ptr errmgr,
                                        Automaton_Label_Type type)
{
  /* The error string is the message
     AUTOMATA_ERROR_UNKOWN_LABEL_TYPE_MSG, the type of the label, and
     the two character ':' and ' '. */
  char* error_string = 
    ALLOC(char, 
          strlen(AUTOMATA_ERROR_UNKOWN_LABEL_TYPE_MSG) +
          get_char_representation_dim(type) +
          3 + /* ';', ' ', ' ' */
          1); /* ending 0 */          
  sprintf(error_string, "%s: %d ", AUTOMATA_ERROR_UNKOWN_LABEL_TYPE_MSG, type);
  ErrorMgr_internal_error(errmgr, "%s", error_string);
}

void automata_error_null_output_file (ErrorMgr_ptr errmgr)
{
  /* The error string is the message
     AUTOMATA_ERROR_NULL_OUTPUT_FILE_MSG, and ' '. */
  char* error_string = 
    ALLOC(char, 
          strlen(AUTOMATA_ERROR_NULL_OUTPUT_FILE_MSG) +
          1 + /* ' ' */
          1); /* ending 0 */          
  sprintf(error_string, "%s ", AUTOMATA_ERROR_NULL_OUTPUT_FILE_MSG);
  ErrorMgr_internal_error(errmgr, "%s", error_string);
}

void automata_error_wrong_label_type (ErrorMgr_ptr errmgr,
                                      Automaton_Label_Type used_label_type,
                                      Automaton_Label_Type new_label_type)
{
  /* The error string is the message
     AUTOMATA_ERROR_WRONG_LABEL_TYPE_MSG, the type of the algorithm, and
     the two character ':' and ' '. */
  char* error_string = 
    ALLOC(char, 
          strlen(AUTOMATA_ERROR_UNKOWN_ALGORITHM_TYPE_MSG) +
          get_char_representation_dim(used_label_type) +
          get_char_representation_dim(new_label_type) +
          6 + /* '- old '  */
          9 + /* ' and new ' */
          1); /* ending 0 */          
  sprintf(error_string, "%s- old %d and new %d", 
          AUTOMATA_ERROR_WRONG_LABEL_TYPE_MSG,
          used_label_type,
          new_label_type);
  ErrorMgr_internal_error(errmgr, "%s", error_string);
}

void 
automata_error_unknown_algorithm_type (ErrorMgr_ptr errmgr,
                                       Automaton_Algorithm_Type algorithm_type)
{
  /* The error string is the message
     AUTOMATA_ERROR_UNKOWN_ALGORITHM_TYPE_MSG, the type of the algorithm, and
     the two character ':' and ' '. */
  char* error_string =
    ALLOC(char,
          strlen(AUTOMATA_ERROR_UNKOWN_ALGORITHM_TYPE_MSG) +
          get_char_representation_dim(algorithm_type) +
          3 + /* ';', ' ', ' ' */
          1); /* ending 0 */
  sprintf(error_string, "%s: %d ", 
          AUTOMATA_ERROR_UNKOWN_ALGORITHM_TYPE_MSG, 
          algorithm_type);
  ErrorMgr_internal_error(errmgr, "%s", error_string);
}

void 
automata_error_unknown_reduction_algo_type (ErrorMgr_ptr errmgr,
                                            Automaton_ReductionAlgorithm_Type type)
{
  /* The error string is the message
     AUTOMATA_ERROR_UNKOWN_REDALGO_TYPE_MSG, the type of the algorithm, and
     the two character ':' and ' '. */
  char* error_string = 
    ALLOC(char, 
          strlen(AUTOMATA_ERROR_UNKOWN_REDALGO_TYPE_MSG) +
          get_char_representation_dim(type) +
          3 + /* ';', ' ', ' ' */
          1); /* ending 0 */
  sprintf(error_string, "%s: %d ",
          AUTOMATA_ERROR_UNKOWN_REDALGO_TYPE_MSG, 
          type);
  ErrorMgr_internal_error(errmgr, "%s", error_string);
}

void automata_error_unknown_sere_operator (ErrorMgr_ptr errmgr,
                                           PslOp psl_op)
{
  /* The error string is the message
     AUTOMATA_ERROR_UNKOWN_SERE_OP, the type of the operator, and
     the two character ':' and ' '. */
  char* error_string = 
    ALLOC(char, 
          strlen(AUTOMATA_ERROR_UNKOWN_SERE_OP) +
          get_char_representation_dim(psl_op) +
          3 + /* ';', ' ', ' ' */
          1); /* ending 0 */
  sprintf(error_string, "%s: %d ", AUTOMATA_ERROR_UNKOWN_SERE_OP, psl_op);
  ErrorMgr_internal_error(errmgr, "%s", error_string);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief Return the number of characters needed to represent
  int_var in a string.

  NOTE: the returned count does not take in account the eventually
  ending 0
*/

static int get_char_representation_dim (int int_var)
{
  int suffix_dim = 0;
  int_var = int_var == 0 ? 1 : int_var;;
  while(int_var != 0) {
    int_var = int_var / 10;
    suffix_dim = suffix_dim + 1;
  }    

  return suffix_dim;
}


