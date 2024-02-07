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
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_RVCCLEANM_1_2(MatterBaseTest):
    def __init__(self, *args):
        super().__init__(*args)
        self.endpoint = 0
        self.commonTags = {0x0: 'Auto',
                           0x1: 'Quick',
                           0x2: 'Quiet',
                           0x3: 'LowNoise',
                           0x4: 'LowEnergy',
                           0x5: 'Vacation',
                           0x6: 'Min',
                           0x7: 'Max',
                           0x8: 'Night',
                           0x9: 'Day'}
        self.cleanTags = [tag.value for tag in Clusters.RvcCleanMode.Enums.ModeTag]
        self.supported_modes_dut = []

    async def read_mod_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.RvcCleanMode
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def pics_TC_RVCCLEANM_1_2(self) -> list[str]:
        return ["RVCCLEANM.S"]

    @async_test_body
    async def test_TC_RVCCLEANM_1_2(self):
        self.endpoint = self.matter_test_config.endpoint

        attributes = Clusters.RvcCleanMode.Attributes

        self.print_step(1, "Commissioning, already done")

        if self.check_pics("RVCCLEANM.S.A0000"):
            self.print_step(2, "Read SupportedModes attribute")
            supported_modes = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.SupportedModes)

            logging.info("SupportedModes: %s" % (supported_modes))

            # Verify that the list has at least 2 and at most 255 entries
            asserts.assert_greater_equal(len(supported_modes), 2, "SupportedModes must have at least 2 entries!")
            asserts.assert_less_equal(len(supported_modes), 255, "SupportedModes must have at most 255 entries!")

            # Verify that each ModeOptionsStruct entry has a unique Mode field value
            for m in supported_modes:
                if m.mode in self.supported_modes_dut:
                    asserts.fail("SupportedModes must have unique mode values!")
                else:
                    self.supported_modes_dut.append(m.mode)

            # Verify that each ModeOptionsStruct entry has a unique Label field value
            labels = []
            for m in supported_modes:
                if m.label in labels:
                    asserts.fail("SupportedModes must have unique mode label values!")
                else:
                    labels.append(m.label)

            # Verify that each ModeOptionsStruct entry's ModeTags field has:
            for m in supported_modes:
                # * at least one entry
                if len(m.modeTags) == 0:
                    asserts.fail("SupportedModes must have at least one mode tag!")

                at_least_one_common_or_clean_tag = False
                for t in m.modeTags:
                    # * the values of the Value fields that are not larger than 16 bits
                    if t.value > 0xFFFF or t.value < 0:
                        asserts.fail("Mode tag values must not be larger than 16 bits!")

                    # * for each Value field: {isCommonOrDerivedOrMfgTagsVal}
                    is_mfg = (0x8000 <= t.value <= 0xBFFF)
                    if (t.value not in self.commonTags and
                            t.value not in self.cleanTags and
                            not is_mfg):
                        asserts.fail("Mode tag value is not a common tag, clean tag or vendor tag!")

                    # * for at least one Value field: {isCommonOrDerivedTagsVal}
                    if not is_mfg:
                        at_least_one_common_or_clean_tag = True

                if not at_least_one_common_or_clean_tag:
                    asserts.fail("At least one mode tag must be a common tag or clean tag!")

            # Verify that at least one ModeOptionsStruct entry includes either the
            # Vacuum(0x4001) mode tag or the Mop(0x4002)mode tag in the ModeTags field
            vacuum_mop_tags = [Clusters.RvcCleanMode.Enums.ModeTag.kVacuum, Clusters.RvcCleanMode.Enums.ModeTag.kMop]
            has_vacuum_or_mop_mode_tag = False
            for m in supported_modes:
                has_vacuum_or_mop_mode_tag = any(t.value in vacuum_mop_tags for t in m.modeTags)
                if has_vacuum_or_mop_mode_tag:
                    break

            asserts.assert_true(has_vacuum_or_mop_mode_tag,
                                "At least one ModeOptionsStruct entry must include either the Vacuum or Mop mode tag")

        if self.check_pics("RVCCLEANM.S.A0001"):
            self.print_step(3, "Read CurrentMode attribute")
            current_mode = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

            logging.info("CurrentMode: %s" % (current_mode))
            asserts.assert_true(current_mode in self.supported_modes_dut, "CurrentMode is not a supported mode!")


if __name__ == "__main__":
    default_matter_test_main()
