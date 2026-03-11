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
import socket


class _WrappedTransport:
    """A wrapper for transports."""

    __slots__ = (
        "fileno",
        "is_ipv6",
        "sock",
        "sock_name",
        "transport",
    )

    def __init__(
        self,
        transport: asyncio.DatagramTransport,
        is_ipv6: bool,
        sock: socket.socket,
        fileno: int,
        sock_name: tuple,
    ) -> None:
        """Initialize the wrapped transport.

        These attributes are used when sending packets.
        """
        self.transport = transport
        self.is_ipv6 = is_ipv6
        self.sock = sock
        self.fileno = fileno
        self.sock_name = sock_name


def make_wrapped_transport(transport: asyncio.DatagramTransport) -> _WrappedTransport:
    """Make a wrapped transport."""
    sock: socket.socket = transport.get_extra_info("socket")
    return _WrappedTransport(
        transport=transport,
        is_ipv6=sock.family == socket.AF_INET6,
        sock=sock,
        fileno=sock.fileno(),
        sock_name=sock.getsockname(),
    )
