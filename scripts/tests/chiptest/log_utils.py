import dataclasses
import logging
import multiprocessing
import threading

import coloredlogs


class ProcessThreadTaskFilter(logging.Filter):
    def __init__(self) -> None:
        super().__init__(name=self.__class__.__name__)
        self.task_name: str | None = None

    def filter(self, record: logging.LogRecord) -> bool:
        components: list[str] = []
        if (process_name := multiprocessing.current_process().name) != "MainProcess":
            components.append(process_name)
        if (thread_name := threading.current_thread().name) != "MainThread":
            components.append(thread_name)

        ret = f"\033[1m[{'/'.join(components)}]\033[0m " if components else ""
        if self.task_name is not None:
            ret += f"{self.task_name}: "
        record.process_thread_task = ret

        return True


@dataclasses.dataclass(frozen=True)
class LogConfig:
    level: int | str = logging.INFO
    timestamps: bool = True

    _filter: ProcessThreadTaskFilter = dataclasses.field(default_factory=ProcessThreadTaskFilter, init=False)

    @property
    def log_level_int(self) -> int:
        if isinstance(self.level, int):
            return self.level
        return coloredlogs.find_defined_levels()[self.level]

    def set_log_fmt(self, task: str | None = None, log_level: int | str | None = None, logger: logging.Logger | None = None) -> None:
        """Set the logger format for the process

        - add timestamp if enabled,
        - add short process name if defined (in bold),
        - add task name if the process has some specific task at hand (can be changed dynamically).
        """
        if log_level is None:
            log_level = self.level

        fmt = "%(asctime)s.%(msecs)03d " if self.timestamps else ""
        fmt += "%(levelname)-7s %(process_thread_task)s%(message)s"

        if logger is None:
            logger = logging.getLogger()

        coloredlogs.install(level=log_level, fmt=fmt, logger=logger)

        self._filter.task_name = task
        for handler in logger.handlers:
            handler.addFilter(self._filter)


LOG_LEVELS = tuple(coloredlogs.find_defined_levels().keys())
