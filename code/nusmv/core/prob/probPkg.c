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
  \author Marco Roveri, Roberto Cavada
  \brief Main routines for the prob package initialization
*/


#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#include "nusmv/core/prob/prop/propInt.h"
#include "nusmv/core/prob/probPkg.h"
#include "nusmv/core/prob/prop/Prop.h"
#include "nusmv/core/prob/prop/PropDbAccessor.h"

#include "nusmv/core/cinit/NuSMVEnv.h"
/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


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

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

void ProbPkg_init(NuSMVEnv_ptr env)
{
  ProblemDb_ptr db;
  PropDbAccessor_ptr prop_dba;

  db = ProblemDb_create(env);
  NuSMVEnv_set_value(env, ENV_PROBLEM_DB, db);

  prop_dba = PropDbAccessor_create(db);
  NuSMVEnv_set_value(env, ENV_PROP_DBA, prop_dba);
}

void ProbPkg_quit(NuSMVEnv_ptr env)
{
  ProblemDb_ptr db =
      PROBLEM_DB(NuSMVEnv_remove_value(env, ENV_PROBLEM_DB));
  PropDbAccessor_ptr pdba =
      PROP_DB_ACCESSOR(NuSMVEnv_remove_value(env, ENV_PROP_DBA));

  PropDbAccessor_destroy(pdba);
  ProblemDb_destroy(db);
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
