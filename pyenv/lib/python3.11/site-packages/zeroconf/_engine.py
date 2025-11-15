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
import itertools
import socket
import threading
from typing import TYPE_CHECKING, cast

from ._record_update import RecordUpdate
from ._utils.asyncio import get_running_loop, run_coro_with_timeout
from ._utils.time import current_time_millis
from .const import _CACHE_CLEANUP_INTERVAL

if TYPE_CHECKING:
    from ._core import Zeroconf


from ._listener import AsyncListener
from ._transport import _WrappedTransport, make_wrapped_transport

_CLOSE_TIMEOUT = 3000  # ms


class AsyncEngine:
    """An engine wraps sockets in the event loop."""

    __slots__ = (
        "_cleanup_timer",
        "_listen_socket",
        "_respond_sockets",
        "_setup_task",
        "loop",
        "protocols",
        "readers",
        "running_future",
        "senders",
        "zc",
    )

    def __init__(
        self,
        zeroconf: Zeroconf,
        listen_socket: socket.socket | None,
        respond_sockets: list[socket.socket],
    ) -> None:
        self.loop: asyncio.AbstractEventLoop | None = None
        self.zc = zeroconf
        self.protocols: list[AsyncListener] = []
        self.readers: list[_WrappedTransport] = []
        self.senders: list[_WrappedTransport] = []
        self.running_future: asyncio.Future[bool | None] | None = None
        self._listen_socket = listen_socket
        self._respond_sockets = respond_sockets
        self._cleanup_timer: asyncio.TimerHandle | None = None
        self._setup_task: asyncio.Task[None] | None = None

    def setup(
        self,
        loop: asyncio.AbstractEventLoop,
        loop_thread_ready: threading.Event | None,
    ) -> None:
        """Set up the instance."""
        self.loop = loop
        self.running_future = loop.create_future()
        self._setup_task = self.loop.create_task(self._async_setup(loop_thread_ready))

    async def _async_setup(self, loop_thread_ready: threading.Event | None) -> None:
        """Set up the instance."""
        self._async_schedule_next_cache_cleanup()
        await self._async_create_endpoints()
        assert self.running_future is not None
        if not self.running_future.done():
            self.running_future.set_result(True)
        if loop_thread_ready:
            loop_thread_ready.set()

    async def _async_create_endpoints(self) -> None:
        """Create endpoints to send and receive."""
        assert self.loop is not None
        loop = self.loop
        reader_sockets = []
        sender_sockets = []
        if self._listen_socket:
            reader_sockets.append(self._listen_socket)
        for s in self._respond_sockets:
            if s not in reader_sockets:
                reader_sockets.append(s)
            sender_sockets.append(s)

        for s in reader_sockets:
            transport, protocol = await loop.create_datagram_endpoint(  # type: ignore[type-var]
                lambda: AsyncListener(self.zc),  # type: ignore[arg-type, return-value]
                sock=s,
            )
            self.protocols.append(cast(AsyncListener, protocol))
            self.readers.append(make_wrapped_transport(cast(asyncio.DatagramTransport, transport)))
            if s in sender_sockets:
                self.senders.append(make_wrapped_transport(cast(asyncio.DatagramTransport, transport)))

    def _async_cache_cleanup(self) -> None:
        """Periodic cache cleanup."""
        now = current_time_millis()
        self.zc.question_history.async_expire(now)
        self.zc.record_manager.async_updates(
            now,
            [RecordUpdate(record, record) for record in self.zc.cache.async_expire(now)],
        )
        self.zc.record_manager.async_updates_complete(False)
        self._async_schedule_next_cache_cleanup()

    def _async_schedule_next_cache_cleanup(self) -> None:
        """Schedule the next cache cleanup."""
        loop = self.loop
        assert loop is not None
        self._cleanup_timer = loop.call_at(loop.time() + _CACHE_CLEANUP_INTERVAL, self._async_cache_cleanup)

    async def _async_close(self) -> None:
        """Cancel and wait for the cleanup task to finish."""
        assert self._setup_task is not None
        await self._setup_task
        self._async_shutdown()
        await asyncio.sleep(0)  # flush out any call soons
        assert self._cleanup_timer is not None
        self._cleanup_timer.cancel()

    def _async_shutdown(self) -> None:
        """Shutdown transports and sockets."""
        assert self.running_future is not None
        assert self.loop is not None
        self.running_future = self.loop.create_future()
        for wrapped_transport in itertools.chain(self.senders, self.readers):
            wrapped_transport.transport.close()

    def close(self) -> None:
        """Close from sync context.

        While it is not expected during normal operation,
        this function may raise EventLoopBlocked if the underlying
        call to `_async_close` cannot be completed.
        """
        assert self.loop is not None
        # Guard against Zeroconf.close() being called from the eventloop
        if get_running_loop() == self.loop:
            self._async_shutdown()
            return
        if not self.loop.is_running():
            return
        run_coro_with_timeout(self._async_close(), self.loop, _CLOSE_TIMEOUT)
