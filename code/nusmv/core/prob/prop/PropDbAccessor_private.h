/* ---------------------------------------------------------------------------

  This file is part of the ``prop'' package of NuSMV version 2.
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
  \brief Private and protected interface of class 'PropDbAccessor'

  This file can be included only by derived and friend classes
*/



#ifndef __NUSMV_CORE_PROP_PROP_DB_ACCESSOR_PRIVATE_H__
#define __NUSMV_CORE_PROP_PROP_DB_ACCESSOR_PRIVATE_H__

#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#include "nusmv/core/prob/prop/PropDbAccessor.h"
#include "nusmv/core/prob/prop/Prop.h"

#include "nusmv/core/prob/ProblemDbAccessor.h"
#include "nusmv/core/prob/ProblemDbAccessor_private.h"

#include "nusmv/core/utils/utils.h"

/*!
  \brief PropDbAccessor class definition derived from
               class Object



  \sa Base class Object
*/


/* Those are the types of the virtual methods. They can be used for
   type casts in subclasses. */

/*!
  \brief \todo Missing synopsis

  \todo Missing description
*/
typedef int (*PropDbAccessor_prop_create_and_add_method)(PropDbAccessor_ptr,
                                                         SymbTable_ptr,
                                                         node_ptr,
                                                         Prop_Type);

/*!
  \brief The problem database accessor for properties

  \sa ProblemDbAccessor
*/
typedef void (*PropDbAccessor_verify_all_method)(const PropDbAccessor_ptr);


typedef struct PropDbAccessor_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(ProblemDbAccessor);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  PropDb_PrintFmt print_fmt; /* print format */

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */
  /* int (*)(PropDbAccessor_ptr, SymbTable_ptr, node_ptr, Prop_Type) */
  PropDbAccessor_prop_create_and_add_method prop_create_and_add;

  /* void (*)(const PropDbAccessor_ptr) */
  PropDbAccessor_verify_all_method verify_all;

} PropDbAccessor;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only        */
/* ---------------------------------------------------------------------- */

/*!
  \methodof PropDbAccessor
  \brief The PropDbAccessor class private initializer

  The PropDbAccessor class private initializer

  \sa PropDbAccessor_create
*/
void prop_db_accessor_init(PropDbAccessor_ptr self,
                           ProblemDb_ptr db,
                           long long tags);

/*!
  \methodof PropDbAccessor
  \brief The PropDbAccessor class private deinitializer

  The PropDbAccessor class private deinitializer

  \sa PropDbAccessor_destroy
*/
void prop_db_accessor_deinit(PropDbAccessor_ptr self);

/*!
  \methodof PropDbAccessor
  \brief Inserts a property in the DB of properties

  Given a formula and its type, a property is
  created and stored in the DB of properties. It returns either -1 in
  case of failure, or the index of the inserted property.

*/
int prop_db_accessor_prop_create_and_add(PropDbAccessor_ptr self,
                                         SymbTable_ptr symb_table,
                                         node_ptr spec,
                                         Prop_Type type);

/*!
  \methodof PropDbAccessor
  \brief Verifies all the properties in the DB

  All the properties stored in the database not
  yet verified will be verified. The properties are verified following
  the order CTL/COMPUTE/LTL/INVAR.
*/
void prop_db_accessor_verify_all(const PropDbAccessor_ptr self);


#endif /* __NUSMV_CORE_PROP_PROP_DB_ACCESSOR_PRIVATE_H__ */
