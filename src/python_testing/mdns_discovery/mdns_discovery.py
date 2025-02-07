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
import logging
from dataclasses import asdict, dataclass
from enum import Enum
from time import sleep
from typing import Dict, List, Optional, Union, cast

from mdns_discovery.mdns_async_service_info import DNSRecordType, MdnsAsyncServiceInfo
from zeroconf import IPVersion, ServiceListener, ServiceStateChange, Zeroconf
from zeroconf._dns import DNSRecord
from zeroconf._engine import AsyncListener
from zeroconf._protocol.incoming import DNSIncoming
from zeroconf.asyncio import AsyncServiceBrowser, AsyncServiceInfo, AsyncZeroconfServiceTypes

logger = logging.getLogger(__name__)


@dataclass
class MdnsServiceInfo:
    # The unique name of the mDNS service.
    service_name: str

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

    # The service type of the service, typically indicating the service protocol and domain.
    service_type: Optional[str] = None


class MdnsServiceType(Enum):
    """
    Enum for Matter mDNS service types used in network service discovery.
    """
    COMMISSIONER = "_matterd._udp.local."
    COMMISSIONABLE = "_matterc._udp.local."
    OPERATIONAL = "_matter._tcp.local."
    BORDER_ROUTER = "_meshcop._udp.local."


class MdnsServiceListener(ServiceListener):
    """
    A service listener required during mDNS service discovery
    """

    def __init__(self):
        self.updated_event = asyncio.Event()

    def add_service(self, zeroconf: Zeroconf, service_type: str, name: str) -> None:
        sleep(0.125)
        self.updated_event.set()

    def remove_service(self, zeroconf: Zeroconf, service_type: str, name: str) -> None:
        pass

    def update_service(self, zeroconf: Zeroconf, service_type: str, name: str) -> None:
        self.updated_event.set()


class MdnsDiscovery:

    DISCOVERY_TIMEOUT_SEC = 15

    def __init__(self, verbose_logging: bool = False):
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

        # Filtering to apply for received data items
        self._name_filter = None

        # An asyncio Event to signal when a service has been discovered
        self._event = asyncio.Event()

        # Verbose logging
        self._verbose_logging = verbose_logging

    # Public methods
    async def get_commissioner_service(self, log_output: bool = False,
                                       discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC
                                       ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers a commissioner mDNS service within the network.

        Optional args:
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 15 seconds.

        Returns:
            Optional[MdnsServiceInfo]: An instance of MdnsServiceInfo or None if timeout reached.
        """
        self._name_filter = None
        return await self._get_service(MdnsServiceType.COMMISSIONER, log_output, discovery_timeout_sec)

    async def get_commissionable_service(self, log_output: bool = False,
                                         discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC
                                         ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers a commissionable mDNS service within the network.

        Optional args:
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 15 seconds.

        Returns:
            Optional[MdnsServiceInfo]: An instance of MdnsServiceInfo or None if timeout reached.
        """
        self._name_filter = None
        return await self._get_service(MdnsServiceType.COMMISSIONABLE, log_output, discovery_timeout_sec)

    async def get_operational_service(self,
                                      node_id: Optional[int] = None,
                                      compressed_fabric_id: Optional[int] = None,
                                      discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
                                      log_output: bool = False
                                      ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers an operational mDNS service within the network.

        Optional args:
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 15 seconds.
            node_id: the node id to create the service name from
            compressed_fabric_id: the fabric id to create the service name from

        Returns:
            Optional[MdnsServiceInfo]: An instance of MdnsServiceInfo or None if timeout reached.
        """

        # Validation to ensure that both node_id and compressed_fabric_id are provided or both are None.
        if (node_id is None) != (compressed_fabric_id is None):
            raise ValueError("Both node_id and compressed_fabric_id must be provided together or not at all.")

        self._name_filter = f'{compressed_fabric_id:016x}-{node_id:016x}.{MdnsServiceType.OPERATIONAL.value}'.upper()
        return await self._get_service(MdnsServiceType.OPERATIONAL, log_output, discovery_timeout_sec)

    async def get_border_router_service(self, log_output: bool = False,
                                        discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC
                                        ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers a border router mDNS service within the network.

        Optional args:
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

        Optional args:
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 15 seconds.

        Returns:
            Dict[str, List[MdnsServiceInfo]]: A dictionary mapping service types (str) to
                                            lists of MdnsServiceInfo objects.
        """
        await self._discover(discovery_timeout_sec, log_output, all_services=True)

        return self._discovered_services

    async def get_service_types(self, log_output: bool = False, discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,) -> List[str]:
        """
        Asynchronously discovers all available mDNS services within the network and returns a list
        of the service types discovered. This method utilizes the AsyncZeroconfServiceTypes.async_find()
        function to perform the network scan for mDNS services.

        Optional args:
            log_output (bool): If set to True, the discovered service types are logged to the console.
                            This can be useful for debugging or informational purposes. Defaults to False.
            discovery_timeout_sec (float): The maximum time (in seconds) to wait for the discovery process. Defaults to 10.0 seconds.

        Returns:
            List[str]: A list containing the service types (str) of the discovered mDNS services. Each
                    element in the list is a string representing a unique type of service found during
                    the discovery process.
        """
        try:
            discovered_services = list(await asyncio.wait_for(AsyncZeroconfServiceTypes.async_find(), timeout=discovery_timeout_sec))
        except asyncio.TimeoutError:
            logger.info(f"MDNS service types discovery timed out after {discovery_timeout_sec} seconds.")
            discovered_services = []

        if log_output:
            logger.info(f"MDNS discovered service types: {discovered_services}")

        return discovered_services

    async def get_service_by_record_type(self, service_name: str,
                                         record_type: DNSRecordType,
                                         service_type: str = None,
                                         discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
                                         log_output: bool = False
                                         ) -> Union[Optional[MdnsServiceInfo], Optional[DNSRecord]]:
        """
        Asynchronously discovers an mDNS service within the network by service name, service type,
        and record type.

        Required args:
            service_name (str): The unique name of the mDNS service.
            Example:            
                C82B83803DECA0B2-0000000012344321._matter._tcp.local.

            record_type (DNSRecordType): The type of record to look for (SRV, TXT, AAAA, A).
            Example:
                _matterd._tcp.local. (from the MdnsServiceType enum)

        Optional args:
            service_type (str): The service type of the service. Defaults to None.
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 15 seconds.

        Returns:
            Union[Optional[MdnsServiceInfo], Optional[DNSRecord]]: An instance of MdnsServiceInfo,
            a DNSRecord object, or None.
        """
        mdns_service_info = None

        if service_type:
            logger.info(
                f"\nLooking for MDNS service type '{service_type}',  service name '{service_name}', record type '{record_type.name}'\n")
        else:
            logger.info(
                f"\nLooking for MDNS service with service name '{service_name}', record type '{record_type.name}'\n")

        # Adds service listener
        service_listener = MdnsServiceListener()
        self._zc.add_service_listener(MdnsServiceType.OPERATIONAL.value, service_listener)

        # Wait for the add/update service event or timeout
        try:
            await asyncio.wait_for(service_listener.updated_event.wait(), discovery_timeout_sec)
        except asyncio.TimeoutError:
            logger.info(f"Service lookup for {service_name} timeout ({discovery_timeout_sec}) reached without an update.")
        finally:
            self._zc.remove_service_listener(service_listener)

        # Prepare and perform query
        service_info = MdnsAsyncServiceInfo(name=service_name, type_=service_type)
        is_discovered = await service_info.async_request(
            self._zc,
            3000,
            record_type=record_type)

        if record_type in [DNSRecordType.A, DNSRecordType.AAAA]:
            # Service type not supplied so we can
            # query against the target/server
            for protocols in self._zc.engine.protocols:
                listener = cast(AsyncListener, protocols)
                if listener.data:
                    dns_incoming = DNSIncoming(listener.data)
                    if dns_incoming.data:
                        answers = dns_incoming.answers()
                        logger.info(f"\nIncoming DNSRecord: {answers}\n")
                        return answers.pop(0) if answers else None
        else:
            # Adds service to discovered services
            if is_discovered:
                mdns_service_info = self._to_mdns_service_info_class(service_info)
            self._discovered_services = {}
            self._discovered_services[service_type] = []
            if mdns_service_info is not None:
                self._discovered_services[service_type].append(mdns_service_info)

            if log_output:
                self._log_output()

            return mdns_service_info

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
            self._service_types = list(set(await AsyncZeroconfServiceTypes.async_find()))

        logger.info(f"Browsing for MDNS service(s) of type: {self._service_types}")

        aiobrowser = AsyncServiceBrowser(zeroconf=self._zc,
                                         type_=self._service_types,
                                         handlers=[self._on_service_state_change]
                                         )

        try:
            await asyncio.wait_for(self._event.wait(), timeout=discovery_timeout_sec)
        except asyncio.TimeoutError:
            logger.error("MDNS service discovery timed out after %d seconds.", discovery_timeout_sec)
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
        if self._verbose_logging:
            logger.info("Service state change: %s on %s/%s", state_change, name, service_type)

        if state_change.value == ServiceStateChange.Removed.value:
            return

        if self._name_filter is not None and name.upper() != self._name_filter:
            if self._verbose_logging:
                logger.info("   Name does NOT match %s", self._name_filter)
            return

        if self._verbose_logging:
            logger.info("Received service data. Unlocking service information")

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
        service_info.async_clear_cache()
        is_service_discovered = await service_info.async_request(zeroconf, 3000)

        if is_service_discovered:
            if self._verbose_logging:
                logger.warning("Service discovered for %s/%s.", service_name, service_type)

            mdns_service_info = self._to_mdns_service_info_class(service_info)

            if service_type not in self._discovered_services:
                self._discovered_services[service_type] = []

            if mdns_service_info is not None:
                self._discovered_services[service_type].append(mdns_service_info)
        elif self._verbose_logging:
            logger.warning("Service information not found.")

        self._event.set()

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
            instance_name=self._get_instance_name(service_info),
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

    def _get_instance_name(self, service_info: AsyncServiceInfo) -> str:
        if service_info.type:
            return service_info.name[: len(service_info.name) - len(service_info.type) - 1]
        else:
            return service_info.name

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
        self._service_types = [service_type.value]
        await self._discover(discovery_timeout_sec, log_output)

        if self._verbose_logging:
            logger.info("Getting service from discovered services: %s", self._discovered_services)

        if service_type.value in self._discovered_services:
            return self._discovered_services[service_type.value][0]
        else:
            return None

    def _log_output(self) -> str:
        """
        Converts the discovered services to a JSON string and logs it.

        The method is intended to be used for debugging or informational purposes, providing a clear and
        comprehensive view of all services discovered during the mDNS service discovery process.
        """
        converted_services = {key: [asdict(item) for item in value] for key, value in self._discovered_services.items()}
        json_str = json.dumps(converted_services, indent=4)
        logger.info("Discovery data:\n%s", json_str)
