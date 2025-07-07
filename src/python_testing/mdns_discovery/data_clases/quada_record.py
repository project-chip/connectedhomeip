import socket
from dataclasses import dataclass
from ipaddress import IPv6Address
from typing import Optional, Union

from zeroconf._utils.ipaddress import (ZeroconfIPv6Address)


@dataclass
class AddressTypeInfo:
    is_global: bool
    is_link_local: bool
    is_loopback: bool
    is_multicast: bool


@dataclass
class SpecialAddressTypes:
    teredo: Optional[str]
    sixtofour: Optional[str]
    is_reserved: bool
    ipv4_mapped: Optional[str]


@dataclass
class AddressMeta:
    max_prefixlen: int
    packed: str


@dataclass
class IPv6AddressInfo:
    address: str
    version: int
    interface: Union[str, int]

    type_info: AddressTypeInfo
    special_types: SpecialAddressTypes
    meta: AddressMeta


@dataclass
class QuadaRecord:
    addresses: list[IPv6AddressInfo]

    @classmethod
    def build(cls, ipv6_addresses: list[ZeroconfIPv6Address]) -> "QuadaRecord":
        address_info_list = []

        for ipv6 in ipv6_addresses:
            address_info = IPv6AddressInfo(
                address=get_valid_compressed_ipv6(ipv6),
                version=ipv6.version,
                interface=get_interface(ipv6.scope_id),
                type_info=AddressTypeInfo(
                    is_global=ipv6.is_global,
                    is_link_local=ipv6.is_link_local,
                    is_loopback=ipv6.is_loopback,
                    is_multicast=ipv6.is_multicast,
                ),
                special_types=SpecialAddressTypes(
                    teredo=str(ipv6.teredo) if ipv6.teredo else None,
                    sixtofour=str(ipv6.sixtofour) if ipv6.sixtofour else None,
                    is_reserved=ipv6.is_reserved,
                    ipv4_mapped=str(ipv6.ipv4_mapped) if ipv6.ipv4_mapped else None,
                ),
                meta=AddressMeta(
                    max_prefixlen=ipv6.max_prefixlen,
                    packed=ipv6.packed.hex(),
                ),
            )
            address_info_list.append(address_info)

        return cls(addresses=address_info_list)


def get_valid_compressed_ipv6(ipv6: IPv6Address) -> str:
    """
    Strips the zone index (e.g., %eth0) from an IPv6 address and returns the compressed form.
    Validates the address format and raises if invalid.
    """
    compressed = str(ipv6)
    if '%' in compressed:
        compressed = compressed.split('%')[0]
    IPv6Address(compressed)  # will raise if invalid
    return compressed


def get_interface(scope_id: str | int | None) -> str | int | None:
    """
    Maps a scope ID (interface index) to its interface name.
    Returns None for invalid/unspecified IDs, or original scope_id if not found.
    """
    if not scope_id:
        return None  # None or falsy (like 0) means invalid or unspecified

    try:
        scope_id = int(scope_id)
    except ValueError:
        return scope_id  # Return original if not a valid integer

    for if_index, if_name in socket.if_nameindex():
        if if_index == scope_id:
            return if_name

    return scope_id  # No match found
