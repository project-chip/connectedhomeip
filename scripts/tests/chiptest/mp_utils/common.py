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
    @abstractmethod
    def start(self) -> None: ...

    @abstractmethod
    def stop(self) -> None: ...

    @property
    @abstractmethod
    def name(self) -> str: ...

    def __enter__(self) -> Self:
        self.start()
        return self

    def __exit__(self, exc_type: type[BaseException] | None, exc: BaseException | None, tb: TracebackType | None) -> Literal[False]:
        if exc_type is None:
            log.debug("Stopping %s", self.name)
        elif issubclass(exc_type, KeyboardInterrupt):
            log.warning("Interrupting %s per user request", self.name)
        elif exc is not None and tb is not None:
            log.exception("Stopping %s due to exception", self.name, exc_info=(exc_type, exc, tb))
        else:
            log.exception("Stopping %s due to exception: %r", self.name, exc)

        self.stop()
        return False

# TODO: Move it to a separate file.
@contextmanager
def mp_wrapped_spawn_context(wrapper_linux: str | None) -> Iterator[SpawnContext]:
    """Create platform-specific multiprocessing context.

    Linux:
    - We need to use spawn for the pool to have separate environment variables per runner, and to be able to use a wrapper script.
    - We need unshare wrapper script to have an option to mount per-worker /tmp (initialized per-worker).
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
            wrapper_file.write(
                '#!/bin/sh\n'
                f'exec {wrapper_linux} "{executable}" "$@"\n')
        mp_wrapper_name.chmod(mp_wrapper_name.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)

        source_context.set_executable(str(mp_wrapper_name))
        yield source_context
    finally:
        # Restore the default executable.
        source_context.set_executable(old_executable)
        if mp_wrapper_name is not None:
            mp_wrapper_name.unlink()
