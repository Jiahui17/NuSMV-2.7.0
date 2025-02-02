# minisat/minisat/meson.build

subdir('core')
subdir('simp')
subdir('utils')

install_headers(
  # find core mtl simp utils -name '*.h' -printf "  '%p',\n"
  'core/Dimacs.h',
  'core/Solver.h',
  'core/SolverTypes.h',
  'mtl/Alg.h',
  'mtl/Alloc.h',
  'mtl/Heap.h',
  'mtl/IntMap.h',
  'mtl/IntTypes.h',
  'mtl/Map.h',
  'mtl/Queue.h',
  'mtl/Rnd.h',
  'mtl/Sort.h',
  'mtl/Vec.h',
  'mtl/XAlloc.h',
  'simp/SimpSolver.h',
  'utils/Options.h',
  'utils/ParseUtils.h',
  'utils/System.h',
  subdir: 'minisat',
  preserve_path: true,
)
