#!/usr/bin/env python3
# Copyright (c) 2001 Chris Withers
#
# This Software is released under the MIT License:
# http://www.opensource.org/licenses/mit-license.html
# See license.txt for more details.
#
# $Id: html2text.py 355 2008-11-27 14:30:17Z fresh $


import sgmllib3k as sgmllib
lower = lambda s, *args: s.lower(*args)
replace = lambda s, *args: s.replace(*args)
split = lambda s, *args: s.split(*args)
join = lambda l, s: s.join(l)



class HTML2Text(sgmllib.SGMLParser):

    from html.entities import entitydefs  # replace entitydefs from sgmllib

    def __init__(self, ignore_tags=(), indent_width=4):
        sgmllib.SGMLParser.__init__(self)
        self.result = ""
        self.indent = 0
        self.ol_number = 0
        self.inde_width = indent_width
        self.lines = []
        self.line = ''
        self.ignore_tags = ignore_tags

    def add_text(self, text):
        self.line += text

    def add_break(self):
        # convert text into words
        words = split(replace(self.line, '\n', ' '))
        self.lines.append((self.indent, words))
        self.line = ''

    def generate(self):
        # join lines with indents
        indent_width = self.inde_width
        out_paras = []
        self.add_break()
        for indent, line in self.lines:

            i = indent * indent_width
            indent_string = i * ' '

            out_para = ''
            out_line = []
            for word in line:
                out_line.append(word)

            out_para = out_para + indent_string + join(out_line, ' ')
            out_paras.append(out_para)

        self.result = out_paras

    def mod_indent(self, i):
        self.indent = self.indent + i
        if self.indent < 0:
            self.indent = 0

    def handle_data(self, data):
        if data:
            self.add_text(data)

    def unknown_starttag(self, tag, attrs):
        """ Convert HTML to something meaningful in plain text """
        tag = lower(tag)

        if tag not in self.ignore_tags:
            if tag[0] == 'h' or tag in ['br', 'pre', 'p', 'hr']:
                # insert a blank line
                self.add_break()

            elif tag == 'img':
                # newline, text, newline
                src = ''

                for k, v in attrs:
                    if lower(k) == 'src':
                        src = v

                self.add_break()
                self.add_text('Image: ' + src)

            elif tag == 'li':
                self.add_break()
                if self.ol_number:
                    # num - text
                    self.add_text(str(self.ol_number) + ' - ')
                    self.ol_number = self.ol_number + 1
                else:
                    # - text
                    self.add_text('- ')

            elif tag in ['dd', 'dt']:
                self.add_break()
                # increase indent
                #self.mod_indent(+1)

            elif tag in ['ul', 'dl', 'ol']:
                # blank line
                # increase indent
                self.mod_indent(+1)
                if tag == 'ol':
                    self.ol_number = 1

    def unknown_endtag(self, tag):
        """ Convert HTML to something meaningful in plain text """
        tag = lower(tag)

        if tag not in self.ignore_tags:
            if tag[0] == 'h' or tag in ['pre']:
                # newline, text, newline
                self.add_break()

            elif tag == 'li':
                self.add_break()

            elif tag in ['dd', 'dt']:
                self.add_break()
                # descrease indent
                self.mod_indent(-1)

            elif tag in ['ul', 'dl', 'ol']:
                # blank line
                self.add_break()
                # decrease indent
                self.mod_indent(-1)
                self.ol_number = 0
