/*!
  \author Sergio Mover
  \brief Implementation of the profiling library functions.
*/

#include "stdio.h"
#include "assert.h"

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

static FILE* out_file_handle;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

void profiler_enable(char* file_name)
{
  out_file_handle = fopen(file_name, "w");
  /* the library does not handle errors at all */
  assert((FILE*) NULL != out_file_handle);
}

void profiler_disable(void)
{
  assert((FILE*) NULL != out_file_handle);
  fflush(out_file_handle);
  fclose(out_file_handle);
}

void profiler_initialize(void)
{
  /* don't know what this function should do, just print an output to stdout */
  fprintf(stderr, "Profiler library call: profiler_initialize\n");
  fprintf(out_file_handle, "Profiler library call: profiler_initialize\n");
}

void profiler_sampling(int id)
{
  /* don't know what this function should do */
  fprintf(stdout, "Profiler library call: profiler_sampling(%d)\n", id);
  fprintf(out_file_handle, "Profiler library call: profiler_sampling(%d)\n", id);
  fflush(out_file_handle);
}

void profiler_output_message(const char* msg, const int bmc_step)
{
  fprintf(stdout, "Profiler library call: " \
          "profiler_output_message\nMessage: %s\nbmc_step: %d\n", msg, bmc_step);
  fprintf(out_file_handle, "%s %d\n", msg, bmc_step);
  fflush(out_file_handle);
}

