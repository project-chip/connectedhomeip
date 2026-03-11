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

import enum
import errno
import ipaddress
import socket
import struct
import sys
import warnings
from collections.abc import Iterable, Sequence
from typing import Any, Union, cast

import ifaddr

from .._logger import log
from ..const import _IPPROTO_IPV6, _MDNS_ADDR, _MDNS_ADDR6, _MDNS_PORT


@enum.unique
class InterfaceChoice(enum.Enum):
    Default = 1
    All = 2


InterfacesType = Union[Sequence[Union[str, int, tuple[tuple[str, int, int], int]]], InterfaceChoice]


@enum.unique
class ServiceStateChange(enum.Enum):
    Added = 1
    Removed = 2
    Updated = 3


@enum.unique
class IPVersion(enum.Enum):
    V4Only = 1
    V6Only = 2
    All = 3


# utility functions


def _is_v6_address(addr: bytes) -> bool:
    return len(addr) == 16


def _encode_address(address: str) -> bytes:
    is_ipv6 = ":" in address
    address_family = socket.AF_INET6 if is_ipv6 else socket.AF_INET
    return socket.inet_pton(address_family, address)


def get_all_addresses_ipv4(adapters: Iterable[ifaddr.Adapter]) -> list[str]:
    return list({addr.ip for iface in adapters for addr in iface.ips if addr.is_IPv4})  # type: ignore[misc]


def get_all_addresses_ipv6(adapters: Iterable[ifaddr.Adapter]) -> list[tuple[tuple[str, int, int], int]]:
    # IPv6 multicast uses positive indexes for interfaces
    # TODO: What about multi-address interfaces?
    return list(
        {(addr.ip, iface.index) for iface in adapters for addr in iface.ips if addr.is_IPv6}  # type: ignore[misc]
    )


def get_all_addresses() -> list[str]:
    warnings.warn(
        "get_all_addresses is deprecated, and will be removed in a future version. Use ifaddr"
        "directly instead to get a list of adapters.",
        DeprecationWarning,
        stacklevel=2,
    )
    return get_all_addresses_ipv4(ifaddr.get_adapters())


def get_all_addresses_v6() -> list[tuple[tuple[str, int, int], int]]:
    warnings.warn(
        "get_all_addresses_v6 is deprecated, and will be removed in a future version. Use ifaddr"
        "directly instead to get a list of adapters.",
        DeprecationWarning,
        stacklevel=2,
    )
    return get_all_addresses_ipv6(ifaddr.get_adapters())


def ip6_to_address_and_index(adapters: Iterable[ifaddr.Adapter], ip: str) -> tuple[tuple[str, int, int], int]:
    if "%" in ip:
        ip = ip[: ip.index("%")]  # Strip scope_id.
    ipaddr = ipaddress.ip_address(ip)
    for adapter in adapters:
        for adapter_ip in adapter.ips:
            # IPv6 addresses are represented as tuples
            if (
                adapter.index is not None
                and isinstance(adapter_ip.ip, tuple)
                and ipaddress.ip_address(adapter_ip.ip[0]) == ipaddr
            ):
                return (adapter_ip.ip, adapter.index)

    raise RuntimeError(f"No adapter found for IP address {ip}")


def interface_index_to_ip6_address(adapters: Iterable[ifaddr.Adapter], index: int) -> tuple[str, int, int]:
    for adapter in adapters:
        if adapter.index == index:
            for adapter_ip in adapter.ips:
                # IPv6 addresses are represented as tuples
                if isinstance(adapter_ip.ip, tuple):
                    return adapter_ip.ip

    raise RuntimeError(f"No adapter found for index {index}")


def ip6_addresses_to_indexes(
    interfaces: Sequence[str | int | tuple[tuple[str, int, int], int]],
) -> list[tuple[tuple[str, int, int], int]]:
    """Convert IPv6 interface addresses to interface indexes.

    IPv4 addresses are ignored.

    :param interfaces: List of IP addresses and indexes.
    :returns: List of indexes.
    """
    result = []
    adapters = ifaddr.get_adapters()

    for iface in interfaces:
        if isinstance(iface, int):
            result.append((interface_index_to_ip6_address(adapters, iface), iface))  # type: ignore[arg-type]
        elif isinstance(iface, str) and ipaddress.ip_address(iface).version == 6:
            result.append(ip6_to_address_and_index(adapters, iface))  # type: ignore[arg-type]

    return result


def normalize_interface_choice(
    choice: InterfacesType, ip_version: IPVersion = IPVersion.V4Only
) -> list[str | tuple[tuple[str, int, int], int]]:
    """Convert the interfaces choice into internal representation.

    :param choice: `InterfaceChoice` or list of interface addresses or indexes (IPv6 only).
    :param ip_address: IP version to use (ignored if `choice` is a list).
    :returns: List of IP addresses (for IPv4) and indexes (for IPv6).
    """
    result: list[str | tuple[tuple[str, int, int], int]] = []
    if choice is InterfaceChoice.Default:
        if ip_version != IPVersion.V4Only:
            # IPv6 multicast uses interface 0 to mean the default. However,
            # the default interface can't be used for outgoing IPv6 multicast
            # requests. In a way, interface choice default isn't really working
            # with IPv6. Inform the user accordingly.
            message = (
                "IPv6 multicast requests can't be sent using default interface. "
                "Use V4Only, InterfaceChoice.All or an explicit list of interfaces."
            )
            log.error(message)
            warnings.warn(message, DeprecationWarning, stacklevel=2)
            result.append((("::", 0, 0), 0))
        if ip_version != IPVersion.V6Only:
            result.append("0.0.0.0")
    elif choice is InterfaceChoice.All:
        adapters = ifaddr.get_adapters()
        if ip_version != IPVersion.V4Only:
            result.extend(get_all_addresses_ipv6(adapters))
        if ip_version != IPVersion.V6Only:
            result.extend(get_all_addresses_ipv4(adapters))
        if not result:
            raise RuntimeError(
                f"No interfaces to listen on, check that any interfaces have IP version {ip_version}"
            )
    elif isinstance(choice, list):
        # First, take IPv4 addresses.
        result = [i for i in choice if isinstance(i, str) and ipaddress.ip_address(i).version == 4]
        # Unlike IP_ADD_MEMBERSHIP, IPV6_JOIN_GROUP requires interface indexes.
        result += ip6_addresses_to_indexes(choice)
    else:
        raise TypeError(f"choice must be a list or InterfaceChoice, got {choice!r}")
    return result


def disable_ipv6_only_or_raise(s: socket.socket) -> None:
    """Make V6 sockets work for both V4 and V6 (required for Windows)."""
    try:
        s.setsockopt(_IPPROTO_IPV6, socket.IPV6_V6ONLY, False)
    except OSError:
        log.error("Support for dual V4-V6 sockets is not present, use IPVersion.V4 or IPVersion.V6")
        raise


def set_so_reuseport_if_available(s: socket.socket) -> None:
    """Set SO_REUSEADDR on a socket if available."""
    # SO_REUSEADDR should be equivalent to SO_REUSEPORT for
    # multicast UDP sockets (p 731, "TCP/IP Illustrated,
    # Volume 2"), but some BSD-derived systems require
    # SO_REUSEPORT to be specified explicitly.  Also, not all
    # versions of Python have SO_REUSEPORT available.
    # Catch OSError and socket.error for kernel versions <3.9 because lacking
    # SO_REUSEPORT support.
    if not hasattr(socket, "SO_REUSEPORT"):
        return

    try:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)  # pylint: disable=no-member
    except OSError as err:
        if err.errno != errno.ENOPROTOOPT:
            raise


def set_respond_socket_multicast_options(
    s: socket.socket,
    ip_version: IPVersion,
) -> None:
    """Set ttl/hops and loop for mDNS respond socket."""
    if ip_version == IPVersion.V4Only:
        # OpenBSD needs the ttl and loop values for the IP_MULTICAST_TTL and
        # IP_MULTICAST_LOOP socket options as an unsigned char.
        ttl = struct.pack(b"B", 255)
        loop = struct.pack(b"B", 1)
        s.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)
        s.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_LOOP, loop)
    elif ip_version == IPVersion.V6Only:
        # However, char doesn't work here (at least on Linux)
        s.setsockopt(_IPPROTO_IPV6, socket.IPV6_MULTICAST_HOPS, 255)
        s.setsockopt(_IPPROTO_IPV6, socket.IPV6_MULTICAST_LOOP, True)
    else:
        # A shared sender socket is not really possible, especially with link-local
        # multicast addresses (ff02::/16), the kernel needs to know which interface
        # to use for routing.
        #
        # It seems that macOS even refuses to take IPv4 socket options if this is an
        # AF_INET6 socket.
        #
        # In theory we could reconfigure the socket on each send, but that is not
        # really practical for Python Zerconf.
        raise RuntimeError("Dual-stack responder socket not supported")


def new_socket(
    bind_addr: tuple[str] | tuple[str, int, int],
    port: int = _MDNS_PORT,
    ip_version: IPVersion = IPVersion.V4Only,
    apple_p2p: bool = False,
) -> socket.socket | None:
    log.debug(
        "Creating new socket with port %s, ip_version %s, apple_p2p %s and bind_addr %r",
        port,
        ip_version,
        apple_p2p,
        bind_addr,
    )
    socket_family = socket.AF_INET if ip_version == IPVersion.V4Only else socket.AF_INET6
    s = socket.socket(socket_family, socket.SOCK_DGRAM)

    if ip_version == IPVersion.All:
        disable_ipv6_only_or_raise(s)

    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    set_so_reuseport_if_available(s)

    if apple_p2p:
        # SO_RECV_ANYIF = 0x1104
        # https://opensource.apple.com/source/xnu/xnu-4570.41.2/bsd/sys/socket.h
        s.setsockopt(socket.SOL_SOCKET, 0x1104, 1)

    # Bind expects (address, port) for AF_INET and (address, port, flowinfo, scope_id) for AF_INET6
    bind_tup = (bind_addr[0], port, *bind_addr[1:])
    try:
        s.bind(bind_tup)
    except OSError as ex:
        if ex.errno == errno.EADDRNOTAVAIL:
            log.warning(
                "Address not available when binding to %s, it is expected to happen on some systems",
                bind_tup,
            )
            return None
        if ex.errno == errno.EADDRINUSE:
            if sys.platform.startswith("darwin") or sys.platform.startswith("freebsd"):
                log.error(
                    "Address in use when binding to %s; "
                    "On BSD based systems sharing the same port with another "
                    "stack may require processes to run with the same UID; "
                    "When using avahi, make sure disallow-other-stacks is set"
                    " to no in avahi-daemon.conf",
                    bind_tup,
                )
            else:
                log.error(
                    "Address in use when binding to %s; "
                    "When using avahi, make sure disallow-other-stacks is set"
                    " to no in avahi-daemon.conf",
                    bind_tup,
                )
            # This is still a fatal error as its not going to work
            # if we can't hear the traffic coming in.
        raise
    log.debug("Created socket %s", s)
    return s


def add_multicast_member(
    listen_socket: socket.socket,
    interface: str | tuple[tuple[str, int, int], int],
) -> bool:
    # This is based on assumptions in normalize_interface_choice
    is_v6 = isinstance(interface, tuple)
    err_einval = {errno.EINVAL}
    if sys.platform == "win32":
        # No WSAEINVAL definition in typeshed
        err_einval |= {cast(Any, errno).WSAEINVAL}  # pylint: disable=no-member
    log.debug("Adding %r (socket %d) to multicast group", interface, listen_socket.fileno())
    try:
        if is_v6:
            try:
                mdns_addr6_bytes = socket.inet_pton(socket.AF_INET6, _MDNS_ADDR6)
            except OSError:
                log.info(
                    "Unable to translate IPv6 address when adding %s to multicast group, "
                    "this can happen if IPv6 is disabled on the system",
                    interface,
                )
                return False
            iface_bin = struct.pack("@I", cast(int, interface[1]))
            _value = mdns_addr6_bytes + iface_bin
            listen_socket.setsockopt(_IPPROTO_IPV6, socket.IPV6_JOIN_GROUP, _value)
        else:
            _value = socket.inet_aton(_MDNS_ADDR) + socket.inet_aton(cast(str, interface))
            listen_socket.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, _value)
    except OSError as e:
        _errno = get_errno(e)
        if _errno == errno.EADDRINUSE:
            log.info(
                "Address in use when adding %s to multicast group, it is expected to happen on some systems",
                interface,
            )
            return False
        if _errno == errno.ENOBUFS:
            # https://github.com/python-zeroconf/python-zeroconf/issues/1510
            if not is_v6 and sys.platform.startswith("linux"):
                log.warning(
                    "No buffer space available when adding %s to multicast group, "
                    "try increasing `net.ipv4.igmp_max_memberships` to `1024` in sysctl.conf",
                    interface,
                )
            else:
                log.warning(
                    "No buffer space available when adding %s to multicast group.",
                    interface,
                )
            return False
        if _errno == errno.EADDRNOTAVAIL:
            log.info(
                "Address not available when adding %s to multicast "
                "group, it is expected to happen on some systems",
                interface,
            )
            return False
        if _errno in err_einval:
            log.info(
                "Interface of %s does not support multicast, it is expected in WSL",
                interface,
            )
            return False
        if _errno == errno.ENOPROTOOPT:
            log.info(
                "Failed to set socket option on %s, this can happen if "
                "the network adapter is in a disconnected state",
                interface,
            )
            return False
        if is_v6 and _errno == errno.ENODEV:
            log.info(
                "Address in use when adding %s to multicast group, "
                "it is expected to happen when the device does not have ipv6",
                interface,
            )
            return False
        raise
    return True


def new_respond_socket(
    interface: str | tuple[tuple[str, int, int], int],
    apple_p2p: bool = False,
    unicast: bool = False,
) -> socket.socket | None:
    """Create interface specific socket for responding to multicast queries."""
    is_v6 = isinstance(interface, tuple)

    # For response sockets:
    # - Bind explicitly to the interface address
    # - Use ephemeral ports if in unicast mode
    # - Create socket according to the interface IP type (IPv4 or IPv6)
    respond_socket = new_socket(
        bind_addr=cast(tuple[tuple[str, int, int], int], interface)[0] if is_v6 else (cast(str, interface),),
        port=0 if unicast else _MDNS_PORT,
        ip_version=(IPVersion.V6Only if is_v6 else IPVersion.V4Only),
        apple_p2p=apple_p2p,
    )
    if unicast:
        return respond_socket

    if not respond_socket:
        return None

    log.debug("Configuring socket %s with multicast interface %s", respond_socket, interface)
    if is_v6:
        iface_bin = struct.pack("@I", cast(int, interface[1]))
        respond_socket.setsockopt(_IPPROTO_IPV6, socket.IPV6_MULTICAST_IF, iface_bin)
    else:
        respond_socket.setsockopt(
            socket.IPPROTO_IP,
            socket.IP_MULTICAST_IF,
            socket.inet_aton(cast(str, interface)),
        )
    set_respond_socket_multicast_options(respond_socket, IPVersion.V6Only if is_v6 else IPVersion.V4Only)
    return respond_socket


def create_sockets(
    interfaces: InterfacesType = InterfaceChoice.All,
    unicast: bool = False,
    ip_version: IPVersion = IPVersion.V4Only,
    apple_p2p: bool = False,
) -> tuple[socket.socket | None, list[socket.socket]]:
    if unicast:
        listen_socket = None
    else:
        listen_socket = new_socket(bind_addr=("",), ip_version=ip_version, apple_p2p=apple_p2p)

    normalized_interfaces = normalize_interface_choice(interfaces, ip_version)

    # If we are using InterfaceChoice.Default with only IPv4 or only IPv6, we can use
    # a single socket to listen and respond.
    if not unicast and interfaces is InterfaceChoice.Default and ip_version != IPVersion.All:
        for interface in normalized_interfaces:
            add_multicast_member(cast(socket.socket, listen_socket), interface)
        # Sent responder socket options to the dual-use listen socket
        set_respond_socket_multicast_options(cast(socket.socket, listen_socket), ip_version)
        return listen_socket, [cast(socket.socket, listen_socket)]

    respond_sockets = []

    for interface in normalized_interfaces:
        # Only create response socket if unicast or becoming multicast member was successful
        if not unicast and not add_multicast_member(cast(socket.socket, listen_socket), interface):
            continue

        respond_socket = new_respond_socket(interface, apple_p2p=apple_p2p, unicast=unicast)

        if respond_socket is not None:
            respond_sockets.append(respond_socket)

    return listen_socket, respond_sockets


def get_errno(e: OSError) -> int:
    return cast(int, e.args[0])


def can_send_to(ipv6_socket: bool, address: str) -> bool:
    """Check if the address type matches the socket type.

    This function does not validate if the address is a valid
    ipv6 or ipv4 address.
    """
    return ":" in address if ipv6_socket else ":" not in address


def autodetect_ip_version(interfaces: InterfacesType) -> IPVersion:
    """Auto detect the IP version when it is not provided."""
    if isinstance(interfaces, list):
        has_v6 = any(
            isinstance(i, int) or (isinstance(i, str) and ipaddress.ip_address(i).version == 6)
            for i in interfaces
        )
        has_v4 = any(isinstance(i, str) and ipaddress.ip_address(i).version == 4 for i in interfaces)
        if has_v4 and has_v6:
            return IPVersion.All
        if has_v6:
            return IPVersion.V6Only

    return IPVersion.V4Only
