import contextlib
import queue
import threading
from abc import ABC, abstractmethod
from multiprocessing.managers import SyncManager
from types import TracebackType
from typing import Generic, TypeVar


class WorkQueueCancelled(Exception):
    """Raised by WorkQueue.get() when cancellation is observed."""

class EndOfWork(Exception):
    """Sentinel to indicate the end of work in the queue."""

QueueElementT = TypeVar("QueueElementT")


class CancellableQueue(ABC, Generic[QueueElementT]):
    def __init__(self, mp_manager: SyncManager, cancel_event: threading.Event | None = None):
        self._queue: queue.Queue[QueueElementT] = mp_manager.Queue()
        self._cond = mp_manager.Condition()
        self._cancel_event = cancel_event if cancel_event is not None else mp_manager.Event()

    def put(self, item: QueueElementT) -> None:
        """Put an item to the queue and notify one consumer."""
        with self._cond:
            self._queue.put(item)
            self._cond.notify()

    def get_or_cancel(self, timeout: float | None = None) -> QueueElementT:
        """Get an item from the queue, or raise WorkQueueCancelled if cancellation event is observed.

        If timeout is not None, also raise TimeoutError on timeout.

        First it performs a check without waiting. It avoids waiting for condition trigger when there are already items in the queue
        or cancellation is already observed.
        """
        with contextlib.suppress(queue.Empty):
            return self.get_or_cancel(timeout=0)

        with self._cond:
            if timeout is not None and timeout > 0 and not self._cond.wait(timeout=timeout):
                raise TimeoutError("Timeout when waiting for work item")
            return self._get_or_cancel_logic()

    @abstractmethod
    def _get_or_cancel_logic(self) -> QueueElementT:
        """Implement the logic to get an item from the queue, and raise WorkQueueCancelled if cancellation is observed.

        This method will be called by get_or_cancel() after waiting for the condition, and should not do any waiting itself.
        """

    def cancel(self) -> None:
        """Set cancel event and notify all consumers.

        If used as a member of WorkQueue, use WorkQueue.cancel() instead, which also takes care of canceling the ready queue.
        """
        with self._cond:
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
        if self._cancel_event.is_set():
            raise WorkQueueCancelled
        return self._queue.get_nowait()


class ResponseQueue(CancellableQueue[QueueElementT]):
    def _get_or_cancel_logic(self) -> QueueElementT:
        """Get a response item from the queue.

        In case of cancellation, first get all response items until the queue is empty, then raise WorkQueueCancelled. This allows
        to process all responses for already processed requests.
        """
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
        self._cancel_event = mp_manager.Event()
        self._req = tuple(RequestQueue[WorkRequestT | EndOfWork](mp_manager, self._cancel_event) for _ in range(req_count))
        self._rsp = ResponseQueue[WorkResponseT](mp_manager, self._cancel_event)

    def put_req(self, req: WorkRequestT | EndOfWork, req_queue_id: int | None = 0) -> None:
        if req_queue_id is not None:
            if req_queue_id > len(self._req):
                raise ValueError(f"No request queue with ID {req_queue_id}")
            self._req[req_queue_id].put(req)

        for req_queue in self._req:
            req_queue.put(req)

    def finalize_req(self, req_queue_id: int | None = None) -> None:
        return self.put_req(EndOfWork(), req_queue_id)

    def get_req_or_cancel(self, req_queue_id: int = 0, timeout: float | None = None) -> WorkRequestT:
        if isinstance(req := self._req[req_queue_id].get_or_cancel(timeout), EndOfWork):
            raise EndOfWork
        return req

    def put_rsp(self, rsp: WorkResponseT) -> None:
        self._rsp.put(rsp)

    def get_rsp_or_cancel(self, timeout: float | None = None) -> WorkResponseT:
        return self._rsp.get_or_cancel(timeout)

    def cancel(self) -> None:
        """Set cancel event and notify all consumers."""
        with contextlib.ExitStack() as stack, self._rsp:
            for req_queue in self._req:
                stack.enter_context(req_queue)

            self._cancel_event.set()

    def wait_for_cancel(self, timeout: float | None = None) -> bool:
        """Wait for cancellation. Returns True if the queue got cancelled, False on timeout."""
        return self._cancel_event.wait(timeout=timeout)
