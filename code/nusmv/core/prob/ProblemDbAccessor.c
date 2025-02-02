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
   \brief Implementation of class 'ProblemDbAccessor'

   \sa ProblemDbAccessor.h
*/

#include "nusmv/core/prob/ProblemDbAccessor.h"
#include "nusmv/core/prob/ProblemDbAccessor_private.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'ProblemDbAccessor_private.h' for class 'ProblemDbAccessor' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void problem_db_accessor_finalize (Object_ptr object, void* dummy);


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

ProblemDbAccessor_ptr ProblemDbAccessor_create(ProblemDb_ptr db,
                                               long long tags_mask)
{
  ProblemDbAccessor_ptr self = ALLOC(ProblemDbAccessor, 1);
  PROBLEM_DB_ACCESSOR_CHECK_INSTANCE(self);

  problem_db_accessor_init(self, db, tags_mask);
  return self;
}

void ProblemDbAccessor_destroy(ProblemDbAccessor_ptr self)
{
  PROBLEM_DB_ACCESSOR_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

ProblemDb_ptr ProblemDbAccessor_get_db(const ProblemDbAccessor_ptr self)
{
  PROBLEM_DB_ACCESSOR_CHECK_INSTANCE(self);
  return self->db;
}

long long ProblemDbAccessor_get_tags_mask(const ProblemDbAccessor_ptr self)
{
  PROBLEM_DB_ACCESSOR_CHECK_INSTANCE(self);
  return self->tags;
}

int ProblemDbAccessor_add(ProblemDbAccessor_ptr self, Problem_ptr prob)
{
  PROBLEM_DB_ACCESSOR_CHECK_INSTANCE(self);
  return ProblemDb_add_problem(self->db, prob);
}

size_t ProblemDbAccessor_clean(ProblemDbAccessor_ptr self)
{
  PROBLEM_DB_ACCESSOR_CHECK_INSTANCE(self);
  return ProblemDb_remove_problems(self->db, self->tags);
}

boolean ProblemDbAccessor_is_index_valid(const ProblemDbAccessor_ptr self,
                                         int idx)
{
  Problem_ptr prob;
  PROBLEM_DB_ACCESSOR_CHECK_INSTANCE(self);

  prob = ProblemDbAccessor_get_problem_at_index(self, idx);
  return (NULL != prob);
}

boolean ProblemDbAccessor_is_iter_valid(const ProblemDbAccessor_ptr self,
                                        const struct ProblemDbIter* iter)
{
  PROBLEM_DB_ACCESSOR_CHECK_INSTANCE(self);
  return ProblemDbAccessor_is_index_valid(self, iter->idx);
}

void ProblemDbAccessor_get_indices_range(const ProblemDbAccessor_ptr self,
                                         int* low, int* high)
{
  ProblemDbAccessor_get_first_problem(self, low);
  ProblemDbAccessor_get_last_problem(self, high);
}

size_t ProblemDbAccessor_count_problems(const ProblemDbAccessor_ptr self)
{
  PROBLEM_DB_ACCESSOR_CHECK_INSTANCE(self);
  return ProblemDb_count_problems(self->db, self->tags);
}

IntVector_ptr ProblemDbAccessor_get_problem_indices(
    const ProblemDbAccessor_ptr self)
{
  IntVector_ptr res = IntVector_create();
  struct ProblemDbIter iter;

  PROBLEM_DB_ACCESSOR_CHECK_INSTANCE(self);
  PROBLEM_DB_ACCESSOR_FOREACH(self, iter) {
    IntVector_push(res, iter.idx);
  }

  return res;
}

Problem_ptr ProblemDbAccessor_get_problem_at_index(
    const ProblemDbAccessor_ptr self, int idx)
{
  Problem_ptr res;
  PROBLEM_DB_ACCESSOR_CHECK_INSTANCE(self);

  res = NULL;
  if (ProblemDb_is_index_valid(self->db, idx)) {
    Problem_ptr prob;
    /* checks if problem contains tags */
    prob = ProblemDb_get_problem_at(self->db, idx);

    if (prob) {
      long long ptags = ProblemDb_get_problem_tags(self->db, prob);
      if (PROBLEM_DB_NO_TAGS == ptags || ptags & self->tags)
        res = prob;
    }
  }

  return res;
}

Problem_ptr ProblemDbAccessor_get_problem_at_iter(
    const ProblemDbAccessor_ptr self, const struct ProblemDbIter* iter)
{
  PROBLEM_DB_ACCESSOR_CHECK_INSTANCE(self);
  return ProblemDbAccessor_get_problem_at_index(self, iter->idx);
}

Problem_ptr
ProblemDbAccessor_get_first_problem(const ProblemDbAccessor_ptr self, int* index)
{
  int idx;
  PROBLEM_DB_ACCESSOR_CHECK_INSTANCE(self);

  for (idx=0; idx < ProblemDb_get_size(self->db); idx++) {
    Problem_ptr prob = ProblemDbAccessor_get_problem_at_index(self, idx);
    if (prob) {
      if (index)
        *index = idx;

      return prob;
    }
  }

  /* not found */
  if (index)
    *index = -1;

  return NULL;
}

Problem_ptr
ProblemDbAccessor_get_last_problem(const ProblemDbAccessor_ptr self, int* index)
{
  int idx;

  PROBLEM_DB_ACCESSOR_CHECK_INSTANCE(self);

  for (idx=ProblemDb_get_size(self->db)-1; idx >= 0; --idx) {
    Problem_ptr prob = ProblemDbAccessor_get_problem_at_index(self, idx);
    if (prob) {
      if (index)
        *index = idx;
      return prob;
    }
  }

  /* not found */
  if (index)
    *index = -1;
  return NULL;
}

void ProblemDbAccessor_print(const ProblemDbAccessor_ptr self,
                             OStream_ptr stream,
                             long long tags_mask)
{
  PROBLEM_DB_ACCESSOR_CHECK_INSTANCE(self);
  self->print(self, stream, tags_mask);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void problem_db_accessor_init(ProblemDbAccessor_ptr self,
                              ProblemDb_ptr db,
                              long long tags_mask)
{
  /* base class initialization */

  /* self shares the same env with the watched db */
  env_object_init(ENV_OBJECT(self),
                  EnvObject_get_environment(ENV_OBJECT(db)));

  /* members initialization */
  self->db = db;
  self->tags = tags_mask;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = problem_db_accessor_finalize;
  OVERRIDE(ProblemDbAccessor, print) = problem_db_accessor_print;
}


void problem_db_accessor_deinit(ProblemDbAccessor_ptr self)
{
  /* members deinitialization */

  /* base class deinitialization */
  env_object_deinit(ENV_OBJECT(self));
}

void problem_db_accessor_print(const ProblemDbAccessor_ptr self,
                               OStream_ptr stream,
                               long long tags_mask)
{
  ProblemDb_ptr db;
  long long tags;
  struct ProblemDbIter iter;

  db = ProblemDbAccessor_get_db(self);
  tags = ProblemDbAccessor_get_tags_mask(self) & tags_mask;

  for (ProblemDb_iter_begin(db, tags, &iter);
       !ProblemDb_iter_is_end(db, &iter);
       ProblemDb_iter_next(db, &iter)) {
    Problem_ptr prob = ProblemDbAccessor_get_problem_at_iter(self, &iter);

    OStream_printf(stream, "%03d: ", iter.idx);
    Problem_print(prob, stream);
    OStream_printf(stream, "\n");
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief The ProblemDbAccessor class virtual finalizer

  Called by the class destructor
*/
static void problem_db_accessor_finalize(Object_ptr object, void* dummy)
{
  ProblemDbAccessor_ptr self = PROBLEM_DB_ACCESSOR(object);

  problem_db_accessor_deinit(self);
  FREE(self);
}
