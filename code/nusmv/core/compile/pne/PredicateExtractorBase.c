
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
   \author Elia Rigo
   \brief Implementation of class 'PredicateExtractorBase'

   

   \sa PredicateExtractorBase.h
*/

#include "nusmv/core/compile/pne/MasterCompilePredicateExtractor_private.h"
#include "nusmv/core/compile/pne/PredicateExtractorBase.h"
#include "nusmv/core/compile/pne/PredicateExtractorBase_private.h"

#include "nusmv/core/compile/FlatHierarchy.h" /* for FlatHierarchy_ptr */
#include "nusmv/core/utils/ErrorMgr.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'PredicateExtractorBase_private.h' for class 'PredicateExtractorBase' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void predicate_extractor_base_finalize (Object_ptr object, void* dummy);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

PredicateExtractorBase_ptr 
PredicateExtractorBase_create(const NuSMVEnv_ptr env, const char* name, 
                              int low, size_t num, boolean can_handle_null)
{
  PredicateExtractorBase_ptr self = ALLOC(PredicateExtractorBase, 1);
  PREDICATE_EXTRACTOR_BASE_CHECK_INSTANCE(self);

  predicate_extractor_base_init(self, env, name, low, num, can_handle_null);
  return self;
}

void PredicateExtractorBase_destroy(PredicateExtractorBase_ptr self)
{
  PREDICATE_EXTRACTOR_BASE_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

VIRTUAL Set_t
PredicateExtractorBase_compute_preds(PredicateExtractorBase_ptr self,
                                     MasterCompilePredicateExtractorSupport_ptr support,
                                     node_ptr expr,
                                     node_ptr context)
{
  PREDICATE_EXTRACTOR_BASE_CHECK_INSTANCE(self);
  return self->compute_preds(self, support, expr, context);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void predicate_extractor_base_init(PredicateExtractorBase_ptr self, 
                                   const NuSMVEnv_ptr env, const char* name, 
                                   int low, size_t num, boolean can_handle_null)
{
  /* base class initialization */
  node_walker_init(NODE_WALKER(self), env, name, low, num, 
                   can_handle_null);

  /* members initialization */

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = predicate_extractor_base_finalize;
  OVERRIDE(PredicateExtractorBase, compute_preds) = predicate_extractor_base_compute_preds;
}


void predicate_extractor_base_deinit(PredicateExtractorBase_ptr self)
{
  /* members deinitialization */

  /* base class deinitialization*/
  node_walker_deinit(NODE_WALKER(self));
}

Set_t
predicate_extractor_base_compute_preds(PredicateExtractorBase_ptr self,
                                       MasterCompilePredicateExtractorSupport_ptr support,
                                       node_ptr expr,
                                       node_ptr context)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  ErrorMgr_internal_error(errmgr, "PredicateExtractorBase: Pure virtual method " \
                          "compute preds not implemented\n");
}

Set_t 
predicate_extractor_base_throw_compute_preds(PredicateExtractorBase_ptr self,
                                             MasterCompilePredicateExtractorSupport_ptr support,
                                             node_ptr expr,
                                             node_ptr context)
{
  if (NodeWalker_can_handle(NODE_WALKER(self), expr)) {
    /* checks if self can handle the node without need of re-throw
       to the master */
    return PredicateExtractorBase_compute_preds(self, support, expr, context);
  }
  return MasterCompilePredicateExtractor_compute_preds(MASTER_COMPILE_PREDICATE_EXTRACTOR(NODE_WALKER(self)->master),
                                                       expr);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The PredicateExtractorBase class virtual finalizer

  Called by the class destructor
*/
static void predicate_extractor_base_finalize(Object_ptr object, void* dummy)
{
  PredicateExtractorBase_ptr self = PREDICATE_EXTRACTOR_BASE(object);

  predicate_extractor_base_deinit(self);
  FREE(self);
}
