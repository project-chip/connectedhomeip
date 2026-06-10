#!/usr/bin/env python3

#
# Copyright (c) 2020 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import os
import subprocess
import sys

# Calculate OUTPUT_ROOT, equivalent to the Bash variable
# This navigates two directories up from the script's location and then into 'out/coverage'
script_dir = os.path.dirname(__file__)
OUTPUT_ROOT = os.path.abspath(os.path.join(script_dir, '..', '..', 'out', 'coverage'))


def parse_input_targets(query_output_lines, possible_rules_set, rules_set, new_targets_set):
    """
    Parses the input from `ninja -t query` output.
    It extracts input targets and identifies associated rules, updating the provided sets.

    Args:
        query_output_lines (list): A list of strings, where each string is a line from the ninja query output.
        possible_rules_set (set): A set of all rules considered 'possible' for cleaning.
        rules_set (set): A set to store rules that need to be cleaned (populated by this function).
        new_targets_set (set): A set to store new targets found that need further querying.
    """
    in_input_block = False
    for line in query_output_lines:

        trimmed_line = line.strip()

        if trimmed_line.startswith("input: "):
            rule = trimmed_line[len("input: "):].strip()
            if rule == "phony":
                in_input_block = True
            elif rule in possible_rules_set:
                rules_set.add(rule)  # Add rule to the set
        elif not in_input_block:
            continue
        elif trimmed_line == "outputs:":
            in_input_block = False
        # Add new targets only if they haven't been queried already
        elif in_input_block:
            new_targets_set.add(trimmed_line)  # Add new target to the set


def get_rules_to_clean(initial_targets):
    """
    Determines the rules that should be cleaned based on the initial targets provided.
    This function simulates the logic of `ninja -t clean -r <rules>`.

    Args:
        initial_targets (list): A list of target names (e.g., ['my_target', 'all']).

    Returns:
        list: A list of rule names that should be cleaned.
    """
    rules_to_clean = set()  # Stores rules identified for cleaning (now a set)
    queried_targets = set()  # Stores targets that have already been queried
    targets_to_query = set(initial_targets)  # Targets for the current iteration

    # Get all the rules that execute targets from the toolchain.ninja file
    # Equivalent to: ninja -C out/coverage -f toolchain.ninja -t rules | grep "__rule"
    try:
        ninja_rules_process = subprocess.run(
            ['ninja', '-C', OUTPUT_ROOT, '-f', 'toolchain.ninja', '-t', 'rules'],
            capture_output=True, text=True, check=True
        )
        # Filter for lines containing "__rule"
        possible_rules_set = {line.strip() for line in ninja_rules_process.stdout.splitlines() if "__rule" in line}
    except subprocess.CalledProcessError as e:
        print(f"Error getting possible rules from ninja: {e}", file=sys.stderr)
        print(f"Stdout: {e.stdout}", file=sys.stderr)
        print(f"Stderr: {e.stderr}", file=sys.stderr)
        sys.exit(1)

    while targets_to_query:

        if "all" in targets_to_query:
            return list(possible_rules_set)

        new_targets = set()  # Stores targets discovered in the current query that need future querying

        # Query the targets and parse their inputs
        query_cmd = ['ninja', '-C', OUTPUT_ROOT, '-t', 'query'] + list(targets_to_query)
        try:
            query_process = subprocess.run(
                query_cmd,
                capture_output=True, text=True, check=True
            )

            # Lines starting with a space and not containing '|' are relevant input targets
            filtered_query_lines = [
                line for line in query_process.stdout.splitlines()
                if line.startswith(" ") and "|" not in line
            ]

            parse_input_targets(
                filtered_query_lines,
                possible_rules_set,
                rules_to_clean,
                new_targets
            )

        except subprocess.CalledProcessError as e:
            print(f"Error querying ninja for targets: {e}", file=sys.stderr)
            print(f"Command: {' '.join(query_cmd)}", file=sys.stderr)
            print(f"Stdout: {e.stdout}", file=sys.stderr)
            print(f"Stderr: {e.stderr}", file=sys.stderr)
            sys.exit(1)

        # Mark the current targets as queried
        queried_targets |= targets_to_query

        # Update targets_to_query for the next iteration with the newly found targets
        targets_to_query = new_targets - queried_targets

    # After all targets are processed, return the rules that were found
    return list(rules_to_clean)


# Main script execution
if __name__ == "__main__":
    # Get command-line arguments (targets to clean)
    targets_from_cli = sys.argv[1:]

    if not targets_from_cli:
        print(f"Usage: {sys.argv[0]} <target1> [target2 ...]", file=sys.stderr)
        sys.exit(1)

    print(f"Determining rules to clean for targets: {targets_from_cli}")
    rules_to_clear = get_rules_to_clean(targets_from_cli)

    if rules_to_clear:
        try:
            # Construct the ninja clean command with the identified rules
            clean_cmd = ['ninja', '-C', OUTPUT_ROOT, '-f', 'toolchain.ninja', '-t', 'clean', '-r'] + rules_to_clear

            subprocess.run(clean_cmd, check=True)  # `check=True` raises CalledProcessError on non-zero exit code
            print("Ninja clean command executed successfully.")
        except subprocess.CalledProcessError as e:
            print(f"Error executing ninja clean command: {e}", file=sys.stderr)
            print(f"Stdout: {e.stdout}", file=sys.stderr)
            print(f"Stderr: {e.stderr}", file=sys.stderr)
            sys.exit(e.returncode)
    else:
        print("No rules found to clean for the given targets.")
