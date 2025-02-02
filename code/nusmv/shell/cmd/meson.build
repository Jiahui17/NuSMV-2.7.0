# NuSMV/code/nusmv/shell/cmd/meson.build

shell_c += files(
  'cmd.c',
  'cmdCmd.c',
  'cmdFile.c',
  'cmdMisc.c',
)

shell_c += configure_file(
  output: 'cmdHelp.c',
  input: 'cmdHelp.c.in',
  configuration: {
    'command_list': run_command(
      python, create_help_py,
      '-c',
      '-p', meson.project_source_root()/'code/nusmv/shell',
      check: true,
    ).stdout(),
    'command_number': run_command(
      python, create_help_py,
      '-n',
      '-p', meson.project_source_root()/'code/nusmv/shell',
      check: true,
    ).stdout().strip(),
  },
)
