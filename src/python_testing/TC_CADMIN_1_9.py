#
#    Copyright (c) 2022 Project CHIP Authors
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

# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args:  --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto

import logging
import random
from time import sleep

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.ChipDeviceCtrl import CommissioningParameters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_CADMIN_1_9(MatterBaseTest):

    def OpenCommissioningWindow(self) -> CommissioningParameters:
        try:
            params = self.th1.OpenCommissioningWindow(
                nodeid=self.dut_node_id, timeout=900, iteration=10000, discriminator=self.discriminator, option=1)
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
            self, params: dict, expectedErrCode: int):

        if expectedErrCode == 3:
            for cycle in range(20):
                logging.info("-----------------Current Iteration {}-------------------------".format(cycle+1))
                setup_code = self.generate_unique_random_value(params.setupPinCode)
                errcode = self.th2.CommissionOnNetwork(
                    nodeId=self.dut_node_id, setupPinCode=setup_code,
                    filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)
                logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(errcode.is_success, errcode))
                asserts.assert_false(errcode.is_success, 'Commissioning complete did not error as expected')
                asserts.assert_true(errcode.sdk_code == expectedErrCode,
                                    'Unexpected error code returned from CommissioningComplete')
                sleep(1)

        elif expectedErrCode == 50:
            logging.info("-----------------Attempting connection expecting timeout-------------------------")
            errcode = self.th2.CommissionOnNetwork(
                nodeId=self.dut_node_id, setupPinCode=params.setupPinCode,
                filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)
            logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(errcode.is_success, errcode))
            asserts.assert_false(errcode.is_success, 'Commissioning complete did not error as expected')
            asserts.assert_true(errcode.sdk_code == expectedErrCode, 'Unexpected error code returned from CommissioningComplete')

    def TC_CADMIN_1_9(self) -> list[str]:
        return ["CADMIN.S"]

    @async_test_body
    async def test_TC_CADMIN_1_9(self):
        self.print_step(1, "Commissioning, already done")

        # Establishing TH1 and TH2
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        self.print_step(2, "TH1 opens commissioning window on DUT with duration set to 900")
        params = self.OpenCommissioningWindow()

        self.print_step(3, "TH2 attempts to connect 20 times to endpoint with incorrect passcode")
        await self.CommissionAttempt(params, expectedErrCode=0x03)

        self.print_step(4, "TH2 attempts to connect to endpoint with correct passcode")
        await self.CommissionAttempt(params, expectedErrCode=0x32)

        self.print_step(5, "TH1 opening Commissioning Window one more time to validate ability to do so")
        params = self.OpenCommissioningWindow()

        self.print_step(6, "TH1 revoking Commissioning Window")
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)
        # The failsafe cleanup is scheduled after the command completes, so give it a bit of time to do that
        sleep(1)


if __name__ == "__main__":
    default_matter_test_main()
