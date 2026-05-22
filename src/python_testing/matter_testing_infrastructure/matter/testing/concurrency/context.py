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

import contextlib
import logging
import threading
from abc import ABC, abstractmethod
from types import TracebackType
from typing import Self

log = logging.getLogger(__name__)


class TerminableResource(contextlib.AbstractContextManager, ABC):
    """Abstract base class for resources that can be terminated."""

    def __enter__(self) -> Self:
        log.debug("Starting %s", self.__class__.__name__)
        try:
            self.resource_start()
        except BaseException as start_ex:
            log.error("Failed to start resource %s: %r", self.__class__.__name__, start_ex)
            start_ex.add_note(f"Failure during start of resource {self.__class__.__name__}")
            try:
                self.resource_terminate()
            except BaseException as term_ex:
                log.error("Failed to terminate resource %s during start failure: %r", self.__class__.__name__, term_ex)
                raise term_ex.with_traceback(term_ex.__traceback__) from start_ex
            raise
        return self

    def __exit__(self, exc_type: type[BaseException] | None, exc_value: BaseException | None,
                 traceback: TracebackType | None) -> bool | None:
        log.debug("Terminating %s", self.__class__.__name__)
        try:
            self.resource_terminate()
            log.debug("Terminated %s", self.__class__.__name__)
        except BaseException as e:
            log.error("Failed to terminate resource %s: %r", self.__class__.__name__, e)
            e.add_note(f"Failure during termination of resource {self.__class__.__name__}")
            if exc_value is not None:
                raise e.with_traceback(e.__traceback__) from exc_value
            raise
        return None

    def resource_start(self) -> None:
        """Initialize or start the resource."""

    @abstractmethod
    def resource_terminate(self) -> None:
        """Terminate the resource."""


class TerminableThread(TerminableResource, threading.Thread):
    """Thread that can be terminated using the TerminableResource context manager."""

    JOIN_TIMEOUT_S: float = 5.0

    def resource_start(self) -> None:
        self.start()

    def resource_thread_join(self) -> bool:
        """Join the thread and return whether it has been successfully joined (i.e. is not alive anymore)."""
        if self.ident is not None:
            log.debug("Joining thread %s", self.name)
            self.join(timeout=self.JOIN_TIMEOUT_S)
        return not self.is_alive()

    def resource_terminate(self) -> None:
        if not self.resource_thread_join():
            raise RuntimeError(f"Thread {self.name} is still alive after termination, it might be stuck on some operation")
