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
   \brief Implementation of class 'MasterCompilePredicateExtractor'



   \sa MasterCompilePredicateExtractor.h
*/

#include "nusmv/core/compile/pne/MasterCompilePredicateExtractor.h"
#include "nusmv/core/compile/pne/MasterCompilePredicateExtractor_private.h"

#include "nusmv/core/compile/pne/PredicateExtractorBase.h"
#include "nusmv/core/compile/pne/PredicateExtractorBase_private.h"
#include "nusmv/core/compile/pne/PredicateExtractorCore.h"
#include "nusmv/core/compile/pne/PredicateExtractorCore_private.h"

#include "nusmv/core/cinit/NuSMVEnv.h"
#include "nusmv/core/utils/WordNumberMgr.h"
#include "nusmv/core/wff/ExprMgr.h"
#include "nusmv/core/parser/symbols.h"
#include "nusmv/core/utils/defs.h"
#include "nusmv/core/utils/ErrorMgr.h"
#include "nusmv/core/utils/Slist.h"
#include "nusmv/core/utils/StreamMgr.h"
#include "nusmv/core/utils/Logger.h"
#include "nusmv/core/compile/compile.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'MasterCompilePredicateExtractor_private.h' for class 'MasterCompilePredicateExtractor' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static
void master_compile_predicate_extractor_finalize (Object_ptr object,
                                                  void* dummy);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

MasterCompilePredicateExtractor_ptr MasterCompilePredicateExtractor_create(SymbTable_ptr st,
                                                                           boolean use_approx)
{
  MasterCompilePredicateExtractor_ptr self = ALLOC(MasterCompilePredicateExtractor, 1);
  MASTER_COMPILE_PREDICATE_EXTRACTOR_CHECK_INSTANCE(self);

  master_compile_predicate_extractor_init(self, st, use_approx);
  return self;
}

void MasterCompilePredicateExtractor_destroy(MasterCompilePredicateExtractor_ptr self)
{
  MASTER_COMPILE_PREDICATE_EXTRACTOR_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

void
MasterCompilePredicateExtractor_support_init(MasterCompilePredicateExtractor_ptr self,
                                             SymbTable_ptr st,
                                             boolean use_approx)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const NodeMgr_ptr nodemgr = NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));
  const WordNumberMgr_ptr words = WORD_NUMBER_MGR(NuSMVEnv_get_value(env, ENV_WORD_NUMBER_MGR));

  node_ptr w0,w1;

  self->support_extractor = ALLOC(MasterCompilePredicateExtractorSupport, 1);

  self->support_extractor->all_preds = Set_MakeEmpty();
  self->support_extractor->unclustered_preds = Set_MakeEmpty();
  self->support_extractor->all_clusters = Set_MakeEmpty();
  self->support_extractor->var2cluster = new_assoc();
  self->support_extractor->cluster2preds = new_assoc();
  self->support_extractor->expr2preds = new_assoc();

  w0 = find_node(nodemgr, NUMBER_UNSIGNED_WORD,
                 NODE_PTR(WordNumberMgr_integer_to_word_number(words, 0, 1)), Nil);
  w1 = find_node(nodemgr, NUMBER_UNSIGNED_WORD,
                 NODE_PTR(WordNumberMgr_integer_to_word_number(words, 1, 1)), Nil);
  self->support_extractor->special_word_preds[0] = Set_MakeSingleton(w0);
  self->support_extractor->special_word_preds[1] = Set_MakeSingleton(w1);
  self->support_extractor->special_word_preds[2] = Set_AddMember(Set_MakeSingleton(w0), w1);

  self->support_extractor->st = st;
  self->support_extractor->checker = SymbTable_get_type_checker(st);
  self->support_extractor->use_approx = use_approx;
}

void
MasterCompilePredicateExtractor_support_deinit(MasterCompilePredicateExtractor_ptr self)
{
  assoc_iter iter;
  Set_t cluster, preds, tmp;
  node_ptr expr;

  nusmv_assert(TYPE_CHECKER(NULL) != self->support_extractor->checker);

  /* free 3 special predicate subparts */
  Set_ReleaseSet(self->support_extractor->special_word_preds[0]);
  Set_ReleaseSet(self->support_extractor->special_word_preds[1]);
  Set_ReleaseSet(self->support_extractor->special_word_preds[2]);

  /* free Set_t of predicate subparts in expr2preds.  different
     expressions may point to the same set of predicates (e.g. this
     happens with defines) => collect all sets in one set and then
     release them.  This allows to avoid double releases. */
  tmp = Set_MakeEmpty();

  ASSOC_FOREACH(self->support_extractor->expr2preds, iter, &expr, &preds) {
    /* preds must exist and should not be one of special predicates set */
    nusmv_assert(preds != NULL &&
                 preds != self->support_extractor->special_word_preds[0] &&
                 preds != self->support_extractor->special_word_preds[1] &&
                 preds != self->support_extractor->special_word_preds[2]);

    if (!IS_FLAG_PREDICATES(preds)) {
      tmp = Set_AddMember(tmp, NODE_PTR(preds));
    }
  }
  Set_ReleaseSetOfSet(tmp);
  free_assoc(self->support_extractor->expr2preds);

  /* free Set_t of predicates in cluster2preds */
  ASSOC_FOREACH(self->support_extractor->cluster2preds, iter, &cluster, &preds) {
    nusmv_assert(preds != NULL);
    Set_ReleaseSet(preds);
  }
  free_assoc(self->support_extractor->cluster2preds);

  /* no need to free cluster in var2cluster */
  free_assoc(self->support_extractor->var2cluster);

  /* free clusters which are Set_t of Set_t */
  Set_ReleaseSetOfSet(self->support_extractor->all_clusters);

  /* predicate sets are just Set_t of node_ptr */
  Set_ReleaseSet(self->support_extractor->unclustered_preds);
  Set_ReleaseSet(self->support_extractor->all_preds);


  /* debugging code : setting to NULL */
  self->support_extractor->all_preds = (Set_t)NULL;
  self->support_extractor->unclustered_preds = (Set_t)NULL;
  self->support_extractor->all_clusters = (Set_t)NULL;
  self->support_extractor->var2cluster = (hash_ptr)NULL;
  self->support_extractor->cluster2preds = (hash_ptr)NULL;
  self->support_extractor->expr2preds = (hash_ptr)NULL;
  self->support_extractor->special_word_preds[0] = (Set_t)NULL;
  self->support_extractor->special_word_preds[1] = (Set_t)NULL;
  self->support_extractor->special_word_preds[2] = (Set_t)NULL;

  self->support_extractor->st = SYMB_TABLE(NULL);
  self->support_extractor->checker = TYPE_CHECKER(NULL);

  FREE(self->support_extractor);
}

Set_t
MasterCompilePredicateExtractor_compute_preds(MasterCompilePredicateExtractor_ptr self,
                                              node_ptr expr)
{
  ListIter_ptr iter;
  iter = NodeList_get_first_iter(MASTER_NODE_WALKER(self)->walkers);
  while(!ListIter_is_end(iter)) {
    PredicateExtractorBase_ptr predicate_extractor_base =
      PREDICATE_EXTRACTOR_BASE(NodeList_get_elem_at(MASTER_NODE_WALKER(self)->walkers,
                                                    iter));

    if (NodeWalker_can_handle(NODE_WALKER(predicate_extractor_base), expr)) {
      return PredicateExtractorBase_compute_preds(predicate_extractor_base,
                                                  self->support_extractor,
                                                  expr,
                                                  Nil /* at first call context is always null */);
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

    StreamMgr_print_error(streams,  "Error: no compatible predicate extractor found for expression:\n");
    StreamMgr_nprint_error(streams, wffprint, "%N", expr);
    StreamMgr_print_error(streams,  "\n");
    StreamMgr_nprint_error(streams, sexpprint, "%N", expr);
    ErrorMgr_nusmv_exit(errmgr, 1);
  }
}

void
MasterCompilePredicateExtractor_compute_preds_from_hierarchy(MasterCompilePredicateExtractor_ptr self,
                                                             FlatHierarchy_ptr fh)
{
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const NodeMgr_ptr nodemgr =
    NODE_MGR(NuSMVEnv_get_value(env, ENV_NODE_MGR));

  int i;
  node_ptr expr;
  array_t * layers_name;
  const char* a_layer_name;

  node_ptr (*fh_access[])(FlatHierarchy_ptr)  = {
    FlatHierarchy_get_init, FlatHierarchy_get_invar,
    FlatHierarchy_get_trans, FlatHierarchy_get_input,
    FlatHierarchy_get_justice, FlatHierarchy_get_compassion,
    NULL};

  for(i = 0; fh_access[i] != NULL; i++) {
    expr = (*fh_access[i])(fh);
    MasterCompilePredicateExtractor_compute_preds(self, expr);
  }

  /* 1. FlatHierarchy_get_init/invar/trans do NOT return assignments.
     2. FlatHierarchy_lookup_assign return assignments with "running".
     3. FlatHierarchy_get_assign returns assignments without "running".
     4. We do NOT care about "running" and created corresponding CASE-expression
        because boolean vars are ignored by predicate extractor and
        assignment of a var to itself does not create new predicate
     Thus it is better to use FlatHierarchy_get_assign instead of
     FlatHierarchy_lookup_assign (there will be no need to iterate over all vars).
  */

  /* Assignments require very special handling because
     FlatHierarchy_get_assign returns the assignments without
     CASE-expressions and "running" variables created when there are
     processes.  To obtain the actual assignments it is necessary to
     collects assignments using FlatHierarchy_lookup_assign.

     NOTE: This code is terrible because API in FlatHierarchy does
     not provided the required function (to access actual assignments).
  */
  assert(self->support_extractor->st != NULL);
  layers_name = SymbTable_get_class_layer_names(self->support_extractor->st, (const char*) NULL);

  arrayForEachItem(const char*, layers_name, i, a_layer_name) {
    SymbLayer_ptr layer = SymbTable_get_layer(self->support_extractor->st, a_layer_name);
    SymbLayerIter iter;

    SYMB_LAYER_FOREACH(layer, iter, STT_VAR) {
      node_ptr name = SymbLayer_iter_get_symbol(layer, &iter);
      node_ptr init_name = find_node(nodemgr, SMALLINIT, name, Nil);
      node_ptr next_name = find_node(nodemgr, NEXT, name, Nil);
      node_ptr invar_expr = FlatHierarchy_lookup_assign(fh, name);
      node_ptr init_expr = FlatHierarchy_lookup_assign(fh, init_name);
      node_ptr next_expr = FlatHierarchy_lookup_assign(fh, next_name);

      if (invar_expr != Nil) {
        expr = find_node(nodemgr, EQDEF, name, invar_expr);
        MasterCompilePredicateExtractor_compute_preds(self, expr);
      }
      if (init_expr != Nil) {
        expr = find_node(nodemgr, EQDEF, init_name, init_expr);
        MasterCompilePredicateExtractor_compute_preds(self, expr);
      }
      if (next_expr != Nil) {
        expr = find_node(nodemgr, EQDEF, next_name, next_expr);
        MasterCompilePredicateExtractor_compute_preds(self, expr);
      }
    }
  }

  return;
}

Set_t
MasterCompilePredicateExtractor_get_all_preds(const MasterCompilePredicateExtractor_ptr self)
{
  MASTER_COMPILE_PREDICATE_EXTRACTOR_CHECK_INSTANCE(self);

  return self->support_extractor->all_preds;
}

Set_t
MasterCompilePredicateExtractor_get_all_clusters(const MasterCompilePredicateExtractor_ptr self)
{
  MASTER_COMPILE_PREDICATE_EXTRACTOR_CHECK_INSTANCE(self);

  /* there are un-clustered predicates => process them at first */
  if (!Set_IsEmpty(self->support_extractor->unclustered_preds)) {

    Set_Iterator_t pred_iter;

    /* iterate over all the predicates */
    SET_FOREACH(self->support_extractor->unclustered_preds, pred_iter) {

      node_ptr predicate = Set_GetMember(self->support_extractor->unclustered_preds, pred_iter);
      Set_t deps = Formula_GetDependencies(self->support_extractor->st, predicate, Nil);

      /* NOTE: if simplification was not done then a predicate may consist
         of constants only. In this case dependency is empty and the predicate
         can be ignored.
      */
      if (!Set_IsEmpty(deps)) {

        /* first var is dealt differently : if a var is not yet in the
           table => create a new cluster for it, otherwise use the
           existing one */
        Set_Iterator_t it = Set_GetFirstIter(deps);
        node_ptr var = Set_GetMember(deps, it);

        Set_t cluster = (Set_t)find_assoc(self->support_extractor->var2cluster, var);

        /* if there is no cluster => create a new one */
        if (NULL == cluster) {
          cluster = Set_MakeSingleton(var);
          insert_assoc(self->support_extractor->var2cluster, var, NODE_PTR(cluster));
          self->support_extractor->all_clusters = Set_AddMember(self->support_extractor->all_clusters, NODE_PTR(cluster));

          /* create new cluster->predicates association and add the predicate */
          insert_assoc(self->support_extractor->cluster2preds, NODE_PTR(cluster),
                       NODE_PTR(Set_MakeSingleton(NODE_PTR(predicate))));
        }
        /* cluster already exist => insert the predicate into existing
           cluster2preds associated */
        else {
          Set_t cl_preds = (Set_t)find_assoc(self->support_extractor->cluster2preds, NODE_PTR(cluster));
          Set_t tmp;
          nusmv_assert(NULL != cl_preds); /* every cluster has some predicate */
          tmp = Set_AddMember(cl_preds, predicate);
          nusmv_assert(tmp == cl_preds); /* debug: the pointer did not change */
        }
        /* note that every cluster always has at least one var, and one
           predicate */

        /* check other vars => insert the var in the cluster or merge
           the clusters */
        for (it = Set_GetNextIter(it); !Set_IsEndIter(it);
             it = Set_GetNextIter(it)) {
          Set_t another_cluster;
          var = Set_GetMember(deps, it);
          another_cluster = (Set_t) find_assoc(self->support_extractor->var2cluster, var);

          /* var has no cluster => add the var to the cluster of previous var */
          if ((Set_t)NULL == another_cluster) {
            another_cluster = Set_AddMember(cluster, var);
            /* debug: the pointer does not change */
            nusmv_assert(cluster == another_cluster);
            insert_assoc(self->support_extractor->var2cluster, var, (node_ptr)cluster);
          }
          /* var has cluster but it is the same as of prev. var => do nothing */
          else if (cluster == another_cluster) {
            /* do nothing */
          }
          /* the var already has its own cluster => push all the info
             into that other cluster and reset the
             hash: var -> cluster and hash: cluster->preds */
          else {
            /* merge the cluster into the other one.  Because of Set class
               implementation, pointer to another_cluster will not change, i.e.
               only cluster associations have to be changed */
            Set_t cl_preds, other_preds, tmp;
            Set_Iterator_t cl_iter;
            SET_FOREACH(cluster, cl_iter) {
              node_ptr a_var = Set_GetMember(cluster, cl_iter);
              Set_t tmp = Set_AddMember(another_cluster, a_var);
              /* debug: the pointer does not change */
              nusmv_assert(another_cluster == tmp);
              insert_assoc(self->support_extractor->var2cluster, a_var, NODE_PTR(another_cluster));
            }
            /* merge the associated predicates */
            cl_preds = (Set_t) find_assoc(self->support_extractor->cluster2preds,
                                          NODE_PTR(cluster));
            other_preds = (Set_t) find_assoc(self->support_extractor->cluster2preds,
                                             NODE_PTR(another_cluster));
            /* every cluster has at least 1 predicate */
            nusmv_assert(NULL != cl_preds && NULL != other_preds);

            tmp = Set_Union(other_preds, cl_preds);
            nusmv_assert(tmp == other_preds); /* debug: other_preds is a union now */

            Set_ReleaseSet(cl_preds);
            remove_assoc(self->support_extractor->cluster2preds, NODE_PTR(cluster));

            self->support_extractor->all_clusters = Set_RemoveMember(self->support_extractor->all_clusters,
                                                  NODE_PTR(cluster));
            Set_ReleaseSet(cluster);

            cluster = another_cluster;
          }
        } /* for */
      } /* if predicate is not empty */

      Set_ReleaseSet(deps);
    }

    Set_ReleaseSet(self->support_extractor->unclustered_preds);
    self->support_extractor->unclustered_preds = Set_MakeEmpty();
  }

  return self->support_extractor->all_clusters;
}

Set_t
MasterCompilePredicateExtractor_get_var_cluster(const MasterCompilePredicateExtractor_ptr self,
                                                node_ptr var)
{
  /* to trigger cluster computation */
  MasterCompilePredicateExtractor_get_all_clusters(self);

  return (Set_t)find_assoc(self->support_extractor->var2cluster, var);
}

Set_t MasterCompilePredicateExtractor_get_preds_of_a_cluster(const MasterCompilePredicateExtractor_ptr self,
                                                             Set_t cluster)
{
  Set_t preds;

  /* MasterCompilePredicateExtractor_compute_preds was called after
     MasterCompilePredicateExtractor_get_all_clusters which is an error */
  nusmv_assert(Set_IsEmpty(self->support_extractor->unclustered_preds));

  preds = (Set_t)find_assoc(self->support_extractor->cluster2preds, NODE_PTR(cluster));

  nusmv_assert(preds != NULL); /* every cluster has predicates */

  return preds;
}

void
MasterCompilePredicateExtractor_print(const MasterCompilePredicateExtractor_ptr self,
                                      FILE* stream,
                                      boolean printPredicates,
                                      boolean printClusters)
{
  Set_t set;
  Set_Iterator_t iter;
  int clst_num = 0;
  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(self));
  const MasterPrinter_ptr wffprint =
    MASTER_PRINTER(NuSMVEnv_get_value(env, ENV_WFF_PRINTER));
  const ErrorMgr_ptr errmgr =
    ERROR_MGR(NuSMVEnv_get_value(env, ENV_ERROR_MANAGER));

  if (!printPredicates && !printClusters) {
    ErrorMgr_rpterr(errmgr, "Function MasterCompilePredicateExtractor_print needs "
           "at least one  of printPredicates and printClusters to be true.");
  }

  /* ----------- print just predicates */
  if (printPredicates && ! printClusters) {
    fprintf(stream, "\nPredicates are :\n-------------------------------\n");
    set = MasterCompilePredicateExtractor_get_all_preds(self);
    SET_FOREACH(set, iter) {
      fprintf(stream, "\n   ");
      print_node(wffprint, stream, Set_GetMember(set, iter));
    }
    fprintf(stream, "\n------------------------------------\n");
    return;
  }

  /* -------------  print clusters */
  set = MasterCompilePredicateExtractor_get_all_clusters(self);
  SET_FOREACH(set, iter) {
    Set_t cluster = (Set_t) Set_GetMember(set, iter);
    Set_Iterator_t sit;

    /* output the clusters */
    fprintf(stream,
            "\n--------------------------------------------------\n"
            "---- Cluster %d \n \t [\n", clst_num);
    /* Clusters */
    SET_FOREACH(cluster, sit) {
      node_ptr var = Set_GetMember(cluster, sit);
      fprintf(stream, " \t   ");
      print_node(wffprint, stream, var);
      fprintf(stream, " : ");
      SymbType_print(SymbTable_get_var_type(self->support_extractor->st, var), wffprint, stream);
      fprintf(stream, "\n");
    }
    fprintf(stream, " \t ]\n");

    /* stream the predicates */
    if (printPredicates) {
      /* Preds */
      Set_t preds = (Set_t)find_assoc(self->support_extractor->cluster2preds,
                                      NODE_PTR(cluster));
      nusmv_assert(NULL != preds); /* every cluster has at least one predicate */

      fprintf(stream, " \t Predicates for Cluster %d\n \t (\n", clst_num);
      SET_FOREACH(preds, sit) {
        node_ptr pr = Set_GetMember(cluster, sit);
        fprintf(stream, " \t   ");
        print_node(wffprint, stream, pr);
        fprintf(stream, "\n");
      }
      fprintf(stream, " \t )\n\n");
    }

    return;
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void master_compile_predicate_extractor_init(MasterCompilePredicateExtractor_ptr self,
                                             SymbTable_ptr st,
                                             boolean use_approx)
{
  Siter iter;
  OptsHandler_ptr opts;
  Slist_ptr predicate_extractor_core_walkers;
  PredicateExtractorCoreWalker_ptr pe_core_walker;

  const NuSMVEnv_ptr env = EnvObject_get_environment(ENV_OBJECT(st));

  /* base class initialization */
  master_node_walker_init(MASTER_NODE_WALKER(self), env);

  /* members initialization */
  predicate_extractor_core_walkers = (Slist_ptr) NuSMVEnv_get_value(env,
                                                                    MASTER_COMPILE_PREDICATE_EXTRACTOR_CORE_WALKERS);
  opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  nusmv_assert(predicate_extractor_core_walkers != NULL);
  if (opt_verbose_level_gt(opts, 4)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger,
               "Initializing all the predicate extractor "
               "within the given environment...\n");
  }
  SLIST_FOREACH(predicate_extractor_core_walkers, iter) {
    PredicateExtractorCore_ptr predicate_extractor_core;
    pe_core_walker = (PredicateExtractorCoreWalker_ptr) Siter_element(iter);
    predicate_extractor_core =
      (PredicateExtractorCore_ptr)pe_core_walker->pecw_function_create(env,
                                                                       pe_core_walker->pecw_name);

    PREDICATE_EXTRACTOR_CORE_CHECK_INSTANCE(predicate_extractor_core);
    MasterNodeWalker_register_walker(MASTER_NODE_WALKER(self),
                                     NODE_WALKER(predicate_extractor_core));
  }

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = master_compile_predicate_extractor_finalize;

  /* self->support_extractor initialization */
  MasterCompilePredicateExtractor_support_init(self, st, use_approx);
}


void master_compile_predicate_extractor_deinit(MasterCompilePredicateExtractor_ptr self)
{
  /* base class deinitialization */
  master_node_walker_deinit(MASTER_NODE_WALKER(self));

  /* members deinitialization */

  /* self->support_extractor deinitialization */
  MasterCompilePredicateExtractor_support_deinit(self);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The MasterCompilePredicateExtractor class virtual finalizer

  Called by the class destructor
*/
static void master_compile_predicate_extractor_finalize(Object_ptr object, void* dummy)
{
  MasterCompilePredicateExtractor_ptr self = MASTER_COMPILE_PREDICATE_EXTRACTOR(object);

  master_compile_predicate_extractor_deinit(self);
  FREE(self);
}

/**AutomaticEnd***************************************************************/
