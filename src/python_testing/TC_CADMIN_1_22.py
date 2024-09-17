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


class TC_CADMIN_1_22(MatterBaseTest):
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
        if fail:
            ctx = asserts.assert_raises(ChipStackError)
            self.print_step(0, ctx)
            with ctx:
                await th.CommissionOnNetwork(
                    nodeId=self.dut_node_id, setupPinCode=setupPinCode,
                    filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)
                errcode = PyChipError.from_code(ctx.exception.err)
            logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(
                errcode.is_success, errcode))
            asserts.assert_false(errcode.is_success, 'Commissioning complete did not error as expected')
            asserts.assert_true(errcode.sdk_code == 0x0000000B,
                                'Unexpected error code returned from CommissioningComplete')

        elif not fail:
            await th.CommissionOnNetwork(
                nodeId=self.dut_node_id, setupPinCode=setupPinCode,
                filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)

    async def get_window_status(self) -> int:
        AC_cluster = Clusters.AdministratorCommissioning
        window_status = await self.read_single_attribute_check_success(dev_ctrl=self.th1, fabric_filtered=False, endpoint=0, cluster=AC_cluster, attribute=AC_cluster.Attributes.WindowStatus)
        return window_status

    def steps_TC_CADMIN_1_22(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH_CR1 opens a commissioning window on DUT_CE using ECM with a value of 900 seconds",
                     "DUT_CE opens its Commissioning window to allow a second commissioning"),
            TestStep(3, "TH_CR1 sends an RevokeCommissioning command to the DUT"),
            TestStep(4, "TH_CR1 reads the window status to verify the DUT_CE window is closed",
                     "DUT_CE windows status shows the window is closed"),
            TestStep(5, "TH_CR1 opens a commissioning window on DUT_CE using ECM with a value of 901 seconds",
                     "DUT_CE does not open its Commissioning window to allow a second commissioning. DUT_CE shows 'Failed to open commissioning window. Global status 0x85'"),
            TestStep(6, "TH_CR1 reads the window status to verify the DUT_CE window is closed",
                     "DUT_CE windows status shows the window is closed"),
        ]

    def pics_TC_CADMIN_1_22(self) -> list[str]:
        return ["CADMIN.S"]

    @async_test_body
    async def test_TC_CADMIN_1_22(self):
        self.step(1)
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)

        self.step(2)
        await self.th1.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=900, iteration=10000, discriminator=self.discriminator, option=1)

        self.step(3)
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)
        # The failsafe cleanup is scheduled after the command completes, so give it a bit of time to do that
        sleep(1)

        self.step(4)
        window_status = await self.get_window_status()
        if window_status != 0:
            asserts.fail("Commissioning window is expected to be closed, but was found to be open")

        self.step(5)
        try:
            await self.th1.OpenCommissioningWindow(
                nodeid=self.dut_node_id, timeout=901, iteration=10000, discriminator=self.discriminator, option=1)

        except ChipStackError as e:
            # Since we provided 901 seconds as the timeout duration,
            # we should not be able to open comm window as duration is too long.
            asserts.assert_equal(e.err,  0x00000585,
                                 "Expected to error as we provided failure value for opening commissioning window")

        self.step(6)
        window_status2 = await self.get_window_status()
        if window_status2 != 0:
            asserts.fail("Commissioning window is expected to be closed, but was found to be open")


if __name__ == "__main__":
    default_matter_test_main()
