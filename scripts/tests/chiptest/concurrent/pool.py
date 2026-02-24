from collections.abc import Callable, Iterable
import itertools
import logging
from multiprocessing.context import SpawnContext
import sys
from multiprocessing.managers import SyncManager

from chiptest.mp_utils.common import StartStopContextMixin
from chiptest.log_utils import LogConfig
from chiptest.mp_utils.pool import WrappedProcessPool
from chiptest.mp_utils.process import ProcessState, mp_wrapped_spawn_context
from chiptest.test_definition import TestDefinition
from .worker import WorkerJob, WorkerResult, WorkerProcessCls
from .config import TestJobConfig, TestSchedulerType, WorkerConfig
from .results import ResultProcessingThread
from .status import PeriodicStatusThread

log = logging.getLogger(__name__)


if sys.platform == "linux":
    # We have a private /run as we're running in unshare, so we can place it in any place under /run. We don't want it in /tmp, as
    # we remount it to worker-specific scratchpad.
    SYNC_MANAGER_PATH = "/run/python_pool_manager.sock"
else:
    # Other platforms will fall back to their default.
    SYNC_MANAGER_PATH = None

SchedulerFunc = Callable[[int, Iterable[TestDefinition]], None]

class TestPool(WrappedProcessPool[WorkerProcessCls, WorkerConfig, WorkerJob, WorkerResult], StartStopContextMixin):
    def __init__(self, process_cls: type[WorkerProcessCls], mp_context: SpawnContext, mp_manager: SyncManager,
                 log_config: LogConfig, config: TestJobConfig, tests_per_iteration: int) -> None:
        super().__init__(process_cls, mp_context, mp_manager, config.concurrency, "Test Pool",
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

    def collect_exceptions(self) -> bool:
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
        self._status_thread.join(self.config_template.stop_timeout)
        if self._status_thread.is_alive():
            log.error("Status thread failed to stop")

        # Results thread should finish on cancel invoked by the pool stop.
        log.debug("Waiting for result processing thread")
        self._results_thread.join(self.config_template.stop_timeout)
        if self._results_thread.is_alive():
            log.error("Results processing thread failed to stop")

        log.debug("Finalized worker pool")
        self._results_thread.print_summary()

    def _scheduler_fast(self, iteration: int, tests: Iterable[TestDefinition]) -> None:
        if self.state.process_ready_queue is None:
            raise RuntimeError("Ready queue is not initialized in the pool state")

        for test in tests:
            worker_id = self.state.process_ready_queue.get_or_cancel()
            self.collect_exceptions()
            log.debug("Enqueuing test %s to worker %i", test.name, worker_id)
            self.work_queue.put_req(WorkerJob(iteration, test), worker_id)

    def _scheduler_reproducible(self, iteration: int, tests: Iterable[TestDefinition]) -> None:
        for test, worker_id in zip(tests, itertools.cycle(range(len(self._pool)))):
            log.debug("Enqueuing test %s to worker %i", test.name, worker_id)
            self.work_queue.put_req(WorkerJob(iteration, test), worker_id)

        self.collect_exceptions()

    @classmethod
    def run_tests(cls, log_config: LogConfig, config: TestJobConfig, tests: list[TestDefinition]) -> None:
        with (SyncManager(address=SYNC_MANAGER_PATH) as mp_manager,
              mp_wrapped_spawn_context(wrapper_linux="unshare --map-root-user -n -m") as mp_ctx,
              TestPool(WorkerProcessCls, mp_ctx, mp_manager, log_config, config, len(tests)) as pool):
            for i in range(config.iterations):
                log.info("Scheduling iteration %d", i+1)
                pool.scheduler(i, tests)

                # If this is the last iteration schedule finalization event.
                if i+1 == config.iterations:
                    pool.work_queue.finalize_req()

            log.info("All jobs scheduled")
            pool.state.wait_for(
                lambda states: pool.collect_exceptions() and all(state.phase == ProcessState.Phase.CLOSED for state in states))
