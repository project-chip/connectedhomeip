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
import subprocess
import threading
from abc import abstractmethod
from types import TracebackType
from typing import Any, Callable, Generic, Self, TypeVar

log = logging.getLogger(__name__)


InternalResourceT = TypeVar("InternalResourceT")


class TerminableResourceBase(contextlib.AbstractContextManager, Generic[InternalResourceT]):
    """Abstract base class for resources that can be terminated."""

    RESOURCE_TIMEOUT_START_S: float = 5.0
    """Timeout for resource start (if applicable), in seconds."""

    RESOURCE_TIMEOUT_TERMINATE_S: float = 5.0
    """Timeout for resource termination (if applicable), in seconds."""

    def __init__(self, name: str | None = None, terminate_debug_logging: bool = True) -> None:
        self._name = name or self.__class__.__name__
        self._terminate_debug_logging = terminate_debug_logging

    def __enter__(self) -> Self | InternalResourceT:
        log.debug("Starting %s", self._name)
        try:
            return resource if (resource := self.resource_start()) is not None else self
        except BaseException as start_ex:
            log.error("Failed to start resource %s: %r", self._name, start_ex)
            start_ex.add_note(f"Failure during start of resource {self._name}")
            try:
                self.resource_terminate()
            except BaseException as term_ex:
                log.error("Failed to terminate resource %s during start failure: %r", self._name, term_ex)
                raise term_ex.with_traceback(term_ex.__traceback__) from start_ex
            raise

    def __exit__(self, exc_type: type[BaseException] | None, exc_value: BaseException | None,
                 traceback: TracebackType | None) -> bool | None:
        if self._terminate_debug_logging:
            log.debug("Terminating %s", self._name)
        try:
            self.resource_terminate()
            if self._terminate_debug_logging:
                log.debug("Terminated %s", self._name)
        except BaseException as e:
            log.error("Failed to terminate resource %s: %r", self._name, e)
            e.add_note(f"Failure during termination of resource {self._name}")
            if exc_value is not None:
                raise e.with_traceback(e.__traceback__) from exc_value
            raise
        return None

    @abstractmethod
    def resource_start(self) -> InternalResourceT:
        """Initialize or start the resource."""

    @abstractmethod
    def resource_terminate(self) -> None:
        """Terminate the resource."""


class TerminableResource(TerminableResourceBase[None]):
    """TerminableResource with no internal resource."""

    def __enter__(self) -> Self:
        super().__enter__()
        return self

    def resource_start(self) -> None:
        """Initialize or start the resource."""


class TerminableThread(TerminableResource, threading.Thread):
    """
    Thread that can be terminated using the TerminableResource context manager.

    If you need to wait for some output from the thread to determine that it has started successfully, you can implement that logic
    in `resource_start()`, and you are expected to raise a TimeoutError exception if the thread fails to start within the
    `RESOURCE_TIMEOUT_START_S` timeout.
    """

    def __init__(self, group: None = None, target: Callable[[], None] | None = None, name: str | None = None,
                 args: tuple[Any, ...] = (), kwargs: dict[str, Any] | None = None, *, daemon: bool | None = None,
                 terminate_debug_logging: bool = True) -> None:
        threading.Thread.__init__(self, group, target, name, args, kwargs, daemon=daemon)
        super().__init__(name, terminate_debug_logging)

    def resource_start(self) -> None:
        self.start()

    def resource_thread_join(self) -> bool:
        """Join the thread and return whether it has been successfully joined (i.e. is not alive anymore)."""
        if self.ident is not None:
            log.debug("Joining thread %s", self.name)
            self.join(timeout=self.RESOURCE_TIMEOUT_TERMINATE_S)
        return not self.is_alive()

    def resource_terminate(self) -> None:
        if not self.resource_thread_join():
            raise RuntimeError(f"Thread {self.name} is still alive after termination, it might be stuck on some operation")


PopenT = TypeVar("PopenT", bytes, str)


class TerminablePopen(TerminableResourceBase[subprocess.Popen[PopenT]]):
    """
    Subprocess that can be terminated using the TerminableResource context manager.

    The subprocess is created and started in the `resource_start()` method, and the process object should be stored in
    `self._process`. The `resource_terminate()` method will then attempt to terminate the process gracefully, and kill it if it does
    not terminate within the specified timeout.

    If you need to wait for some output from the process to determine that it has started successfully, you can implement that logic
    in `resource_start()`, and you are expected to raise a TimeoutError exception if the process fails to start within the
    `RESOURCE_TIMEOUT_START_S` timeout.
    """

    def __init__(self, create_popen: Callable[[], subprocess.Popen[PopenT]], name: str | None = None,
                 terminate_debug_logging: bool = True) -> None:
        super().__init__(name, terminate_debug_logging)
        self._process: subprocess.Popen[PopenT] | None = None
        self._create_popen: Callable[[], subprocess.Popen[PopenT]] = create_popen

    def resource_start(self) -> subprocess.Popen[PopenT]:
        self._process = self._create_popen()
        return self._process

    def resource_terminate(self) -> None:
        if self._process is None:
            return

        try:
            # Check if process already exited.
            if self._process.poll() is not None:
                return
            cmd = str(self._process.args)

            # SIGTERM
            log.debug('Terminating leftover process "%s"', cmd)
            try:
                self._process.terminate()
            except OSError:
                # Can occur in case of race condition when process exits between poll and terminate.
                return
            with contextlib.suppress(subprocess.TimeoutExpired):
                self._process.wait(self.RESOURCE_TIMEOUT_TERMINATE_S)
                return

            # SIGKILL
            log.warning('Failed to terminate the process "%s". Killing instead', cmd)
            try:
                self._process.kill()
            except OSError:
                return
            with contextlib.suppress(subprocess.TimeoutExpired):
                self._process.wait(self.RESOURCE_TIMEOUT_TERMINATE_S)
                return

            log.error('Failed to kill process "%s". It may become a zombie', cmd)
        finally:
            if self._process.stdout is not None:
                self._process.stdout.close()
            if self._process.stderr is not None:
                self._process.stderr.close()
            if self._process.stdin is not None:
                self._process.stdin.close()
