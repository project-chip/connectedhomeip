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
    """Raised by CancellableQueue to indicate the end of work in the queue."""


QueueElementT = TypeVar("QueueElementT")


class CancellableQueue(Generic[QueueElementT]):
    """
    Queue that supports synchronized cancellation and end-of-work signaling.

    - Cancellation is immediate, i.e. if `cancel()` is called, any consumer waiting on `get()` will immediately receive a
      `QueueCancelled` exception, and any future calls to `put()` will also raise `QueueCancelled`.
    - End-of-queue is cooperative, i.e. if `close()` is called, any consumer waiting on `get()` will receive an `EndOfQueue`
      exception only after all items that are on the queue have been consumed. Any future calls to `put()` after `close()` will
      also raise `EndOfQueue` exception.

    It's not possible to "uncancel" or "un-close" a queue.
    """

    def __init__(self) -> None:
        self._cond = threading.Condition()
        self._queue: queue.Queue[QueueElementT] = queue.Queue()
        self._cancel_event = threading.Event()
        self._end_of_queue = threading.Event()

    def put(self, item: QueueElementT) -> None:
        """
        Put an item to the queue and notify a single consumer.

        Raises:
            QueueCancelled: if the queue is already cancelled.
            EndOfQueue: if the queue is already closed.
        """
        with self._cond:
            # Check for cancellation and end of queue before putting the item, to prevent adding new work to the queue.
            if self._cancel_event.is_set():
                raise QueueCancelled
            if self._end_of_queue.is_set():
                raise EndOfQueue

            self._queue.put(item)
            self._cond.notify()

    def get(self, timeout: float | None = None) -> QueueElementT:
        """
        Get an item from the queue.

        Raises:
            QueueCancelled: if cancellation event is observed.
            EndOfQueue: when there is no more work to do.
            TimeoutError: on timeout if `timeout` is not None.
            queue.Empty: if called with `timeout=0` and the queue is empty while the queue is neither cancelled nor closed.
        """
        with self._cond:
            if timeout != 0:
                # First check without waiting, to avoid race conditions and unnecessary waiting. Allows to pass QueueCancelled and
                # EndOfQueue exceptions to the consumer without waiting for the condition.
                with contextlib.suppress(queue.Empty):
                    return self.get(timeout=0)

                # We wait for the condition within the timeout (or infinitely when timeout=None).
                if not self._cond.wait(timeout):
                    raise TimeoutError("Timeout when waiting for queue item")

            # Check for cancel event.
            if self._cancel_event.is_set():
                raise QueueCancelled

            # Check for the end of queue sentinel.
            try:
                # Check if there is a new queue element. Use block=False since we've already synchronized via self._cond.
                return self._queue.get(block=False)
            except queue.Empty:
                # If there is no new queue element, check if the end of queue event is set. If not, re-raise the exception to
                # indicate an empty queue, which should not normally happen.
                if self._end_of_queue.is_set():
                    raise EndOfQueue
                raise

    def cancel(self) -> None:
        """Set cancel event and notify all consumers."""
        with self._cond:
            if self._cancel_event.is_set():
                return

            self._cancel_event.set()
            self._cond.notify_all()

    def close(self) -> None:
        """Set end-of-queue event and notify all consumers."""
        with self._cond:
            if self._end_of_queue.is_set():
                return

            self._end_of_queue.set()
            self._cond.notify_all()
