/* ---------------------------------------------------------------------------


  This file is part of the ``compile.type_checking.checkers'' package
  of NuSMV version 2. Copyright (C) 2006 by  FBK-irst.

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
  \author Cavada Roberto
  \brief The private interface of the checkers sub-package.

  This package contains the functions required to
  the checkers sub-package internally

*/

#ifndef __NUSMV_CORE_COMPILE_TYPE_CHECKING_CHECKERS_CHECKERS_INT_H__
#define __NUSMV_CORE_COMPILE_TYPE_CHECKING_CHECKERS_CHECKERS_INT_H__


#include "nusmv/core/compile/type_checking/checkers/CheckerBase.h"

#include "nusmv/core/node/node.h"
#include "nusmv/core/opt/opt.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*!
  \brief A set of constants, each of which identifies
  particular kinds of possible type system violations.


  These constants are one of the parameters passed to a violation
  handler when a type system violation is encountered.
  Depending on the kind of the type violation, the violation handler
  will output an error or warning message or possibly ignore the
  violation.
*/

typedef enum
  {
    TC_VIOLATION_FIRST, /* This MUST be the first! */
    /* --------------------------------------------------------------------- */

    TC_VIOLATION_UNDEF_IDENTIFIER, /* undefined identifier is met */
    TC_VIOLATION_AMBIGUOUS_IDENTIFIER, /* identifier has two interpretations */
    TC_VIOLATION_TYPE_MANDATORY,   /* the most general type system
                                      violation, and this is mandatory
                                      to report
                                   */
    TC_VIOLATION_TYPE_BACK_COMP,   /* type system is violated, but it
                                      was not an error before the
                                      introduction of the type
                                      checking package.
                                   */
    TC_VIOLATION_TYPE_WARNING,    /* type system is violated, but is
                                     NOT an error, i.e. just
                                     a warning is output.
                                  */
    TC_VIOLATION_OUT_OF_WORD_WIDTH, /* access to the bits outside
                                       of the Word width */
    TC_VIOLATION_INCORRECT_WORD_WIDTH, /* incorrectly formed Word type */

    TC_VIOLATION_OUT_OF_WORDARRAY_WIDTH, /* the address or
                                            value word widths are not
                                            the specified ones */
    TC_VIOLATION_INCORRECT_WORDARRAY_WIDTH,/* incorrectly formed WordArray type*/

    TC_VIOLATION_DUPLICATE_CONSTANTS, /* duplicate constants in an enum type */

    TC_VIOLATION_ATTIME_NESTED,  /* nested attime operator */
    TC_VIOLATION_ATTIME_NUM_REQ, /* time must be a constant number */

    TC_VIOLATION_PARAMS_NUM_ERROR, /* Invalid number of parameters in
                                    NFunctions calls*/

    TC_VIOLATION_PARAMS_TYPE_ERROR, /* Invalid number of parameters in
                                       NFunctions calls*/

    TC_VIOLATION_UNCONSTANT_EXPRESSION,

    TC_VIOLATION_INVALID_RANGE,

    TC_VIOLATION_FORBIDDEN_CLOCK, /* Clock type not allowed in expression */

    /* --------------------------------------------------------------------- */
    TC_VIOLATION_LAST, /* This MUST be the last! */

  } _TypeSystemViolation;


typedef int TypeSystemViolation;

/*!
  \brief The type of a type violation handler function.

  The violation handler function is invoked when
  an expression being checked violates the type system.

  The violation function takes as parameters the checker it is begin
  called, a kind of the type system violation (see
  TypeSystemViolation), and the expression where this violation has
  been detected.  The function returns a boolean value indicating
  whether the given kind of violation should be considered as an
  error, i.e. the type checking has to terminate. So, if the violation
  handler returns false, the type checking will try to continue.

  In the case of violations related to the incorrectly formed types,
  the input expression should be CONS with the variable name
  (the problematic type belongs to) as the left child and
  the type's body as the left child. The line info of this expression
  should be correct (suitable for error messages).

  NB: Not all kinds of type system violation may be ignored.
*/
typedef boolean
(*TypeCheckingViolationHandler_ptr) (CheckerBase_ptr checker,
                                     TypeSystemViolation violation,
                                     node_ptr expression);



/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/*!
  \brief Returns true iff given violation number is
  supported by the system.


*/
boolean
TypeSystemViolation_is_valid(TypeSystemViolation violation);



#endif /* __NUSMV_CORE_COMPILE_TYPE_CHECKING_CHECKERS_CHECKERS_INT_H__ */
