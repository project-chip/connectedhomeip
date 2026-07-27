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
import os
import signal
from abc import ABC, abstractmethod
from multiprocessing.context import SpawnContext
from multiprocessing.managers import SyncManager
from typing import Generic, TypeVar

from chiptest.concurrency.config import ProcessConfig
from chiptest.concurrency.context import with_annotated_exception
from chiptest.concurrency.state import ProcessPhase, ProcessState
from chiptest.concurrency.work_queue import CancellableQueue, EndOfQueue, QueueCancelled

log = logging.getLogger(__name__)


WorkerConfigT = TypeVar("WorkerConfigT", bound=ProcessConfig)
WorkRequestT = TypeVar("WorkRequestT")
WorkResponseT = TypeVar("WorkResponseT")


class WrappedProcess(ABC, Generic[WorkerConfigT, WorkRequestT, WorkResponseT]):
    """
    Base class for wrapped Python subprocesses.

    WrappedProcess owns a `multiprocessing.Process` and coordinates its lifecycle through shared `ProcessState` and work/response
    queues.

    Intended wrapping flow:
    1. The caller enters `mp_wrapped_spawn_context()` from `concurrent/context.py`.
    2. The context always uses the spawn start method, and on Linux can optionally install a temporary wrapper executable that
       launches Python through `wrapper_linux`.
    3. That `SpawnContext` is passed to WrappedProcess and used to create the child process.
    4. Parent-side lifecycle is driven by `start()` / `stop()`.
    5. Child-side lifecycle is driven by `run()` and the `_proc_*` hook methods.
    6. On context exit, `mp_wrapped_spawn_context()` restores the original multiprocessing executable and removes the wrapper.

    The lifecycle of the subprocess is coordinated through `ProcessState` and follows these phases:
    1. Parent calls `start()`.
    2. Subprocess enters `run()` and sets phase `UNINITIALIZED`.
    3. `_proc_init()` is invoked once for subclass-specific initialization. All
       resources that require cleanup should be registered in the provided `ExitStack`.
    4. On success, phase becomes `READY`.
    5. `_proc_work()` executes the main loop (by default waits for cancellation). Implementations may optionally toggle between
       `READY` and `WORKING` to expose idle vs active periods. You can use working_context() for that.
    6. After finishing work, the resources initialized in `_proc_init()` are cleaned up by the exit stack.
    7. Phase becomes `CLOSED` when the subprocess exits.

    Error handling behavior:
    - Unhandled exceptions from subprocess code are stored in `state.exception`.
    - `start()` waits for initialization and fails if the process reports an exception, closes early, or times out.
    - `stop()` attempts graceful shutdown first, then escalates with SIGINT, SIGTERM, and SIGKILL with configured timeouts.

    Subclasses are expected to implement `_proc_init()`, and may optionally override `_proc_work()`.
    """
    # Methods run in the parent process.

    def __init__(self, mp_context: SpawnContext, mp_manager: SyncManager, config: WorkerConfigT,
                 work_queue: CancellableQueue[WorkRequestT], rsp_queue: CancellableQueue[WorkResponseT]) -> None:
        # Neither mp_context or mp_manager should be saved as fields, as they are not picklable between processes. They can be used
        # to initialize some shared resources in the constructor.

        self._config = config
        self.work_queue = work_queue
        self._rsp_queue = rsp_queue
        self.state = ProcessState(mp_manager, config)

        # Create multiprocessing.Process in the given context.
        self._proc = mp_context.Process(target=self.run, name=self._config.name)
        self._stopped = False

    def __getstate__(self) -> dict[str, object]:
        state = self.__dict__.copy()
        # Drop _proc field to exclude it from pickling on process spawn.
        state.pop("_proc", None)
        return state

    @property
    def name(self) -> str:
        return self._config.name

    @with_annotated_exception
    def start(self) -> None:
        """
        Start the subprocess and wait until initialization completes.

        After triggering the process start, wait until phase leaves `NOT_STARTED`/`UNINITIALIZED` and then validate final startup
        state: `READY` is success, and `CLOSED` or any recorded exception is failure.
        """
        if self._stopped:
            raise RuntimeError("Cannot start a process that has been already stopped")

        log.debug("Starting process %s", self.name)
        self._proc.start()

        try:
            if not self.state.wait_for(lambda phase, _: phase not in (ProcessPhase.NOT_STARTED, ProcessPhase.UNINITIALIZED),
                                       self._config.start_timeout_sec):
                raise TimeoutError("Timeout when waiting for initialization")

            with self.state:
                # Propagate KeyboardInterrupt as is.
                if isinstance(self.state.exception, KeyboardInterrupt):
                    raise self.state.exception

                if self.state.exception is not None:
                    raise RuntimeError("Process failed initialization") from self.state.exception

                match self.state.phase:
                    case ProcessPhase.READY:
                        log.debug("Process %s started successfully", self.name)
                    case ProcessPhase.CLOSED:
                        raise RuntimeError("Process closed immediately after initialization")
                    case _:
                        raise RuntimeError(f"Process is in an unexpected state: {self.state.phase!r}")
        except BaseException as start_exc:
            if not isinstance(start_exc, KeyboardInterrupt):
                log.error("Stopping process %s on failure during initialization", self.name)
            try:
                self.stop()
            except KeyboardInterrupt:
                raise
            except BaseException as stop_exc:
                log.error("Error when stopping process %s after failure during initialization: %r", self.name, stop_exc)
                raise stop_exc.with_traceback(stop_exc.__traceback__) from start_exc
            raise

    def has_stopped(self, timeout: float) -> bool:
        self._proc.join(timeout)
        if not self._proc.is_alive():
            log.debug("Process %s got stopped", self.name)
            return True
        return False

    @with_annotated_exception
    def stop(self, *, cancel_queue: bool = False) -> None:
        """
        Stop the subprocess with escalating termination signals.

        The method waits for a graceful exit first (typically triggered when the work queue is cancelled by its owner). If still
        alive, it escalates to SIGINT, then SIGTERM, then SIGKILL.

        Work queue cancellation is optional, as the caller may have already cancelled it. If `cancel_queue` is True, the work queue
        is cancelled before attempting to stop the process.
        """
        if self._stopped:
            log.debug("Process %s is already stopped", self.name)
            return

        try:
            # First check if the process is alive before trying to join, as it is an error to attempt to join a process before it
            # has been started.
            if not self._proc.is_alive():
                log.debug("Process %s hasn't been started yet or has been already stopped", self.name)
                return

            # Wait for the external work queue to be cancelled by its owner or if cancel_queue is True. It should signal the process
            # to gracefully stop.
            if cancel_queue:
                self.work_queue.cancel()
            if self.has_stopped(self._config.stop_timeout_sec):
                return

            log.debug("Sending interrupt signal to process %s", self.name)
            if self._proc.pid is not None:
                os.kill(self._proc.pid, signal.SIGINT)  # TODO Python 3.14: self._proc.interrupt()
            if self.has_stopped(self._config.stop_timeout_sec):
                return

            log.warning("Timeout when waiting for process %s to stop. Sending terminate signal", self.name)
            self._proc.terminate()
            if self.has_stopped(self._config.termination_timeout_sec):
                return

            log.warning("Timeout when waiting for process %s to terminate. Sending kill signal", self.name)
            self._proc.kill()
            if self.has_stopped(self._config.termination_timeout_sec):
                return

            raise TimeoutError(f"Failed to terminate the process {self.name}. May become a zombie")
        finally:
            self._stopped = True
            if isinstance(self.state.exception, KeyboardInterrupt):
                raise self.state.exception
            if self.state.exception is not None:
                raise RuntimeError("Process reported an exception during execution") from self.state.exception

    # Methods run in the subprocess.

    def run(self) -> None:
        """
        Subprocess entrypoint coordinating init/work/cleanup and state.

        This method drives the lifecycle hooks in order: `_proc_init()` -> `_proc_work()`. It updates `state.phase` and captures
        unexpected exceptions in `state.exception`.

        We use several nested try/except blocks:
        1. The outermost block captures any exception that passed through other filters, ensures that it is stored in state and
           ensures that the process phase is set to CLOSED at the end.
        2. The middle block filters out expected exceptions related to normal process shutdown (like `QueueCancelled`), so that they
           are not treated as errors in scope of the exit stack.
        3. The innermost blocks around `_proc_init()` and `_proc_work()` enrich the exceptions with additional context about the
           failure stage.
        """
        try:
            # Signal that the process has started initialization.
            self.state.phase = ProcessPhase.UNINITIALIZED

            # Initialize global logger in the subprocess.
            self._config.log_config.set_fmt()

            with contextlib.ExitStack() as stack:
                try:
                    try:
                        log.debug("Initializing")
                        self._proc_init(stack)
                        self.state.phase = ProcessPhase.READY
                        log.debug("Initialized successfully")
                    except BaseException as e:
                        e.add_note("Failure during process initialization")
                        raise

                    try:
                        self._proc_work()
                    except BaseException as e:
                        e.add_note("Failure during process work")
                        raise

                # Capture exceptions that are not errors.
                except QueueCancelled:
                    log.warning("Received a cancel event")
                except EndOfQueue:
                    log.debug("Received end of work signal")
        except BaseException as e:
            if isinstance(e, KeyboardInterrupt):
                log.debug("Process interrupted by user")
            else:
                log.error("Process failed with an exception: %r", e)
            self.state.exception = e
        finally:
            self.state.phase = ProcessPhase.CLOSED

    @abstractmethod
    def _proc_init(self, exit_stack: contextlib.ExitStack) -> None:
        """Initialize subprocess resources before work begins.

        The long-lived resources that need cleanup should be registered in the provided `ExitStack`.
        """

    def _proc_work(self) -> None:
        """
        Perform the work.

        Default behavior waits for cancellation. Override to consume `self._work_queue` and publish activity by toggling
        `state.phase` between `READY` and `WORKING` as appropriate.
        """
        self.work_queue.wait_for_cancelled()
