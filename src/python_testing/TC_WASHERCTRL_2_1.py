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
#       --endpoint 1
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts

logger = logging.getLogger(__name__)


MAX_SPIN_SPEEDS = 16


class TC_WASHERCTRL_2_1(MatterBaseTest):

    def desc_TC_WASHERCTRL_2_1(self) -> str:
        """Returns a description of this test"""
        return "[TC-WASHERCTRL-2.1] Optional Spin attributes with DUT as Server"

    def pics_TC_WASHERCTRL_2_1(self) -> list[str]:
        pics = [
            "WASHERCTRL.S.F00",
            "WASHERCTRL.S.A0000",
            "WASHERCTRL.S.A0001"
        ]
        return pics

    def steps_TC_WASHERCTRL_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done",
                     is_commissioning=True),
            TestStep(2, description="TH reads from the DUT the SpinSpeeds attribute",
                     expectation="Verify that the DUT response contains a list of strings. The maximum size of the list is 16."),
            TestStep(3, description="TH reads from the DUT the SpinSpeedCurrent attribute",
                     expectation="Verify that the DUT response contains a uint8 with value between 0 and numSpinSpeeds-1 inclusive."),
            TestStep(4, description="TH writes a supported SpinSpeedCurrent attribute that is a valid index into the list"
                     + "of spin speeds (0 to numSpinSpeeds - 1) and then read the SpinSpeedCurrent value",
                     expectation="Verify DUT responds w/ status SUCCESS(0x00) and the SpinSpeedCurrent value was set accordingly"),
            TestStep(5, description="TH writes an unsupported SpinSpeedCurrent attribute that is other than 0 to DUT",
                     expectation="Verify that the DUT response contains Status CONSTRAINT_ERROR response")
        ]

        return steps

    @run_if_endpoint_matches(has_feature(Clusters.LaundryWasherControls,
                                         Clusters.LaundryWasherControls.Bitmaps.Feature.kSpin))
    async def test_TC_WASHERCTRL_2_1(self):

        endpoint = self.get_endpoint(default=1)

        self.step(1)

        # Read the SpinSpeeds attributes
        self.step(2)
        list_speed_speeds = await self.read_single_attribute_check_success(endpoint=endpoint,
                                                                           cluster=Clusters.Objects.LaundryWasherControls,
                                                                           attribute=Clusters.LaundryWasherControls.Attributes.SpinSpeeds)

        asserts.assert_true(isinstance(list_speed_speeds, list), "Returned value was not a list")
        numSpinSpeeds = len(list_speed_speeds)
        asserts.assert_less_equal(numSpinSpeeds, MAX_SPIN_SPEEDS, "List of SpinSpeeds larger than maximum allowed")

        # Read the SpinSpeedCurrent attribute
        self.step(3)
        spin_speed_current = await self.read_single_attribute_check_success(endpoint=endpoint,
                                                                            cluster=Clusters.Objects.LaundryWasherControls,
                                                                            attribute=Clusters.LaundryWasherControls.Attributes.SpinSpeedCurrent)
        asserts.assert_true(isinstance(spin_speed_current, int), "SpinSpeedCurrent has an invalid value")
        asserts.assert_true(0 <= spin_speed_current <= (numSpinSpeeds - 1), "SpinSpeedCurrent outside valid range")

        self.step(4)
        for requested_speed in range(0, numSpinSpeeds):
            # Write a valid SpinSpeedCurrent value
            result = await self.write_single_attribute(attribute_value=Clusters.LaundryWasherControls.Attributes.SpinSpeedCurrent(requested_speed),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write a valid SpinSpeed value")

            # Read SpinSpeedCurrent value and verify that was changed.
            current_value = await self.read_single_attribute_check_success(endpoint=endpoint,
                                                                           cluster=Clusters.Objects.LaundryWasherControls,
                                                                           attribute=Clusters.LaundryWasherControls.Attributes.SpinSpeedCurrent)
            asserts.assert_equal(current_value, requested_speed, "Value obtained different than the previously written one")

        # Try to write an invalid value (outside supported range)
        self.step(5)
        result = await self.write_single_attribute(attribute_value=Clusters.LaundryWasherControls.Attributes.SpinSpeedCurrent(numSpinSpeeds),
                                                   endpoint_id=endpoint, expect_success=False)
        asserts.assert_equal(result, Status.ConstraintError, "Trying to write an invalid value should return ConstraintError")


if __name__ == "__main__":
    default_matter_test_main()
