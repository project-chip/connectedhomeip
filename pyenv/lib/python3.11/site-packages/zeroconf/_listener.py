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
import logging
import random
from functools import partial
from typing import TYPE_CHECKING, cast

from ._logger import QuietLogger, log
from ._protocol.incoming import DNSIncoming
from ._transport import _WrappedTransport, make_wrapped_transport
from ._utils.time import current_time_millis, millis_to_seconds
from .const import _DUPLICATE_PACKET_SUPPRESSION_INTERVAL, _MAX_MSG_ABSOLUTE

if TYPE_CHECKING:
    from ._core import Zeroconf

_TC_DELAY_RANDOM_INTERVAL = (400, 500)


_bytes = bytes
_str = str
_int = int
_float = float

DEBUG_ENABLED = partial(log.isEnabledFor, logging.DEBUG)


class AsyncListener:
    """A Listener is used by this module to listen on the multicast
    group to which DNS messages are sent, allowing the implementation
    to cache information as it arrives.

    It requires registration with an Engine object in order to have
    the read() method called when a socket is available for reading."""

    __slots__ = (
        "_deferred",
        "_query_handler",
        "_record_manager",
        "_registry",
        "_timers",
        "data",
        "last_message",
        "last_time",
        "sock_description",
        "transport",
        "zc",
    )

    def __init__(self, zc: Zeroconf) -> None:
        self.zc = zc
        self._registry = zc.registry
        self._record_manager = zc.record_manager
        self._query_handler = zc.query_handler
        self.data: bytes | None = None
        self.last_time: float = 0
        self.last_message: DNSIncoming | None = None
        self.transport: _WrappedTransport | None = None
        self.sock_description: str | None = None
        self._deferred: dict[str, list[DNSIncoming]] = {}
        self._timers: dict[str, asyncio.TimerHandle] = {}
        super().__init__()

    def datagram_received(self, data: _bytes, addrs: tuple[str, int] | tuple[str, int, int, int]) -> None:
        data_len = len(data)
        debug = DEBUG_ENABLED()

        if data_len > _MAX_MSG_ABSOLUTE:
            # Guard against oversized packets to ensure bad implementations cannot overwhelm
            # the system.
            if debug:
                log.debug(
                    "Discarding incoming packet with length %s, which is larger "
                    "than the absolute maximum size of %s",
                    data_len,
                    _MAX_MSG_ABSOLUTE,
                )
            return
        now = current_time_millis()
        self._process_datagram_at_time(debug, data_len, now, data, addrs)

    def _process_datagram_at_time(
        self,
        debug: bool,
        data_len: _int,
        now: _float,
        data: _bytes,
        addrs: tuple[str, int] | tuple[str, int, int, int],
    ) -> None:
        if (
            self.data == data
            and (now - _DUPLICATE_PACKET_SUPPRESSION_INTERVAL) < self.last_time
            and self.last_message is not None
            and not self.last_message.has_qu_question()
        ):
            # Guard against duplicate packets
            if debug:
                log.debug(
                    "Ignoring duplicate message with no unicast questions"
                    " received from %s [socket %s] (%d bytes) as [%r]",
                    addrs,
                    self.sock_description,
                    data_len,
                    data,
                )
            return

        if len(addrs) == 2:
            v6_flow_scope: tuple[()] | tuple[int, int] = ()
            # https://github.com/python/mypy/issues/1178
            addr, port = addrs
            addr_port = addrs
            if TYPE_CHECKING:
                addr_port = cast(tuple[str, int], addr_port)
            scope = None
        else:
            # https://github.com/python/mypy/issues/1178
            addr, port, flow, scope = addrs
            if debug:  # pragma: no branch
                log.debug("IPv6 scope_id %d associated to the receiving interface", scope)
            v6_flow_scope = (flow, scope)
            addr_port = (addr, port)

        msg = DNSIncoming(data, addr_port, scope, now)
        self.data = data
        self.last_time = now
        self.last_message = msg
        if msg.valid is True:
            if debug:
                log.debug(
                    "Received from %r:%r [socket %s]: %r (%d bytes) as [%r]",
                    addr,
                    port,
                    self.sock_description,
                    msg,
                    data_len,
                    data,
                )
        else:
            if debug:
                log.debug(
                    "Received from %r:%r [socket %s]: (%d bytes) [%r]",
                    addr,
                    port,
                    self.sock_description,
                    data_len,
                    data,
                )
            return

        if not msg.is_query():
            self._record_manager.async_updates_from_response(msg)
            return

        if not self._registry.has_entries:
            # If the registry is empty, we have no answers to give.
            return

        if TYPE_CHECKING:
            assert self.transport is not None
        self.handle_query_or_defer(msg, addr, port, self.transport, v6_flow_scope)

    def handle_query_or_defer(
        self,
        msg: DNSIncoming,
        addr: _str,
        port: _int,
        transport: _WrappedTransport,
        v6_flow_scope: tuple[()] | tuple[int, int],
    ) -> None:
        """Deal with incoming query packets.  Provides a response if
        possible."""
        if not msg.truncated:
            self._respond_query(msg, addr, port, transport, v6_flow_scope)
            return

        deferred = self._deferred.setdefault(addr, [])
        # If we get the same packet we ignore it
        for incoming in reversed(deferred):
            if incoming.data == msg.data:
                return
        deferred.append(msg)
        delay = millis_to_seconds(random.randint(*_TC_DELAY_RANDOM_INTERVAL))  # noqa: S311
        loop = self.zc.loop
        assert loop is not None
        self._cancel_any_timers_for_addr(addr)
        self._timers[addr] = loop.call_at(
            loop.time() + delay,
            self._respond_query,
            None,
            addr,
            port,
            transport,
            v6_flow_scope,
        )

    def _cancel_any_timers_for_addr(self, addr: _str) -> None:
        """Cancel any future truncated packet timers for the address."""
        if addr in self._timers:
            self._timers.pop(addr).cancel()

    def _respond_query(
        self,
        msg: DNSIncoming | None,
        addr: _str,
        port: _int,
        transport: _WrappedTransport,
        v6_flow_scope: tuple[()] | tuple[int, int],
    ) -> None:
        """Respond to a query and reassemble any truncated deferred packets."""
        self._cancel_any_timers_for_addr(addr)
        packets = self._deferred.pop(addr, [])
        if msg:
            packets.append(msg)

        self._query_handler.handle_assembled_query(packets, addr, port, transport, v6_flow_scope)

    def error_received(self, exc: Exception) -> None:
        """Likely socket closed or IPv6."""
        # We preformat the message string with the socket as we want
        # log_exception_once to log a warning message once PER EACH
        # different socket in case there are problems with multiple
        # sockets
        msg_str = f"Error with socket {self.sock_description}): %s"
        QuietLogger.log_exception_once(exc, msg_str, exc)

    def connection_made(self, transport: asyncio.BaseTransport) -> None:
        wrapped_transport = make_wrapped_transport(cast(asyncio.DatagramTransport, transport))
        self.transport = wrapped_transport
        self.sock_description = f"{wrapped_transport.fileno} ({wrapped_transport.sock_name})"

    def connection_lost(self, exc: Exception | None) -> None:
        """Handle connection lost."""
