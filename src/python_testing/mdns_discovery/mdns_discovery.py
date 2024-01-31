#
#    Copyright (c) 2024 Project CHIP Authors
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


import asyncio
import json
from dataclasses import asdict, dataclass
from enum import Enum
from typing import Dict, List, Optional

from zeroconf import IPVersion, ServiceStateChange, Zeroconf
from zeroconf.asyncio import AsyncServiceBrowser, AsyncServiceInfo, AsyncZeroconfServiceTypes


@dataclass
class MdnsServiceInfo:
    # The unique name of the mDNS service.
    service_name: str

    # The service type of the service, typically indicating the service protocol and domain.
    service_type: str

    # The instance name of the service.
    instance_name: str

    # The domain name of the machine hosting the service.
    server: str

    # The network port on which the service is available.
    port: int

    # A list of IP addresses associated with the service.
    addresses: list

    # A dictionary of key-value pairs representing the service's metadata.
    txt_record: dict

    # The priority of the service, used in service selection when multiple instances are available.
    priority: int

    # The network interface index on which the service is advertised.
    interface_index: int

    # The relative weight for records with the same priority, used in load balancing.
    weight: int

    # The time-to-live value for the host name in the DNS record.
    host_ttl: int

    # The time-to-live value for other records associated with the service.
    other_ttl: int


class MdnsServiceType(Enum):
    """
    Enum for Matter mDNS service types used in network service discovery.
    """
    COMMISSIONER = "_matterd._udp.local."
    COMMISSIONABLE = "_matterc._udp.local."
    OPERATIONAL = "_matter._tcp.local."
    BORDER_ROUTER = "_meshcop._udp.local."


class MdnsDiscovery:

    DEFAULT_DISCOVERY_DURATION_SEC = 3

    def __init__(self):
        """
        Initializes the MdnsDiscovery instance.

        Main methods:
            - get_commissioner_service_info()
            - get_commissionable_service_info()
            - get_operational_service_info()
            - get_border_router_service_info()
            - discover()
        """
        # An instance of Zeroconf to manage mDNS operations.
        # This is used for handling the low-level details of mDNS.
        self._zc = Zeroconf()

        # A dictionary to store discovered services.
        # As services are discovered, they are added to this dictionary.
        self._discovered_services = {}

    async def discover(self,
                       discovery_duration_sec: float = DEFAULT_DISCOVERY_DURATION_SEC,
                       log_output: bool = True) -> dict:
        """
        Asynchronously discovers network services of specified types using mDNS and returns the discovered services.

        This method initiates a multicast DNS (mDNS) service discovery process to find network services. The
        discovery process runs for a configurable duration, allowing network responses to be received and
        processed.

        Args:
            discovery_duration_sec (float): The duration in seconds for which the discovery process should wait
                                            to allow for service announcements. If not provided, defaults to a
                                            predetermined duration (e.g., 3 seconds).
            log_output (bool): Logs discovered services to the console in JSON format, defaults to True.

        Note:
            The discovery duration may need adjustment based on network conditions and the expected response
            time for service announcements.

        Returns:
            dict: A dictionary containing the discovered services, organized by service type. Each key in the
                dictionary represents a service type, and each value is a list of MdnsServiceInfo objects, each
                containing details of a discovered service.
        """
        service_types = list(await AsyncZeroconfServiceTypes.async_find())
        self._zc = Zeroconf(ip_version=IPVersion.V6Only)
        aiobrowser = AsyncServiceBrowser(zeroconf=self._zc,
                                         type_=service_types,
                                         handlers=[self._on_service_state_change]
                                         )
        # Wait for discovery
        await asyncio.sleep(discovery_duration_sec)
        if aiobrowser is not None:
            await aiobrowser.async_cancel()

        if log_output:
            # Log discovered services in JSON format
            converted_services = {key: [asdict(item) for item in value] for key, value in self._discovered_services.items()}
            json_str = json.dumps(converted_services, indent=4)
            print(json_str)

        return self._discovered_services

    def _on_service_state_change(
        self,
        zeroconf: Zeroconf,
        service_type: str,
        name: str,
        state_change: ServiceStateChange
    ) -> None:
        """
        Callback method triggered on mDNS service state change.

        This method is called by the Zeroconf library when there is a change in the state of an mDNS service.
        It handles the addition of new services by initiating a query for their detailed information.

        Args:
            zeroconf (Zeroconf): The Zeroconf instance managing the network operations.
            service_type (str): The service type of the mDNS service that changed state.
            name (str): The service name of the mDNS service.
            state_change (ServiceStateChange): The type of state change that occurred.

        Returns:
            None: This method does not return any value.
        """
        if state_change.value == ServiceStateChange.Added.value:
            asyncio.ensure_future(self._query_service_info(
                zeroconf,
                service_type,
                name)
            )

    async def _query_service_info(self, zeroconf: Zeroconf, service_type: str, service_name: str) -> None:
        """
        This method queries for service details such as its address, port, and TXT records
        containing metadata. 

        Args:
            zeroconf (Zeroconf): The Zeroconf instance used for managing network operations and service discovery.
            service_type (str): The type of the mDNS service being queried.
            service_name (str): The specific service name of the mDNS service to query. This service name uniquely
            identifies the service instance within the local network.

        Returns:
            None: This method does not return any value. It processes and stores the results of the service query
                internally in a dictionary which organizes services by their type for easy
                retrieval.
        """
        # Get service info
        service_info = AsyncServiceInfo(service_type, service_name)
        is_service_discovered = await service_info.async_request(zeroconf, 3000)
        service_info.async_clear_cache()

        if is_service_discovered:
            mdns_service_info = self._to_mdns_service_class(service_info)

            if service_type not in self._discovered_services:
                self._discovered_services[service_type] = [mdns_service_info]
            else:
                self._discovered_services[service_type].append(mdns_service_info)

    def _to_mdns_service_class(self, service_info: AsyncServiceInfo) -> MdnsServiceInfo:
        """
        Converts an AsyncServiceInfo object into a MdnsServiceInfo data class.

        Args:
            service_info (AsyncServiceInfo): The service information to convert.

        Returns:
            MdnsServiceInfo: The converted service information as a data class.
        """
        mdns_service_info = MdnsServiceInfo(
            service_name=service_info.name,
            service_type=service_info.type,
            instance_name=service_info.get_name(),
            server=service_info.server,
            port=service_info.port,
            addresses=service_info.parsed_addresses(),
            txt_record=service_info.decoded_properties,
            priority=service_info.priority,
            interface_index=service_info.interface_index,
            weight=service_info.weight,
            host_ttl=service_info.host_ttl,
            other_ttl=service_info.other_ttl
        )

        return mdns_service_info

    def get_commissioner_service_info(self) -> List[Dict[str, any]]:
        """
        Retrieves the service information for Commissioner services.

        Returns:
            list: A list of discovered Commissioner services.
        """
        return self._get_service_info(MdnsServiceType.COMMISSIONER)

    def get_commissionable_service_info(self) -> List[Dict[str, any]]:
        """
        Retrieves the service information for Commissionable services.

        Returns:
            list: A list of discovered Commissionable services.
        """
        return self._get_service_info(MdnsServiceType.COMMISSIONABLE)

    async def get_operational_service_info(self, service_name: str, service_type: str) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously retrieves service information for a specified service name and service_type.

        Args:
            service_name (str): The name of the service to discover.
            service_type (str): The service type of the service to discover.

        Returns:
            MdnsServiceInfo | None: The discovered service information if discovered, otherwise None.

        Raises:
            ValueError: If either 'service_name' or 'service_type' is None.
        """
        # Validate arguments to ensure they are not None
        if service_name is None or service_type is None:
            raise ValueError("Neither 'service_name' nor 'service_type' can be None.")

        # Get service info
        service_info = AsyncServiceInfo(type, service_name)
        is_discovered = await service_info.async_request(self._zc, 3000)

        return self._to_mdns_service_class(service_info) if is_discovered else None

    def get_border_router_service_info(self) -> List[Dict[str, any]]:
        """
        Retrieves the service information for Border Router services.

        Returns:
            list: A list of discovered Border Router services.
        """
        return self._get_service_info(MdnsServiceType.BORDER_ROUTER)
