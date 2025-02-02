#   NuSMV/code/nusmv/addons_core/compass/parser/prob/meson.build

addon_c += files(
  'ParserProb.c',
)

addon_c += custom_target(
  output: 'prob_input.c',
  input: 'prob_input.l',
  command: [
    flex,
    '-o', '@OUTPUT@',
    '@INPUT@',
  ],
)

prob_grammar_ch = custom_target(
  output: [
    'prob_grammar.c',
    'prob_grammar.h',
  ],
  input: 'prob_grammar.y',
  command: [
    bison,
    '-d',
    '-p', 'parser_prob_',
    '-o', '@OUTPUT0@',
    '@INPUT@',
  ],
  install_dir: [
    false,
    fs.relative_to('.', meson.project_source_root()),
  ],
)
addon_c           += prob_grammar_ch[0]
addon_generated_h += prob_grammar_ch[1]
