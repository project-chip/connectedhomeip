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
# test-runner-run/run1/script-args:  --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import random
from time import sleep

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.ChipDeviceCtrl import CommissioningParameters
from chip.exceptions import ChipStackError
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_CADMIN_1_11(MatterBaseTest):
    async def OpenCommissioningWindow(self, th, expectedErrCode) -> CommissioningParameters:
        if expectedErrCode == 0x00:
            params = await th.OpenCommissioningWindow(
                nodeid=self.dut_node_id, timeout=self.timeout, iteration=10000, discriminator=self.discriminator, option=1)
            return params

        elif expectedErrCode == 0x02:
            ctx = asserts.assert_raises(ChipStackError)
            with ctx:
                await th.OpenCommissioningWindow(
                    nodeid=self.dut_node_id, timeout=self.timeout, iteration=10000, discriminator=self.discriminator, option=1)
            errcode = ctx.exception.chip_error
            logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(errcode.is_success, errcode))
            asserts.assert_false(errcode.is_success, 'Commissioning complete did not error as expected')
            asserts.assert_true(errcode.sdk_code == expectedErrCode,
                                'Unexpected error code returned from CommissioningComplete')

    async def OpenBasicCommissioningWindow(self, th, expectedErrCode) -> CommissioningParameters:
        if expectedErrCode == 0x00:
            params = await th.OpenBasicCommissioningWindow(
                nodeid=self.dut_node_id, timeout=self.timeout)
            return params

        elif expectedErrCode == 0x02:
            ctx = asserts.assert_raises(ChipStackError)
            with ctx:
                await th.OpenBasicCommissioningWindow(
                    nodeid=self.dut_node_id, timeout=self.timeout)
            errcode = ctx.exception.chip_error
            logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(errcode.is_success, errcode))
            asserts.assert_false(errcode.is_success, 'Commissioning complete did not error as expected')
            asserts.assert_true(errcode.sdk_code == expectedErrCode,
                                'Unexpected error code returned from CommissioningComplete')

    async def read_currentfabricindex(self, th: ChipDeviceCtrl) -> int:
        cluster = Clusters.Objects.OperationalCredentials
        attribute = Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        current_fabric_index = await self.read_single_attribute_check_success(dev_ctrl=th, endpoint=0, cluster=cluster, attribute=attribute)
        return current_fabric_index

    def steps_TC_CADMIN_1_11(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(
                2, "TH_CR1 gets the MaxCumulativeFailsafeSeconds value from BasicCommissioningInfo attribute in GeneralCommissioning Cluster"),
            TestStep(
                3, "TH_CR1 opens commissioning window on DUT with duration set to value for MaxCumulativeFailsafeSeconds"),
            TestStep(4, "TH_CR2 fully commissions the DUT"),
            TestStep(
                5, "TH_CR1 opens commissioning window on DUT with duration set to value from BasicCommissioningInfo"),
            TestStep(6, "TH_CR1 sends an OpenCommissioningWindow command to the DUT and attempts to open another commissioning window"),
            TestStep(7, "TH_CR2 sends an OpenCommissioningWindow command to the DUT and attempts to open another commissioning window"),
            TestStep(8, "TH_CR1 sends an RevokeCommissioning command to the DUT"),
            TestStep(9, "TH_CR1 reads the FeatureMap from the Administrator Commissioning Cluster to check to see if BC is supported on DUT"),
            TestStep("9a", "TH_CR1 opens commissioning window on DUT with duration set to value from BasicCommissioningInfo"),
            TestStep("9b", "TH_CR1 sends an OpenBasicCommissioningWindow command to the DUT and attempts to open another commissioning window"),
            TestStep("9c", "TH_CR2 sends an OpenBasicCommissioningWindow command to the DUT and attempts to open another commissioning window"),
            TestStep("9d", "TH_CR1 sends a RevokeCommissioning command to the DUT"),
            TestStep(10, "TH_CR2 reads the CurrentFabricIndex attribute from the Operational Credentials cluster and saves as th2_idx"),
            TestStep(11, "TH_CR1 sends the RemoveFabric command to the DUT with the FabricIndex set to th2_idx"),
        ]

    def generate_unique_random_value(self, value):
        while True:
            random_value = random.randint(10000000, 99999999)
            if random_value != value:
                return random_value

    async def CommissionAttempt(
            self, setupPinCode: int):

        logging.info("-----------------Commissioning with TH_CR2-------------------------")
        await self.th2.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)

    def pics_TC_CADMIN_1_11(self) -> list[str]:
        return ["CADMIN.S"]

    @async_test_body
    async def test_TC_CADMIN_1_11(self):
        self.step(1)

        # Establishing TH1 and TH2 controllers
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        self.step(2)
        GC_cluster = Clusters.GeneralCommissioning
        attribute = GC_cluster.Attributes.BasicCommissioningInfo
        duration = await self.read_single_attribute_check_success(endpoint=0, cluster=GC_cluster, attribute=attribute)
        self.timeout = duration.maxCumulativeFailsafeSeconds

        self.step(3)
        self.count = 0
        params = await self.OpenCommissioningWindow(self.th1, 0x00)
        setupPinCode = params.setupPinCode

        self.step(4)
        await self.CommissionAttempt(setupPinCode)

        self.step(5)
        await self.OpenCommissioningWindow(self.th1, 0x00)

        self.step(6)
        await self.OpenCommissioningWindow(self.th1, 0x02)

        self.step(7)
        await self.OpenCommissioningWindow(self.th2, 0x02)

        self.step(8)
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)
        # The failsafe cleanup is scheduled after the command completes, so give it a bit of time to do that
        sleep(1)

        self.step(9)
        AC_cluster = Clusters.AdministratorCommissioning
        features = await self.read_single_attribute(dev_ctrl=self.default_controller, node_id=self.dut_node_id,
                                                    endpoint=0, attribute=AC_cluster.Attributes.FeatureMap)
        self.supports_bc = bool(features & AC_cluster.Bitmaps.Feature.kBasic)

        if self.supports_bc:
            self.count = 0
            self.step("9a")
            await self.OpenBasicCommissioningWindow(self.th1, 0x00)

            self.step("9b")
            await self.OpenBasicCommissioningWindow(self.th1, 0x00)

            self.step("9c")
            await self.OpenBasicCommissioningWindow2(self.th2, 0x02)

            self.step("9d")
            revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
            await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)
            # The failsafe cleanup is scheduled after the command completes, so give it a bit of time to do that
            sleep(1)

        else:
            self.skip_step("9a")
            self.skip_step("9b")
            self.skip_step("9c")
            self.skip_step("9d")

        # Read CurrentFabricIndex attribute from the Operational Credentials cluster
        self.step(10)
        th2_idx = await self.read_currentfabricindex(self.th2)

        self.step(11)
        removeFabricCmd = Clusters.OperationalCredentials.Commands.RemoveFabric(th2_idx)
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=removeFabricCmd)


if __name__ == "__main__":
    default_matter_test_main()
