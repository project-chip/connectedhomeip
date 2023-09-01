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
from mobly import asserts

# This test requires several additional command line arguments
# run with
# --int-arg PIXIT_ENDPOINT:<endpoint>


class TC_RVCRUNM_1_2(MatterBaseTest):

    async def read_mod_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.RvcRunMode
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    @async_test_body
    async def test_TC_RVCRUNM_1_2(self):

        asserts.assert_true('PIXIT_ENDPOINT' in self.matter_test_config.global_test_params,
                            "PIXIT_ENDPOINT must be included on the command line in "
                            "the --int-arg flag as PIXIT_ENDPOINT:<endpoint>")

        self.endpoint = self.matter_test_config.global_test_params['PIXIT_ENDPOINT']

        attributes = Clusters.RvcRunMode.Attributes

        self.print_step(1, "Commissioning, already done")

        if self.check_pics("RVCRUNM.S.A0000"):
            self.print_step(2, "Read SupportedModes attribute")
            supported_modes = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.SupportedModes)

            logging.info("SupportedModes: %s" % (supported_modes))

            asserts.assert_greater_equal(len(supported_modes), 2, "SupportedModes must have at least 2 entries!")
            asserts.assert_less_equal(len(supported_modes), 255, "SupportedModes must have at most 255 entries!")

            modes = []
            for m in supported_modes:
                if m.mode in modes:
                    asserts.fail("SupportedModes must have unique mode values!")
                else:
                    modes.append(m.mode)

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

            runTags = [tag.value for tag in Clusters.RvcRunMode.Enums.ModeTag
                       if tag is not Clusters.RvcRunMode.Enums.ModeTag.kUnknownEnumValue]

            for m in supported_modes:
                for t in m.modeTags:
                    is_mfg = (0x8000 <= t.value and t.value <= 0xBFFF)
                    asserts.assert_true(t.value in commonTags.keys() or t.value in runTags or is_mfg,
                                        "Found a SupportedModes entry with invalid mode tag value!")
                    if t.value == Clusters.RvcRunMode.Enums.ModeTag.kIdle:
                        idle_present = True
                    if t.value == Clusters.RvcRunMode.Enums.ModeTag.kCleaning:
                        cleaning_present = True
            asserts.assert_true(idle_present, "The Supported Modes does not have an entry of Idle(0x4000)")
            asserts.assert_true(cleaning_present, "The Supported Modes does not have an entry of Cleaning(0x4001)")

        if self.check_pics("RVCRUNM.S.A0001"):
            self.print_step(3, "Read CurrentMode attribute")
            current_mode = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

            logging.info("CurrentMode: %s" % (current_mode))
            asserts.assert_true(current_mode in modes, "CurrentMode is not a supported mode!")

        if self.check_pics("RVCRUNM.S.A0003"):
            self.print_step(4, "Read OnMode attribute")
            on_mode = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.OnMode)

            logging.info("OnMode: %s" % (on_mode))
            asserts.assert_true(on_mode in modes or on_mode == NullValue, "OnMode is not a supported mode!")

        if self.check_pics("RVCRUNM.S.A0002"):
            self.print_step(5, "Read StartUpMode attribute")
            startup_mode = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.StartUpMode)

            logging.info("StartUpMode: %s" % (startup_mode))
            asserts.assert_true(startup_mode in modes or startup_mode == NullValue, "StartUpMode is not a supported mode!")


if __name__ == "__main__":
    default_matter_test_main()
