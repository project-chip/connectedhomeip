#
#    Copyright (c) 2023 Project CHIP Authors
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

import ipaddress
from typing import Callable

from .log import get_logger

_LOGGER = get_logger(__file__)
WIFI_CHANNELS_2G = [str(j) for j in [i for i in range(1, 15)]]


def safe_ip_type_check(ip_type_filter: Callable[[str], bool]) -> Callable[[str], bool]:
    def wrapper(ip_to_check: str) -> bool:
        try:
            return ip_type_filter(ip_to_check)
        except ipaddress.AddressValueError:
            return False
    return wrapper


@safe_ip_type_check
def is_ipv4(ip: str) -> bool:
    return ipaddress.IPv4Address(ip) is not None


@safe_ip_type_check
def is_ipv6_global_unicast(ip: str) -> bool:
    return ipaddress.IPv6Address(ip).is_global


@safe_ip_type_check
def is_ipv6_link_local(ip: str) -> bool:
    return ipaddress.IPv6Address(ip).is_link_local


@safe_ip_type_check
def is_ipv6_unique_local(ip: str) -> bool:
    return ipaddress.IPv6Address(ip).is_private and not is_ipv6_link_local(ip)


@safe_ip_type_check
def is_ipv6(ip: str) -> bool:
    return ipaddress.IPv6Address(ip) is not None


def get_addr_type(ip: str) -> str:
    if is_ipv4(ip):
        return "V4"
    elif is_ipv6_global_unicast(ip):
        return "V6 Global unicast"
    elif is_ipv6_link_local(ip):
        return "V6 Link local"
    elif is_ipv6_unique_local(ip):
        return "V6 Unique local"
    elif is_ipv6(ip):
        return "V6"
    _LOGGER.debug(f"Failed to parse IP {ip}")
    return "ERROR: UNKNOWN ADDR TYPE"
