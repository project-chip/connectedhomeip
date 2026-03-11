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

from functools import cache, lru_cache
from ipaddress import AddressValueError, IPv4Address, IPv6Address, NetmaskValueError
from typing import Any

from .._dns import DNSAddress
from ..const import _TYPE_AAAA

bytes_ = bytes
int_ = int


class ZeroconfIPv4Address(IPv4Address):
    __slots__ = ("__hash__", "_is_link_local", "_is_loopback", "_is_unspecified", "_str", "zc_integer")

    def __init__(self, *args: Any, **kwargs: Any) -> None:
        """Initialize a new IPv4 address."""
        super().__init__(*args, **kwargs)
        self._str = super().__str__()
        self._is_link_local = super().is_link_local
        self._is_unspecified = super().is_unspecified
        self._is_loopback = super().is_loopback
        self.__hash__ = cache(lambda: IPv4Address.__hash__(self))  # type: ignore[method-assign]
        self.zc_integer = int(self)

    def __str__(self) -> str:
        """Return the string representation of the IPv4 address."""
        return self._str

    @property
    def is_link_local(self) -> bool:
        """Return True if this is a link-local address."""
        return self._is_link_local

    @property
    def is_unspecified(self) -> bool:
        """Return True if this is an unspecified address."""
        return self._is_unspecified

    @property
    def is_loopback(self) -> bool:
        """Return True if this is a loop back."""
        return self._is_loopback


class ZeroconfIPv6Address(IPv6Address):
    __slots__ = ("__hash__", "_is_link_local", "_is_loopback", "_is_unspecified", "_str", "zc_integer")

    def __init__(self, *args: Any, **kwargs: Any) -> None:
        """Initialize a new IPv6 address."""
        super().__init__(*args, **kwargs)
        self._str = super().__str__()
        self._is_link_local = super().is_link_local
        self._is_unspecified = super().is_unspecified
        self._is_loopback = super().is_loopback
        self.__hash__ = cache(lambda: IPv6Address.__hash__(self))  # type: ignore[method-assign]
        self.zc_integer = int(self)

    def __str__(self) -> str:
        """Return the string representation of the IPv6 address."""
        return self._str

    @property
    def is_link_local(self) -> bool:
        """Return True if this is a link-local address."""
        return self._is_link_local

    @property
    def is_unspecified(self) -> bool:
        """Return True if this is an unspecified address."""
        return self._is_unspecified

    @property
    def is_loopback(self) -> bool:
        """Return True if this is a loop back."""
        return self._is_loopback


@lru_cache(maxsize=512)
def _cached_ip_addresses(
    address: str | bytes | int,
) -> ZeroconfIPv4Address | ZeroconfIPv6Address | None:
    """Cache IP addresses."""
    try:
        return ZeroconfIPv4Address(address)
    except (AddressValueError, NetmaskValueError):
        pass

    try:
        return ZeroconfIPv6Address(address)
    except (AddressValueError, NetmaskValueError):
        return None


cached_ip_addresses_wrapper = _cached_ip_addresses
cached_ip_addresses = cached_ip_addresses_wrapper


def get_ip_address_object_from_record(
    record: DNSAddress,
) -> ZeroconfIPv4Address | ZeroconfIPv6Address | None:
    """Get the IP address object from the record."""
    if record.type == _TYPE_AAAA and record.scope_id:
        return ip_bytes_and_scope_to_address(record.address, record.scope_id)
    return cached_ip_addresses_wrapper(record.address)


def ip_bytes_and_scope_to_address(
    address: bytes_, scope: int_
) -> ZeroconfIPv4Address | ZeroconfIPv6Address | None:
    """Convert the bytes and scope to an IP address object."""
    base_address = cached_ip_addresses_wrapper(address)
    if base_address is not None and base_address.is_link_local:
        # Avoid expensive __format__ call by using PyUnicode_Join
        return cached_ip_addresses_wrapper("".join((str(base_address), "%", str(scope))))
    return base_address


def str_without_scope_id(addr: ZeroconfIPv4Address | ZeroconfIPv6Address) -> str:
    """Return the string representation of the address without the scope id."""
    if addr.version == 6:
        address_str = str(addr)
        return address_str.partition("%")[0]
    return str(addr)


__all__ = (
    "cached_ip_addresses",
    "get_ip_address_object_from_record",
    "ip_bytes_and_scope_to_address",
    "str_without_scope_id",
)
