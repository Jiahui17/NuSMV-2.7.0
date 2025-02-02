# subprojects/packagefiles/cudd-2.4.1/meson.build

project(
  'cudd', 'c',
  version: '2.4.1',
)

linux  = host_machine.system() == 'linux'
darwin = host_machine.system() == 'darwin'
unix   = linux or darwin

cc = meson.get_compiler('c')
cc_unistd_h        = cc.check_header('unistd.h').to_string('1', '0')
cc_sizeof_int      = cc.sizeof('int')           .to_string()
cc_sizeof_long_int = cc.sizeof('long int')      .to_string()
cc_sizeof_void_ptr = cc.sizeof('void *')        .to_string()

add_project_arguments(
  '-DBSD',
  '-DHAVE_IEEE_754',
  f'-DNUSMV_HAVE_UNISTD_H=@cc_unistd_h@',
  f'-DNUSMV_SIZEOF_INT=@cc_sizeof_int@',
  f'-DNUSMV_SIZEOF_LONG=@cc_sizeof_long_int@',
  f'-DNUSMV_SIZEOF_VOID_P=@cc_sizeof_void_ptr@',
  language: 'c',
)

if linux
  add_project_arguments('-DUNIX', language: 'c')
elif darwin
  add_project_arguments('-D__MAC_OS_X__', language: 'c')
endif

libm_dep = unix\
    ? cc.find_library('m')\
    : dependency('', required: false)

subdir('epd' )
subdir('mtr' )
subdir('st'  )
subdir('util')
subdir('cudd')

install_headers(
  cudd_h, mtr_h, st_h, util_h,
  subdir: 'cudd',
)

cudd_lib = library(
  'cudd', [
    cudd_c,
    epd_c,
    mtr_c,
    st_c,
    util_c,
  ],
  include_directories: [
    cudd_inc,
    epd_inc,
    mtr_inc,
    st_inc,
    util_inc,
  ],
  dependencies: [
    libm_dep,
  ],
  install: true,
)
libcudd_dep = declare_dependency(
  include_directories: ['.', st_inc],
  link_with: cudd_lib,
)

meson.override_dependency('cudd-2.4.1', libcudd_dep)

pkgconfig = import('pkgconfig')
pkgconfig.generate(
  cudd_lib,
  description: 'Decision diagrams package',
  url: 'https://github.com/ivmai/cudd',
)
