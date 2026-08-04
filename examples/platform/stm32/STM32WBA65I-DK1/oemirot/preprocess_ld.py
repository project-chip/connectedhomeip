#!/usr/bin/env python3
# Copyright (c) 2024 Project CHIP Authors
#
# Preprocesses a linker script using arm-none-eabi-gcc -E -P
# This resolves #include directives and macro definitions
#
# Usage: preprocess_ld.py --input <input.ld.in> --output <output.ld> --include-dir <path> [--define <DEF>]

import argparse
import subprocess
import sys
import os

def main():
    parser = argparse.ArgumentParser(description='Preprocess linker script with GCC preprocessor')
    parser.add_argument('--input', required=True, help='Input linker script with preprocessor directives')
    parser.add_argument('--output', required=True, help='Output preprocessed linker script')
    parser.add_argument('--include-dir', action='append', dest='include_dirs', default=[], help='Include directory (can be repeated)')
    parser.add_argument('--define', action='append', dest='defines', default=[], help='Preprocessor define (can be repeated)')
    args = parser.parse_args()

    # Build the GCC command
    cmd = ['arm-none-eabi-gcc', '-E', '-P', '-xc']

    # Add defines
    for define in args.defines:
        cmd.append(f'-D{define}')

    # Add include directories
    for inc_dir in args.include_dirs:
        cmd.append(f'-I{inc_dir}')

    # Add output and input
    cmd.extend(['-o', args.output, args.input])

    print(f"Preprocessing linker script: {args.input} -> {args.output}")
    print(f"Command: {' '.join(cmd)}")

    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        if result.stdout:
            print(result.stdout)
        print(f"Successfully generated: {args.output}")
        return 0
    except subprocess.CalledProcessError as e:
        print(f"Error preprocessing linker script: {e}", file=sys.stderr)
        if e.stderr:
            print(e.stderr, file=sys.stderr)
        return 1
    except FileNotFoundError:
        print("Error: arm-none-eabi-gcc not found. Make sure ARM GCC toolchain is in PATH.", file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
