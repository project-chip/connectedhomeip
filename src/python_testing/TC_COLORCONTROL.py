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

# Example, Run in Python venv with all-clusters-app -

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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main


class TC_COLORCONTROL(MatterBaseTest):
    """Tests for colorcontrol cluster app server code."""

    _COLORCONTROL_ENDPOINT = 1

    async def _read_remaining_time(self):
        return await self.read_single_attribute_check_success(
            endpoint=self._COLORCONTROL_ENDPOINT, cluster=Clusters.Objects.ColorControl, attribute=Clusters.Objects.ColorControl.Attributes.RemainingTime)

    def desc_TC_COLORCONTROL(self) -> str:
        return "[TC_COLORCONTROL] chef colortemperaturelight functionality test."

    def steps_TC_COLORCONTROL(self):
        return [TestStep(1, "[TC_COLORCONTROL] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[TC_COLORCONTROL] Test move-color-temperature command.")]

    @async_test_body
    async def test_TC_COLORCONTROL(self):
        """Run all steps."""

        self.step(1)
        # Commissioning already done.

        self.step(2)
        asserts.assert_equal(await self._read_remaining_time(), 0)  # Check no transition in progress.
        await self.send_single_cmd(
            cmd=Clusters.Objects.ColorControl.Commands.MoveColorTemperature(
                moveMode=Clusters.Objects.ColorControl.Enums.MoveModeEnum.kUp,
                rate=1,  # 1 step per second
                colorTemperatureMinimumMireds=0,  # default to ColorTempPhysicalMinMireds
                colorTemperatureMaximumMireds=0,  # default to ColorTempPhysicalMaxMireds
                optionsMask=Clusters.Objects.ColorControl.Bitmaps.OptionsBitmap.kExecuteIfOff,
                optionsOverride=Clusters.Objects.ColorControl.Bitmaps.OptionsBitmap.kExecuteIfOff,
            ),
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._COLORCONTROL_ENDPOINT,
        )  # Start transition
        asserts.assert_greater(await self._read_remaining_time(), 0)  # Check transition started
        await self.send_single_cmd(
            cmd=Clusters.Objects.ColorControl.Commands.StopMoveStep(
                optionsMask=Clusters.Objects.ColorControl.Bitmaps.OptionsBitmap.kExecuteIfOff,
                optionsOverride=Clusters.Objects.ColorControl.Bitmaps.OptionsBitmap.kExecuteIfOff,
            ),
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._COLORCONTROL_ENDPOINT,
        )  # Stop transition
        asserts.assert_equal(await self._read_remaining_time(), 0)  # Check transition stopped


if __name__ == "__main__":
    default_matter_test_main()
