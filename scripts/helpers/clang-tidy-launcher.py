#!/usr/bin/env python

import subprocess
import sys


def main():
    if len(sys.argv) < 2:
        return 1

    cc = sys.argv[1]
    if not cc.startswith("clang"):
        return 0

    command = ["clang-tidy"]
    clang_args = []

    i = 2
    while i < len(sys.argv):
        if sys.argv[i] == "-c":
            # Source file
            i += 1
            if i >= len(sys.argv):
                print("Got -c without argument", file=sys.stderr)
                return 1
            command.append(sys.argv[i])
        elif sys.argv[i] == "-o":
            # Ignore output
            i += 1
            if i >= len(sys.argv):
                print("Got -o without argument", file=sys.stderr)
                return 1
        else:
            clang_args.append(sys.argv[i])

        i += 1

    tidy_result = subprocess.run(command + ["--"] + clang_args)
    if tidy_result.returncode != 0:
        return tidy_result.returncode

    clang_result = subprocess.run(sys.argv[1:])
    return clang_result.returncode


if __name__ == "__main__":
    sys.exit(main())
