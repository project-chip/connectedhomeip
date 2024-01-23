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
    """
    A helper class for managing operational mDNS tasks within test cases.

    This class provides utilities for mDNS-related operations needed in tests,
    such as discovering services, registering listeners, and querying service information.
    It's designed to work closely with a specific test case, using its context
    or functionalities for network service interactions.

    Attributes:
        _listener (EmptyServiceListener): A listener for mDNS service events.
        _zc (AsyncZeroconf): An instance of AsyncZeroconf for asynchronous network discovery.
        _tc: The test case with which this helper is associated.
        _service_types: Stores the types of services discovered via mDNS.
        _service_info: Stores detailed information about discovered services.
        _name (str): The name of the service constructed for mDNS operations.

    Args:
        tc: An instance of a test case.
    """
    def __init__(self, tc):
        """
        Initializes the OperationalMdnsHelper instance with a given test case.

        Args:
            tc: An instance of the test case that requires mDNS functionality.
        """
        self._zc: Zeroconf = Zeroconf()
        self._tc = tc
        self._service_types = [mdns_type.value for mdns_type in MdnsType]
        self._discovered_services = {}
        self._discovery_performed = False
        self._dut_operational_service_name = self._get_dut_operational_service_name()

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
                            handlers=[self.async_on_service_state_change]
        )
        # Wait for discovery
        await asyncio.sleep(discovery_duration)
        self._discovery_performed = True

    def async_on_service_state_change(
        self,
        zeroconf: Zeroconf,
        service_type: str,
        name: str,
        state_change: ServiceStateChange
    ) -> None:
        """
        Callback method triggered on mDNS service state changes.

        This method is called by the AsyncServiceBrowser whenever there is a change in the state
        of a service being browsed. It specifically reacts to the addition of new services by
        scheduling the `async_display_service_info` coroutine for execution.

        Args:
            zeroconf (Zeroconf): The Zeroconf instance associated with the service browser.
            service_type (str): The type of the service that changed state.
            name (str): The name of the service that changed state.
            state_change (ServiceStateChange): An enum indicating the type of state change
                                            (e.g., service added, removed, or updated).

        Note:
            This method currently only handles the 'Added' state change. Other state changes
            like 'Removed' or 'Updated' are not processed.

        Returns:
            None: This method does not return any value. It schedules `async_display_service_info`
                for execution when a service is added.
        """
        if state_change.value == ServiceStateChange.Added.value:
            asyncio.ensure_future(self.gather_service_info(
                zeroconf,
                service_type,
                name)
            )

    async def gather_service_info(self, zeroconf: Zeroconf, service_type: str, name: str) -> None:
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

    def service_info_to_dict(self, service_info):
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
        node_id = self._tc.dut_node_id
        node_id_hex = str(hex(int(node_id))[2:].upper()).zfill(16)
        compressed_fabricid = self._tc.default_controller.GetCompressedFabricId()
        compressed_fabricid_hex = hex(int(compressed_fabricid))[2:].upper()
        name = f"{compressed_fabricid_hex}-{node_id_hex}"
        service_name = f"{name}.{MdnsType.OPERATIONAL}"
        return service_name

    def _getServiceInfo(self, mdns_type: MdnsType):
        # Check if discovery has been performed
        if not self._discovery_performed:
            raise DiscoveryNotPerformedError()

        return self._discovered_services[mdns_type.name]

    def getCommisionerServiceInfo(self):
        return self._getServiceInfo(MdnsType.COMMISIONER)

    def getCommissionableServiceInfo(self):
        return self._getServiceInfo(MdnsType.COMMISSIONABLE)

    def getOperationalServiceInfo(self):
        return self._getServiceInfo(MdnsType.OPERATIONAL)

    def getBorderRouterServiceInfo(self):
        return self._getServiceInfo(MdnsType.BORDER_ROUTER)
