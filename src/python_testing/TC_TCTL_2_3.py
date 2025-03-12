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
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
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
            TestStep(2, "TH reads from the DUT the SelectedTemperatureLevel attribute",
                     "Verify that the DUT response contains the value of _SelectedTemperatureLevel_ with a range of 0 to 31"),
            TestStep(3, "TH reads from the DUT the SupportedTemperatureLevels attribute and verifies string lengths",
                     ("Verify that the DUT response contains a _SupportedTemperatureLevels_ list\n\n"
                      "* List length has to be equal or less than 32 \n"
                      "* Each temperature level should be a string\n"
                      "* Length of each temperature level string has to be equal or less than 16\n")),
        ]
        return steps

    @run_if_endpoint_matches(has_feature(Clusters.TemperatureControl, Clusters.TemperatureControl.Bitmaps.Feature.kTemperatureLevel))
    async def test_TC_TCTL_2_3(self):
        self.endpoint = self.get_endpoint()
        cluster = Clusters.TemperatureControl
        attributes = cluster.Attributes
        self.step(1)

        # Step 2: Read SelectedTemperatureLevel attribute
        self.step(2)
        selected_temp = await self.read_single_attribute_check_success(
            cluster=cluster,
            attribute=attributes.SelectedTemperatureLevel
        )
        asserts.assert_true(0 <= selected_temp <= 31,
                            f"SelectedTemperatureLevel {selected_temp} is out of range [0-31]")

        # Step 3: Read SupportedTemperatureLevels attribute
        self.step(3)
        supported_temps = await self.read_single_attribute_check_success(
            cluster=cluster,
            attribute=attributes.SupportedTemperatureLevels
        )
        asserts.assert_true(isinstance(supported_temps, list),
                            "SupportedTemperatureLevels should be a list")
        asserts.assert_true(len(supported_temps) <= 32,
                            f"SupportedTemperatureLevels list length {len(supported_temps)} exceeds maximum of 32")

        # Verify string lengths
        for level in supported_temps:
            asserts.assert_true(isinstance(level, str),
                                f"Temperature level {level} is not a string")
            asserts.assert_true(len(level) <= 16,
                                f"Temperature level string '{level}' exceeds maximum length of 16")


if __name__ == "__main__":
    default_matter_test_main()
