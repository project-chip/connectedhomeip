import logging
from dataclasses import dataclass
from functools import cache

import coloredlogs

log = logging.getLogger(__name__)

@dataclass(frozen=True)
class LogConfig:
    log_level: int | str = logging.INFO
    log_timestamps: bool = True
    process_name: str | None = None

    @cache
    def get_log_format(self, task: str | None = None):
        """Set the logger format for the process

        - add timestamp if enabled,
        - add short process name if defined (in bold),
        - add task name if the process has some specific task at hand (can be changed dynamically).
        """
        log_fmt = "%(levelname)-7s "
        if self.process_name is not None:
            proc_task_sep = ": " if task is None else " → "
            log_fmt += f"\033[1m{self.process_name}\033[0m{proc_task_sep}"
        if task is not None:
            log_fmt += f"{task}: "
        log_fmt += "%(message)s"
        if self.log_timestamps:
            return "%(asctime)s.%(msecs)03d " + log_fmt
        return log_fmt

    @property
    def log_level_int(self) -> int:
        if isinstance(self.log_level, int):
            return self.log_level
        return coloredlogs.find_defined_levels()[self.log_level]

    def set_log_fmt(self, task: str | None = None):
        coloredlogs.install(level=self.log_level_int, fmt=self.get_log_format(task))
