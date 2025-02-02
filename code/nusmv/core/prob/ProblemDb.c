/* ---------------------------------------------------------------------------

  This file is part of the ``prob'' package of NuSMV version 2.
  Copyright (C) 2015 by FBK-irst.

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
   \author Roberto Cavada
   \brief Implementation of class 'ProblemDb'

   \sa ProblemDb.h
*/

#include "nusmv/core/prob/ProblemDb.h"
#include "nusmv/core/prob/ProblemDb_private.h"

/* this is used to control owners counter: */
#include "nusmv/core/prob/Problem_private.h"

#include "nusmv/core/utils/assoc.h"
#include "nusmv/core/utils/ucmd.h"
#include "nusmv/core/utils/Vector.h"
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
/* See 'ProblemDb_private.h' for class 'ProblemDb' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void problem_db_finalize (Object_ptr object, void* dummy);

static void problem_db_remove_problem_at(ProblemDb_ptr self,
                                         Problem_ptr prob,
                                         int idx);

static int problem_db_set_tags(const ProblemDb_ptr self,
                               string_ptr uname,
                               const long long tags,
                               /* outputs: */
                               long long* previous_tags);

static int problem_db_find_tags(const ProblemDb_ptr self,
                                const long long tags);

static int problem_db_add_tags(ProblemDb_ptr self,
                               string_ptr uname,
                               const long long tags);

static int problem_db_find_problem_within_tags(const ProblemDb_ptr self,
                                               int idx_from,
                                               const long long tags_mask);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

ProblemDb_ptr ProblemDb_create(NuSMVEnv_ptr env)
{
  ProblemDb_ptr self = ALLOC(ProblemDb, 1);
  PROBLEM_DB_CHECK_INSTANCE(self);

  problem_db_init(self, env);
  return self;
}

void ProblemDb_destroy(ProblemDb_ptr self)
{
  PROBLEM_DB_CHECK_INSTANCE(self);
  Object_destroy(OBJECT(self), NULL);
}

size_t ProblemDb_get_size(const ProblemDb_ptr self)
{
  PROBLEM_DB_CHECK_INSTANCE(self);
  return VECTOR_SIZE(self->problems);
}

int ProblemDb_add_problem(ProblemDb_ptr self, Problem_ptr problem)
{
  PROBLEM_DB_CHECK_INSTANCE(self);
  problem_inc_owners(problem);
  Vector_push(self->problems, problem);
  return VECTOR_SIZE(self->problems) - 1;
}

int ProblemDb_remove_problem(ProblemDb_ptr self, Problem_ptr problem)
{
  int idx;
  PROBLEM_DB_CHECK_INSTANCE(self);

  idx = ProblemDb_find_problem(self, problem);
  if (0 <= idx) {
    problem_db_remove_problem_at(self, problem, idx);
  }
  return idx;
}

size_t ProblemDb_remove_problems(ProblemDb_ptr self,
                                 const long long tags_mask)
{
  size_t num;
  int idx;
  PROBLEM_DB_CHECK_INSTANCE(self);

  num = 0;
  for (idx=0; idx < VECTOR_SIZE(self->problems); ++idx) {
    Problem_ptr prob = ProblemDb_get_problem_at(self, idx);
    long long ptags = ProblemDb_get_problem_tags(self, prob);
    if (prob && (PROBLEM_DB_ALL_TAGS == tags_mask || ptags & tags_mask)) {
      problem_db_remove_problem_at(self, prob, idx);
      num += 1;
    }
  }

  return num;
}

int ProblemDb_find_problem(const ProblemDb_ptr self, const Problem_ptr problem)
{
  int idx;
  PROBLEM_DB_CHECK_INSTANCE(self);

  for (idx=0; idx < VECTOR_SIZE(self->problems); ++idx) {
    if (problem == ProblemDb_get_problem_at(self, idx))
      return idx;
  }
  return -1;  /* not found */
}

Problem_ptr ProblemDb_get_last_problem(const ProblemDb_ptr self)
{
  Problem_ptr res;

  PROBLEM_DB_CHECK_INSTANCE(self);
  if (0 == VECTOR_SIZE(self->problems)) {
    res = NULL;
  }
  else {
    res = VECTOR_LAST(self->problems);
  }

  return res;
}

Problem_ptr ProblemDb_get_problem_at(const ProblemDb_ptr self,
                                     int idx)
{
  Problem_ptr res;

  PROBLEM_DB_CHECK_INSTANCE(self);
  if (!ProblemDb_is_index_valid(self, idx)) {
    const ErrorMgr_ptr errmgr =
        ERROR_MGR(NuSMVEnv_get_value(
            EnvObject_get_environment(ENV_OBJECT(self)),
            ENV_ERROR_MANAGER));

    ErrorMgr_rpterr(errmgr, "ProblemDB %x: invalid problem index %d",
                    self, idx);
  }
  else {
    res = VECTOR_AT(self->problems, idx);
  }

  return res;
}

Vector_ptr ProblemDb_get_problems_at_indices(const ProblemDb_ptr self,
                                             const char* indices,
                                             const long long tags_mask)
{
  Vector_ptr res;
  const char* delimiters=",:";
  char* _copy_indices;
  char* token;
  ErrorMgr_ptr errmgr;

  PROBLEM_DB_CHECK_INSTANCE(self);

  errmgr = ERROR_MGR(NuSMVEnv_get_value(
      EnvObject_get_environment(ENV_OBJECT(self)), ENV_ERROR_MANAGER));

  res = Vector_create();
  _copy_indices = util_strsav(indices);

  for (token=strtok(_copy_indices, delimiters); token != (char*) NULL;
       token=strtok((char*) NULL, delimiters)) {
    char* dash = strchr(token, '-');
    int low, high;

    if (dash == (char*) NULL) {  /* no range */
      if (util_str2int(token, &low) != 0) {
        ErrorMgr_error_invalid_number(errmgr, token);
        FREE(_copy_indices);
        ErrorMgr_rpterr(errmgr, NULL);
      }
      high = low;
    }
    else { /* a range has been specified */
      *dash = '\0'; /* splits the range */
      if (util_str2int(token, &low) != 0) {
        ErrorMgr_error_invalid_number(errmgr, token);
        FREE(_copy_indices);
        Vector_destroy(res);
        ErrorMgr_rpterr(errmgr, NULL);
      }
      if (util_str2int(dash+1, &high) != 0) {
        ErrorMgr_error_invalid_number(errmgr, token);
        FREE(_copy_indices);
        Vector_destroy(res);
        ErrorMgr_rpterr(errmgr, NULL);
      }
    }

    if (low > high) {
      FREE(_copy_indices);
      Vector_destroy(res);
      ErrorMgr_rpterr(errmgr, "Range error: %d-%d", low, high);
    }

    {
      int idx;
      for (idx=low; idx <= high; ++idx) {
        Problem_ptr prob;
        long long tag;

        CATCH(errmgr) {
          prob = ProblemDb_get_problem_at(self, idx);
        }
        FAIL(errmgr) {
          FREE(_copy_indices);
          Vector_destroy(res);
          ErrorMgr_rpterr(
              errmgr,
              "Problem index %d is not valid (must be in the range [0,%d])",
              idx, ProblemDb_get_size(self)-1);
        }

        if (prob) {
          tag = ProblemDb_get_problem_tags(self, prob);
          if (PROBLEM_DB_ALL_TAGS == tags_mask || tags_mask & tag) {
            Vector_push(res, prob);
          }
        }
      } /* for all problems indices */
    }
  }

  FREE(_copy_indices);
  return res;
}

boolean ProblemDb_is_index_valid(const ProblemDb_ptr self, int idx)
{
  PROBLEM_DB_CHECK_INSTANCE(self);
  return (0 <= idx && idx < VECTOR_SIZE(self->problems));
}

long long ProblemDb_get_problem_tags(const ProblemDb_ptr self,
                                     const Problem_ptr problem)
{
  int idx;
  PROBLEM_DB_CHECK_INSTANCE(self);

  idx = (PTR_TO_INT(find_assoc(self->prob2tagidx, (node_ptr) problem)) - 1);
  if (0 > idx) {  /* never tagged this problem */
    return 0;
  }

  return VECTOR_AT(self->tags, idx);
}

void ProblemDb_set_problem_tags(ProblemDb_ptr self,
                                const Problem_ptr problem,
                                const long long tags)
{
  int idx;
  PROBLEM_DB_CHECK_INSTANCE(self);

  /* already inserted this tag? */
  idx = problem_db_find_tags(self, tags);

  if (0 > idx) {
    idx = problem_db_add_tags(self, NULL /*unnamed tag*/, tags);
  }

  /* check if unchanging tag information */
  if (idx == PTR_TO_INT(
          find_assoc(self->prob2tagidx, (node_ptr) problem)) - 1)
    return;

  /* stores the association */
  insert_assoc(self->prob2tagidx, (node_ptr) problem,
               PTR_FROM_INT(node_ptr, idx + 1));
}

void ProblemDb_add_problem_tags(ProblemDb_ptr self,
                                const Problem_ptr problem,
                                const long long tags)
{
  long long ptags;
  PROBLEM_DB_CHECK_INSTANCE(self);

  ptags = ProblemDb_get_problem_tags(self, problem) | tags;
  ProblemDb_set_problem_tags(self, problem, ptags);
}

void ProblemDb_remove_problem_tags(ProblemDb_ptr self,
                                   const Problem_ptr problem,
                                   const long long tags)
{
  long long ptags;
  PROBLEM_DB_CHECK_INSTANCE(self);

  ptags = ProblemDb_get_problem_tags(self, problem) & ~tags;
  ProblemDb_set_problem_tags(self, problem, ptags);
}

long long ProblemDb_new_tag(ProblemDb_ptr self)
{
  long long tag;

  PROBLEM_DB_CHECK_INSTANCE(self);

  tag = self->next_avail_tag;
  /* not reached the maximum value */
  nusmv_assert(0 != tag);

  self->next_avail_tag <<= 1;

  return tag;
}

long long ProblemDb_get_tags(ProblemDb_ptr self,
                             const char* name)
{
  string_ptr uname;
  int idx;

  PROBLEM_DB_CHECK_INSTANCE(self);

  uname = UStringMgr_find_string(self->usmgr, name);
  idx = (
      PTR_TO_INT(find_assoc(self->tagname2tagidx, (node_ptr) uname)) - 1);

  if (0 > idx) {
    /* this is a new tag */
    long long ptags = 0;
    long long tag = ProblemDb_new_tag(self);
    idx = problem_db_set_tags(self, uname, tag, &ptags);
    nusmv_assert(0 == ptags);  /* not previously set */
  }

  nusmv_assert(0 <= idx);

  return VECTOR_AT(self->tags, idx);
}

boolean ProblemDb_tag_name_exists(const ProblemDb_ptr self,
                                  const char* name)
{
  string_ptr uname;
  PROBLEM_DB_CHECK_INSTANCE(self);

  uname = UStringMgr_find_string(self->usmgr, name);
  return (find_assoc(self->tagname2tagidx, (node_ptr) uname) != NULL);
}

long long ProblemDb_set_tags(const ProblemDb_ptr self,
                             const char* name,
                             const long long tags)
{
  long long previous_tags = 0;
  string_ptr uname;

  PROBLEM_DB_CHECK_INSTANCE(self);

  uname = UStringMgr_find_string(self->usmgr, name);
  problem_db_set_tags(self, uname, tags, &previous_tags);

  return previous_tags;
}

const char* ProblemDb_tags_to_name(ProblemDb_ptr self,
                                   long long tags)
{
  const char* tagname = NULL;
  int idx;

  PROBLEM_DB_CHECK_INSTANCE(self);

  idx = problem_db_find_tags(self, tags);

  if (0 <= idx) {
    string_ptr uname = VECTOR_AT(self->tagnames, idx);
    nusmv_assert(NULL != uname);
    tagname = UStringMgr_get_string_text(uname);
  }

  return tagname;
}

void ProblemDb_iter_begin(const ProblemDb_ptr self,
                          const long long tags_mask,
                          struct ProblemDbIter* iter)
{
  PROBLEM_DB_CHECK_INSTANCE(self);
  ProblemDb_iter_begin_at(self, 0, tags_mask, iter);
}

void ProblemDb_iter_begin_at(const ProblemDb_ptr self,
                             const int start_idx,
                             const long long tags_mask,
                             /* output: */
                             struct ProblemDbIter* iter)
{
  int idx;
  PROBLEM_DB_CHECK_INSTANCE(self);

  idx = ((0 <= start_idx && start_idx < ProblemDb_get_size(self)) ?
         start_idx : -1);

  iter->idx = problem_db_find_problem_within_tags(self, idx, tags_mask);
  iter->tags_mask = tags_mask;
}


Problem_ptr ProblemDb_iter_get_problem(const ProblemDb_ptr self,
                                       const struct ProblemDbIter* iter)
{
  PROBLEM_DB_CHECK_INSTANCE(self);
  return ProblemDb_get_problem_at(self, iter->idx);
}

void ProblemDb_iter_next(const ProblemDb_ptr self,
                         struct ProblemDbIter* iter)
{
  PROBLEM_DB_CHECK_INSTANCE(self);
  if (iter->idx >= 0) {
    iter->idx = problem_db_find_problem_within_tags(
        self, iter->idx+1, iter->tags_mask);
  }
}

boolean ProblemDb_iter_is_end(const ProblemDb_ptr self,
                              const struct ProblemDbIter* iter)
{
  PROBLEM_DB_CHECK_INSTANCE(self);
  return iter->idx < 0;
}

size_t ProblemDb_count_problems(const ProblemDb_ptr self,
                                const long long tags_mask)
{
  size_t res;
  struct ProblemDbIter iter;
  PROBLEM_DB_CHECK_INSTANCE(self);

  res = 0;
  PROBLEM_DB_ITER_FOREACH(self, tags_mask, iter) {
    res += 1;
  }
  return res;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void problem_db_init(ProblemDb_ptr self, NuSMVEnv_ptr env)
{
  /* base class initialization */
  env_object_init(ENV_OBJECT(self), env);

  /* members initialization */
  self->problems = Vector_create();
  self->prob2tagidx = new_assoc();

  /* handling of tags assign and naming */
  self->usmgr = UStringMgr_create();
  self->tagname2tagidx = new_assoc();
  self->tagnames = Vector_create();
  self->tags = LLVector_create();
  self->next_avail_tag = 1;

  {  /* creates 0 tag */
    int idx;
    string_ptr zero = UStringMgr_find_string(self->usmgr, NO_TAG_NAME);
    idx = problem_db_add_tags(self, zero, 0);

    /* store index+1 is on purpose to distinguish from NULL */
    insert_assoc(self->tagname2tagidx, (node_ptr) zero,
                 PTR_FROM_INT(node_ptr, idx+1));
  }
  {  /* creates all-tags combination */
    int idx;
    string_ptr all = UStringMgr_find_string(self->usmgr, ALL_TAGS_NAME);
    idx = problem_db_add_tags(self, all, ~((long long) 0));

    /* store index+1 is on purpose to distinguish from NULL */
    insert_assoc(self->tagname2tagidx, (node_ptr) all,
                 PTR_FROM_INT(node_ptr, idx+1));
  }

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = problem_db_finalize;
}

void problem_db_deinit(ProblemDb_ptr self)
{
  /* members deinitialization */
  LLVector_destroy(self->tags);
  Vector_destroy(self->tagnames);
  free_assoc(self->tagname2tagidx);
  UStringMgr_destroy(self->usmgr);

  free_assoc(self->prob2tagidx);

  { /* frees owned orphan problems */
    int idx;
    for (idx=0; idx < VECTOR_SIZE(self->problems); ++idx) {
      Problem_ptr problem = ProblemDb_get_problem_at(self, idx);
      if (problem) {
        problem_dec_owners(problem);
        Problem_destroy(problem);
      }
    }

    Vector_destroy(self->problems);
  }

  /* base class deinitialization */
  env_object_deinit(ENV_OBJECT(self));
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The ProblemDb class virtual finalizer

  Called by the class destructor
*/
static void problem_db_finalize(Object_ptr object, void* dummy)
{
  ProblemDb_ptr self = PROBLEM_DB(object);

  problem_db_deinit(self);
  FREE(self);
}

static void problem_db_remove_problem_at(ProblemDb_ptr self,
                                         Problem_ptr prob,
                                         int idx)
{
  VECTOR_AT(self->problems, idx) = NULL;
  remove_assoc(self->prob2tagidx, (node_ptr) prob);

  if (prob) {
    /* detach the problem */
    problem_dec_owners(prob);
    Problem_destroy(prob);
  }
}

/* Sets the given tags associating them to the given name (name cannot
   be NULL, i.e. unnamed tags cannot be used here).
   Returns the index of the inserted tag
*/
static int problem_db_set_tags(const ProblemDb_ptr self,
                               string_ptr uname,
                               const long long tags,
                               /* outputs: */
                               long long* previous_tags)
{
  int idx;

  nusmv_assert(NULL != uname);

  idx = (
      PTR_TO_INT(find_assoc(self->tagname2tagidx, (node_ptr) uname)) - 1);

  if (0 > idx) {  /* never seen this name */
    *previous_tags = 0;
    idx = problem_db_find_tags(self, tags);

    if (-1 == idx) {  /* never seen this tag */
      idx = problem_db_add_tags(self, uname, tags);
    }

    /* store the idx+1, to distinguish from 0 wich cannot be stored
     * in idx hash, as not distringuishable from NULL */
    insert_assoc(self->tagname2tagidx, (node_ptr) uname,
                 PTR_FROM_INT(node_ptr, idx+1));
  }
  else {
    /* tag already exists, overwrite it */
    *previous_tags = VECTOR_AT(self->tags, idx);
    VECTOR_AT(self->tags, idx) = tags;
  }

  return idx;
}

/* Returns the index of given tags (within the tags vector, or -1 if
 * not found */
static int problem_db_find_tags(const ProblemDb_ptr self,
                                const long long tags)
{
  int idx;

  PROBLEM_DB_CHECK_INSTANCE(self);
  for (idx=0; idx < VECTOR_SIZE(self->tags); ++idx) {
    if (tags == VECTOR_AT(self->tags, idx))
      return idx;  /* found */
  }

  return -1;
}

/* Adds given named tag (name can be NULL for unnamed tags).
   Returns the index of the added element */
static int problem_db_add_tags(ProblemDb_ptr self,
                               string_ptr uname,
                               const long long tags)
{
  LLVector_push(self->tags, tags);
  Vector_push(self->tagnames, uname);
  nusmv_assert(VECTOR_SIZE(self->tags) == VECTOR_SIZE(self->tagnames));
  return VECTOR_SIZE(self->tags) - 1;
}

/* Returns the index of the first matching problem, whose tags are in
   the given mask.  Returns -1 if not found */
static int problem_db_find_problem_within_tags(const ProblemDb_ptr self,
                                               int idx_from,
                                               const long long tags_mask)
{
  int idx;
  for (idx=idx_from; idx < VECTOR_SIZE(self->problems); ++idx) {
    Problem_ptr prob = ProblemDb_get_problem_at(self, idx);
    if (prob) {
      long long ptags = ProblemDb_get_problem_tags(self, prob);
      if (PROBLEM_DB_NO_TAGS == ptags || ptags & tags_mask)
        return idx;
    }
  }
  return -1;
}
