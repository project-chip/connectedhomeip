import logging
from abc import ABC
from collections.abc import Callable
from concurrent.futures import ThreadPoolExecutor, as_completed
from multiprocessing.context import SpawnContext
from multiprocessing.managers import SyncManager
from typing import Generic, TypeVar

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
        self.state = ProcessGroupState(mp_manager, process_ready_queue=True)
        self.work_queue: WorkQueue[WorkRequestT, WorkResponseT] = WorkQueue(mp_manager, concurrency)
        self._pool = tuple(process_cls(mp_context, mp_manager, config_template.with_formatted_name(id), self.work_queue,
                                       self.state)
                           for id in range(concurrency))

    @property
    def name(self) -> str:
        return self._name

    def collect_exceptions(self) -> bool:
        return self.state.collect_exceptions()

    def _execute_for_all_workers(self, fn: Callable[[WrappedProcessT], None], exception_message: str) -> None:
        exceptions: list[Exception] = []
        with ThreadPoolExecutor(max_workers=len(self._pool)) as pool:
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
            self._execute_for_all_workers(lambda process: process.start(), f"Failed to start {self.name}")
        except BaseException:
            self.stop()
            raise

    def stop(self) -> None:
        self.work_queue.cancel()

        if self.state.phase_min == ProcessState.Phase.CLOSED:
            return

        if self.state.process_ready_queue is not None:
            self.state.process_ready_queue.cancel()
        self._execute_for_all_workers(lambda process: process.stop(), f"Exception when stopping {self.name}")
