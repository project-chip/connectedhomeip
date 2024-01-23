#
#    Copyright (c) 2023 Project CHIP Authors
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
from zeroconf.asyncio import AsyncServiceInfo, AsyncServiceBrowser
from mdns_discovery.exceptions import DiscoveryNotPerformedError
from .mdns_type_enum import MdnsType
from zeroconf import ServiceStateChange, Zeroconf


class MdnsDiscovery:
    def __init__(self, tc):
        """
        Initializes the MdnsDiscovery instance with necessary configurations.

        Args:
            tc: A test case object containing node and fabric information.

        Attributes:
            _zc (Zeroconf): An instance of Zeroconf to manage mDNS operations.
            _tc: The test case object.
            _service_types (list): A list of mDNS service types to discover.
            _discovered_services (dict): A dictionary to store discovered services.
            _discovery_performed (bool): Flag to indicate if discovery has been performed.
            dut_operational_service_name (str): The operational service name for the DUT (Device Under Test).
        """
        self._zc: Zeroconf = Zeroconf()
        self._tc = tc
        self._service_types = [mdns_type.value for mdns_type in MdnsType]
        self._discovered_services = {}
        self._discovery_performed = False
        self.dut_operational_service_name = self._get_dut_operational_service_name()

    async def discover(self, discovery_duration: float = 3) -> None:
        """
        Asynchronously discovers network services of specified types using mDNS.

        This method initiates a multicast DNS (mDNS) service discovery process to find
        network services matching the specified types in the `MdnsType` enum. It uses
        an asynchronous service browser to listen for service announcements and updates.

        The discovery process is given a configurable period to operate, during which the method
        asynchronously waits to allow network responses to be received and processed.

        Args:
            discovery_duration (float): The duration in seconds for which the discovery
                                        process should wait to allow for service announcements.
                                        If not provided, defaults to 3 seconds.

        Note:
            The discovery duration may need adjustment based on network conditions and the
            expected response time for service announcements.

        Returns:
            None: This method does not return any value. The results of the service discovery
                  are handled by the callback function.
        """
        self._discovered_services = {mdns_type.name: [] for mdns_type in MdnsType}
        AsyncServiceBrowser(zeroconf=self._zc,
                            type_=self._service_types,
                            handlers=[self.on_service_state_change]
        )
        # Wait for discovery
        await asyncio.sleep(discovery_duration)
        self._discovery_performed = True

    def on_service_state_change(
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
            asyncio.ensure_future(self.query_service_info(
                zeroconf,
                service_type,
                name)
            )

    async def query_service_info(self, zeroconf: Zeroconf, service_type: str, name: str) -> None:
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
        service_info: AsyncServiceInfo = AsyncServiceInfo(service_type, name)
        is_service_discovered = await service_info.async_request(zeroconf, 3000)

        if is_service_discovered:
            # Add service info to discovered services
            for mdns_type in MdnsType:
                if service_type == mdns_type.value:
                    service_info_dict = self.service_info_to_dict(service_info)
                    self._discovered_services[mdns_type.name].append(service_info_dict)
                    break

    def service_info_to_dict(self, service_info: AsyncServiceInfo):
        """
        Converts an AsyncServiceInfo object to a dictionary for easier access and manipulation.

        This method facilitates the handling of service information by converting the AsyncServiceInfo
        object, which contains details about an mDNS service, into a more accessible dictionary format.

        Args:
            service_info (AsyncServiceInfo): The AsyncServiceInfo object to convert.
            
        Returns:
            dict: A dictionary representation of the AsyncServiceInfo object.
        """
        service_info_dict = {
            "service_name": service_info.name,
            "name": service_info.get_name(),
            "type": service_info.type,
            "server": service_info.server,
            "port": service_info.port,
            "addresses": service_info.parsed_addresses(),
            "txt_record": service_info.decoded_properties,
            "priority": service_info.priority,
            "interface_index": service_info.interface_index,
            "weight": service_info.weight,
            "host_ttl": service_info.host_ttl,
            "other_ttl": service_info.other_ttl
        }
        return service_info_dict

    def _get_dut_operational_service_name(self):
        """
        Constructs the operational service name for the DUT.

        This method generates a service name string based on the DUT's node ID and the compressed
        fabric ID, following the operational service naming convention.

        Returns:
            str: A string representing the operational service name.
        """
        node_id = self._tc.dut_node_id
        node_id_hex = str(hex(int(node_id))[2:].upper()).zfill(16)
        compressed_fabricid = self._tc.default_controller.GetCompressedFabricId()
        compressed_fabricid_hex = hex(int(compressed_fabricid))[2:].upper()
        name = f"{compressed_fabricid_hex}-{node_id_hex}"
        service_name = f"{name}.{MdnsType.OPERATIONAL}"
        return service_name

    def _getServiceInfo(self, mdns_type: MdnsType):
        """
        This method returns the discovered service information for a specified mDNS type.
        It raises an exception if the discovery process has not been performed before calling this method.

        Args:
            mdns_type (MdnsType): The mDNS type for which to retrieve the service info.

        Returns:
            list: A list of discovered services of the specified mDNS type.

        Raises:
            DiscoveryNotPerformedError: If discovery has not been performed before calling this method.
        """
        if not self._discovery_performed:
            raise DiscoveryNotPerformedError()

        return self._discovered_services[mdns_type.name]

    def getCommisionerServiceInfo(self):
        """
        Retrieves the service information for Commissioner services.

        This method is a convenience wrapper around `_getServiceInfo` specifically for
        returning Commissioner services.

        Returns:
            list: A list of discovered Commissioner services.

        Note: Each element in the array is a service info dict which might have nested
              dicts as it's value.

        Service Info Dict Keys:
            service_name: The unique name of the mDNS service.
            name: The human-readable name of the service.
            type: The type of the service, typically indicating the service protocol and domain.
            server: The domain name of the machine hosting the service.
            port: The network port on which the service is available.
            addresses: A list of IP addresses associated with the service.
            txt_record: A dictionary of key-value pairs representing the service's metadata.
            priority: The priority of the service, used in service selection when multiple instances are available.
            interface_index: The network interface index on which the service is advertised.
            weight: The relative weight for records with the same priority, used in load balancing.
            host_ttl: The time-to-live value for the host name in the DNS record.
            other_ttl: The time-to-live value for other records associated with the service.
        """
        return self._getServiceInfo(MdnsType.COMMISIONER)

    def getCommissionableServiceInfo(self):
        """
        Retrieves the service information for Commissionable services.

        This method is a convenience wrapper around `_getServiceInfo` specifically for
        returning Commissionable services.

        Returns:
            list: A list of discovered Commissionable services.
            
        Note: Each element in the array is a service info dict which might have nested
              dicts as it's value.

        Service Info Dict Keys:
            service_name: The unique name of the mDNS service.
            name: The human-readable name of the service.
            type: The type of the service, typically indicating the service protocol and domain.
            server: The domain name of the machine hosting the service.
            port: The network port on which the service is available.
            addresses: A list of IP addresses associated with the service.
            txt_record: A dictionary of key-value pairs representing the service's metadata.
            priority: The priority of the service, used in service selection when multiple instances are available.
            interface_index: The network interface index on which the service is advertised.
            weight: The relative weight for records with the same priority, used in load balancing.
            host_ttl: The time-to-live value for the host name in the DNS record.
            other_ttl: The time-to-live value for other records associated with the service.            
        """
        return self._getServiceInfo(MdnsType.COMMISSIONABLE)

    def getOperationalServiceInfo(self):
        """
        Retrieves the service information for Operational services.

        This method is a convenience wrapper around `_getServiceInfo` specifically for
        returning Operational services.

        Returns:
            list: A list of discovered Operational services.
            
        Note: Each element in the array is a service info dict which might have nested
              dicts as it's value.

        Service Info Dict Keys:
            service_name: The unique name of the mDNS service.
            name: The human-readable name of the service.
            type: The type of the service, typically indicating the service protocol and domain.
            server: The domain name of the machine hosting the service.
            port: The network port on which the service is available.
            addresses: A list of IP addresses associated with the service.
            txt_record: A dictionary of key-value pairs representing the service's metadata.
            priority: The priority of the service, used in service selection when multiple instances are available.
            interface_index: The network interface index on which the service is advertised.
            weight: The relative weight for records with the same priority, used in load balancing.
            host_ttl: The time-to-live value for the host name in the DNS record.
            other_ttl: The time-to-live value for other records associated with the service.            
        """
        return self._getServiceInfo(MdnsType.OPERATIONAL)

    def getBorderRouterServiceInfo(self):
        """
        Retrieves the service information for Border Router services.

        This method is a convenience wrapper around `_getServiceInfo` specifically for
        returning Border Router services.

        Returns:
            list: A list of discovered Border Router services.
            
        Note: Each element in the array is a service info dict which might have nested
              dicts as it's value.

        Service Info Dict Keys:
            service_name: The unique name of the mDNS service.
            name: The human-readable name of the service.
            type: The type of the service, typically indicating the service protocol and domain.
            server: The domain name of the machine hosting the service.
            port: The network port on which the service is available.
            addresses: A list of IP addresses associated with the service.
            txt_record: A dictionary of key-value pairs representing the service's metadata.
            priority: The priority of the service, used in service selection when multiple instances are available.
            interface_index: The network interface index on which the service is advertised.
            weight: The relative weight for records with the same priority, used in load balancing.
            host_ttl: The time-to-live value for the host name in the DNS record.
            other_ttl: The time-to-live value for other records associated with the service.            
        """
        return self._getServiceInfo(MdnsType.BORDER_ROUTER)
