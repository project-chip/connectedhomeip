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
#     app-args: --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import random
from time import sleep

import chip.clusters as Clusters
from chip.ChipDeviceCtrl import CommissioningParameters
from chip.exceptions import ChipStackError
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_CADMIN_1_22_24(MatterBaseTest):
    async def OpenCommissioningWindow(self) -> CommissioningParameters:
        try:
            params = await self.th1.OpenCommissioningWindow(
                nodeid=self.dut_node_id, timeout=self.max_window_duration, iteration=10000, discriminator=self.discriminator, option=1)
            return params

        except Exception as e:
            logging.exception('Error running OpenCommissioningWindow %s', e)
            asserts.fail('Failed to open commissioning window')

    async def get_window_status(self) -> int:
        AC_cluster = Clusters.AdministratorCommissioning
        window_status = await self.read_single_attribute_check_success(dev_ctrl=self.th1, fabric_filtered=False, endpoint=0, cluster=AC_cluster, attribute=AC_cluster.Attributes.WindowStatus)
        return window_status

    def pics_TC_CADMIN_1_22(self) -> list[str]:
        return ["CADMIN.S"]

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
        asserts.assert_equal(window_status, Clusters.AdministratorCommissioning.Enums.CommissioningWindowStatusEnum.kWindowNotOpen,
                             "Commissioning window is expected to be closed, but was found to be open")

        self.step(5)
        try:
            await self.th1.OpenCommissioningWindow(
                nodeid=self.dut_node_id, timeout=901, iteration=10000, discriminator=self.discriminator, option=1)

        except ChipStackError as e:
            # Since we provided 901 seconds as the timeout duration,
            # we should not be able to open comm window as duration is too long.
            # we are expected receive Failed to open commissioning window: IM Error 0x00000585: General error: 0x85 (INVALID_COMMAND)
            _INVALID_COMMAND = 0x00000585
            asserts.assert_equal(e.err,  _INVALID_COMMAND,
                                 "Expected to error as we provided failure value for opening commissioning window")

        self.step(6)
        window_status2 = await self.get_window_status()
        asserts.assert_equal(window_status2, Clusters.AdministratorCommissioning.Enums.CommissioningWindowStatusEnum.kWindowNotOpen,
                             "Commissioning window is expected to be closed, but was found to be open")

    def pics_TC_CADMIN_1_24(self) -> list[str]:
        return ["CADMIN.S"]

    def steps_TC_CADMIN_1_24(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH_CR1 opens a commissioning window on DUT_CE using ECM with a value of 180 seconds",
                     "DUT_CE opens its Commissioning window to allow a second commissioning"),
            TestStep(3, "Wait for the commissioning window in step 2 to timeout"),
            TestStep(4, "TH_CR1 reads the window status to verify the DUT_CE window is closed",
                     "DUT_CE windows status shows the window is closed"),
            TestStep(5, "TH_CR1 opens a commissioning window on DUT_CE using ECM with a value of 179 seconds",
                     "DUT_CE does not open its Commissioning window to allow a second commissioning. DUT_CE shows 'Failed to open commissioning window. Global status 0x85'"),
            TestStep(6, "TH_CR1 reads the window status to verify the DUT_CE window is closed",
                     "DUT_CE windows status shows the window is closed"),
        ]

    @async_test_body
    async def test_TC_CADMIN_1_24(self):
        self.step(1)
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)

        self.step(2)
        await self.th1.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=180, iteration=10000, discriminator=self.discriminator, option=1)

        self.step(3)
        sleep(180)

        self.step(4)
        # TODO: Issue noticed when initially attempting to check window status after waiting for timeout to occur, issue is detailed here: https://github.com/project-chip/connectedhomeip/issues/35983
        # Workaround in place below until above issue resolved
        try:
            window_status = await self.get_window_status()
        except asyncio.CancelledError:
            window_status = await self.get_window_status()

        asserts.assert_equal(window_status, Clusters.AdministratorCommissioning.Enums.CommissioningWindowStatusEnum.kWindowNotOpen,
                             "Commissioning window is expected to be closed, but was found to be open")

        self.step(5)
        try:
            await self.th1.OpenCommissioningWindow(
                nodeid=self.dut_node_id, timeout=179, iteration=10000, discriminator=self.discriminator, option=1)

        except ChipStackError as e:
            # Since we provided 179 seconds as the timeout duration,
            # we should not be able to open comm window as duration is too long.
            # we are expected receive Failed to open commissioning window: IM Error 0x00000585: General error: 0x85 (INVALID_COMMAND)
            _INVALID_COMMAND = 0x00000585
            asserts.assert_equal(e.err, _INVALID_COMMAND,
                                 "Expected to error as we provided failure value for opening commissioning window")

        self.step(6)
        window_status2 = await self.get_window_status()
        asserts.assert_equal(window_status2, Clusters.AdministratorCommissioning.Enums.CommissioningWindowStatusEnum.kWindowNotOpen,
                             "Commissioning window is expected to be closed, but was found to be open")


if __name__ == "__main__":
    default_matter_test_main()
