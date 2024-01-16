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

from zeroconf import Zeroconf, ServiceListener, ServiceBrowser, DNSText
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

class TC_SC_4_10(MatterBaseTest):
    
    @async_test_body
    async def test_TC_SC_4_10(self):
        print("\n" * 10)
        azc = AsyncZeroconf()
        node_id = self.dut_node_id
        node_id_hex = str(hex(int(node_id))[2:].upper()).zfill(16)
        compressed_fabricid = self.default_controller.GetCompressedFabricId()
        compressed_fabricid_hex = hex(int(compressed_fabricid))[2:].upper()
        service_name = f"{compressed_fabricid_hex}-{node_id_hex}"

        try:
            service_types = list(await AsyncZeroconfServiceTypes.async_find())

            for service_type in service_types:
                if service_type == MDNS_TYPE_OPERATIONAL:
                    name = f"{service_name}.{service_type}"

                    service_info: AsyncServiceInfo = await azc.async_get_service_info(
                        name=name,
                        type_=service_type
                    )

                    if service_info is not None:
                        for attribute_name in ['type', 'name', 'addresses',
                                               'port', 'weight', 'priority',
                                               'server', 'properties',
                                               'interface_index']:
                            attribute_value = getattr(service_info, attribute_name, None)
                            print(f"{attribute_name}: {attribute_value}")
                    else:
                        print(f"Service info for {name} not found")
        except Exception as e:
            print(("\nError: " + str(e)) * 10)











        print("\n" * 10)

if __name__ == "__main__":
    default_matter_test_main()
