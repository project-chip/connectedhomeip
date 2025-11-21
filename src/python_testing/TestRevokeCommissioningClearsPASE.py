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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===


from mobly import asserts

import chip.clusters as Clusters
from chip.exceptions import ChipStackError
from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main


class TestRevokeCommissioningClearsPASE(MatterBaseTest):

    @async_test_body
    async def test_TestRevokeCommissioningClearsPASE(self):

        self.print_step("precondition", "Commissioning - already done")

        self.TH1 = self.default_controller

        self.print_step("precondition", "Create Second Controller")

        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        self.TH2_nodeid = self.matter_test_config.controller_node_id + 1
        self.TH2 = fabric_admin.NewController(
            nodeId=self.TH2_nodeid,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
        )

        self.print_step(1, "TH1 sends an OpenCommissioningWindow command to DUT to allow it to be commissioned by TH2")
        resp = await self.open_commissioning_window()

        self.print_step(2, "TH2 establishes a PASE session with DUT")
        pase_node_id = self.dut_node_id + 1
        await self.TH2.FindOrEstablishPASESession(setupCode=resp.commissioningParameters.setupQRCode, nodeid=pase_node_id)

        self.print_step(3, "Read VendorName from BasicInformation Cluster using TH2 over PASE, to ensure PASE session is established")

        VendorNameAttr = Clusters.BasicInformation.Attributes.VendorName
        ROOT_NODE_ENDPOINT_ID = 0

        read_step3 = await self.TH2.ReadAttribute(
            nodeid=pase_node_id,
            attributes=(ROOT_NODE_ENDPOINT_ID, VendorNameAttr),
        )

        asserts.assert_in(
            VendorNameAttr,
            read_step3[ROOT_NODE_ENDPOINT_ID][Clusters.BasicInformation],
            "VendorName should be present in the read response"
        )

        self.print_step(4, "TH1 Sends RevokeCommissioning (over CASE) to clear PASE session on DUT")
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.TH1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)

        self.print_step(
            5, "Ensure that the PASE Session got cleared, by attempting to read VendorName using TH2 (over PASE) and Ensuring that the Command times out")

        _CHIP_TIMEOUT_ERROR = 50

        with asserts.assert_raises(ChipStackError) as e:
            await self.TH2.ReadAttribute(
                nodeid=pase_node_id,
                attributes=(ROOT_NODE_ENDPOINT_ID, VendorNameAttr))
        asserts.assert_equal(e.exception.err, _CHIP_TIMEOUT_ERROR,
                             f"Expected timeout error reading VendorName attribute over PASE, got {e.exception.err}")

        # ---------------------------- Repeat test, but sending RevokeCommissioning over PASE this time --------------------------------

        self.print_step(
            6, "recreate Second Controller; to establish a new PASE session and repeat test, but sending RevokeCommissioning over PASE this time")

        self.TH2.Shutdown()
        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        self.TH2_nodeId = self.matter_test_config.controller_node_id + 1
        self.TH2 = fabric_admin.NewController(
            nodeId=self.TH2_nodeId,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
        )

        self.print_step(7, "TH1 sends an OpenCommissioningWindow command to DUT to allow it to be commissioned by TH2")
        resp = await self.open_commissioning_window()

        self.print_step(8, "TH2 establishes a PASE session with DUT")
        await self.TH2.FindOrEstablishPASESession(setupCode=resp.commissioningParameters.setupQRCode, nodeid=pase_node_id)

        self.print_step(9, "TH2 Sends RevokeCommissioning (Over PASE) to clear PASE session on DUT")

        await self.TH2.SendCommand(nodeid=pase_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)

        self.print_step(
            10, "Ensure that the PASE Session got cleared, by attempting to read VendorName using TH2 (over PASE) and Ensuring that the Command times out")

        with asserts.assert_raises(ChipStackError) as e:
            await self.TH2.ReadAttribute(
                nodeid=pase_node_id,
                attributes=(ROOT_NODE_ENDPOINT_ID, VendorNameAttr))
        asserts.assert_equal(e.exception.err, _CHIP_TIMEOUT_ERROR,
                             f"Expected timeout error reading VendorName attribute over PASE, got {e.exception.err}")


if __name__ == "__main__":
    default_matter_test_main()
