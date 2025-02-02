# NuSMV/code/nusmv/core/sat/meson.build

core_c += files(
  'satUtils.c',
  'SatSolver.c',
  'SatIncSolver.c',
)

if with_minisat_ft.enabled()
  core_c += files(
    'solvers/SatMinisat.c',
  )
endif
