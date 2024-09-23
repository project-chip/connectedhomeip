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
# test-runner-run/run1/app: ${ENERGY_MANAGEMENT_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --application water-heater
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_WHM_1_2(MatterBaseTest):

    async def read_mode_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.WaterHeaterMode
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_WHM_1_2(self) -> str:
        return "[TC-WHM-1.2] Cluster attributes with DUT as Server"

    def steps_TC_WHM_1_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the SupportedModes attribute"),
            TestStep(3, "Read the CurrentMode attribute"),
        ]
        return steps

    def pics_TC_WHM_1_2(self) -> list[str]:
        pics = [
            "WHM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_WHM_1_2(self):

        endpoint = self.user_params.get("endpoint", 1)

        attributes = Clusters.WaterHeaterMode.Attributes

        self.step(1)

        self.step(2)
        supported_modes = await self.read_mode_attribute_expect_success(endpoint=endpoint, attribute=attributes.SupportedModes)
        asserts.assert_greater_equal(len(supported_modes), 2,
                                     "SupportedModes must have at least 2 entries!")
        asserts.assert_less_equal(len(supported_modes), 255,
                                  "SupportedModes must have at most 255 entries!")
        modes = set([m.mode for m in supported_modes])
        asserts.assert_equal(len(modes), len(supported_modes),
                             "SupportedModes must have unique mode values")

        labels = set([m.label for m in supported_modes])
        asserts.assert_equal(len(labels), len(supported_modes),
                             "SupportedModes must have unique mode label values")

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
        derivedTags = [tag.value for tag in Clusters.WaterHeaterMode.Enums.ModeTag]

        logging.info("Derived tags: %s" % derivedTags)

        # According to the Mode spec:
        # At least one entry in the SupportedModes attribute SHALL include the Manual mode tag in the ModeTags field list.
        # At least one entry in the SupportedModes attribute SHALL include the Off mode tag in the ModeTags field list.
        # An entry in the SupportedModes attribute that includes one of an Off, Manual, or Timed tag
        # SHALL NOT also include an additional instance of any one of these tag types.
        off_present = 0
        manual_present = 0
        timed_present = 0

        for m in supported_modes:
            off_manual_timed_present_in_this_mode = 0
            for t in m.modeTags:
                is_mfg = (0x8000 <= t.value and t.value <= 0xBFFF)
                asserts.assert_true(t.value in commonTags.keys() or t.value in derivedTags or is_mfg,
                                    "Found a SupportedModes entry with invalid mode tag value!")
                if t.value == Clusters.WaterHeaterMode.Enums.ModeTag.kOff:
                    off_present += 1
                    off_manual_timed_present_in_this_mode += 1
                    logging.info(
                        "Found Off mode tag %s with tag value %s", m.mode, t.value)

                if t.value == Clusters.WaterHeaterMode.Enums.ModeTag.kManual:
                    manual_present += 1
                    off_manual_timed_present_in_this_mode += 1
                    logging.info(
                        "Found Manual mode tag %s with tag value %s", m.mode, t.value)

                if t.value == Clusters.WaterHeaterMode.Enums.ModeTag.kTimed:
                    timed_present += 1
                    off_manual_timed_present_in_this_mode += 1
                    logging.info(
                        "Found Timed mode tag %s with tag value %s", m.mode, t.value)

            asserts.assert_less_equal(off_manual_timed_present_in_this_mode, 1,
                                      f"The supported mode ({m.mode}) should only include one of OFF, MANUAL or TIMED, but includes more than one.")

        asserts.assert_greater(off_present, 0,
                               "SupportedModes does not have an entry of Off(0x4000)")
        asserts.assert_greater(manual_present, 0,
                               "SupportedModes does not have an entry of Manual(0x4001)")

        asserts.assert_less_equal(off_present, 1,
                                  "SupportedModes cannot have more than one instance of Off(0x4000)")
        asserts.assert_less_equal(manual_present, 1,
                                  "SupportedModes cannot have more than one instance of Manual(0x4001)")
        asserts.assert_less_equal(timed_present, 1,
                                  "SupportedModes cannot have more than one instance of Timed(0x4002)")

        self.step(3)
        current_mode = await self.read_mode_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentMode)
        logging.info("CurrentMode: %s" % current_mode)
        asserts.assert_true(current_mode in modes,
                            "CurrentMode is not a supported mode!")


if __name__ == "__main__":
    default_matter_test_main()
