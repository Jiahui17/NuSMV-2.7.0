# NuSMV/code/nusmv/core/parser/meson.build

subdir('ord')
subdir('idlist')
subdir('psl')

core_c += files(
  'parserUtil.c',
  'parser.c',
  'parserPkg.c',
)

input_l = custom_target(
  output: 'input.l',
  input: [
    'input.l.1.25',
    'input.l.1.50',
    'input.l.2.25',
    'input.l.2.50',
    'input.l.2.51',
    'input.l.2.75',
    'input.l.3.50',
  ],
  command: [
    python, combine_grammar_py,
    '--output', '@OUTPUT@',
    '@INPUT@',
  ],
)

grammar_c += custom_target(
  output: 'input.c',
  input: input_l,
  command: [
    flex,
    '-P', 'nusmv_yy',
    '-o', '@OUTPUT@',
    '@INPUT@',
  ],
)

grammar_y = custom_target(
  output: 'grammar.y',
  input: [
    'grammar.y.1.25',
    'grammar.y.1.50',
    'grammar.y.2.50',
    'grammar.y.2.51',
    'grammar.y.3.50',
  ],
  command: [
    python, combine_grammar_py,
    '--output', '@OUTPUT@',
    '--start', 'begin',
    '@INPUT@',
  ],
)

grammar_ch = custom_target(
  output: [
    'grammar.c',
    'grammar.h',
  ],
  input: grammar_y,
  command: [
    bison,
    '-d',
    '-p', 'nusmv_yy',
    '-o', '@OUTPUT0@',
    '@INPUT@',
  ],
  install: true,
  install_dir: [
    false,
    includedir / 'core/parser',
  ],
)
grammar_c           += grammar_ch[0]
grammar_generated_h += grammar_ch[1]
