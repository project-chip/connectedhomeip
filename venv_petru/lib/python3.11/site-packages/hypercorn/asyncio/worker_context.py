from __future__ import annotations

import asyncio
from typing import Callable, Optional, Type, Union

from ..typing import Event, SingleTask, TaskGroup


class AsyncioSingleTask:
    def __init__(self) -> None:
        self._handle: Optional[asyncio.Task] = None
        self._lock = asyncio.Lock()

    async def restart(self, task_group: TaskGroup, action: Callable) -> None:
        async with self._lock:
            if self._handle is not None:
                self._handle.cancel()
                try:
                    await self._handle
                except asyncio.CancelledError:
                    pass

            self._handle = task_group._task_group.create_task(action())  # type: ignore

    async def stop(self) -> None:
        async with self._lock:
            if self._handle is not None:
                self._handle.cancel()
                try:
                    await self._handle
                except asyncio.CancelledError:
                    pass

            self._handle = None


class EventWrapper:
    def __init__(self) -> None:
        self._event = asyncio.Event()

    async def clear(self) -> None:
        self._event.clear()

    async def wait(self) -> None:
        await self._event.wait()

    async def set(self) -> None:
        self._event.set()

    def is_set(self) -> bool:
        return self._event.is_set()


class WorkerContext:
    event_class: Type[Event] = EventWrapper
    single_task_class: Type[SingleTask] = AsyncioSingleTask

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
        return await asyncio.sleep(wait)

    @staticmethod
    def time() -> float:
        return asyncio.get_event_loop().time()
