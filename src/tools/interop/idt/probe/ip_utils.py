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


def is_ipv4(ip: str) -> bool:
    try:
        ipaddress.IPv4Address(ip)
        return True
    except ipaddress.AddressValueError:
        return False


def is_ipv6_ll(ip: str) -> bool:
    try:
        return ipaddress.IPv6Address(ip).is_link_local
    except ipaddress.AddressValueError:
        return False


def is_ipv6(ip: str) -> bool:
    try:
        ipaddress.IPv6Address(ip)
        return True
    except ipaddress.AddressValueError:
        return False


def get_addr_type(ip: str) -> str:
    if is_ipv4(ip):
        return "V4"
    elif is_ipv6_ll(ip):
        return "V6 Link Local"
    elif is_ipv6(ip):
        return "V6"
