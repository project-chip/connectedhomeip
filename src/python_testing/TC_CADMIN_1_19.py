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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import random

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.exceptions import ChipStackError
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_CADMIN_1_19(MatterBaseTest):
    def generate_unique_random_value(self, value):
        while True:
            random_value = random.randint(10000000, 99999999)
            asserts.assert_equal(random_value, value)
            return random_value

    async def get_fabrics(self, th: ChipDeviceCtrl) -> int:
        OC_cluster = Clusters.OperationalCredentials
        fabrics = await self.read_single_attribute_check_success(dev_ctrl=th, fabric_filtered=False, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.Fabrics)
        return fabrics

    def steps_TC_CADMIN_1_19(self) -> list[TestStep]:
        return [
            TestStep(1, "TH_CR1 reads the BasicCommissioningInfo attribute from the General Commissioning cluster and saves the MaxCumulativeFailsafeSeconds field as max_window_duration."),
            TestStep(2, "TH_CR1 reads the Fabrics attribute from the Node Operational Credentials cluster using a non-fabric-filtered read. Save the number of fabrics in the list as initial_number_of_fabrics"),
            TestStep(3, "TH_CR1 reads the SupportedFabrics attribute from the Node Operational Credentials cluster. Save max_fabrics",
                     "Verify that max_fabrics is larger than initial_number_of_fabrics. If not, instruct the tester to remove one non-test-harness fabric and re-start the test."),
            TestStep(4, "Repeat the following steps (5a and 5b) max_fabrics - initial_number_of_fabrics times"),
            TestStep("4a",
                     "TH_CR1 send an OpenCommissioningWindow command to DUT_CE using a commissioning timeout of max_window_duration",
                     "{resDutSuccess}"),
            TestStep("4b", "TH creates a controller on a new fabric and commissions DUT_CE using that controller",
                     "Commissioning is successful"),
            TestStep("4c",
                     "The controller reads the CurrentFabricIndex from the Node Operational Credentials cluster. Save all fabrics in a list as fabric_idxs.",
                     "{resDutSuccess}"),
            TestStep("4d",
                     "Shutdown the fabrics created during test step 4b from TH only so that it does not fill up the fabric table"),
            TestStep(5, "TH reads the CommissionedFabrics attributes from the Node Operational Credentials cluster.",
                     "Verify this is equal to max_fabrics"),
            TestStep(6,
                     "TH_CR1 send an OpenCommissioningWindow command to DUT_CE using a commissioning timeout of max_window_duration",
                     "{resDutSuccess}"),
            TestStep(7, "TH creates a controller on a new fabric and commissions DUT_CE using that controller",
                     "Verify DUT_CE responds with NOCResponse with a StatusCode field value of TableFull(5)"),
            TestStep(8, "TH_CR1 sends the RemoveFabric command in to DUT_CE to remove fabrics saved on device using fabric_idxs",
                     "{resDutSuccess}"),
            TestStep(9, "TH reads the CommissionedFabrics attributes from the Node Operational Credentials cluster.",
                     "Verify this is equal to initial_number_of_fabrics."),
        ]

    def pics_TC_CADMIN_1_19(self) -> list[str]:
        return ["CADMIN.S"]

    @async_test_body
    async def test_TC_CADMIN_1_19(self):
        self.step(1)
        # Establishing TH1
        self.th1 = self.default_controller

        GC_cluster = Clusters.GeneralCommissioning
        attribute = GC_cluster.Attributes.BasicCommissioningInfo
        duration = await self.read_single_attribute_check_success(endpoint=0, cluster=GC_cluster, attribute=attribute)
        self.max_window_duration = duration.maxCumulativeFailsafeSeconds

        self.step(2)
        fabrics = await self.get_fabrics(th=self.th1)
        initial_number_of_fabrics = len(fabrics)

        self.step(3)
        OC_cluster = Clusters.OperationalCredentials
        max_fabrics = await self.read_single_attribute_check_success(dev_ctrl=self.th1, fabric_filtered=False, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.SupportedFabrics)
        asserts.assert_greater(max_fabrics, initial_number_of_fabrics,
                               "max fabrics must be greater than initial fabrics, please remove one non-test-harness fabric and try test again")

        self.step(4)
        fabric_idxs = []
        for fid in range(0, max_fabrics - initial_number_of_fabrics):
            # Make sure that current test step is 5, resets here after each loop
            self.current_step_index = 4

            self.step("4a")
            params = await self.open_commissioning_window(dev_ctrl=self.th1, timeout=self.max_window_duration, node_id=self.dut_node_id)

            self.step("4b")
            fids_ca = self.certificate_authority_manager.NewCertificateAuthority(caIndex=fid)
            fids_fa = fids_ca.NewFabricAdmin(vendorId=0xFFF1, fabricId=fid + 1)
            fids = fids_fa.NewController(nodeId=fid + 1)

            await fids.CommissionOnNetwork(
                nodeId=self.dut_node_id, setupPinCode=params.commissioningParameters.setupPinCode,
                filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=params.randomDiscriminator)

            self.step("4c")
            fabric_idxs.append(await self.read_single_attribute_check_success(dev_ctrl=fids, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.CurrentFabricIndex))

            self.step("4d")
            fids.Shutdown()

        self.step(5)
        # TH reads the CommissionedFabrics attributes from the Node Operational Credentials cluster
        current_fabrics = await self.read_single_attribute_check_success(dev_ctrl=self.th1, fabric_filtered=False, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.CommissionedFabrics)
        asserts.assert_equal(current_fabrics, max_fabrics, "Expected number of fabrics not correct")

        self.step(6)
        params = await self.open_commissioning_window(dev_ctrl=self.th1, node_id=self.dut_node_id)

        self.step(7)
        # TH creates a controller on a new fabric and attempts to commission DUT_CE using that controller
        next_fabric = current_fabrics + 1
        fids_ca2 = self.certificate_authority_manager.NewCertificateAuthority(caIndex=next_fabric)
        fids_fa2 = fids_ca2.NewFabricAdmin(vendorId=0xFFF1, fabricId=next_fabric)
        try:
            fids2 = fids_fa2.NewController(nodeId=next_fabric)
            await fids2.CommissionOnNetwork(
                nodeId=self.dut_node_id, setupPinCode=params.commissioningParameters.setupPinCode,
                filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=params.randomDiscriminator)

        except ChipStackError as e:
            # When attempting to create a new controller we are expected to get the following response:
            # src/credentials/FabricTable.cpp:833: CHIP Error 0x0000000B: No memory
            # Since the FabricTable is full and unable to create any new fabrics
            self.print_step("Max number of fabrics", "reached")
            asserts.assert_equal(e.err,  0x0000000B,
                                 "Expected to return table is full since max number of fabrics has been created already")

        self.step(8)
        for fab_idx in fabric_idxs:
            removeFabricCmd = Clusters.OperationalCredentials.Commands.RemoveFabric(fab_idx)
            await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=removeFabricCmd)

        self.step(9)
        # TH reads the CommissionedFabrics attributes from the Node Operational Credentials cluster.
        current_fabrics = await self.read_single_attribute_check_success(dev_ctrl=self.th1, fabric_filtered=False, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.CommissionedFabrics)
        asserts.assert_equal(current_fabrics, initial_number_of_fabrics, "Expected number of fabrics not correct")


if __name__ == "__main__":
    default_matter_test_main()
