# NuSMV/meson.build

project(
  'NuSMV', 'c',
  version: '2.7.0',
  license: 'LGPL-2.1-only',
  license_files: 'LGPL-2.1',
  default_options: {
    # no shared libraries due to mutual dependencies
    'default_library': 'static',
  },
  meson_version: '>=1.5.0',
)

fs = import('fs')

summary({
    'build options'      : meson.build_options(),
    'global source root' : meson.global_source_root(),
    'global build root'  : meson.global_build_root(),
    'is cross build'     : meson.is_cross_build(),
    'is unity'           : meson.is_unity(),
    'is subproject'      : meson.is_subproject(),
    'project source root': meson.project_source_root(),
    'project build root' : meson.project_build_root(),
  },
  section: 'Meson',
)


### Options #################################################################

build_release = get_option('buildtype') == 'release'
prefer_static = get_option('prefer_static')

datadir    = get_option('datadir'   ) / 'nusmv'
includedir = get_option('includedir') / 'nusmv'

with_compass  = get_option('with-compass' ).enable_auto_if(build_release).allowed()
with_ltl2smv  = get_option('with-ltl2smv' ).enable_auto_if(build_release).allowed()
with_profiler = get_option('with-profiler')                              .enabled()
with_watchdog = get_option('with-watchdog')                              .enabled()

build_doc_ft       = get_option('build-doc'      ).enable_auto_if(build_release)
with_bignumbers_ft = get_option('with-bignumbers')
with_gtest_ft      = get_option('with-gtest'     )\
  .disable_if(
    build_release,
    error_message: 'linking a relase build to gtest is nonsense',
  )
with_libxml2_ft    = get_option('with-libxml2'   ).enable_auto_if(build_release)
with_minisat_ft    = get_option('with-minisat'   ).enable_auto_if(build_release)
with_shell_ft      = get_option('with-shell'     ).enable_auto_if(build_release)

## Summary
summary({
    'build-doc'      : build_doc_ft,
    'with-bignumbers': with_bignumbers_ft,
    'with-compass'   : with_compass,
    'with-gtest'     : with_gtest_ft,
    'with-libxml2'   : with_libxml2_ft,
    'with-ltl2smv'   : with_ltl2smv,
    'with-minisat'   : with_minisat_ft,
    'with-profiler'  : with_profiler,
    'with-shell'     : with_shell_ft,
    'with-watchdog'  : with_watchdog,
  },
  section: 'Options',
)


### Environment #############################################################

## Host
linux   = host_machine.system() == 'linux'
cygwin  = host_machine.system() == 'cygwin'
darwin  = host_machine.system() == 'darwin'
windows = host_machine.system() == 'windows'
unix    = linux or cygwin or darwin

## C
cc = meson.get_compiler('c')
cc_gcc  = cc.get_id() == 'gcc'
cc_msvc = cc.get_id() == 'msvc'

## C++
cpp_found = add_languages('cpp', required: with_gtest_ft)
if cpp_found
  cpp = meson.get_compiler('cpp')
endif

## Python
python = find_program('python')

meson_util_py = find_program(
  'meson-util.py',
  dirs: meson.current_source_dir() / 'scripts',
)

## Summary
summary({
    'Host OS'   : host_machine.system(),
    'C compiler': cc.get_id(),
    'C standard': get_option('c_std'),
  },
  section: 'Environment',
)


### Dependencies ############################################################

## Libraries

# libm
libm_dep = unix\
    ? cc.find_library('m')\
    : dependency('', required: false)

# CUDD
libcudd_dep = dependency(
  'cudd-2.4.1',
)

# minisat
libminisat_dep = dependency(
  'minisat-2.2.0.g37dc6c6',
  required: with_minisat_ft,
)

# GMP
libgmp_dep = dependency(
  'gmp',
  required: with_bignumbers_ft,
)

# Google Test
libgtest_dep = dependency(
  'gtest',
  required: with_gtest_ft.require(cpp_found),
  disabler: true,
)
libgtest_main_dep = dependency(
  'gtest_main',
  required: with_gtest_ft.require(cpp_found),
  disabler: true,
)

# editline
libeditline_dep = dependency(
  'libedit', 'wineditline',
  required: with_shell_ft,
)
with_libedit     = libeditline_dep.name() == 'libedit'
with_wineditline = libeditline_dep.name() == 'wineditline'

# regex
if cc.has_function('regcomp')
  libregex_dep = dependency('', required: false)
else
  libregex_dep = dependency(
    'gnurx',
    required: true,
  )
endif

# libxml2
libxml2_dep = dependency(
  'libxml2', 'libxml-2.0',
  required: with_libxml2_ft,
)

# iconv
libiconv_dep = cc_msvc\
    ? dependency('iconv')\
    : dependency('', required: false)


## Internal Programs
combine_grammar_py = files('code/scripts/combine_grammar.py')[0]
create_help_py     = files('code/scripts/create_help.py'    )[0]


## External Programs
bison = find_program('bison')
flex  = find_program('flex' )

doxygen = find_program(
  'doxygen',
  required: build_doc_ft,
  disabler: true,
)
latexmk = find_program(
  'latexmk',
  required: build_doc_ft,
  disabler: true,
)

cppcheck = find_program(
  'cppcheck',
  required: false,
  disabler: true,
)


### Compilation #############################################################

nusmv_var = {
  'with_bignumbers': libgmp_dep     .found().to_string(),
  'with_compass'   : with_compass           .to_string(),
  'with_libxml2'   : libxml2_dep    .found().to_string(),
  'with_ltl2smv'   : with_ltl2smv           .to_string(),
  'with_profiler'  : with_profiler          .to_string(),
  'with_shell'     : libeditline_dep.found().to_string(),
  'with_watchdog'  : with_watchdog          .to_string(),
}

code_inc = include_directories('code')
addon_c             = []
addon_generated_h   = []
addon_inc           = []
core_c              = []
core_generated_h    = []
grammar_c           = []
grammar_generated_h = []
profiler_c          = []
profiler_inc        = []
rbc_c               = []
shell_c             = []
watchdog_c          = []
watchdog_inc        = []

subdir('code')

# addons
nusmvaddonscore_lib = library(
  'nusmvaddonscore', [
    addon_c,
  ],
  sources: addon_generated_h,
  include_directories: [
    addon_inc,
    code_inc,
  ],
  dependencies: [
      libcudd_dep,
  ],
  install: true,
)
libnusmvaddonscore_dep = declare_dependency(
  include_directories: [
    code_inc,
  ],
  dependencies: [
      libcudd_dep,
  ],
  link_with: nusmvaddonscore_lib,
  variables: nusmv_var,
)
meson.override_dependency('nusmvaddonscore', libnusmvaddonscore_dep)

# profiler
if with_profiler
  profiler_lib = library(
    'nusmvprofiler',
    profiler_c,
    include_directories: profiler_inc,
    install: true,
  )
  libnusmvprofiler_dep = declare_dependency(
    include_directories: profiler_inc,
    link_with: profiler_lib,
  )
  meson.override_dependency('nusmvprofiler', libnusmvprofiler_dep)
else
  libnusmvprofiler_dep = dependency('', required: false)
endif

# watchdog
if with_watchdog
  watchdog_lib = library(
    'nusmvwatchdog',
    watchdog_c,
    include_directories: watchdog_inc,
    install: true,
  )
  libnusmvwatchdog_dep = declare_dependency(
    include_directories: watchdog_inc,
    link_with: watchdog_lib,
  )
  meson.override_dependency('nusmvwatchdog', libnusmvwatchdog_dep)
else
  libnusmvwatchdog_dep = dependency('', required: false)
endif

# core
nusmvcore_lib = library(
  'nusmvcore', [
    addon_c,
    core_c,
  ],
  sources: core_generated_h,
  include_directories: [
    addon_inc,
    code_inc,
  ],
  dependencies: [
    libcudd_dep,
    libgmp_dep,
    libminisat_dep,
    libnusmvprofiler_dep,
    libnusmvwatchdog_dep,
    libregex_dep,
    libxml2_dep,
  ],
  install: true,
)
libnusmvcore_dep = declare_dependency(
  link_with: nusmvcore_lib,
  include_directories: code_inc,
  dependencies: [
    libcudd_dep,
    libnusmvprofiler_dep,
    libnusmvwatchdog_dep,
    libxml2_dep,
  ],
  variables: nusmv_var,
)
meson.override_dependency('nusmvcore', libnusmvcore_dep)

nusmvgrammar_lib = library(
  'nusmvgrammar', [
    grammar_c,
  ],
  sources: grammar_generated_h,
  include_directories: code_inc,
  dependencies: [
    libcudd_dep,
  ],
  install: true,
)
libnusmvgrammar_dep = declare_dependency(
  link_with: nusmvgrammar_lib,
  variables: nusmv_var,
)
meson.override_dependency('nusmvgrammar', libnusmvgrammar_dep)

nusmvrbc_lib = library(
  'nusmvrbc',
  rbc_c,
  include_directories: code_inc,
  dependencies: libcudd_dep,
  install: true,
)
libnusmvrbc_dep = declare_dependency(
  link_with: nusmvrbc_lib,
  variables: nusmv_var,
)
meson.override_dependency('nusmvrbc', libnusmvrbc_dep)

# shell
shell_dep = []
if libeditline_dep.found()
  nusmvshell_lib = library(
    'nusmvshell', [
      shell_c,
    ],
    include_directories: code_inc,
    dependencies: [
      libcudd_dep,
      libeditline_dep,
      libnusmvprofiler_dep,
      libnusmvwatchdog_dep,
    ],
    install: true,
  )
  libnusmvshell_dep = declare_dependency(
    link_with: nusmvshell_lib,
    variables: nusmv_var,
  )
  meson.override_dependency('nusmvshell', libnusmvshell_dep)
  shell_dep = [
    libeditline_dep,
    libnusmvshell_dep,
  ]
endif

## Executables
nusmv_exe = executable(
  'NuSMV',
  'code/nusmv/main.c',
  include_directories: code_inc,
  dependencies: [
    libcudd_dep,
    libnusmvcore_dep,
    libnusmvgrammar_dep,
    libnusmvrbc_dep,
    shell_dep,
  ],
  install: true,
  install_tag: 'bin',
)
meson.override_find_program('nusmv', nusmv_exe)

if with_ltl2smv
  ltl2smv_exe = executable(
    'ltl2smv',
    'code/nusmv/ltl2smvMain.c',
    include_directories: code_inc,
    dependencies: [
      libcudd_dep,
      libnusmvcore_dep,
      libnusmvgrammar_dep,
      libnusmvrbc_dep,
      shell_dep,
    ],
    install: true,
    install_tag: 'bin',
  )
  meson.override_find_program('ltl2smv', ltl2smv_exe)
endif


### Tests ###################################################################

test(
  'short',
  nusmv_exe,
  args: files('examples/smv-dist/short.smv'),
)

if fs.exists('examples/bmc')
  # examples/bmc is not distributed
  test(
    'barrel5',
    nusmv_exe,
    args: [
      '-bmc',
      files('examples/bmc/barrel5.smv'),
    ],
  )
endif

if fs.exists('test/unittest')
  # test/unittest is not distributed
  subdir('test/unittest')
  test(
    'problem_db',
    run_unit_tests,
  )
endif

# run_cppcheck.sh is not distributed
run_cppcheck = find_program(
  'run_cppcheck.sh',
  dirs: meson.project_source_root()/'cppcheck',
  required: false,
  disabler: true,
)
run_target(
  'cppcheck',
  command: [
    run_cppcheck,
    '-c', cppcheck.full_path(),
    '-s', meson.project_source_root(),
    '-o', 'cppcheck',
    '-d',
    '-n',
  ],
)


### Documentation ###########################################################

subdir('doc')

alias_target(
  'doc',
  prog_man_html,
  prog_man_txt,
  tutorial_pdf,
  user_man_pdf,
)


### Install #################################################################

install_subdir(
  'contrib',
  install_dir: datadir,
)
install_subdir(
  'examples',
  install_dir: datadir,
)

pkgconfig = import('pkgconfig')
pkgconfig.generate(
  nusmvcore_lib,
  description: 'NuSMV core library',
  variables: nusmv_var,
)
pkgconfig.generate(
  nusmvgrammar_lib,
  description: 'NuSMV grammar library',
  variables: nusmv_var,
)
pkgconfig.generate(
  nusmvrbc_lib,
  description: 'NuSMV RBC library',
  variables: nusmv_var,
)
pkgconfig.generate(
  nusmvaddonscore_lib,
  description: 'NuSMV addons library',
  variables: nusmv_var,
)
if libeditline_dep.found()
  pkgconfig.generate(
    nusmvshell_lib,
    description: 'NuSMV shell library',
    variables: nusmv_var,
  )
endif


### Dist ####################################################################

meson.add_dist_script(
  meson_util_py, 'fix',
  '--symlinks', [
    'code/scripts/combine_grammar.py',
    'code/scripts/create_help.py',
    'code/scripts/html2text.py',
    'code/scripts/sgmllib3k.py',
    'scripts/meson-util.py',
    'subprojects/cudd-2.4.1.wrap',
    'subprojects/minisat-2.2.0.g37dc6c6.wrap',
    'subprojects/packagefiles/cudd-2.4.1',
    'subprojects/packagefiles/cudd-2.4.1+0001-nusmv.patch',
    'subprojects/packagefiles/cudd-2.4.1.tar.gz',
    'subprojects/packagefiles/minisat-2.2.0.g37dc6c6',
    'subprojects/packagefiles/minisat-2.2.0.g37dc6c6+0001-nusmv.patch',
    'subprojects/packagefiles/minisat-2.2.0.g37dc6c6+0001-proof.patch',
    'subprojects/packagefiles/minisat-2.2.0.g37dc6c6.zip',
  ],
  '--check',
  # '--debug',
)

meson.add_install_script(
  meson_util_py,
  # '--debug',
  'pack', meson.project_name(),
  '--version', meson.project_version(),
  '--system', host_machine.system(),
  install_tag: 'pack',
)
