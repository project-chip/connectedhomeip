from abc import ABC
from collections.abc import Callable
import logging
from concurrent.futures import ThreadPoolExecutor, as_completed
from multiprocessing.context import SpawnContext
from multiprocessing.managers import SyncManager
from typing import Generic, TypeVar
from .process import ProcessConfigMixin, WrappedProcess, ProcessConfigTemplate, ProcessState
from .queue import WorkQueue

log = logging.getLogger(__name__)


WrappedProcessT = TypeVar("WrappedProcessT", bound=WrappedProcess)
ConfigT = TypeVar("ConfigT", bound=ProcessConfigMixin)
WorkRequestT = TypeVar("WorkRequestT")
WorkResponseT = TypeVar("WorkResponseT")

class WrappedProcessPool(ABC, Generic[WrappedProcessT, ConfigT, WorkRequestT, WorkResponseT]):
    def __init__(self, process_cls: type[WrappedProcessT], mp_context: SpawnContext, mp_manager: SyncManager, concurrency: int,
                 name: str, config_cls: type[ConfigT], config_template: ProcessConfigTemplate) -> None:
        self.name = name
        self.config_template = config_template
        self.state_changed = mp_manager.Condition()
        self.work_queue = WorkQueue[WorkRequestT, WorkResponseT](mp_manager, concurrency)
        self._pool = tuple(process_cls(mp_context, mp_manager, config_cls.from_template(id, config_template), self.work_queue,
                                       self.state_changed)
                           for id in range(concurrency))
        self._active = False

    def __len__(self) -> int:
        return len(self._pool)

    @property
    def state(self) -> tuple[ProcessState, ...]:
        return tuple(process.state for process in self._pool)

    @property
    def phase_min(self) -> ProcessState.Phase:
        return min((state.phase for state in self.state), default=ProcessState.Phase.NOT_STARTED)

    @property
    def phase_max(self) -> ProcessState.Phase:
        return max((state.phase for state in self.state), default=ProcessState.Phase.NOT_STARTED)

    def check_if_error(self, raise_error: bool = False) -> bool:
        if not (errors := tuple((id, state.exception) for id, state in enumerate(self.state) if state.exception is not None)):
            return False

        if raise_error:
            raise ExceptionGroup("Caught exceptions in processes",
                                 [exception for _, exception in errors if isinstance(exception, Exception)])
        return True

    def _execute_for_all_workers(self, fn: Callable[[WrappedProcessT], None], exception_message: str) -> None:
        exceptions: list[Exception] = []
        with ThreadPoolExecutor(max_workers=len(self)) as pool:
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
        if self.phase_min == ProcessState.Phase.CLOSED:
            return

        self._execute_for_all_workers(lambda process: process.stop(), f"Exception when stopping {self.name}")
        self._active = False
