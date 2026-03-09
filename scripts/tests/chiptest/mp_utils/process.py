import functools
import logging
import os
import signal
from abc import ABC, abstractmethod
from collections.abc import Callable
from multiprocessing.context import SpawnContext
from multiprocessing.managers import SyncManager
from typing import Concatenate, Generic, ParamSpec, TypeVar

from chiptest.mp_utils.config import ProcessConfigTemplate
from chiptest.mp_utils.queue import EndOfWork, WorkQueue, WorkQueueCancelled
from chiptest.mp_utils.state import ProcessGroupState, ProcessState

log = logging.getLogger(__name__)

S = TypeVar("S", bound="WrappedProcess")
P = ParamSpec("P")
R = TypeVar("R")


def with_annotated_exception(fn: Callable[Concatenate[S, P], R]) -> Callable[Concatenate[S, P], R]:
    """Decorator to enrich exceptions from WrappedProcess methods with process information."""

    @functools.wraps(fn)
    def wrapper(self: S, *args: P.args, **kwargs: P.kwargs) -> R:
        try:
            return fn(self, *args, **kwargs)
        except BaseException as e:
            e.add_note(f"Exception in process {self._config.name}")
            raise

    return wrapper


ConfigT = TypeVar("ConfigT", bound=ProcessConfigTemplate)
WorkRequestT = TypeVar("WorkRequestT")
WorkResponseT = TypeVar("WorkResponseT")

class WrappedProcess(ABC, Generic[ConfigT, WorkRequestT, WorkResponseT]):
    # Methods run in the parent process.

    def __init__(self, mp_context: SpawnContext, mp_manager: SyncManager, config: ConfigT, work_queue: WorkQueue[WorkRequestT, WorkResponseT] | None = None,
                 group_state: ProcessGroupState | None = None) -> None:
        # Neither mp_context or mp_manager should be saved in the instance, as they are not picklable between processes but they can
        # be used to initialize some shared resources.

        # Create state and work queue.
        self._config = config
        self.state = ProcessState(mp_manager, config, group_state)

        self.work_queue: WorkQueue[WorkRequestT, WorkResponseT]
        if work_queue is None:
            self._external_work_queue = False
            self.work_queue = WorkQueue(mp_manager)
        else:
            self._external_work_queue = True
            self.work_queue = work_queue

        # Create multiprocessing.Process in the given context.
        self._proc = mp_context.Process(target=self.run, name=self._config.name)

    @property
    def name(self) -> str:
        return self._config.name

    @with_annotated_exception
    def start(self) -> None:
        log.debug("Starting process %s", self.name)
        self._proc.start()

        # No start timeout means that the caller doesn't want to wait for initialization to finish.
        if self._config.start_timeout is None:
            return

        try:
            if not self.state.wait_for(lambda phase: phase not in (ProcessState.Phase.NOT_STARTED,
                                                                   ProcessState.Phase.UNINITIALIZED), self._config.start_timeout):
                raise TimeoutError("Timeout when waiting for initialization")

            with self.state:
                match self.state.exception:
                    case KeyboardInterrupt():
                        raise KeyboardInterrupt()
                    case BaseException() as exception:
                        raise RuntimeError(f"Process failed initialization with exception: {exception!r}")

                match self.state.phase:
                    case ProcessState.Phase.READY:
                        log.debug("Process %s started successfully", self.name)
                    case ProcessState.Phase.CLOSED:
                        raise RuntimeError("Process closed immediately after initialization")
                    case _:
                        raise RuntimeError("Process is in unexpected state")
        except BaseException as e:
            if not isinstance(e, KeyboardInterrupt):
                log.exception("Stopping process %s on failure during inititialization", self.name)
            self.stop()
            raise

    @with_annotated_exception
    def stop(self) -> None:
        def has_stopped(timeout: float) -> bool:
            self._proc.join(timeout)
            if not self._proc.is_alive():
                log.debug("Process %s got stopped", self.name)
                return True
            return False

        if not self._proc.is_alive():
            log.debug("Process %s hasn't started yet or is already stopped", self.name)
            return

        if self._external_work_queue:
            # Wait for the external work queue to be cancelled by its owner, which should signal the process to stop.
            if has_stopped(self._config.stop_timeout):
                return
            log.debug("Sending interrupt signal")
        else:
            log.debug("Cancelling work in process %s", self.name)
            self.work_queue.req_cancel()
            self.work_queue.rsp_cancel()

            if has_stopped(self._config.stop_timeout):
                return
            log.info("Process is not responding to the cancel event. Sending interrupt signal")

        if self._proc.pid is not None:
            os.kill(self._proc.pid, signal.SIGINT)  # TODO Python 3.14: self.process.interrupt()
        if has_stopped(self._config.stop_timeout):
            return

        log.warning("Timeout when waiting for process to stop. Sending terminate signal")
        self._proc.terminate()
        if has_stopped(self._config.termination_timeout):
            return

        log.warning("Timeout when waiting for process to terminate. Sending kill signal")
        self._proc.kill()
        if has_stopped(self._config.termination_timeout):
            return

        raise TimeoutError(f"Failed to terminate the process {self._config.name}. May become a zombie")

    # Methods run in the subprocess.

    def run(self) -> None:
        try:
            self.state.phase = ProcessState.Phase.UNINITIALIZED

            # Initialize global logger in the subprocess.
            self._config.log_config.set_log_fmt()

            # Initialize.
            log.debug("Initializing")
            self._proc_init()
            self.state.phase = ProcessState.Phase.READY
            log.debug("Initialized successfully")

            # Perform work.
            self._proc_work()
        except EndOfWork:
            log.debug("Received end of work signal")
        except WorkQueueCancelled:
            log.debug("Received a cancel event")
        except BaseException as e:
            if isinstance(e, KeyboardInterrupt):
                log.debug("Caught an interrupt")
            self.state.exception = e
        finally:
            log.debug("Cleaning up")
            self._proc_cleanup()
            self.state.phase = ProcessState.Phase.CLOSED
            log.debug("Process finished cleanly")

    @abstractmethod
    def _proc_init(self):
        """Initialize the process.

        If there are some structures which are created, they should be stored as instance fields and cleaned up in _proc_cleanup().
        """

    def _proc_work(self) -> None:
        """Perform the work.

        Should wait for cancellation (default) or consume the work_queue while actively setting `Phase.READY` and `Phase.WORKING`.
        """
        self.work_queue.req_wait_for_cancel()

    @abstractmethod
    def _proc_cleanup(self):
        """Clean up the process state."""
