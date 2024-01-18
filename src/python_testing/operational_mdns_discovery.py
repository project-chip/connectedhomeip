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

import time
from typing import Dict, Union

from mobly import asserts
from zeroconf import ServiceListener, Zeroconf
from zeroconf.asyncio import AsyncServiceInfo, AsyncZeroconf, AsyncZeroconfServiceTypes

MDNS_TYPE_OPERATIONAL = "_matter._tcp.local."


class EmptyServiceListener(ServiceListener):
    '''
    A service listener for the TXT record data to get populated
    '''

    def add_service(self, zc: Zeroconf, type: str, name: str) -> None:
        time.sleep(1)
        pass

    def remove_service(self, zc: Zeroconf, type: str, name: str) -> None:
        time.sleep(1)
        pass

    def update_service(self, zc: Zeroconf, type: str, name: str) -> None:
        time.sleep(1)
        pass


class OperationalMdnsDiscovery:
    """
    A helper class for managing operational mDNS tasks within test cases.

    This class provides utilities for mDNS-related operations needed in tests,
    such as discovering services, registering listeners, and querying service information.
    It's designed to work closely with a specific test case, using its context
    or functionalities for network service interactions.

    Attributes:
        _listener (EmptyServiceListener): A listener for mDNS service events.
        _azc (AsyncZeroconf): An instance of AsyncZeroconf for asynchronous network discovery.
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
        self._listener = EmptyServiceListener()
        self._azc: AsyncZeroconf = AsyncZeroconf()
        self._tc = tc
        self._service_types = None
        self._service_info = None
        self._name = f"{self._get_service_name()}.{MDNS_TYPE_OPERATIONAL}"

    def _get_service_name(self):
        node_id = self._tc.dut_node_id
        node_id_hex = str(hex(int(node_id))[2:].upper()).zfill(16)
        compressed_fabricid = self._tc.default_controller.GetCompressedFabricId()
        compressed_fabricid_hex = hex(int(compressed_fabricid))[2:].upper()
        service_name = f"{compressed_fabricid_hex}-{node_id_hex}"
        return service_name

    def _get_txt_record_key_value(self, key: str):
        # Convert the key from string to bytes, as the dictionary uses bytes
        byte_key = key.encode('utf-8')

        # Check if the key exists in the dictionary
        asserts.assert_in(byte_key, self._service_info.properties, f"Property '{key}' not found")
        value = self._service_info.properties[byte_key]

        # Convert the value from bytes to string
        return None if value is None else value.decode('utf-8')

    async def getTxtRecord(self, key: str = None, refresh=False) -> Union[Dict[str, str], str, None]:
        """
        Asynchronously retrieves the TXT record for a given device.

        This method queries the MDNS service to find the TXT record associated with
        the specified device. It handles the lookup process and extracts specific
        property values based on the provided key.

        Args:
            key: An optional string specifying which particular property to extract from
                    the TXT record. If None, all properties will be returned.
            refresh (bool): If set to True, the method ignores any cached data and performs
                            a fresh lookup. If False, it uses cached data if available.
                            Defaults to False.

        Returns:
            The value of the specified key from the device's TXT record. If the key is None,
            returns all properties. If the service or key is not found, returns None.

        Raises:
            AssertError: If the service information is not found or the MDNS operational
                            service type is missing.
        """

        await self.getOperationalServiceInfo(refresh=refresh)

        if key is None:
            # Return all properties if no key specified
            return self._service_info.properties
        else:
            # Return specified value by key
            return self._get_txt_record_key_value(key)

    async def getOperationalServiceInfo(self, refresh=False):
        """
        Asynchronously retrieves the operational service information for the current device.

        This method performs an MDNS lookup to fetch the operational service information,
        which typically includes the service's TXT record containing various properties.
        The method can be configured to either use cached service information or refresh it.

        The process involves fetching available service types, and then specifically
        querying for the operational service type defined by 'MDNS_TYPE_OPERATIONAL'.
        The result includes details such as service name, port, addresses, and TXT record
        properties.

        Args:
            refresh (bool): If set to True, the method ignores any cached data and performs
                            a fresh lookup. If False, it uses cached data if available.
                            Defaults to False.

        Returns:
            AsyncServiceInfo: An object containing details about the operational service.
                              This includes TXT record properties and other service-related
                              information. If the service or required type is not found,
                              the method asserts a failure.

        Raises:
            AssertError: If no running services are found, or the specific MDNS operational
                         service type is not available. Also asserts if the service info
                         for the given name and type is not found.
        """

        # Setup service listener
        await self._azc.async_remove_all_service_listeners()
        await self._azc.async_add_service_listener(MDNS_TYPE_OPERATIONAL, self._listener)

        # Fetch service types and cache them
        if self._service_types is None or refresh:
            service_types = list(await AsyncZeroconfServiceTypes.async_find())
            asserts.assert_greater(len(service_types), 0, "No running services found")
            self._service_types = service_types

        asserts.assert_in(MDNS_TYPE_OPERATIONAL, self._service_types,
                          f"The MDNS operational service type '{MDNS_TYPE_OPERATIONAL}' was not found")

        # Fetch service info and properties (TXT) and cache them
        service_info = None
        if self._service_info is None or refresh:
            service_info: AsyncServiceInfo = await self._azc.async_get_service_info(
                name=self._name,
                type_=MDNS_TYPE_OPERATIONAL
            )
            self._service_info = service_info

        return None if service_info is None else self._service_info
