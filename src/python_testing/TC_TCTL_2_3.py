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
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_TCTL_2_3(MatterBaseTest):
    def desc_TC_TCTL_2_3(self) -> str:
        return "[TC-TCTL-2.3] Optional temperature level attributes with DUT as Server"

    def pics_TC_TCTL_2_3(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "TCTL.S",      # Temperature Control as a Server
            "TCTL.S.F01",  # Does a device support temperature level feature
        ]
        return pics

    def steps_TC_TCTL_2_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the SelectedTemperatureLevel attribute"),
            TestStep(3, "TH reads from the DUT the SupportedTemperatureLevels attribute and verifies string lengths"),
        ]
        return steps

    @async_test_body
    async def test_TC_TCTL_2_3(self):
        self.step(1)

        # Step 2: Read SelectedTemperatureLevel attribute
        self.step(2)
        if self.check_pics("TCTL.S.A0004"):  # SelectedTemperatureLevel attribute
            selected_temp = await self.default_controller.ReadAttribute(
                nodeid=self.dut_node_id,
                attributes=[(1, Clusters.TemperatureControl.Attributes.SelectedTemperatureLevel)]
            )
        temp_level = selected_temp[1][Clusters.TemperatureControl][Clusters.TemperatureControl.Attributes.SelectedTemperatureLevel]
        asserts.assert_true(0 <= temp_level <= 31,
                            f"SelectedTemperatureLevel {temp_level} is out of range [0-31]")

        # Step 3: Read SupportedTemperatureLevels attribute
        self.step(3)
        if self.check_pics("TCTL.S.A0004"):  # SupportedTemperatureLevels attribute
            supported_temps = await self.default_controller.ReadAttribute(
                nodeid=self.dut_node_id,
                attributes=[(1, Clusters.TemperatureControl.Attributes.SupportedTemperatureLevels)]
            )

        temp_levels = supported_temps[1][Clusters.TemperatureControl][Clusters.TemperatureControl.Attributes.SupportedTemperatureLevels]
        asserts.assert_true(isinstance(temp_levels, list),
                            "SupportedTemperatureLevels should be a list")
        asserts.assert_true(len(temp_levels) <= 32,
                            f"SupportedTemperatureLevels list length {len(temp_levels)} exceeds maximum of 32")

        # Verify string lengths
        for level in temp_levels:
            asserts.assert_true(isinstance(level, str),
                                f"Temperature level {level} is not a string")
            asserts.assert_true(len(level) <= 16,
                                f"Temperature level string '{level}' exceeds maximum length of 16")


if __name__ == "__main__":
    default_matter_test_main()
