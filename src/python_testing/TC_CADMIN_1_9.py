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


class TC_CADMIN_1_9(MatterBaseTest):
    async def OpenCommissioningWindow(self) -> CommissioningParameters:
        try:
            cluster = Clusters.GeneralCommissioning
            attribute = cluster.Attributes.BasicCommissioningInfo
            duration = await self.read_single_attribute_check_success(endpoint=0, cluster=cluster, attribute=attribute)
            params = await self.th1.OpenCommissioningWindow(
                nodeid=self.dut_node_id, timeout=duration.maxCumulativeFailsafeSeconds, iteration=10000, discriminator=self.discriminator, option=1)
            return params

        except Exception as e:
            logging.exception('Error running OpenCommissioningWindow %s', e)
            asserts.assert_true(False, 'Failed to open commissioning window')

    def steps_TC_CADMIN_1_9(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(
                2, "TH1 opens commissioning window on DUT with duration set to value for maxCumulativeFailsafeSeconds"),
            TestStep(3, "TH2 attempts to connect 20 times to endpoint with incorrect passcode"),
            TestStep(4, "TH2 attempts to connect to endpoint with correct passcode"),
            TestStep(5, "TH1 opening Commissioning Window one more time to validate ability to do so"),
            TestStep(6, "TH1 revoking Commissioning Window"),
        ]

    def generate_unique_random_value(self, value):
        while True:
            random_value = random.randint(10000000, 99999999)
            if random_value != value:
                return random_value

    async def CommissionOnNetwork(
        self, setup_code: int
    ):
        ctx = asserts.assert_raises(ChipStackError)
        with ctx:
            await self.th2.CommissionOnNetwork(
                nodeId=self.dut_node_id, setupPinCode=setup_code,
                filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)
        errcode = PyChipError.from_code(ctx.exception.err)
        return errcode

    async def CommissionAttempt(
            self, setupPinCode: int, expectedErrCode: int):

        if expectedErrCode == 3:
            for cycle in range(20):
                logging.info("-----------------Current Iteration {}-------------------------".format(cycle+1))
                setup_code = self.generate_unique_random_value(setupPinCode)
                errcode = await self.CommissionOnNetwork(setup_code)
                logging.info('Commissioning complete done. Successful? {}, errorcode = {}, cycle={}'.format(
                    errcode.is_success, errcode, (cycle+1)))
                asserts.assert_false(errcode.is_success, 'Commissioning complete did not error as expected')
                asserts.assert_true(errcode.sdk_code == expectedErrCode,
                                    'Unexpected error code returned from CommissioningComplete')

        elif expectedErrCode == 50:
            logging.info("-----------------Attempting connection expecting timeout-------------------------")
            errcode = await self.CommissionOnNetwork(setupPinCode)
            logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(errcode.is_success, errcode))
            asserts.assert_false(errcode.is_success, 'Commissioning complete did not error as expected')
            asserts.assert_true(errcode.sdk_code == expectedErrCode, 'Unexpected error code returned from CommissioningComplete')

    def pics_TC_CADMIN_1_9(self) -> list[str]:
        return ["CADMIN.S"]

    @async_test_body
    async def test_TC_CADMIN_1_9(self):
        self.step(1)

        # Establishing TH1 and TH2
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        self.step(2)
        params = await self.OpenCommissioningWindow()
        setupPinCode = params.setupPinCode

        self.step(3)
        await self.CommissionAttempt(setupPinCode, expectedErrCode=0x03)
        # TODO: Found if we don't add sleep time after test completes that we get unexpected error code and response after the 21st iteration.
        # Link to Bug Filed: https://github.com/project-chip/connectedhomeip/issues/34383
        sleep(1)

        self.step(4)
        await self.CommissionAttempt(setupPinCode, expectedErrCode=0x32)

        self.step(5)
        params = await self.OpenCommissioningWindow()

        self.step(6)
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)
        # The failsafe cleanup is scheduled after the command completes, so give it a bit of time to do that
        sleep(1)


if __name__ == "__main__":
    default_matter_test_main()
