import logging
import threading
import time

from chiptest.concurrent.config import WorkerConfig
from chiptest.concurrent.results import ResultProcessingThread
from chiptest.concurrent.worker import WorkerJob, WorkerResult
from chiptest.mp_utils.queue import WorkQueue
from chiptest.mp_utils.state import ProcessGroupState

log = logging.getLogger(__name__)


class PeriodicStatusThread(threading.Thread):
    def __init__(self, config: WorkerConfig, work_queue: WorkQueue[WorkerJob, WorkerResult], results_thread: ResultProcessingThread,
                 pool_state: ProcessGroupState) -> None:
        super().__init__(name="Status")
        self.work_queue = work_queue
        self.config = config
        self.results_thread = results_thread
        self.pool_state = pool_state

        # Calculate periodicity of status overview thread.
        self.periodicity: float | None
        match config.concurrency_status:
            case num if num < 0:
                # "Automatic" periodicity. Could be improved with checking activity of logger.
                # TODO: Make it responsive to logger activity (every x lines + interval).
                self.periodicity = (0.5 if config.log_config.log_level_int <= logging.DEBUG else
                                    2 if config.concurrency > 4 else
                                    5 if config.concurrency > 1 else
                                    10)
            case 0:
                self.periodicity = None
            case num:
                self.periodicity = num

    def run(self) -> None:
        if self.periodicity is None:
            log.debug("Periodic status overview is disabled per configuration")
            return

        log.debug("Launching periodic status overview thread with periodicity of %0.2f seconds", self.periodicity)
        start = time.monotonic()
        while not self.work_queue.req_wait_for_cancel(max(0, self.periodicity - (time.monotonic() - start))):
            start += self.periodicity

            # Skip status overview if there is no activity, to avoid unnecessary logging, e.g., during initialization.
            if (utilization := self.pool_state.working_count) == 0:
                log.debug("Skipping status overview due to no worker activity")
                continue

            current_iteration = self.results_thread.current_iteration
            successful_tests, failed_tests = self.results_thread.successful_failed_tests

            test_status: list[str] = []
            if successful_tests > 0:
                test_status.append(f"{successful_tests} successful")
            if failed_tests > 0:
                test_status.append(f"{failed_tests} failed")
            if not test_status:
                test_status.append("no tests completed")

            worker_utilization = (f", worker utilization: {utilization}/{self.config.concurrency}"
                                  if self.config.concurrency > 1 else "")

            # Style: bold, blue
            log.info("\033[34;1mIteration %i/%i: %i/%i tests (%s)%s\033[0m", current_iteration, self.config.iterations,
                     successful_tests + failed_tests, self.results_thread.expected_test_count, ", ".join(test_status),
                     worker_utilization)

        log.debug("Status overview thread has stopped")
