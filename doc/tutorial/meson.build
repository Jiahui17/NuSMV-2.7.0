# NuSMV/doc/tutorial/meson.build

tutorial_cfg = configuration_data({
  'IF_USE_PACKAGE_HYPERREF': '\\iftrue',
  'LATEX_BIBINPUTS'        : meson.current_source_dir(),
  'LATEX_JOBNAME'          : 'tutorial',
  'LATEX_OUTPUT'           : meson.current_build_dir(),
  'LATEX_TEXINPUTS'        : '\',\''.join([
    meson.current_source_dir(),
    meson.current_build_dir (),
  ]),
  'NUSMV_MAJ_VER'          : meson.project_version().split('.')[0],
  'NUSMV_MIN_VER'          : meson.project_version().split('.')[1],
})

tutorial_latexmkrc = configure_file(
  output: 'latexmkrc',
  input: 'latexmkrc.in',
  configuration: tutorial_cfg,
)

tutorial_files = [
  'main.tex',
  configure_file(
    output: 'nusmvtut.sty',
    input: 'nusmvtut.sty.in',
    configuration: tutorial_cfg,
  ),
]

tutorial_pdf = custom_target(
  'pdf',
  output: tutorial_cfg.get('LATEX_JOBNAME') + '.pdf',
  input: tutorial_files[0],
  depend_files: tutorial_files,
  depfile: '@PLAINNAME@.dep',
  command: [
    latexmk,
    '-quiet',
    '-r', tutorial_latexmkrc,
    '-M', '-MF', '@DEPFILE@',
    '@BASENAME@',
  ],
  build_by_default: build_doc_ft.enabled(),
  install: true,
  install_dir: datadir / 'doc/tutorial',
  install_tag: 'doc',
)
