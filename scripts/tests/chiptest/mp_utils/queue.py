import contextlib
import queue
import threading
from abc import ABC, abstractmethod
from multiprocessing.managers import SyncManager
from types import TracebackType
from typing import Generic, Protocol, TypeVar
import time


MP_MANAGED_WAIT_POLLING_S = 0.1
"""Polling interval for waiting on resources managed by multiprocessing.Manager."""


class Waitable(Protocol):
    def wait(self, timeout: float | None = None) -> bool: ...


def wait_for_mp_managed(waitable: Waitable, timeout: float | None = None) -> bool:
    """Wait for a resource managed by multiprocessing.Manager.

    Required because otherwise we wouldn't be able to catch a KeyboardInterrupt for a resource managed by multiprocessing.Manager,
    as the manager process explicitly ignores SIGINT.
    """
    # Blocking wait with no timeout. Cancellable only with a KeyboardInterrupt.
    if timeout is None:
        while not waitable.wait(MP_MANAGED_WAIT_POLLING_S):
            pass
        return True

    # Special case for non-positive timeout, to avoid waiting at all and return immediately.
    if timeout <= 0:
        return waitable.wait(timeout)

    # Countdown wait with polling, to be able to catch KeyboardInterrupt.
    start = time.monotonic()
    end = start + timeout
    while (time_left := end - time.monotonic()) > 0:
        if waitable.wait(min(MP_MANAGED_WAIT_POLLING_S, time_left)):
            return True

    return False


class WorkQueueCancelled(Exception):
    """Raised by WorkQueue.get() when cancellation is observed."""


class EndOfWork(Exception):
    """Sentinel to indicate the end of work in the queue."""


QueueElementT = TypeVar("QueueElementT")
"""Queue element, which can be anything but None."""


class CancellableQueue(ABC, Generic[QueueElementT]):
    def __init__(self, mp_manager: SyncManager, cancel_event: threading.Event | None = None):
        self._queue: queue.Queue[QueueElementT] = mp_manager.Queue()
        self._cond = mp_manager.Condition()

        if cancel_event is None:
            self._cancel_event = mp_manager.Event()
            self._external_cancel_event = False
        else:
            self._cancel_event = cancel_event
            self._external_cancel_event = True

    def put(self, item: QueueElementT) -> None:
        """Put an item to the queue and notify consumers.

        If the queue is already cancelled, raise WorkQueueCancelled instead.
        """
        with self._cond:
            if self._cancel_event.is_set():
                raise WorkQueueCancelled
            self._queue.put(item)
            self._cond.notify_all()

    def get_or_cancel(self, timeout: float | None = None) -> QueueElementT:
        """Get an item from the queue, or raise WorkQueueCancelled if cancellation event is observed.

        If timeout is not None, also raise TimeoutError on timeout.
        """
        with self._cond:
            # First check without waiting, to avoid unnecessary waiting and possible race conditions.
            with contextlib.suppress(queue.Empty):
                return self._get_or_cancel_logic()

            if not wait_for_mp_managed(self._cond, timeout):
                raise TimeoutError("Timeout when waiting for queue item")
            return self._get_or_cancel_logic()

    @abstractmethod
    def _get_or_cancel_logic(self) -> QueueElementT:
        """Implement the logic to get an item from the queue, and raise WorkQueueCancelled if cancellation is observed.

        This method will be called by get_or_cancel() after waiting for the condition, and should not do any waiting itself.
        """

    def cancel(self) -> None:
        """Set cancel event and notify all consumers.

        If used as a member of WorkQueue, use WorkQueue.cancel() instead, which also takes care of canceling the sub-queues.
        """
        if self._external_cancel_event:
            raise RuntimeError("Cannot cancel a queue with an external cancel event")

        with self._cond:
            if self._cancel_event.is_set():
                return

            self._cancel_event.set()
            self._cond.notify_all()

    def __enter__(self) -> bool:
        return self._cond.__enter__()

    def __exit__(self, exc_type: type[BaseException] | None, exc_val: BaseException | None, exc_tb: TracebackType | None):
        self._cond.notify_all()
        return self._cond.__exit__(exc_type, exc_val, exc_tb)


class RequestQueue(CancellableQueue[QueueElementT]):
    def _get_or_cancel_logic(self) -> QueueElementT:
        """Get a request item from the queue.

        Raise WorkQueueCancelled as soon as the cancellation event is set.
        """
        with self._cond:
            if self._cancel_event.is_set():
                raise WorkQueueCancelled
            return self._queue.get_nowait()


class ResponseQueue(CancellableQueue[QueueElementT]):
    def _get_or_cancel_logic(self) -> QueueElementT:
        """Get a response item from the queue.

        In case of cancellation, first get all response items until the queue is empty, then raise WorkQueueCancelled. This allows
        to process all responses for already processed requests.
        """
        with self._cond:
            try:
                return self._queue.get_nowait()
            except queue.Empty:
                if self._cancel_event.is_set():
                    raise WorkQueueCancelled
                raise


WorkRequestT = TypeVar("WorkRequestT")
WorkResponseT = TypeVar("WorkResponseT")


class WorkQueue(Generic[WorkRequestT, WorkResponseT]):
    """Work queue with separate request and response queues, and cancellation support.

    The class supports multiple request queues to allow different producers to have separate queues, while the response queue is
    shared to allow consumers to process responses in a single place. Cancellation is implemented with an event, and get() methods
    of the queues will raise WorkQueueCancelled when cancellation is observed. ResponseQueue.get_or_cancel() will first return all
    responses from the queue, and only then raise WorkQueueCancelled on cancellation, allowing to process all responses for already
    processed requests.
    """

    def __init__(self, mp_manager: SyncManager, req_count: int = 1) -> None:
        self._req_cancel_event = mp_manager.Event()
        self._req = tuple(RequestQueue[WorkRequestT | EndOfWork](mp_manager, self._req_cancel_event) for _ in range(req_count))

        self._rsp = ResponseQueue[WorkResponseT](mp_manager)

    def req_put(self, req: WorkRequestT | EndOfWork, req_queue_id: int | None = 0) -> None:
        """Put a request to the queue.

        If req_queue_id is not None, put to the corresponding request queue, otherwise put to all request queues (broadcast).
        """
        if req_queue_id is not None:
            # Single queue (more likely).
            if req_queue_id > len(self._req):
                raise ValueError(f"No request queue with ID {req_queue_id}")
            self._req[req_queue_id].put(req)
        else:
            # Broadcast to all request queues.
            for req_queue in self._req:
                req_queue.put(req)

    def req_put_or_cancel(self, req_queue_id: int = 0, timeout: float | None = None) -> WorkRequestT:
        if isinstance(req := self._req[req_queue_id].get_or_cancel(timeout), EndOfWork):
            raise EndOfWork
        return req

    def req_cancel(self) -> None:
        """Trigger cancel event for the request queues."""
        # Fast path check without acquiring the condition, to avoid unnecessary locking when cancellation is already observed.
        if self._req_cancel_event.is_set():
            return

        with contextlib.ExitStack() as stack:
            for req_queue in self._req:
                stack.enter_context(req_queue)

            self._req_cancel_event.set()

    def req_wait_for_cancel(self, timeout: float | None = None) -> bool:
        """Wait for cancellation. Returns True if the queue got cancelled, False on timeout."""
        return wait_for_mp_managed(self._req_cancel_event, timeout)

    def rsp_put(self, rsp: WorkResponseT) -> None:
        self._rsp.put(rsp)

    def rsp_get_or_cancel(self, timeout: float | None = None) -> WorkResponseT:
        return self._rsp.get_or_cancel(timeout)

    def rsp_cancel(self) -> None:
        """Trigger cancel event for the response queue."""
        self._rsp.cancel()
