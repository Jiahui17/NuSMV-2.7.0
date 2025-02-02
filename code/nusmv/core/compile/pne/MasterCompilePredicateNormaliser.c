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
   \brief Implementation of class 'MasterCompilePredicateNormaliser'



   \sa MasterCompilePredicateNormaliser.h
*/

#include "nusmv/core/compile/pne/MasterCompilePredicateNormaliser.h"
#include "nusmv/core/compile/pne/MasterCompilePredicateNormaliser_private.h"

#include "nusmv/core/compile/pne/PredicateNormaliserBase.h"
#include "nusmv/core/compile/pne/PredicateNormaliserBase_private.h"
#include "nusmv/core/compile/pne/PredicateNormaliserCore.h"
#include "nusmv/core/compile/pne/PredicateNormaliserCore_private.h"

#include "nusmv/core/compile/compileInt.h"
#include "nusmv/core/compile/FlatHierarchy.h" /* for FlatHierarchy_ptr */
#include "nusmv/core/cinit/NuSMVEnv.h"
#include "nusmv/core/utils/error.h"
#include "nusmv/core/utils/ErrorMgr.h"
#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/utils/Logger.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'MasterCompilePredicateNormaliser_private.h' for class 'MasterCompilePredicateNormaliser' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
/*
  \brief The name of the hash used for the predicate normalizer support within
  the Symbol Table
 */
#define PREDICATE_NORMALIZER_HASH "___predicate_normalizer_core_hash___"

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void master_compile_predicate_normaliser_finalize (Object_ptr object,
                                                          void* dummy);

static hash_ptr
master_compile_predicate_normaliser_get_handled_hash(MasterCompilePredicateNormaliser_ptr self);

static assoc_retval
master_compile_predicate_normaliser_hash_free(char *key, char *data, char * arg);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

MasterCompilePredicateNormaliser_ptr
MasterCompilePredicateNormaliser_create(SymbTable_ptr st)
{
  MasterCompilePredicateNormaliser_ptr self = ALLOC(MasterCompilePredicateNormaliser,
                                                    1);
  MASTER_COMPILE_PREDICATE_NORMALISER_CHECK_INSTANCE(self);

  master_compile_predicate_normaliser_init(self, st);
  return self;
}

void MasterCompilePredicateNormaliser_destroy(MasterCompilePredicateNormaliser_ptr self)
{
  MASTER_COMPILE_PREDICATE_NORMALISER_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

void
MasterCompilePredicateNormaliser_support_init(MasterCompilePredicateNormaliser_ptr self,
                                              SymbTable_ptr st)
{
  self->support_normaliser = ALLOC(MasterCompilePredicateNormaliserSupport, 1);
  /* This is needed since the master_compile_predicate_normaliser_get_handled_hash
    requires the ST in the structure */
  self->support_normaliser->st = st;

  self->support_normaliser->expr2normalisedPredicate = master_compile_predicate_normaliser_get_handled_hash(self);

  self->support_normaliser->checker = SymbTable_get_type_checker(st);
}

void
MasterCompilePredicateNormaliser_support_deinit(MasterCompilePredicateNormaliser_ptr self)
{
  nusmv_assert(TYPE_CHECKER(NULL) != self->support_normaliser->checker);
  /* check this is initialised object */
  nusmv_assert((hash_ptr)NULL != self->support_normaliser->expr2normalisedPredicate);

  self->support_normaliser->expr2normalisedPredicate = (hash_ptr) NULL;
  self->support_normaliser->st = SYMB_TABLE(NULL);
  self->support_normaliser->checker = TYPE_CHECKER(NULL);

  FREE(self->support_normaliser);
}

node_ptr
MasterCompilePredicateNormaliser_normalise_expr(MasterCompilePredicateNormaliser_ptr self,
                                                node_ptr expr,
                                                boolean expand)
{
  ListIter_ptr iter;
  iter = NodeList_get_first_iter(MASTER_NODE_WALKER(self)->walkers);
  while(! ListIter_is_end(iter)) {
    PredicateNormaliserBase_ptr predicate_normaliser_base =
      PREDICATE_NORMALISER_BASE(NodeList_get_elem_at(MASTER_NODE_WALKER(self)->walkers,
                                                     iter));

    if (NodeWalker_can_handle(NODE_WALKER(predicate_normaliser_base), expr)) {
      return PredicateNormaliserBase_normalise_expr(predicate_normaliser_base,
                                                    self->support_normaliser,
                                                    expr,
                                                    Nil, /* at first call context is always null */
                                                    expand);
    }
    iter = ListIter_get_next(iter);
  }

  {
    /* Fall back */
    NuSMVEnv_ptr const env = EnvObject_get_environment(ENV_OBJECT(self));
    MasterPrinter_ptr const wffprint =
      MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));
    const MasterPrinter_ptr sexpprint =
      MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_SEXP_PRINTER));
    ErrorMgr_ptr const errmgr =
      ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
    StreamMgr_ptr const streams =
      STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));

    StreamMgr_print_error(streams,  "Error: no compatible predicate normaliser found for expression:\n");
    StreamMgr_nprint_error(streams, wffprint, "%N", expr);
    StreamMgr_print_error(streams,  "\n");
    StreamMgr_nprint_error(streams, sexpprint, "%N", expr);
    ErrorMgr_nusmv_exit(errmgr, 1);
  }
}

node_ptr
MasterCompilePredicateNormaliser_normalise_specification(MasterCompilePredicateNormaliser_ptr self,
                                                         node_ptr expr)
{
  ListIter_ptr iter;
  iter = NodeList_get_first_iter(MASTER_NODE_WALKER(self)->walkers);
  while(! ListIter_is_end(iter)) {
    PredicateNormaliserBase_ptr predicate_normaliser_base =
      PREDICATE_NORMALISER_BASE(NodeList_get_elem_at(MASTER_NODE_WALKER(self)->walkers,
                                                     iter));

    if (NodeWalker_can_handle(NODE_WALKER(predicate_normaliser_base), expr)) {
      return PredicateNormaliserBase_normalise_specification(predicate_normaliser_base,
                                                             self->support_normaliser,
                                                             expr);
    }
    iter = ListIter_get_next(iter);
  }

  {
    /* Fall back */
    NuSMVEnv_ptr const env = EnvObject_get_environment(ENV_OBJECT(self));
    MasterPrinter_ptr const wffprint =
      MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));
    const MasterPrinter_ptr sexpprint =
      MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_SEXP_PRINTER));
    ErrorMgr_ptr const errmgr =
      ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
    StreamMgr_ptr const streams =
      STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));

    StreamMgr_print_error(streams,  "Error: no compatible predicate normaliser found for expression:\n");
    StreamMgr_nprint_error(streams, wffprint, "%N", expr);
    StreamMgr_print_error(streams,  "\n");
    StreamMgr_nprint_error(streams, sexpprint, "%N", expr);
    ErrorMgr_nusmv_exit(errmgr, 1);
  }

  return Nil;
}

void
MasterCompilePredicateNormaliser_get_predicates_only(const MasterCompilePredicateNormaliser_ptr self,
                                                     Set_t* preds, node_ptr expr)
{
  ListIter_ptr iter;
  iter = NodeList_get_first_iter(MASTER_NODE_WALKER(self)->walkers);
  while(! ListIter_is_end(iter)) {
    PredicateNormaliserBase_ptr predicate_normaliser_base =
      PREDICATE_NORMALISER_BASE(NodeList_get_elem_at(MASTER_NODE_WALKER(self)->walkers,
                                                     iter));

    if (NodeWalker_can_handle(NODE_WALKER(predicate_normaliser_base), expr)) {
      { /* debugging code : only boolean (or statement) expressions can be met here */
        SymbType_ptr type = TypeChecker_get_expression_type(self->support_normaliser->checker, expr, Nil);
        nusmv_assert(SymbType_is_boolean(type) ||
                    SYMB_TYPE_STATEMENT == SymbType_get_tag(type));
      }

      PredicateNormaliserBase_get_predicates_only(predicate_normaliser_base,
                                                  self->support_normaliser,
                                                  preds, expr);
      return;
    }
    iter = ListIter_get_next(iter);
  }

  {
    /* Fall back */
    NuSMVEnv_ptr const env = EnvObject_get_environment(ENV_OBJECT(self));
    MasterPrinter_ptr const wffprint =
      MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));
    const MasterPrinter_ptr sexpprint =
      MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_SEXP_PRINTER));
    ErrorMgr_ptr const errmgr =
      ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));
    StreamMgr_ptr const streams =
      STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));

    StreamMgr_print_error(streams,  "Error: no compatible predicate normaliser found for expression:\n");
    StreamMgr_nprint_error(streams, wffprint, "%N", expr);
    StreamMgr_print_error(streams,  "\n");
    StreamMgr_nprint_error(streams, sexpprint, "%N", expr);
    ErrorMgr_nusmv_exit(errmgr, 1);
  }
}

void
MasterCompilePredicateNormaliser_print_predicates_only(const MasterCompilePredicateNormaliser_ptr self,
                                                       FILE* stream, node_ptr expr)
{
  Set_t preds;
  Set_Iterator_t iter;

  MASTER_COMPILE_PREDICATE_NORMALISER_CHECK_INSTANCE(self);
  nusmv_assert(self->support_normaliser != NULL);

  preds = Set_MakeEmpty();
  MasterCompilePredicateNormaliser_get_predicates_only(self, &preds, expr);
  {
    const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
    const MasterPrinter_ptr wffprint =
      MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));

    SET_FOREACH(preds, iter) {
      node_ptr pred = Set_GetMember(preds, iter);
      print_node(wffprint, stream, pred);
      fprintf(stream, "\n\n");
    }
  }

  Set_ReleaseSet(preds);
  return;
}
/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void master_compile_predicate_normaliser_init(MasterCompilePredicateNormaliser_ptr self,
                                              SymbTable_ptr st)
{
  Siter iter;
  OptsHandler_ptr opts;
  Slist_ptr predicate_normaliser_core_walkers;
  PredicateNormaliserCoreWalker_ptr pn_core_walker;

  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(st));

  /* base class initialization */
  master_node_walker_init(MASTER_NODE_WALKER(self), env);

  /* members initialization */
  predicate_normaliser_core_walkers = (Slist_ptr) NuSMVEnv_get_value(env,
                                                                     MASTER_COMPILE_PREDICATE_NORMALISER_CORE_WALKERS);
  opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  nusmv_assert(predicate_normaliser_core_walkers != NULL);

  if (opt_verbose_level_gt(opts, 4)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger,
               "Initializing all the predicate normaliser "
               "within the given environment...\n");
  }
  SLIST_FOREACH(predicate_normaliser_core_walkers, iter) {
    PredicateNormaliserCore_ptr predicate_normaliser_core;
    pn_core_walker = (PredicateNormaliserCoreWalker_ptr) Siter_element(iter);
    predicate_normaliser_core =
      (PredicateNormaliserCore_ptr)pn_core_walker->pncw_function_create(env,
                                                                        pn_core_walker->pncw_name);
    PREDICATE_NORMALISER_CORE_CHECK_INSTANCE(predicate_normaliser_core);
    MasterNodeWalker_register_walker(MASTER_NODE_WALKER(self),
                                     NODE_WALKER(predicate_normaliser_core));
  }

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = master_compile_predicate_normaliser_finalize;

  /* self->support_normaliser initialization */
  MasterCompilePredicateNormaliser_support_init(self, st);
}


void master_compile_predicate_normaliser_deinit(MasterCompilePredicateNormaliser_ptr self)
{
  /* base class deinitialization */
  master_node_walker_deinit(MASTER_NODE_WALKER(self));

  /* members deinitialization */

  /* support_normaliser deinitialization */
  MasterCompilePredicateNormaliser_support_deinit(self);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The MasterCompilePredicateNormaliser class virtual finalizer

  Called by the class destructor
*/
static void master_compile_predicate_normaliser_finalize(Object_ptr object, void* dummy)
{
  MasterCompilePredicateNormaliser_ptr self = MASTER_COMPILE_PREDICATE_NORMALISER(object);

  master_compile_predicate_normaliser_deinit(self);
  FREE(self);
}

static hash_ptr
master_compile_predicate_normaliser_get_handled_hash(MasterCompilePredicateNormaliser_ptr self) {
  hash_ptr result = (hash_ptr)NULL;

  MASTER_COMPILE_PREDICATE_NORMALISER_CHECK_INSTANCE(self);

  result =
    SymbTable_get_handled_hash_ptr(self->support_normaliser->st,
                                   PREDICATE_NORMALIZER_HASH,
                                   (ST_PFICPCP)NULL,
                                   (ST_PFICPI)NULL,
                                   master_compile_predicate_normaliser_hash_free,
                                   (SymbTableTriggerFun)NULL,
                                   SymbTable_clear_handled_remove_action_hash,
                                   (SymbTableTriggerFun)NULL);
  return result;
}

static assoc_retval master_compile_predicate_normaliser_hash_free(char *key,
                                                                  char *data,
                                                                  char * arg)
{
  return ASSOC_DELETE;
}

/**AutomaticEnd***************************************************************/
