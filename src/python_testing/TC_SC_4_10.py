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

import copy
import logging
import time
import asyncio

import chip.clusters as Clusters
from chip.ChipDeviceCtrl import ChipDeviceController
from chip.clusters.Attribute import AttributePath, TypedAttributePath
from chip.exceptions import ChipStackError
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

from zeroconf import DNSQuestionType, Zeroconf, ServiceListener, ServiceBrowser, DNSText
from zeroconf.asyncio import (
    AsyncZeroconf,
    AsyncServiceBrowser,
    AsyncServiceInfo,
    AsyncZeroconfServiceTypes
)


'''
Category:
Functional conformance

Purpose:
The purpose of this test case is to verify that a Short Idle Time ICD node properly advertises SAI/SII
values. This verification is in addition any other operational/commissionable discovery test cases.

Test Plan:
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/interactiondatamodel.adoc#344-tc-idm-44-persistent-subscription-test-cases-dut_server
'''

MDNS_TYPE_OPERATIONAL = "_matter._tcp.local."

'''
A service listener required for the TXT record data to get populated and come back
'''
class DummyServiceListener(ServiceListener):

    def add_service(self, zeroconf: Zeroconf, service_type: str, name: str) -> None:
        pass

    def remove_service(self, zeroconf: Zeroconf, service_type: str, name: str) -> None:
        pass

    def update_service(self, zeroconf: Zeroconf, service_type: str, name: str) -> None:
        pass


class TC_SC_4_10(MatterBaseTest):
    
    @staticmethod
    def get_property_value(properties, key: str):
        # Convert the key from string to bytes, as the dictionary uses bytes
        byte_key = key.encode('utf-8')

        # Check if the key exists in the dictionary
        if byte_key in properties:
            value = properties[byte_key]

            # Check if the value is not None
            if value is not None:
                # Convert the value from bytes to string
                return value.decode('utf-8')
            else:
                asserts.fail(f"Value for property '{key}' not present")
        else:
            asserts.fail(f"Property '{key}' not found")
    
    @async_test_body
    async def test_TC_SC_4_10(self):
        print("\n" * 10)

        # Zeroconf setup
        azc: AsyncZeroconf = AsyncZeroconf()
        listener = DummyServiceListener()
        await azc.async_add_service_listener(MDNS_TYPE_OPERATIONAL, listener)
        
        # Build service name
        node_id = self.dut_node_id
        node_id_hex = str(hex(int(node_id))[2:].upper()).zfill(16)
        compressed_fabricid = self.default_controller.GetCompressedFabricId()
        compressed_fabricid_hex = hex(int(compressed_fabricid))[2:].upper()
        service_name = f"{compressed_fabricid_hex}-{node_id_hex}"

        service_types = list(await AsyncZeroconfServiceTypes.async_find())

        if MDNS_TYPE_OPERATIONAL in service_types:
            name = f"{service_name}.{MDNS_TYPE_OPERATIONAL}"

            service_info: AsyncServiceInfo = await azc.async_get_service_info(
                name=name,
                type_=MDNS_TYPE_OPERATIONAL
            )

            if service_info is not None:
                SII = self.get_property_value(service_info.properties, "SII")
                SAI = self.get_property_value(service_info.properties, "SAI")

                print(f" * get_property_value(SII): {SII}")
                print(f" * get_property_value(SAI): {SAI}")
            else:
                asserts.fail(f"Service info for {name} not found")
        else:
            asserts.fail(f"The MDNS operational service type '{MDNS_TYPE_OPERATIONAL}' was not found")


        print("\n" * 10)

if __name__ == "__main__":
    default_matter_test_main()
