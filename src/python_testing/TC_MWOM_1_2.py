#
#    Copyright (c) 2023 Project CHIP Authors
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
# test-runner-runs: run1
# test-runner-run/run1/app: ${CHIP_MICROWAVE_OVEN_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_MWOM_1_2(MatterBaseTest):

    async def read_mod_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.MicrowaveOvenMode
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_MWOM_1_2(self) -> str:
        return "[TC-MWOM-1.2] Cluster attributes with DUT as Server"

    def steps_TC_MWOM_1_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the SupportedModes attribute"),
            TestStep(3, "Read the CurrentMode attribute"),
        ]
        return steps

    def pics_TC_MWOM_1_2(self) -> list[str]:
        pics = [
            "MWOM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_MWOM_1_2(self):

        endpoint = self.user_params.get("endpoint", 1)

        attributes = Clusters.MicrowaveOvenMode.Attributes

        self.step(1)

        self.step(2)
        supported_modes = await self.read_mod_attribute_expect_success(endpoint=endpoint, attribute=attributes.SupportedModes)
        asserts.assert_greater_equal(len(supported_modes), 2, "SupportedModes must have at least 2 entries!")
        asserts.assert_less_equal(len(supported_modes), 255, "SupportedModes must have at most 255 entries!")
        modes = set([m.mode for m in supported_modes])
        asserts.assert_equal(len(modes), len(supported_modes), "SupportedModes must have unique mode values")

        labels = set([m.label for m in supported_modes])
        asserts.assert_equal(len(labels), len(supported_modes), "SupportedModes must have unique mode label values")

        # common mode tags
        commonTags = {0x0: 'Auto',
                      0x1: 'Quick',
                      0x2: 'Quiet',
                      0x3: 'LowNoise',
                      0x4: 'LowEnergy',
                      0x5: 'Vacation',
                      0x6: 'Min',
                      0x7: 'Max',
                      0x8: 'Night',
                      0x9: 'Day'}

        # derived cluster defined tags
        # kUnknownEnumValue may not be defined
        try:
            derivedTags = [tag.value for tag in Clusters.MicrowaveOvenMode.Enums.ModeTag
                           if tag is not Clusters.MicrowaveOvenMode.Enums.ModeTag.kUnknownEnumValue]
        except AttributeError:
            derivedTags = Clusters.MicrowaveOvenMode.Enums.ModeTag

        logging.info("Derived tags: %s" % derivedTags)

        for m in supported_modes:
            for t in m.modeTags:
                is_mfg = (0x8000 <= t.value and t.value <= 0xBFFF)
                asserts.assert_true(t.value in commonTags.keys() or t.value in derivedTags or is_mfg,
                                    "Found a SupportedModes entry with invalid mode tag value!")
                if t.value == Clusters.MicrowaveOvenMode.Enums.ModeTag.kNormal:
                    normal_present = True
                    logging.info("Found normal mode tag %s with tag value %s", m.mode, t.value)

        asserts.assert_true(normal_present, "SupportedModes does not have an entry of Normal(0x4000)")

        self.step(3)
        current_mode = await self.read_mod_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentMode)
        logging.info("CurrentMode: %s" % current_mode)
        asserts.assert_true(current_mode in modes, "CurrentMode is not a supported mode!")


if __name__ == "__main__":
    default_matter_test_main()
