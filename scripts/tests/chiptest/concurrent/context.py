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

import logging
import multiprocessing
import multiprocessing.spawn
import stat
import sys
import tempfile
from abc import ABC, abstractmethod
from contextlib import contextmanager
from multiprocessing.context import SpawnContext
from pathlib import Path
from types import TracebackType
from typing import Iterator, Literal, Self

log = logging.getLogger(__name__)


class StartStopContextMixin(ABC):
    """
    Abstract mixin that adds context-manager lifecycle semantics (`start`/`stop`) to long-running or managed resources.

    Behavior:
    - Entering the context (`with ...`) calls `start()` and returns `self`.
    - Exiting the context always calls `stop()`.
    - Exit-time logging varies by outcome:
        - Normal exit: debug log for stopping.
        - `KeyboardInterrupt`: warning log indicating user interruption.
        - Other exceptions: error log with traceback/details, then propagation.
    """

    @abstractmethod
    def start(self) -> None:
        """Initialize and begin the resource/process."""

    @abstractmethod
    def stop(self) -> None:
        """Tear down and release the resource/process."""

    @property
    @abstractmethod
    def name(self) -> str:
        """Human-readable identifier used in log messages."""

    def __enter__(self) -> Self:
        self.start()
        return self

    def __exit__(self, exc_type: type[BaseException] | None, exc: BaseException | None, tb: TracebackType | None) -> Literal[False]:
        match exc:
            case None:
                log.debug("Stopping %s", self.name)
            case KeyboardInterrupt():
                log.warning("Interrupting %s per user request", self.name)
            case _:
                log.error("Stopping %s due to an exception: %r", self.name, exc)
        self.stop()
        return False


@contextmanager
def mp_wrapped_spawn_context(wrapper_linux: str | None) -> Iterator[SpawnContext]:
    """
    Create platform-specific multiprocessing context.

    Linux:
    - We need to use spawn for the pool to have separate environment variables per runner, and to be able to use a wrapper script.
    - We need unshare wrapper script to have an option to run the subprocess in a separate network namespace or mount per-worker
      /tmp (initialized per-worker).
    """
    source_context = multiprocessing.get_context("spawn")

    if sys.platform != "linux" or wrapper_linux is None:
        yield source_context
        return

    mp_wrapper_name: Path | None = None
    old_executable = multiprocessing.spawn.get_executable()
    executable = old_executable.decode('utf-8') if isinstance(old_executable, bytes) else old_executable
    try:
        with tempfile.NamedTemporaryFile("w", encoding="utf8", delete=False) as wrapper_file:
            mp_wrapper_name = Path(wrapper_file.name)
            wrapper_file.write('#!/bin/sh\n'
                               f'exec {wrapper_linux} "{executable}" "$@"\n')
        mp_wrapper_name.chmod(mp_wrapper_name.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)

        source_context.set_executable(str(mp_wrapper_name))
        yield source_context
    finally:
        # Restore the default executable.
        source_context.set_executable(old_executable)
        if mp_wrapper_name is not None:
            mp_wrapper_name.unlink()
