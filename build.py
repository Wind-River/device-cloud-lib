#!/usr/bin/env python
#
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
#

import argparse
import os
import re
import sys
import yaml

CFG_FILE="build.yml"
IN_FILE="iot_build.h.in"
OUT_FILE="iot_build.h"

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='build system setup script')
    parser.add_argument( '--src', type=str, metavar='FILE', default=".",
        help='source directory' )
    parser.add_argument( '--dest', type=str, metavar='FILE', default=".",
        help='destination directory' )
    args = parser.parse_args()

    cfg_file = os.path.join( args.src, CFG_FILE )
    in_file = os.path.join( args.src, IN_FILE )
    out_file = os.path.join( args.dest, OUT_FILE )
    try:
        cfg_stream = open( cfg_file, "r" )
        cfg_items = yaml.load(cfg_stream)
        cfg_stream.close()

        in_stream = open( in_file, "r" )
        lines = in_stream.readlines()
        in_stream.close()

        # method that returns the value from the config file or blank
        def find_str(match_obj):
            match_str = match_obj.group(1)
            cur_level = cfg_items
            for level in match_str.split("."):
                if level in cur_level:
                    cur_level = cur_level[level]
                else:
                    cur_level = None
                    break
            if cur_level is not None:
                if isinstance(cur_level, list):
                    return ", ".join(cur_level)
                else:
                    return str(cur_level)
            else:
                return ""

        out_stream = open( out_file, "w" )
        regex = re.compile( '@([a-zA-Z_][a-zA-Z0-9_.]*)@' )
        for line in lines:
            out_stream.write( regex.sub( find_str, line ) )
        out_stream.close()


    except yaml.YAMLError as exc:
        print(exc)
        sys.exit(1)
sys.exit(0)
