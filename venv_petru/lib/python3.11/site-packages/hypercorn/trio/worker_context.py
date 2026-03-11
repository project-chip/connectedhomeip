from __future__ import annotations

from functools import wraps
from typing import Awaitable, Callable, Optional, Type, Union

import trio

from ..typing import Event, SingleTask, TaskGroup


def _cancel_wrapper(func: Callable[[], Awaitable[None]]) -> Callable[[], Awaitable[None]]:
    @wraps(func)
    async def wrapper(
        task_status: trio._core._run._TaskStatus = trio.TASK_STATUS_IGNORED,
    ) -> None:
        cancel_scope = trio.CancelScope()
        task_status.started(cancel_scope)
        with cancel_scope:
            await func()

    return wrapper


class TrioSingleTask:
    def __init__(self) -> None:
        self._handle: Optional[trio.CancelScope] = None
        self._lock = trio.Lock()

    async def restart(self, task_group: TaskGroup, action: Callable) -> None:
        async with self._lock:
            if self._handle is not None:
                self._handle.cancel()
            self._handle = await task_group._nursery.start(_cancel_wrapper(action))  # type: ignore

    async def stop(self) -> None:
        async with self._lock:
            if self._handle is not None:
                self._handle.cancel()
            self._handle = None


class EventWrapper:
    def __init__(self) -> None:
        self._event = trio.Event()

    async def clear(self) -> None:
        self._event = trio.Event()

    async def wait(self) -> None:
        await self._event.wait()

    async def set(self) -> None:
        self._event.set()

    def is_set(self) -> bool:
        return self._event.is_set()


class WorkerContext:
    event_class: Type[Event] = EventWrapper
    single_task_class: Type[SingleTask] = TrioSingleTask

    def __init__(self, max_requests: Optional[int]) -> None:
        self.max_requests = max_requests
        self.requests = 0
        self.terminate = self.event_class()
        self.terminated = self.event_class()

    async def mark_request(self) -> None:
        if self.max_requests is None:
            return

        self.requests += 1
        if self.requests > self.max_requests:
            await self.terminate.set()

    @staticmethod
    async def sleep(wait: Union[float, int]) -> None:
        return await trio.sleep(wait)

    @staticmethod
    def time() -> float:
        return trio.current_time()
