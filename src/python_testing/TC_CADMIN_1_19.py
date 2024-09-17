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
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto --PICS src/app/tests/suites/certification/ci-pics-values
# === END CI TEST ARGUMENTS ===

import logging
import random
from time import sleep

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.ChipDeviceCtrl import CommissioningParameters
from chip.exceptions import ChipStackError
from chip.native import PyChipError
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_CADMIN_1_19(MatterBaseTest):
    async def OpenCommissioningWindow(self) -> CommissioningParameters:
        try:
            params = await self.th1.OpenCommissioningWindow(
                nodeid=self.dut_node_id, timeout=self.max_window_duration, iteration=10000, discriminator=self.discriminator, option=1)
            return params

        except Exception as e:
            logging.exception('Error running OpenCommissioningWindow %s', e)
            asserts.assert_true(False, 'Failed to open commissioning window')

    def generate_unique_random_value(self, value):
        while True:
            random_value = random.randint(10000000, 99999999)
            if random_value != value:
                return random_value

    async def CommissionAttempt(
            self, setupPinCode: int, thnum: int, th: str, fail: bool):

        logging.info(f"-----------------Commissioning with TH_CR{str(thnum)}-------------------------")
        if fail == True:
            ctx = asserts.assert_raises(ChipStackError)
            self.print_step(0, ctx)
            with ctx:
                await th.CommissionOnNetwork(
                    nodeId=self.dut_node_id, setupPinCode=setup_code,
                    filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)
                errcode = PyChipError.from_code(ctx.exception.err)
            logging.info('Commissioning complete done. Successful? {}, errorcode = {}, cycle={}'.format(
                errcode.is_success, errcode, (cycle+1)))
            asserts.assert_false(errcode.is_success, 'Commissioning complete did not error as expected')
            asserts.assert_true(errcode.sdk_code == 0x0000000B,
                                'Unexpected error code returned from CommissioningComplete')

        elif fail == False:
            await th.CommissionOnNetwork(
                nodeId=self.dut_node_id, setupPinCode=setupPinCode,
                filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)

    async def get_fabrics(self, th: ChipDeviceCtrl) -> int:
        OC_cluster = Clusters.OperationalCredentials
        fabrics = await self.read_single_attribute_check_success(dev_ctrl=th, fabric_filtered=False, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.Fabrics)
        return fabrics

    def steps_TC_CADMIN_1_19(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(
                2, "TH_CR1 reads the BasicCommissioningInfo attribute from the General Commissioning cluster and saves the MaxCumulativeFailsafeSeconds field as max_window_duration."),
            TestStep(3, "TH_CR1 reads the Fabrics attribute from the Node Operational Credentials cluster using a non-fabric-filtered read. Save the number of fabrics in the list as initial_number_of_fabrics"),
            TestStep(4, "TH_CR1 reads the SupportedFabrics attribute from the Node Operational Credentials cluster. Save max_fabrics"),
            TestStep(5, "Repeat the following steps (5a and 5b) max_fabrics - initial_number_of_fabrics times"),
            TestStep(
                "5a", "TH_CR1 send an OpenCommissioningWindow command to DUT_CE using a commissioning timeout of max_window_duration", "{resDutSuccess}"),
            TestStep("5b", "TH creates a controller on a new fabric and commissions DUT_CE using that controller",
                     "Commissioning is successful"),
            TestStep(6, "TH reads the CommissionedFabrics attributes from the Node Operational Credentials cluster.",
                     "Verify this is equal to max_fabrics"),
            TestStep(
                7, "TH_CR1 send an OpenCommissioningWindow command to DUT_CE using a commissioning timeout of max_window_duration", "{resDutSuccess}"),
            TestStep(8, "TH creates a controller on a new fabric and commissions DUT_CE using that controller",
                     "Verify DUT_CE responds with NOCResponse with a StatusCode field value of TableFull(5)"),
            TestStep(9, "Repeat the following steps (9a and 9b) for each controller (TH_CRn) created by this test"),
            TestStep("9a", "The controller reads the CurrentFabricIndex from the Node Operational Credentials cluster. Save as fabric_index."),
            TestStep("9b", "TH_CR1 sends the RemoveFabric command to DUT_CE", "{resDutSuccess}"),
            TestStep(10, "TH reads the CommissionedFabrics attributes from the Node Operational Credentials cluster.",
                     "Verify this is equal to initial_number_of_fabrics."),
        ]

    def pics_TC_CADMIN_1_19(self) -> list[str]:
        return ["CADMIN.S"]

    @async_test_body
    async def test_TC_CADMIN_1_19(self):
        self.step(1)

        # Establishing TH1 and TH2
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        self.step(2)
        GC_cluster = Clusters.GeneralCommissioning
        attribute = GC_cluster.Attributes.BasicCommissioningInfo
        duration = await self.read_single_attribute_check_success(endpoint=0, cluster=GC_cluster, attribute=attribute)
        self.max_window_duration = duration.maxCumulativeFailsafeSeconds

        self.step(3)
        fabrics = await self.get_fabrics(th=self.th1)
        initial_number_of_fabrics = len(fabrics)

        self.step(4)
        OC_cluster = Clusters.OperationalCredentials
        max_fabrics = await self.read_single_attribute_check_success(dev_ctrl=self.th1, fabric_filtered=False, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.SupportedFabrics)

        self.print_step(0, (max_fabrics - initial_number_of_fabrics))

        self.step(5)
        fids_ca_dir = {}
        fids_fa_dir = {}
        fids = {}
        for fid in range(1, max_fabrics - initial_number_of_fabrics):
            # Make sure that current test step is 5, resets here after each loop
            self.current_step_index = 5

            self.step("5a")
            params = await self.OpenCommissioningWindow()
            setupPinCode = params.setupPinCode

            self.step("5b")
            fids_ca_dir['thca' + str(fid)] = self.certificate_authority_manager.NewCertificateAuthority()
            fids_fa_dir['thfa' + str(fid)] = fids_ca_dir['thca' + str(fid)].NewFabricAdmin(vendorId=0xFFF1, fabricId=fid)
            fids['th' + str(fid)] = fids_fa_dir['thfa' + str(fid)].NewController(nodeId=fid, useTestCommissioner=True)
            await self.CommissionAttempt(setupPinCode, thnum=fid, th=fids['th' + str(fid)], fail=False)

        self.step(6)
        # TH reads the CommissionedFabrics attributes from the Node Operational Credentials cluster
        current_fabrics = await self.read_single_attribute_check_success(dev_ctrl=self.th1, fabric_filtered=False, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.SupportedFabrics)
        if current_fabrics != max_fabrics:
            asserts.fail(f"Expected number of fabrics not correct, instead was {str(current_fabrics)}")

        self.step(7)
        params = await self.OpenCommissioningWindow()
        setupPinCode = params.setupPinCode

        self.step(8)
        # TH creates a controller on a new fabric and commissions DUT_CE using that controller
        fids_ca_dir['thca' + str(current_fabrics + 1)] = self.certificate_authority_manager.NewCertificateAuthority()
        fids_fa_dir['thfa' + str(current_fabrics + 1)] = fids_ca_dir['thca' + str(current_fabrics + 1)
                                                                     ].NewFabricAdmin(vendorId=0xFFF1, fabricId=current_fabrics + 1)
        try:
            fids['th' + str(current_fabrics + 1)] = fids_fa_dir['thfa' + str(current_fabrics + 1)
                                                                ].NewController(nodeId=current_fabrics + 1, useTestCommissioner=True)
            await self.CommissionAttempt(setupPinCode, thnum=fid, th=fids['th' + str(current_fabrics + 1)], fail=True)
            asserts.fail("Expected exception not thrown")

        except ChipStackError as e:
            # When attempting to create a new controller we are expected to get the following response:
            # src/credentials/FabricTable.cpp:833: CHIP Error 0x0000000B: No memory
            # Since the FabricTable is full and unable to create any new fabrics
            asserts.assert_equal(e.err,  0x0000000B,
                                 "Expected to return table is full since max number of fabrics has been created already")

        self.step(9)
        for thc in fids.keys():
            # Make sure that current test step is 11 (9 + 2 since 5a and 5b test steps included in count), resets here after each loop
            self.current_step_index = 11

            self.step("9a")
            fabric_index = await self.read_single_attribute_check_success(dev_ctrl=fids[thc], endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.CurrentFabricIndex)

            self.step("9b")
            removeFabricCmd = Clusters.OperationalCredentials.Commands.RemoveFabric(fabric_index)
            await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=removeFabricCmd)

        self.step(10)
        # TH reads the CommissionedFabrics attributes from the Node Operational Credentials cluster.
        current_fabrics = await self.read_single_attribute_check_success(dev_ctrl=self.th1, fabric_filtered=False, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.CommissionedFabrics)
        if current_fabrics != initial_number_of_fabrics:
            asserts.fail(f"Found more than expected fabrics: {str(current_fabrics)}")


if __name__ == "__main__":
    default_matter_test_main()
