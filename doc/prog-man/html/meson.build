# NuSMV/doc/prog-man/html/meson.build

prog_man_html = custom_target(
  'doxygen',
  output: 'index.html',
  input: [doxyfile, doxy_files],
  command: [
    doxygen,
    '-q',
    '@INPUT0@',
  ],
  build_by_default: build_doc_ft.enabled(),
)

meson.add_install_script(
  meson_util_py,
  'copytree',
  meson.current_build_dir(),
  '--to-install-destdir-prefix', datadir / 'doc/html',
  install_tag: 'doc',
)
