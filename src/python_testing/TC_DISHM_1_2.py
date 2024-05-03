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

import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

class TC_DISHM_1_2(MatterBaseTest):

    async def read_mode_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.DishwasherMode
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_DISHM_1_2(self) -> str:
        return "[TC-DISHM-1.2] Cluster attributes with DUT as Server"

    def steps_TC_DISHM_1_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the SupportedModes attribute."),
            TestStep(3, "TH reads from the DUT the CurrentMode attribute."),
        ]
        return steps

    def pics_TC_DISHM_1_2(self) -> list[str]:
        pics = [
            "DISHM.S",
        ]
        return pics


    @async_test_body
    async def test_TC_DISHM_1_2(self):

        self.endpoint = self.user_params.get("endpoint", 1)
        logging.info("This test expects to find this cluster on endpoint 1")

        attributes = Clusters.DishwasherMode.Attributes

        self.step(1)

        self.step(2)
        supported_modes = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.SupportedModes)

        logging.info("SupportedModes: %s" % (supported_modes))

        # Check the number of modes in SupportedModes.
        asserts.assert_greater_equal(len(supported_modes), 2, "SupportedModes must have at least two entries!")
        asserts.assert_less_equal(len(supported_modes), 255, "SupportedModes must have at most 255 entries!")

        # Check that each Mode field is unique
        supported_modes_dut = []
        for m in supported_modes:
            if m.mode in supported_modes_dut:
                asserts.fail("SupportedModes must have unique mode values!")
            else:
                supported_modes_dut.append(m.mode)

        # Check that each label is unique
        labels = []
        for m in supported_modes:
            if m.label in labels:
                asserts.fail("SupportedModes must have unique mode label values!")
            else:
                labels.append(m.label)

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

        # kUnknownEnumValue may not be defined
        try:
            modeTags = [tag.value for tag in Clusters.DishwasherMode.Enums.ModeTag
                        if tag is not Clusters.DishwasherMode.Enums.ModeTag.kUnknownEnumValue]
        except AttributeError:
            modeTags = Clusters.DishwasherMode.Enums.ModeTag

        normal_present = false
        for m in supported_modes:
            # need at least 1 mode tag entry
            asserts.assert_greater(len(m.modeTags, 0, "Must have at least one mode tag."))
            for t in m.modeTags:
                # value can't exceed 16 bits
                asserts.assert_true(t.value <= 0xFFFF, "Tag value is > 16 bits")

                # check that value field is as expected
                is_mfg = (0x8000 <= t.value and t.value <= 0xBFFF)
                asserts.assert_true(t.value in commonTags.keys() or t.value in modeTags or is_mfg,
                                    "Found a SupportedModes entry with invalid mode tag value!")

                # is this a normal tag? (need at least 1)
                if t.value == Clusters.DishwasherMode.Enums.ModeTag.kNormal:
                    normal_present = True

        # need at least one mode with the normal tag
        asserts.assert_true(normal_present, "The Supported Modes does not have an entry of Normal(0x4000)")

        self.step(3)
        current_mode = await self.read_mode_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

        logging.info("CurrentMode: %s" % (current_mode))
        asserts.assert_true(current_mode in supported_modes_dut, "CurrentMode is not a supported mode!")

if __name__ == "__main__":
    default_matter_test_main()
