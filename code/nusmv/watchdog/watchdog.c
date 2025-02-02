/*!
  \author Sergio Mover
  \brief Implementation of the watchdog library functions.
*/

#include "stdio.h"
#include "watchdog.h"

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

int watchdog_mod(char* name, unsigned long long int period,
                 void (*action) (void*),
                 watchdogOption option, void* env)
{
  fprintf(stderr, "Creating watchdog %s with period %d\n", name, (int) period);
  if (TIMER_PERIODIC == option) {
    fprintf(stderr, "Watchdog option is TIMER_PERIODIC\n");
  }
  else {
    fprintf(stderr, "Watchdog option is TIMER_ONE_SHOT\n");
  }

  return 0;
}

int watchdog_start(char* name)
{
  fprintf(stderr, "Starting the watchdog %s\n", name);
  return 0;
}

int watchdog_pause(char* name)
{
  fprintf(stderr, "Pausing the watchdog %s\n", name);

  return 0;
}

/* The function does not exists in the watchdog library implementation */
/* unsigned long long int watchdog_get_status(const char* name, int* status) */
/* { */
/*   fprintf(stderr, "Status of %s\n", name); */
/*   *status = 1; */
/*   return 0; */
/* } */
