# NuSMV/code/nusmv/core/parser/idlist/meson.build

core_c += files(
  'ParserIdList.c',
)

core_c += custom_target(
  output: 'idlist_input.c',
  input: 'idlist_input.l',
  command: [
    flex,
    '-o', '@OUTPUT@',
    '@INPUT@',
  ],
)

idlist_grammar_ch = custom_target(
  output: [
    'idlist_grammar.c',
    'idlist_grammar.h',
  ],
  input: 'idlist_grammar.y',
  command: [
    bison,
    '-d',
    '-p', 'parser_idlist_',
    '-o', '@OUTPUT0@',
    '@INPUT@',
  ],
  install: true,
  install_dir: [
    false,
    includedir / 'core/parser/idlist',
  ],
)
core_c           += idlist_grammar_ch[0]
core_generated_h += idlist_grammar_ch[1]
