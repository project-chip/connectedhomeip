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

import contextlib
import enum
import threading
import traceback
from collections.abc import Callable, Iterable, Iterator
from multiprocessing.managers import SyncManager, ValueProxy
from types import TracebackType
from typing import Literal, Protocol

from chiptest.concurrency.config import ProcessConfig
from chiptest.concurrency.work_queue import CancellableQueue, wait_for_mp_managed


class ProcessPhase(enum.IntEnum):
    """Phase of the process lifecycle."""

    NOT_STARTED = enum.auto()
    """The process has not started yet."""

    UNINITIALIZED = enum.auto()
    """The process has started but has not finished initialization yet."""

    READY = enum.auto()
    """The process has finished initialization and is ready to work. Can cycle between READY and WORKING phases multiple times."""

    WORKING = enum.auto()
    """The process is working on a task. Optional and can be used to distinguish between idle and busy processes."""

    CLOSED = enum.auto()
    """The process has finished all work and is closed (exited). Mind that it could have been closed due to an exception."""


class ProcessState:
    """
    Shared process state for synchronized lifecycle coordination and error propagation between the parent and child processes.

    Can be used as a context manager to synchronize access to the state. Needed only if accessing multiple fields atomically,
    otherwise individual fields are synchronized internally.
    """

    def __init__(self, mp_manager: SyncManager, config: ProcessConfig, group_state: ProcessGroupState | None = None) -> None:
        self._config = config
        self._phase: ValueProxy[ProcessPhase] = mp_manager.Value(object, ProcessPhase.NOT_STARTED)
        self._exception: ValueProxy[BaseException | None] = mp_manager.Value(object, None)

        # Keep only manager-backed synchronization primitives in this object. ProcessGroupState itself contains parent-only holders
        # (e.g. result processing thread) that are not picklable and must not be captured by spawned worker payloads.
        self._process_ready_queue: CancellableQueue[int] | None
        if group_state is None:
            self._state_changed = mp_manager.Condition()
            self._process_ready_queue = None
        else:
            self._state_changed = group_state.register_process_state(self)
            self._process_ready_queue = group_state.process_ready_queue

    @property
    def phase(self) -> ProcessPhase:
        return self._phase.get()

    @phase.setter
    def phase(self, value: ProcessPhase) -> None:
        with self._state_changed:
            self._phase.set(value)
            if self._process_ready_queue is not None and value == ProcessPhase.READY:
                self._process_ready_queue.put(self._config.process_id)
            self._state_changed.notify_all()

    @property
    def exception(self) -> BaseException | None:
        return self._exception.get()

    @exception.setter
    def exception(self, value: BaseException | None) -> None:
        with self._state_changed:
            if isinstance(value, BaseException):
                value.add_note(f"Exception in process {self._config.name}")

                # Traceback is not automatically propagated across process boundaries, so we need to add it to the exception
                # manually as a note.
                value.add_note("".join(traceback.format_exception(value)))
            self._exception.set(value)
            self._state_changed.notify_all()

    def __enter__(self) -> bool:
        return self._state_changed.__enter__()

    def __exit__(self, exc_type: type[BaseException] | None, exc_val: BaseException | None, exc_tb: TracebackType | None):
        return self._state_changed.__exit__(exc_type, exc_val, exc_tb)

    def wait_for(self, predicate: Callable[[ProcessPhase, BaseException | None], bool], timeout: float | None = None):
        """
        Wait until the predicate returns True for the current phase and/or exception.

        Timeout as for `Condition.wait_for()`, i.e. it can be a positive float for maximum wait time, or None to wait indefinitely.

        Returns the result of the predicate.

        We need the wait_for_mp_managed wrapper because otherwise we wouldn't be able to catch a KeyboardInterrupt for the condition
        which is managed by multiprocessing.Manager, as the manager process explicitly ignores SIGINT.
        """
        with self._state_changed:
            return wait_for_mp_managed(self._state_changed, lambda: predicate(self._phase.get(), self._exception.get()), timeout)

    @contextlib.contextmanager
    def working_context(self) -> Iterator[None]:
        """Context manager to set the process phase to WORKING for the duration of the context."""
        self.phase = ProcessPhase.WORKING
        try:
            yield
        finally:
            self.phase = ProcessPhase.READY


class WithException(Protocol):
    """Protocol for objects that hold an exception."""

    @property
    def exception(self) -> BaseException | None:
        ...


class ProcessGroupState:
    """State of a group of processes.

    It is used for synchronized lifecycle coordination and error propagation between the parent and child processes.
    """

    def __init__(self, mp_manager: SyncManager) -> None:
        self._state_changed = mp_manager.Condition()
        self._states: list[ProcessState] = []
        self._exception_holders: list[WithException] = []
        self.process_ready_queue: CancellableQueue[int] = CancellableQueue(mp_manager)

    def register_process_state(self, state: ProcessState) -> threading.Condition:
        """Register a process state as part of the process group.

        Used to notify the parent process when any of the child processes change their state. The returned condition needs to be
        used by the child process to notify the parent process when its state changes.
        """
        with self._state_changed:
            self._states.append(state)
            self._exception_holders.append(state)
            self._state_changed.notify_all()
        return self._state_changed

    def register_exception_holder(self, holder: WithException) -> threading.Condition:
        """Register an object which has an exception.

        It can be, e.g., a thread which can generate an exception. The returned condition needs to be used by the object to notify
        the parent process when its exception changes.
        """
        with self._state_changed:
            self._exception_holders.append(holder)
            self._state_changed.notify_all()
        return self._state_changed

    def __len__(self) -> int:
        return len(self._states)

    @property
    def phase_min(self) -> ProcessPhase:
        """Return the minimum phase of all registered subprocesses, or NOT_STARTED if there are no registered processes."""
        return min((state.phase for state in self._states), default=ProcessPhase.NOT_STARTED)

    @property
    def phase_max(self) -> ProcessPhase:
        """Return the maximum phase of all registered subprocesses, or NOT_STARTED if there are no registered processes."""
        return max((state.phase for state in self._states), default=ProcessPhase.NOT_STARTED)

    def count_by_phase(self, phase: ProcessPhase) -> int:
        """Return the number of registered subprocesses in the given phase."""
        return sum(state.phase == phase for state in self._states)

    @property
    def working_count(self) -> int:
        """Return the number of registered subprocesses in the WORKING phase."""
        return self.count_by_phase(ProcessPhase.WORKING)

    def wait_for(self, predicate: Callable[[Iterable[ProcessState]], bool], timeout: float | None = None):
        """Wait for the given state predicate to become True, with an optional timeout."""
        with self._state_changed:
            return self._state_changed.wait_for(lambda: predicate(self._states), timeout)

    def collect_exceptions(self) -> Literal[True]:
        """Collect exceptions from all registered exception holders.

        Raise them as an ExceptionGroup if there are any, or return True if there are no exceptions, which allows to use this method
        in a predicate for wait_for.
        """
        if (exceptions := tuple(ex_holder.exception
                                for ex_holder in self._exception_holders
                                if isinstance(ex_holder.exception, BaseException))):
            if len(exceptions) == 1:
                raise exceptions[0]
            raise BaseExceptionGroup("Caught exceptions in process group", exceptions)
        return True
