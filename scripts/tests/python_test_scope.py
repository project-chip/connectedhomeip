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
jobs; see `.github/workflows/pull-request-scope.yaml`.

The REPL test jobs, which run the Python tests, always run. On such a pull
request their shards run only the tests it changes and the tests that import a
module it changes, found by scanning the imports of every file in the tree.

Usage, with one repository path per line on standard input:

    git diff --no-renames --name-only HEAD^1 HEAD | scripts/tests/python_test_scope.py classify

prints `python_tests_only=true` or `python_tests_only=false`, ready to append to
`$GITHUB_OUTPUT`, and the reason on standard error.

    git diff --no-renames --name-only HEAD^1 HEAD | scripts/tests/python_test_scope.py select

prints `python_test_slices=<JSON>`: for each REPL shard filter, a regex that
selects the shard's share of the tests, or "none". It prints an empty value when
every shard must run as usual. `--previous-metadata` takes test_metadata.yaml as
it was before the change, so that a change to it selects only the tests whose
exclusion it changes. `--github-env` also writes PYTHON_TEST_APPS, the
comma-delimited REPL environment variables of the apps those tests and the
unfiltered REPL job need, for the REPL build job's app build steps.
"""

import argparse
import ast
import json
import re
import sys
from collections import deque
from collections.abc import Iterable
from dataclasses import dataclass, field
from pathlib import Path
from typing import TextIO

CHIP_ROOT = Path(__file__).resolve().parents[2]

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

# Inside the test tree, and they decide which tests the REPL shards run.
METADATA_FILE = "src/python_testing/test_metadata.yaml"
REPL_SELECTION_FILES = frozenset({"src/python_testing/execute_python_tests.py", METADATA_FILE})

# Modules here are not selected through their importers: the unfiltered REPL job
# runs them, or CI does not run them at all.
UNSHARDED_DIRECTORIES = ("test_testing/", "post_certification_tests/")

WORKFLOW = ".github/workflows/tests.yaml"
SHARD_JOB = "repl_tests_linux_run"
ENV_FILE_STEP = "Generate an argument environment file"
NO_TESTS = "none"

# Built for every pull request: the unfiltered REPL job runs its tests on them.
UNFILTERED_JOB_APPS = ("ALL_CLUSTERS_APP", "SU_OTA_REQUESTOR_V2")

CI_ARGUMENTS = re.compile(r"=== BEGIN CI TEST ARGUMENTS ===(.*?)=== END CI TEST ARGUMENTS ===", re.S)


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


def runner_matches(pattern: str, path: str) -> bool:
    """Match a shard filter the way `execute_python_tests.py run --regex` does."""
    if pattern.startswith("!"):
        return re.search(pattern[1:], path) is None
    return re.search(pattern, path) is not None


def module_name(rel: str) -> str:
    """Return the import name of a file given relative to the test tree."""
    parts = rel[:-len(".py")].split("/")
    if parts[-1] == "__init__":
        parts.pop()
    return ".".join(parts)


def imported_names(source: str, rel: str) -> set[str]:
    """Return every module, or module attribute, a file imports, as dotted names.

    `from a.b import c` yields both `a.b` and `a.b.c`, since c may be a submodule.
    Relative imports are resolved against the file's own package.
    """
    try:
        tree = ast.parse(source)
    except SyntaxError:
        return set()
    package = rel.split("/")[:-1]
    names = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.Import):
            names.update(alias.name for alias in node.names)
        elif isinstance(node, ast.ImportFrom):
            if node.level:
                if node.level - 1 > len(package):
                    continue
                base = package[:len(package) - (node.level - 1)]
                if node.module:
                    base = base + node.module.split(".")
                module = ".".join(base)
            else:
                module = node.module or ""
            if module:
                names.add(module)
            names.update(f"{module}.{alias.name}" if module else alias.name
                         for alias in node.names if alias.name != "*")
    return names


def imports_module(names: set[str], module: str) -> bool:
    """Whether importing `names` runs `module`; importing a.b.c runs a and a.b too."""
    return any(name == module or name.startswith(module + ".") for name in names)


@dataclass
class TreeFile:
    module: str
    imports: set[str]
    # Every `name.py` the file mentions, to catch helpers launched by path.
    mentioned_files: set[str]


def scan_tree(tree: Path) -> dict[str, TreeFile]:
    """Index every Python file in the test tree outside the framework package."""
    files = {}
    framework = Path(FRAMEWORK_PACKAGE).relative_to(TEST_TREE)
    for path in sorted(tree.rglob("*.py")):
        rel_path = path.relative_to(tree)
        if rel_path.is_relative_to(framework) or "__pycache__" in rel_path.parts:
            continue
        rel = rel_path.as_posix()
        source = path.read_text(errors="replace")
        files[rel] = TreeFile(module=module_name(rel),
                              imports=imported_names(source, rel),
                              mentioned_files=set(re.findall(r"(?<![\w-])([\w-]+\.py)\b", source)))
    return files


def shard_filters(chip_root: Path) -> list[str]:
    """Return the REPL shard filters, as the split estimator reads them."""
    import yaml
    workflow = yaml.safe_load((chip_root / WORKFLOW).read_text())
    return [f for f in workflow["jobs"][SHARD_JOB]["strategy"]["matrix"]["filter"] if f]


def load_metadata(path: Path) -> dict:
    import yaml
    return yaml.safe_load(path.read_text()) or {}


def excluded_tests(metadata: dict) -> dict[str, str]:
    """Return the tests the regular REPL run leaves out, with where they run instead."""
    excluded = {item["name"]: "is listed under not_automated in test_metadata.yaml"
                for item in metadata.get("not_automated") or []}
    excluded.update({item["name"]: "runs only in the nightly workflow"
                     for item in metadata.get("nightly") or []})
    return excluded


def ci_variables(source: str) -> set[str]:
    """Return the ${VARIABLE} placeholders in a test's CI arguments block, across all its runs."""
    block = CI_ARGUMENTS.search(source)
    return set(re.findall(r"\$\{([A-Z_0-9]+)\}", block.group(1))) if block else set()


def environment_file(chip_root: Path) -> dict[str, str]:
    """Return the variables the REPL environment file defines, with their paths."""
    import yaml
    workflow = yaml.safe_load((chip_root / WORKFLOW).read_text())
    for step in workflow["jobs"][SHARD_JOB]["steps"]:
        if step.get("name") == ENV_FILE_STEP:
            return dict(re.findall(r'echo "([A-Z_0-9]+): ([^"]+)" >> /tmp/test_env.yaml', step["run"]))
    return {}


def built_apps(variables: dict[str, str]) -> set[str]:
    """Return the variables whose path is an artifact of the REPL build job."""
    return {name for name, path in variables.items() if path.startswith("objdir-clone/")}


@dataclass
class Selection:
    """The REPL tests a pull request needs.

    `full_reason` is set when every shard must run as usual; otherwise `slices`
    maps each shard filter to the regex for its share of `tests`, or NO_TESTS.
    """
    full_reason: str | None = None
    tests: dict[str, str] = field(default_factory=dict)
    not_run: dict[str, str] = field(default_factory=dict)
    slices: dict[str, str] = field(default_factory=dict)
    # The built apps the selected tests and the unfiltered REPL job need.
    apps: list[str] = field(default_factory=list)


def select(paths: Iterable[str], chip_root: Path = CHIP_ROOT, previous_metadata: dict | None = None) -> Selection:
    """Select the REPL tests that the changed paths can affect.

    `previous_metadata` is test_metadata.yaml before the change. When the change
    edits that file, the tests whose exclusion it changes are selected; without
    the previous version every shard runs.
    """
    changed = sorted({path.strip() for path in paths if path.strip()})
    python_tests_only, reason = classify(changed)
    if not python_tests_only:
        return Selection(full_reason=reason)

    tree = chip_root / TEST_TREE
    excluded = excluded_tests(load_metadata(tree / "test_metadata.yaml"))
    files = scan_tree(tree)
    selection = Selection()

    def runnable(rel: str) -> bool:
        return "/" not in rel and rel.endswith(".py") and rel not in excluded and rel in files

    queue = deque()  # (module, its file, the changed file that led to it)
    seen = set()
    for path in changed:
        rel = path[len(TEST_TREE):]
        if path == METADATA_FILE and previous_metadata is not None:
            before = excluded_tests(previous_metadata)
            for name in sorted(set(before) ^ set(excluded)):
                if runnable(name):
                    selection.tests[name] = "test_metadata.yaml now runs it on pull requests"
                elif name in excluded and name in files:
                    selection.not_run[name] = excluded[name]
            continue
        if path in REPL_SELECTION_FILES:
            return Selection(full_reason=f"{path} decides which tests run")
        if not rel.endswith(".py"):
            if rel.endswith(".md"):
                continue
            return Selection(full_reason=f"{path} is not Python, so which tests read it is unknown")
        if runnable(rel):
            selection.tests[rel] = "changed"
        elif "/" not in rel and rel in excluded and rel in files:
            selection.not_run[rel] = excluded[rel]
        module = module_name(rel)
        seen.add(module)
        queue.append((module, rel, rel))

    while queue:
        module, rel, origin = queue.popleft()
        filename = rel.rsplit("/", 1)[-1]
        importers = [other for other, info in files.items() if other != rel and (
            imports_module(info.imports, module) or
            (filename != "__init__.py" and filename in info.mentioned_files))]
        for other in importers:
            if runnable(other):
                selection.tests.setdefault(other, f"uses {TEST_TREE}{origin}")
            if files[other].module not in seen:
                seen.add(files[other].module)
                queue.append((files[other].module, other, origin))
        if rel != origin:
            continue
        if importers:
            # A helper the regular run leaves out reaches the shards through its importers.
            selection.not_run.pop(rel, None)
        elif rel in files and "/" in rel and not rel.startswith(UNSHARDED_DIRECTORIES) and "/tests/" not in f"/{rel}":
            return Selection(full_reason=f"{TEST_TREE}{rel} is imported by no test, so which tests use it is unknown")

    filters = shard_filters(chip_root)
    for rel in selection.tests:
        if not any(runner_matches(f, TEST_TREE + rel) for f in filters):
            return Selection(full_reason=f"{TEST_TREE}{rel} matches no REPL shard filter")
    for f in filters:
        share = sorted(rel[:-len(".py")] for rel in selection.tests if runner_matches(f, TEST_TREE + rel))
        selection.slices[f] = "/(" + "|".join(map(re.escape, share)) + r")\.py$" if share else NO_TESTS

    variables = environment_file(chip_root)
    apps = set(UNFILTERED_JOB_APPS)
    for rel in selection.tests:
        for name in ci_variables((tree / rel).read_text(errors="replace")):
            if name not in variables:
                return Selection(full_reason=f"{TEST_TREE}{rel} uses ${{{name}}}, which the REPL environment file does not define")
            apps.add(name)
    selection.apps = sorted(apps & built_apps(variables))
    return selection


def summary_markdown(selection: Selection) -> str:
    """Describe a selection for the job summary."""
    lines = ["### Python tests for this pull request", ""]
    if selection.full_reason:
        return "\n".join(lines + [f"Every REPL shard runs as usual: {selection.full_reason}.", ""])
    count = len(selection.tests)
    if count:
        lines.append(f"The REPL shards run {count} {'test' if count == 1 else 'tests'} and skip the rest:")
    else:
        lines.append("The REPL shards have no test to run; the unfiltered REPL job still runs.")
    lines.extend(f"- `{rel}`: {why}" for rel, why in sorted(selection.tests.items()))
    lines += ["", "The REPL build job builds only " + ", ".join(f"`{app}`" for app in selection.apps) + "."]
    if selection.not_run:
        lines += ["", "Changed but not run on pull requests:"]
        lines.extend(f"- `{rel}` {why}" for rel, why in sorted(selection.not_run.items()))
    return "\n".join(lines + [""])


def main(argv: list[str] | None = None, stdin: TextIO = sys.stdin) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser(
        "classify",
        help="read changed paths from standard input and print python_tests_only=true|false")
    select_parser = commands.add_parser(
        "select",
        help="read changed paths from standard input and print python_test_slices=<JSON>")
    select_parser.add_argument("--output", type=argparse.FileType("a"), default=sys.stdout,
                               help="file to append the output line to, such as $GITHUB_OUTPUT")
    select_parser.add_argument("--summary", type=argparse.FileType("a"),
                               help="file to append a markdown description to, such as $GITHUB_STEP_SUMMARY")
    select_parser.add_argument("--github-env", type=argparse.FileType("a"),
                               help="file to append PYTHON_TEST_APPS to, such as $GITHUB_ENV; nothing is written when every app must be built")
    select_parser.add_argument("--previous-metadata", type=Path,
                               help="test_metadata.yaml before the change; without it a change to that file runs every shard")
    select_parser.add_argument("--chip-root", type=Path, default=CHIP_ROOT, help=argparse.SUPPRESS)
    args = parser.parse_args(argv)

    if args.command == "classify":
        python_tests_only, reason = classify(stdin)
        print(f"python_tests_only={'true' if python_tests_only else 'false'}")
        print(f"{'Python tests only' if python_tests_only else 'Full run'}: {reason}", file=sys.stderr)
        return 0

    previous = None
    if args.previous_metadata and args.previous_metadata.is_file() and args.previous_metadata.stat().st_size:
        previous = load_metadata(args.previous_metadata)
    selection = select(stdin, args.chip_root, previous)
    slices = "" if selection.full_reason else json.dumps(selection.slices, separators=(",", ":"))
    print(f"python_test_slices={slices}", file=args.output)
    if args.github_env and not selection.full_reason:
        print(f"PYTHON_TEST_APPS=,{','.join(selection.apps)},", file=args.github_env)
    if args.summary:
        print(summary_markdown(selection), file=args.summary)
    print(summary_markdown(selection), file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
