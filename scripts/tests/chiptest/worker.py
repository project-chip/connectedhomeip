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
import threading
from collections.abc import Callable
from typing import ClassVar, TypeAlias

from chiptest.queue import CancellableQueue, EndOfQueue, QueueCancelled
from chiptest.results import ResultQueueT, TestResult

log = logging.getLogger(__name__)


WorkQueueT: TypeAlias = CancellableQueue[Callable[[], TestResult]]


class WorkerThread(threading.Thread):
    """Worker thread that executes test jobs from the work queue and puts results to the result queue."""

    THREAD_TERMINATE_TIMEOUT_S: ClassVar[float] = 5.0

    def __init__(self, work_queue: WorkQueueT, result_queue: ResultQueueT) -> None:
        super().__init__(name="Worker")

        self._work_queue = work_queue
        self._result_queue = result_queue
        self.exception: BaseException | None = None

    def run(self) -> None:
        log.debug("Starting worker thread")
        try:
            while True:
                work_func = self._work_queue.get()
                self._result_queue.put(result := work_func())

                # Ensure to propagate KeyboardInterrupt if it's raised during execution, to prevent next test from running.
                # Otherwise, it's up to the result thread to decide whether to stop test execution.
                if isinstance(result.exception, KeyboardInterrupt):
                    raise result.exception
        except EndOfQueue:
            log.debug("Received end of work signal")
        except QueueCancelled:
            log.info("Received a cancel event")
        except BaseException as e:
            self.exception = e

    def terminate(self) -> None:
        # Cancel the work queue to unblock the thread if it's waiting for work, and prevent putting new work to the queue.
        self._work_queue.cancel()

        # Wait for the thread to finish.
        self.join(self.THREAD_TERMINATE_TIMEOUT_S)
        if self.is_alive():
            log.warning("Result processing thread is still alive, it might be stuck on processing results")
