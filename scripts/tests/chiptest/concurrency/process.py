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

import dataclasses
import enum
import functools
import logging
import multiprocessing
import os
import signal
import threading
import traceback
from abc import ABC, abstractmethod
from collections.abc import Callable
from multiprocessing.context import SpawnContext
from multiprocessing.managers import SyncManager, ValueProxy
from types import TracebackType
from typing import ClassVar, Concatenate, Generic, ParamSpec, Protocol, Self, TypeVar

from chiptest.concurrency.work_queue import CancellableQueue, EndOfQueue, QueueCancelled
from chiptest.log_config import LogConfig

log = logging.getLogger(__name__)


class WithName(Protocol):
    @property
    def name(self) -> str: ...


S = TypeVar("S", bound="WithName")
P = ParamSpec("P")
R = TypeVar("R")


def with_annotated_exception(fn: Callable[Concatenate[S, P], R]) -> Callable[Concatenate[S, P], R]:
    """Decorator to enrich exceptions from thread and process methods with their names for easier debugging."""

    @functools.wraps(fn)
    def wrapper(self: S, *args: P.args, **kwargs: P.kwargs) -> R:
        try:
            return fn(self, *args, **kwargs)
        except BaseException as e:
            kind = ("thread" if isinstance(self, threading.Thread) else
                    "process" if isinstance(self, (multiprocessing.Process, WrappedProcess)) else
                    self.__class__.__name__)

            e.add_note(f"Exception in {kind} {self.name}")
            raise

    return wrapper


@dataclasses.dataclass
class ProcessConfig:
    """Process configuration template."""

    DEFAULT_START_TIMEOUT_SEC: ClassVar[float] = 4.0
    DEFAULT_STOP_TIMEOUT_SEC: ClassVar[float] = 4.0
    DEFAULT_TERMINATION_TIMEOUT_SEC: ClassVar[float] = 2.0

    id: int = 0
    name: str = "Process{id}"
    log_config: LogConfig = dataclasses.field(default_factory=LogConfig)
    start_timeout_sec: float = DEFAULT_START_TIMEOUT_SEC
    stop_timeout_sec: float = DEFAULT_STOP_TIMEOUT_SEC
    termination_timeout_sec: float = DEFAULT_TERMINATION_TIMEOUT_SEC

    def with_formatted_name(self) -> Self:
        """Format the name using the id and process hierarchy. Return a new instance with the formatted name."""
        name = self.name.format(id=self.id)

        # Indicate process hierarchy in the name so that it's easier to understand the dependencies in the logs.
        if (parent_proc_name := multiprocessing.current_process().name) != "MainProcess":
            name = f"{parent_proc_name}/{name}"

        return dataclasses.replace(self, name=name)


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

    def __init__(self, mp_manager: SyncManager, config: ProcessConfig) -> None:
        self._config = config
        self._phase: ValueProxy[ProcessPhase] = mp_manager.Value(object, ProcessPhase.NOT_STARTED)
        self._exception: ValueProxy[BaseException | None] = mp_manager.Value(object, None)
        self._state_changed = mp_manager.Condition()

    @property
    def phase(self) -> ProcessPhase:
        return self._phase.get()

    @phase.setter
    def phase(self, value: ProcessPhase) -> None:
        with self._state_changed:
            self._phase.set(value)
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
                if (tb := value.__traceback__) is not None:
                    value.add_note("".join(traceback.format_tb(tb)))
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
        """
        with self._state_changed:
            return self._state_changed.wait_for(lambda: predicate(self._phase.get(), self._exception.get()), timeout)


WorkRequestT = TypeVar("WorkRequestT")
WorkResponseT = TypeVar("WorkResponseT")


class WrappedProcess(ABC, Generic[WorkRequestT, WorkResponseT]):
    """
    Base class for wrapped Python subprocesses.

    WrappedProcess owns a `multiprocessing.Process` and coordinates its lifecycle through shared `ProcessState` and work/response
    queues.

    Intended wrapping flow:
    1. The caller enters `mp_wrapped_spawn_context()` from `concurrent/context.py`.
    2. The context always uses the spawn start method, and on Linux can optionally install a temporary wrapper executable that
       launches Python through `wrapper_linux`.
    3. That `SpawnContext` is passed to WrappedProcess and used to create the child process.
    4. Parent-side lifecycle is driven by `start()` / `stop()`.
    5. Child-side lifecycle is driven by `run()` and the `_proc_*` hook methods.
    6. On context exit, `mp_wrapped_spawn_context()` restores the original multiprocessing executable and removes the wrapper.

    The lifecycle of the subprocess is coordinated through `ProcessState` and follows these phases:
    1. Parent calls `start()`.
    2. Subprocess enters `run()` and sets phase `UNINITIALIZED`.
    3. `_proc_init()` is invoked once for subclass-specific initialization.
    4. On success, phase becomes `READY`.
    5. `_proc_work()` executes the main loop (by default waits for cancellation). Implementations may optionally toggle between
       `READY` and `WORKING` to expose idle vs active periods.
    6. `_proc_cleanup()` is always called from the finally block.
    7. Phase becomes `CLOSED` when the subprocess exits.

    Error handling behavior:
    - Unhandled exceptions from subprocess code are stored in `state.exception`.
    - `start()` waits for initialization and fails if the process reports an exception, closes early, or times out.
    - `stop()` attempts graceful shutdown first, then escalates with SIGINT, SIGTERM, and SIGKILL with configured timeouts.

    Subclasses are expected to implement `_proc_init()` and `_proc_cleanup()`, and may optionally override `_proc_work()`.
    """
    # Methods run in the parent process.

    def __init__(self, mp_context: SpawnContext, mp_manager: SyncManager, config: ProcessConfig,
                 work_queue: CancellableQueue[WorkRequestT], rsp_queue: CancellableQueue[WorkResponseT]) -> None:
        # Neither mp_context or mp_manager should be saved as fields, as they are not picklable between processes. They can be used
        # to initialize some shared resources in the constructor.

        self._config = config
        self._work_queue = work_queue
        self._rsp_queue = rsp_queue
        self.state = ProcessState(mp_manager, config)

        # Create multiprocessing.Process in the given context.
        self._proc = mp_context.Process(target=self.run, name=self._config.name)
        self._stopped = False

    def __getstate__(self) -> dict[str, object]:
        state = self.__dict__.copy()
        # Drop _proc field to exclude it from pickling on process spawn.
        state.pop("_proc", None)
        return state

    @property
    def name(self) -> str:
        return self._config.name

    @with_annotated_exception
    def start(self) -> None:
        """
        Start the subprocess and wait until initialization completes.

        After triggering the process start, wait until phase leaves `NOT_STARTED`/`UNINITIALIZED` and then validate final startup
        state: `READY` is success, and `CLOSED` or any recorded exception is failure.
        """
        if self._stopped:
            raise RuntimeError("Cannot start a process that has been already stopped")

        log.debug("Starting process %s", self.name)
        self._proc.start()

        try:
            if not self.state.wait_for(lambda phase, _: phase not in (ProcessPhase.NOT_STARTED, ProcessPhase.UNINITIALIZED),
                                       self._config.start_timeout_sec):
                raise TimeoutError("Timeout when waiting for initialization")

            with self.state:
                # Propagate KeyboardInterrupt as is.
                if isinstance(self.state.exception, KeyboardInterrupt):
                    raise self.state.exception

                if self.state.exception is not None:
                    raise RuntimeError("Process failed initialization") from self.state.exception

                match self.state.phase:
                    case ProcessPhase.READY:
                        log.debug("Process %s started successfully", self.name)
                    case ProcessPhase.CLOSED:
                        raise RuntimeError("Process closed immediately after initialization")
                    case _:
                        raise RuntimeError(f"Process is in an unexpected state: {self.state.phase!r}")
        except BaseException as start_exc:
            if not isinstance(start_exc, KeyboardInterrupt):
                log.error("Stopping process %s on failure during initialization", self.name)
            self.stop(raise_on_proc_error=False)
            raise

    def has_stopped(self, timeout: float) -> bool:
        self._proc.join(timeout)
        if not self._proc.is_alive():
            log.debug("Process %s got stopped", self.name)
            return True
        return False

    @with_annotated_exception
    def stop(self, raise_on_proc_error: bool = True) -> None:
        """
        Stop the subprocess with escalating termination signals.

        The method waits for a graceful exit first (typically triggered when the work queue is cancelled by its owner). If still
        alive, it escalates to SIGINT, then SIGTERM, then SIGKILL.
        """
        if self._stopped:
            log.debug("Process %s is already stopped", self.name)
            return

        try:
            # First check if the process is alive before trying to join, as it is an error to attempt to join a process before it
            # has been started.
            if not self._proc.is_alive():
                log.debug("Process %s hasn't been started yet or has been already stopped", self.name)
                return

            # Wait for the external work queue to be cancelled by its owner, which should signal the process to gracefully stop.
            if self.has_stopped(self._config.stop_timeout_sec):
                return

            log.debug("Sending interrupt signal to process %s", self.name)
            if self._proc.pid is not None:
                os.kill(self._proc.pid, signal.SIGINT)  # TODO Python 3.14: self._proc.interrupt()
            if self.has_stopped(self._config.stop_timeout_sec):
                return

            log.warning("Timeout when waiting for process %s to stop. Sending terminate signal", self.name)
            self._proc.terminate()
            if self.has_stopped(self._config.termination_timeout_sec):
                return

            log.warning("Timeout when waiting for process %s to terminate. Sending kill signal", self.name)
            self._proc.kill()
            if self.has_stopped(self._config.termination_timeout_sec):
                return

            raise TimeoutError(f"Failed to terminate the process {self.name}. May become a zombie")
        finally:
            self._stopped = True
            if raise_on_proc_error and self.state.exception is not None:
                raise RuntimeError("Process reported an exception during execution") from self.state.exception

    # Methods run in the subprocess.

    def run(self) -> None:
        """
        Subprocess entrypoint coordinating init/work/cleanup and state.

        This method drives the lifecycle hooks in order: `_proc_init()` -> `_proc_work()` -> `_proc_cleanup()`. It updates
        `state.phase` and captures unexpected exceptions in `state.exception`.

        We use a nested try block in finally to ensure that errors from all abstract methods, including `_proc_cleanup()` are
        captured in `state.exception`.
        """
        try:
            # Signal that the process has started initialization.
            self.state.phase = ProcessPhase.UNINITIALIZED

            # Initialize global logger in the subprocess.
            self._config.log_config.set_fmt()

            # Initialize.
            log.debug("Initializing")
            self._proc_init()
            self.state.phase = ProcessPhase.READY
            log.debug("Initialized successfully")

            # Perform work.
            self._proc_work()
        except QueueCancelled:
            log.warning("Received a cancel event")
        except EndOfQueue:
            log.debug("Received end of work signal")
        except KeyboardInterrupt:
            log.debug("Caught an interrupt")
        except BaseException as e:
            log.error("Process failed with an exception: %r", e)
            self.state.exception = e
        finally:
            log.debug("Cleaning up")
            try:
                self._proc_cleanup()
                log.debug("Process finished cleanly")
            except BaseException as cleanup_exc:
                log.error("Cleanup failed: %r", cleanup_exc)
                op_exc = self.state.exception
                self.state.exception = cleanup_exc  # Save to the state to annotate and thus prepare for propagation to the parent.

                # In case of an exception during cleanup, we want to preserve the original exception if it exists.
                if op_exc is not None:
                    self.state.exception = BaseExceptionGroup("Failed to cleanup process after previous failure",
                                                              [op_exc, self.state.exception])
            finally:
                self.state.phase = ProcessPhase.CLOSED

    @abstractmethod
    def _proc_init(self):
        """Initialize subprocess resources before work begins.

        Allocate long-lived resources here and store them on `self` so they can be released in `_proc_cleanup()`.
        """

    def _proc_work(self) -> None:
        """
        Perform the work.

        Default behavior waits for cancellation. Override to consume `self._work_queue` and publish activity by toggling
        `state.phase` between `READY` and `WORKING` as appropriate.
        """
        self._work_queue.wait_for_cancelled()

    @abstractmethod
    def _proc_cleanup(self):
        """
        Release resources created by `_proc_init()`.

        This hook is always called, including after exceptions.
        """
