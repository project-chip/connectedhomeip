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

from mobly import asserts
from zeroconf import Zeroconf, ServiceListener
from zeroconf.asyncio import (
    AsyncZeroconf,
    AsyncServiceInfo,
    AsyncZeroconfServiceTypes
)

MDNS_TYPE_OPERATIONAL = "_matter._tcp.local."

'''
A service listener for the TXT record data to get populated
'''
class DummyServiceListener(ServiceListener):

    def add_service(self, zc: Zeroconf, type: str, name: str) -> None:
        pass

    def remove_service(self, zc: Zeroconf, type: str, name: str) -> None:
        pass

    def update_service(self, zc: Zeroconf, type: str, name: str) -> None:
        pass

class MdnsHelper:
    def __init__(self, dut):
        self._listener = DummyServiceListener()
        self._azc: AsyncZeroconf = AsyncZeroconf()
        self._dut = dut
        self._service_types = None
        self._service_info = None
        self._name = f"{self._get_service_name()}.{MDNS_TYPE_OPERATIONAL}"

    def _get_service_name(self):
        node_id = self._dut.dut_node_id
        node_id_hex = str(hex(int(node_id))[2:].upper()).zfill(16)
        compressed_fabricid = self._dut.default_controller.GetCompressedFabricId()
        compressed_fabricid_hex = hex(int(compressed_fabricid))[2:].upper()
        service_name = f"{compressed_fabricid_hex}-{node_id_hex}"
        return service_name

    def _get_txt_record_key_value(self, key: str):
        # Convert the key from string to bytes, as the dictionary uses bytes
        byte_key = key.encode('utf-8')

        # Check if the key exists in the dictionary
        asserts.assert_in(byte_key, self._service_info.properties, f"Property '{key}' not found")
        value = self._service_info.properties[byte_key]

        # Check if the value is not None
        asserts.assert_is_not_none(value, f"Value for property '{key}' not present")

        # Convert the value from bytes to string
        return value.decode('utf-8')

    """
    Asynchronously retrieves the TXT record for a given device.

    This method queries the MDNS service to find the TXT record associated with
    the specified device. It handles the lookup process and extracts specific
    property values based on the provided key.

    Args:
        dut: The device under test. It should contain attributes like `dut_node_id` and 
                `default_controller`.
        key: An optional string specifying which particular property to extract from 
                the TXT record. If None, all properties will be returned.
        refresh: An optional flag that skips returning cached values, it will peform
                the calls again.

    Returns:
        The value of the specified key from the device's TXT record. If the key is None,
        returns all properties. If the service or key is not found, returns None.

    Raises:
        AssertError: If the service information is not found or the MDNS operational 
                        service type is missing.
    """
    async def getTxtRecord(self, key: str = None, refresh = False):

        await self.getOperationalServiceInfo(refresh = refresh)

        if key is None:
            # Return all properties if no key specified
            return self._service_info.properties
        else:
            # Return specified value by key
            return self._get_txt_record_key_value(key)
    
    async def getOperationalServiceInfo(self, refresh = False):

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
        if self._service_info is None or refresh:
            service_info: AsyncServiceInfo = await self._azc.async_get_service_info(
                name=self._name,
                type_=MDNS_TYPE_OPERATIONAL
            )
            asserts.assert_is_not_none(service_info, f"Service info for {self._name} not found")
            self._service_info = service_info

        return self._service_info