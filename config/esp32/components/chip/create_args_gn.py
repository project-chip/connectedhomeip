#
#    Copyright (c) 2021 Project CHIP Authors
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

with open(compile_commands_path) as compile_commands_json:
    compile_commands = json.load(compile_commands_json)

    def get_compile_flags(src_file):
        compile_command = list(map(lambda res: res["command"],
                               filter(lambda cmd: cmd["file"] == src_file, compile_commands)))

        if len(compile_command) != 1:
            raise Exception("Failed to resolve compile flags for %s", src_file)

        compile_command = compile_command[0]
        # Trim compiler, input and output
        compile_flags = compile_command.split()[1:-4]

        replace = "-I%s" % args.idf_path
        replace_with = "-isystem%s" % args.idf_path

        compile_flags = list(map(lambda f: ('"%s"' % f).replace(
            replace, replace_with), compile_flags))

        if args.filter_out:
            filter_out = list(map(lambda f: ('"%s"' % f),
                              args.filter_out.split(';')))
            compile_flags = [c for c in compile_flags if c not in filter_out]

        return compile_flags

    c_flags = get_compile_flags(args.c_file)
    cpp_flags = get_compile_flags(args.cpp_file)

    with open(args.input) as args_input:
        with open(args.output, "w") as args_output:
            args_output.write(args_input.read())

            args_output.write("target_cflags_c = [%s]" % ', '.join(c_flags))
            args_output.write("\n")
            args_output.write("target_cflags_cc = [%s]" % ', '.join(cpp_flags))
