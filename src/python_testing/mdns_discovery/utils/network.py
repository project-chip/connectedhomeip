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

import ifaddr
from zeroconf import InterfaceChoice

log = logging.getLogger(__name__)


def get_host_ipv6_addresses():
    """
    Retrieves a list of valid IPv6 addresses for all network interfaces on the host.

    This function scans all network adapters for assigned IPv6 addresses and filters out
    unusable ones such as loopback (::1) and invalid addresses. It includes both link-local
    (with interface scope) and routable (ULA or global) IPv6 addresses.

    Returns:
        list[str] | InterfaceChoice:
            - A list of formatted IPv6 addresses (with scope if required), or
            - InterfaceChoice.All if no usable IPv6 addresses are found, in which case Zeroconf's
              default interface selection behavior will apply.
    """
    results = []

    for adapter in ifaddr.get_adapters():
        # Get list of IPv6 addresses for the interface
        for ip in adapter.ips:
            # Get IPv6 address and interface
            if isinstance(ip.ip, tuple):
                base_addr = ip.ip[0]
                iface_name = adapter.nice_name

                # Verify valid IPv6 address
                try:
                    ipv6 = ipaddress.IPv6Address(base_addr)
                except ValueError:
                    continue

                # Skip ::1 (loopback)
                if ipv6.is_loopback:
                    continue

                if ipv6.is_link_local:
                    # Link-local addresses require a scope identifier (interface name)
                    # to be usable for communication, so we append it
                    results.append(f"{base_addr}%{iface_name}")
                elif ipv6.is_private or ipv6.is_global:
                    # ULA (fd00::/8) and global (2000::/3) addresses are routable and
                    # do not require a scope, so we keep only the base address
                    results.append(base_addr)

    if not results:
        log.info("Returning Zeroconf's interface defaults")
        return InterfaceChoice.All

    log.info(f"Discovered IPv6 addresses: {results}")
    return results


async def is_dut_tcp_supported(instance_qname: str):
    """
    Determines whether the DUT supports TCP by inspecting its operational service TXT record.

    This function queries the DUT's operational mDNS service and examines the T key in the
    TXT record. MRP-only support (TCP unsupported) is indicated when the T key is absent,
    empty, or has a value of "0". Any other value indicates TCP support.

    Args:
        instance_qname (str): The fully qualified instance name of the DUT.

    Returns:
        bool:
            - True if the DUT supports TCP (T key present and not "0")
            - False if the DUT supports MRP-only (T key absent, empty, or "0")

    Raises:
        Exception: If the TXT record cannot be retrieved from the DUT.
    """
    # Import here to avoid circular dependency
    from mdns_discovery.mdns_discovery import MdnsDiscovery, MdnsServiceType

    try:
        # Get the operational service TXT record
        txt_record = await MdnsDiscovery().get_txt_record(
            service_name=instance_qname, service_type=MdnsServiceType.OPERATIONAL.value
        )
    except Exception as e:
        raise Exception(
            f"Unable to determine TCP support for '{instance_qname}'. "
            f"Failure during TXT record retrieval: {e}"
        ) from e

    # Returns True when TCP is supported (T key present and not "0")
    # Returns False when MRP-only (T key absent, empty, or "0")
    t_key = txt_record.txt.get('T', None)
    return not (t_key is None or not str(t_key).strip() or str(t_key).strip() == "0")
