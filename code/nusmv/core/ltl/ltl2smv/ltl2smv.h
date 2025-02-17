/* ---------------------------------------------------------------------------


  This file is part of the ``ltl2smv'' package of NuSMV version 2.
  Copyright (C) 2005 by FBK-irst.

  NuSMV version 2 is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  NuSMV version 2 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

  For more information on NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>.

-----------------------------------------------------------------------------*/

/*!
  \author Andrei Tchaltsev, Marco Roveri
  \brief A function to run ltl2smv routine.

  Here we perform a convertion from LTL formula to SMV module.
  The invoker provides the LTL fromula in the form of node_ptr expression.
  The function 'ltl2smv' will convert this formula to a SMV module, which is
  also node_ptr MODULE.

  This file provides routines for reducing LTL model
  checking to CTL model checking. This work is based on the work
  presented in \[1\]<br>

  <ul><li> O. Grumberg E. Clarke and K. Hamaguchi.  <cite>Another Look
          at LTL Model Checking</cite>. <em>Formal Methods in System
          Design, 10(1):57--71, February 1997.</li>
  </ul>

*/


#ifndef __NUSMV_CORE_LTL_LTL2SMV_LTL2SMV_H__
#define __NUSMV_CORE_LTL_LTL2SMV_LTL2SMV_H__

#include "nusmv/core/node/node.h"
#include "nusmv/core/compile/symb_table/SymbTable.h"

/*!
  \brief
*/
typedef struct Ltl2SmvPrefixes_TAG
{
  char* pre_prefix;
  char* prefix_name;
  char* ltl_module_base_name;
} Ltl2SmvPrefixes;

/*---------------------------------------------------------------------------*/
/* MACRO declaration                                                         */
/*---------------------------------------------------------------------------*/
/**MAcro********************************************************************

  Synopsis    [This macro is a string represeting the prefix of the name
  of modules generated by ltl2smv procedure]

  Description []

  SeeAlso     [ltl2smv]

******************************************************************************/

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
#define LTL_MODULE_BASE_NAME "ltl_spec_"

/*!
  \brief translation options used in LTL2SMV

  \sa ltl2smv_core_extended
*/

/* Enable to use legacy version of ltl2smv that does not require
   the symbols to be declared in the symbol table.
   see ltl2smvMain.c
*/
#define LTL2SMV_OPTION_NO_SYMBOL_TABLE (1 << 0)

/* Whenever enabled, `ltl2smv_core_extended` may return extra outputs,
   depending on other options. In this case, the `extra_output` argument
   of `ltl2smv_core_extended` must be non-NULL.
 */
#define LTL2SMV_OPTION_EXTRA_OUTPUT (1 << 1)

/* Whenever enabled, LTL2SMV is optimized by considering the
   polarity of LTL sub-expressions. This results into smaller
   justice sets.
 */
#define LTL2SMV_OPTION_USE_POLARITY (1 << 2)

/* Whenever enabled, it builds a monitor
   such that there is a single fairness condition (standard Buechi)
   instead of possibly many (generalized Buechi).
 */
#define LTL2SMV_OPTION_SINGLE_JUSTICE (1 << 3)

/* Whenever enabled, there will be no initial assignments (to false)
   for past variables generated by `ltl2smv`, whose output is a Büchi
   automaton accepting the language of the LTL property WITH CONFIGURABLE
   MEMORIES OF THE PAST. To actually config the memories (of the past),
   a conjunction of all past variables (with possibly different values)
   can be further AND into the initial condition of Büchi automaton.
   The list of all past variables can be retrieved as a NodeList_ptr
   by further setting the LTL2SMV_OPTION_EXTRA_OUTPUT option, which
   also gives the list of all future variables of independent interests.

   If LTL2SMV_OPTION_EXTRA_OUTPUT is also set, the caller must allocate
   an array of NodeList_ptr of the length at least 2, to receive the
   list of future and past variables (for setting the initial assignments
   later).
 */
#define LTL2SMV_OPTION_NO_INIT_PAST (1 << 4)

/* Whenever enabled, it builds a transition relation with implication
   instead of biimplications
 */
#define LTL2SMV_OPTION_IMPLICATION_IN_TRANSITION (1 << 5)

/* Use safety ltl to smv algorithm instead of standard ltl to smv
 */
#define LTL2SMV_OPTION_SAFETY (1 << 6)

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/*!
  \brief The routine converting an LTL formula to am SMV Module.

  The parameters are:
  symb_table - symbol table containing all symbols used in in_ltl_expr;
               See `ltl2smv_core_extended` for generating the smv module without
               using the symbol table.
  uniqueId - is a positive integer number, which is converted to string
            representation and then used as a part of the generated
            SMV models name.  (_LTL_uniqueId_SPECF_N_)
  in_ltl_expr - the input LTL Formula in the form of node_ptr. The
            expression should be flattened, rewritten wrt input
            variables, negated, i.e. be ready for conversion without
            any additional transformations

  The function also performs memory-sharing on the input expression,
  since the returned module may be smaller if the memory of
  expression is shared. So DO NOT modify the output expressions.

  But the invoker can modify the declarations and module itself (but
  not their expressions). See generate_smv_module for more info.

  The return value is the created SMV module in the form of node_ptr.

  The computed module may contain more than one justice condition.


  \sa ltl2smv_core
*/
node_ptr ltl2smv(SymbTable_ptr symb_table,
                 unsigned int uniqueId,
                 node_ptr in_ltl_expr);

/*!
  \brief The routine converting a safety LTL formula to am SMV Module using
         the safety ltl to smv algorithm.

  The parameters are:
  symb_table - symbol table containing all symbols used in in_ltl_expr;
               See `ltl2smv_core_extended` for generating the smv module without
               using the symbol table.
  uniqueId - is a positive integer number, which is converted to string
            representation and then used as a part of the generated
            SMV models name.  (_LTL_uniqueId_SPECF_N_)
  in_ltl_expr - the input LTL Formula in the form of node_ptr. The
            expression should be flattened, rewritten wrt input
            variables, negated, i.e. be ready for conversion without
            any additional transformations

  The function also performs memory-sharing on the input expression,
  since the returned module may be smaller if the memory of
  expression is shared. So DO NOT modify the output expressions.

  But the invoker can modify the declarations and module itself (but
  not their expressions). See generate_smv_module for more info.

  The return value is the created SMV module in the form of node_ptr.

  The computed module may contain more than one justice condition.


  \sa ltl2smv_core
*/
node_ptr safetyltl2smv(SymbTable_ptr symb_table, unsigned int uniqueId,
                       node_ptr in_ltl_expr, node_ptr *out_invarspec);

/*!
  \brief The main routine converting an LTL formula to am SMV Module.

  The parameters are:
  symb_table - symbol table containing all symbols used in in_ltl_expr;
               See `ltl2smv_core_extended` for generating the smv module without
               using the symbol table.
  uniqueId - is a positive integer number, which is converted to string
            representation and then used as a part of the generated
            SMV models name.  (_LTL_uniqueId_SPECF_N_)
  in_ltl_expr - the input LTL Formula in the form of node_ptr. The
            expression should be flattened, rewritten wrt input
            variables, negated, i.e. be ready for conversion without
            any additional transformations
  safety - is a boolean that when true uses the algorithm safety ltl
            to smv instead of the standard ltl to smv
  single_justice - is a boolean that when true, it builds a monitor
            such that there is a single fairness condition instead of
            possibly many.

  The function also performs memory-sharing on the input expression,
  since the returned module may be smaller if the memory of
  expression is shared. So DO NOT modify the output expressions.

  But the invoker can modify the declarations and module itself (but
  not their expressions). See generate_smv_module for more info.

  The return value is the created SMV module in the form of node_ptr.

*/
node_ptr ltl2smv_core(SymbTable_ptr symb_table, unsigned int uniqueId,
                      node_ptr in_ltl_expr, boolean single_justice,
                      boolean safety, const Ltl2SmvPrefixes *prefixes,
                      node_ptr *spec);

/*!
  \brief An extended version of `ltl2smv_core`, Added/changed parameters:

  options - translation options, as combination of LTL2SMV_OPTION_ macros
  extra_output - array of extra outputs (NodeList_ptr):

  - If LTL2SMV_OPTION_NO_INIT_PAST is set, the first two elements of
    `extra_output` will be filled. extra_output[0] holds the list of all
    future variables, extra_output[1] holds the list of all past variables.

  - If LTL2SMV_OPTION_NO_SYMBOL_TABLE is set, the symbol table can be empty;
    all symbols used in in_ltl_expr are assumed to be declared with a correct type
    in some external-module. See ltl2smvMain.

  The extra_output must be freed by the caller.

  \sa ltl2smv_core, LTL2SMV_OPTION_ macros
*/
node_ptr
ltl2smv_core_extended(SymbTable_ptr symb_table, unsigned int uniqueId,
                      node_ptr in_ltl_expr,
                      const Ltl2SmvPrefixes *prefixes,
                      const int options, NodeList_ptr *extra_output,
                      node_ptr *spec);

/*!
  \brief The routine converting an LTL formula to am SMV Module.

  The parameters are:
  symb_table - symbol table containing all symbols used in in_ltl_expr;
               See `ltl2smv_core_extended` for generating the smv module without
               using the symbol table.
  uniqueId - is a positive integer number, which is converted to string
            representation and then used as a part of the generated
            SMV models name.  (_LTL_uniqueId_SPECF_N_)
  in_ltl_expr - the input LTL Formula in the form of node_ptr. The
            expression should be flattened, rewritten wrt input
            variables, negated, i.e. be ready for conversion without
            any additional transformations

  The function also performs memory-sharing on the input expression,
  since the returned module may be smaller if the memory of
  expression is shared. So DO NOT modify the output expressions.

  But the invoker can modify the declarations and module itself (but
  not their expressions). See generate_smv_module for more info.

  The return value is the created SMV module in the form of node_ptr.

  The computed module will contain less than one justice condition.


*/
node_ptr ltl2smv_single_justice(SymbTable_ptr env,
                                unsigned int uniqueId,
                                node_ptr in_ltl_expr);


/*!
  \brief perform memory sharing of sub-expression

  This method is required for ltl2smv.
  In ltl2smv the symbol table is not populated.
  This method simply performs the memory sharing of the nodes:
  does not need a symbol table.
  (see issue 709)
  This method can be called with a NULL symbol table as 
  last parameter: this however forbids some trivial simplifications
  during the construction of the new memory-shared formula. 
  (see issue 748 and `ExprMgr_resolve`)
*/
node_ptr perform_memory_sharing(const NuSMVEnv_ptr env, node_ptr t, SymbTable_ptr st);


/*!
  \brief The routine for converting multiple LTL formulas together (by Chun Tian)

  The parameters are:
  symb_table - symbol table containing all symbols used in in_ltl_expr;
               See `ltl2smv_core_extended` for generating the smv module without
               using the symbol table.

  uniqueId - is a positive integer number, which is converted to string
            representation and then used as a part of the generated
            SMV models name (_LTL_uniqueId_SPECF_N_), aka Elementary Variables.

      NOTE: duplicated sub-formulas in multiple LTL formulas are converted to
            single elementary variables shared by all formulas (Thus the total
            number of additional elementary variables may be reduced when
            multiple LTL formulas highly resemble each other.

  in_ltl_exprs - the input LTL formulas as elements of NodeList_ptr.

   options - translation options, as combination of LTL2SMV_OPTION_ macros
   extra_output - array of extra outputs (NodeList_ptr):

  - If LTL2SMV_OPTION_NO_INIT_PAST and LTL2SMV_OPTION_EXTRA_OUTPUT is set,
    the first two elements of `extra_output` will be filled. extra_output[0] holds
    the list of all future variables (of all LTL formulas), extra_output[1] holds
    the list of all past variables (of all LTL formulas).

  - If LTL2SMV_OPTION_NO_SYMBOL_TABLE is set, the symbol table can be empty;
    all symbols used in in_ltl_expr are assumed to be declared with a correct type
    in some external-module. See ltl2smvMain.

  The extra_output must be freed by the caller.

  The function also performs memory-sharing on the input expression,
  since the returned module may be smaller if the memory of
  expression is shared. So DO NOT modify the output expressions.

  But the invoker can modify the declarations and module itself (but
  not their expressions). See generate_smv_module for more info.

  The return value is the NodeList_ptr of translated SMV models, each
  corresponding to the input LTL formula provided in "in_ltl_exprs".

  \sa ltl2smv_core_extended
*/
NodeList_ptr ltl2smv_core_multiple (SymbTable_ptr symb_table,
                                    unsigned int uniqueId,
                                    const NodeList_ptr in_ltl_exprs,
                                    const Ltl2SmvPrefixes* prefixes,
                                    const int options,
                                    NodeList_ptr *extra_output);

#endif /* __NUSMV_CORE_LTL_LTL2SMV_LTL2SMV_H__ */
