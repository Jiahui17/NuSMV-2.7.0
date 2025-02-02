# NuSMV/code/nusmv/addons_core/compass/meson.build

subdir('parser/ap')
subdir('parser/prob')

addon_c += files(
  'compass.c',
  'compassCmd.c',
  'compassPkg.c',
  'compile/ProbAssign.c',
  'compile/compile.c',
  'sigref/sigrefWrite.c',
)

# find code/nusmv/addons_core/compass -name '*.h' -printf "  '%P',\n"
install_headers(
  'compass.h',
  'compassCmd.h',
  'compassInt.h',
  'compile/ProbAssign.h',
  'parser/ap/ParserAp.h',
  'parser/ap/ParserAp_private.h',
  'parser/ap/apInt.h',
  'parser/prob/ParserProb.h',
  'parser/prob/ParserProb_private.h',
  'parser/prob/probInt.h',
  # https://github.com/mesonbuild/meson/issues/13738
  # install_dir: includedir,
  subdir: fs.relative_to(includedir, get_option('includedir')) / 'addons_core/compass',
  preserve_path: true,
)
