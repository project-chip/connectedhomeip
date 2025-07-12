#
#    Copyright (c) 2025 Project CHIP Authors
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${LIT_ICD_APP}
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

from asyncio import sleep
import ipaddress
import logging
import re
from typing import Any

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mdns_discovery.mdns_discovery import MdnsDiscovery, MdnsServiceType
from mobly import asserts

'''
Purpose
The purpose of this test case is to verify that a device is able to
correctly advertise Commissionable Node Discovery service.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/securechannel.adoc#341-tc-sc-41-commissionable-node-discovery-dut_commissionee
'''

PICS_MCORE_ROLE_COMMISSIONEE = "MCORE.ROLE.COMMISSIONEE"

class TC_SC_4_1(MatterBaseTest):

    def steps_TC_SC_4_1(self):
        return [TestStep(1, "Action", "Expectation", is_commissioning=True),
                ]

    async def read_attribute(self, attribute: Any) -> Any:
        cluster = Clusters.Objects.IcdManagement
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    @staticmethod
    def is_valid_dns_sd_instance_name(name: str) -> bool:
        """
        Validates that the DNS-SD instance name is a 64-bit randomly selected ID,
        expressed as a 16-character hexadecimal string using capital letters.

        Args:
            name (str): The DNS-SD instance name to validate.

        Returns:
            bool: True if valid, False otherwise.
        """
        return bool(re.fullmatch(r'[A-F0-9]{16}', name))

    def desc_TC_TC_SC_4_1(self) -> str:
        return "[TC-SC-4.1] Commissionable Node Discovery with DUT as Commissionee"

    def pics_TC_SC_4_1(self) -> list[str]:
        return [PICS_MCORE_ROLE_COMMISSIONEE]

    @async_test_body
    async def test_TC_SC_4_1(self):
        cluster = Clusters.Objects.IcdManagement
        attr = cluster.Attributes
        long_discriminator = 3840
        short_discriminator = 15
        self.endpoint = self.get_endpoint(default=1)
        self.endpoint = 1
        active_mode_threshold = None

        # *** STEP 1 ***
        # Action.
        self.step(1)

        params = await self.default_controller.OpenCommissioningWindow(
            nodeid=self.dut_node_id,
            timeout=600,
            iteration=10000,
            discriminator=long_discriminator,
            option=1
        )

        active_mode_threshold_present = self.check_pics("ICDM.S.A0002")
        logging.info(f"\n\n\t\t\t active_mode_threshold_present: {active_mode_threshold_present}\n\n\n")

        if active_mode_threshold_present:
            active_mode_threshold = await self.read_attribute(attr.ActiveModeThreshold)
            logging.info(f"\n\n\t\t\t active_mode_threshold: {active_mode_threshold}\n\n\n")

        # ######################################################################################################
        mdns = MdnsDiscovery()

        # Get DUT's commissionable service
        commissionable_service = await mdns.get_commissionable_service(discovery_timeout_sec=3, log_output=True)
        
        # Verify presence of DUT's comissionable service
        asserts.assert_is_not_none(commissionable_service, "DUT's commissionable service not present")

        # Verify DUT's commissionable service is a valid DNS-SD instance name
        # (64-bit randomly selected ID expressed as a sixteen-char hex string with capital letters)
        asserts.assert_true(self.is_valid_dns_sd_instance_name(commissionable_service.instance_name),
                            f"Invalid DNS-SD instance name: {commissionable_service.instance_name}")
        
        # Verify DUT's commissionable service service type is '_matterc._udp' and service domain '.local.'
        asserts.assert_equal(commissionable_service.service_type, MdnsServiceType.COMMISSIONABLE.value,
                             f"Invalid service type '{commissionable_service.service_type}', must be '{MdnsServiceType.COMMISSIONABLE.value}'")
        
        
        
        
        
        
        
        
        
        # devices = await self.default_controller.DiscoverCommissionableNodes(
        #     filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=long_discriminator, stopOnFirst=False)
        # logging.info(f"\n\n\n\n\n\n\n\n\n\n\n\n\t\t\t\t commissionable devices: {devices}\n\n\n\n\n\n\n")

        # await self.default_controller.CommissionOnNetwork(
        #     nodeId=self.dut_node_id,
        #     setupPinCode=params.setupPinCode,
        #     filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
        #     filter=long_discriminator)


if __name__ == "__main__":
    default_matter_test_main()
