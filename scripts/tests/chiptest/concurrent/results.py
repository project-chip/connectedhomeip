import dataclasses
import logging
import threading

from chiptest.concurrent.config import WorkerConfig
from chiptest.concurrent.worker import WorkerError, WorkerJob, WorkerResult
from chiptest.mp_utils.process import WorkQueueCancelled
from chiptest.mp_utils.queue import WorkQueue

log = logging.getLogger(__name__)


@dataclasses.dataclass
class TestResultSummary:
    total: int = 0
    passed: int = 0
    runtime_sum: float = 0.0
    worker_ids: list[int] = dataclasses.field(default_factory=list)
    first_exception: BaseException | None = None

    @property
    def mean_runtime(self) -> float | None:
        return self.runtime_sum / self.total if self.total > 0 else None

    def update(self, result: WorkerResult) -> None:
        self.total += 1

        if result.exception is None:
            self.passed += 1
        elif self.first_exception is None:
            self.first_exception = result.exception

        if result.runtime is not None:
            self.runtime_sum += result.runtime

        if result.worker_id is not None:
            self.worker_ids.append(result.worker_id)

    def table_row(self) -> tuple[str, str, str, str]:
        return (f"{self.passed}/{self.total}",
                f"{mean_runtime:.2f}" if (mean_runtime := self.mean_runtime) is not None else "—",
                ", ".join(str(worker_id) for worker_id in self.worker_ids) if self.worker_ids else "—",
                f"❌ {self.first_exception!r}" if self.first_exception is not None else "✅")


class ResultProcessingThread(threading.Thread):
    def __init__(self, config: WorkerConfig, work_queue: WorkQueue[WorkerJob, WorkerResult], tests_per_iteration: int) -> None:
        super().__init__(name=self.__class__.__name__)
        self.config = config
        self.work_queue = work_queue
        self.test_per_iteration = tests_per_iteration

        # This will hold any exception raised during result processing, which can be re-raised in the main thread after joining.
        self.exception: Exception | None = None

        self._lock = threading.Lock()
        self._results: dict[int, list[WorkerResult]] = {}
        self._summaries: dict[str, TestResultSummary] = {}

    def run(self) -> None:
        try:
            while True:
                # Double check the result type. Mismatch can happen if the mp_manager is already shut down for some reason, which can
                # happen in some unclean cancellation cases.
                if not isinstance(result := self.work_queue.get_rsp_or_cancel(), WorkerResult):
                    log.warning("Wrong work result: %r", result)
                    continue

                self._process_result(result)
        except WorkQueueCancelled:
            log.debug("No more results to process, finishing result processing thread")
        except Exception as e:
            self.exception = e

    def _process_result(self, result: WorkerResult) -> None:
        iteration = result.job.iteration
        with self._lock:
            if iteration not in self._results:
                self._results[iteration] = []
            self._results[iteration].append(result)

            if result.job.test.name not in self._summaries:
                self._summaries[result.job.test.name] = TestResultSummary()
            self._summaries[result.job.test.name].update(result)

            if result.exception is not None and not self.config.keep_going:
                raise WorkerError("Task failed and --keep-going flag is not set.")

            if len(self._results[iteration]) == self.test_per_iteration:
                log.debug("All results for iteration %i are in, checking failure count", iteration)
                observed_failures = sum(r.exception is not None for r in self._results[iteration])
                if observed_failures != self.config.expected_failures:
                    raise WorkerError(
                        f"Iteration {iteration}: expected failure count {self.config.expected_failures}, "
                        f"but got {observed_failures}")

    @property
    def expected_test_count(self) -> int:
        return self.config.iterations * self.test_per_iteration

    @property
    def current_iteration(self) -> int:
        with self._lock:
            return len(self._results)

    @property
    def successful_failed_tests(self) -> tuple[int, int]:
        with self._lock:
            successful, failed = tuple(map(sum, zip(
                (0, 0),  # Needed to avoid empty input to map when there are no results yet.
                *((result.exception is None, result.exception is not None)
                  for results in self._results.values()
                  for result in results))))
            return successful, failed

    def print_summary(self) -> None:
        if not self._results:
            log.info("No test results available.")
            return

        with self._lock:
            table: tuple[tuple[str, ...], ...] = (("Test", "Passed", "Time", "Worker ID", "Status"),) + tuple(
                (name, *summary.table_row()) for name, summary in sorted(self._summaries.items()))

        # Remove workers column if only one worker is used.
        if self.config.concurrency == 1:
            table = tuple(row[:3] + row[4:] for row in table)

        # Compute max width for each column from headers and rows. Skip the last column, as it can be quite wide for exceptions.
        widths = tuple(max(len(row[i]) for row in table) for i in range(len(table[0])-1))
        fmt = "  ".join(f"{{:{w}}}" for w in widths) + "  {}"

        # Print header, separator and rows.
        log.info(fmt.format(*table[0]))
        log.info("  ".join("-" * w for w in widths) + "  ------")
        for row in table[1:]:
            log.info(fmt.format(*row))

        # Print a note where to find /tmp for concurrent tests.
        # TODO: It's relevant only for Linux!
        if self.config.concurrency > 1:
            log.info("Note: You can find %s of each runner in %s/{Worker ID}", self.config.tmp_dir_default,
                     self.config.tmp_dir_worker_base)
        else:
            log.info("Note: You can find %s in %s/0", self.config.tmp_dir_default, self.config.tmp_dir_worker_base)

        # TODO: Check why the last test doesn't show up in the summary.
