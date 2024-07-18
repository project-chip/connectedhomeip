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
# test-runner-run/run1/app: ${WATER_HEATER_MANAGEMENT_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --featureSet 0x03
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from EWATERHTRBase import EWATERHTRBase
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_EWATERHTR_2_1(MatterBaseTest, EWATERHTRBase):

    def desc_TC_EWATERHTR_2_1(self) -> str:
        """Returns a description of this test"""
        return "[TC-EWATERHTR-2.1] Attributes with attributes with DUT as Server\n" \
            "This test case verifies the non-global attributes of the Water Heater Management cluster server."

    def pics_TC_EWATERHTR_2_1(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["EWATERHTR.S", "EWATERHTR.S.F00", "EWATERHTR.S.F01"]

    def steps_TC_EWATERHTR_2_1(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "TH reads HeaterTypes attribute. DUT as Server replies with a WaterHeaterTypeBitmap (enum8) value to match the DUT type."),
            TestStep("2b", "TH reads HeatDemand attribute. DUT as Server replies with a WaterHeaterDemandBitmap (enum8)."),
            TestStep("2c", "TH reads TankVolume attribute. DUT as Server replies with a uint16 value."),
            TestStep("2d", "TH reads EstimatedHeatRequired attribute. DUT as Server replies with an energy-mWh value."),
            TestStep("2e", "TH reads TankPercentage attribute. DUT as Server replies with a percent value."),
            TestStep("2f", "TH reads BoostState attribute. DUT as Server replies with a BoostStateEnum (enum8) value."),
        ]

        return steps

    @async_test_body
    async def test_TC_EWATERHTR_2_1(self):

        self.step("1")
        # Commission DUT - already done

        # Note the values used here are configured in WhmManufacturer::Init()
        self.step("2a")
        await self.check_whm_attribute("HeaterTypes", 0)

        self.step("2b")
        await self.check_whm_attribute("HeatDemand", 0)

        self.step("2c")
        await self.check_whm_attribute("TankVolume", 0)

        self.step("2d")
        await self.check_whm_attribute("EstimatedHeatRequired", 0)

        self.step("2e")
        await self.check_whm_attribute("TankPercentage", 0)

        self.step("2f")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)


if __name__ == "__main__":
    default_matter_test_main()
