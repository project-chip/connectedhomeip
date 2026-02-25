import dataclasses
from typing import ClassVar, Self

from chiptest.log_utils import LogConfig


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
