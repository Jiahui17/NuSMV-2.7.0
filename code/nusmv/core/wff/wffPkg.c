/* ---------------------------------------------------------------------------


  This file is part of the ``wff'' package of NuSMV version 2.
  Copyright (C) 2011 by FBK-irst.

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
  \brief Initialization and deinitialization for package wff and
  subpackages

  Initialization and deinitialization for package wff and
  subpackages

*/


#include "nusmv/core/wff/wffPkg.h"
#include "nusmv/core/utils/Slist.h"
#include "nusmv/core/node/NodeWalker.h"
#include "nusmv/core/wff/rewriters/WffRewriterCore.h"
#include "nusmv/core/wff/w2w/w2wInt.h"
#include "nusmv/core/wff/ExprMgr.h"
#include "nusmv/core/wff/lr/MasterLogicRecognizer.h"

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
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
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

void WffPkg_init(const NuSMVEnv_ptr env)
{
  {
    const ExprMgr_ptr expr_mgr = ExprMgr_create(env);
    NuSMVEnv_set_value(env, ENV_EXPR_MANAGER, expr_mgr);
  }

  {
    const MasterLogicRecognizer_ptr master_recogn =
      MasterLogicRecognizer_create_with_default_recognizers(env);
    NuSMVEnv_set_value(env, ENV_MASTER_LOGIC_RECOGNIZER, master_recogn);
  }

  { /* create list of rewriters in the environment,
       MasterWffRewriter_create takes each of the walkers in the list and
       registers it.
    */
    const Slist_ptr rewriters_list = Slist_create();

    NuSMVEnv_set_value(env, ENV_WFF_REWRITER_LIST, rewriters_list);

    WffPkg_register_rewriter(env, (typeWffRewriterCreate)WffRewriterCore_create);
  }
}

void WffPkg_quit(const NuSMVEnv_ptr env)
{

  {
    const hash_ptr wff2nnf =
      NuSMVEnv_get_handled_hash_ptr(env, ENV_W2W_WFF2NNF_HASH);
    clear_assoc(wff2nnf);
  }

  {
    const ExprMgr_ptr expr_mgr =
      EXPR_MGR(NuSMVEnv_remove_value(env, ENV_EXPR_MANAGER));
    ExprMgr_destroy(expr_mgr);
  }

  {
    const MasterLogicRecognizer_ptr master_recogn =
      NuSMVEnv_remove_value(env, ENV_MASTER_LOGIC_RECOGNIZER);
    MasterLogicRecognizer_destroy(master_recogn);
  }

  { /* free rewriters list */
    const Slist_ptr rewriters_list =
      SLIST(NuSMVEnv_remove_value(env, ENV_WFF_REWRITER_LIST));

    Slist_destroy(rewriters_list);
  }
}

void WffPkg_register_rewriter(const NuSMVEnv_ptr env,
                              const typeWffRewriterCreate factory_fun)
{
  const Slist_ptr rewriters_list =
      SLIST(NuSMVEnv_get_value(env, ENV_WFF_REWRITER_LIST));

  SLIST_CHECK_INSTANCE(rewriters_list);
  nusmv_assert(factory_fun != NULL);

  nusmv_assert(!Slist_contains(rewriters_list, factory_fun));

  Slist_push(rewriters_list, factory_fun);
}

boolean WffPkg_unregister_rewriter(const NuSMVEnv_ptr env,
                                   const typeWffRewriterCreate factory_fun)
{
  const Slist_ptr rewriters_list =
    SLIST(NuSMVEnv_get_value(env, ENV_WFF_REWRITER_LIST));

  SLIST_CHECK_INSTANCE(rewriters_list);
  nusmv_assert(factory_fun != NULL);

  return Slist_remove(rewriters_list, factory_fun);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
