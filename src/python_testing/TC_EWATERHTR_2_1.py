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
# test-runner-run/run1/app: ${ENERGY_MANAGEMENT_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --featureSet 0x03 --application water-heater
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
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
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test).",
                     is_commissioning=True),
            TestStep("2", "TH reads from the DUT the FeatureMap attribute.",
                     "Verify that the DUT response contains the FeatureMap attribute. Store the value as FeatureMap."),
            TestStep("3", "TH reads from the DUT the HeaterTypes attribute.",
                     "Verify that the DUT response contains a WaterHeaterTypeBitmap (enum8) greater than 0x00 (at least one type supported), and less than 0x20 (no undefined types supported)"),
            TestStep("4", "TH reads from the DUT the HeatDemand attribute.",
                     "Verify that the DUT response contains a WaterHeaterDemandBitmap (enum8) value less than 0x20 (no undefined types supported)."),
            TestStep("5", "TH reads from the DUT the TankVolume attribute.",
                     "Verify that the DUT response contains a uint16 value."),
            TestStep("6", "TH reads from the DUT the EstimatedHeatRequired attribute.",
                     "Verify that the DUT response contains an energy-mWh value that is greater or equal to 0."),
            TestStep("7", "TH reads from the DUT the TankPercentage attribute.",
                     "Verify that the DUT response contains a percent value that is between 0 and 100 inclusive."),
            TestStep("8", "TH reads from the DUT the BoostState attribute.",
                     "Verify that the DUT response contains a BoostStateEnum (enum8) value that is less than or equal to 1."),
        ]

        return steps

    @async_test_body
    async def test_TC_EWATERHTR_2_1(self):

        self.step("1")
        # Commission DUT - already done

        # Get the feature map for later
        self.step("2")
        feature_map = await self.read_whm_attribute_expect_success(attribute="FeatureMap")
        em_supported = bool(feature_map & Clusters.WaterHeaterManagement.Bitmaps.Feature.kEnergyManagement)
        tp_supported = bool(feature_map & Clusters.WaterHeaterManagement.Bitmaps.Feature.kTankPercent)
        logger.info(f"FeatureMap: {feature_map} : TP supported: {tp_supported} | EM supported: {em_supported}")

        self.step("3")
        heaterTypes = await self.read_whm_attribute_expect_success(attribute="HeaterTypes")
        asserts.assert_greater(heaterTypes, 0,
                               f"Unexpected HeaterTypes value - expected {heaterTypes} > 0")
        asserts.assert_less_equal(heaterTypes, Clusters.WaterHeaterManagement.Bitmaps.WaterHeaterHeatSourceBitmap.kOther,
                                  f"Unexpected HeaterTypes value - expected {heaterTypes} <= WaterHeaterHeatSourceBitmap.kOther")

        self.step("4")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_less_equal(heatDemand, Clusters.WaterHeaterManagement.Bitmaps.WaterHeaterHeatSourceBitmap.kOther,
                                  f"Unexpected HeatDemand value - expected {heatDemand} <= WaterHeaterHeatSourceBitmap.kOther")

        self.step("5")
        if em_supported:
            value = await self.read_whm_attribute_expect_success(attribute="TankVolume")
        else:
            logging.info("Skipping step 5 as PICS.EWATERHTR.F00(EnergyManagement) not supported")

        self.step("6")
        if em_supported:
            value = await self.read_whm_attribute_expect_success(attribute="EstimatedHeatRequired")
            asserts.assert_greater_equal(value, 0, f"Unexpected EstimatedHeatRequired value - expected {value} >= 0")
        else:
            logging.info("Skipping step 6 as PICS.EWATERHTR.F00(EnergyManagement) not supported")

        self.step("7")
        if tp_supported:
            value = await self.read_whm_attribute_expect_success(attribute="TankPercentage")
            asserts.assert_greater_equal(value, 0, f"Unexpected TankPercentage value - expected {value} >= 0")
            asserts.assert_less_equal(value, 100, f"Unexpected TankPercentage value - expected {value} <= 100")
        else:
            logging.info("Skipping step 7 as PICS.EWATERHTR.F01(TankPercenage) not supported")

        self.step("8")
        boost_state = await self.read_whm_attribute_expect_success(attribute="BoostState")
        asserts.assert_less_equal(boost_state, Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive,
                                  f"Unexpected BoostState value - expected {boost_state} should be BoostStateEnum (enum8) value in range 0x00 to 0x01")


if __name__ == "__main__":
    default_matter_test_main()
