/* ---------------------------------------------------------------------------


  This file is part of the ``prop'' package.
  %COPYRIGHT%


-----------------------------------------------------------------------------*/

/*!
  \author Michele Dorigatti
  \brief The public implementation of the prop package

  \todo: Missing description

*/

#include "nusmv/core/prob/prop/propInt.h"
#include "nusmv/core/prob/prop/propProp.h"
#include "nusmv/core/prob/prop/PropDbAccessor.h"
#include "nusmv/core/utils/ErrorMgr.h"
#include "nusmv/core/utils/ucmd.h"

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

lsList Prop_convert_props_to_invar(PropDbAccessor_ptr prop_dba, lsList props)
{
  lsGen iterator;
  Prop_ptr prop = NULL;
  Prop_ptr invar = NULL;
  lsList retval = NULL;

  retval = lsCreate();

  lsForEachItem(props, iterator, prop) {
    invar = Prop_convert_to_invar(prop);

    if (NULL != invar) {
      int index = 0;
      lsStatus ls_status = LS_NIL;

#ifndef NDEBUG
      {
        NuSMVEnv_ptr const env = EnvObject_get_environment(ENV_OBJECT(prop_dba));
        SymbTable_ptr const symb_table =
          SYMB_TABLE(NuSMVEnv_get_value(env, ENV_SYMB_TABLE));
        TypeChecker_ptr type_checker = SymbTable_get_type_checker(symb_table);

        nusmv_assert(TypeChecker_check_property(type_checker, invar));
      }
#endif

      PropDbAccessor_add(prop_dba, invar);
      index = Prop_get_index(invar);
      ls_status = lsNewEnd(retval, PTR_FROM_INT(lsGeneric, index), LS_NH);
      nusmv_assert(LS_OK == ls_status);
    }
  }

  return retval;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
