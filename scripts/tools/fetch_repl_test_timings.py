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

"""Fetch per-test timings of "REPL Tests - Linux (RUN)" jobs from GitHub Actions runs.

For every given workflow run of `.github/workflows/tests.yaml` this script:
  - reads wall-clock durations of the "REPL Tests - Linux (RUN)" matrix jobs
  - downloads the `test-summaries-python-<filter>` artifacts (written by
    `src/python_testing/execute_python_tests.py --summary-file`)
  - prints per-job and slowest-test tables
  - optionally writes all data as JSON, to be used by `estimate_repl_test_split.py`

Requires an authenticated `gh` CLI.

Example:
    scripts/tools/fetch_repl_test_timings.py --run-id 36724354164 --output /tmp/timings.json
"""

import datetime
import json
import logging
import re
import subprocess
import tempfile
from dataclasses import asdict, dataclass, field
from pathlib import Path

import click

log = logging.getLogger(__name__)

DEFAULT_REPOSITORY = "project-chip/connectedhomeip"
JOB_NAME_PREFIX = "REPL Tests - Linux (RUN)"
ARTIFACT_PREFIX = "test-summaries-python-"
SUMMARY_FILE_NAME = "python-tests.json"


@dataclass
class TestTiming:
    name: str
    status: str
    duration_seconds: float


@dataclass
class JobTiming:
    filter: str
    conclusion: str
    job_minutes: float | None
    tests: list[TestTiming] = field(default_factory=list)
    has_summary: bool = False

    @property
    def test_minutes(self) -> float:
        return sum(t.duration_seconds for t in self.tests) / 60


@dataclass
class RunTiming:
    run_id: int
    jobs: list[JobTiming] = field(default_factory=list)


def gh(*args: str) -> str:
    """Run a `gh` CLI command and return its stdout."""
    result = subprocess.run(["gh", *args], check=True, capture_output=True, text=True)
    return result.stdout


def filter_from_job_name(name: str) -> str | None:
    """Extract the matrix filter from a job name like "REPL Tests - Linux (RUN) (TC_A)".

    Returns "" for the job without a filter suffix and None for unrelated jobs.
    """
    if not name.startswith(JOB_NAME_PREFIX):
        return None
    suffix = name[len(JOB_NAME_PREFIX):].strip()
    m = re.fullmatch(r"\((.*)\)", suffix)
    return m.group(1) if m else ""


def minutes_between(start: str | None, end: str | None) -> float | None:
    if not start or not end:
        return None
    delta = datetime.datetime.fromisoformat(end) - datetime.datetime.fromisoformat(start)
    return delta.total_seconds() / 60


def fetch_run(repo: str, run_id: int) -> RunTiming:
    run = RunTiming(run_id=run_id)

    jobs = json.loads(gh("run", "view", str(run_id), "-R", repo, "--json", "jobs"))["jobs"]
    by_filter: dict[str, JobTiming] = {}
    for job in jobs:
        job_filter = filter_from_job_name(job["name"])
        if job_filter is None:
            continue
        by_filter[job_filter] = JobTiming(
            filter=job_filter,
            conclusion=job.get("conclusion") or job.get("status") or "unknown",
            job_minutes=minutes_between(job.get("startedAt"), job.get("completedAt")),
        )

    with tempfile.TemporaryDirectory() as tmp:
        try:
            gh("run", "download", str(run_id), "-R", repo, "--pattern", f"{ARTIFACT_PREFIX}*", "--dir", tmp)
        except subprocess.CalledProcessError as e:
            log.warning("Run %d: failed to download %s* artifacts: %s", run_id, ARTIFACT_PREFIX, e.stderr.strip())

        for summary_path in sorted(Path(tmp).glob(f"{ARTIFACT_PREFIX}*/{SUMMARY_FILE_NAME}")):
            job_filter = summary_path.parent.name[len(ARTIFACT_PREFIX):]
            job = by_filter.setdefault(job_filter, JobTiming(filter=job_filter, conclusion="unknown", job_minutes=None))
            job.has_summary = True
            for r in json.loads(summary_path.read_text())["results"]:
                job.tests.append(TestTiming(name=r["name"], status=r["status"], duration_seconds=r["duration_seconds"]))

    for job in by_filter.values():
        # The "" job runs individual test steps and does not produce a summary.
        if job.filter and not job.has_summary:
            log.warning("Run %d: no timing summary for filter '%s' (conclusion: %s)", run_id, job.filter, job.conclusion)

    run.jobs = sorted(by_filter.values(), key=lambda j: j.filter)
    return run


def format_minutes(value: float | None) -> str:
    return "-" if value is None else f"{value:.1f}"


def print_run(run: RunTiming) -> None:
    print(f"\nRun {run.run_id}")
    print(f"  {'filter':<16} {'conclusion':<12} {'tests':>6} {'test min':>9} {'job min':>8} {'overhead':>9}")
    for job in run.jobs:
        test_minutes = job.test_minutes if job.has_summary else None
        overhead = None
        if test_minutes is not None and job.job_minutes is not None:
            overhead = job.job_minutes - test_minutes
        print(
            f"  {repr(job.filter):<16} {job.conclusion:<12} {len(job.tests) if job.has_summary else '-':>6} "
            f"{format_minutes(test_minutes):>9} {format_minutes(job.job_minutes):>8} {format_minutes(overhead):>9}"
        )


def print_slowest(runs: list[RunTiming], count: int) -> None:
    longest: dict[str, float] = {}
    for run in runs:
        for job in run.jobs:
            for t in job.tests:
                longest[t.name] = max(longest.get(t.name, 0.0), t.duration_seconds)

    if not longest or count <= 0:
        return

    print(f"\nSlowest {count} tests (max over runs):")
    for name, seconds in sorted(longest.items(), key=lambda x: -x[1])[:count]:
        print(f"  {name:<60} {seconds / 60:>6.1f} min")


@click.command(help=__doc__, context_settings={"max_content_width": 120})
@click.option("--run-id", "run_ids", type=int, multiple=True, required=True, help="GitHub Actions run ID. May be repeated.")
@click.option("--repo", default=DEFAULT_REPOSITORY, show_default=True, help="GitHub repository.")
@click.option("--output", type=click.Path(dir_okay=False, path_type=Path), help="Write collected timings to this JSON file.")
@click.option("--top-slowest", default=20, show_default=True, help="Number of slowest tests to print.")
@click.option("--log-level", default="INFO", type=click.Choice(list(logging.getLevelNamesMapping()), case_sensitive=False))
def main(run_ids: tuple[int, ...], repo: str, output: Path | None, top_slowest: int, log_level: str) -> None:
    logging.basicConfig(level=log_level.upper(), format="%(levelname)s: %(message)s")

    runs = []
    for run_id in run_ids:
        log.info("Fetching run %d", run_id)
        runs.append(fetch_run(repo, run_id))

    for run in runs:
        print_run(run)
    print_slowest(runs, top_slowest)

    if output is not None:
        output.write_text(json.dumps({"runs": [asdict(r) for r in runs]}, indent=2))
        log.info("Timings written to %s", output)


if __name__ == "__main__":
    main()
