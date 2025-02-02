# NuSMV/code/nusmv/core/utils/meson.build

core_c += files(
  'array.c',
  'avl.c',
  'heap.c',
  'NodeList.c',
  'Olist.c',
  'range.c',
  'Sset.c',
  'ucmd.c',
  'utils.c',
  'EnvObject.c',
  'OStream.c',
  'Logger.c',
  'assoc.c',
  'error.c',
  'list.c',
  'object.c',
  'portability.c',
  'Slist.c',
  'TimerBench.c',
  'ustring.c',
  'utils_io.c',
  'NodeGraph.c',
  'Stack.c',
  'Pair.c',
  'Triple.c',
  'UStringMgr.c',
  'StreamMgr.c',
  'ErrorMgr.c',
  'LRUCache.c',
  'OAHash.c',
  'Tuple5.c',
  'DLlist.c',
  'BiMap.c',
  'watchdog_util.c',
  'Vector.c',
)

if libgmp_dep.found()
  core_c += files(
    'BigWordNumber.c',
    'BigWordNumberMgr.c',
    'BigWordNumberInt.c',

    'bignumbers/bignumbers.c',
    'bignumbers/numbers.c',
  )
else
  core_c += files(
    'WordNumber.c',
    'WordNumberMgr.c',
  )
endif

if cc_msvc
  core_c += files(
    'utils_msvc.c',
  )
endif
