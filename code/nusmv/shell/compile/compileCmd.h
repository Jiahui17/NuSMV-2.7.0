/* ---------------------------------------------------------------------------


  This file is part of the ``compile'' package.
  %COPYRIGHT%
  

-----------------------------------------------------------------------------*/

/*!
  \author Michele Dorigatti
  \brief Module header file for shell commands

  \todo: Missing description

*/


#ifndef __NUSMV_SHELL_COMPILE_COMPILE_CMD_H__
#define __NUSMV_SHELL_COMPILE_COMPILE_CMD_H__

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
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/*!
  \brief Initializes the commands provided by this package

  
*/
void Compile_init_cmd(NuSMVEnv_ptr env);
int CommandGoBmc(NuSMVEnv_ptr env, int argc, char** argv);
int CommandGo(NuSMVEnv_ptr env, int argc, char** argv);

/**AutomaticEnd***************************************************************/

#endif /* __NUSMV_SHELL_COMPILE_COMPILE_CMD_H__ */
