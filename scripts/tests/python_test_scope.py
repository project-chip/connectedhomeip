#!/usr/bin/env python3
#
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
#

"""Decide how much CI a pull request needs from the files it changes.

A pull request whose changes all sit in the Python certification test tree
(`src/python_testing`) cannot affect any build, or any test outside that tree,
as long as it leaves alone the test framework package and the files the builds
read. The workflows that only build or test code elsewhere then skip their
jobs; see `.github/workflows/pull-request-scope.yaml`. The REPL test jobs, which
run the Python tests, always run.

Usage, with one repository path per line on standard input:

    git diff --no-renames --name-only HEAD^1 HEAD | scripts/tests/python_test_scope.py classify

prints `python_tests_only=true` or `python_tests_only=false`, ready to append to
`$GITHUB_OUTPUT`, and the reason on standard error.
"""

import argparse
import sys
from collections.abc import Iterable
from typing import TextIO

TEST_TREE = "src/python_testing/"

# Inside the test tree, but every test depends on it.
FRAMEWORK_PACKAGE = "src/python_testing/matter_testing_infrastructure/"

# Inside the test tree, but installed by every Python environment build,
# including the macOS one that a Python test change skips.
PYTHON_ENVIRONMENT_FILES = frozenset({
    "src/python_testing/requirements.nfc.txt",
    "src/python_testing/requirements.txt",
})

BUILD_FILE_SUFFIXES = (".gn", ".gni")


def full_run_reason(path: str) -> str | None:
    """Return why a changed path needs the full CI run, or None for a Python test change."""
    if not path.startswith(TEST_TREE):
        return "outside the Python test tree"
    if path.startswith(FRAMEWORK_PACKAGE):
        return "in the test framework package"
    if path in PYTHON_ENVIRONMENT_FILES:
        return "installed by every Python environment build"
    if path.endswith(BUILD_FILE_SUFFIXES):
        return "a build file"
    return None


def classify(paths: Iterable[str]) -> tuple[bool, str]:
    """Return whether every changed path is a Python test change, and why.

    An empty list is not a Python test change: with nothing to go on, CI runs in full.
    """
    changed = [path.strip() for path in paths if path.strip()]
    if not changed:
        return False, "no changed files were listed"
    for path in changed:
        reason = full_run_reason(path)
        if reason:
            return False, f"{path} is {reason}"
    noun = "file" if len(changed) == 1 else "files"
    return True, f"{len(changed)} changed {noun}, all Python tests or test helpers"


def main(argv: list[str] | None = None, stdin: TextIO = sys.stdin) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser(
        "classify",
        help="read changed paths from standard input and print python_tests_only=true|false")
    parser.parse_args(argv)

    python_tests_only, reason = classify(stdin)
    print(f"python_tests_only={'true' if python_tests_only else 'false'}")
    print(f"{'Python tests only' if python_tests_only else 'Full run'}: {reason}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
