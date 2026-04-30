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

from chiptest.concurrency.work_queue import CancellableQueue, EndOfQueue, QueueCancelled
from chiptest.results import ResultQueueT, TestResult

log = logging.getLogger(__name__)


TaskQueueT: TypeAlias = CancellableQueue[Callable[[], TestResult]]


class WorkerThread(threading.Thread):
    """Worker thread that executes test jobs from the work queue and puts results to the result queue."""

    THREAD_TERMINATE_TIMEOUT_S: ClassVar[float] = 5.0

    def __init__(self, task_queue: TaskQueueT, result_queue: ResultQueueT) -> None:
        super().__init__(name="Worker")

        self._task_queue = task_queue
        self._result_queue = result_queue
        self.exception: BaseException | None = None

    def run(self) -> None:
        try:
            log.debug("Starting worker thread")
            while True:
                work_func = self._task_queue.get()
                self._result_queue.put(result := work_func())

                # Ensure to propagate KeyboardInterrupt if it's raised during execution, to prevent next test from running.
                # Otherwise, it's up to the result thread to decide whether to stop test execution.
                if isinstance(result.exception, KeyboardInterrupt):
                    raise result.exception
        except EndOfQueue:
            log.debug("Received end of work signal")
        except QueueCancelled:
            # While it's not expected for the work queue to be cancelled in normal flow, it's not a bug but a part of cleanup.
            log.warning("Received a cancel event")
        except BaseException as e:
            self.exception = e
        finally:
            log.debug("Worker thread finished")

    def terminate(self) -> None:
        # Immediately cancel the work queue to unblock the thread if it's waiting for work. In regular flow, the work queue is
        # expected to be externally closed instead, to allow for graceful shutdown. In that case, cancellation is effectively a
        # no-op, as the thread should be already stopped.
        self._task_queue.cancel()

        # Wait for the thread to finish if it had been started.
        if self.ident is not None:
            self.join(self.THREAD_TERMINATE_TIMEOUT_S)
            if self.is_alive():
                raise RuntimeError("Worker thread is still alive, it might be stuck on processing work items")
