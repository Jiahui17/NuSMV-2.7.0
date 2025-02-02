/* ---------------------------------------------------------------------------

  This file is part of the ``compile.predicate'' package of NuSMV version 2.
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
   \brief Package function of the compile.predicate package

   \todo: Missing description

   \sa pnePkg.h
*/

#include "nusmv/core/compile/pne/pnePkg.h"
#include "nusmv/core/compile/pne/PredicateNormaliserCore.h"
#include "nusmv/core/compile/pne/PredicateNormaliserCore_private.h"

#include "nusmv/core/compile/pne/PredicateExtractorCore.h"
#include "nusmv/core/compile/pne/PredicateExtractorCore_private.h"

#include "nusmv/core/utils/Logger.h"
#include "nusmv/core/node/NodeMgr.h"
#include "nusmv/core/cinit/NuSMVEnv.h"
#include "nusmv/core/utils/StreamMgr.h"
#include "nusmv/core/opt/opt.h"
#include "nusmv/core/utils/Slist.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/*!
  \brief Initialization of the predicate package.

  Initialization of the predicate package.

  It associates a predicate to the global environment.
*/
void PnePkg_init(NuSMVEnv_ptr env)
{
  PredicateExtractorPkg_init(env);
  PredicateNormaliserPkg_init(env);
}


/*!
  \brief Deinitialization of the predicate package


*/
void PnePkg_quit(NuSMVEnv_ptr env)
{
  PredicateExtractorPkg_quit(env);
  PredicateNormaliserPkg_quit(env);
}

/*---------------------------------------------------------------------------*/
/* Predicate Normaliser                                                      */
/*---------------------------------------------------------------------------*/

/*!
  \brief Initialization of the predicate normaliser in the predicate package.

  Initialization of the predicate normaliser in the predicate package.

  It associates a predicate normaliser to the global environment.
*/
void PredicateNormaliserPkg_init(NuSMVEnv_ptr env)
{
  //Commented operation are done by PredicateNormaliser.c
  //MasterCompilePredicateNormaliser_ptr predicate_normaliser;
  OptsHandler_ptr opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  if (opt_verbose_level_gt(opts, 4)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger,
               "Instantiating the Predicate Normaliser instance "
               "within the given environment...\n");
  }

  PnePkg_init_predicate_normaliser_walkers_list(env);
  {
    PredicateNormaliserCoreWalker_ptr pn_core_walker;
    pn_core_walker = ALLOC(PredicateNormaliserCoreWalker, 1);
    pn_core_walker->pncw_name = PREDICATE_NORMALISER_CORE_NAME;
    pn_core_walker->pncw_function_create = PredicateNormaliserCore_create;
    PnePkg_register_predicate_normaliser_walker(env,
                                                pn_core_walker);
  }
}

/*!
  \brief Deinitialization of the predicate normaliser package


*/
void PredicateNormaliserPkg_quit(NuSMVEnv_ptr env)
{
  OptsHandler_ptr opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  if (opt_verbose_level_gt(opts, 4)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger,
               "Clearing the Predicate Normaliser instance in "
               "the given environment...\n");
  }
  /* Destroy predicate normaliser walkers list */
  PnePkg_destroy_predicate_normaliser_walkers_list(env);
}

void PnePkg_register_predicate_normaliser_walker(NuSMVEnv_ptr env,
                                                 PredicateNormaliserCoreWalker_ptr predicate_normaliser_core_walker)
{
  const Slist_ptr predicate_normaliser_core_walkers =
    SLIST(NuSMVEnv_get_value(env,
                             MASTER_COMPILE_PREDICATE_NORMALISER_CORE_WALKERS));
  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  nusmv_assert(predicate_normaliser_core_walkers != NULL);

  if (opt_verbose_level_gt(opts, 4)) {
    const Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger,
               "Registering the predicate normaliser "
               "within the given environment...\n");
  }
  //add predicate_normaliser_core_walker to predicate_normaliser_core_walkers Slist in the env
  Slist_push(predicate_normaliser_core_walkers, predicate_normaliser_core_walker);
}

boolean
PnePkg_unregister_predicate_normaliser_walker(NuSMVEnv_ptr env,
                                              const char* walker_name)
{
  Siter iter;
  PredicateNormaliserCoreWalker_ptr predicate_normaliser_core_walker;
  Slist_ptr predicate_normaliser_core_walkers = (Slist_ptr) NuSMVEnv_get_value(env,
                                                                               MASTER_COMPILE_PREDICATE_NORMALISER_CORE_WALKERS);
  OptsHandler_ptr opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
  nusmv_assert(predicate_normaliser_core_walkers != NULL);
  if (opt_verbose_level_gt(opts, 4)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger,
               "Removing the predicate normalise instance "
               "within the given environment...\n");
  }
  //search predicate_normaliser_core_walker in predicate_normaliser_core_walkers Slist in th env
  //solutions to optimize the following code are welcome
  SLIST_FOREACH(predicate_normaliser_core_walkers, iter) {
    predicate_normaliser_core_walker = (PredicateNormaliserCoreWalker_ptr) Siter_element(iter);
    if (strcmp(predicate_normaliser_core_walker->pncw_name, walker_name) == 0) {
      return Slist_remove(predicate_normaliser_core_walkers, predicate_normaliser_core_walker);
    }
  }
  return false;
}

/*!
  \brief Initialization of the predicate normaliser list package.

  Initialization of the predicate normaliser list package.

  It associates a predicate normaliser list to the global environment.
*/
void PnePkg_init_predicate_normaliser_walkers_list(NuSMVEnv_ptr env)
{
  Slist_ptr predicate_normaliser_core_walkers;
  OptsHandler_ptr opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  /* initialize predicate_normaliser_core_walkers Slist in the env */
  if (!NuSMVEnv_has_value(env, MASTER_COMPILE_PREDICATE_NORMALISER_CORE_WALKERS)) {
    if (opt_verbose_level_gt(opts, 4)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger,
               "Initializing the PredicateNormaliserCoreWalkers Slist "
               "within the given environment...\n");
    }
    predicate_normaliser_core_walkers = Slist_create();
    NuSMVEnv_set_value(env,
                       MASTER_COMPILE_PREDICATE_NORMALISER_CORE_WALKERS,
                       predicate_normaliser_core_walkers);
  }
}

void PnePkg_destroy_predicate_normaliser_walkers_list(NuSMVEnv_ptr env)
{
  const OptsHandler_ptr opts =
    OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  const Slist_ptr predicate_normaliser_core_walkers =
    SLIST(NuSMVEnv_remove_value(env, MASTER_COMPILE_PREDICATE_NORMALISER_CORE_WALKERS));

  /* deallocate elements in the list */
  while (!Slist_is_empty(predicate_normaliser_core_walkers)) {
    PredicateNormaliserCoreWalker_ptr predicate_normaliser_core_walker =
      (PredicateNormaliserCoreWalker_ptr) Slist_pop(predicate_normaliser_core_walkers);
    FREE(predicate_normaliser_core_walker);
  }

  if (opt_verbose_level_gt(opts, 4)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger,
               "Clearing the Predicate instance in "
               "the given environment...\n");
  }
  Slist_destroy(predicate_normaliser_core_walkers);
}

/*---------------------------------------------------------------------------*/
/* Predicate Extractor                                                       */
/*---------------------------------------------------------------------------*/

/*!
  \brief Initialization of the predicate extractor in the predicate package.

  Initialization of the predicate extractor in the predicate package.

  It associates a predicate extractor to the global environment.
*/
void PredicateExtractorPkg_init(NuSMVEnv_ptr env)
{
  OptsHandler_ptr opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  if (opt_verbose_level_gt(opts, 4)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger,
               "Instantiating the Predicate Extractor instance "
               "within the given environment...\n");
  }

  PnePkg_init_predicate_extractor_walkers_list(env);
  {
    PredicateExtractorCoreWalker_ptr pe_core_walker;
    pe_core_walker = ALLOC(PredicateExtractorCoreWalker, 1);
    pe_core_walker->pecw_name = PREDICATE_EXTRACTOR_CORE_NAME;
    pe_core_walker->pecw_function_create = PredicateExtractorCore_create;
    PnePkg_register_predicate_extractor_walker(env,
                                               pe_core_walker);
  }
}

/*!
  \brief Deinitialization of the predicate extractor package


*/
void PredicateExtractorPkg_quit(NuSMVEnv_ptr env)
{
  OptsHandler_ptr opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  if (opt_verbose_level_gt(opts, 4)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger,
               "Clearing the Predicate Extractor instance in "
               "the given environment...\n");
  }
  /* Destroy predicate extractor walkers list */
  PnePkg_destroy_predicate_extractor_walkers_list(env);
}

void
PnePkg_register_predicate_extractor_walker(NuSMVEnv_ptr env,
                                           PredicateExtractorCoreWalker_ptr predicate_extractor_core_walker)
{
  Slist_ptr predicate_extractor_core_walkers =
    SLIST(NuSMVEnv_get_value(env,
                             MASTER_COMPILE_PREDICATE_EXTRACTOR_CORE_WALKERS));
  OptsHandler_ptr opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  nusmv_assert(predicate_extractor_core_walkers != NULL);

  if (opt_verbose_level_gt(opts, 4)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger,
               "Registering the predicate extractor instance "
               "within the given environment...\n");
  }

  Slist_push(predicate_extractor_core_walkers, predicate_extractor_core_walker);
}

boolean
PnePkg_unregister_predicate_extractor_walker(NuSMVEnv_ptr env,
                                             const char* walker_name)
{
  const OptsHandler_ptr opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));
  const Slist_ptr predicate_extractor_core_walkers =
    SLIST(NuSMVEnv_get_value(env,
                             MASTER_COMPILE_PREDICATE_EXTRACTOR_CORE_WALKERS));
  Siter iter;

  nusmv_assert(predicate_extractor_core_walkers != NULL);

  if (opt_verbose_level_gt(opts, 4)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger,
               "Removing the predicate extractor instance "
               "within the given environment...\n");
  }
  //search predicate_extractor_core_walker in predicate_extractor_core_walkers Slist in th env
  //solutions to optimize the following code are welcome
  SLIST_FOREACH(predicate_extractor_core_walkers, iter) {
    PredicateExtractorCoreWalker_ptr predicate_extractor_core_walker =
      (PredicateExtractorCoreWalker_ptr) Siter_element(iter);

    if (strcmp(predicate_extractor_core_walker->pecw_name, walker_name) == 0) {
      const boolean retval =
        Slist_remove(predicate_extractor_core_walkers,
                     predicate_extractor_core_walker);
      if (retval)
        FREE(predicate_extractor_core_walker);
      return retval;
    }
  }
  return false;
}

/*!
  \brief Initialization of the predicate extractor list package.

  Initialization of the predicate extractor list package.

  It associates a predicate extractor list to the global environment.
*/
void PnePkg_init_predicate_extractor_walkers_list(NuSMVEnv_ptr env)
{
  Slist_ptr predicate_extractor_core_walkers;
  OptsHandler_ptr opts = OPTS_HANDLER(NuSMVEnv_get_value(env, ENV_OPTS_HANDLER));

  /* initialize predicate_extractor_core_walkers Slist in the env */
  if (!NuSMVEnv_has_value(env, MASTER_COMPILE_PREDICATE_EXTRACTOR_CORE_WALKERS)) {
    if (opt_verbose_level_gt(opts, 4)) {
      Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
      Logger_log(logger,
                "Initializing the PredicateExtractorCoreWalkers Slist "
                "within the given environment...\n");
    }
    predicate_extractor_core_walkers = Slist_create();
    NuSMVEnv_set_value(env,
                       MASTER_COMPILE_PREDICATE_EXTRACTOR_CORE_WALKERS,
                       predicate_extractor_core_walkers);
  }
}


/*!
  \brief Deinitialization of the predicate extractor list package


*/
void PnePkg_destroy_predicate_extractor_walkers_list(NuSMVEnv_ptr env)
{
  Slist_ptr predicate_extractor_core_walkers;
  PredicateExtractorCoreWalker_ptr predicate_extractor_core_walker;

  OptsHandler_ptr opts = OPTS_HANDLER(NuSMVEnv_get_value(env,
                                                         ENV_OPTS_HANDLER));

  predicate_extractor_core_walkers = (Slist_ptr) NuSMVEnv_remove_value(env,
                                                                       MASTER_COMPILE_PREDICATE_EXTRACTOR_CORE_WALKERS);

  /* deallocate elements in the list */
  while (!Slist_is_empty(predicate_extractor_core_walkers)) {
    predicate_extractor_core_walker =
      (PredicateExtractorCoreWalker_ptr) Slist_pop(predicate_extractor_core_walkers);
    FREE(predicate_extractor_core_walker);
  }

  if (opt_verbose_level_gt(opts, 4)) {
    Logger_ptr logger = LOGGER(NuSMVEnv_get_value(env, ENV_LOGGER));
    Logger_log(logger,
               "Clearing the Predicate Extractor instance in "
               "the given environment...\n");
  }
  Slist_destroy(predicate_extractor_core_walkers);
}
