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

import ipaddress
import logging
import netifaces
from zeroconf import InterfaceChoice

logger = logging.getLogger(__name__)


def get_ipv6_addresses():
    results = []
    for iface in netifaces.interfaces():
        try:
            # Get list of IPv6 addresses for the interface
            addrs = netifaces.ifaddresses(iface).get(netifaces.AF_INET6, [])
        except ValueError:
            # Skip interfaces that don’t support IPv6
            continue

        for addr_info in addrs:
            addr = addr_info.get('addr', '')
            if not addr:
                continue

            # Normalize by stripping any existing scope
            base_addr = addr.split('%')[0]

            try:
                ip = ipaddress.IPv6Address(base_addr)
            except ValueError:
                # Skip invalid addresses
                continue

            # Include link-local, ULA (fdxx::/8), and global (2000::/3), skip loopback (::1)
            if ip.is_loopback:
                continue  # Skip ::1 (loopback); not usable for external discovery

            if ip.is_link_local:
                results.append(f"{base_addr}%{iface}")  # Append scope for link-local
            elif ip.is_private or ip.is_global:
                results.append(base_addr)  # ULA or global addresses

    if not results:
        # No usable IPv6, fallback to Zeroconf default
        logger.info("\n\nDefaulting to InterfaceChoice.All\n")
        return InterfaceChoice.All

    logger.info(f"\n\nDiscovered IPv6 addresses: {results}\n")
    return results
