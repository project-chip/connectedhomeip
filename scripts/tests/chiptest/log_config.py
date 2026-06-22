#
#    Copyright (c) 2026 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import contextlib
import dataclasses
import logging
import threading
from collections.abc import Iterator
from multiprocessing.managers import SyncManager, ValueProxy
from types import TracebackType
from typing import Self

import coloredlogs

LOG_LEVELS = tuple(coloredlogs.find_defined_levels().keys())
"""Possible log levels for coloredlogs."""

_FIELD_STYLES = coloredlogs.DEFAULT_FIELD_STYLES | {
    "process_thread": {"bold": True},
    # We need to explicitly disable bold for subsequent fields in case process_thread is empty, as coloredlogs don't seem to close
    # the formatting in that case.
    "task": {"bold": False},
    "message": {"bold": False},
    "status": {"color": "blue", "bold": True},
}


class LogMessageCounter:
    """Cross-thread or cross-process cancellable counter for printed log messages."""

    _counter: int | ValueProxy

    def __init__(self, mp_manager: SyncManager | None = None) -> None:
        if mp_manager is None:
            self._cond = threading.Condition()
            self._counter = 0
            self._cancelled = threading.Event()
        else:
            self._cond = mp_manager.Condition()
            self._counter = mp_manager.Value('i', 0)
            self._cancelled = mp_manager.Event()

    def increment(self) -> None:
        """Atomically increment the shared message count."""
        with self._cond:
            if self.cancelled:
                return

            if isinstance(self._counter, int):
                self._counter += 1
            else:
                self._counter.value += 1
            self._cond.notify_all()

    def cancel(self) -> None:
        """Cancel any waiting on the counter."""
        with self._cond:
            self._cancelled.set()
            self._cond.notify_all()

    @property
    def cancelled(self) -> bool:
        """Check if the counter has been cancelled."""
        return self._cancelled.is_set()

    @property
    def total(self) -> int:
        """Return total number of printed messages."""
        with self._cond:
            return self._counter if isinstance(self._counter, int) else self._counter.value

    def wait_for_count_or_cancel(self, count: int, timeout: float | None = None) -> bool:
        """Wait until the total message count reaches at least the specified count or until cancelled."""
        with self._cond:
            return self._cond.wait_for(lambda: self.total >= count or self.cancelled, timeout=timeout)

    def __enter__(self) -> Self:
        return self

    def __exit__(self, exc_type: type[BaseException] | None, exc_val: BaseException | None, exc_tb: TracebackType | None) -> None:
        self.cancel()


class ProcessThreadTaskFilter(logging.Filter):
    """Logging filter to add process/thread and task information to log records."""

    def __init__(self, msg_counter: LogMessageCounter | None = None) -> None:
        super().__init__(name=self.__class__.__name__)
        self.task_name: str | None = None
        self.msg_counter = msg_counter

    def filter(self, record: logging.LogRecord) -> bool:
        """Add process/thread and task information to the log record and count the message.

        Process/thread component (styled as bold) shows hierarchy of the current process and thread names joined with slash,
        filtering out the default "MainProcess" and "MainThread".
        """
        proc_thread = '/'.join(name for name, default in ((record.processName, "MainProcess"), (record.threadName, "MainThread"))
                               if name is not None and name != default)

        record.process_thread = f"[{proc_thread}] " if proc_thread else ""
        record.task = f"{self.task_name}: " if self.task_name else ""
        if not hasattr(record, "status"):
            record.status = ""

        # Count printed messages if they're not explicitly marked to be ignored by setting the "count" attribute to False.
        if self.msg_counter is not None and getattr(record, "count", True):
            self.msg_counter.increment()

        return True


@dataclasses.dataclass
class LogConfig:
    """Logger configuration."""

    level_regular: int | str = logging.INFO
    """Regular logger level used for the majority of logging."""

    level_tests: int | str = logging.INFO
    """Logger level used during execution of a test."""

    level_rpc: int | str = logging.INFO
    """Logger level used for RPC-related logging."""

    timestamps: bool = True
    """Enable timestamps in the log output."""

    # Internal state.
    filter: ProcessThreadTaskFilter = dataclasses.field(default_factory=ProcessThreadTaskFilter, init=False)
    _cur_task: str | None = dataclasses.field(default=None, init=False)
    _cur_level: int | str | None = dataclasses.field(default=None, init=False)

    def set_fmt(self, task: str | None = None, level: int | str | None = None) -> None:
        """Set the logger format."""
        if level is None:
            level = self.level_regular

        fmt = "%(asctime)s.%(msecs)03d " if self.timestamps else ""
        fmt += "%(levelname)-7s %(process_thread)s%(task)s%(message)s%(status)s"

        logger = logging.getLogger()
        coloredlogs.install(level=level, fmt=fmt, logger=logger, field_styles=_FIELD_STYLES)

        self.filter.task_name = task
        for handler in logger.handlers:
            # addFilter is idempotent, so we don't need to check if it's already added.
            handler.addFilter(self.filter)

        self._cur_task = task
        self._cur_level = level

    @contextlib.contextmanager
    def fmt_context(self, task: str | None = None, level: int | str | None = None) -> Iterator[None]:
        """Context manager to temporarily change the logger format."""
        prev_task = self._cur_task
        prev_level = self._cur_level
        try:
            self.set_fmt(task if task is not None else prev_task, level if level is not None else prev_level)
            yield
        finally:
            self.set_fmt(prev_task, prev_level)
