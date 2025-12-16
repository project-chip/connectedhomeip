
import dataclasses
import itertools
import logging
import multiprocessing
import multiprocessing.spawn
import os
import signal
import stat
import sys
import tempfile
import threading
from abc import ABC, abstractmethod
from concurrent.futures import ThreadPoolExecutor, as_completed
from contextlib import contextmanager
from enum import Enum, auto
from multiprocessing.context import SpawnContext
from multiprocessing.managers import SyncManager, ValueProxy
from pathlib import Path
from types import TracebackType
from typing import Generic, Iterator, TypeVar, Literal
import time
from contextlib import suppress
import queue

from .log_utils import LogConfig

log = logging.getLogger(__name__)


@contextmanager
def mp_wrapped_spawn_context(wrapper: str, source_context: SpawnContext = multiprocessing.get_context("spawn")) -> Iterator[SpawnContext]:
    """Create platform-specific multiprocessing context.

    Linux:
    - We need to use spawn for the pool to have separate environment variables per runner, and to be able to use a wrapper script.
    - We need unshare wrapper script to have an option to mount per-worker /tmp.
    """
    if sys.platform != "linux":
        yield source_context
        return

    mp_wrapper_name: Path | None = None
    old_executable = multiprocessing.spawn.get_executable()
    executable = old_executable.decode('utf-8') if isinstance(old_executable, bytes) else old_executable
    try:
        with tempfile.NamedTemporaryFile("w", encoding="utf8", delete=False) as wrapper_file:
            mp_wrapper_name = Path(wrapper_file.name)
            wrapper_file.write(f'#!/bin/sh\nexec {wrapper} {executable} "$@"')
        mp_wrapper_name.chmod(mp_wrapper_name.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)

        source_context.set_executable(str(mp_wrapper_name))
        yield source_context
    finally:
        # Restore the default executable.
        source_context.set_executable(old_executable)
        if mp_wrapper_name is not None:
            mp_wrapper_name.unlink()


class ProcessState(Enum):
    NOT_STARTED = auto()
    UNINITIALIZED = auto()
    READY = auto()
    ERROR = auto()
    CLOSED = auto()


PROCESS_EXIT_STATES = (ProcessState.ERROR, ProcessState.CLOSED)


class WrappedProcess(ABC):
    DEFAULT_START_TIMEOUT = 2.0
    DEFAULT_STOP_TIMEOUT = 2.0

    def __init__(self, mp_context: SpawnContext, mp_manager: SyncManager, process_name_long: str,
                 process_name_short: str | None = None, log_config: LogConfig = LogConfig(),
                 state_changed: threading.Condition | None = None, cancel_event: threading.Event | None = None) -> None:
        # Neither mp_context or mp_manager should be saved in the instance, as they are not picklable between processes but they can
        # be used to initialize some shared resources.

        # Create state and cancel events.
        self.state_changed = mp_manager.Condition() if state_changed is None else state_changed
        self.state = mp_manager.Value(object, ProcessState.NOT_STARTED)
        self.state_exception: ValueProxy[BaseException | None] = mp_manager.Value(object, None)
        self.cancel_event = mp_manager.Event() if cancel_event is None else cancel_event

        # Create multiprocessing.Process in a given context.
        self.process_name_long = process_name_long
        self.process_name_short = process_name_short
        self._process = mp_context.Process(target=self.run, name=process_name_long)

        self.log_config = dataclasses.replace(log_config, process_name=process_name_short)

    def start(self, timeout: float | None = DEFAULT_START_TIMEOUT) -> None:
        log.debug("Starting process %s", self.process_name_long)
        self._process.start()

        # No start timeout means that the caller doesn't want to wait for initialization to finish.
        if timeout is None:
            return

        try:
            with self.state_changed:
                if not self.state_changed.wait_for(lambda: self.state.get() not in (ProcessState.NOT_STARTED,
                                                                                    ProcessState.UNINITIALIZED),
                                                   timeout):
                    raise TimeoutError("Timeout when waiting for initialization of %s", self.process_name_long)
                match self.state.get():
                    case ProcessState.READY:
                        log.debug("Process %s started successfully", self.process_name_long)
                        return
                    case ProcessState.ERROR:
                        if isinstance(exception := self.state_exception.get(), KeyboardInterrupt):
                            raise KeyboardInterrupt()
                        raise RuntimeError(
                            f"Process {self.process_name_long} failed initialization with exception: {exception}")
                    case ProcessState.CLOSED:
                        raise RuntimeError(f"Process {self.process_name_long} closed immediately after initialization")
                    case _:
                        raise RuntimeError(f"Process {self.process_name_long} in unknown state")
        except BaseException as e:
            if not isinstance(e, KeyboardInterrupt):
                log.info("Stopping the process on failure")
            self.stop()
            raise

    def stop(self, timeout: float = DEFAULT_STOP_TIMEOUT) -> None:
        for method in itertools.count():
            match method:
                case 0:
                    log.debug("Cancelling work in process %s", self.process_name_long)
                    self.cancel_event.set()
                case 1:
                    if self._process.pid is not None and os.name == "posix":
                        log.debug("%s process is not responding. Sending interrupt signal", self.process_name_long)
                        os.kill(self._process.pid, signal.SIGINT)  # TODO Python 3.14: self.process.interrupt()
                case 2:
                    log.warning("Timeout when waiting for %s to stop. Sending terminate signal", self.process_name_long)
                    self._process.terminate()
                case 3:
                    log.warning("Timeout when waiting for %s to terminate. Sending kill signal", self.process_name_long)
                    self._process.kill()
                case _:
                    raise TimeoutError(f"Failed to terminate the process {self.process_name_long}. May become a zombie")

            self._process.join(timeout)
            if not self._process.is_alive():
                log.debug("Process %s got stopped", self.process_name_long)
                return

    def run(self) -> None:
        try:
            with self.state_changed:
                self.state.set(ProcessState.UNINITIALIZED)
                self.state_changed.notify_all()

            # Logger needs to be initialized per-task.
            self.log_config.set_log_fmt()

            log.debug("Initializing %s", self.process_name_long)
            self._proc_init()
            with self.state_changed:
                self.state.set(ProcessState.READY)
                self.state_changed.notify_all()
            log.debug("Initialized %s successfully", self.process_name_long)

            self._proc_work(self.cancel_event)
            log.debug("Received a cancel event for %s", self.process_name_long)
        except BaseException as e:
            if isinstance(e, KeyboardInterrupt):
                log.debug("Caught an interrupt for %s", self.process_name_long)
            with self.state_changed:
                self.state_exception.set(e)
                self.state.set(ProcessState.ERROR)
                self.state_changed.notify_all()
        finally:
            log.debug("Cleaning up %s", self.process_name_long)
            self._proc_cleanup()
            with self.state_changed:
                if self.state_exception.get() is None:
                    self.state.set(ProcessState.CLOSED)
                    self.state_changed.notify_all()
            log.debug("Process %s finished cleanly", self.process_name_long)
            time.sleep(1)

    @abstractmethod
    def _proc_init(self):
        """Initialize the process.

        If there are some structures which are created, they should be stored as instance fields and cleaned up in _proc_cleanup().
        """

    def _proc_work(self, cancel_event: threading.Event):
        """Perform the work.

        Needs to somehow wait for the provided cancel_event.
        """
        cancel_event.wait()

    @abstractmethod
    def _proc_cleanup(self):
        """Clean up the process state."""


class WrappedProcessContext(WrappedProcess):
    def __init__(self, mp_context: SpawnContext, mp_manager: SyncManager, process_name_long: str,
                 process_name_short: str | None = None, log_config: LogConfig = LogConfig(),
                 start_timeout: float | None = WrappedProcess.DEFAULT_START_TIMEOUT,
                 stop_timeout: float = WrappedProcess.DEFAULT_STOP_TIMEOUT, state_changed: threading.Condition | None = None,
                 cancel_event: threading.Event | None = None) -> None:
        super().__init__(mp_context, mp_manager, process_name_long, process_name_short, log_config, state_changed, cancel_event)
        self._start_timeout = start_timeout
        self._stop_timeout = stop_timeout

    def __enter__(self):
        self.start(self._start_timeout)
        return self

    def __exit__(self, exc_type: type[BaseException] | None, exc: BaseException | None, tb: TracebackType | None) -> Literal[False]:
        if exc_type == KeyboardInterrupt:
            log.debug("Interrupting %s", self.process_name_long)
        else:
            log.debug("Terminating %s on error", self.process_name_long)
        self.stop(self._stop_timeout)
        return False


WrappedProcessT = TypeVar("WrappedProcessT", bound=WrappedProcess)


class WrappedProcessPool(ABC, Generic[WrappedProcessT]):
    def __init__(self, process_cls: type[WrappedProcessT], mp_context: SpawnContext, mp_manager: SyncManager, concurrency: int, name: str,
                 stop_timeout: float = WrappedProcess.DEFAULT_STOP_TIMEOUT) -> None:
        self.name = name
        self.state_changed = mp_manager.Condition()
        self.cancel_event = mp_manager.Event()
        self._pool = tuple(self._init_process(process_cls, id, mp_context, mp_manager, self.state_changed, self.cancel_event)
                           for id in range(concurrency))
        self._active = False
        self._stop_timeout = stop_timeout

    @abstractmethod
    def _init_process(self, process_cls: type[WrappedProcessT], id: int, mp_context: SpawnContext, mp_manager: SyncManager, state_changed: threading.Condition,
                      cancel_event: threading.Event) -> WrappedProcessT:
        """Initialize a process with index `id`."""

    def __len__(self) -> int:
        return len(self._pool)

    @property
    def active(self) -> bool:
        return self._active

    @property
    def state(self) -> tuple[ProcessState, ...]:
        return tuple(process.state.get() for process in self._pool)

    def get_with_state(self, *state: ProcessState) -> tuple[tuple[int, ProcessState, BaseException | None], ...]:
        return tuple((id, s, self._pool[id].state_exception.get()) for id, s in enumerate(self.state) if s in state)

    def check_if_error(self, raise_error: bool = False) -> bool:
        if errors := self.get_with_state(ProcessState.ERROR):
            log.error("Caught exceptions in the following processes:")
            for id, _, exception in errors:
                log.error("  %i: %r", id, exception)
            if raise_error:
                raise RuntimeError(f"Caught exceptions in processes: {', '.join(str(id) for id in errors)}")
            return True
        return False

    def start(self, timeout: float | None = WrappedProcess.DEFAULT_START_TIMEOUT) -> None:
        log.info("Initializing %s with concurrency of %i", self.name, len(self._pool))
        self._active = True
        exceptions: list[BaseException] = []
        try:
            with ThreadPoolExecutor(max_workers=len(self)) as pool:
                start_jobs = (pool.submit(lambda process: process.start(timeout), process) for process in self._pool)
                for id, result in enumerate(as_completed(start_jobs)):
                    try:
                        result.result()
                    except Exception as e:
                        log.error("Exception when starting process %i: %r", id, e)
                        exceptions.append(e)
            if exceptions:
                raise exceptions[0]
        except BaseException:
            self.stop()
            raise

    def stop(self) -> bool:
        if not self.active:
            return True

        # queue.Empty might happen on KeyboardInterrupt race.
        with suppress(queue.Empty):
            self.cancel_event.set()

        ret = True
        with ThreadPoolExecutor(max_workers=len(self)) as pool:
            stop_jobs = (pool.submit(lambda process: process.stop(self._stop_timeout), process) for process in self._pool)
            for id, result in enumerate(as_completed(stop_jobs)):
                try:
                    result.result()
                except Exception as e:
                    log.warning("Exception when stopping process %i: %r", id, e)
                    ret = False
        self._active = False
        return ret


class WrappedProcessPoolContext(Generic[WrappedProcessT], WrappedProcessPool[WrappedProcessT]):
    def __init__(self,process_cls: type[WrappedProcessT],  mp_context: SpawnContext, mp_manager: SyncManager, concurrency: int, name: str,
                 start_timeout: float | None = WrappedProcess.DEFAULT_START_TIMEOUT,
                 stop_timeout: float = WrappedProcess.DEFAULT_STOP_TIMEOUT) -> None:
        super().__init__(process_cls, mp_context, mp_manager, concurrency, name, stop_timeout)
        self._start_timeout = start_timeout

    def __enter__(self):
        self.start(self._start_timeout)
        return self

    def __exit__(self, exc_type: type[BaseException] | None, exc: BaseException | None, tb: TracebackType | None) -> Literal[False]:
        if exc_type == KeyboardInterrupt:
            log.debug("Interrupting %s", self.name)
        else:
            log.debug("Terminating %s on error", self.name)

        self.stop()
        return False
