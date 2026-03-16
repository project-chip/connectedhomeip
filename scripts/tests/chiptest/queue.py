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
import queue
import threading
from typing import Generic, TypeVar


class QueueCancelled(Exception):
    """Raised by CancellableQueue when cancellation is observed."""


class EndOfQueue(Exception):
    """Sentinel to indicate the end of work in the queue."""


QueueElementT = TypeVar("QueueElementT")


class CancellableQueue(Generic[QueueElementT]):
    """Queue that supports synchronized cancellation and end-of-queue signaling."""

    def __init__(self) -> None:
        self._cond = threading.Condition()
        self._queue: queue.Queue[QueueElementT | EndOfQueue] = queue.Queue()
        self._cancel_event = threading.Event()

    def put(self, item: QueueElementT | EndOfQueue) -> None:
        """
        Put an item to the queue and notify a single consumer.

        Raises:
            QueueCancelled: if the queue is already cancelled.
        """
        with self._cond:
            if self._cancel_event.is_set():
                raise QueueCancelled
            self._queue.put(item)
            self._cond.notify()

    def get(self, timeout: float | None = None) -> QueueElementT:
        """
        Get an item from the queue.

        Raises:
            QueueCancelled: if cancellation event is observed.
            EndOfQueue: when there is no more work to do.
            TimeoutError: on timeout if `timeout` is not None.
            queue.Empty: if the queue is empty and `timeout` is 0.
        """
        with self._cond:
            if timeout != 0:
                # First check without waiting, to avoid race conditions and unnecessary waiting.
                with contextlib.suppress(queue.Empty):
                    return self.get(timeout=0)

                # Now, we wait for the condition within the timeout (or infinitely for timeout=None).
                if not self._cond.wait(timeout):
                    raise TimeoutError("Timeout when waiting for queue item")

            # Check for cancel event.
            if self._cancel_event.is_set():
                raise QueueCancelled

            # Check for the end of queue sentinel. Use block=False since we've already synchronized via self._cond.
            if isinstance(item := self._queue.get(block=False), EndOfQueue):
                raise item

            return item

    def cancel(self) -> None:
        """Set cancel event and notify all consumers."""
        with self._cond:
            if self._cancel_event.is_set():
                return

            self._cancel_event.set()
            self._cond.notify_all()

    def close(self) -> None:
        """Put an EndOfQueue sentinel to the queue to indicate no more work."""
        self.put(EndOfQueue())
