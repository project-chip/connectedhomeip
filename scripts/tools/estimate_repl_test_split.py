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

"""Estimate "REPL Tests - Linux (RUN)" job durations for a list of test filters.

Uses timings collected by `fetch_repl_test_timings.py` and the python tests
present in the local checkout. Filters use the same semantics as the `--regex`
argument of `src/python_testing/execute_python_tests.py` (regex search on the
script path, a leading "!" negates). The "" filter is skipped: that job runs
individual test steps instead of a filter.

When no --filter is given, the filter list is read from the
`repl_tests_linux_run` matrix in `.github/workflows/tests.yaml`.

Also reports tests matched by no filter or by more than one filter. Exits
with a non-zero code in that case.

Example:
    scripts/tools/estimate_repl_test_split.py --timings /tmp/timings.json \\
        --filter 'TC_[A-B]' --filter 'TC_C[A-K]' --filter '!TC_[A-C]'
"""

import glob
import json
import logging
import os
import re
import statistics
import sys
from pathlib import Path

import click
import yaml

log = logging.getLogger(__name__)

CHIP_ROOT = Path(__file__).resolve().parents[2]
SEARCH_DIRECTORY = "src/python_testing"
WORKFLOW_JOB = "repl_tests_linux_run"

AGGREGATES = {
    "median": statistics.median,
    "mean": statistics.mean,
    "max": max,
}


def load_durations(timing_files: tuple[Path, ...], aggregate: str) -> dict[str, float]:
    """Return test script name -> aggregated duration in seconds."""
    samples: dict[str, list[float]] = {}
    for path in timing_files:
        for run in json.loads(path.read_text())["runs"]:
            for job in run["jobs"]:
                for t in job["tests"]:
                    samples.setdefault(t["name"], []).append(t["duration_seconds"])
    return {name: AGGREGATES[aggregate](values) for name, values in samples.items()}


def load_workflow_filters(workflow: Path) -> list[str]:
    data = yaml.safe_load(workflow.read_text())
    return list(data["jobs"][WORKFLOW_JOB]["strategy"]["matrix"]["filter"])


def list_ci_test_scripts() -> list[str]:
    """List test script paths the same way execute_python_tests.py does (non-nightly run).

    Paths are relative to CHIP_ROOT (e.g. "src/python_testing/TC_ACE_1_2.py"), matching
    what the CI regex filters are applied to.
    """
    with open(CHIP_ROOT / SEARCH_DIRECTORY / "test_metadata.yaml") as f:
        metadata = yaml.safe_load(f)
    excluded = {item["name"] for item in metadata["not_automated"]} | {item["name"] for item in metadata["nightly"]}

    scripts = sorted(glob.glob(os.path.join(SEARCH_DIRECTORY, "*.py"), root_dir=CHIP_ROOT))
    return [s for s in scripts if os.path.basename(s) not in excluded]


def filter_matches(pattern: str, path: str) -> bool:
    """Match semantics of `execute_python_tests.py run --regex`."""
    if pattern.startswith("!"):
        return re.search(pattern[1:], path) is None
    return re.search(pattern, path) is not None


def format_minutes(value: float) -> str:
    return f"{value:.1f}"


@click.command(help=__doc__, context_settings={"max_content_width": 120})
@click.option("--timings", "timing_files", type=click.Path(exists=True, dir_okay=False, path_type=Path), multiple=True, required=True,
              help="JSON file written by fetch_repl_test_timings.py. May be repeated.")
@click.option("--filter", "filters", multiple=True, help="Test filter (regex). May be repeated. Default: read from --workflow.")
@click.option("--workflow", type=click.Path(exists=True, dir_okay=False, path_type=Path),
              default=CHIP_ROOT / ".github/workflows/tests.yaml", show_default=True, help="Workflow to read default filters from.")
@click.option("--aggregate", type=click.Choice(list(AGGREGATES)), default="median", show_default=True,
              help="How to combine durations of a test seen in several runs.")
@click.option("--overhead-minutes", type=float, default=4.5, show_default=True,
              help="Per-job setup time (checkout, bootstrap, artifact unpacking) added to the estimate.")
@click.option("--log-level", default="INFO", type=click.Choice(list(logging.getLevelNamesMapping()), case_sensitive=False))
def main(timing_files: tuple[Path, ...], filters: tuple[str, ...], workflow: Path, aggregate: str, overhead_minutes: float,
         log_level: str) -> None:
    logging.basicConfig(level=log_level.upper(), format="%(levelname)s: %(message)s")

    filter_list = list(filters) if filters else load_workflow_filters(workflow)
    filter_list = [f for f in filter_list if f != ""]

    durations = load_durations(timing_files, aggregate)
    scripts = list_ci_test_scripts()

    matched_by: dict[str, list[str]] = {s: [] for s in scripts}

    print(f"{'filter':<16} {'tests':>6} {'test min':>9} {'est. job min':>13} {'no timing':>10}")
    for f in filter_list:
        selected = [s for s in scripts if filter_matches(f, s)]
        known = [s for s in selected if os.path.basename(s) in durations]
        missing = [s for s in selected if os.path.basename(s) not in durations]
        for s in selected:
            matched_by[s].append(f)

        test_minutes = sum(durations[os.path.basename(s)] for s in known) / 60
        print(f"{f:<16} {len(selected):>6} {format_minutes(test_minutes):>9} "
              f"{format_minutes(test_minutes + overhead_minutes):>13} {len(missing):>10}")
        for s in missing:
            log.debug("  no timing data: %s", s)

    unmatched = [s for s, fs in matched_by.items() if not fs]
    duplicated = {s: fs for s, fs in matched_by.items() if len(fs) > 1}

    for s in unmatched:
        log.error("Not matched by any filter: %s", s)
    for s, fs in duplicated.items():
        log.error("Matched by multiple filters %s: %s", fs, s)

    if unmatched or duplicated:
        sys.exit(1)


if __name__ == "__main__":
    main()
