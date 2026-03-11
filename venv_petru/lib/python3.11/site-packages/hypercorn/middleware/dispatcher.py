from __future__ import annotations

import asyncio
from functools import partial
from typing import Callable, Dict

from ..asyncio.task_group import TaskGroup
from ..typing import ASGIFramework, Scope

MAX_QUEUE_SIZE = 10


class _DispatcherMiddleware:
    def __init__(self, mounts: Dict[str, ASGIFramework]) -> None:
        self.mounts = mounts

    async def __call__(self, scope: Scope, receive: Callable, send: Callable) -> None:
        if scope["type"] == "lifespan":
            await self._handle_lifespan(scope, receive, send)
        else:
            for path, app in self.mounts.items():
                if scope["path"].startswith(path):
                    scope["path"] = scope["path"][len(path) :] or "/"
                    return await app(scope, receive, send)
            await send(
                {
                    "type": "http.response.start",
                    "status": 404,
                    "headers": [(b"content-length", b"0")],
                }
            )
            await send({"type": "http.response.body"})

    async def _handle_lifespan(self, scope: Scope, receive: Callable, send: Callable) -> None:
        pass


class AsyncioDispatcherMiddleware(_DispatcherMiddleware):
    async def _handle_lifespan(self, scope: Scope, receive: Callable, send: Callable) -> None:
        self.app_queues: Dict[str, asyncio.Queue] = {
            path: asyncio.Queue(MAX_QUEUE_SIZE) for path in self.mounts
        }
        self.startup_complete = {path: False for path in self.mounts}
        self.shutdown_complete = {path: False for path in self.mounts}

        async with TaskGroup(asyncio.get_event_loop()) as task_group:
            for path, app in self.mounts.items():
                task_group.spawn(
                    app,
                    scope,
                    self.app_queues[path].get,
                    partial(self.send, path, send),
                )

            while True:
                message = await receive()
                for queue in self.app_queues.values():
                    await queue.put(message)
                if message["type"] == "lifespan.shutdown":
                    break

    async def send(self, path: str, send: Callable, message: dict) -> None:
        if message["type"] == "lifespan.startup.complete":
            self.startup_complete[path] = True
            if all(self.startup_complete.values()):
                await send({"type": "lifespan.startup.complete"})
        elif message["type"] == "lifespan.shutdown.complete":
            self.shutdown_complete[path] = True
            if all(self.shutdown_complete.values()):
                await send({"type": "lifespan.shutdown.complete"})


class TrioDispatcherMiddleware(_DispatcherMiddleware):
    async def _handle_lifespan(self, scope: Scope, receive: Callable, send: Callable) -> None:
        import trio

        self.app_queues = {path: trio.open_memory_channel(MAX_QUEUE_SIZE) for path in self.mounts}
        self.startup_complete = {path: False for path in self.mounts}
        self.shutdown_complete = {path: False for path in self.mounts}

        async with trio.open_nursery() as nursery:
            for path, app in self.mounts.items():
                nursery.start_soon(
                    app,
                    scope,
                    self.app_queues[path][1].receive,
                    partial(self.send, path, send),
                )

            while True:
                message = await receive()
                for channels in self.app_queues.values():
                    await channels[0].send(message)
                if message["type"] == "lifespan.shutdown":
                    break

    async def send(self, path: str, send: Callable, message: dict) -> None:
        if message["type"] == "lifespan.startup.complete":
            self.startup_complete[path] = True
            if all(self.startup_complete.values()):
                await send({"type": "lifespan.startup.complete"})
        elif message["type"] == "lifespan.shutdown.complete":
            self.shutdown_complete[path] = True
            if all(self.shutdown_complete.values()):
                await send({"type": "lifespan.shutdown.complete"})


DispatcherMiddleware = AsyncioDispatcherMiddleware  # Remove with version 0.11
