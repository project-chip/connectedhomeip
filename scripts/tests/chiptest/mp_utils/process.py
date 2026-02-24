import dataclasses
import functools
import logging
import multiprocessing
import multiprocessing.spawn
import os
import signal
import stat
import sys
import tempfile
from abc import ABC, abstractmethod
from collections.abc import Callable
from contextlib import contextmanager
from multiprocessing.context import SpawnContext
from multiprocessing.managers import SyncManager
from pathlib import Path
from typing import ClassVar, Concatenate, Generic, Iterator, ParamSpec, Self, TypeVar

from .log_utils import LogConfig
from .queue import EndOfWork, WorkQueue, WorkQueueCancelled
from .state import ProcessGroupState, ProcessState

log = logging.getLogger(__name__)


@contextmanager
def mp_wrapped_spawn_context(wrapper: str | None,
                             source_context: SpawnContext = multiprocessing.get_context("spawn")) -> Iterator[SpawnContext]:
    """Create platform-specific multiprocessing context.

    Linux:
    - We need to use spawn for the pool to have separate environment variables per runner, and to be able to use a wrapper script.
    - We need unshare wrapper script to have an option to mount per-worker /tmp.
    """
    if sys.platform != "linux" or wrapper is None:
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

@dataclasses.dataclass(frozen=True)
class ProcessConfigTemplate:
    DEFAULT_START_TIMEOUT: ClassVar[float] = 4.0
    DEFAULT_STOP_TIMEOUT: ClassVar[float] = 4.0
    DEFAULT_TERMINATION_TIMEOUT: ClassVar[float] = 2.0

    id: int = -1
    name_short: str = "P{id}"
    name_long: str = "Process {id}"
    log_config: LogConfig = dataclasses.field(default_factory=LogConfig)
    start_timeout: float | None = DEFAULT_START_TIMEOUT
    stop_timeout: float = DEFAULT_STOP_TIMEOUT
    termination_timeout: float = DEFAULT_TERMINATION_TIMEOUT

    def with_formatted_name(self, id: int) -> Self:
        name_short=self.name_short.format(id=id)
        name_long=self.name_long.format(id=id)

        # If the logger config defines a name of the base process, use it as a prefix for the short name and in the log config
        # itself, to make it more clear which process is logging.
        if self.log_config.process_name is not None:
            name_short = f"{self.log_config.process_name}/{name_short}"

        return dataclasses.replace(self, id=id, name_short=name_short, name_long=name_long,
                                   log_config=dataclasses.replace(self.log_config, process_name=name_short))


S = TypeVar("S", bound="WrappedProcess")
P = ParamSpec("P")
R = TypeVar("R")


def with_annotated_exception(fn: Callable[Concatenate[S, P], R]) -> Callable[Concatenate[S, P], R]:
    """Decorator to enrich exceptions from WrappedProcess methods with process information."""

    @functools.wraps(fn)
    def wrapper(self: S, *args: P.args, **kwargs: P.kwargs) -> R:
        try:
            return fn(self, *args, **kwargs)
        except BaseException as e:
            e.add_note(f"Exception in process {self._config.name_long}")
            raise

    return wrapper


ConfigT = TypeVar("ConfigT", bound=ProcessConfigTemplate)
WorkRequestT = TypeVar("WorkRequestT")
WorkResponseT = TypeVar("WorkResponseT")


class WrappedProcess(ABC, Generic[ConfigT, WorkRequestT, WorkResponseT]):
    def __init__(self, mp_context: SpawnContext, mp_manager: SyncManager, config: ConfigT,
                 work_queue: WorkQueue[WorkRequestT, WorkResponseT] | None = None,
                 group_state: ProcessGroupState | None = None) -> None:
        # Neither mp_context or mp_manager should be saved in the instance, as they are not picklable between processes but they can
        # be used to initialize some shared resources.

        # Create state and work queue.
        self._config = config
        self.work_queue = work_queue if work_queue is not None else WorkQueue[WorkRequestT, WorkResponseT](mp_manager)
        self.state = ProcessState(mp_manager, config, group_state)

        # Create multiprocessing.Process in the given context.
        self._proc = mp_context.Process(target=self.run, name=self._config.name_short)

    @property
    def name(self) -> str:
        return self._config.name_long

    @with_annotated_exception
    def start(self) -> None:
        log.debug("Starting process")
        self._proc.start()

        # No start timeout means that the caller doesn't want to wait for initialization to finish.
        if self._config.start_timeout is None:
            return

        try:
            if not self.state.wait_for(lambda phase: phase not in (ProcessState.Phase.NOT_STARTED,
                                                                   ProcessState.Phase.UNINITIALIZED), self._config.start_timeout):
                raise TimeoutError("Timeout when waiting for initialization")

            with self.state:
                match self.state.exception:
                    case KeyboardInterrupt():
                        raise KeyboardInterrupt()
                    case BaseException() as exception:
                        raise RuntimeError(f"Process failed initialization with exception: {exception!r}")

                match self.state.phase:
                    case ProcessState.Phase.READY:
                        log.debug("Process started successfully")
                    case ProcessState.Phase.CLOSED:
                        raise RuntimeError("Process closed immediately after initialization")
                    case _:
                        raise RuntimeError("Process is in unexpected state")
        except BaseException as e:
            if not isinstance(e, KeyboardInterrupt):
                log.info("Stopping the process on failure during inititialization")
            self.stop()
            raise

    @with_annotated_exception
    def stop(self) -> None:
        def has_stopped(timeout: float) -> bool:
            self._proc.join(timeout)
            if not self._proc.is_alive():
                log.debug("Process got stopped")
                return True
            return False

        log.debug("Cancelling work in process")
        self.work_queue.cancel()
        if has_stopped(self._config.stop_timeout):
            return

        if self._proc.pid is not None:
            log.info("Process is not responding to the cancel event. Sending interrupt signal")
            os.kill(self._proc.pid, signal.SIGINT)  # TODO Python 3.14: self.process.interrupt()
        if has_stopped(self._config.stop_timeout):
            return

        log.warning("Timeout when waiting for process to stop. Sending terminate signal")
        self._proc.terminate()
        if has_stopped(self._config.termination_timeout):
            return

        log.warning("Timeout when waiting for process to terminate. Sending kill signal")
        self._proc.kill()
        if has_stopped(self._config.termination_timeout):
            return

        raise TimeoutError(f"Failed to terminate the process {self._config.name_long}. May become a zombie")

    # Methods run in the subprocess.

    def run(self) -> None:
        try:
            self.state.phase = ProcessState.Phase.UNINITIALIZED

            # Logger needs to be initialized per-process.
            self._config.log_config.set_log_fmt(log)

            # Initialize.
            log.debug("Initializing")
            self._proc_init()
            self.state.phase = ProcessState.Phase.READY
            log.debug("Initialized successfully")

            # Perform work.
            self._proc_work()
        except EndOfWork:
            log.debug("Received end of work signal")
        except WorkQueueCancelled:
            log.debug("Received a cancel event")
        except BaseException as e:
            if isinstance(e, KeyboardInterrupt):
                log.debug("Caught an interrupt")
            self.state.exception = e
        finally:
            log.debug("Cleaning up")
            self._proc_cleanup()
            self.state.phase = ProcessState.Phase.CLOSED
            log.debug("Process finished cleanly")

    @abstractmethod
    def _proc_init(self):
        """Initialize the process.

        If there are some structures which are created, they should be stored as instance fields and cleaned up in _proc_cleanup().
        """

    def _proc_work(self) -> None:
        """Perform the work.

        Should wait for cancellation (default) or consume the work_queue while actively setting `Phase.READY` and `Phase.WORKING`.
        """
        self.work_queue.wait_for_cancel()

    @abstractmethod
    def _proc_cleanup(self):
        """Clean up the process state."""
