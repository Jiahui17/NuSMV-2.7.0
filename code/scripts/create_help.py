#!/usr/bin/env python3
from __future__ import print_function
import re
from fnmatch import fnmatch
import os
import os.path
import collections
from html2text import HTML2Text
import argparse
import textwrap

"""
This file reads all files in a a given directory (e.g., for NuSMV is
NuSMV/code/nusmv/shell) to find patterns of documentation of an
interactive command. Once found, it returns the string to
be used when a user prompts 'help command' (aka long help).
"""


CommandMatch = collections.namedtuple('CommandMatch',
                                      'found_index, command_name, description')

def convert_html2text(s, ignore_tags=(), indent_width=4):
    ignore_tags = [t.lower() for t in ignore_tags]
    parser = HTML2Text(ignore_tags, indent_width)

    parser.feed(s)
    parser.close()
    parser.generate()
    return parser.result


def find_next_command(text, start_index):
    """
    @returns: CommandMatch, or None if not found
    """

    pattern = (r'\\command{\s*(\w+)\s*}([\*\[\]\(\)\w\s\.\,\"\-\\\']+)'
               r'\\command_args{([^}]*)}(((?!\*\/).)*)') # catch all
                                                      # before '*/' ending
    aux_text = text[start_index:]
    regex = re.compile(r'\/\*!\s*(%s)' % pattern, re.M | re.DOTALL) # flags
                                                     # for multiline regexp
    m = re.search(regex, aux_text)
    if m:
        command = m.group(2)
        o = re.search(r'((?!\\sa).)*', m.group(5), re.M | re.DOTALL)
        p = re.sub(r'\\se', r'Side Effects:', o.group(0))
        st = m.group(4).replace(r'\[','[').replace(r'\]',']') # this happens in xSAP and ocra
        short_description = re.sub(r'^\s*', r'', m.group(3))
        if len(short_description)>1:
            short_description = short_description[0].lower() + short_description[1:]
        new_text = ("Command %s: %s<p>usage: %s %s<p>%s" %
                    (command, short_description, command, st, p))
        paragraph_list = convert_html2text(new_text)

        result_text=''
        for paragraph in paragraph_list:
            list_of_lines = textwrap.wrap(paragraph, width=60, tabsize=4)
            result_text += '\n'.join(list_of_lines)
            result_text += '\n\n'

        result_text= re.sub(r'\n(\s*\n)+', r'\n\n', result_text, flags=re.M)

        return (CommandMatch(m.start() + start_index, command, result_text.strip('\n')),
                m.end() + start_index)
    else:
        return (None, None)


def find_all_commands_in_a_text(text):
    """
    @returns: list of CommandMatch
    """
    command_list = []
    start_index = 0
    end_index = 0
    last_CommandMatch = 1

    while last_CommandMatch is not None:
        last_CommandMatch, end_index = find_next_command(text, start_index)

        if last_CommandMatch is not None:
            command_list.append(last_CommandMatch)
            start_index = end_index + 2

    return command_list


def find_all_commands(search_paths, commands_list):
    """
     @returns: lista comandi
    """

    for path in search_paths:
        for root, subFolders, files in os.walk(path):
            for fn in files:
                if (fnmatch(fn, "*.[hc]")):
                    try:
                        _file = open(os.path.abspath(os.path.join(root, fn)), encoding="latin1")
                        text = _file.read()
                        _file.close()

                    except IOError:
                        #  print void because it print on cmdHelp.c
                        #  file so in case of exception it print a
                        #  wrong message on cmdHelp.c
                        print('')

                    else:
                        commands_list_file = find_all_commands_in_a_text(text)
                        commands_list += commands_list_file

    return commands_list


def print_commands_on_txt_files(path_trunk, commands_list):
    help_dir = os.path.abspath(os.path.join(path_trunk, "nusmv", "help"))

    if not os.path.exists(help_dir):
        os.makedirs(help_dir)
    for com in commands_list:
        file_name = "%sCmd.txt" % com.command_name
        try:
            F = open(os.path.abspath(os.path.join(help_dir, file_name)), 'w')
        except IOError:
            print(f'Error: impossible to open the file {file_name}!!')
        F.write(com.description)
        F.close()


def print_commands_for_csource(commands_list):
    output_list = []
    for i, com in enumerate(commands_list):
        rgx = re.sub(r'\\\"', r'"', com.description)
        rgx1 = re.sub(r'"', r'\"', rgx)
        rgx2 = re.sub(r'\n', r'\\n"\n"', rgx1)
        output_list.append(f'/* {i} */ {{"{com.command_name}",\n"{rgx2}"}}')
    print(',\n'.join(output_list))


def get_opts():
    p = argparse.ArgumentParser()
    p.add_argument('-t', '--txtfiles',
                 help="""create help documentation in .txt format """
                 """in the specified directory.""")
    p.add_argument('-c', '--csource',
                 help='create help documentation for a c file',
                 action='store_true', dest='csource')
    p.add_argument('-p', '--paths',
                 help='Specify search paths, separated by semicolon ";"',
                 default=os.getcwd())
    p.add_argument('-n', '--number-of-commands',
                 help='calculate the number of commmands',
                 action='store_true')

    opts = p.parse_args()
    return opts


def main():
    #1 takes options from command line
    opts = get_opts()

    if (not opts.csource) and (opts.txtfiles is None) and (not opts.number_of_commands):
        print("Please specify at least one of the following options: -c, -p, -n")
        exit(1)

    #2 if the param is -t i check if the path inserted of where
    #  put the txt files is correct else exit
    if opts.txtfiles:
        if not os.path.exists(opts.txtfiles):
            print(f'Error: path {opts.txtfiles} not found')
            exit(1)

    search_paths = [_dir if os.path.isabs(_dir)
                    else os.path.join(os.getcwd(), _dir)
                    for _dir in opts.paths.split(";")]

    not_found = [_dir
                 for _dir in search_paths
                 if not os.path.isdir(_dir)]

    if not_found:
        for _dir in not_found:
            print('Error: path {} not found'.format(_dir))
        exit(1)

    commands_list = find_all_commands(search_paths, [])
    if len(commands_list) == 0:
        print(f'No commands have been found in the specified paths: {",".join(search_paths)}')
        exit(1)

    #4 If param is -t i call the function for write .txt files
    if opts.txtfiles:
        print_commands_on_txt_files(opts.txtfiles, commands_list)
        print(f"Documentation in txt format created in {opts.txtfiles}")

    #5 if param is -c i call function for create .c
    if opts.csource:
        print_commands_for_csource(commands_list)

    if opts.number_of_commands:
        print(len(commands_list))


if __name__ == '__main__':
    main()
