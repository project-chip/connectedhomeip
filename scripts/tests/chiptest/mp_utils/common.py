import logging
from abc import ABC, abstractmethod
from types import TracebackType
from typing import Literal, Self

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
