# NuSMV/code/meson.build

### Configuration ###########################################################

cfg = configuration_data()

## __attribute__
foreach key, attr : {
    'NUSMV_FUNCATTR_NORETURN'     : 'noreturn',
    'NUSMV_GCC_WARN_UNUSED_RESULT': 'warn_unused_result',
  }
  cfg.set(key, cc.has_function_attribute(attr) ? f'__attribute__ ((@attr@))' : '')
endforeach

cfg.set10('NUSMV_HAVE_MALLOC', cc.has_function_attribute('malloc'))

## Types
cfg.set10('NUSMV_HAVE__BOOL', cc.has_type('_Bool'))

## sizeof
cfg.set('NUSMV_SIZEOF_INT'      , cc.sizeof('int'      ))
cfg.set('NUSMV_SIZEOF_LONG'     , cc.sizeof('long'     ))
cfg.set('NUSMV_SIZEOF_LONG_LONG', cc.sizeof('long long'))
cfg.set('NUSMV_SIZEOF_VOID_P'   , cc.sizeof('void *'   ))

## Headers
foreach h : [
    'dirent.h',
    'dlfcn.h',
    'errno.h',
    'float.h',
    'fnmatch.h',
    'inttypes.h',
    'limits.h',
    'malloc.h',
    'memory.h',
    'ndir.h',
    'regex.h',
    'signal.h',
    'stdbool.h',
    'stddef.h',
    'stdint.h',
    'stdlib.h',
    'strings.h',
    'string.h',
    'sys/dir.h',
    'sys/ioctl.h',
    'sys/malloc.h',
    'sys/ndir.h',
    'sys/param.h',
    'sys/resource.h',
    'sys/signal.h',
    'sys/stat.h',
    'sys/time.h',
    'sys/types.h',
    'unistd.h',
  ]
  cfg.set10(
    'NUSMV_HAVE_' + h.underscorify().to_upper(),
    cc.has_header(h),
  )
endforeach

## Functions
foreach f : [
    'floor',
    'getenv',
    'getpid',
    'isatty',
    'memmove',
    'memset',
    'mkstemp',
    'mktemp',
    'popen',
    'pow',
    'random',
    'realloc',
    'setvbuf',
    'srandom',
    'strcasecmp',
    'strchr',
    'strrchr',
    'strstr',
    'strtod',
    'strtol',
    'strtoull',
    'system',
    'tmpnam',
    'vprintf',
  ]
  cfg.set10(
    'NUSMV_HAVE_' + f.to_upper(),
    cc.has_function(f),
  )
endforeach

## Dependencies
cfg.set10('NUSMV_HAVE_BIGNUMBERS'          , libgmp_dep     .found())
cfg.set10('NUSMV_HAVE_BMC_PROFILER_LIBRARY', with_profiler          )
cfg.set10('NUSMV_HAVE_CUDD_24'             , libcudd_dep    .found())
cfg.set10('NUSMV_HAVE_INTERACTIVE_SHELL'   , libeditline_dep.found())
cfg.set10('NUSMV_HAVE_LIBEDIT'             , with_libedit           )
cfg.set10('NUSMV_HAVE_LIBGMP'              , libgmp_dep     .found())
cfg.set10('NUSMV_HAVE_LIBM'                , libm_dep       .found())
cfg.set10('NUSMV_HAVE_LIBXML2'             , libxml2_dep    .found())
cfg.set10('NUSMV_HAVE_SAT_SOLVER'          , libminisat_dep .found())
cfg.set10('NUSMV_HAVE_SOLVER_MINISAT'      , libminisat_dep .found())
cfg.set10('NUSMV_HAVE_WATCHDOG_LIBRARY'    , with_watchdog          )
cfg.set10('NUSMV_HAVE_WINEDITLINE'         , with_wineditline       )

## Others
addons = []
if with_compass
  addons += 'compass'
endif

now = run_command(
  python, '-c', 'import time; print(time.asctime())',
  capture: true,
  check: true,
).stdout().strip()
year = now.split()[-1]
if get_option('buildtype') != 'release'
  # avoid re-compilation when developing
  now = '(unknown)'
endif

cfg.set_quoted('NUSMV_LIBRARY_BUGREPORT' , 'Please report bugs to <nusmv-users\x40fbk.eu>')
cfg.set_quoted('NUSMV_LIBRARY_BUILD_DATE', now)
cfg.set_quoted('NUSMV_LIBRARY_EMAIL'     , 'nusmv-users\x40fbk.eu')
cfg.set_quoted('NUSMV_LIBRARY_NAME'      , meson.project_name())
cfg.set_quoted('NUSMV_LIBRARY_VERSION'   , meson.project_version())
cfg.set_quoted('NUSMV_LIBRARY_WEBSITE'   , 'http://nusmv.fbk.eu')

cfg.set       ('NUSMV_PACKAGE_BUGREPORT'   , cfg.get('NUSMV_LIBRARY_BUGREPORT'))
cfg.set_quoted('NUSMV_PACKAGE_BUILD_DATE'  , now)
cfg.set_quoted('NUSMV_PACKAGE_NAME'        , meson.project_name())
cfg.set_quoted('NUSMV_PACKAGE_RELEASE_YEAR', year)
cfg.set_quoted('NUSMV_PACKAGE_STRING'      , meson.project_name()+' '+meson.project_version())
cfg.set_quoted('NUSMV_PACKAGE_TARNAME'     , 'nusmv')
cfg.set_quoted('NUSMV_PACKAGE_URL'         , '')
cfg.set_quoted('NUSMV_PACKAGE_VERSION'     , meson.project_version())
cfg.set_quoted('NUSMV_PROG_CPP'            , 'cpp ' + (cc_gcc ? '-E -x c' : '-xc'))

cfg.set_quoted('NUSMV_EXEEXT'              , windows ? 'exe' : '')
cfg.set_quoted('NUSMV_LINKED_CORE_ADDONS'  , ' '.join(addons))
cfg.set_quoted('NUSMV_PROJECT_RELEASE_YEAR', year)

cfg.set10('NUSMV_HAVE_COMPASS'              , with_compass)
cfg.set10('NUSMV_HAVE_CPP'                  , true)
cfg.set10('NUSMV_HAVE_MINISAT_INTERPOLATION', false)
cfg.set10('NUSMV_NO_POINTER_ORDERING'       , true)
cfg.set10('NUSMV_STDC_HEADERS'              , true)

foreach key : [
    'NUSMV_LIBRARY_BUGREPORT',
    'NUSMV_LIBRARY_BUILD_DATE',
    'NUSMV_LIBRARY_EMAIL',
    'NUSMV_LIBRARY_NAME',
    'NUSMV_LIBRARY_VERSION',
    'NUSMV_LIBRARY_WEBSITE',
    'NUSMV_PACKAGE_BUGREPORT',
    'NUSMV_PACKAGE_BUILD_DATE',
    'NUSMV_PACKAGE_NAME',
    'NUSMV_PACKAGE_RELEASE_YEAR',
    'NUSMV_PACKAGE_STRING',
    'NUSMV_PACKAGE_TARNAME',
    'NUSMV_PACKAGE_URL',
    'NUSMV_PACKAGE_VERSION',
    'NUSMV_PROG_CPP',
    'NUSMV_EXEEXT',
    'NUSMV_LINKED_CORE_ADDONS',
    'NUSMV_PROJECT_RELEASE_YEAR',
    'NUSMV_HAVE_COMPASS',
    'NUSMV_HAVE_LIBEDIT',
    'NUSMV_HAVE_MINISAT_INTERPOLATION',
    'NUSMV_HAVE_WINEDITLINE',
    'NUSMV_NO_POINTER_ORDERING',
    'NUSMV_STDC_HEADERS',
  ]
  summary(
    key, cfg.get(key),
    section: 'Configuration',
    bool_yn: true,
  )
endforeach


core_generated_h += configure_file(
  configuration: cfg,
  output: 'nusmv-config.h',
  install: true,
  install_dir: get_option('includedir'),
)


### Arguments ###############################################################

add_project_arguments('-DHAVE_CONFIG_H', language: ['c', 'cpp'])

if get_option('data-dir') != ''
  add_project_arguments(
    '-DDATADIR="@0@"'.format(get_option('data-dir')),
    language: 'c',
  )
endif

if cc_gcc
  add_project_arguments(
    '-Wdeclaration-after-statement',
    '-Werror=declaration-after-statement',
    language: 'c',
  )
  if prefer_static
    add_project_arguments(
      '-DLIBXML_STATIC',
      language: ['c', 'cpp'],
    )
    add_project_link_arguments(
      '-static',
      '-static-libgcc',
      '-static-libstdc++',
      language: ['c', 'cpp'],
    )
  elif windows and build_release
    warning('add setup option `--prefer-static` for DLL independence')
  endif
endif

if cc_msvc
  add_project_arguments(
    '-DYY_NO_UNISTD_H',
    language: 'c',
  )
endif


### Compilation #############################################################

subdir('nusmv')
