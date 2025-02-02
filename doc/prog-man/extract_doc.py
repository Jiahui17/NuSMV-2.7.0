"""
Extracts the NuSMV commands help from the HTML directory
and converts it into text documentation

Authors: Fahad Mehmood
"""

import sys
import re
import html
import os
import textwrap
import argparse
from typing import List, Tuple


def get_file_content(file_path: str) -> str:
    """
    Open a file and return its content.

    Args:
        file_path (str): The path to the file.

    Returns:
        file_content (str): The content of the file.
    """
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            file_content= file.read()
    except FileNotFoundError:
        print(f"Error: The file {file_path} does not exist.")
        sys.exit(1)
    except IOError as e:
        print(f"Error: An IOError occurred while reading the file {file_path}. {e}")
        sys.exit(1)

    return file_content

def set_file_content(file_content: str, file_path: str) -> None:
    """
    Open a file and write the content to it.

    Args:
        file_content (str): The content to write to the file.
        file_path (str): The path to the file.

    Returns:
        None
    """
    try:
        with open(file_path, 'w', encoding='utf-8') as file:
            file.write(file_content)
    except IOError as e:
        print(f"Error: An IOError occurred while writing to the file {file_path}. {e}")
        sys.exit(1)

def get_html_file_list(directory: str, search_text: str) -> List[str]:
    """
    Search all the HTML files in a directory which contain the 'search_text' in their file names.

    Args:
        directory (str): The path to the directory to search.
        search_text (str): The text to search for in the file names.

    Returns:
        cmd_html_files (List[str]): A list of HTML file names that contain the search_text.
    """
    cmd_html_files: List[str] = []

    for filename in os.listdir(directory): # Iterate over all files in the given directory
        if filename.endswith(".html") and search_text in filename: # Check if the file is an HTML file and contains search_text in its name
            cmd_html_files.append(filename)

    return cmd_html_files

def remove_html_tags(html_content: str) -> str:
    """
    Removes all the HTML tags from a string.

    Args:
        html_content (str): The string containing HTML content.

    Returns:
        str: The string with all HTML tags removed.
    """
    html_content = html.unescape(html_content) # interprete html entities
    clean = re.compile(r'<.*?>') # Remove all HTML tags using a regular expression
    return re.sub(clean, '', html_content)

def extract_raw_html(html_content: str) -> List[str]:
    """
    Search for the 'memdoc' and 'cmd' tags and return an array of strings containing cmd help in HTML format.

    Args:
        html_content (str): The HTML content to search within.

    Returns:
        cmd_data (List[str]): An array of strings containing the cmd help in HTML format.
    """
    pattern = re.compile(r'<div class="memdoc">(.*?)</div>', re.DOTALL) # Use a regular expression to find the text within the <div class="memdoc"> tags
    matches = pattern.findall(html_content)
    cmd_data: List[str] = []

    for match in matches: # Check each <div class="memdoc"> for <dl class="cmd">
        if '<dl class="cmd">' in match:
            trimmed_content = re.split(r'<dl class="section see">', match)[0]
            cmd_content = trimmed_content.strip()
            cmd_data.append(remove_html_tags(cmd_content))

    return cmd_data

def reformat_text(text_content: str) -> Tuple[str, str]:
    """
    Reformat the clean text string from HTML file, extracts the 'command_name' and convert it into readible text help 'command_help' by adding indentation and linefeeds.

    Args:
        text_content (str): The clean text content from HTML to reformat.

    Returns:
        command_help,command_name (Tuple[str, str]): A tuple containing the reformatted command help (str) and the command name (str).
    """
    lines = text_content.splitlines()
    reformatted_text = []
    long_spaces = False
    command_found = False

    for line in lines:
        if line.startswith("Command:"): # Checks if the line starts with the "Command:" to extract 'command_name'
            line = line.replace("Command:", "")
            command_name = line.split(":")[0]
            title = line.replace(":", " -")

            command_found = True
            wrapped_line = textwrap.fill(title, width = 80, initial_indent = " "*2, subsequent_indent =" "*3)
            reformatted_text.append(wrapped_line)
            reformatted_text.append("\n")
            reformatted_text.append(" "*5 + "_"*65 + "\n\n") # Add underline after the command text
            continue

        if command_found:
            line = line.strip()
            if line != "": # check if it is not an empty line
                if line.startswith("Command arguments:"): # Checks if the line starts with the "Command arguments:" to extract command arguments
                    line = line.replace("Command arguments:", "").strip()
                    wrapped_line = textwrap.fill(f"{command_name} {line}", width = 80, initial_indent = " "*3, subsequent_indent=" "*3)
                    reformatted_text.append(wrapped_line)
                    reformatted_text.append("\n\n")
                elif line.startswith("Command options:"): # Checks if the line starts with the "Command options:" to extract command options
                    long_spaces = True
                    reformatted_text.append("\n" + "   Command options:")
                    reformatted_text.append("\n")
                elif line.startswith("-"):
                    reformatted_text.append("\n" + " "*3 + line)
                    reformatted_text.append("\n")
                else:
                    if long_spaces:
                        wrapped_line = textwrap.fill(line, width = 80, initial_indent=" "*10, subsequent_indent =" "*10)
                    else:
                        wrapped_line = textwrap.fill(line, width=80, initial_indent=" "*3, subsequent_indent =" "*3)
                    reformatted_text.append(wrapped_line)
                    reformatted_text.append("\n")

    reformatted_text.append(" "*5 + "_"*65 + "\n") # Add underline at the end
    command_help = "".join(reformatted_text)
    return command_help,command_name

def extract_command_help(html_file_path: str, text_file_dir: str) -> None:
    """
    Parse a single HTML file and extract the help of all the available commands and save in seperate .txt files.

    Args:
        html_file_path (str): The path to the HTML file.
        text_file_dir (str): The directory to save the extracted command help text files.

    Returns:
        None
    """
    raw_html_content = get_file_content(html_file_path)
    cmd_array: List[str] = extract_raw_html(raw_html_content)

    for cmd in cmd_array:
        cmd_text_help, cmd_name = reformat_text(cmd)
        set_file_content(cmd_text_help,f"{text_file_dir}/{cmd_name}Cmd.txt")

def get_opts() -> argparse.Namespace:
    """
    Defines the expected input arguments for command execution.

    Returns:
        opts (Namespace): An argparse.Namespace object containing the parsed arguments.
    """
    p = argparse.ArgumentParser(description='Process NuSMV HTML files and extract help into text files.')
    p.add_argument("-s", "--src_html_dir", help='Source directory containing HTML files', required=True)
    p.add_argument("-d", "--dst_txt_dir", help='Destination directory to save text files', required=True)
    opts = p.parse_args()

    return opts

def main():

    opts = get_opts()

    if opts.src_html_dir:
        if not os.path.exists(opts.src_html_dir):
            print(f"Error: path {opts.src_html_dir} not found")
            exit(1)

    if opts.dst_txt_dir:
        if not os.path.exists(opts.dst_txt_dir):
            print(f"Creating a new directory ->{opts.dst_txt_dir}<- since it does not exist")
            os.makedirs(opts.dst_txt_dir)

    html_file_dir = opts.src_html_dir
    text_file_dir = opts.dst_txt_dir

    cmd_file_list = get_html_file_list(html_file_dir,"Cmd")

    for cmd_file in cmd_file_list:
        file_path = html_file_dir + "/" + cmd_file
        extract_command_help(file_path, text_file_dir)

if __name__ == "__main__":
    main()
