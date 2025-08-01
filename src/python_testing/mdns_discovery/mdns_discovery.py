#
#    Copyright (c) 2024-2025 Project CHIP Authors
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

import json
import logging
import time
import asyncio
from asyncio import Event, TimeoutError, create_task, ensure_future, sleep, wait_for
from functools import partial
from typing import Dict, List, Optional

from mdns_discovery.data_clases.mdns_service_info import MdnsServiceInfo
from mdns_discovery.data_clases.ptr_record import PtrRecord
from mdns_discovery.data_clases.quada_record import QuadaRecord
from mdns_discovery.enums.mdns_service_type import MdnsServiceType
from mdns_discovery.mdns_async_service_info import AddressResolverIPv6, MdnsAsyncServiceInfo
from mdns_discovery.service_listeners.mdns_service_listener import MdnsServiceListener
from mdns_discovery.utils.net_utils import get_ipv6_addresses
from zeroconf import IPVersion, ServiceStateChange, Zeroconf
from zeroconf.asyncio import AsyncServiceBrowser, AsyncZeroconf, AsyncZeroconfServiceTypes
from zeroconf.const import _TYPE_A, _TYPE_AAAA, _TYPE_SRV, _TYPE_TXT

logger = logging.getLogger(__name__)


DISCOVERY_TIMEOUT_SEC = 30
QUERY_RECORD_TYPES = {_TYPE_SRV, _TYPE_TXT, _TYPE_A, _TYPE_AAAA}
DNS_TYPE_MAP = {_TYPE_SRV: "SRV", _TYPE_TXT: "TXT", _TYPE_A: "A", _TYPE_AAAA: "AAAA"}


class MdnsDiscovery:

    def __init__(self, verbose_logging: bool = False):
        """
        A class for performing asynchronous mDNS service discovery using Zeroconf.

        This utility supports querying various types of mDNS records (PTR, SRV, TXT, A, AAAA)
        over IPv6 interfaces and can discover Matter-specific services such as commissioner,
        commissionable, operational, and border router services.

        Key Methods:
            - `get_commissioner_service()`: Discover Matter commissioner services.
            - `get_commissionable_service()`: Discover Matter commissionable devices.
            - `get_operational_service()`: Discover operational Matter nodes.
            - `get_border_router_service()`: Discover Thread border routers.
            - `get_all_services()`: Get service info from all available discovered services.
            - `get_all_service_types()`: Discover all available service types.
            - `get_ptr_records()`: Discover PTR records for given service types.
            - `get_srv_record()`: Query SRV records for a given service instance.
            - `get_txt_record()`: Query TXT records for a given service instance.
            - `get_quada_records()`: Query A/AAAA records for a given host.
            - `get_commissionable_subtypes()`: Discover supported subtypes for commissionable nodes.

        Attributes:
            interfaces (list[str]): IPv6 interfaces used for discovery.
            _azc (AsyncZeroconf): Zeroconf manager instance.
            _discovered_services (dict): Stores results of service discovery.
            _name_filter (str | None): Optional filter for matching specific service names.
            _event (asyncio.Event): Event used to synchronize async discovery.
            _verbose_logging (bool): Enables detailed logging output.
        """
        # List of IPv6 addresses to use for mDNS discovery.
        self.interfaces = get_ipv6_addresses()

        # An instance of Zeroconf to manage mDNS operations.
        self._azc = AsyncZeroconf(interfaces=self.interfaces)

        # A dictionary to store discovered services.
        self._discovered_services = {}

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
            discovery_timeout_sec (float): Defaults to 30 seconds.

        Returns:
            Optional[MdnsServiceInfo]: An instance of MdnsServiceInfo or None if timeout reached.
        """
        self._name_filter = None
        return await self._get_service(MdnsServiceType.COMMISSIONER.value, log_output, discovery_timeout_sec)

    async def get_commissionable_service(self, log_output: bool = False,
                                         discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC
                                         ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers a commissionable mDNS service within the network.

        Optional args:
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 30 seconds.

        Returns:
            Optional[MdnsServiceInfo]: An instance of MdnsServiceInfo or None if timeout reached.
        """
        self._name_filter = None
        return await self._get_service(MdnsServiceType.COMMISSIONABLE.value, log_output, discovery_timeout_sec)

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
            discovery_timeout_sec (float): Defaults to 30 seconds.
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
        return await self._get_service(MdnsServiceType.OPERATIONAL.value, log_output, discovery_timeout_sec, self._name_filter)

    async def get_border_router_service(self, log_output: bool = False,
                                        discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC
                                        ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers a border router mDNS service within the network.

        Optional args:
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 30 seconds.

        Returns:
            Optional[MdnsServiceInfo]: An instance of MdnsServiceInfo or None if timeout reached.
        """
        return await self._get_service(MdnsServiceType.BORDER_ROUTER.value, log_output, discovery_timeout_sec)

    async def get_all_services(self, log_output: bool = False,
                               discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC
                               ) -> Dict[str, List[MdnsServiceInfo]]:
        """
        Asynchronously discovers all available mDNS services within the network.

        Optional args:
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 30 seconds.

        Returns:
            Dict[str, List[MdnsServiceInfo]]: A dictionary mapping service types (str) to
                                            lists of MdnsServiceInfo objects.
        """
        await self.discover(discovery_timeout_sec, log_output, all_services=True, query_service=True, append_results=True)

        return self._discovered_services

    async def get_all_service_types(self, log_output: bool = False, discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,) -> List[str]:
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
        record = await self._query_service_info(
            service_name=service_name,
            service_type=service_type,
            discovery_timeout_sec=discovery_timeout_sec,
            query_record_types={_TYPE_SRV},
            log_output=log_output
        )

        return record

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
        record = await self._query_service_info(
            service_name=service_name,
            service_type=service_type,
            discovery_timeout_sec=discovery_timeout_sec,
            query_record_types={_TYPE_TXT},
            log_output=log_output
        )

        return record

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
            Optional[list[QuadaRecord]]: A list of discovered QuadaRecord objects, or None if no services were found.
        """
        logger.info(f"Service record information lookup (AAAA) for '{hostname}' in progress...")

        async with AsyncZeroconf(interfaces=self.interfaces) as azc:
            # Perform AAAA query
            addr_resolver = AddressResolverIPv6(server=hostname)

            is_discovered = await addr_resolver.async_request(
                azc.zeroconf,
                timeout=discovery_timeout_sec * 1000)

            if is_discovered:
                if self._verbose_logging:
                    logger.info(f"Service record information (AAAA) for '{hostname}' discovered.")

                # Get IPv6 addresses and convert to QuadaRecord objects
                ipv6_addresses = addr_resolver.ip_addresses_by_version(IPVersion.V6Only)
                if ipv6_addresses:
                    quada_records: list[QuadaRecord] = [
                        QuadaRecord(ipv6)
                        for ipv6 in ipv6_addresses
                    ]

                # Adds service to discovered services
                self._discovered_services = {hostname: [ipv6 for ipv6 in quada_records]}

                if log_output:
                    self._log_output()

                return quada_records
            else:
                logger.error(f"Service record information (AAAA) for '{hostname}' not found.")
                return None

    async def get_ptr_records(self,
                              service_types: list[str],
                              discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
                              log_output: bool = False,
                              ) -> Optional[list[PtrRecord]]:
        """
        Asynchronously discovers mDNS PTR records for the given service types.

        Args:
            service_types (list[str]): A list of service types (e.g., "_matter._tcp.local.", "_Ixxxx._sub._matter._tcp.local.") to browse for.
            discovery_timeout_sec (float): Time in seconds to wait for responses after sending the browse request.
            log_output (bool): If True, logs the discovered PTR records grouped by service type as JSON.

        Returns:
            Optional[list[PtrRecord]]: A list of discovered PtrRecord objects, or None if no services were found.
        """
        logger.info(f"Service record information lookup (PTR) for '{service_types}' in progress...")
        await self.discover(
            discovery_timeout_sec=discovery_timeout_sec,
            log_output=log_output,
            service_types=service_types,
            query_service=False,
        )

        if self._discovered_services:
            if self._verbose_logging:
                logger.info(f"Service record information (PTR) for '{service_types}' discovered.")
            ptr_records = [
                record
                for records in self._discovered_services.values()
                for record in records
            ]
            return ptr_records
        else:
            logger.error(f"Service record information (PTR) for '{service_types}' not found.")
            return None

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
        service_types = await self.get_all_service_types(
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

    async def discover(self,
                       discovery_timeout_sec: float,
                       log_output: bool,
                       service_types: Optional[List[str]] = None,
                       all_services: bool = False,
                       query_service: bool = True,
                       append_results: bool = False
                       ) -> None:
        """
        Asynchronously discovers network services using multicast DNS (mDNS).

        Args:
            discovery_timeout_sec (float): The duration in seconds to wait for the discovery process, allowing for service
                                        announcements to be collected.
            log_output (bool): If True, logs the discovered services to the console in JSON format for debugging or informational
                            purposes. Defaults to False.
            service_types (Optional[List[str]]): Specific service types to discover (e.g., ['_matterc._udp.local.']).
            all_services (bool): If True, discovers all available mDNS services.
            query_service (bool): If True, queries the service info for each of the discovered service names, defaluts to True.
            append_results (bool): If True, appends the result to `self._discovered_services` without clearing previous entries.
                                   If False, resets `self._discovered_services` before storing the new result. This only applies
                                   when 'query_service' is True, as it's passed down to the '_query_service_info' function.
        Raises:
            ValueError: If both `all_services` and `service_types` are provided.

        Returns:
            None: This method does not return any value.
        """
        if all_services and service_types:
            raise ValueError("Specify either 'all_services' or 'service_types', not both.")

        types = []
        if all_services:
            types = await self.get_all_service_types(
                discovery_timeout_sec=discovery_timeout_sec,
                log_output=False
            )

        if service_types:
            types = service_types

        logger.info(f"Browsing for mDNS service(s) of type: {types}")

        # Setup fresh discovery
        self._event.clear()
        self._discovered_services = {}
        self._last_discovery_time = time.time()

        aiobrowser = AsyncServiceBrowser(
            zeroconf=self._azc.zeroconf, type_=types, handlers=[self._on_service_state_change]
        )

        # Background monitor to end discovery early after
        # a period of silence (inactivity, no new services)
        create_task(self._monitor_discovery_silence(silence_threshold=2))

        try:
            # Wait for either the silence timeout (triggered by the background monitor)
            # or the full discovery timeout to elapse, whichever comes first
            await wait_for(self._event.wait(), timeout=discovery_timeout_sec)
        except TimeoutError:
            logger.info("mDNS browse finished after %d seconds", discovery_timeout_sec)
        finally:
            logger.info("Stopping mDNS browse and cleaning up")
            self._event.set()
            await aiobrowser.async_cancel()

        if log_output:
            self._log_output()

        # Log discovery stats
        services_count = sum(len(ptr_list) for ptr_list in self._discovered_services.values())
        types_count = len(self._discovered_services)
        logger.info(f"Discovered {services_count} mDNS services across {types_count} service types")

        # If service querying is enabled, perform controlled parallel queries to
        # retrieve service information (TXT, SRV, A/AAAA) for each discovered PTR
        # record. This is helpful when many PTR records are found, as it prevents
        # system overload by limiting concurrent mDNS queries.
        if query_service:
            logger.info("Querying service information for discovered services...")
            semaphore = asyncio.Semaphore(5)  # Limit to 5 concurrent queries

            async def limited_query(ptr):
                async with semaphore:
                    await self._query_service_info(
                        service_type=ptr.service_type,
                        service_name=ptr.service_name,
                        append_results=True,
                        log_output=log_output
                    )

            tasks = []
            for ptr_list in self._discovered_services.values():
                for ptr in ptr_list:
                    tasks.append(limited_query(ptr))

            self._discovered_services = {}

            await asyncio.gather(*tasks)

            if append_results and log_output:
                self._log_output()

    # Private methods
    async def _get_service(self, service_type: str,
                           log_output: bool,
                           discovery_timeout_sec: float,
                           service_name: str = None,
                           query_service: bool = True
                           ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers a specific type of mDNS service within the network and returns its details.

        Args:
            service_type (str): Represents the type of mDNS service to discover.
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 30 seconds.
            service_name (str): Defaults to none as currently only utilized to gather specific record in multiple discovery records if available.
            query_service (bool): If True, queries the service info for each of the discovered service names, defaluts to True.

        Returns:
            Optional[MdnsServiceInfo]: An instance of MdnsServiceInfo representing the discovered service, if
                                    any. Returns None if no service of the specified type is discovered within
                                    the timeout period.
        """
        await self.discover(
            discovery_timeout_sec=discovery_timeout_sec,
            log_output=log_output,
            service_types=[service_type],
            query_service=query_service,
        )

        if self._verbose_logging:
            logger.info("Getting service from discovered services: %s", self._discovered_services)

        if service_type in self._discovered_services:
            if service_name is not None:
                for service in self._discovered_services[service_type]:
                    if service.service_name == service_name.replace("._MATTER._TCP.LOCAL.", "._matter._tcp.local."):
                        return service
            else:
                return self._discovered_services[service_type][0]
        else:
            return None

    def _on_service_state_change(
        self,
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
            zeroconf (Zeroconf): The Zeroconf instance managing the network operations.
            service_type (str): The service type of the mDNS service that changed state.
            name (str): The service name of the mDNS service.
            state_change (ServiceStateChange): The type of state change that occurred.

        Returns:
            None: This method does not return any value.
        """
        if self._verbose_logging:
            logger.info("Received service data. State change: %s on %s / %s", state_change, name, service_type)

        if state_change in [ServiceStateChange.Removed, ServiceStateChange.Updated]:
            return

        if self._name_filter is not None and name.upper() != self._name_filter:
            if self._verbose_logging:
                logger.info("   Name does NOT match \'%s\' vs \'%s\'", self._name_filter, name.upper())
            return

        self._last_discovery_time = time.time()

        # Gather PTR record information
        self._discovered_services.setdefault(service_type, [])

        # Prevent duplicate PtrRecord entries for the same 'service_name' under a 'service_type'
        existing_names = {r.service_name for r in self._discovered_services[service_type]}
        if name not in existing_names:
            self._discovered_services[service_type].append(PtrRecord(service_type=service_type, service_name=name))

    async def _query_service_info(self,
                                  service_type: str,
                                  service_name: str,
                                  discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
                                  query_record_types: set[int] = QUERY_RECORD_TYPES,
                                  append_results: bool = False,
                                  log_output: bool = False
                                  ) -> None:
        """
        Queries mDNS service record details for a given service instance, including SRV, TXT, A, and AAAA records.

        Args:
            service_type (str): The type of the mDNS service (e.g., '_matter._tcp.local.').
            service_name (str): The full service name instance to query (e.g., 'device123._matter._tcp.local.').
            discovery_timeout_sec (float): Maximum time in seconds to wait for discovery events. Defaults to DISCOVERY_TIMEOUT_SEC.
            query_record_types (set[int]): DNS record types to request (e.g., {33, 16, 1, 28} for SRV, TXT, A, AAAA). Defaults to QUERY_RECORD_TYPES.
            append_results (bool): If True, appends the result to `self._discovered_services` without clearing previous entries.
                                   If False, resets `self._discovered_services` before storing the new result.
            log_output (bool): Whether to log the resulting service info as JSON after discovery.

        Returns:
            None: Results are stored in `self._discovered_services`.
        """
        rec_types = "(" + ", ".join(DNS_TYPE_MAP.get(t, str(t)) for t in query_record_types) + ")"

        async with AsyncZeroconf(interfaces=self.interfaces) as azc:

            # Adds service listener
            service_listener = MdnsServiceListener()
            await azc.async_add_service_listener(service_type, service_listener)

            # Wait for the add/update service event or timeout
            try:
                logger.info(f"Service record information lookup {rec_types} for '{service_name}' in progress...")
                await wait_for(service_listener.updated_event.wait(), discovery_timeout_sec)
            except TimeoutError:
                logger.info(
                    f"Service record information lookup {rec_types} for '{service_name}' timeout ({discovery_timeout_sec} seconds) reached without an update.")

            # Prepare and perform query
            service_info = MdnsAsyncServiceInfo(name=service_name, type_=service_type)
            service_info._query_record_types = query_record_types
            is_discovered = await service_info.async_request(
                azc.zeroconf,
                discovery_timeout_sec * 1000)

            # Remove service listener and unblock waiters
            await azc.async_remove_service_listener(service_listener)
            self._event.set()

            if is_discovered:
                if self._verbose_logging:
                    logger.info(f"Service record information {rec_types} for '{service_name}' / '{service_type}' discovered.")

                # Convert discovred service info into MdnsServiceInfo object
                mdns_service_info = MdnsServiceInfo(service_info)

                # Add service to discovered services
                if not append_results:
                    self._discovered_services = {}
                self._discovered_services.setdefault(service_type, []).append(mdns_service_info)

                if not append_results:
                    if log_output:
                        self._log_output()

                return mdns_service_info

            logger.error(f"Service record information {rec_types} for '{service_name}' not found.")
            return None

    async def _monitor_discovery_silence(self, silence_threshold: float):
        """
        Monitor service discovery and end it after a period of inactivity (silence).

        Periodically checks how much time has passed since the last discovered service.
        If the duration exceeds the specified `silence_threshold` *and* at least one
        service has been discovered, it considers the discovery process complete and
        sets an internal event (`self._event`) to signal early termination of the main
        discovery logic. This coroutine also exits early if the main discovery timeout
        triggers first and sets the event.

        Args:
            silence_threshold (float): Number of seconds to wait after the last discovery
                                    before ending the discovery process early.

        Returns:
            None
        """
        while not self._event.is_set():
            # Polling delay of 0.5 seconds between checks
            # for inactivity to reduce CPU usage
            await sleep(0.5)

            time_since_last_discovery = time.time() - self._last_discovery_time

            # If the time since the last discovered service exceeds the silence threshold,
            # we assume discovery is complete and signal to stop the browsing process.
            if self._discovered_services and time_since_last_discovery >= silence_threshold:
                logger.info(f"No new mDNS services discovered after {silence_threshold:.1f} seconds, stopping browse")
                self._event.set()
                break

    def _log_output(self) -> str:
        """
        Converts the discovered services to a JSON string and logs it.

        The method is intended to be used for debugging or informational purposes, providing a clear and
        comprehensive view of all services discovered during the mDNS service discovery process.
        """
        converted_services = {
            key: [item.json_dict() for item in value]
            for key, value in self._discovered_services.items()
        }
        json_str = json.dumps(converted_services, indent=4)
        logger.info("Discovery data:\n%s", json_str)
