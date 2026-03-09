import logging
from abc import ABC
from collections.abc import Callable
from concurrent.futures import ThreadPoolExecutor, as_completed
from multiprocessing.context import SpawnContext
from multiprocessing.managers import SyncManager
from typing import Generic, Literal, TypeVar

from chiptest.mp_utils.process import ProcessConfigTemplate, ProcessState, WrappedProcess
from chiptest.mp_utils.queue import WorkQueue
from chiptest.mp_utils.state import ProcessGroupState

log = logging.getLogger(__name__)


WrappedProcessT = TypeVar("WrappedProcessT", bound=WrappedProcess)
ConfigT = TypeVar("ConfigT", bound=ProcessConfigTemplate)
WorkRequestT = TypeVar("WorkRequestT")
WorkResponseT = TypeVar("WorkResponseT")

class WrappedProcessPool(ABC, Generic[WrappedProcessT, ConfigT, WorkRequestT, WorkResponseT]):
    def __init__(self, process_cls: type[WrappedProcessT], mp_context: SpawnContext, mp_manager: SyncManager, concurrency: int,
                 name: str, config_template: ConfigT) -> None:
        self._name = name
        self.config_template = config_template
        self.state = ProcessGroupState(mp_manager)
        self.work_queue: WorkQueue[WorkRequestT, WorkResponseT] = WorkQueue(mp_manager, concurrency)
        self._pool = tuple(process_cls(mp_context, mp_manager, config_template.with_formatted_name(id), self.work_queue, self.state)
                           for id in range(concurrency))

    @property
    def name(self) -> str:
        return self._name

    def collect_exceptions(self) -> Literal[True]:
        return self.state.collect_exceptions()

    def _execute_for_all_workers(self, fn: Callable[[WrappedProcessT], None], exception_message: str, thread_name_prefix: str) -> None:
        exceptions: list[Exception] = []
        with ThreadPoolExecutor(max_workers=len(self._pool), thread_name_prefix=thread_name_prefix) as pool:
            for result in as_completed(pool.submit(lambda process: fn(process), process) for process in self._pool):
                try:
                    result.result()
                except Exception as e:
                    exceptions.append(e)
        if exceptions:
            raise ExceptionGroup(exception_message, exceptions)

    def start(self) -> None:
        log.info("Initializing %s with concurrency of %i", self.name, len(self._pool))
        try:
            self._execute_for_all_workers(lambda process: process.start(), f"Failed to start {self.name}", "WorkerStart")
        except BaseException:
            self.stop()
            raise

    def stop(self) -> None:
        # Cancel queues. For now, we want to cancel only request queue, as we want to process all results.
        self.state.process_ready_queue.cancel()
        self.work_queue.req_cancel()

        # Stop processes.
        try:
            if self.state.phase_min != ProcessState.Phase.CLOSED:
                self._execute_for_all_workers(lambda process: process.stop(), f"Exception when stopping {self.name}", "WorkerStop")
        finally:
            # Now, once all processes are stopped, is the time to close the response queue to stop the results processing thread.
            self.work_queue.rsp_close()
