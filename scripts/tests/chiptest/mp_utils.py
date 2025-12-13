
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
from contextlib import contextmanager, ContextBaseClass
from multiprocessing.context import SpawnContext
from multiprocessing.managers import SyncManager
from pathlib import Path
from types import TracebackType
from typing import Iterator

from .log_utils import LogConfig

log = logging.getLogger(__name__)


@contextmanager
def mp_wrapped_spawn_context(wrapper: str, source_context: SpawnContext = multiprocessing.get_context("spawn")) -> Iterator[SpawnContext]:
    """Create platform-specific multiprocessing context.

    - We need to use spawn for the pool to have separate environment variables per runner, and to be able to use a wrapper script.
    - [Linux] We need unshare wrapper script to have an option to mount per-worker /tmp.
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


class WrappedMultiprocessingProcessContext(ABC):
    DEFAULT_START_TIMEOUT = 2.0
    DEFAULT_STOP_TIMEOUT = 2.0

    def __init__(self, mp_context: SpawnContext, mp_manager: SyncManager, process_name_long: str,
                 process_name_short: str | None = None, log_config: LogConfig = LogConfig(),
                 start_timeout: float = DEFAULT_START_TIMEOUT, stop_timeout: float = DEFAULT_STOP_TIMEOUT) -> None:
        # Neither mp_context or mp_manager should be saved in the instance, as they are not picklable.

        # Create init and cancel events.
        self._init_done = mp_manager.Event()
        self._cancel_event = mp_manager.Event()

        # Create multiprocessing.Process in a given context.
        self.process_name_long = process_name_long
        self.process_name_short = process_name_short
        self._process = mp_context.Process(target=self.run, name=process_name_long)

        self.log_config = dataclasses.replace(log_config, process_name=process_name_short)
        self.start_timeout = start_timeout
        self.stop_timeout = stop_timeout

    def start(self):
        log.debug("Starting process %s", self.process_name_long)
        self._process.start()
        if self._init_done.wait(self.start_timeout):
            log.debug("Process %s started", self.process_name_long)
            return

        if self._process.exitcode is None:
            self.stop()
            raise TimeoutError("Timeout when waiting for initialization of %s", self.process_name_long)

        raise TimeoutError(f"The process {self.process_name_long} exited with code {self._process.exitcode} during initialization")

    def __enter__(self):
        self.start()
        return self

    def stop(self) -> None:
        for method in itertools.count():
            match method:
                case 0:
                    log.debug("Cancelling work in process %s", self.process_name_long)
                    self._cancel_event.set()
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
                    raise TimeoutError(f"Failed to terminate the process {self.process_name_long}")

            self._process.join(self.stop_timeout)
            if not self._process.is_alive():
                log.debug("Process %s got stopped", self.process_name_long)
                return

    def __exit__(self, exc_type: type[BaseException] | None, exc: BaseException | None, tb: TracebackType | None) -> bool:
        if exc_type == KeyboardInterrupt:
            log.debug("Interrupting %s", self.process_name_long)
            self.stop()
            return True

        log.debug("Terminating %s on error", self.process_name_long)
        self.stop()
        return False

    def run(self) -> None:
        try:
            # Logger needs to be initialized per-task.
            self.log_config.set_log_fmt()

            log.debug("Initializing %s", self.process_name_long)
            self._proc_init()
            self._init_done.set()
            log.debug("Initialized %s successfully", self.process_name_long)

            self._proc_work(self._cancel_event)
            log.debug("Received a cancel event for %s", self.process_name_long)
        except KeyboardInterrupt:
            log.debug("Caught an interrupt for %s", self.process_name_long)
        finally:
            log.debug("Cleaning up %s", self.process_name_long)
            self._proc_cleanup()
            log.debug("Process %s finished cleanly", self.process_name_long)

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
