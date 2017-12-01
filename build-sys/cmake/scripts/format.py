#!/usr/bin/env python

# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software  distributed
# under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
# OR CONDITIONS OF ANY KIND, either express or implied.

import argparse
import os
import subprocess
import sys


def get_git_head( git ):
    rev_parse = subprocess.Popen([git, 'rev-parse', '--verify', 'HEAD'],
         stdout=subprocess.PIPE)
    if rev_parse.returncode:
         return '4b825dc642cb6eb9a060e54bf8d69288fbee4904'
    return "HEAD"

def get_git_root( git ):
    rev_parse = subprocess.Popen([git, 'rev-parse', '--show-toplevel'],
        stdout=subprocess.PIPE)
    return rev_parse.stdout.read().strip()

def get_edited_files( git, in_place ):
    head = get_git_head( git )
    git_args = [git, 'diff-index']
    if not in_place:
        git_args.append('--cached')
    git_args.extend(['--diff-filter=ACMR', '--name-only', head])
    diff_index = subprocess.Popen( git_args, stdout=subprocess.PIPE)
    diff_index_ret = diff_index.stdout.read().strip()
    diff_index_ret = diff_index_ret.decode()
    return diff_index_ret.split('\n')

def is_formattable( file_name ):
    file_ext = os.path.splitext(file_name)[1]
    for ext in ['.h', '.cpp', '.hpp', '.c', '.cc', '.hh', '.cxx', '.hxx']:
         if ext == file_ext:
              return True

def format_file( clang_format, file_name, git_root ):
    subprocess.Popen([clang_format, '-i', os.path.join( git_root, file_name )])
    return


def requires_format( git, clang_format, file_name ):
    git_show = subprocess.Popen([git, "show", ":" + file_name],
        stdout=subprocess.PIPE)
    clang_format = subprocess.Popen([clang_format], stdin=git_show.stdout,
            stdout=subprocess.PIPE)
    formatted_content = clang_format.stdout.read()

    file_content = subprocess.Popen([git,"show", ":" + file_name],
        stdout=subprocess.PIPE).stdout.read()

    if formatted_content == file_content:
        return False
    return True

def which(file):
    for path in os.environ["PATH"].split(os.pathsep):
        if os.path.exists(os.path.join(path,file)):
            return os.path.join(path,file)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Code format script")
    parser.add_argument( "--in-place", "-i", action='store_true', help="make format changes in-place" )
    parser.add_argument( "--git", metavar="path", type=str, help="git executable to use" )
    parser.add_argument( "--clang-format", metavar="path", type=str, help="clang-format executable to use" )
    args = parser.parse_args()

    return_code = 0
    git = args.git
    clang_format = args.clang_format
    if not git:
        git = which('git')
        if not git:
            print( "Error: git not found, use --git option" )
            parser.print_help()
            return_code = 1

    if not clang_format:
        clang_format = which('clang-format')
        if not clang_format:
            print( "Error: clang-format not found, use --clang-format option" )
            parser.print_help()
            return_code = 1

    if 0 == return_code:
        edited_files = get_edited_files(git, args.in_place)
        if args.in_place:
            git_root = get_git_root( git )
            for file_name in edited_files:
                if is_formattable( file_name ):
                    format_file( clang_format, file_name, git_root )
        else:
            for file_name in edited_files:
                if not is_formattable( file_name ):
                    continue
                if requires_format( git, clang_format, file_name ):
                    print( "'%s' must be formatted" % file_name )
                    return_code = 1

            if 1 == return_code:
                subprocess.Popen([git, "reset", "HEAD", "--","."])
    sys.exit( return_code )
