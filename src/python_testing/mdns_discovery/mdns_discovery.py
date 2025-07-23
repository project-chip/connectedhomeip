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
import json
import logging
from asyncio import Event, TimeoutError, ensure_future, wait_for
from dataclasses import asdict
from enum import Enum
from functools import partial
from typing import Dict, List, Optional

import netifaces
from mdns_discovery.data_clases.mdns_service_info import MdnsServiceInfo
from mdns_discovery.data_clases.ptr_record import PtrRecord
from mdns_discovery.data_clases.quada_record import QuadaRecord
from mdns_discovery.mdns_async_service_info import AddressResolverIPv6, MdnsAsyncServiceInfo
from zeroconf import IPVersion, ServiceListener, ServiceStateChange, Zeroconf
from zeroconf._utils.net import InterfaceChoice
from zeroconf.asyncio import AsyncServiceBrowser, AsyncServiceInfo, AsyncZeroconf, AsyncZeroconfServiceTypes
from zeroconf.const import _TYPE_A, _TYPE_AAAA, _TYPE_SRV, _TYPE_TXT

logger = logging.getLogger(__name__)


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
        self.updated_event = Event()

    def add_service(self, zeroconf: Zeroconf, service_type: str, name: str) -> None:
        self.updated_event.set()

    def update_service(self, zeroconf: Zeroconf, service_type: str, name: str) -> None:
        self.updated_event.set()

    def remove_service(self, zeroconf: Zeroconf, service_type: str, name: str) -> None:
        pass


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
            - get_service_types
            - get_srv_record
            - get_txt_record
            - get_quada_records
            - get_ptr_records
            - get_commissionable_subtypes
        """
        # List of IPv6 addresses to use for mDNS discovery.
        self.interfaces = self._get_ipv6_addresses()

        # An instance of Zeroconf to manage mDNS operations.
        self._azc = AsyncZeroconf(interfaces=self.interfaces)

        # A dictionary to store discovered services.
        self._discovered_services = {}

        # A list of service types
        self._service_types = []

        # Filtering to apply for received data items
        self._name_filter = None

        # An asyncio Event to signal when a service has been discovered
        self._event = Event()

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
        logger.info(f"name filter {self._name_filter}")
        return await self._get_service(MdnsServiceType.OPERATIONAL, log_output, discovery_timeout_sec, self._name_filter)

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
        Asynchronously discovers all available mDNS service types within the network and returns a list
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
        logger.info("Discovering all available mDNS service types...")
        try:
            service_types = list(set(await wait_for(AsyncZeroconfServiceTypes.async_find(aiozc=self._azc, interfaces=self.interfaces), timeout=discovery_timeout_sec)))
        except TimeoutError:
            logger.info(f"mDNS service types discovery timed out after {discovery_timeout_sec} seconds.")
            service_types = []

        if log_output:
            logger.info(
                "\n\nDiscovered mDNS service types:\n%s\n", "\n".join(f"  - {s}" for s in service_types))

        return service_types

    async def get_srv_record(self, service_name: str,
                             service_type: str,
                             discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
                             log_output: bool = False
                             ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers the SRV record associated with an mDNS service.

        This method performs an mDNS browse and then issues a targeted SRV query 
        for the specified service instance. If a response is received within the timeout,
        it returns an MdnsServiceInfo object populated with SRV record details 
        (including target host and port).

        Args:
            service_name (str): The full mDNS service instance name.
                Example: '974B15BD2CC5278E-0000000012344321._matter._tcp.local.'
            service_type (str): The mDNS service type.
                Example: '_matter._tcp.local.'
            discovery_timeout_sec (float, optional): Maximum time to wait for discovery.
                Defaults to DISCOVERY_TIMEOUT_SEC.
            log_output (bool, optional): If True, logs discovered service info. Defaults to False.

        Returns:
            Optional[MdnsServiceInfo]:
                - MdnsServiceInfo with parsed SRV data if discovered.
                - None if no valid response is received within the timeout.
        """
        logger.info(f"Looking for mDNS record, type 'SRV', service name '{service_name}'")

        async with AsyncZeroconf() as azc:
            mdns_service_info = None

            # Adds service listener
            service_listener = MdnsServiceListener()
            await azc.async_add_service_listener(service_type, service_listener)

            # Wait for the add/update service event or timeout
            try:
                await wait_for(service_listener.updated_event.wait(), discovery_timeout_sec)
            except TimeoutError:
                logger.info(
                    f"Service lookup for {service_name} timeout ({discovery_timeout_sec} seconds) reached without an update.")
            finally:
                await azc.async_remove_service_listener(service_listener)

            # Prepare and perform query
            service_info = MdnsAsyncServiceInfo(name=service_name, type_=service_type)
            service_info._query_record_types = {_TYPE_SRV}
            is_discovered = await service_info.async_request(
                azc.zeroconf,
                discovery_timeout_sec * 1000)

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
            else:
                logger.error(f"SRV Record for service '{service_name}' of type '{service_type}' not found.")
                return None

    async def get_txt_record(self, service_name: str,
                             service_type: str,
                             discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
                             log_output: bool = False
                             ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers the TXT record associated with an mDNS service.

        This method performs an mDNS browse and then issues a targeted TXT query 
        for the specified service instance. If a response is received within the timeout,
        it returns an MdnsServiceInfo object populated with TXT record key-value metadata.

        Args:
            service_name (str): The full mDNS service instance name.
                Example: '974B15BD2CC5278E-0000000012344321._matter._tcp.local.'
            service_type (str): The mDNS service type.
                Example: '_matter._tcp.local.'
            discovery_timeout_sec (float, optional): Maximum time to wait for discovery.
                Defaults to DISCOVERY_TIMEOUT_SEC.
            log_output (bool, optional): If True, logs discovered service info. Defaults to False.

        Returns:
            Optional[MdnsServiceInfo]:
                - MdnsServiceInfo with parsed TXT data if discovered.
                - None if no valid response is received within the timeout.
        """
        logger.info(f"Looking for mDNS record, type 'TXT', service name '{service_name}'")

        async with AsyncZeroconf() as azc:
            mdns_service_info = None

            # Adds service listener
            service_listener = MdnsServiceListener()
            await azc.async_add_service_listener(service_type, service_listener)

            # Wait for the add/update service event or timeout
            # This is necessary whem requesting TXT records,
            # as the service may not be immediately available
            try:
                await wait_for(service_listener.updated_event.wait(), discovery_timeout_sec)
            except TimeoutError:
                logger.info(
                    f"Service lookup for {service_name} timeout ({discovery_timeout_sec} seconds) reached without an update.")
            finally:
                await azc.async_remove_service_listener(service_listener)

            # Prepare and perform query
            service_info = MdnsAsyncServiceInfo(name=service_name, type_=service_type)
            service_info._query_record_types = {_TYPE_TXT}
            is_discovered = await service_info.async_request(
                azc.zeroconf,
                discovery_timeout_sec * 1000)

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
            else:
                logger.error(f"TXT Record for service '{service_name}' of type '{service_type}' not found.")
                return None

    async def get_quada_records(self, hostname: str,
                                discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
                                log_output: bool = False
                                ) -> Optional[list[QuadaRecord]]:
        """
        Asynchronously retrieves the AAAA (IPv6) record of a device on the local network via mDNS.

        Args:
            hostname (str): The fully qualified hostname of the target device.
                Example: '00155DF32EEB.local.'

            discovery_timeout_sec (float, optional): Time in seconds to wait for discovery.
                Defaults to 3.

            log_output (bool, optional): Whether to log the discovered information. Defaults to False.

        Returns:
            Returns a QuadaRecord containing the resolved IPv6 addresses, or None if resolution fails.
        """
        logger.info(f"Looking for mDNS record, type 'AAAA',  hostname '{hostname}'")

        async with AsyncZeroconf() as azc:
            # Perform AAAA query
            addr_resolver = AddressResolverIPv6(server=hostname)

            is_discovered = await addr_resolver.async_request(
                azc.zeroconf,
                timeout=discovery_timeout_sec * 1000)

            if not is_discovered:
                logger.warning(f"No AAAA record found for '{hostname}' within {discovery_timeout_sec}s")
                return None

            # Get IPv6 addresses
            ipv6_addresses = addr_resolver.ip_addresses_by_version(IPVersion.V6Only)
            if ipv6_addresses:
                quada_records: list[QuadaRecord] = [
                    QuadaRecord.build(ipv6)
                    for ipv6 in ipv6_addresses
                ]

            # Adds service to discovered services
            self._discovered_services = {}
            self._discovered_services[hostname] = []
            for ipv6 in quada_records:
                self._discovered_services[hostname].append(ipv6)

            if log_output:
                self._log_output()

            return quada_records

    async def get_ptr_records(self,
                              service_types: list[str],
                              discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
                              log_output: bool = False,
                              ) -> list[PtrRecord]:
        """
        Asynchronously discovers mDNS PTR records for the given service types.

        Args:
            discovery_timeout_sec (float): Time in seconds to wait for responses after sending the browse request.
            service_types (list[str]): A list of service types (e.g., "_matter._tcp.local.", "_Ixxxx._sub._matter._tcp.local.") to browse for.
            log_output (bool): If True, logs the discovered PTR records grouped by service type as JSON.

        Returns:
            list[PtrRecord]: A flat list of all discovered PtrRecord objects across all requested service types.

        Note:
            This method performs a PTR browse using AsyncServiceBrowser for each specified service type.
            It waits up to the specified timeout for responses and handles them via internal callbacks.
            The resulting records include instance name extraction and optional subtype support.
        """
        self._event.clear()

        logger.info(f"Browsing for mDNS service(s) of type: {service_types}")

        self._discovered_services = {}
        aiobrowser = AsyncServiceBrowser(zeroconf=self._azc.zeroconf,
                                         type_=service_types,
                                         handlers=[partial(self._on_service_state_change, unlock_service=False)]
                                         )

        try:
            await wait_for(self._event.wait(), timeout=discovery_timeout_sec)
        except TimeoutError:
            logger.info("mDNS browse finished after %d seconds.", discovery_timeout_sec)
        finally:
            self._event.set()
            await aiobrowser.async_cancel()

        if log_output:
            self._log_output()

        ptr_records = [
            record
            for records in self._discovered_services.values()
            for record in records
        ]

        return ptr_records

    async def get_commissionable_subtypes(self,
                                          discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
                                          log_output: bool = False,) -> list[str]:
        """
        Asynchronously retrieves a list of commissionable mDNS service subtypes.

        Args:
            discovery_timeout_sec (float, optional): Timeout in seconds for the service type discovery. Defaults to DISCOVERY_TIMEOUT_SEC.
            log_output (bool, optional): If True, logs the discovered commissionable subtypes. Defaults to False.

        Returns:
            list[str]: A list of strings representing the discovered commissionable mDNS service subtypes.
        """
        # Get service types
        service_types = await self.get_service_types(
            discovery_timeout_sec=discovery_timeout_sec,
            log_output=False
        )

        # Filter for commissionable subtypes
        sub_types = [
            st for st in service_types
            if st.startswith('_') and f'._sub.{MdnsServiceType.COMMISSIONABLE.value}' in st
        ]

        if log_output:
            logger.info(
                "\n\nDiscovered mDNS commissionable service subtypes:\n%s\n", "\n".join(f"  - {s}" for s in sub_types))

        return sub_types

    # Private methods
    async def _get_service(self, service_type: MdnsServiceType,
                           log_output: bool,
                           discovery_timeout_sec: float,
                           service_name: str = None,
                           unlock_service: bool = True
                           ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers a specific type of mDNS service within the network and returns its details.

        Args:
            service_type (MdnsServiceType): The enum representing the type of mDNS service to discover.
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 15 seconds.
            service_name (str): Defaults to none as currently only utilized to gather specific record in multiple discovery records if available.
            unlock_service (bool): If True, queries the service info for each of the discovered service names, defaluts to True.

        Returns:
            Optional[MdnsServiceInfo]: An instance of MdnsServiceInfo representing the discovered service, if
                                    any. Returns None if no service of the specified type is discovered within
                                    the timeout period.
        """
        self._service_types = [service_type.value]

        await self._discover(
            discovery_timeout_sec=discovery_timeout_sec,
            log_output=log_output,
            unlock_service=unlock_service,
        )

        if self._verbose_logging:
            logger.info("Getting service from discovered services: %s", self._discovered_services)

        if service_type.value in self._discovered_services:
            if service_name is not None:
                for service in self._discovered_services[service_type.value]:
                    if service.service_name == service_name.replace("._MATTER._TCP.LOCAL.", "._matter._tcp.local."):
                        return service
            else:
                return self._discovered_services[service_type.value][0]
        else:
            return None

    async def _discover(self,
                        discovery_timeout_sec: float,
                        log_output: bool,
                        service_types: Optional[List[str]] = None,
                        all_services: bool = False,
                        unlock_service: bool = True,
                        ) -> None:
        """
        Asynchronously discovers network services using multicast DNS (mDNS).

        Args:
            discovery_timeout_sec (float): The duration in seconds to wait for the discovery process, allowing for service
                                        announcements to be collected.
            log_output (bool): If True, logs the discovered services to the console in JSON format for debugging or informational
                            purposes. Defaults to False.
            service_types (Optional[List[str]]): Specific service types to discover (e.g., ['_matterc._udp.local.']).
            unlock_service (bool): If True, queries the service info for each of the discovered service names, defaluts to True.
            all_services (bool): If True, discovers all available mDNS services. If False, discovers services based on the
                                predefined `_service_types` list. Defaults to False.

        Raises:
            ValueError: If both `all_services` and `service_types` are provided.

        Returns:
            None: This method does not return any value.

        Note:
            The discovery duration may need to be adjusted based on network conditions and expected response times for service
            announcements.
        """
        if all_services and service_types:
            raise ValueError("Specify either 'all_services' or 'service_types', not both.")

        self._event.clear()

        if all_services:
            self._service_types = []
            self._service_types = list(set(await AsyncZeroconfServiceTypes.async_find(aiozc=self._azc, interfaces=self.interfaces)))

        if service_types:
            self._service_types = []
            self._service_types = service_types

        logger.info(f"Browsing for mDNS service(s) of type: {self._service_types}")

        aiobrowser = AsyncServiceBrowser(zeroconf=self._azc.zeroconf,
                                         type_=self._service_types,
                                         handlers=[partial(self._on_service_state_change, unlock_service=unlock_service)]
                                         )

        try:
            await wait_for(self._event.wait(), timeout=discovery_timeout_sec)
        except TimeoutError:
            if unlock_service:
                logger.info("mDNS browse finished after %d seconds.", discovery_timeout_sec)
            else:
                logger.error("mDNS service discovery timed out after %d seconds.", discovery_timeout_sec)
        finally:
            self._event.set()
            await aiobrowser.async_cancel()

        if log_output:
            self._log_output()

    def _on_service_state_change(
        self,
        unlock_service: bool,
        zeroconf: Zeroconf,
        service_type: str,
        name: str,
        state_change: ServiceStateChange,
    ) -> None:
        """
        Callback method triggered on mDNS service state change.

        This method is called by the Zeroconf library when there is a change in the state of an mDNS service.
        It handles the addition of new services by initiating a query for their detailed information.

        Args:
            unlock_service (bool): If True, queries the service info for each of the discovered service names.
            zeroconf (Zeroconf): The Zeroconf instance managing the network operations.
            service_type (str): The service type of the mDNS service that changed state.
            name (str): The service name of the mDNS service.
            state_change (ServiceStateChange): The type of state change that occurred.

        Returns:
            None: This method does not return any value.
        """
        if self._verbose_logging:
            logger.info("Service state change: %s on %s/%s", state_change, name, service_type)

        if state_change in [ServiceStateChange.Removed, ServiceStateChange.Updated]:
            return

        if self._name_filter is not None and name.upper() != self._name_filter:
            if self._verbose_logging:
                logger.info("   Name does NOT match \'%s\' vs \'%s\'", self._name_filter, name.upper())
            return

        if self._verbose_logging:
            logger.info("Received service data. Unlocking service information")

        if unlock_service:
            # Get service information
            ensure_future(self._query_service_info(
                service_type,
                name)
            )
        else:
            # Only gather PTR record information
            if service_type not in self._discovered_services:
                self._discovered_services[service_type] = []
            self._discovered_services[service_type].append(PtrRecord(service_type=service_type, service_name=name))

    async def _query_service_info(self, service_type: str, service_name: str) -> None:
        """
        This method queries for service details such as its address, port, and TXT records
        containing metadata.

        Args:
            service_type (str): The type of the mDNS service being queried.
            service_name (str): The specific service name of the mDNS service to query. This service name uniquely
            identifies the service instance within the local network.

        Returns:
            None: This method does not return any value.
        """
        async with AsyncZeroconf() as azc:
            mdns_service_info = None
            discovery_timeout_sec = 3

            # Adds service listener
            service_listener = MdnsServiceListener()
            await azc.async_add_service_listener(service_type, service_listener)

            # Wait for the add/update service event or timeout
            # This is necessary whem requesting TXT records,
            # as the service may not be immediately available
            try:
                await wait_for(service_listener.updated_event.wait(), discovery_timeout_sec)
            except TimeoutError:
                logger.info(
                    f"Service lookup for {service_name} timeout ({discovery_timeout_sec} seconds) reached without an update.")
            finally:
                await azc.async_remove_service_listener(service_listener)

            # Prepare and perform query
            service_info = MdnsAsyncServiceInfo(name=service_name, type_=service_type)
            service_info._query_record_types = {_TYPE_SRV, _TYPE_TXT, _TYPE_A, _TYPE_AAAA}
            is_discovered = await service_info.async_request(
                azc.zeroconf,
                discovery_timeout_sec * 1000)

            if is_discovered:
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

    @staticmethod
    def _get_instance_name(service_info: AsyncServiceInfo) -> str:
        if service_info.type:
            return service_info.name[: len(service_info.name) - len(service_info.type) - 1]
        else:
            return service_info.name

    def _log_output(self) -> str:
        """
        Converts the discovered services to a JSON string and logs it.

        The method is intended to be used for debugging or informational purposes, providing a clear and
        comprehensive view of all services discovered during the mDNS service discovery process.
        """
        converted_services = {key: [asdict(item) for item in value] for key, value in self._discovered_services.items()}
        json_str = json.dumps(converted_services, indent=4)
        logger.info("Discovery data:\n%s", json_str)

    @staticmethod
    def _get_ipv6_addresses():
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
