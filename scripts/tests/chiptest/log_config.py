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
from collections.abc import Iterator

import coloredlogs

LOG_LEVELS = tuple(coloredlogs.find_defined_levels().keys())
"""Possible log levels for coloredlogs."""

_FIELD_STYLES = coloredlogs.DEFAULT_FIELD_STYLES | {
    "process_thread": {"bold": True},
    # We need to explicitly disable bold for subsequent fields in case process_thread is empty, as coloredlogs don't seem to close
    # the formatting in that case.
    "task": {"bold": False},
    "message": {"bold": False},
}


class ProcessThreadTaskFilter(logging.Filter):
    """Logging filter to add process/thread and task information to log records."""

    def __init__(self) -> None:
        super().__init__(name=self.__class__.__name__)
        self.task_name: str | None = None

    def filter(self, record: logging.LogRecord) -> bool:
        """Add process/thread and task information to the log record.

        Process/thread component (styled as bold) shows hierarchy of the current process and thread names joined with slash,
        filtering out the default "MainProcess" and "MainThread".
        """
        proc_thread = '/'.join(name for name, default in ((record.processName, "MainProcess"), (record.threadName, "MainThread"))
                               if name is not None and name != default)

        record.process_thread = f"[{proc_thread}] " if proc_thread else ""
        record.task = f"{self.task_name}: " if self.task_name else ""
        return True


@dataclasses.dataclass
class LogConfig:
    """Logger configuration."""

    level_regular: int | str = logging.INFO
    """Regular logger level used for the majority of logging."""

    level_tests: int | str = logging.INFO
    """Logger level used during execution of a test."""

    timestamps: bool = True
    """Enable timestamps in the log output."""

    # Internal state.
    _filter: ProcessThreadTaskFilter = dataclasses.field(default_factory=ProcessThreadTaskFilter, init=False)
    _cur_task: str | None = dataclasses.field(default=None, init=False)
    _cur_level: int | str | None = dataclasses.field(default=None, init=False)

    def set_fmt(self, task: str | None = None, level: int | str | None = None) -> None:
        """Set the logger format."""
        if level is None:
            level = self.level_regular

        fmt = ("%(asctime)s.%(msecs)03d " if self.timestamps else "") + "%(levelname)-7s %(process_thread)s%(task)s%(message)s"

        logger = logging.getLogger()
        coloredlogs.install(level=level, fmt=fmt, logger=logger, field_styles=_FIELD_STYLES)

        self._filter.task_name = task
        for handler in logger.handlers:
            # addFilter is idempotent, so we don't need to check if it's already added.
            handler.addFilter(self._filter)

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
