#
#    Copyright (c) 2025 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import socket
from dataclasses import dataclass, field
from ipaddress import IPv6Address
from typing import Optional, Union

from zeroconf._utils.ipaddress import ZeroconfIPv6Address

from .json_serializable import JsonSerializable


@dataclass
class AddressTypeInfo:
    """
    Represents general classification flags for an IP address.
    Each flag indicates whether the address belongs to a specific
    address category defined in IPv4/IPv6 standards.
    """
    # True if the address is globally routable on the Internet
    # (e.g., IPv6 2000::/3 or IPv4 public ranges).
    is_global: bool

    # True if the address is link-local (usable only on the local link/segment).
    # IPv6: fe80::/10, IPv4: 169.254.0.0/16.
    is_link_local: bool

    # True if the address is a loopback address.
    # IPv6: ::1/128, IPv4: 127.0.0.0/8 (commonly 127.0.0.1).
    is_loopback: bool

    # True if the address is multicast.
    # IPv6: ff00::/8, IPv4: 224.0.0.0/4.
    is_multicast: bool


@dataclass
class SpecialAddressTypes:
    """
    Represents special categories of IPv6 addresses that can be derived
    from or mapped to IPv4. Used to identify and annotate IPv6 addresses
    with special handling or semantics.
    """
    # IPv6 address derived from a Teredo tunnel (2001::/32 range).
    # Used for NAT traversal — wraps IPv4 inside IPv6.
    teredo: Optional[str]

    # IPv6 address derived from 6to4 tunneling (2002::/16 range).
    # Automatically maps an IPv4 into an IPv6 address.
    sixtofour: Optional[str]

    # Whether the address falls into a reserved IPv6 block
    # (e.g., ::/128, ::1/128, ::ffff:0:0/96, etc.).
    is_reserved: bool

    # An IPv6 address that is explicitly mapped to an IPv4 address
    # (::ffff:0:0/96 prefix). Example: ::ffff:192.0.2.128
    ipv4_mapped: Optional[str]


@dataclass
class AddressMeta:
    """
    Metadata about an IP address, capturing low-level details useful
    for classification, serialization, or storage.
    """
    # The maximum number of bits in the address prefix.
    # IPv4 = 32, IPv6 = 128.
    max_prefixlen: int

    # Binary representation of the address, stored as a string.
    # Typically corresponds to the output of ipaddress.packed,
    # which gives the raw bytes of the address.
    packed: str


@dataclass
class AaaaRecord(JsonSerializable):
    ipv6: ZeroconfIPv6Address = field(repr=False, compare=False)
    address: str = field(init=False)
    version: int = field(init=False)
    interface: Union[str, int] = field(init=False)

    type_info: AddressTypeInfo = field(init=False)
    special_types: SpecialAddressTypes = field(init=False)
    meta: AddressMeta = field(init=False)

    def __post_init__(self):
        ip = self.ipv6
        self.address = get_valid_compressed_ipv6(ip)
        self.version = ip.version
        self.interface = get_interface(ip.scope_id)

        self.type_info = AddressTypeInfo(
            is_global=ip.is_global,
            is_link_local=ip.is_link_local,
            is_loopback=ip.is_loopback,
            is_multicast=ip.is_multicast,
        )

        self.special_types = SpecialAddressTypes(
            teredo=str(ip.teredo) if ip.teredo else None,
            sixtofour=str(ip.sixtofour) if ip.sixtofour else None,
            is_reserved=ip.is_reserved,
            ipv4_mapped=str(ip.ipv4_mapped) if ip.ipv4_mapped else None,
        )

        self.meta = AddressMeta(
            max_prefixlen=ip.max_prefixlen,
            packed=ip.packed.hex(),
        )


def get_valid_compressed_ipv6(ipv6: IPv6Address) -> str:
    """
    Strips the zone index (e.g., %eth0) from an IPv6 address and returns the compressed form.
    Validates the address format and raises if invalid.
    """
    # This raises if format is invalid. Drop everything after the first `%` if one exists.
    return str(IPv6Address(str(ipv6).split('%', 1)[0]))


def get_interface(scope_id: str | int | None) -> str | int | None:
    """
    Maps a scope ID (interface index) to its interface name.
    Returns None for invalid/unspecified IDs, or original scope_id if not found.
    """
    if not scope_id:
        return None

    try:
        scope_id = int(scope_id)
    except ValueError:
        return scope_id

    for if_index, if_name in socket.if_nameindex():
        if if_index == scope_id:
            return if_name

    return scope_id
