# NuSMV/code/nusmv/core/parser/psl/meson.build

core_c += files(
  'pslExpr.c',
  'pslNode.c',
  'pslConv.c',
)

core_c += custom_target(
  output: 'psl_input.c',
  input: 'psl_input.l',
  command: [
    flex,
    '-P', 'psl_yy',
    '-o', '@OUTPUT@',
    '@INPUT@',
  ],
)

psl_grammar_ch = custom_target(
  output: [
    'psl_grammar.c',
    'psl_grammar.h',
  ],
  input: 'psl_grammar.y',
  command: [
    bison,
    '-d',
    '-p', 'psl_yy',
    '-o', '@OUTPUT0@',
    '@INPUT@',
  ],
  install: true,
  install_dir: [
    false,
    includedir / 'core/parser/psl',
  ],
)
core_c           += psl_grammar_ch[0]
core_generated_h += psl_grammar_ch[1]
