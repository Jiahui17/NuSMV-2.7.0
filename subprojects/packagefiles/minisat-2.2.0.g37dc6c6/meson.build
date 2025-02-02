# minisat/meson.build

project(
  'MiniSat', ['c', 'cpp'],
  version: '2.2.0.g37dc6c6',
)


### Environment #############################################################

cc = meson.get_compiler('c')

subdir('external')
subdir('minisat')


### Compilation #############################################################

add_project_arguments(
  '-D__STDC_FORMAT_MACROS',
  '-D__STDC_LIMIT_MACROS',
  language: ['c', 'cpp'],
)

minisat_lib = library(
  'minisat', [
    core_c,
    simp_c,
    utils_c,
  ],
  include_directories: [
    external_inc,
    core_inc,
    simp_inc,
    utils_inc,
    'minisat',
  ],
  install: true,
)


### Variables ###############################################################

libminisat_var = {
  'has_preferred' : 'true',
  'has_dec_budget': 'false', # needs minisat-2.2.0.g37dc6c6+0001-proof.patch
  'has_proof'     : 'false', # needs minisat-2.2.0.g37dc6c6+0001-proof.patch
}

summary(
  libminisat_var,
  section: 'Variables',
)


### Library #################################################################

libminisat_dep = declare_dependency(
  include_directories: '.',
  link_with: minisat_lib,
  variables: libminisat_var,
)

meson.override_dependency('minisat-2.2.0.g37dc6c6', libminisat_dep)

pkgconfig = import('pkgconfig')
pkgconfig.generate(
  minisat_lib,
  description: 'A minimalistic and high-performance SAT solver',
  url: 'http://minisat.se',
  variables: libminisat_var,
  uninstalled_variables: libminisat_var,
)
