from __future__ import annotations

import asyncio
from functools import partial
from typing import Any, Callable, Iterable

from ..app_wrappers import WSGIWrapper
from ..typing import ASGIReceiveCallable, ASGISendCallable, Scope, WSGIFramework

MAX_BODY_SIZE = 2**16

WSGICallable = Callable[[dict, Callable], Iterable[bytes]]


class InvalidPathError(Exception):
    pass


class _WSGIMiddleware:
    def __init__(self, wsgi_app: WSGIFramework, max_body_size: int = MAX_BODY_SIZE) -> None:
        self.wsgi_app = WSGIWrapper(wsgi_app, max_body_size)
        self.max_body_size = max_body_size

    async def __call__(
        self, scope: Scope, receive: ASGIReceiveCallable, send: ASGISendCallable
    ) -> None:
        pass


class AsyncioWSGIMiddleware(_WSGIMiddleware):
    async def __call__(
        self, scope: Scope, receive: ASGIReceiveCallable, send: ASGISendCallable
    ) -> None:
        loop = asyncio.get_event_loop()

        def _call_soon(func: Callable, *args: Any) -> Any:
            future = asyncio.run_coroutine_threadsafe(func(*args), loop)
            return future.result()

        await self.wsgi_app(scope, receive, send, partial(loop.run_in_executor, None), _call_soon)


class TrioWSGIMiddleware(_WSGIMiddleware):
    async def __call__(
        self, scope: Scope, receive: ASGIReceiveCallable, send: ASGISendCallable
    ) -> None:
        import trio

        await self.wsgi_app(scope, receive, send, trio.to_thread.run_sync, trio.from_thread.run)
