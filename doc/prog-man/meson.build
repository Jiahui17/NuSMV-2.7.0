# NuSMV/doc/prog-man/meson.build

prog_man_cfg = configuration_data({
  'DOXY_HTML_OUTPUT'    : meson.current_build_dir() / 'html',
  'DOXY_INPUT'          : ' '.join([
    meson.project_source_root() / 'code/nusmv',
    meson.project_source_root() / 'doc/prog-man',
  ]),
  'DOXY_STRIP_FROM_PATH': meson.project_source_root() / 'code',
  'VERSION'             : meson.version(),
})

doxy_files = files(
  'main.doxy',
  'packages/core.doxy',
  'packages/node.doxy',
  'packages/utils.doxy',
)

doxyfile = configure_file(
  input: 'Doxyfile.in',
  output: 'Doxyfile',
  configuration: prog_man_cfg,
)

extract_doc_py = files('extract_doc.py')[0]

subdir('html')
subdir('txt')
