from __future__ import annotations

import enum
import threading
from collections.abc import Callable, Iterable
from multiprocessing.managers import SyncManager, ValueProxy
from types import TracebackType

from .process import ProcessConfigTemplate
from .queue import RequestQueue


class ProcessGroupState:
    def __init__(self, mp_manager: SyncManager, *, process_ready_queue: bool) -> None:
        self._state_changed = mp_manager.Condition()
        self.process_ready_queue = RequestQueue[int](mp_manager, mp_manager.Event()) if process_ready_queue else None
        self._states: list[ProcessState] = []

    def register_process(self, state: ProcessState) -> threading.Condition:
        with self._state_changed:
            self._states.append(state)
            self._state_changed.notify_all()
        return self._state_changed

    @property
    def phase_min(self) -> ProcessState.Phase:
        return min((state.phase for state in self._states), default=ProcessState.Phase.NOT_STARTED)

    @property
    def phase_max(self) -> ProcessState.Phase:
        return max((state.phase for state in self._states), default=ProcessState.Phase.NOT_STARTED)

    def count_by_phase(self, phase: ProcessState.Phase) -> int:
        return sum(state.phase == phase for state in self._states)

    @property
    def working_count(self) -> int:
        return self.count_by_phase(ProcessState.Phase.WORKING)

    def wait_for(self, predicate: Callable[[Iterable[ProcessState]], bool], timeout: float | None = None):
        with self._state_changed:
            return self._state_changed.wait_for(lambda: predicate(self._states), timeout)

    def collect_exceptions(self) -> None:
        if (exceptions := tuple(state.exception for state in self._states if isinstance(state.exception, Exception))):
            raise ExceptionGroup("Caught exceptions in processes", exceptions)


class ProcessState:
    # TODO: Document behavior of the states and transitions between them, and how to use the state as a synchronization primitive.

    class Phase(enum.IntEnum):
        NOT_STARTED = enum.auto()
        UNINITIALIZED = enum.auto()
        READY = enum.auto()
        WORKING = enum.auto()
        CLOSED = enum.auto()

    def __init__(self, mp_manager: SyncManager, config: ProcessConfigTemplate, group_state: ProcessGroupState | None = None,
                 initial_state: Phase = Phase.NOT_STARTED) -> None:
        self._config = config
        self._group_state = group_state if group_state is not None else ProcessGroupState(mp_manager, process_ready_queue=False)
        self._phase: ValueProxy[ProcessState.Phase] = mp_manager.Value(object, initial_state)
        self._exception: ValueProxy[BaseException | None] = mp_manager.Value(object, None)

        self._state_changed = self._group_state.register_process(self)

    @property
    def phase(self) -> Phase:
        with self._state_changed:
            return self._phase.get()

    @phase.setter
    def phase(self, value: Phase) -> None:
        with self._state_changed:
            self._phase.set(value)
            if self._group_state.process_ready_queue is not None and value == ProcessState.Phase.READY:
                self._group_state.process_ready_queue.put(self._config.id)
            self._state_changed.notify_all()

    @property
    def exception(self) -> BaseException | None:
        return self._exception.get()

    @exception.setter
    def exception(self, value: BaseException | None) -> None:
        with self._state_changed:
            if value is not None:
                value.add_note(f"Exception in process {self._config.name_long} in phase {self.phase}")
            self._exception.set(value)
            self._state_changed.notify_all()

    def __enter__(self) -> bool:
        return self._state_changed.__enter__()

    def __exit__(self, exc_type: type[BaseException] | None, exc_val: BaseException | None, exc_tb: TracebackType | None):
        return self._state_changed.__exit__(exc_type, exc_val, exc_tb)

    def wait_for(self, predicate: Callable[[Phase], bool], timeout: float | None = None):
        with self._state_changed:
            return self._state_changed.wait_for(lambda: predicate(self._phase.get()), timeout)
