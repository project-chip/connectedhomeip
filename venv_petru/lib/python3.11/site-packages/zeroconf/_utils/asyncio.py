"""Multicast DNS Service Discovery for Python, v0.14-wmcbrine
Copyright 2003 Paul Scott-Murphy, 2014 William McBrine

This module provides a framework for the use of DNS Service Discovery
using IP multicast.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA
"""

from __future__ import annotations

import asyncio
import concurrent.futures
import contextlib
import sys
from collections.abc import Awaitable, Coroutine
from typing import Any

from .._exceptions import EventLoopBlocked
from ..const import _LOADED_SYSTEM_TIMEOUT
from .time import millis_to_seconds

# The combined timeouts should be lower than _CLOSE_TIMEOUT + _WAIT_FOR_LOOP_TASKS_TIMEOUT
_TASK_AWAIT_TIMEOUT = 1
_GET_ALL_TASKS_TIMEOUT = 3
_WAIT_FOR_LOOP_TASKS_TIMEOUT = 3  # Must be larger than _TASK_AWAIT_TIMEOUT


def _set_future_none_if_not_done(fut: asyncio.Future) -> None:
    """Set a future to None if it is not done."""
    if not fut.done():  # pragma: no branch
        fut.set_result(None)


def _resolve_all_futures_to_none(futures: set[asyncio.Future]) -> None:
    """Resolve all futures to None."""
    for fut in futures:
        _set_future_none_if_not_done(fut)
    futures.clear()


async def wait_for_future_set_or_timeout(
    loop: asyncio.AbstractEventLoop, future_set: set[asyncio.Future], timeout: float
) -> None:
    """Wait for a future or timeout (in milliseconds)."""
    future = loop.create_future()
    future_set.add(future)
    handle = loop.call_later(millis_to_seconds(timeout), _set_future_none_if_not_done, future)
    try:
        await future
    finally:
        handle.cancel()
        future_set.discard(future)


async def wait_future_or_timeout(future: asyncio.Future[bool | None], timeout: float) -> None:
    """Wait for a future or timeout."""
    loop = asyncio.get_running_loop()
    handle = loop.call_later(timeout, _set_future_none_if_not_done, future)
    try:
        await future
    except asyncio.CancelledError:
        if sys.version_info >= (3, 11) and (task := asyncio.current_task()) and task.cancelling():
            raise
    finally:
        handle.cancel()


async def _async_get_all_tasks(loop: asyncio.AbstractEventLoop) -> set[asyncio.Task]:
    """Return all tasks running."""
    await asyncio.sleep(0)  # flush out any call_soon_threadsafe
    # If there are multiple event loops running, all_tasks is not
    # safe EVEN WHEN CALLED FROM THE EVENTLOOP
    # under PyPy so we have to try a few times.
    for _ in range(3):
        with contextlib.suppress(RuntimeError):
            return asyncio.all_tasks(loop)
    return set()


async def _wait_for_loop_tasks(wait_tasks: set[asyncio.Task]) -> None:
    """Wait for the event loop thread we started to shutdown."""
    await asyncio.wait(wait_tasks, timeout=_TASK_AWAIT_TIMEOUT)


async def await_awaitable(aw: Awaitable) -> None:
    """Wait on an awaitable and the task it returns."""
    task = await aw
    await task


def run_coro_with_timeout(aw: Coroutine, loop: asyncio.AbstractEventLoop, timeout: float) -> Any:
    """Run a coroutine with a timeout.

    The timeout should only be used as a safeguard to prevent
    the program from blocking forever. The timeout should
    never be expected to be reached during normal operation.

    While not expected during normal operations, the
    function raises `EventLoopBlocked` if the coroutine takes
    longer to complete than the timeout.
    """
    try:
        return asyncio.run_coroutine_threadsafe(aw, loop).result(
            millis_to_seconds(timeout) + _LOADED_SYSTEM_TIMEOUT
        )
    except concurrent.futures.TimeoutError as ex:
        raise EventLoopBlocked from ex


def shutdown_loop(loop: asyncio.AbstractEventLoop) -> None:
    """Wait for pending tasks and stop an event loop."""
    pending_tasks = set(
        asyncio.run_coroutine_threadsafe(_async_get_all_tasks(loop), loop).result(_GET_ALL_TASKS_TIMEOUT)
    )
    pending_tasks -= {task for task in pending_tasks if task.done()}
    if pending_tasks:
        asyncio.run_coroutine_threadsafe(_wait_for_loop_tasks(pending_tasks), loop).result(
            _WAIT_FOR_LOOP_TASKS_TIMEOUT
        )
    loop.call_soon_threadsafe(loop.stop)


def get_running_loop() -> asyncio.AbstractEventLoop | None:
    """Check if an event loop is already running."""
    with contextlib.suppress(RuntimeError):
        return asyncio.get_running_loop()
    return None
