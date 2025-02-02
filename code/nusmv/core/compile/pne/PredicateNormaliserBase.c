
/* ---------------------------------------------------------------------------

  This file is part of the ``compile.pne'' package of NuSMV version 2.
  Copyright (C) 2014 by FBK-irst.

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
  \author Andrei Tchaltsev
  (Implementation of modularity by Elia Rigo)
  \brief A Predicate-Normaliser class

  See PredicateNormaliser.h for more info
*/

#include "nusmv/core/compile/pne/MasterCompilePredicateNormaliser_private.h"
#include "nusmv/core/compile/pne/PredicateNormaliserBase.h"
#include "nusmv/core/compile/pne/PredicateNormaliserBase_private.h"

#include "nusmv/core/node/MasterNodeWalker.h"
#include "nusmv/core/utils/ErrorMgr.h"
#include "nusmv/core/utils/object.h"
#include "nusmv/core/set/set.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'PredicateNormaliserBase_private.h' for class 'PredicateNormaliserBase' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void predicate_normaliser_base_finalize (Object_ptr object, void* dummy);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

PredicateNormaliserBase_ptr 
PredicateNormaliserBase_create(const NuSMVEnv_ptr env, const char* name, 
                               int low, size_t num, boolean can_handle_null)
{
  PredicateNormaliserBase_ptr self = ALLOC(PredicateNormaliserBase, 1);
  PREDICATE_NORMALISER_BASE_CHECK_INSTANCE(self);

  predicate_normaliser_base_init(self, env, name, low, num, can_handle_null);
  return self;
}

void PredicateNormaliserBase_destroy(PredicateNormaliserBase_ptr self)
{
  PREDICATE_NORMALISER_BASE_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

VIRTUAL node_ptr 
PredicateNormaliserBase_normalise_expr(PredicateNormaliserBase_ptr self,
                                       MasterCompilePredicateNormaliserSupport_ptr support,
                                       node_ptr expr,
                                       node_ptr context,
                                       boolean expand)
{
  PREDICATE_NORMALISER_BASE_CHECK_INSTANCE(self);
  return self->normalise_expr(self, support, expr, context, expand);
}

VIRTUAL 
node_ptr PredicateNormaliserBase_normalise_specification(PredicateNormaliserBase_ptr self,
                                                         MasterCompilePredicateNormaliserSupport_ptr support,
                                                         node_ptr expr)
{
  PREDICATE_NORMALISER_BASE_CHECK_INSTANCE(self);
  return self->normalise_specification(self, support, expr);
}

VIRTUAL
void PredicateNormaliserBase_get_predicates_only(const PredicateNormaliserBase_ptr self,
                                                 MasterCompilePredicateNormaliserSupport_ptr support,
                                                 Set_t* preds, node_ptr expr)
{
  PREDICATE_NORMALISER_BASE_CHECK_INSTANCE(self);
  self->get_predicates_only(self, support, preds, expr);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void predicate_normaliser_base_init(PredicateNormaliserBase_ptr self, 
                                    const NuSMVEnv_ptr env, const char* name, 
                                    int low, size_t num, boolean can_handle_null)
{
  /* base class initialization */
  node_walker_init(NODE_WALKER(self), env, name, low, num, can_handle_null);

  /* members initialization */

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = predicate_normaliser_base_finalize;
  OVERRIDE(PredicateNormaliserBase, normalise_expr) = predicate_normaliser_base_normalise_expr;
  OVERRIDE(PredicateNormaliserBase, normalise_specification) = predicate_normaliser_base_normalise_specification;
  OVERRIDE(PredicateNormaliserBase, get_predicates_only) = predicate_normaliser_base_get_predicates_only;
}


void predicate_normaliser_base_deinit(PredicateNormaliserBase_ptr self)
{
  env_object_deinit(ENV_OBJECT(self));
  /* members deinitialization */

  /* base class deinitialization */
  node_walker_deinit(NODE_WALKER(self));
}

node_ptr 
predicate_normaliser_base_normalise_expr(PredicateNormaliserBase_ptr self,
                                         MasterCompilePredicateNormaliserSupport_ptr support,
                                         node_ptr expr,
                                         node_ptr context,
                                         boolean expand)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  ErrorMgr_internal_error(errmgr, "PredicateNormaliserBase: Pure virtual method " \
                          "normalise expr not implemented\n");
  return 0;
}

node_ptr 
predicate_normaliser_base_normalise_specification(PredicateNormaliserBase_ptr self,
                                                  MasterCompilePredicateNormaliserSupport_ptr support,
                                                  node_ptr expr)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  ErrorMgr_internal_error(errmgr, "PredicateNormaliserBase: Pure virtual method " \
                          "normalise specification not implemented\n");
  return 0;
}

void
predicate_normaliser_base_get_predicates_only(const PredicateNormaliserBase_ptr self,
                                              MasterCompilePredicateNormaliserSupport_ptr support,
                                              Set_t* preds, node_ptr expr)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  ErrorMgr_internal_error(errmgr, "PredicateNormaliserBase: Pure virtual method " \
                          "get predicates only not implemented\n");
}

node_ptr 
predicate_normaliser_base_throw_normalise_expr(PredicateNormaliserBase_ptr self,
                                               MasterCompilePredicateNormaliserSupport_ptr support,
                                               node_ptr expr,
                                               node_ptr context,
                                               boolean expand)
{
  if (NodeWalker_can_handle(NODE_WALKER(self), expr)) {
    /* checks if self can handle the node without need of re-throw
       to the master */
    return PredicateNormaliserBase_normalise_expr(self, support, expr, context, expand);
  }
  return MasterCompilePredicateNormaliser_normalise_expr(MASTER_COMPILE_PREDICATE_NORMALISER(NODE_WALKER(self)->master),
                                                         expr, expand);
}

void 
predicate_normaliser_base_throw_get_predicates_only(PredicateNormaliserBase_ptr self,
                                                    MasterCompilePredicateNormaliserSupport_ptr support,
                                                    Set_t* preds, node_ptr expr)
{
  if (NodeWalker_can_handle(NODE_WALKER(self), expr)) {
    /* checks if self can handle the node without need of re-throw
       to the master */
    PredicateNormaliserBase_get_predicates_only(self, support, preds, expr);
    return;
  }
  MasterCompilePredicateNormaliser_get_predicates_only(MASTER_COMPILE_PREDICATE_NORMALISER(NODE_WALKER(self)->master),
                                                       preds, expr);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The PredicateNormaliserBase class virtual finalizer

  Called by the class destructor
*/
static void predicate_normaliser_base_finalize(Object_ptr object, void* dummy)
{
  PredicateNormaliserBase_ptr self = PREDICATE_NORMALISER_BASE(object);

  predicate_normaliser_base_deinit(self);
  FREE(self);
}
