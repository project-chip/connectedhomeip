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
from typing import Dict, List

from mdns_discovery.exceptions import DiscoveryNotPerformedError
from zeroconf import IPVersion, ServiceStateChange, Zeroconf
from zeroconf.asyncio import AsyncServiceBrowser, AsyncServiceInfo
from dataclasses import dataclass
from enum import Enum


@dataclass
class MdnsServiceInfo:
    # The unique name of the mDNS service.
    service_name: str

    # The human-readable name of the service.
    name: str

    # The type of the service, typically indicating the service protocol and domain.
    type: str

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
    COMMISSIONER = "_matterd._udp.local."
    COMMISSIONABLE = "_matterc._udp.local."
    OPERATIONAL = "_matter._tcp.local."
    BORDER_ROUTER = "_meshcop._udp.local."


class MdnsDiscovery:

    DEFAULT_DISCOVERY_DURATION_SEC = 3

    def __init__(self):
        """
        Initializes the MdnsDiscovery instance with necessary configurations.

        Main methods, return a list of MdnsServiceInfo objects
            - get_commissioner_service_info()
            - get_commissionable_service_info()
            - get_operational_service_info()
            - get_border_router_service_info()
        """
        # An instance of Zeroconf to manage mDNS operations.
        # This is used for handling the low-level details of mDNS.
        self._zc = None

        # An instance of the AsyncServiceBrowser browser to discover services.
        # This browser is responsible for actively looking for services on the network.
        self._aiobrowser = None

        # A list of mDNS service types to discover.
        # This list defines the types of services that we are interested in discovering.
        self._service_types = [mdns_type.value for mdns_type in MdnsServiceType]

        # A dictionary to store discovered services.
        # As services are discovered, they are added to this dictionary.
        self._discovered_services = {}

        # Flag to indicate if discovery has been performed.
        # This helps in managing the state of service discovery.
        self._discovery_performed = False

    @classmethod
    async def create(cls,discovery_duration_sec: float = DEFAULT_DISCOVERY_DURATION_SEC):
        instance = cls()
        await instance.discover(discovery_duration_sec)
        return instance

    async def discover(self, discovery_duration_sec: float = DEFAULT_DISCOVERY_DURATION_SEC) -> None:
        """
        Asynchronously discovers network services of specified types using mDNS.

        This method initiates a multicast DNS (mDNS) service discovery process to find
        network services matching the specified types in the `MdnsServiceType` enum. It uses
        an asynchronous service browser to listen for service announcements and updates.

        The discovery process is given a configurable period to operate, during which the method
        asynchronously waits to allow network responses to be received and processed.

        Args:
            discovery_duration_sec (float): The duration in seconds for which the discovery
                                        process should wait to allow for service announcements.
                                        If not provided, defaults to 3 seconds.

        Note:
            The discovery duration may need adjustment based on network conditions and the
            expected response time for service announcements.

        Returns:
            None: This method does not return any value. The results of the service discovery
                  are handled by the callback function.
        """
        self._zc = Zeroconf(ip_version=IPVersion.V6Only)
        self._discovered_services = {mdns_type.name: [] for mdns_type in MdnsServiceType}
        self._aiobrowser = AsyncServiceBrowser(zeroconf=self._zc,
                                               type_=self._service_types,
                                               handlers=[self._on_service_state_change]
                                               )
        # Wait for discovery
        await asyncio.sleep(discovery_duration_sec)
        self._discovery_performed = True
        await self._async_close()

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
            service_type (str): The type of the mDNS service that changed state.
            name (str): The name of the mDNS service.
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

    async def _query_service_info(self, zeroconf: Zeroconf, service_type: str, name: str) -> None:
        """
        This method requests detailed information about an mDNS service, such as its address,
        port, and other service metadata. The information is requested asynchronously.

        Args:
            zeroconf (Zeroconf): The Zeroconf instance managing the network operations.
            service_type (str): The type of the mDNS service to query.
            name (str): The name of the mDNS service.

        Returns:
            None: This method does not return any value. The results of the query are processed internally.
        """
        # Get service info
        service_info = AsyncServiceInfo(service_type, name)
        is_service_discovered = await service_info.async_request(zeroconf, 3000)

        if is_service_discovered:
            # Add service info to discovered services
            for mdns_type in MdnsServiceType:
                if service_type == mdns_type.value:
                    service_info_dict = self._service_info_to_class(service_info)
                    self._discovered_services[mdns_type.name].append(service_info_dict)
                    break

    def _service_info_to_class(self, service_info: AsyncServiceInfo) -> MdnsServiceInfo:
        """
        Converts an AsyncServiceInfo object to a dictionary for easier access and manipulation.

        This method facilitates the handling of service information by converting the AsyncServiceInfo
        object, which contains details about an mDNS service, into a more accessible dictionary format.

        Args:
            service_info (AsyncServiceInfo): The AsyncServiceInfo object to convert.

        Returns:
            MdnsServiceInfo: A dataclass representation of the AsyncServiceInfo object.
        """

        mdns_service_info = MdnsServiceInfo(
            service_name=service_info.name,
            name=service_info.get_name(),
            type=service_info.type,
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

    def _get_service_info(self, mdns_type: MdnsServiceType) -> List[Dict[str, any]]:
        """
        This method returns the discovered service information for a specified mDNS type.
        It raises an exception if the discovery process has not been performed before calling this method.

        Args:
            mdns_type (MdnsServiceType): The mDNS type for which to retrieve the service info.

        Returns:
            list: A list of discovered services of the specified mDNS type.

        Raises:
            DiscoveryNotPerformedError: If discovery has not been performed before calling this method.
        """
        if not self._discovery_performed:
            raise DiscoveryNotPerformedError()

        return self._discovered_services[mdns_type.name]

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

    def get_operational_service_info(self) -> List[Dict[str, any]]:
        """
        Retrieves the service information for Operational services.

        Returns:
            list: A list of discovered Operational services.
        """
        return self._get_service_info(MdnsServiceType.OPERATIONAL)

    def get_border_router_service_info(self) -> List[Dict[str, any]]:
        """
        Retrieves the service information for Border Router services.

        Returns:
            list: A list of discovered Border Router services.
        """
        return self._get_service_info(MdnsServiceType.BORDER_ROUTER)

    async def _async_close(self) -> None:
        if self._aiobrowser is not None:
            await self._aiobrowser.async_cancel()
        self._zc = None
        self._aiobrowser = None
