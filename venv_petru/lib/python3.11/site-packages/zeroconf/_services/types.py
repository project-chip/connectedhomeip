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

import time

from .._core import Zeroconf
from .._services import ServiceListener
from .._utils.net import InterfaceChoice, InterfacesType, IPVersion
from ..const import _SERVICE_TYPE_ENUMERATION_NAME
from .browser import ServiceBrowser


class ZeroconfServiceTypes(ServiceListener):
    """
    Return all of the advertised services on any local networks
    """

    def __init__(self) -> None:
        """Keep track of found services in a set."""
        self.found_services: set[str] = set()

    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        """Service added."""
        self.found_services.add(name)

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        """Service updated."""

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        """Service removed."""

    @classmethod
    def find(
        cls,
        zc: Zeroconf | None = None,
        timeout: int | float = 5,
        interfaces: InterfacesType = InterfaceChoice.All,
        ip_version: IPVersion | None = None,
    ) -> tuple[str, ...]:
        """
        Return all of the advertised services on any local networks.

        :param zc: Zeroconf() instance.  Pass in if already have an
                instance running or if non-default interfaces are needed
        :param timeout: seconds to wait for any responses
        :param interfaces: interfaces to listen on.
        :param ip_version: IP protocol version to use.
        :return: tuple of service type strings
        """
        local_zc = zc or Zeroconf(interfaces=interfaces, ip_version=ip_version)
        listener = cls()
        browser = ServiceBrowser(local_zc, _SERVICE_TYPE_ENUMERATION_NAME, listener=listener)

        # wait for responses
        time.sleep(timeout)

        browser.cancel()

        # close down anything we opened
        if zc is None:
            local_zc.close()

        return tuple(sorted(listener.found_services))
