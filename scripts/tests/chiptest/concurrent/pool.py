import itertools
import logging
import sys
from collections.abc import Callable, Iterable
from multiprocessing.context import SpawnContext
from multiprocessing.managers import SyncManager
from typing import Literal

from chiptest.concurrent.config import TestJobConfig, TestSchedulerType, WorkerConfig
from chiptest.concurrent.results import ResultProcessingThread
from chiptest.concurrent.status import PeriodicStatusThread
from chiptest.concurrent.worker import WorkerError, WorkerJob, WorkerProcessCls, WorkerResult
from chiptest.log_utils import LogConfig
from chiptest.mp_utils.common import StartStopContextMixin, mp_wrapped_spawn_context
from chiptest.mp_utils.pool import WrappedProcessPool
from chiptest.mp_utils.process import ProcessState, WorkQueueCancelled
from chiptest.test_definition import TestDefinition

log = logging.getLogger(__name__)


if sys.platform == "linux":
    # We have a private /run as we're running in unshare, so we can place it in any place under /run. We don't want it in /tmp, as
    # we remount it to worker-specific scratchpad.
    SYNC_MANAGER_PATH = "/run/python_pool_manager.sock"
else:
    # Other platforms will fall back to their default.
    SYNC_MANAGER_PATH = None

SchedulerFunc = Callable[[Iterable[TestDefinition]], Iterable[tuple[int, TestDefinition]]]
"""Function type for scheduling tests to workers.

It takes an iterable of tests and returns an iterable of worker id and test pairs.
"""


class TestPool(WrappedProcessPool[WorkerProcessCls, WorkerConfig, WorkerJob, WorkerResult], StartStopContextMixin):
    def __init__(self, mp_context: SpawnContext, mp_manager: SyncManager, log_config: LogConfig, config: TestJobConfig,
                 tests_per_iteration: int) -> None:
        super().__init__(WorkerProcessCls, mp_context, mp_manager, config.concurrency, "Test Pool",
                         WorkerConfig.from_test_job_config(log_config, config))

        self._results_thread = ResultProcessingThread(self.config_template, self.work_queue, tests_per_iteration)
        self._status_thread = PeriodicStatusThread(self.config_template, self.work_queue, self._results_thread, self.state)

        self.scheduler: SchedulerFunc
        match config.concurrenct_scheduler:
            case TestSchedulerType.FAST:
                self.scheduler = self._scheduler_fast
            case TestSchedulerType.REPRODUCIBLE:
                self.scheduler = self._scheduler_reproducible
            case _:
                raise ValueError(f"Unknown scheduler type: {self.config.concurrenct_scheduler}")

    def collect_exceptions(self) -> Literal[True]:
        super().collect_exceptions()

        if self._results_thread.exception is not None:
            raise self._results_thread.exception

        return True

    def start(self) -> None:
        self._results_thread.start()
        super().start()
        self._status_thread.start()

    def stop(self) -> None:
        super().stop()

        # Status thread should finish on cancel invoked by the pool stop.
        if self._status_thread.is_alive():
            self._status_thread.join(self.config_template.stop_timeout)
        if self._status_thread.is_alive():
            log.error("Status thread failed to stop")

        # Results thread should finish on cancel invoked by the pool stop.
        log.debug("Waiting for result processing thread")
        if self._results_thread.is_alive():
            self._results_thread.join(self.config_template.stop_timeout)
        if self._results_thread.is_alive():
            log.error("Results processing thread failed to stop")

        log.debug("Finalized worker pool")
        self._results_thread.print_summary()

    def _scheduler_fast(self, tests: Iterable[TestDefinition]) -> Iterable[tuple[int, TestDefinition]]:
        for test in tests:
            worker_id = self.state.process_ready_queue.get_or_cancel()
            self.collect_exceptions()
            yield worker_id, test

    def _scheduler_reproducible(self, tests: Iterable[TestDefinition]) -> Iterable[tuple[int, TestDefinition]]:
        for worker_id, test in zip(itertools.cycle(range(len(self._pool))), tests):
            yield worker_id, test

        self.collect_exceptions()

    @classmethod
    def run_tests(cls, log_config: LogConfig, config: TestJobConfig, tests: list[TestDefinition]) -> None:
        with (SyncManager(address=SYNC_MANAGER_PATH) as mp_manager,
              mp_wrapped_spawn_context(wrapper_linux="unshare --map-root-user -n -m") as mp_ctx,
              cls(mp_ctx, mp_manager, log_config, config, len(tests)) as pool):
            try:
                for i in range(1, config.iterations+1):
                    log.info("Scheduling iteration %d", i)
                    for worker_id, test in pool.scheduler(tests):
                        log.debug("Enqueuing test %s to worker %i", test.name, worker_id)
                        pool.work_queue.req_put(WorkerJob(i, test), worker_id)

                    # If this is the last iteration schedule finalization event.
                    if i == config.iterations:
                        pool.work_queue.req_close()

                log.info("All jobs scheduled")
                pool.state.wait_for(
                    lambda states: pool.collect_exceptions() and all(state.phase == ProcessState.Phase.CLOSED for state in states))
            except WorkQueueCancelled:
                log.debug("Received a cancel event on a work queue")
            except WorkerError as e:
                log.error("%s", e)
