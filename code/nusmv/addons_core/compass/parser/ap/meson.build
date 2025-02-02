# NuSMV/code/nusmv/addons_core/compass/parser/ap/meson.build

addon_c += files(
  'ParserAp.c',
)

addon_c += custom_target(
  output: 'ap_input.c',
  input: 'ap_input.l',
  command: [
    flex,
    '-o', '@OUTPUT@',
    '@INPUT@',
  ],
)

ap_grammar_ch = custom_target(
  output: [
    'ap_grammar.c',
    'ap_grammar.h',
  ],
  input: 'ap_grammar.y',
  command: [
    bison,
    '-d',
    '-p', 'parser_ap_',
    '-o', '@OUTPUT0@',
    '@INPUT@',
  ],
  install_dir: [
    false,
    fs.relative_to('.', meson.project_source_root()),
  ],
)
addon_c           += ap_grammar_ch[0]
addon_generated_h += ap_grammar_ch[1]
