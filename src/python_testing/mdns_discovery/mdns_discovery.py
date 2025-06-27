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

import ast
import ipaddress
import json
import logging
from asyncio import Event, TimeoutError, ensure_future, wait_for
from dataclasses import asdict, dataclass
from enum import Enum
from typing import Dict, List, Optional, cast

from mdns_discovery.mdns_async_service_info import DNSRecordType, MdnsAsyncServiceInfo
from zeroconf import IPVersion, ServiceListener, ServiceStateChange, Zeroconf
from zeroconf._dns import DNSAddress, DNSRecord
from zeroconf._engine import AsyncListener
from zeroconf._protocol.incoming import DNSIncoming
from zeroconf.asyncio import AsyncServiceBrowser, AsyncServiceInfo, AsyncZeroconf, AsyncZeroconfServiceTypes

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


@dataclass
class MdnsAddressInfo:
    # The domain name (hostname) associated with the address record.
    hostname: str

    # The type of DNS record (e.g., 'A' or 'AAAA').
    record_type: str

    # The resolved IPv6 address for the given name.
    address: str


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

    def __init__(self, retries: int = 3, delay: float = 1.0):
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
        """
        # An instance of Zeroconf to manage mDNS operations.
        self._azc = AsyncZeroconf(ip_version=IPVersion.All)

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
            discovered_services = list(await wait_for(AsyncZeroconfServiceTypes.async_find(), timeout=discovery_timeout_sec))
        except TimeoutError:
            logger.info(f"MDNS service types discovery timed out after {discovery_timeout_sec} seconds.")
            discovered_services = []

        if log_output:
            logger.info(f"MDNS discovered service types: {discovered_services}")

        return discovered_services

    async def get_srv_record(self, service_name: str,
                             service_type: str = None,
                             discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
                             log_output: bool = False
                             ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers the SRV record associated with an mDNS service.

        This method listens for service announcements and actively queries the network 
        for SRV records corresponding to the specified service name and type. If discovered,
        it constructs and returns an MdnsServiceInfo object with parsed SRV data.

        Args:
            service_name (str): The full mDNS service instance name.
                Example: '974B15BD2CC5278E-0000000012344321._matter._tcp.local.'
            service_type (str, optional): The service type.
                Example: '_matter._tcp.local.' If None, only the name will be used.
            discovery_timeout_sec (float, optional): Maximum time to wait for discovery.
                Defaults to DISCOVERY_TIMEOUT_SEC.
            log_output (bool, optional): If True, logs discovered service info. Defaults to False.

        Returns:
            Optional[MdnsServiceInfo]:
                - MdnsServiceInfo containing parsed SRV data if discovered.
                - None if the service was not found within the timeout.
        """
        mdns_service_info = None

        logger.info(f"Looking for MDNS record, type 'SRV', service name '{service_name}'")

        # Adds service listener
        service_listener = MdnsServiceListener()
        await self._azc.async_add_service_listener(service_type, service_listener)

        # Wait for the add/update service event or timeout
        try:
            await wait_for(service_listener.updated_event.wait(), discovery_timeout_sec)
        except TimeoutError:
            logger.info(f"Service lookup for {service_name} timeout ({discovery_timeout_sec} seconds) reached without an update.")
        finally:
            await self._azc.async_remove_service_listener(service_listener)

        # Prepare and perform query
        service_info = MdnsAsyncServiceInfo(name=service_name, type_=service_type)

        is_discovered = await service_info.async_request(
            self._azc.zeroconf,
            discovery_timeout_sec * 1000,
            record_type=DNSRecordType.SRV)

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

    async def get_txt_record(self, service_name: str,
                             service_type: str = None,
                             discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
                             log_output: bool = False
                             ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers the TXT record associated with an mDNS service.

        This method attempts to locate and retrieve the TXT record for a given service name 
        and service type on the local network. It first listens for service announcements, 
        then sends an active mDNS query for TXT records. If the TXT record is not found 
        through the standard response, it attempts to recover it from the local Zeroconf cache.

        Args:
            service_name (str): The full mDNS service instance name.
                Example: '974B15BD2CC5278E-0000000012344321._matter._tcp.local.'
            service_type (str, optional): The service type.
                Example: '_matter._tcp.local.' If None, only the name will be used.
            discovery_timeout_sec (float, optional): Maximum time to wait for discovery.
                Defaults to DISCOVERY_TIMEOUT_SEC.
            log_output (bool, optional): If True, logs discovered service info. Defaults to False.

        Returns:
            Optional[MdnsServiceInfo]: An MdnsServiceInfo object containing the TXT record and 
            other metadata, or None if the service could not be resolved.
        """
        logger.info(f"Looking for MDNS record, type 'TXT', service name '{service_name}'")
        mdns_service_info = None
        txt_from_cache = None

        # Adds service listener
        service_listener = MdnsServiceListener()
        await self._azc.async_add_service_listener(service_type, service_listener)

        # Wait for the add/update service event or timeout
        # This is necessary whem requesting TXT records,
        # as the service may not be immediately available
        try:
            await wait_for(service_listener.updated_event.wait(), discovery_timeout_sec)
        except TimeoutError:
            logger.info(f"Service lookup for {service_name} timeout ({discovery_timeout_sec} seconds) reached without an update.")
        finally:
            await self._azc.async_remove_service_listener(service_listener)

        # Prepare and perform query
        service_info = MdnsAsyncServiceInfo(name=service_name, type_=service_type)

        is_discovered = await service_info.async_request(
            self._azc.zeroconf,
            discovery_timeout_sec * 1000,
            record_type=DNSRecordType.TXT)

        if is_discovered:
            if not service_info.decoded_properties:
                logging.info(f"TXT Record found but no properties for service {service_name} of type {service_type}")
                for service, records in self._azc.zeroconf.cache.cache.items():
                    for _, dns_record in records.items():
                        if service_name.lower() == dns_record.name.lower():
                            txt_found = get_txt_from_dns_record(dns_record)
                            if txt_found:
                                txt_from_cache = txt_found
                                logging.info("TXT data recovered from cache.")

            # Adds service to discovered services
            mdns_service_info = self._to_mdns_service_info_class(service_info)
            if txt_from_cache:
                mdns_service_info.txt_record = txt_from_cache
            self._discovered_services = {}
            self._discovered_services[service_type] = []
            if mdns_service_info is not None:
                self._discovered_services[service_type].append(mdns_service_info)

            if log_output:
                self._log_output()

            return mdns_service_info

    async def get_quada_record(self, hostname: str,
                               discovery_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
                               log_output: bool = False
                               ) -> Optional[MdnsAddressInfo]:
        """
        Asynchronously retrieves the AAAA (IPv6) record of a device on the local network via mDNS.

        Args:
            hostname (str): The fully qualified hostname of the target device.
                Example: '00155DF32EEB.local.'

            discovery_timeout_sec (float, optional): Time in seconds to wait for discovery.
                Defaults to 3.

            log_output (bool, optional): Whether to log the discovered information. Defaults to False.

        Returns:
            Optional[MdnsAddressInfo]: An MdnsAddressInfo object containing the IPv6 address,
            record type, and hostname if found, otherwise None.
        """
        logger.info(f"Looking for MDNS record, type 'AAAA',  hostname '{hostname}'")

        service_type = MdnsServiceType.OPERATIONAL.value
        service_info = MdnsAsyncServiceInfo(name=hostname, type_=service_type)
        mdns_address_info = None

        await service_info.async_request(
            self._azc.zeroconf,
            timeout=discovery_timeout_sec * 1000,
            record_type=DNSRecordType.AAAA
        )

        for protocols in self._azc.zeroconf.engine.protocols:
            listener = cast(AsyncListener, protocols)
            if not listener.data:
                continue

            dns_incoming = DNSIncoming(listener.data)
            if not dns_incoming.data:
                continue

            answers = dns_incoming.answers()
            if not answers:
                continue

            dns_address: DNSAddress = answers.pop(0)
            mdns_address_info = self._to_mdns_address_info_class(dns_address)

        # Adds service to discovered services
        if mdns_address_info:
            self._discovered_services = {}
            self._discovered_services[service_type] = []
            self._discovered_services[service_type].append(mdns_address_info)

            if log_output:
                self._log_output()

        return mdns_address_info

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

        aiobrowser = AsyncServiceBrowser(zeroconf=self._azc.zeroconf,
                                         type_=self._service_types,
                                         handlers=[self._on_service_state_change]
                                         )

        try:
            await wait_for(self._event.wait(), timeout=discovery_timeout_sec)
        except TimeoutError:
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

        ensure_future(self._query_service_info(
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

    def _to_mdns_address_info_class(self, dns_address: DNSAddress) -> MdnsServiceInfo:
        """
        Converts a DNSAddress object into an MdnsAddressInfo data class.

        Args:
            dns_address (DNSAddress): The dns address to convert.

        Returns:
            MdnsAddressInfo: The converted dns address as a data class.
        """
        mdns_address_info = MdnsAddressInfo(
            hostname=dns_address.name,
            record_type=dns_address.get_type(dns_address.type),
            address=str(ipaddress.IPv6Address(dns_address.address))
        )

        return mdns_address_info

    def _get_instance_name(self, service_info: AsyncServiceInfo) -> str:
        if service_info.type:
            return service_info.name[: len(service_info.name) - len(service_info.type) - 1]
        else:
            return service_info.name

    async def _get_service(self, service_type: MdnsServiceType,
                           log_output: bool,
                           discovery_timeout_sec: float,
                           service_name: str = None,
                           ) -> Optional[MdnsServiceInfo]:
        """
        Asynchronously discovers a specific type of mDNS service within the network and returns its details.

        Args:
            service_type (MdnsServiceType): The enum representing the type of mDNS service to discover.
            log_output (bool): Logs the discovered services to the console. Defaults to False.
            discovery_timeout_sec (float): Defaults to 15 seconds.
            service_name (str): Defaults to none as currently only utilized to gather specific record in multiple discovery records if available

        Returns:
            Optional[MdnsServiceInfo]: An instance of MdnsServiceInfo representing the discovered service, if
                                    any. Returns None if no service of the specified type is discovered within
                                    the timeout period.
        """
        self._discovered_services = {}
        self._service_types = [service_type.value]
        await self._discover(discovery_timeout_sec, log_output)

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

    def _log_output(self) -> str:
        """
        Converts the discovered services to a JSON string and logs it.

        The method is intended to be used for debugging or informational purposes, providing a clear and
        comprehensive view of all services discovered during the mDNS service discovery process.
        """
        converted_services = {key: [asdict(item) for item in value] for key, value in self._discovered_services.items()}
        json_str = json.dumps(converted_services, indent=4)
        logger.info("Discovery data:\n%s", json_str)


def get_txt_from_dns_record(dns_record: DNSRecord) -> dict[str, str | None]:
    """
    Decodes a DNS TXT record into a dictionary of key-value pairs.

    This function takes a DNSRecord object (typically of TXT type), extracts the
    raw byte content using its string representation, and parses it according to
    the DNS TXT format (length-prefixed key=value entries).

    Returns:
        A dictionary where each key and value is a string. Entries without a value
        (e.g., "flag") are skipped. If the record is not a TXT type or the format is
        invalid, an empty dictionary is returned.

    Raises:
        ValueError: If the TXT data cannot be parsed from the record.
    """
    dns_record_str = str(dns_record)
    if "record[txt," not in dns_record_str:
        return {}

    try:
        _, byte_literal = dns_record_str.rsplit(",", 1)
        raw_bytes = ast.literal_eval(byte_literal.strip())

        decoded: dict[str, str] = {}
        i = 0
        while i < len(raw_bytes):
            length = raw_bytes[i]
            i += 1
            entry = raw_bytes[i:i+length].decode("utf-8", errors="replace").strip()
            i += length

            if "=" in entry:
                k, v = entry.split("=", 1)
                decoded[k.strip()] = v.strip()

        return decoded

    except Exception as e:
        raise ValueError(f"Failed to decode TXT record: {e}")
