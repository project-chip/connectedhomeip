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
from asyncio import Event, Semaphore, TimeoutError, create_task, gather, sleep, wait_for
from typing import Dict, List, Optional

from mdns_discovery.data_classes.aaaa_record import AaaaRecord
from mdns_discovery.data_classes.mdns_service_info import MdnsServiceInfo
from mdns_discovery.data_classes.ptr_record import PtrRecord
from mdns_discovery.enums.mdns_service_type import MdnsServiceType
from mdns_discovery.mdns_async_service_info import AddressResolverIPv6, MdnsAsyncServiceInfo
from mdns_discovery.service_listeners.mdns_service_listener import MdnsServiceListener
from mdns_discovery.utils.network import get_host_ipv6_addresses
from zeroconf import IPVersion, ServiceStateChange, Zeroconf
from zeroconf.asyncio import AsyncServiceBrowser, AsyncZeroconf, AsyncZeroconfServiceTypes
from zeroconf.const import _TYPE_A, _TYPE_AAAA, _TYPE_SRV, _TYPE_TXT, _TYPES

logger = logging.getLogger(__name__)


DISCOVERY_TIMEOUT_SEC = 15
SERVICE_LISTENER_TIMEOUT_SEC = 5
QUERY_TIMEOUT_SEC = 10
DISCOVERY_SILENCE_THRESHOLD_SEC = 2
QUERY_RECORD_TYPES = {_TYPE_SRV, _TYPE_TXT, _TYPE_A, _TYPE_AAAA}


class MdnsDiscovery:

    def __init__(self):
        """
        A class for performing asynchronous mDNS service discovery using Zeroconf.

        This utility supports querying various types of mDNS records (PTR, SRV, TXT, A, AAAA)
        over IPv6 interfaces and can discover Matter-specific services such as commissioner,
        commissionable, operational, and border router services.

        Key Methods:
            - `get_commissioner_services()`: Discover Matter commissioner services.
            - `get_commissionable_services()`: Discover Matter commissionable devices.
            - `get_operational_services()`: Discover operational Matter nodes.
            - `get_border_router_services()`: Discover Thread border routers.
            - `get_all_services()`: Get service info from all available discovered services.
            - `get_ptr_records()`: Discover PTR records for given service types.
            - `get_srv_record()`: Query SRV records for a given service instance.
            - `get_txt_record()`: Query TXT records for a given service instance.
            - `get_quada_records()`: Query A/AAAA records for a given host.
            - `get_all_service_types()`: Discover all available service types.
            - `get_commissionable_subtypes()`: Discover supported subtypes for commissionable nodes.
            - `discover()`: The mDNS discovery engine powering the discovery methods.

        Attributes:
            interfaces (list[str]): IPv6 interfaces used for discovery.
            _discovered_services (dict): Stores results of service discovery.
            _event (asyncio.Event): Event used to synchronize async discovery.
        """
        # List of IPv6 addresses to use for mDNS discovery.
        self.interfaces = get_host_ipv6_addresses()

        # A dictionary to store discovered services.
        self._discovered_services = {}

        # An asyncio Event to signal when a service has been discovered
        self._event = Event()

    # Public methods
    async def get_commissioner_services(self, log_output: bool = False,
                                        discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC
                                        ) -> List[MdnsServiceInfo]:
        """
        Asynchronously discovers commissioner mDNS services on the network.

        Args:
            log_output (bool, optional): If True, logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): How long to wait for discovery, in seconds. Defaults to DISCOVERY_TIMEOUT_SEC (15 sec).

        Returns:
            List[MdnsServiceInfo]: List of discovered commissioner services, or an empty list if none found.
        """
        await self.discover(
            service_types=[MdnsServiceType.COMMISSIONER.value],
            query_service=True,
            append_results=True,
            discovery_timeout_sec=discovery_timeout_sec,
            log_output=log_output
        )

        return self._discovered_services.get(MdnsServiceType.COMMISSIONER.value, [])

    async def get_commissionable_services(self, log_output: bool = False,
                                          discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC
                                          ) -> List[MdnsServiceInfo]:
        """
        Asynchronously discovers commissionable mDNS services on the network.

        Args:
            log_output (bool, optional): If True, logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): How long to wait for discovery, in seconds. Defaults to DISCOVERY_TIMEOUT_SEC (15 sec).

        Returns:
            List[MdnsServiceInfo]: List of discovered commissionable services, or an empty list if none found.
        """
        await self.discover(
            service_types=[MdnsServiceType.COMMISSIONABLE.value],
            query_service=True,
            append_results=True,
            discovery_timeout_sec=discovery_timeout_sec,
            log_output=log_output
        )

        return self._discovered_services.get(MdnsServiceType.COMMISSIONABLE.value, [])

    async def get_operational_services(self, log_output: bool = False,
                                       discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC
                                       ) -> List[MdnsServiceInfo]:
        """
        Asynchronously discovers operational mDNS services on the network.

        Args:
            log_output (bool, optional): If True, logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): How long to wait for discovery, in seconds. Defaults to DISCOVERY_TIMEOUT_SEC (15 sec).

        Returns:
            List[MdnsServiceInfo]: List of discovered operational services, or an empty list if none found.
        """
        await self.discover(
            service_types=[MdnsServiceType.OPERATIONAL.value],
            query_service=True,
            append_results=True,
            discovery_timeout_sec=discovery_timeout_sec,
            log_output=log_output
        )

        return self._discovered_services.get(MdnsServiceType.OPERATIONAL.value, [])

    async def get_border_router_services(self, log_output: bool = False,
                                         discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC
                                         ) -> List[MdnsServiceInfo]:
        """
        Asynchronously discovers border router mDNS services on the network.

        Args:
            log_output (bool, optional): If True, logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): How long to wait for discovery, in seconds. Defaults to DISCOVERY_TIMEOUT_SEC (15 sec).

        Returns:
            List[MdnsServiceInfo]: List of discovered border router services, or an empty list if none found.
        """
        await self.discover(
            service_types=[MdnsServiceType.BORDER_ROUTER.value],
            query_service=True,
            append_results=True,
            discovery_timeout_sec=discovery_timeout_sec,
            log_output=log_output
        )

        return self._discovered_services.get(MdnsServiceType.BORDER_ROUTER.value, [])

    async def get_all_services(self, log_output: bool = False,
                               discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC
                               ) -> Dict[str, List[MdnsServiceInfo]]:
        """
        Asynchronously discovers all available mDNS services within the network.

        Optional args:
            log_output (bool, optional): If True, logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): How long to wait for discovery, in seconds. Defaults to DISCOVERY_TIMEOUT_SEC (15 sec).

        Returns:
            Dict[str, List[MdnsServiceInfo]]: A dictionary mapping service types (str) to
                                            lists of MdnsServiceInfo objects.
        """
        await self.discover(
            all_services=True,
            query_service=True,
            append_results=True,
            discovery_timeout_sec=discovery_timeout_sec,
            log_output=log_output
        )

        return self._discovered_services

    async def get_srv_record(self, service_name: str,
                             service_type: str,
                             query_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
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
            query_timeout_sec (float): Maximum time in seconds to wait for query results.
                Defaults to QUERY_TIMEOUT_SEC (10 sec).
            log_output (bool, optional): If True, logs the discovered service info
                to the console. Defaults to False.

        Returns:
            Optional[MdnsServiceInfo]:
                - MdnsServiceInfo with parsed SRV data if discovered.
                - None if no valid response is received within the timeout.
        """
        record = await self._query_service_info(
            service_name=service_name,
            service_type=service_type,
            query_timeout_sec=query_timeout_sec,
            query_record_types={_TYPE_SRV}
        )

        if log_output:
            self._log_output()

        return record

    async def get_txt_record(self, service_name: str,
                             service_type: str,
                             query_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
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
            query_timeout_sec (float): Maximum time in seconds to wait for query results.
                Defaults to QUERY_TIMEOUT_SEC (10 sec).
            log_output (bool, optional): If True, logs the discovered service info
                to the console. Defaults to False.

        Returns:
            Optional[MdnsServiceInfo]:
                - MdnsServiceInfo with parsed TXT data if discovered.
                - None if no valid response is received within the timeout.
        """
        record = await self._query_service_info(
            service_name=service_name,
            service_type=service_type,
            query_timeout_sec=query_timeout_sec,
            query_record_types={_TYPE_TXT}
        )

        if log_output:
            self._log_output()

        return record

    async def get_quada_records(self, hostname: str,
                                query_timeout_sec: float = QUERY_TIMEOUT_SEC,
                                log_output: bool = False
                                ) -> list[AaaaRecord]:
        """
        Asynchronously retrieves the AAAA (IPv6) record of a device on the local network via mDNS.

        Args:
            hostname (str): The fully qualified hostname of the target device.
                Example: '00155DF32EEB.local.'

            query_timeout_sec (float): Maximum time in seconds to wait for query results.
                Defaults to QUERY_TIMEOUT_SEC (10 sec).

            log_output (bool, optional): If True, logs the discovered records to the console.
                Defaults to False.

        Returns:
            list[AaaaRecord]: A list of discovered AaaaRecord objects.
        """
        logger.info(f"Service record information lookup (AAAA) for '{hostname}' in progress...")

        async with AsyncZeroconf(interfaces=self.interfaces) as azc:
            # Perform AAAA query
            addr_resolver = AddressResolverIPv6(hostname)

            is_discovered = await addr_resolver.async_request(
                azc.zeroconf,
                timeout_ms=query_timeout_sec * 1000)

            if is_discovered:
                logger.info(f"Service record information (AAAA) for '{hostname}' discovered.")

                # Get IPv6 addresses and convert to AaaaRecord objects
                ipv6_addresses = addr_resolver.ip_addresses_by_version(IPVersion.V6Only)
                quada_records: list[AaaaRecord] = []

                if ipv6_addresses:
                    quada_records = [AaaaRecord(ipv6) for ipv6 in ipv6_addresses]

                # Adds service to discovered services
                self._discovered_services = {hostname: list(quada_records)}

                if log_output:
                    self._log_output()

                return quada_records

            logger.error(f"Service record information (AAAA) for '{hostname}' not found.")
            return []

    async def get_ptr_records(self,
                              service_types: list[str],
                              discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
                              log_output: bool = False,
                              ) -> list[PtrRecord]:
        """
        Asynchronously discovers mDNS PTR records for the given service types.

        Args:
            service_types (list[str]): A list of service types (e.g., "_matter._tcp.local.", "_Ixxxx._sub._matter._tcp.local.") to browse for.
            discovery_timeout_sec (float): Time in seconds to wait for responses after sending the browse request.
                Defaults to DISCOVERY_TIMEOUT_SEC (15 sec).
            log_output (bool, optional): If True, logs the discovered records to the console.
                Defaults to False.

        Returns:
            list[PtrRecord]: A list of discovered PtrRecord objects.
        """
        logger.info(f"Service record information lookup (PTR) for '{service_types}' in progress...")
        await self.discover(
            discovery_timeout_sec=discovery_timeout_sec,
            log_output=log_output,
            service_types=service_types
        )

        if self._discovered_services:
            logger.info(f"Service record information (PTR) for '{service_types}' discovered.")

            return [
                record
                for record_list in self._discovered_services.values()
                for record in record_list
            ]

        logger.error(f"Service record information (PTR) for '{service_types}' not found.")
        return []

    async def get_all_service_types(self, log_output: bool = False, discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC) -> List[str]:
        """
        Asynchronously discovers all available mDNS service types within the network and returns a list
        of the service types discovered. This method utilizes the AsyncZeroconfServiceTypes.async_find()
        function to perform the network scan for mDNS services.

        Optional args:
            log_output (bool, optional): If True, logs the discovered service types to the console.
                Defaults to False.
            discovery_timeout_sec (float): The maximum time (in seconds) to wait for the discovery process.
                Defaults to DISCOVERY_TIMEOUT_SEC (15 sec).

        Returns:
            List[str]: A list containing the service types (str) of the discovered mDNS services. Each
                    element in the list is a string representing a unique type of service found during
                    the discovery process.
        """
        logger.info("Discovering all available mDNS service types...")
        async with AsyncZeroconf(interfaces=self.interfaces) as azc:
            try:
                service_types = list(set(await wait_for(AsyncZeroconfServiceTypes.async_find(aiozc=azc, interfaces=self.interfaces), timeout=discovery_timeout_sec)))
            except TimeoutError:
                logger.info(f"mDNS service types discovery timed out after {discovery_timeout_sec} seconds.")
                service_types = []

            if log_output:
                logger.info(
                    "\n\nDiscovered mDNS service types:\n%s\n", "\n".join(f"  - {s}" for s in service_types))

            return service_types

    async def get_commissionable_subtypes(self, log_output: bool = False, discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC) -> list[str]:
        """
        Asynchronously retrieves a list of commissionable mDNS service subtypes.

        Args:
            log_output (bool, optional): If True, logs the discovered commissionable subtypes to the console.
                Defaults to False.
            discovery_timeout_sec (float, optional): Timeout in seconds for the service type discovery.
                Defaults to DISCOVERY_TIMEOUT_SEC (15 sec).

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
                       service_types: Optional[List[str]] = None,
                       all_services: bool = False,
                       discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
                       query_service: bool = False,
                       query_timeout_sec=QUERY_TIMEOUT_SEC,
                       append_results: bool = False,
                       log_output: bool = False
                       ) -> None:
        """
        Asynchronously discovers network services using multicast DNS (mDNS).

        Args:
            service_types (Optional[List[str]]): Specific service types to discover
                (e.g., ['_matterc._udp.local.']).
            all_services (bool): If True, discovers all available mDNS services.
            discovery_timeout_sec (float): Maximum time in seconds to wait for service
                announcements during the discovery phase. Defaults to DISCOVERY_TIMEOUT_SEC (15 sec).
            query_service (bool): If True, queries the full service info for each
             discovered service name. Defaults to False.
            query_timeout_sec (float): Maximum time in seconds to wait for the record
                query phase. Only applies when `query_service` is True. Defaults to QUERY_TIMEOUT_SEC (10 sec).
            append_results (bool): If True, appends results to `self._discovered_services`
                without clearing previous entries. If False, clears the dictionary before
                storing new results. Only applies when `query_service` is True. Defaults to False.
            log_output (bool, optional): If True, logs the discovered services to the
                console. Defaults to False.

        Raises:
            ValueError: If both `all_services` and `service_types` are provided.
            ValueError: If `service_types` is provided but is an empty list.
            ValueError: If `query_timeout_sec` is set when `query_service` is False.
            ValueError: If `append_results` is True when `query_service` is False.

        Returns:
            None: This method does not return any value.`
        """
        if all_services and service_types:
            raise ValueError("Specify either 'all_services' or 'service_types', not both.")

        if service_types is not None and not service_types:
            raise ValueError("`service_types` cannot be an empty list.")

        if not query_service and query_timeout_sec != QUERY_TIMEOUT_SEC:
            raise ValueError("`query_timeout_sec` is only valid when `query_service=True`.")

        if not query_service and append_results:
            raise ValueError("`append_results` is only valid when `query_service=True`.")

        types = (
            await self.get_all_service_types(
                discovery_timeout_sec=discovery_timeout_sec,
                log_output=False
            )
            if all_services else service_types
        )

        logger.info(f"Browsing for mDNS service(s) of type: {types}")

        # Setup fresh discovery
        self._event.clear()
        self._discovered_services = {}
        self._last_discovery_time = time.time()

        async with AsyncZeroconf(interfaces=self.interfaces) as azc:
            aiobrowser = AsyncServiceBrowser(
                zeroconf=azc.zeroconf, type_=types, handlers=[self._on_service_state_change]
            )

            # Background monitor to end discovery early after
            # a period of silence (inactivity, no new services)
            create_task(self._monitor_discovery_silence(silence_threshold=DISCOVERY_SILENCE_THRESHOLD_SEC))

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

            # Log discovered services stats found during the browse
            services_count = sum(len(ptr_list) for ptr_list in self._discovered_services.values())
            types_count = len(self._discovered_services)
            logger.info(f"Discovered {services_count} mDNS service(s) across {types_count} service type(s)")
            if log_output:
                self._log_output()

            # If service querying is enabled, perform controlled parallel queries to retrieve
            # service information (TXT, SRV, A/AAAA) for each discovered PTR record. This
            # helps prevent system overload by limiting concurrent mDNS queries.
            if query_service:
                logger.info("Querying service information for discovered services...")
                semaphore = Semaphore(5)  # Limit to 5 concurrent queries

                async def limited_query(ptr):
                    async with semaphore:
                        await self._query_service_info(
                            service_type=ptr.service_type,
                            service_name=ptr.service_name,
                            query_timeout_sec=query_timeout_sec,
                            append_results=True
                        )

                tasks = []
                for ptr_list in self._discovered_services.values():
                    for ptr in ptr_list:
                        tasks.append(limited_query(ptr))

                self._discovered_services = {}

                await gather(*tasks)

                # Log the full service info details
                # from all the discovered services
                if log_output:
                    self._log_output()

    # Private methods
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

        Args:
            zeroconf (Zeroconf): The Zeroconf instance managing the network operations.
            service_type (str): The service type of the mDNS service that changed state.
            name (str): The service name of the mDNS service.
            state_change (ServiceStateChange): The type of state change that occurred.

        Returns:
            None: This method does not return any value.
        """
        # Exit if status isn't 'Added'
        if state_change != ServiceStateChange.Added:
            return

        logger.info(f"Service info added. Service name: '{name}', Service Type: '{service_type}'")

        # Used by the _monitor_discovery_silence function
        self._last_discovery_time = time.time()

        # Gather PTR record information
        self._discovered_services.setdefault(service_type, [])
        existing_names = {r.service_name for r in self._discovered_services[service_type]}
        if name not in existing_names:
            self._discovered_services[service_type].append(PtrRecord(service_type=service_type, service_name=name))

    async def _query_service_info(self,
                                  service_type: str,
                                  service_name: str,
                                  query_timeout_sec: float = QUERY_TIMEOUT_SEC,
                                  query_record_types: set[int] = QUERY_RECORD_TYPES,
                                  append_results: bool = False
                                  ) -> Optional[MdnsServiceInfo]:
        """
        Queries mDNS service record details for a given service instance, including SRV, TXT, A, and AAAA records.

        Args:
            service_type (str): The type of the mDNS service (e.g., '_matter._tcp.local.').
            service_name (str): The full service name instance to query (e.g., 'device123._matter._tcp.local.').
            query_timeout_sec (float): Maximum time in seconds to wait for query results. Defaults to QUERY_TIMEOUT_SEC (10 sec).
            query_record_types (set[int]): DNS record types to request (e.g., {33, 16, 1, 28} for SRV, TXT, A, AAAA).
                Defaults to QUERY_RECORD_TYPES (SRV, TXT, A, AAAA).
            append_results (bool): If True, appends the results to `self._discovered_services` without clearing previous entries.
                                   If False, clears `self._discovered_services` before storing the new result. Defaults to False.

        Returns:
            Optional[MdnsServiceInfo]: A fully resolved service instance containing details such as host address, port,
            and associated TXT records, or None if the service could not be resolved.
        """
        rec_types = "(" + ", ".join(_TYPES.get(t, str(t)).upper() for t in query_record_types) + ")"

        async with AsyncZeroconf(interfaces=self.interfaces) as azc:

            # Adds service listener
            service_listener = MdnsServiceListener()
            await azc.async_add_service_listener(service_type, service_listener)

            # Wait for the add/update service event or timeout
            await service_listener.wait_for_service_update(service_name, rec_types, SERVICE_LISTENER_TIMEOUT_SEC)

            # Prepare and perform query
            service_info = MdnsAsyncServiceInfo(name=service_name, type_=service_type)
            service_info._query_record_types = query_record_types
            is_discovered = await service_info.async_request(
                zc=azc.zeroconf,
                timeout_ms=query_timeout_sec * 1000)

            # Remove service listener
            await azc.async_remove_service_listener(service_listener)

            if is_discovered:
                logger.info(f"Service record information {rec_types} for '{service_name}' / '{service_type}' discovered.")

                # Convert discovered service info into MdnsServiceInfo object
                mdns_service_info = MdnsServiceInfo(service_info)

                # - If not appending service info results to the discovered services list,
                #   empty the list on every call so it holds only a single result (as used
                #   by the `get_srv_record` and `get_txt_record` methods).
                # - Otherwise append service info results to the discovered services
                #   list (as used by the `discover` method)
                if not append_results:
                    self._discovered_services = {}

                # Add service to discovered services
                self._discovered_services.setdefault(service_type, []).append(mdns_service_info)

                return mdns_service_info

            logger.error(f"Service record information {rec_types} for '{service_name}' not found.")
            return None

    async def _monitor_discovery_silence(self, silence_threshold: float) -> None:
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

    def _log_output(self) -> None:
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
