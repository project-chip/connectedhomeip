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

from mdns_support import TxtRecordUtil

'''
Category:
Functional conformance

Purpose:
The purpose of this test case is to verify that a Short Idle Time ICD node properly advertises SAI/SII
values. This verification is in addition any other operational/commissionable discovery test cases.

Test Plan:
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/securechannel.adoc#tc-sc-4-10-operational-discovery-sit-icd-node-dut_commissionee
'''

class TC_SC_4_10(MatterBaseTest):
    
    @async_test_body
    async def test_TC_SC_4_10(self):
        print("\n" * 10)
        
        tru = TxtRecordUtil(self)
        SAI = await tru.get(key="SAI", refresh=True)
        SII = await tru.get(key="SII", refresh=True)
        props = await tru.get(refresh=True)

        print(f"SAI: {SAI}")
        print(f"SII: {SII}")
        print(f"props: {props}")

        print("\n" * 10)

if __name__ == "__main__":
    default_matter_test_main()
