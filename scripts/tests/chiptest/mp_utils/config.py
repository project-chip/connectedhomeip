import dataclasses
import multiprocessing
from typing import ClassVar, Self

from chiptest.log_utils import LogConfig


@dataclasses.dataclass
class ProcessConfigTemplate:
    DEFAULT_START_TIMEOUT: ClassVar[float] = 4.0
    DEFAULT_STOP_TIMEOUT: ClassVar[float] = 4.0
    DEFAULT_TERMINATION_TIMEOUT: ClassVar[float] = 2.0

    id: int = 0
    name: str = "Process{id}"
    log_config: LogConfig = dataclasses.field(default_factory=LogConfig)
    start_timeout: float | None = DEFAULT_START_TIMEOUT
    stop_timeout: float = DEFAULT_STOP_TIMEOUT
    termination_timeout: float = DEFAULT_TERMINATION_TIMEOUT

    def with_formatted_name(self, id: int = 0) -> Self:
        name=self.name.format(id=id)

        # Indicate process hierarchy in the name so that it's easier to understand the dependencies in the logs.
        if (parent_proc_name := multiprocessing.current_process().name) != "MainProcess":
            name = f"{parent_proc_name}/{name}"

        return dataclasses.replace(self, id=id, name=name)
