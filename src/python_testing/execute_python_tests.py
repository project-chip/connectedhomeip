#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2024 Project CHIP Authors
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

import datetime
import fnmatch
import glob as g
import json
import logging
import os
import re
import subprocess
import sys
import time
from dataclasses import asdict, dataclass, field
from pathlib import Path
from xml.dom.minidom import parseString
from xml.etree.ElementTree import Element, SubElement, tostring

import click
import coloredlogs
import yaml

log = logging.getLogger(__name__)


@dataclass
class TestResult:
    name: str
    status: str          # "passed" | "failed" | "dry_run"
    duration_seconds: float
    error_message: str | None = None


@dataclass
class RunSummary:
    run_timestamp: datetime.datetime
    total_runs: int = 0
    passed: int = 0
    failed: int = 0
    results: list[TestResult] = field(default_factory=list)

    def record(self, name: str, status: str, duration: float, error_message: str | None = None) -> None:
        self.results.append(TestResult(name=name, status=status, duration_seconds=round(duration, 3), error_message=error_message))
        if status == "passed":
            self.passed += 1
        elif status == "failed":
            self.failed += 1

    def write_json(self, path: Path) -> None:
        data = asdict(self)
        data["run_timestamp"] = self.run_timestamp.isoformat()
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(data, indent=2))
        log.info("Test run summary written to %s", path)

    def write_junit_xml(self, suite_name: str, path: Path) -> None:
        failures = sum(1 for r in self.results if r.status == "failed")
        skipped = sum(1 for r in self.results if r.status == "dry_run")
        total_time = sum(r.duration_seconds for r in self.results)

        suite = Element("testsuite", {
            "name": suite_name,
            "tests": str(len(self.results)),
            "failures": str(failures),
            "skipped": str(skipped),
            "errors": "0",
            "time": f"{total_time:.3f}",
            "timestamp": self.run_timestamp.strftime("%Y-%m-%dT%H:%M:%S"),
        })

        for r in self.results:
            tc = SubElement(suite, "testcase", {
                "name": r.name,
                "classname": suite_name,
                "time": f"{r.duration_seconds:.3f}",
            })
            if r.status == "failed":
                failure = SubElement(tc, "failure", {"message": f"{r.name} failed"})
                if r.error_message:
                    failure.text = r.error_message
                    SubElement(tc, "system-out").text = r.error_message
            elif r.status == "dry_run":
                SubElement(tc, "skipped", {"message": "dry run"})

        path.parent.mkdir(parents=True, exist_ok=True)
        xml_str = parseString(tostring(suite, encoding="unicode")).toprettyxml(indent="  ")
        path.write_text(xml_str)
        log.info("JUnit XML written to %s", path)


def load_env_from_yaml(file_path):
    """
    Load environment variables from the specified YAML file.

    The YAML file contains key-value pairs that define --app environment variables
    required for the test scripts to run. These variables configurations needed during the test execution.

    This function reads the YAML file and sets the environment variables
    in the current process's environment using os.environ.

    Args:
        file_path (str): The path to the YAML file containing the environment variables.
    """
    with open(file_path) as f:
        for key, value in yaml.full_load(f).items():
            os.environ[key] = value


@click.group()
def main():
    pass


@main.command('run', help='Execute the Python certification tests.')
@click.option(
    "--search-directory",
    type=str,
    default="src/python_testing",
    help="Directory to search for Python scripts.",
)
@click.option(
    "--env-file",
    type=str,
    default="/tmp/test_env.yaml",
    help="Path to the environment variables file.",
)
@click.option(
    "--keep-going",
    is_flag=True,
    help="Run ALL the tests, report a final status of what passed/failed.",
)
@click.option(
    "--dry-run",
    is_flag=True,
    help="If true, just print out what will be run but do not execute.",
)
@click.option(
    "--glob",
    multiple=True,
    help="Glob the tests to pick. Use `!` to negate the glob. Glob FILTERS out non-matching (i.e. you can use it to restrict more and more, but not to add)",
)
@click.option(
    "--regex",
    multiple=True,
    help="Regex the tests to pick. Use `!` to negate the expression. Expressions FILTERS out non-matching (i.e. you can use it to restrict more and more, but not to add)",
)
@click.option(
    "--nightly",
    is_flag=True,
    help="If set only run tests under the nightly section.",
)
@click.option(
    "--summary-file",
    type=click.Path(dir_okay=False, path_type=Path),
    default=None,
    help="If provided, write a JSON test-run summary to this file at the end of the run.",
)
@click.option(
    "--junit-file",
    type=click.Path(dir_okay=False, path_type=Path),
    default=None,
    help="If provided, write test results as JUnit XML to this file at the end of the run.",
)
@click.option(
    "--junit-suite-name",
    type=str,
    default="execute_python_tests script",
    help="Name for the JUnit XML test suite (default: execute_python_tests script).",
)
def cmd_run(search_directory, env_file, keep_going, dry_run: bool, glob: list[str], regex: list[str], nightly: bool, summary_file: Path | None, junit_file: Path | None, junit_suite_name: str):
    chip_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

    load_env_from_yaml(env_file)

    base_command = os.path.join(chip_root, "scripts/tests/run_python_test.py")

    with open(os.path.join(chip_root, "src/python_testing/test_metadata.yaml")) as f:
        metadata = yaml.full_load(f)
    excluded_patterns = {item["name"] for item in metadata["not_automated"]}
    nightly_tests = {item["name"] for item in metadata["nightly"]}

    all_python_files = g.glob(os.path.join(search_directory, "*.py"))

    for pattern in glob:
        if pattern.startswith('!'):
            def match(p): return not fnmatch.fnmatch(p, f"*{pattern[1:]}*")
        else:
            def match(p): return fnmatch.fnmatch(p, f"*{pattern}*")
        all_python_files = [path for path in all_python_files if match(path)]

    for pattern in regex:
        if pattern.startswith('!'):
            r = re.compile(pattern[1:])
            def match(p): return not r.search(p)
        else:
            r = re.compile(pattern)
            def match(p): return r.search(p) is not None
        all_python_files = [path for path in all_python_files if match(path)]

    # If nightly flag is set, only run tests listed under the nightly section.
    # Otherwise, exclude both not_automated tests and nightly tests from the regular CI run
    # (nightly tests are reserved for the nightly workflow).
    if nightly and nightly_tests is not None:
        python_files = [file for file in all_python_files if os.path.basename(file) in nightly_tests]
    else:
        python_files = [
            file for file in all_python_files
            if os.path.basename(file) not in excluded_patterns
            and os.path.basename(file) not in nightly_tests
        ]

    if len(python_files) == 0:
        # No files match
        log.error("No tests to execute")
        sys.exit(1)

    run_summary = RunSummary(run_timestamp=datetime.datetime.now(datetime.timezone.utc))

    failed_scripts = []
    try:
        for script in python_files:
            test_start = time.monotonic()
            try:
                full_command = f"{base_command} --load-from-env {env_file} --script {script}"
                if dry_run:
                    print(f"DRY-RUN(skip): {full_command}", flush=True)
                    run_summary.record(os.path.basename(script), "dry_run", time.monotonic() - test_start)
                else:
                    print(f"Running command: {full_command}", flush=True)
                    subprocess.run(full_command, shell=True, check=True)
                    run_summary.record(os.path.basename(script), "passed", time.monotonic() - test_start)
            except Exception as e:
                run_summary.record(os.path.basename(script), "failed", time.monotonic() - test_start, error_message=str(e))
                if keep_going:
                    failed_scripts.append(script)
                else:
                    raise
    finally:
        run_summary.total_runs = len(run_summary.results)
        if summary_file is not None:
            run_summary.write_json(summary_file)
        if junit_file is not None:
            run_summary.write_junit_xml(junit_suite_name, junit_file)

    if failed_scripts:
        log.error("FAILURES detected:")
        for s in failed_scripts:
            log.error("   - %s", s)
        sys.exit(1)


@main.command('summarize', help='Pretty-print a JSON summary file produced by the "run" command.')
@click.option(
    '--summary-file',
    required=True,
    type=click.Path(exists=True, dir_okay=False, path_type=Path),
    help='Path to the JSON summary file to display.',
)
@click.option(
    '--top-slowest',
    default=20,
    show_default=True,
    type=click.IntRange(min=1),
    help='Number of slowest tests to include in the timing table.',
)
def cmd_summarize(summary_file: Path, top_slowest: int) -> None:
    raw = json.loads(summary_file.read_text())
    results: list[dict] = raw.get("results", [])

    passed = raw.get("passed", 0)
    failed = raw.get("failed", 0)
    total = raw.get("total_runs", len(results))
    ts = raw.get("run_timestamp", "unknown")

    sep = "=" * 72

    print(sep)
    print("  PYTHON TEST RUN SUMMARY")
    print(sep)
    print(f"  Run timestamp : {ts}")
    print(f"  Total runs    : {total}")
    print(f"  Passed        : {passed}")
    print(f"  Failed        : {failed}")
    if total:
        print(f"  Pass rate     : {100 * passed / total:.1f}%")
    print(sep)

    failed_results = [r for r in results if r["status"] == "failed"]
    if failed_results:
        print(f"\n  FAILED TESTS ({len(failed_results)}):")
        print(f"  {'Test name':<60} {'Duration':>10}")
        print("  " + "-" * 72)
        for r in sorted(failed_results, key=lambda x: x["name"]):
            print(f"  {'✗  ' + r['name']:<60} {r['duration_seconds']:>9.2f}s")
    else:
        print("\n  No failures recorded.")

    slowest = sorted(
        [r for r in results if r["status"] != "dry_run"],
        key=lambda x: -x["duration_seconds"]
    )[:top_slowest]

    if slowest:
        print(f"\n  SLOWEST {top_slowest} TEST RUNS:")
        print(f"  {'Test name':<60} {'Status':<8}  {'Duration':>10}")
        print("  " + "-" * 82)
        for r in slowest:
            mark = "✓" if r["status"] == "passed" else "✗"
            print(f"  {mark + '  ' + r['name']:<60} {r['status']:<8}  {r['duration_seconds']:>9.2f}s")

    print(sep)


if __name__ == "__main__":
    coloredlogs.install(level="INFO")
    main(auto_envvar_prefix="CHIP")
