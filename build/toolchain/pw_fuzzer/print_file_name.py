#!/usr/bin/env python3
# Copyright (c) 2026 Project CHIP Authors
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

# Resolves a clang runtime library path via `<compiler> -print-file-name=<lib>`.
# Used by //build/toolchain/pw_fuzzer to link libclang_rt.fuzzer_no_main.a without
# hardcoding the clang runtime version, for both the local CIPD clang and the
# OSS-Fuzz-provided compiler ($CXX). Prints the resolved absolute path on stdout
# (consumed by GN exec_script with input_conversion "trim string").

import shlex
import subprocess
import sys


def main() -> int:
    if len(sys.argv) != 3:
        sys.stderr.write(
            "usage: print_file_name.py <compiler> <runtime-lib-filename>\n")
        return 1
    compiler, libname = sys.argv[1], sys.argv[2]
    # $CXX may carry a launcher or flags (e.g. "ccache clang++", "clang++ -stdlib=libc++");
    # split it so the full command is invoked, not treated as one executable name.
    cmd = shlex.split(compiler) + ["-print-file-name=" + libname]
    try:
        path = subprocess.check_output(cmd, text=True).strip()
    except (OSError, subprocess.SubprocessError) as e:
        sys.stderr.write("error: failed to run '%s': %s\n" % (" ".join(cmd), e))
        return 1
    # clang echoes the bare filename back when it cannot locate the library;
    # fail loudly rather than emitting a bogus linker input.
    if path == libname or not path:
        sys.stderr.write(
            "error: %s could not locate %s\n" % (compiler, libname))
        return 1
    sys.stdout.write(path)
    return 0


if __name__ == "__main__":
    sys.exit(main())
