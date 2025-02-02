# NuSMV/doc/user-man/meson.build

user_man_cfg = configuration_data({
  'IF_USE_PACKAGE_HYPERREF': '\\iftrue',
  'LATEX_BIBINPUTS'        : meson.current_source_dir(),
  'LATEX_JOBNAME'          : 'nusmv',
  'LATEX_TEXINPUTS'        : '\',\''.join([
    meson.current_source_dir(),
    meson.current_build_dir (),
  ]),
  'LATEX_OUTPUT'           : meson.current_build_dir(),
  'VERSION'                : meson.project_version(),
})

user_man_latexmkrc = configure_file(
  output: 'latexmkrc',
  input: 'latexmkrc.in',
  configuration: user_man_cfg,
)

user_man_files = [
  'main.tex',
  configure_file(
    output: 'nusmv.sty',
    input: 'nusmv.sty.in',
    configuration: user_man_cfg,
  ),
]

user_man_pdf = custom_target(
  'pdf',
  output: user_man_cfg.get('LATEX_JOBNAME') + '.pdf',
  input: user_man_files[0],
  depend_files: user_man_files,
  depfile: '@PLAINNAME@.dep',
  command: [
    latexmk,
    '-quiet',
    '-r', user_man_latexmkrc,
    '-M', '-MF', '@DEPFILE@',
    '@BASENAME@',
  ],
  console: true,
  build_by_default: build_doc_ft.enabled(),
  install: true,
  install_dir: datadir / 'doc/user-man',
  install_tag: 'doc',
)
