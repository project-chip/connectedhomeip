#!/usr/bin/env -S python3 -B

# Copyright (c) 2021 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import subprocess
import sys


def consumeArgument(args):
    if len(args) == 0:
        return None
    return args.pop(0)


def maybeRunClangTidy(args):
    # If the command is not a clang command, do no try to run clang-tidy on it
    cc = consumeArgument(args)
    if cc is None or not cc.startswith("clang"):
        return 0

    clang_tidy_srcs = []
    clang_tidy_args = []

    # If warnings comes from third_party, ignore it as there is little we can do.
    ignored_list = ['third_party/mbedtls', 'third_party/lwip']

    arg = consumeArgument(args)
    while arg:
        if arg == "-c":
            sourceFile = consumeArgument(args)
            if sourceFile is None:
                return 1

            for name in ignored_list:
                if name in sourceFile:
                    return 0

            clang_tidy_srcs.append(sourceFile)

        elif arg == "-o":
            objectFile = consumeArgument(args)
            if objectFile is None:
                return 1
        else:
            clang_tidy_args.append(arg)

        arg = consumeArgument(args)

    command = ["clang-tidy"] + clang_tidy_srcs + ["--"] + clang_tidy_args
    return subprocess.run(command).returncode


def main():
    returnCode = maybeRunClangTidy(sys.argv[1:])
    if returnCode != 0:
        return returnCode

    return subprocess.run(sys.argv[1:]).returncode


if __name__ == "__main__":
    sys.exit(main())
