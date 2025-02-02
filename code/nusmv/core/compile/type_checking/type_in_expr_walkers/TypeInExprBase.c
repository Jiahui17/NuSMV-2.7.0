/* ---------------------------------------------------------------------------

  This file is part of the ``compile.type_checking.checkers'' package
  of NuSMV version 2. Copyright (C) 2006 by FBK-irst.

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
  \author Enrico Magnago
  \brief Implementaion of class 'TypeInExprBase'

  \todo: Missing description

*/


#include "nusmv/core/utils/ErrorMgr.h"
#include "nusmv/core/compile/type_checking/type_in_expr_walkers/TypeInExprBase.h"
#include "nusmv/core/compile/type_checking/type_in_expr_walkers/TypeInExprBase_private.h"
#include "nusmv/core/compile/type_checking/TypeChecker_private.h"

#include "nusmv/core/utils/utils.h"
#include "nusmv/core/utils/error.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'TypeInExprBase_private.h' for class 'TypeInExprBase' definition.     */

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

static void type_in_expr_base_finalize(Object_ptr object, void* dummy);

static Object_ptr type_in_expr_base_copy(const Object_ptr self);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

VIRTUAL node_ptr TypeInExprBase_find_type_in_expr(const TypeInExprBase_ptr self,
                                                  const node_ptr expression,
                                                  const node_ptr context,
                                                  const SymbType_ptr type)
{
  assert(self->find_type_in_expr != NULL);
  return self->find_type_in_expr(self, expression, context, type);
}


void TypeInExprBase_destroy(TypeInExprBase_ptr self)
{
  TYPE_IN_EXPR_BASE_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void type_in_expr_base_init(const TypeInExprBase_ptr self,
                            const TypeChecker_ptr type_checker,
                            const char* name, const int low, const size_t num,
                            const boolean can_handle_null)
{
  const NuSMVEnv_ptr env = ENV_OBJECT_GET_ENV(type_checker);

  NUSMV_ENV_CHECK_INSTANCE(env);

  node_walker_init(NODE_WALKER(self), env, name, low, num,
                   can_handle_null);

  /* members initialisation */
  self->type_checker = type_checker;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = type_in_expr_base_finalize;
  OVERRIDE(Object, copy) = type_in_expr_base_copy;
  OVERRIDE(TypeInExprBase, find_type_in_expr) =
    type_in_expr_base_find_type_in_expr;
}


void type_in_expr_base_deinit(TypeInExprBase_ptr self)
{
  node_walker_deinit(NODE_WALKER(self));
}


void type_in_expr_base_copy_aux(const TypeInExprBase_ptr self,
                                TypeInExprBase_ptr other)
{
  /* base class copy constructor */
  node_walker_copy_aux(NODE_WALKER(self), NODE_WALKER(other));

  /* members copy */
  other->type_checker = self->type_checker; /* we do not own the type_checker */

  /* virtual methods copy */
  other->find_type_in_expr = self->find_type_in_expr;
}

node_ptr type_in_expr_base_find_type_in_expr(const TypeInExprBase_ptr self,
                                             const node_ptr exp,
                                             const node_ptr ctx,
                                             const SymbType_ptr type)
{
  error_unreachable_code();
}

TypeChecker_ptr type_in_expr_base_get_type_checker(const TypeInExprBase_ptr self)
{
  TYPE_IN_EXPR_BASE_CHECK_INSTANCE(self);
  TYPE_CHECKER_CHECK_INSTANCE(self->type_checker);
  return self->type_checker;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The TypeInExprBase class virtual finalizer

  Called by the class destructor
*/
static void type_in_expr_base_finalize(Object_ptr object, void* dummy)
{
  TypeInExprBase_ptr self = TYPE_IN_EXPR_BASE(object);

  type_in_expr_base_deinit(self);
  FREE(self);
}

/*!
  \brief Private copy constructor

  \sa node_walker_copy_aux
*/
static Object_ptr type_in_expr_base_copy(const Object_ptr self)
{
  /* should not create an instance of TypeInExprBase: pure virtual */
  error_unreachable_code();
}
