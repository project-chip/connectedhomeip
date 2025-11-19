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
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
from enum import Enum
from typing import Optional

from mobly import asserts

import matter.clusters as Clusters
from matter.testing import matter_asserts
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, has_attribute

logger = logging.getLogger(__name__)


class TC_DA_1_1(MatterBaseTest):

    def desc_TC_DA_1_1(self) -> str:
        return "The NOC SHALL be wiped on Factory Reset [DUT - Commissionee]"

    def pics_TC_DA_1_1(self):
        """Return PICS definitions asscociated with this test."""
        pics = [
            "MCORE.ROLE.COMMISSIONEE"
        ]
        return pics

    def steps_TC_DA_1_1(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("precondition", "DUT Commissioned to TH1's fabric", is_commissioning=True),
        ]

        return steps

    @async_test_body
    async def test_TC_DA_1_1(self):

        self.step("precondition")
        self.th1 = self.default_controller
        th1_fabric_id = self.th1.fabricId
        opcreds = Clusters.OperationalCredentials
        
        TH1_nodeid = self.matter_test_config.controller_node_id
        logging.info(f"\n\n\n\n\n\t\t\t TH1_nodeid: {TH1_nodeid}\n\n\n\n\n\n")
        
        
        
        # new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        # new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.matter_test_config.fabric_id + 1)

        # TH2_nodeid = self.matter_test_config.controller_node_id + 2

        # self.th2 = new_fabric_admin.NewController(nodeId=TH2_nodeid,
        #                                           paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))




        noc_th1 = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1,
            # node_id=TH1_nodeid,
            # endpoint=0,
            cluster=opcreds,
            attribute=opcreds.Attributes.NOCs,
            fabric_filtered=False)

        # logging.info(f"\n\n\n\n\n\t\t\t noc_th1: {noc_th1}, len: {len(noc_th1)}, noc: {noc_th1[0]}\n\n\n\n\n\n")
        
        
        
        asserts.assert_true(len(noc_th1) == 1, "NOCs attribute must contain single entry in the list")
        
        fabrics_th1 = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1,
            cluster=opcreds,
            attribute=opcreds.Attributes.Fabrics,
            fabric_filtered=False)

        asserts.assert_true(len(fabrics_th1) == 1, "Fabrics attribute must contain single entry in the list")
        asserts.assert_equal(fabrics_th1[0].fabricID, th1_fabric_id, "TH1 FabricID and Fabrics attribute FabricID must match")
        
        logging.info(f"\n\n\n\n\n\t\t\t fabrics_th1: {fabrics_th1}, len: {len(fabrics_th1)}, noc: {fabrics_th1[0]}\n\n\n\n\n\n")
        


if __name__ == "__main__":
    default_matter_test_main()
