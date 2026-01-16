#!/usr/bin/env python3
#
# Copyright 2025 The Project CHIP Authors
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
#
""".
This script is designed to automate the process of fixing `[[nodiscard]]` related
build errors. It works by repeatedly building the project, parsing the compiler
output for specific errors about ignored return values, and then automatically
inserting a `TEMPORARY_RETURN_IGNORED` macro to suppress the error.

The script will continue this cycle of building and patching until the build
succeeds.

Usage:
    python3 scripts/tests/refactor_nodiscard.py
    python3 scripts/tests/refactor_nodiscard.py --input-file /path/to/build_log.txt
"""

import os
import re
import subprocess
import tempfile

import click


def run_command(command_str, capture_output=True, text=True, check=False, input_data=None):
    """
    Runs a shell command string and returns the result.
    """
    print(f"Running shell command: {command_str}")
    return subprocess.run(
        ['bash', '-c', command_str],
        capture_output=capture_output,
        text=text,
        check=check,
        input=input_data
    )


def process_build_output(build_output):
    """
    Parses build output for `[[nodiscard]]` errors and applies fixes.
    Returns True if fixes were applied, False otherwise.
    """
    # Pattern to extract file path and line number from a grep output error line.
    grep_error_line_pattern = re.compile(r"^\d+:(?:\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2} INFO\s+)?([^:]+):(\d+):")

    # Grep for the error
    grep_result = run_command(
        'grep -E -A 1 -n "error: ignor|warning: ignor"',
        input_data=build_output
    )

    if grep_result.returncode != 0 and not grep_result.stdout:
        print("Could not find any 'error: ignor' or 'warning: ignor' lines with grep.")
        return False

    lines_to_fix = {}
    grep_lines = grep_result.stdout.splitlines()
    i = 0
    while i < len(grep_lines):
        line = grep_lines[i]
        i += 1

        error_match = grep_error_line_pattern.match(line)
        if error_match and ("error: ignor" in line or "warning: ignor" in line):
            raw_file_path_from_grep = error_match.group(1)
            line_number = int(error_match.group(2))
            project_root = os.getcwd()
            connectedhomeip_marker = "connectedhomeip/"

            idx = raw_file_path_from_grep.rfind(connectedhomeip_marker)
            if idx != -1:
                relative_path_within_submodule = raw_file_path_from_grep[idx + len(connectedhomeip_marker):]
                file_path = os.path.normpath(os.path.join(project_root, relative_path_within_submodule))
                print(f"Updated {file_path}")
            else:
                if not os.path.isabs(raw_file_path_from_grep):
                    file_path = os.path.normpath(os.path.join(os.path.dirname(
                        __file__), raw_file_path_from_grep.replace("../../", "../", 1)))
                else:
                    file_path = raw_file_path_from_grep

            if i < len(grep_lines) and not grep_lines[i].startswith('--'):
                context_line = grep_lines[i]
                i += 1

                last_pipe_index = context_line.rfind('|')
                if last_pipe_index != -1:
                    expected_content = context_line[last_pipe_index + 1:]
                    lines_to_fix[(file_path, line_number)] = expected_content
                else:
                    print(f"Warning: Could not parse context line for {file_path}:{line_number}: {context_line}")
            else:
                print(f"Warning: No context line found for error at {file_path}:{line_number}")
        elif line.startswith('--'):
            pass

    if not lines_to_fix:
        print("Grep found potential errors, but couldn't parse file and line numbers or context.")
        print("Grep output:")
        print(grep_result.stdout)
        return False

    for (file_path, line_number), expected_content in sorted(lines_to_fix.items(), key=lambda item: item[0][1], reverse=True):
        print(f"Attempting to patch {file_path}:{line_number}")
        try:
            with open(file_path, 'r') as f:
                lines = f.readlines()

            if 1 <= line_number <= len(lines):
                original_line = lines[line_number - 1]
                stripped_original_line = original_line.strip()
                stripped_expected_content = expected_content.strip()

                if stripped_original_line == stripped_expected_content:
                    indentation = original_line[:-len(original_line.lstrip())]
                    modified_line = indentation + "TEMPORARY_RETURN_IGNORED " + original_line.lstrip()
                    lines[line_number - 1] = modified_line

                    print(f"  - Modified line {line_number}:")
                    print(f"    - {original_line.strip()}")
                    print(f"    + {modified_line.strip()}")
                else:
                    print(f"  - Skipping line {line_number} in {file_path} due to content mismatch.")
                    print(f"    Expected: '{stripped_expected_content}'")
                    print(f"    Found:    '{stripped_original_line}'")
            else:
                print(f"  - Warning: line number {line_number} is out of bounds for {file_path} (total lines: {len(lines)})")

            with open(file_path, 'w') as f:
                f.writelines(lines)

        except FileNotFoundError:
            print(f"Error: file not found: {file_path}")
        except Exception as e:
            print(f"An error occurred while processing {file_path}: {e}")

    print("Finished applying patches.")
    return True


@click.command()
@click.option('--input-file', type=click.Path(exists=True, dir_okay=False), help='Path to a file containing build output to process instead of running a build.')
def main(input_file):
    """
    Main function to run the build-fix loop or process a single file.
    """
    if input_file:
        print(f"Processing build output from file: {input_file}")
        with open(input_file, 'r', encoding='utf-8') as f:
            build_output = f.read()
        process_build_output(build_output)
    else:
        combined_build_command = "source ./scripts/activate.sh && ./gn_build.sh"
        iteration = 1
        while True:
            print(f"\n--- Iteration {iteration} ---")
            iteration += 1

            print("Running activate and build...")
            build_result = run_command(combined_build_command)

            if build_result.returncode == 0:
                print("Build successful!")
                break

            print("Build failed. Dumping output to file for manual checking...")
            build_output = build_result.stdout + build_result.stderr

            with tempfile.NamedTemporaryFile(mode='w', delete=False, encoding='utf-8') as f:
                f.write(build_output)
                build_output_filepath = f.name
            print(f"Build output dumped to: {build_output_filepath}")

            if not process_build_output(build_output):
                print("No applicable errors found to fix in the build output. Aborting.")
                break


if __name__ == "__main__":
    main()
