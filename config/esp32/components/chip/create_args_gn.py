#
#    Copyright (c) 2021-2026 Project CHIP Authors
#    Copyright (c) 2018 Nest Labs, Inc.
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
#    Description:
#      Component makefile for building CHIP within the ESP32 ESP-IDF environment.
#

import argparse
import json
import os
import shlex

# Parse the build's compile_commands.json to generate
# final args file for CHIP build.

argparser = argparse.ArgumentParser()

argparser.add_argument("build_dir")
argparser.add_argument("idf_path")
argparser.add_argument("c_file")
argparser.add_argument("cpp_file")
argparser.add_argument("input")
argparser.add_argument("output")
argparser.add_argument("--filter-out")

args = argparser.parse_args()

compile_commands_path = os.path.join(args.build_dir, "compile_commands.json")

# From esp-idf v6.0 and onwards, compile flags are stored in files, so we need to explicitly expand them
# eg: @\"/Users/esp/connectedhomeip/examples/lighting-app/esp32/build/toolchain/asmflags\"
#     @\"/Users/esp/connectedhomeip/examples/lighting-app/esp32/build/toolchain/cflags\"
# this function reads that file and returns the flags


def expand_response_file(flag):
    """Expand response file references (flags starting with @) into their contents."""
    # Handle both @path and @"path" formats
    if flag.startswith('@"') and flag.endswith('"'):
        response_file_path = flag[2:-1]  # Remove @" and trailing "
    elif flag.startswith('@'):
        response_file_path = flag[1:]  # Remove @
    else:
        return [flag]

    try:
        with open(response_file_path, 'r') as f:
            content = f.read()
            return shlex.split(content)
    except (IOError, OSError) as e:
        raise Exception(f"Failed to read response file '{response_file_path}': {e}")


with open(compile_commands_path) as compile_commands_json:
    compile_commands = json.load(compile_commands_json)

    def get_compile_flags(src_file):
        compile_command = [
            res["command"] for res in compile_commands if os.path.normpath(res["file"]) == os.path.normpath(src_file)
        ]

        if len(compile_command) != 1:
            raise Exception(f"Failed to resolve compile flags for {src_file}")

        compile_command = compile_command[0]
        compile_parts = shlex.split(compile_command)
        # Trim compiler, input and output
        compile_flags = compile_parts[1:-4]

        # Expand any response file references
        expanded_flags = []
        for flag in compile_flags:
            expanded_flags.extend(expand_response_file(flag))

        replace = "-I%s" % args.idf_path
        replace_with = "-isystem%s" % args.idf_path

        # Escape any embedded double quotes for GN string syntax, then wrap in quotes
        def quote_for_gn(flag):
            # Escape backslashes first, then escape double quotes
            escaped = flag.replace('\\', '\\\\').replace('"', '\\"')
            return f'"{escaped}"'

        expanded_flags = [quote_for_gn(f).replace(replace, replace_with) for f in expanded_flags]

        if args.filter_out:
            filter_out = [f'"{f}"' for f in args.filter_out.split(';')]
            expanded_flags = [c for c in expanded_flags if c not in filter_out]

        return expanded_flags

    c_flags = get_compile_flags(args.c_file)
    cpp_flags = get_compile_flags(args.cpp_file)

    with open(args.input) as args_input, open(args.output, "w") as args_output:
        args_output.write(args_input.read())

        args_output.write("target_cflags_c = [%s]" % ', '.join(c_flags))
        args_output.write("\n")
        args_output.write("target_cflags_cc = [%s]" % ', '.join(cpp_flags))
