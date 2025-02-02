# NuSMV/code/nusmv/core/parser/ord/meson.build

core_c += files(
  'ParserOrd.c',
)

core_c += custom_target(
  output: 'ord_input.c',
  input: 'ord_input.l',
  command: [
    flex,
    '-o', '@OUTPUT@',
    '@INPUT@',
  ],
)

ord_grammar_ch = custom_target(
  output: [
    'ord_grammar.c',
    'ord_grammar.h',
  ],
  input: 'ord_grammar.y',
  command: [
    bison,
    '-d',
    '-p', 'parser_ord_',
    '-o', '@OUTPUT0@',
    '@INPUT@',
  ],
  install: true,
  install_dir: [
    false,
    includedir / 'core/parser/ord',
  ],
)
core_c           += ord_grammar_ch[0]
core_generated_h += ord_grammar_ch[1]
