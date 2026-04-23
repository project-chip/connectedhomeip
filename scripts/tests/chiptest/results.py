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

from __future__ import annotations

import base64
import contextlib
import datetime
import enum
import json
import logging
import threading
import time
from collections import defaultdict
from collections.abc import Callable, Iterable, Mapping
from dataclasses import dataclass, field, fields, is_dataclass
from pathlib import Path
from types import TracebackType
from typing import Any, ClassVar, TypeAlias

from chiptest.log_config import LogConfig
from chiptest.work_queue import CancellableQueue, EndOfQueue

log = logging.getLogger(__name__)

ExceptionInfoT: TypeAlias = BaseException | str | None


class TestStatus(enum.StrEnum):
    """Status of test execution."""

    PASSED = enum.auto()
    FAILED = enum.auto()
    DRY_RUN = enum.auto()
    CANCELLED = enum.auto()

    @property
    def symbol(self) -> str:
        match self:
            case TestStatus.PASSED | TestStatus.DRY_RUN:
                return "✓"
            case TestStatus.FAILED:
                return "✗"
            case TestStatus.CANCELLED:
                return "!"
            case unknown:
                raise ValueError(f"Unknown test status: {unknown}")

    def decorate_name(self, name: str) -> str:
        return f"{self.symbol} {name}"


@dataclass
class TestResult:
    """Summary of execution of a single test."""

    name: str
    iteration: int
    status: TestStatus
    duration_seconds: float
    exception: ExceptionInfoT = None

    @property
    def name_decorated(self) -> str:
        return TestStatus(self.status).decorate_name(self.name)

    def __post_init__(self):
        # Ensure that status is of type TestStatus, even if it's passed as a string (e.g. when loading from JSON).
        self.status = TestStatus(self.status)

    @classmethod
    def run_test(cls, name: str, iteration: int, dry_run: bool, log_config: LogConfig, test_func: Callable[[], None]) -> TestResult:
        """Run a test and generate execution summary.

        Mind that it catches any exception and saves it in the result. It's up to the caller to reraise the exception.
        """
        with log_config.fmt_context(task=name, level=log_config.level_tests):
            log.info("%s", "Would run test" if dry_run else "Starting test")

            result = cls(name, iteration, TestStatus.FAILED, duration_seconds=0, exception=None)
            test_start = test_end = time.monotonic()
            try:
                test_func()
                test_end = time.monotonic()
                result.status = TestStatus.DRY_RUN if dry_run else TestStatus.PASSED
            except BaseException as e:
                test_end = time.monotonic()
                result.exception = e

                if isinstance(e, KeyboardInterrupt):
                    result.status = TestStatus.CANCELLED
                else:
                    result.status = TestStatus.FAILED
                    if (pcap_path := Path("thread.pcap")).exists():
                        print("base64 -d - >thread.pcap <<EOF")
                        print(base64.b64encode(pcap_path.read_bytes()).decode("ascii"))
                        print("EOF")
            finally:
                result.duration_seconds = test_end - test_start

                symbol = result.status.symbol
                match result.status:
                    case TestStatus.PASSED:
                        log.info("%s Completed in %0.2f seconds", symbol, result.duration_seconds)
                    case TestStatus.CANCELLED:
                        log.warning("%s Cancelled after %0.2f seconds", symbol, result.duration_seconds)
                    case TestStatus.FAILED:
                        assert isinstance(result.exception, BaseException), "Exception should be set for failed test results"
                        log.error("%s Failed in %0.2f seconds", symbol, result.duration_seconds,
                                  exc_info=(type(result.exception), result.exception, result.exception.__traceback__))

                return result


@dataclass
class RunStats:
    """Statistics of test runs, aggregated across iterations."""
    total_runs: int = field(default=0, init=False)
    passed: int = field(default=0, init=False)
    failed: int = field(default=0, init=False)
    cancelled: int = field(default=0, init=False)
    mean_duration: float = field(default=0.0, init=False)
    exception_first: ExceptionInfoT = field(default=None, init=False)

    @property
    def pass_rate(self) -> float:
        return self.passed / self.total_runs if self.total_runs > 0 else 0.0

    @property
    def fail_rate(self) -> float:
        return self.failed / self.total_runs if self.total_runs > 0 else 0.0

    @property
    def status_msg(self) -> str:
        if self.cancelled:
            return f"{TestStatus.CANCELLED.symbol} Cancelled"
        if self.failed:
            exc = self.exception_first if isinstance(self.exception_first, str) else repr(self.exception_first)
            return f"{TestStatus.FAILED.symbol} {exc}"
        return TestStatus.PASSED.symbol

    def record(self, result: TestResult) -> None:
        """Record a test result."""
        # Increment counters depending on the result status.
        self.total_runs += 1
        match result.status:
            case TestStatus.PASSED | TestStatus.DRY_RUN:
                self.passed += 1
            case TestStatus.FAILED:
                self.failed += 1
            case TestStatus.CANCELLED:
                self.cancelled += 1

        # Calculate cumulative average.
        self.mean_duration += (result.duration_seconds - self.mean_duration) / self.total_runs

        # Save the exception if it's the first one.
        if result.exception is not None and self.exception_first is None:
            self.exception_first = result.exception


@dataclass
class RunSummary(RunStats):
    """
    Summary of a test run, including results of all iterations and aggregated statistics (both global and per-test).

    If operated in multithreaded environment, it should be used as a context manager to ensure thread safety when recording results.
    """
    iterations: int
    tests_per_iteration: int
    run_timestamp: datetime.datetime | str = field(default_factory=lambda: datetime.datetime.now(datetime.timezone.utc))
    results: list[TestResult] = field(default_factory=list, init=False)
    test_stats: dict[str, RunStats] = field(default_factory=dict, init=False)
    exceptions: defaultdict[int, dict[str, ExceptionInfoT]] = field(default_factory=lambda: defaultdict(dict), init=False)

    def __post_init__(self):
        self._lock = threading.Lock()

    def __enter__(self):
        return self._lock.__enter__()

    def __exit__(self, exc_type: type[BaseException] | None, exc_val: BaseException | None, exc_tb: TracebackType | None) -> None:
        return self._lock.__exit__(exc_type, exc_val, exc_tb)

    def record(self, result: TestResult) -> None:
        """Record a test result."""
        self.results.append(result)

        # Increment global counters.
        super().record(result)

        # Increment per-test statistics.
        if result.name not in self.test_stats:
            self.test_stats[result.name] = RunStats()
        self.test_stats[result.name].record(result)

        # Record exception per iteration.
        self.exceptions[result.iteration][result.name] = result.exception

    @property
    def expected_test_count(self) -> int:
        """Total number of tests expected to be run across all iterations."""
        return self.iterations * self.tests_per_iteration

    @property
    def current_iteration(self) -> int:
        """Estimate the current iteration based on the number of recorded results."""
        return len(self.exceptions)

    def write_json(self, path: Path) -> None:
        """Write the test run summary to a JSON file."""
        def encode(obj: Any) -> Any:
            """JSON encoder for non-serializable objects.

            We cannot use json.dumps(default) for all cases, as it doesn't touch floats, and `asdict` doesn't work for defaultdict.
            """
            if is_dataclass(obj):
                return {f.name: encode(getattr(obj, f.name)) for f in fields(obj)}
            if isinstance(obj, Mapping):
                return {encode(key): encode(value) for key, value in obj.items()}
            if isinstance(obj, Iterable) and not isinstance(obj, (str, bytes)):
                return [encode(item) for item in obj]
            if isinstance(obj, datetime.datetime):
                return obj.isoformat()
            if isinstance(obj, float):
                return round(obj, 3)  # Round floats to 3 decimal places for better readability.
            return obj

        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(encode(self), indent=2, default=repr))
        log.info("Test run summary written to %s", path)

    @classmethod
    def from_json(cls, path: Path) -> RunSummary:
        """Read the test run summary from a JSON file."""
        raw = json.loads(path.read_text())
        ret = RunSummary(iterations=raw.get("iterations", 1), tests_per_iteration=raw.get("tests_per_iteration", 0))

        # Recover a timestamp.
        timestamp = raw.get("run_timestamp", "unknown")
        with contextlib.suppress(TypeError, ValueError):
            timestamp = datetime.datetime.fromisoformat(timestamp)
        ret.run_timestamp = timestamp

        # We use only contents of "results" to reconstruct the summary, as all the statistics can be recalculated from the results.
        for result in raw.get("results", []):
            try:
                ret.record(TestResult(**result))
            except Exception as e:
                log.warning("Skipping result record %r due to an exception while parsing JSON file %s: %r", result, path, e)
        return ret

    @staticmethod
    def _print_table(rows: Iterable[tuple[str, ...]], title: str = "", headers_fmt: tuple[tuple[str, str], ...] | None = None,
                     top_btm_rule: str | None = None, mid_rule: str | None = "-", col_sep: str = "  ", content_padding: int = 2,
                     rule_padding: int = 2, last_col_max_width: int | None = None, no_content_msg: str | None = None) -> None:
        """Print a table to the console.

        Arguments:
            rows -- iterable of rows to print, where each row is a tuple of cell values (as strings).

        Keyword Arguments:
            title -- optional table title surrounded by top_btm_rules.
            headers_fmt -- optional tuple of header title and format pairs.
            top_btm_rule -- optional top and bottom rule character or an explicit string.
            mid_rule -- optional between header and content rule character or an explicit string.
            col_sep -- column separator.
            content_padding -- number of spaces before and after row content on the outer border of the table.
            rule_padding -- number of spaces before and after rules on the outer border of the table.
            last_col_max_width -- optional constraint for the last column. It doesn't truncate the data, but only sets format.
            no_content_msg -- optional message to show if there are no rows. Raise ValueError if not present and there are no rows.
        """
        # Filter out empty rows, and check if there are any rows defined.
        content_padding_str = " " * content_padding
        if not (rows := tuple(row for row in rows if any(cell for cell in row))):
            if no_content_msg is not None:
                print()
                print(f"{content_padding_str}{no_content_msg}")
                return
            raise ValueError("There should be at least one row in the table")

        # If headers are not defined, prepare default alignment. Otherwise, unpack to headers and formats tuples.
        headers, fmt = zip(*((("", "<") for _ in rows[0]) if headers_fmt is None else headers_fmt))

        # Check if number of columns is consistent for all rows.
        all_rows = (headers,) + rows
        if len({len(row) for row in all_rows}) != 1:
            raise ValueError("All rows must have the same number of columns")

        # Calculate widths of columns, the table and rules.
        col_widths = [max(len(row[i]) for row in all_rows) for i in range(len(rows[0]))]
        if last_col_max_width is not None:
            col_widths[-1] = min(col_widths[-1], last_col_max_width)
        total_width = 2 * content_padding + max(len(title), sum(col_widths) + len(col_sep)*(len(col_widths) - 1))
        rule_width = total_width - 2 * rule_padding

        # Prepare formatting strings.
        rule_pad_str = " " * rule_padding
        top_btm_rule_str = f"{rule_pad_str}{top_btm_rule*rule_width}" if top_btm_rule and len(top_btm_rule) == 1 else top_btm_rule
        mid_rule_str = f"{rule_pad_str}{mid_rule*rule_width}" if mid_rule and len(mid_rule) == 1 else mid_rule
        row_format = content_padding_str + col_sep.join(f"{{:{fmt}{width}}}" for fmt, width in zip(fmt, col_widths))

        # Print the table.
        print()
        if top_btm_rule_str:
            print(top_btm_rule_str)
        if title:
            print(f"{content_padding_str}{title}")
            if top_btm_rule_str:
                print(top_btm_rule_str)
        if headers_fmt:
            print(row_format.format(*headers))
            if mid_rule_str:
                print(mid_rule_str)
        for row in rows:
            print(row_format.format(*row))
        if top_btm_rule_str:
            print(top_btm_rule_str)

    def print_summary(self, show_failed: bool = True, show_flaky: bool = True, top_slowest: int = 20,
                      show_all: bool = True) -> None:
        """Print summary of test run.

        Keyword Arguments:
            show_failed -- separately list failed tests.
            show_flaky -- if running multiple iterations, list tests with the highest failure rate across iterations.
            top_slowest -- list the slowest tests. If 0 don't list any, if negative list all tests.
            show_all -- list stats for all tests across all iterations, including mean duration and status message (passed, failed
                        with exception, or cancelled).
        """
        self._print_table(title="TEST RUN SUMMARY", top_btm_rule="=", col_sep=" : ", rule_padding=0,
                          rows=(("Run timestamp", str(self.run_timestamp)),
                                ("Iterations", str(self.iterations)),
                                ("Total runs", str(self.total_runs)),
                                ("Passed", str(self.passed)),
                                ("Failed", str(self.failed)),
                                ("Cancelled", str(self.cancelled)),
                                (("Pass rate", f"{100 * self.pass_rate:.1f}%") if self.total_runs else ())))

        if show_failed:
            failed_results = tuple(r for r in self.results if r.status == TestStatus.FAILED)
            self._print_table(title=f"FAILED TESTS ({len(failed_results)}):",
                              no_content_msg="No failures recorded",
                              headers_fmt=(("Test name", "<"), ("Iter", ">"), ("Duration", ">")),
                              rows=((r.name_decorated, str(r.iteration), f"{r.duration_seconds:.2f}s")
                              for r in sorted(failed_results, key=lambda x: x.name)))

        if show_flaky and self.iterations > 1:
            flaky = tuple((name, stats) for name, stats in self.test_stats.items() if stats.failed > 0)
            self._print_table(title=f"FAILURE RATE BY TEST (across {self.iterations} iterations)",
                              no_content_msg="No flaky results",
                              headers_fmt=(("Test name", "<"), ("Failures", ">"), ("Rate", ">")),
                              rows=((name, f"{stats.failed}/{stats.total_runs:<2}", f"{100 * stats.fail_rate:.1f}%")
                              for name, stats in sorted(flaky, key=lambda item: -item[1].failed)))

        if top_slowest:
            slowest = sorted((r for r in self.results if r.status not in (TestStatus.DRY_RUN, TestStatus.CANCELLED)),
                             key=lambda x: -x.duration_seconds)

            # Slice only the top slowest tests if the limit is set to a positive value. If it's negative, show all tests.
            if top_slowest > 0:
                slowest = slowest[:top_slowest]

            self._print_table(title=f"SLOWEST {len(slowest)} TEST RUNS:", no_content_msg="No tests to show for slowest list",
                              headers_fmt=(("Test name", "<"), ("Status", "<"), ("Iter", ">"), ("Duration", ">")),
                              rows=((r.name_decorated, r.status, str(r.iteration), f"{r.duration_seconds:.2f}s")
                              for r in slowest))

        if show_all:
            self._print_table(title="STATS OF ALL TESTS:", no_content_msg="No tests to show", last_col_max_width=20,
                              headers_fmt=(("Test name", "<"), ("Passed", ">"), ("Mean time", ">"), ("Status", "<")),
                              rows=((name, f"{stats.passed}/{stats.total_runs}", f"{stats.mean_duration:.2f}s", stats.status_msg)
                                    for name, stats in self.test_stats.items()))

        # Final vertical padding.
        print()


class ResultError(Exception):
    """Exception raised when processing results."""


ResultQueueT: TypeAlias = CancellableQueue[TestResult]


@dataclass(eq=False)
class ResultProcessingThread(threading.Thread):
    """Thread that processes test results from the result queue, keeps track of test run summary and prints it at the end."""

    summary: RunSummary
    expected_failures: int
    keep_going: bool
    summary_file: Path | None

    THREAD_TERMINATE_TIMEOUT_S: ClassVar[float] = 5.0

    def __post_init__(self) -> None:
        super().__init__(name="Results")

        self.result_queue: ResultQueueT = CancellableQueue()
        self.exception: BaseException | None = None

    def run(self) -> None:
        try:
            log.debug("Starting result processing thread")
            while True:
                self._process_result(self.result_queue.get())
        except EndOfQueue:
            log.debug("No more results to process, finishing result processing thread")
        except BaseException as e:
            self.exception = e
        finally:
            log.debug("Result processing thread finished")

    def _process_result(self, result: TestResult) -> None:
        iteration = result.iteration
        with self.summary:
            self.summary.record(result)

            # Check for keep going on failure.
            if result.exception is not None and not isinstance(result.exception, KeyboardInterrupt) and not self.keep_going:
                raise ResultError("Test failed and --keep-going flag is not set.")

            # Check if all results for the iteration are in.
            if len(self.summary.exceptions[iteration]) < self.summary.tests_per_iteration:
                return

            log.debug("All results for iteration %i are in, checking failure count", iteration)
            observed_failures = sum(exc is not None and not isinstance(exc, KeyboardInterrupt)
                                    for exc in self.summary.exceptions[iteration].values())
            if observed_failures != self.expected_failures:
                raise ResultError(
                    f"Iteration {iteration}: expected failure count {self.expected_failures}, but got {observed_failures}")

    def terminate(self) -> None:
        """Terminate the result processing thread."""
        try:
            # Close the result queue to unblock the thread if it's waiting for results.
            self.result_queue.close()

            if self.ident is not None:
                self.join(self.THREAD_TERMINATE_TIMEOUT_S)
                if self.is_alive():
                    raise RuntimeError("Result processing thread is still alive, it might be stuck on processing results")
        except Exception as e:
            # Try to forcefully cancel the result queue to unblock the thread.
            self.result_queue.cancel()

            # Wait for the thread to finish processing results if it had been started.
            if self.ident is not None:
                self.join(self.THREAD_TERMINATE_TIMEOUT_S)
                if self.is_alive():
                    raise RuntimeError(
                        "Failed to terminate result processing thread. Result summary may be incomplete or corrupted") from e
        finally:
            # We don't take the lock to ensure there is no deadlock in case of the thread being stuck on acquiring the lock. This
            # may lead to incomplete or corrupted summary, but it's better than hanging indefinitely.
            self.summary.print_summary(show_failed=True, show_flaky=False, top_slowest=0, show_all=True)
            if self.summary_file is not None:
                self.summary.write_json(self.summary_file)
