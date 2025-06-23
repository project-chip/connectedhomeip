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


import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.exceptions import ChipStackError
from chip.testing.event_attribute_reporting import ClusterAttributeChangeAccumulator
from chip.testing.matter_testing import AttributeValue, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from support_modules.cadmin_support import CADMINSupport


class TC_CADMIN_1_19(MatterBaseTest):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.support = CADMINSupport(self)

    def steps_TC_CADMIN_1_19(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH_CR1 reads the BasicCommissioningInfo attribute from the General Commissioning cluster and saves the MaxCumulativeFailsafeSeconds field as max_window_duration."),
            TestStep(3, "TH_CR1 reads the Fabrics attribute from the Node Operational Credentials cluster using a non-fabric-filtered read. Save the number of fabrics in the list as initial_number_of_fabrics"),
            TestStep(4, "TH_CR1 reads the SupportedFabrics attribute from the Node Operational Credentials cluster. Save max_fabrics",
                     "Verify that max_fabrics is larger than initial_number_of_fabrics. If not, instruct the tester to remove one non-test-harness fabric and re-start the test."),
            TestStep(5, "Repeat the following steps (5a and 5b) max_fabrics - initial_number_of_fabrics times"),
            TestStep("5a",
                     "TH_CR1 send an OpenCommissioningWindow command to DUT_CE using a commissioning timeout of max_window_duration",
                     "{resDutSuccess}"),
            TestStep("5b", "TH creates a controller on a new fabric and commissions DUT_CE using that controller",
                     "Commissioning is successful"),
            TestStep("5c",
                     "The controller reads the CurrentFabricIndex from the Node Operational Credentials cluster. Save all fabrics in a list as fabric_idxs.",
                     "{resDutSuccess}"),
            TestStep("5d",
                     "Shutdown the fabrics created during test step 5b from TH only so that it does not fill up the fabric table"),
            TestStep(6, "TH reads the CommissionedFabrics attributes from the Node Operational Credentials cluster.",
                     "Verify this is equal to max_fabrics"),
            TestStep(7,
                     "TH_CR1 send an OpenCommissioningWindow command to DUT_CE using a commissioning timeout of max_window_duration",
                     "{resDutSuccess}"),
            TestStep(8, "TH creates a controller on a new fabric and commissions DUT_CE using that controller",
                     "Verify DUT_CE responds with NOCResponse with a StatusCode field value of TableFull(5)"),
            TestStep(9, "TH_CR1 sends the RemoveFabric command in to DUT_CE to remove fabrics saved on device using fabric_idxs",
                     "{resDutSuccess}"),
            TestStep(10, "TH reads the CommissionedFabrics attributes from the Node Operational Credentials cluster.",
                     "Verify this is equal to initial_number_of_fabrics."),
            TestStep(11, "TH subscribes to the window status attribute", "Success"),
            TestStep(12, "TH sends the RevokeCommissioning command", "Success"),
            TestStep(13, "TH waits to receive an attribute report that indicates the window status is closed", "Report is received"),
        ]

    def pics_TC_CADMIN_1_19(self) -> list[str]:
        return ["CADMIN.S"]

    @async_test_body
    async def test_TC_CADMIN_1_19(self):
        self.step(1)
        # Establishing TH1
        self.th1 = self.default_controller

        self.step(2)
        GC_cluster = Clusters.GeneralCommissioning
        attribute = GC_cluster.Attributes.BasicCommissioningInfo
        duration = await self.read_single_attribute_check_success(endpoint=0, cluster=GC_cluster, attribute=attribute)
        self.max_window_duration = duration.maxCumulativeFailsafeSeconds

        self.step(3)
        fabrics = await self.support.get_fabrics(th=self.th1, fabric_filtered=False)
        initial_number_of_fabrics = len(fabrics)

        self.step(4)
        OC_cluster = Clusters.OperationalCredentials
        max_fabrics = await self.read_single_attribute_check_success(dev_ctrl=self.th1, fabric_filtered=False, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.SupportedFabrics)
        asserts.assert_greater(max_fabrics, initial_number_of_fabrics,
                               "max fabrics must be greater than initial fabrics, please remove one non-test-harness fabric and try test again")

        self.step(5)
        fabric_idxs = []
        for fid in range(0, max_fabrics - initial_number_of_fabrics):
            # Make sure that current test step is 5, resets here after each loop
            self.current_step_index = 5

            self.step("5a")
            params = await self.open_commissioning_window(dev_ctrl=self.th1, timeout=self.max_window_duration, node_id=self.dut_node_id)

            self.step("5b")
            fids_ca = self.certificate_authority_manager.NewCertificateAuthority(caIndex=fid)
            fids_fa = fids_ca.NewFabricAdmin(vendorId=0xFFF1, fabricId=fid + 1)
            fids = fids_fa.NewController(nodeId=fid + 1)

            await fids.CommissionOnNetwork(
                nodeId=self.dut_node_id, setupPinCode=params.commissioningParameters.setupPinCode,
                filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=params.randomDiscriminator)

            self.step("5c")
            fabric_idxs.append(await self.read_single_attribute_check_success(dev_ctrl=fids, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.CurrentFabricIndex))

            self.step("5d")
            fids.Shutdown()

        self.step(6)
        # TH reads the CommissionedFabrics attributes from the Node Operational Credentials cluster
        current_fabrics = await self.read_single_attribute_check_success(dev_ctrl=self.th1, fabric_filtered=False, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.CommissionedFabrics)
        asserts.assert_equal(current_fabrics, max_fabrics, "Expected number of fabrics not correct")

        self.step(7)
        params = await self.open_commissioning_window(dev_ctrl=self.th1, node_id=self.dut_node_id)

        self.step(8)
        # TH creates a controller on a new fabric and attempts to commission DUT_CE using that controller
        next_fabric = current_fabrics + 1
        fids_ca2 = self.certificate_authority_manager.NewCertificateAuthority(caIndex=next_fabric)
        fids_fa2 = fids_ca2.NewFabricAdmin(vendorId=0xFFF1, fabricId=next_fabric)
        with asserts.assert_raises(ChipStackError) as cm:
            fids2 = fids_fa2.NewController(nodeId=next_fabric)
            await fids2.CommissionOnNetwork(
                nodeId=self.dut_node_id, setupPinCode=params.commissioningParameters.setupPinCode,
                filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=params.randomDiscriminator
            )
        # When attempting to create a new controller we are expected to get the following response:
        # src/credentials/FabricTable.cpp:833: CHIP Error 0x0000000B: No memory
        # Since the FabricTable is full and unable to create any new fabrics
        self.print_step("Max number of fabrics", "reached")
        asserts.assert_equal(cm.exception.err,  0x0000000B,
                             "Expected to return table is full since max number of fabrics has been created already")

        self.step(9)
        for fab_idx in fabric_idxs:
            removeFabricCmd = Clusters.OperationalCredentials.Commands.RemoveFabric(fab_idx)
            await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=removeFabricCmd)

        self.step(10)
        # TH reads the CommissionedFabrics attributes from the Node Operational Credentials cluster.
        current_fabrics = await self.read_single_attribute_check_success(dev_ctrl=self.th1, fabric_filtered=False, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.CommissionedFabrics)
        asserts.assert_equal(current_fabrics, initial_number_of_fabrics, "Expected number of fabrics not correct")

        self.step(11)
        attribute_reports = ClusterAttributeChangeAccumulator(
            expected_cluster=Clusters.AdministratorCommissioning, expected_attribute=Clusters.AdministratorCommissioning.Attributes.WindowStatus)
        await attribute_reports.start(dev_ctrl=self.th1, node_id=self.dut_node_id, endpoint=0)

        self.step(12)
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.send_single_cmd(cmd=revokeCmd, dev_ctrl=self.th1, node_id=self.dut_node_id, endpoint=0, timedRequestTimeoutMs=6000)

        self.step(13)
        val = AttributeValue(endpoint_id=0, attribute=Clusters.AdministratorCommissioning.Attributes.WindowStatus,
                             value=Clusters.AdministratorCommissioning.Enums.CommissioningWindowStatusEnum.kWindowNotOpen)
        attribute_reports.await_all_final_values_reported([val], timeout_sec=5)


if __name__ == "__main__":
    default_matter_test_main()
