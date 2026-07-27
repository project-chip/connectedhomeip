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

import logging
from collections.abc import Callable
from concurrent.futures import ThreadPoolExecutor, as_completed
from multiprocessing.context import SpawnContext
from multiprocessing.managers import SyncManager
from typing import Generic, Literal, TypeVar

from chiptest.concurrency.config import ProcessConfig
from chiptest.concurrency.process import WrappedProcess
from chiptest.concurrency.state import ProcessGroupState
from chiptest.concurrency.work_queue import CancellableQueue

log = logging.getLogger(__name__)


WrappedProcessT = TypeVar("WrappedProcessT", bound=WrappedProcess)
ConfigT = TypeVar("ConfigT", bound=ProcessConfig)
WorkRequestT = TypeVar("WorkRequestT")
WorkResponseT = TypeVar("WorkResponseT")


class WrappedProcessPool(Generic[WrappedProcessT, ConfigT, WorkRequestT, WorkResponseT]):
    """A pool of wrapped processes that can be managed collectively.

    It has a single common response queue for all processes, and a shared ProcessGroupState for subprocess lifecycle coordination
    and exception propagation.
    """

    def __init__(self, process_cls: type[WrappedProcessT], mp_context: SpawnContext, mp_manager: SyncManager,
                 rsp_queue: CancellableQueue[WorkResponseT], concurrency: int, name: str, config_template: ConfigT) -> None:
        self._name = name
        self.config_template = config_template
        self.state = ProcessGroupState(mp_manager)

        self._pool: tuple[WrappedProcessT, ...] = tuple(
            process_cls(mp_context, mp_manager, config_template.with_formatted_name(process_id), CancellableQueue(mp_manager),
                        rsp_queue, self.state)
            for process_id in range(concurrency))
        self._rsp_queue = rsp_queue

    @property
    def name(self) -> str:
        """Return the name of the process pool.

        It is a property so that it can be used with StartStopMixinContext.
        """
        return self._name

    def collect_exceptions(self) -> Literal[True]:
        """Collect exceptions from all registered exception holders.

        Raise them as an ExceptionGroup if there are any, or return True if there are no exceptions, which allows to use this method
        in a predicate for wait_for.
        """
        return self.state.collect_exceptions()

    def _execute_for_all_workers(self, fn: Callable[[WrappedProcessT], None], exception_message: str,
                                 thread_name_prefix: str) -> None:
        """Execute the given function for all workers in the pool, in parallel threads.

        If any of the threads raises an exception, collect all exceptions and raise them as an ExceptionGroup.
        """
        exceptions: list[Exception] = []
        with ThreadPoolExecutor(max_workers=len(self._pool), thread_name_prefix=thread_name_prefix) as thread_pool:
            for result in as_completed(thread_pool.submit(lambda process: fn(process), process) for process in self._pool):
                try:
                    result.result()
                except Exception as e:
                    exceptions.append(e)
        if exceptions:
            raise ExceptionGroup(exception_message, exceptions)

    def start(self) -> None:
        """Start all workers in the pool."""
        log.info("Starting %s with concurrency of %i", self.name, len(self._pool))
        try:
            self._execute_for_all_workers(lambda process: process.start(), f"Failed to start {self.name}", "WorkerStart")
        except BaseException:
            self.stop()
            raise

    def close(self) -> None:
        """Close the work queues of all workers in the pool.

        This results in the workers finishing their work and deinitializing.
        """
        self._execute_for_all_workers(lambda process: process.work_queue.close(), f"Exception when closing {self.name}",
                                      "WorkerClose")

    def stop(self) -> None:
        """Stop all workers in the pool.

        This cancels their request queues and stops the processes, so it should be used only when the pool is no longer needed.
        """
        self.state.process_ready_queue.cancel()
        self._execute_for_all_workers(lambda process: process.stop(cancel_queue=True),
                                      f"Exception when cancelling request queues for {self.name}", "WorkerReqCancel")
