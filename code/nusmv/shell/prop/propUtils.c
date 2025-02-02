#include "nusmv/shell/prop/prop.h"
#include "nusmv/core/compile/compile.h"
#include "nusmv/core/utils/ucmd.h"

extern cmp_struct_ptr cmps;


int PropShell_check_prop_index(const PropDbAccessor_ptr pdba, int idx)
{
  NuSMVEnv_ptr env;
  StreamMgr_ptr streams;
  int low, high;

  env = EnvObject_get_environment(ENV_OBJECT(pdba));
  streams = STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));

  ProblemDbAccessor_get_indices_range(PROBLEM_DB_ACCESSOR(pdba),
                                      &low, &high);
  if (low < 0) {
    if (cmp_struct_get_flatten_hrc(cmps) == 0) {
      StreamMgr_print_error(
          streams,
          "The hierarchy must be flattened before. "    \
          "Use the \"flatten_hierarchy\" command.\n");
    }
    else {
      StreamMgr_print_error(
          streams,
          "Error: there isn\'t any property available.\n");
    }
    return 1;
  }

  if (idx < low || idx > high) {
    StreamMgr_print_output(
        streams,
        "Error: %d is not a valid property index, "
        "must be in the range [%d,%d].\n",
        idx, low, high);
    return 1;
  }

  if (!ProblemDbAccessor_is_index_valid(PROBLEM_DB_ACCESSOR(pdba), idx)) {
    StreamMgr_print_output(
        streams,
        "Error: %d is not a valid property index, "
        "as it does not contain a property\n",
        idx);
    return 1;
  }

  return 0;
}


int PropShell_get_prop_index_from_string(const PropDbAccessor_ptr pdba,
                                         const char* idx)
{
  NuSMVEnv_ptr env;
  StreamMgr_ptr streams;
  int num;

  env = EnvObject_get_environment(ENV_OBJECT(pdba));
  streams = STREAM_MGR(NuSMVEnv_get_value(env, ENV_STREAM_MANAGER));

  if (util_str2int(idx, &num) != 0) {
    StreamMgr_print_error(
        streams,
        "Error: property index \"%s\" is not a valid value "    \
        "(must be integer).\n", idx);
    return -1;
  }

  if (PropShell_check_prop_index(pdba, num))
    return -1;

  return num;
}
