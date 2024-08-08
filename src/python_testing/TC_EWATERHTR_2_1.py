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


# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --featureSet 0x03
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto --int-arg PIXIT.EWATERHTR.EM:1 PIXIT.EWATERHTR.TP:2
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_EWATERHTRBase import EWATERHTRBase

logger = logging.getLogger(__name__)


class TC_EWATERHTR_2_1(MatterBaseTest, EWATERHTRBase):

    def desc_TC_EWATERHTR_2_1(self) -> str:
        """Returns a description of this test"""
        return "[TC-EWATERHTR-2.1] Attributes with attributes with DUT as Server\n" \
            "This test case verifies the non-global attributes of the Water Heater Management cluster server."

    def pics_TC_EWATERHTR_2_1(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["EWATERHTR.S"]

    def steps_TC_EWATERHTR_2_1(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done",
                     is_commissioning=True),
            TestStep("2", "TH reads HeaterTypes attribute.",
                     "DUT as Server replies with a WaterHeaterTypeBitmap (enum8) greater than 0x00 (at least one type supported), and less than 0x20 (no undefined types supported)."),
            TestStep("3", "TH reads HeatDemand attribute.",
                     "DUT as Server replies with a WaterHeaterDemandBitmap (enum8)."),
            TestStep("4", "TH reads TankVolume attribute.",
                     "DUT as Server replies with a uint16 value."),
            TestStep("5", "TH reads EstimatedHeatRequired attribute.",
                     "DUT as Server replies with an energy-mWh value."),
            TestStep("6", "TH reads TankPercentage attribute.",
                     "DUT as Server replies with a percent value."),
            TestStep("7", "TH reads BoostState attribute.",
                     "DUT as Server replies with a BoostStateEnum (enum8) value."),
        ]

        return steps

    @async_test_body
    async def test_TC_EWATERHTR_2_1(self):

        em_supported = self.matter_test_config.global_test_params['PIXIT.EWATERHTR.EM']
        tp_supported = self.matter_test_config.global_test_params['PIXIT.EWATERHTR.TP']

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        heaterTypes = await self.read_whm_attribute_expect_success(attribute="HeaterTypes")
        asserts.assert_greater(heaterTypes, 0,
                               f"Unexpected HeaterTypes value - expected {heaterTypes} > 0")
        asserts.assert_less_equal(heaterTypes, Clusters.WaterHeaterManagement.Bitmaps.WaterHeaterTypeBitmap.kOther,
                                  f"Unexpected HeaterTypes value - expected {heaterTypes} <= WaterHeaterTypeBitmap.kOther")

        self.step("3")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0,
                               f"Unexpected HeatDemand value - expected {heatDemand} > 0")
        asserts.assert_less_equal(heatDemand, Clusters.WaterHeaterManagement.Bitmaps.WaterHeaterDemandBitmap.kOther,
                                  f"Unexpected HeatDemand value - expected {heatDemand} <= WaterHeaterDemandBitmap.kOther")

        self.step("4")
        if em_supported:
            value = await self.read_whm_attribute_expect_success(attribute="TankVolume")
        else:
            logging.info("Skipping step 4 as PIXIT.EWATERHTR.EM not supported")

        self.step("5")
        if em_supported:
            value = await self.read_whm_attribute_expect_success(attribute="EstimatedHeatRequired")
            asserts.assert_greater_equal(value, 0, f"Unexpected EstimatedHeatRequired value - expected {value} >= 0")
        else:
            logging.info("Skipping step 5 as PIXIT.EWATERHTR.EM not supported")

        self.step("6")
        if tp_supported:
            value = await self.read_whm_attribute_expect_success(attribute="TankPercentage")
            asserts.assert_greater_equal(value, 0, f"Unexpected TankPercentage value - expected {value} >= 0")
            asserts.assert_less_equal(value, 100, f"Unexpected TankPercentage value - expected {value} <= 100")
        else:
            logging.info("Skipping step 6 as PIXIT.EWATERHTR.TP not supported")

        self.step("7")
        boost_state = await self.read_whm_attribute_expect_success(attribute="BoostState")
        asserts.assert_less_equal(boost_state, Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive,
                                  f"Unexpected BoostState value - expected {boost_state} should be BoostStateEnum (enum8) value in range 0x00 to 0x01")


if __name__ == "__main__":
    default_matter_test_main()
