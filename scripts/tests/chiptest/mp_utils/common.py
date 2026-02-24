import logging
from abc import ABC, abstractmethod
from types import TracebackType
from typing import Literal,  Self

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
            log.debug("Interrupting %s", self.name)
        else:
            log.debug("Stopping %s due to exception", self.name)

        self.stop()
        return False
