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
# === BEGIN CI TEST ARGUMENTS ===# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --featureSet 0x03
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto --int-arg PIXIT.EWATERHTR.EM:1 PIXIT.EWATERHTR.TP:2
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from TC_EWATERHTRBase import EWATERHTRBase
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

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
        heater_types = await self.read_whm_attribute_expect_success(attribute="HeaterTypes")
        asserts.assert_greater(heater_types, 0,
                               f"Unexpected HeaterTypes value - expected {heater_types} > 0")
        asserts.assert_less_equal(heater_types, Clusters.WaterHeaterManagement.Bitmaps.WaterHeaterTypeBitmap.kOther,
                                  f"Unexpected HeaterTypes value - expected {heater_types} <= WaterHeaterTypeBitmap.kOther")

        self.step("3")
        heat_demand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heat_demand, 0,
                                     f"Unexpected HeatDemand value - expected {heat_demand} > 0")
        asserts.assert_less_equal(heat_demand, Clusters.WaterHeaterManagement.Bitmaps.WaterHeaterDemandBitmap.kOther,
                                  f"Unexpected HeatDemand value - expected {heat_demand} <= WaterHeaterDemandBitmap.kOther")

        self.step("4")
        if em_supported:
            value = await self.read_whm_attribute_expect_success(attribute="TankVolume")
            asserts.assert_greater_equal(value, 0, f"Unexpected TankVolume value - expected {value} >= 0")
        else:
            logging.info("Skipping step 2c as PIXIT.EWATERHTR.EM not supported")

        self.step("5")
        if em_supported:
            value = await self.read_whm_attribute_expect_success(attribute="EstimatedHeatRequired")
            asserts.assert_greater_equal(value, 0, f"Unexpected EstimatedHeatRequired value - expected {value} >= 0")
        else:
            logging.info("Skipping step 2d as PIXIT.EWATERHTR.EM not supported")

        self.step("6")
        if tp_supported:
            value = await self.read_whm_attribute_expect_success(attribute="TankPercentage")
            asserts.assert_greater_equal(value, 0, f"Unexpected TankPercentage value - expected {value} >= 0")
            asserts.assert_less_equal(value, 100, f"Unexpected TankPercentage value - expected {value} <= 100")
        else:
            logging.info("Skipping step 2e as PIXIT.EWATERHTR.TP not supported")

        self.step("7")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)


if __name__ == "__main__":
    default_matter_test_main()
