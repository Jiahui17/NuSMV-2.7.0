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
   \brief Implementation of class 'Problem'

   \sa Problem.h
*/

#include "nusmv/core/prob/Problem.h"
#include "nusmv/core/prob/Problem_private.h"
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
/* See 'Problem_private.h' for class 'Problem' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void problem_finalize(Object_ptr object, void* dummy);
static boolean problem_pre_finalizer(Object_ptr object, void* dummy);
static void problem_print(const Problem_ptr self, OStream_ptr stream);
static Object_ptr problem_copy(const Object_ptr self);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

Problem_ptr Problem_create(const NuSMVEnv_ptr env)
{
  Problem_ptr self = ALLOC(Problem, 1);
  PROBLEM_CHECK_INSTANCE(self);

  problem_init(self, env);
  return self;
}


void Problem_destroy(Problem_ptr self)
{
  PROBLEM_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

void Problem_print(const Problem_ptr self, OStream_ptr stream)
{
  PROBLEM_CHECK_INSTANCE(self);
  self->print(self, stream);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

void problem_init(Problem_ptr self, const NuSMVEnv_ptr env)
{
  /* base class initialization */
  env_object_init(ENV_OBJECT(self), env);

  /* members initialization */
  self->owners = 0;

  /* virtual methods settings */
  OVERRIDE(Object, pre_finalizer) = problem_pre_finalizer;
  OVERRIDE(Object, finalize) = problem_finalize;
  OVERRIDE(Object, copy) = problem_copy;
  OVERRIDE(Problem, print) = problem_print;

  /* for example, to override a base class' virtual method: */
  /*OVERRIDE(EnvObject, virtual_method) = problem_virtual_method;*/
}


void problem_deinit(Problem_ptr self)
{
  /* members deinitialization */

  /* base class deinitialization */
  env_object_deinit(ENV_OBJECT(self));
}


void problem_inc_owners(Problem_ptr self)
{
  ++self->owners;
}

void problem_dec_owners(Problem_ptr self)
{
  nusmv_assert(self->owners > 0);
  --self->owners;
}

void problem_copy_aux(const Problem_ptr self,
                      Problem_ptr other)
{
  PROBLEM_CHECK_INSTANCE(other);

  env_object_copy_aux(ENV_OBJECT(self), ENV_OBJECT(other));

  /* The copy does not have owners */
  other->owners = 0;

  other->print = self->print;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/*!
  \brief Control if finalizer has to be really called

  The finalizer (deallocator) is called only if the object is no
  longer referenced by any owner, i.e. all ProblemDb have released
  their ownership on the object */
static boolean problem_pre_finalizer(Object_ptr object, void* dummy)
{
  Problem_ptr self = PROBLEM(object);
  nusmv_assert(self->owners >= 0);
  return 0 == self->owners;
}

/*!
  \brief The Problem class virtual finalizer

  Called by the class destructor
*/
static void problem_finalize(Object_ptr object, void* dummy)
{
  Problem_ptr self = PROBLEM(object);
  problem_deinit(self);
  FREE(self);
}

/* this method is pure */
static void problem_print(const Problem_ptr self, OStream_ptr stream)
{
  const ErrorMgr_ptr errmgr =
      ERROR_MGR(NuSMVEnv_get_value(
          EnvObject_get_environment(ENV_OBJECT(self)),
          ENV_ERROR_MANAGER));

  ErrorMgr_internal_error(
      errmgr,
      "Virtual Method Problem_print had to be implemented in derived class");
}

static Object_ptr problem_copy(const Object_ptr self)
{
  Problem_ptr copy;

  copy = ALLOC(Problem, 1);
  PROBLEM_CHECK_INSTANCE(copy);

  problem_copy_aux(PROBLEM(self), copy);

  return OBJECT(copy);
}
