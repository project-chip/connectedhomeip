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
    addresses: list[str]

    # A dictionary of key-value pairs representing the service's metadata.
    txt_record: dict[str, str]

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

    DISCOVERY_TIMEOUT_SEC = 15

    def __init__(self):
        """
        Initializes the MdnsDiscovery instance.

        Main methods:
            - get_commissioner_service
            - get_commissionable_service
            - get_operational_service
            - get_border_router_service
            - get_all_services
        """
        # An instance of Zeroconf to manage mDNS operations.
        self._zc = Zeroconf(ip_version=IPVersion.V6Only)

        # A dictionary to store discovered services.
        self._discovered_services = {}

        # A list of service types
        self._service_types = []

        # An asyncio Event to signal when a service has been discovered
        self._event = asyncio.Event()

    # Public methods
    async def get_commissioner_service(self, log_output: bool = False,
                                       discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC
                                       ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers a commissioner mDNS service within the network.

        Args:
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 15 seconds.

        Returns:
            Optional[MdnsServiceInfo]: An instance of MdnsServiceInfo or None if timeout reached.
        """
        return await self._get_service(MdnsServiceType.COMMISSIONER, log_output, discovery_timeout_sec)

    async def get_commissionable_service(self, log_output: bool = False,
                                         discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC
                                         ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers a commissionable mDNS service within the network.

        Args:
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 15 seconds.

        Returns:
            Optional[MdnsServiceInfo]: An instance of MdnsServiceInfo or None if timeout reached.
        """
        return await self._get_service(MdnsServiceType.COMMISSIONABLE, log_output, discovery_timeout_sec)

    async def get_operational_service(self, service_name: str = None,
                                      service_type: str = None,
                                      discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
                                      log_output: bool = False
                                      ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers an operational mDNS service within the network.

        Args:
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 15 seconds.
            service_name (str): The unique name of the mDNS service. Defaults to None.
            service_type (str): The service type of the service. Defaults to None.

        Returns:
            Optional[MdnsServiceInfo]: An instance of MdnsServiceInfo or None if timeout reached.
        """
        # Validation to ensure both or none of the parameters are provided
        if (service_name is None) != (service_type is None):
            raise ValueError("Both service_name and service_type must be provided together or not at all.")

        mdns_service_info = None

        if service_name is None and service_type is None:
            mdns_service_info = await self._get_service(MdnsServiceType.OPERATIONAL, log_output, discovery_timeout_sec)
        else:
            print(f"Looking for MDNS service type '{service_type}',  service name '{service_name}'")

            # Get service info
            service_info = AsyncServiceInfo(service_type, service_name)
            is_discovered = await service_info.async_request(self._zc, 3000)
            if is_discovered:
                mdns_service_info = self._to_mdns_service_info_class(service_info)
            self._discovered_services = {}
            self._discovered_services[service_type] = [mdns_service_info]

            if log_output:
                self._log_output()

        return mdns_service_info

    async def get_border_router_service(self, log_output: bool = False,
                                        discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC
                                        ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers a border router mDNS service within the network.

        Args:
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 15 seconds.

        Returns:
            Optional[MdnsServiceInfo]: An instance of MdnsServiceInfo or None if timeout reached.
        """
        return await self._get_service(MdnsServiceType.BORDER_ROUTER, log_output, discovery_timeout_sec)

    async def get_all_services(self, log_output: bool = False,
                               discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC
                               ) -> Dict[str, List[MdnsServiceInfo]]:
        """
        Asynchronously discovers all available mDNS services within the network.

        Args:
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 15 seconds.

        Returns:
            Dict[str, List[MdnsServiceInfo]]: A dictionary mapping service types (str) to
                                            lists of MdnsServiceInfo objects.
        """
        await self._discover(discovery_timeout_sec, log_output, all_services=True)

        return self._discovered_services

    # Private methods
    async def _discover(self,
                        discovery_timeout_sec: float,
                        log_output: bool,
                        all_services: bool = False
                        ) -> None:
        """
        Asynchronously discovers network services using multicast DNS (mDNS).

        Args:
            discovery_timeout_sec (float): The duration in seconds to wait for the discovery process, allowing for service
                                        announcements to be collected.
            all_services (bool): If True, discovers all available mDNS services. If False, discovers services based on the
                                predefined `_service_types` list. Defaults to False.
            log_output (bool): If True, logs the discovered services to the console in JSON format for debugging or informational
                            purposes. Defaults to False.

        Returns:
            None: This method does not return any value.

        Note:
            The discovery duration may need to be adjusted based on network conditions and expected response times for service
            announcements. The method leverages an asyncio event to manage asynchronous waiting and cancellation based on discovery
            success or timeout.
        """
        self._event.clear()

        if all_services:
            self._service_types = list(await AsyncZeroconfServiceTypes.async_find())

        print(f"Browsing for MDNS service(s) of type: {self._service_types}")

        aiobrowser = AsyncServiceBrowser(zeroconf=self._zc,
                                         type_=self._service_types,
                                         handlers=[self._on_service_state_change]
                                         )

        try:
            await asyncio.wait_for(self._event.wait(), timeout=discovery_timeout_sec)
        except asyncio.TimeoutError:
            print(f"MDNS service discovery timed out after {discovery_timeout_sec} seconds.")
        finally:
            await aiobrowser.async_cancel()

        if log_output:
            self._log_output()

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
            self._event.set()
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
            None: This method does not return any value.
        """
        # Get service info
        service_info = AsyncServiceInfo(service_type, service_name)
        is_service_discovered = await service_info.async_request(zeroconf, 3000)
        service_info.async_clear_cache()

        if is_service_discovered:
            mdns_service_info = self._to_mdns_service_info_class(service_info)

            if service_type not in self._discovered_services:
                self._discovered_services[service_type] = [mdns_service_info]
            else:
                self._discovered_services[service_type].append(mdns_service_info)

    def _to_mdns_service_info_class(self, service_info: AsyncServiceInfo) -> MdnsServiceInfo:
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

    async def _get_service(self, service_type: MdnsServiceType,
                           log_output: bool,
                           discovery_timeout_sec: float
                           ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers a specific type of mDNS service within the network and returns its details.

        Args:
            service_type (MdnsServiceType): The enum representing the type of mDNS service to discover.
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 15 seconds.

        Returns:
            Optional[MdnsServiceInfo]: An instance of MdnsServiceInfo representing the discovered service, if
                                    any. Returns None if no service of the specified type is discovered within
                                    the timeout period.
        """
        mdns_service_info = None
        self._service_types = [service_type.value]
        await self._discover(discovery_timeout_sec, log_output)
        if service_type.value in self._discovered_services:
            mdns_service_info = self._discovered_services[service_type.value][0]

        return mdns_service_info

    def _log_output(self) -> str:
        """
        Converts the discovered services to a JSON string and prints it.

        The method is intended to be used for debugging or informational purposes, providing a clear and
        comprehensive view of all services discovered during the mDNS service discovery process.
        """
        converted_services = {key: [asdict(item) for item in value] for key, value in self._discovered_services.items()}
        json_str = json.dumps(converted_services, indent=4)
        print(json_str)
