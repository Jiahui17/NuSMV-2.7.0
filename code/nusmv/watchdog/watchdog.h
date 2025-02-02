/*!
  \author Sergio Mover
  \brief Api of the watchdog library.

  Library for watchdog library.
*/

#ifndef WATCHDOG_LIBRARY_H
#define WATCHDOG_LIBRARY_H

#include "stdint.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef enum watchdogOption {
  TIMER_PERIODIC,
  TIMER_ONE_SHOT,
} watchdogOption;
typedef void (*watchdogActionPointer)(void);

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

int watchdog_mod(char* name, unsigned long long int period,
                 void (*action) (void*), watchdogOption option, void* env);
int watchdog_start(char* name);
int watchdog_pause(char* name);
/* The function does not exist in the target watchdog library implementation */
/* unsigned long long int watchdog_get_status(const char* name, int* status); */

#endif /* WATCHDOG_LIBRARY_H */
