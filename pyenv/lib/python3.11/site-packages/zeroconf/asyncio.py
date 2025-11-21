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
import contextlib
from collections.abc import Awaitable
from types import TracebackType  # used in type hints
from typing import Callable

from ._core import Zeroconf
from ._dns import DNSQuestionType
from ._exceptions import NotRunningException
from ._services import ServiceListener
from ._services.browser import _ServiceBrowserBase
from ._services.info import AsyncServiceInfo, ServiceInfo
from ._services.types import ZeroconfServiceTypes
from ._utils.net import InterfaceChoice, InterfacesType, IPVersion
from .const import _BROWSER_TIME, _MDNS_PORT, _SERVICE_TYPE_ENUMERATION_NAME

__all__ = [
    "AsyncServiceBrowser",
    "AsyncServiceInfo",
    "AsyncZeroconf",
    "AsyncZeroconfServiceTypes",
]


class AsyncServiceBrowser(_ServiceBrowserBase):
    """Used to browse for a service for specific type(s).

    Constructor parameters are as follows:

    * `zc`: A Zeroconf instance
    * `type_`: fully qualified service type name
    * `handler`: ServiceListener or Callable that knows how to process ServiceStateChange events
    * `listener`: ServiceListener
    * `addr`: address to send queries (will default to multicast)
    * `port`: port to send queries (will default to mdns 5353)
    * `delay`: The initial delay between answering questions
    * `question_type`: The type of questions to ask (DNSQuestionType.QM or DNSQuestionType.QU)

    The listener object will have its add_service() and
    remove_service() methods called when this browser
    discovers changes in the services availability.
    """

    def __init__(
        self,
        zeroconf: Zeroconf,
        type_: str | list,
        handlers: ServiceListener | list[Callable[..., None]] | None = None,
        listener: ServiceListener | None = None,
        addr: str | None = None,
        port: int = _MDNS_PORT,
        delay: int = _BROWSER_TIME,
        question_type: DNSQuestionType | None = None,
    ) -> None:
        super().__init__(zeroconf, type_, handlers, listener, addr, port, delay, question_type)
        self._async_start()

    async def async_cancel(self) -> None:
        """Cancel the browser."""
        self._async_cancel()

    async def __aenter__(self) -> AsyncServiceBrowser:
        return self

    async def __aexit__(
        self,
        exc_type: type[BaseException] | None,
        exc_val: BaseException | None,
        exc_tb: TracebackType | None,
    ) -> bool | None:
        await self.async_cancel()
        return None


class AsyncZeroconfServiceTypes(ZeroconfServiceTypes):
    """An async version of ZeroconfServiceTypes."""

    @classmethod
    async def async_find(
        cls,
        aiozc: AsyncZeroconf | None = None,
        timeout: int | float = 5,
        interfaces: InterfacesType = InterfaceChoice.All,
        ip_version: IPVersion | None = None,
    ) -> tuple[str, ...]:
        """
        Return all of the advertised services on any local networks.

        :param aiozc: AsyncZeroconf() instance.  Pass in if already have an
                instance running or if non-default interfaces are needed
        :param timeout: seconds to wait for any responses
        :param interfaces: interfaces to listen on.
        :param ip_version: IP protocol version to use.
        :return: tuple of service type strings
        """
        local_zc = aiozc or AsyncZeroconf(interfaces=interfaces, ip_version=ip_version)
        listener = cls()
        async_browser = AsyncServiceBrowser(
            local_zc.zeroconf, _SERVICE_TYPE_ENUMERATION_NAME, listener=listener
        )

        # wait for responses
        await asyncio.sleep(timeout)

        await async_browser.async_cancel()

        # close down anything we opened
        if aiozc is None:
            await local_zc.async_close()

        return tuple(sorted(listener.found_services))


class AsyncZeroconf:
    """Implementation of Zeroconf Multicast DNS Service Discovery

    Supports registration, unregistration, queries and browsing.

    The async version is currently a wrapper around Zeroconf which
    is now also async. It is expected that an asyncio event loop
    is already running before creating the AsyncZeroconf object.
    """

    def __init__(
        self,
        interfaces: InterfacesType = InterfaceChoice.All,
        unicast: bool = False,
        ip_version: IPVersion | None = None,
        apple_p2p: bool = False,
        zc: Zeroconf | None = None,
    ) -> None:
        """Creates an instance of the Zeroconf class, establishing
        multicast communications, and listening.

        :param interfaces: :class:`InterfaceChoice` or a list of IP addresses
            (IPv4 and IPv6) and interface indexes (IPv6 only).

            IPv6 notes for non-POSIX systems:
            * `InterfaceChoice.All` is an alias for `InterfaceChoice.Default`
              on Python versions before 3.8.

            Also listening on loopback (``::1``) doesn't work, use a real address.
        :param ip_version: IP versions to support. If `choice` is a list, the default is detected
            from it. Otherwise defaults to V4 only for backward compatibility.
        :param apple_p2p: use AWDL interface (only macOS)
        """
        self.zeroconf = zc or Zeroconf(
            interfaces=interfaces,
            unicast=unicast,
            ip_version=ip_version,
            apple_p2p=apple_p2p,
        )
        self.async_browsers: dict[ServiceListener, AsyncServiceBrowser] = {}

    async def async_register_service(
        self,
        info: ServiceInfo,
        ttl: int | None = None,
        allow_name_change: bool = False,
        cooperating_responders: bool = False,
        strict: bool = True,
    ) -> Awaitable:
        """Registers service information to the network with a default TTL.
        Zeroconf will then respond to requests for information for that
        service.  The name of the service may be changed if needed to make
        it unique on the network. Additionally multiple cooperating responders
        can register the same service on the network for resilience
        (if you want this behavior set `cooperating_responders` to `True`).

        The service will be broadcast in a task. This task is returned
        and therefore can be awaited if necessary.
        """
        return await self.zeroconf.async_register_service(
            info, ttl, allow_name_change, cooperating_responders, strict
        )

    async def async_unregister_all_services(self) -> None:
        """Unregister all registered services.

        Unlike async_register_service and async_unregister_service, this
        method does not return a future and is always expected to be
        awaited since its only called at shutdown.
        """
        await self.zeroconf.async_unregister_all_services()

    async def async_unregister_service(self, info: ServiceInfo) -> Awaitable:
        """Unregister a service.

        The service will be broadcast in a task. This task is returned
        and therefore can be awaited if necessary.
        """
        return await self.zeroconf.async_unregister_service(info)

    async def async_update_service(self, info: ServiceInfo) -> Awaitable:
        """Registers service information to the network with a default TTL.
        Zeroconf will then respond to requests for information for that
        service.

        The service will be broadcast in a task. This task is returned
        and therefore can be awaited if necessary.
        """
        return await self.zeroconf.async_update_service(info)

    async def async_close(self) -> None:
        """Ends the background threads, and prevent this instance from
        servicing further queries."""
        if not self.zeroconf.done:
            with contextlib.suppress(NotRunningException):
                await self.zeroconf.async_wait_for_start(timeout=1.0)
        await self.async_remove_all_service_listeners()
        await self.async_unregister_all_services()
        await self.zeroconf._async_close()  # pylint: disable=protected-access

    async def async_get_service_info(
        self,
        type_: str,
        name: str,
        timeout: int = 3000,
        question_type: DNSQuestionType | None = None,
    ) -> AsyncServiceInfo | None:
        """Returns network's service information for a particular
        name and type, or None if no service matches by the timeout,
        which defaults to 3 seconds.

        :param type_: fully qualified service type name
        :param name: the name of the service
        :param timeout: milliseconds to wait for a response
        :param question_type: The type of questions to ask (DNSQuestionType.QM or DNSQuestionType.QU)
        """
        return await self.zeroconf.async_get_service_info(type_, name, timeout, question_type)

    async def async_add_service_listener(self, type_: str, listener: ServiceListener) -> None:
        """Adds a listener for a particular service type.  This object
        will then have its add_service and remove_service methods called when
        services of that type become available and unavailable."""
        await self.async_remove_service_listener(listener)
        self.async_browsers[listener] = AsyncServiceBrowser(self.zeroconf, type_, listener)

    async def async_remove_service_listener(self, listener: ServiceListener) -> None:
        """Removes a listener from the set that is currently listening."""
        if listener in self.async_browsers:
            await self.async_browsers[listener].async_cancel()
            del self.async_browsers[listener]

    async def async_remove_all_service_listeners(self) -> None:
        """Removes a listener from the set that is currently listening."""
        await asyncio.gather(
            *(self.async_remove_service_listener(listener) for listener in list(self.async_browsers))
        )

    async def __aenter__(self) -> AsyncZeroconf:
        return self

    async def __aexit__(
        self,
        exc_type: type[BaseException] | None,
        exc_val: BaseException | None,
        exc_tb: TracebackType | None,
    ) -> bool | None:
        await self.async_close()
        return None
