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
# pylint: disable=invalid-name

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ENERGY_MANAGEMENT_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --featureSet 0x7e
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

"""Define Matter test case TC_DEM_2_1."""


import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_DEMTestBase import DEMTestBase

logger = logging.getLogger(__name__)


class TC_DEM_2_1(MatterBaseTest, DEMTestBase):
    """Implementation of test case TC_DEM_2_1."""

    def desc_TC_DEM_2_1(self) -> str:
        """Return a description of this test."""
        return "4.1.2. [TC-DEM-2.1] Attributes with DUT as Server"

    def pics_TC_DEM_2_1(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "DEM.S",
        ]
        return pics

    def steps_TC_DEM_2_1(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("1", "Commissioning, already done",
                     is_commissioning=True),
            TestStep("2", "TH reads from the DUT ESAType attribute.",
                     "Verify that the DUT response contains an a ESATypeEnum (enum8) value to match the DUT type."),
            TestStep("3", "TH reads from the DUT the ESACanGenerate attribute.",
                     "Verify that the DUT response contains a boolean value to match the DUT capability."),
            TestStep("4", "TH reads from the DUT ESAState attribute.",
                     "Verify that the DUT response contains an a ESAStateEnum (enum8) value to match the DUT state (Online or Offline)."),
            TestStep("5", "TH reads from the DUT the AbsMinPower attribute.",
                     "Verify that the DUT response contains a power-mW value to match the DUT capability, and is negative if and only if the ESACanGenerate read in step 3 is TRUE."),
            TestStep("6", "TH reads from the DUT the AbsMaxPower attribute.",
                     "Verify that the DUT response contains a power-mW value to match the DUT capability, and greater than or equal to the AbsMinPower read in step 5."),
            TestStep("7", "TH reads from the DUT the PowerAdjustmentCapability attribute.",
                     "Verify that the DUT response contains a list of PowerAdjustStruct value."),
            TestStep("8", "TH reads from the DUT the Forecast attribute.",
                     "Verify that the DUT response contains a ForecastStruct value."),
            TestStep("9", "TH reads from the DUT the OptOutState attribute.",
                     "Verify that the DUT response contains an OptOutStateEnum (enum8) value 0 (NoOptOut)."),
        ]

        return steps

    @async_test_body
    async def test_TC_DEM_2_1(self):
        # pylint: disable=too-many-locals, too-many-statements
        """Run the test steps."""

        self.step("1")
        # Commission DUT - already done

        # Get the feature map for later
        feature_map = await self.read_dem_attribute_expect_success(attribute="FeatureMap")
        logger.info(f"FeatureMap: {feature_map}")

        self.step("2")
        esa_type = await self.read_dem_attribute_expect_success(attribute="ESAType")
        logger.info(f"ESAType is {esa_type} [CHECK THIS IS EXPECTED VALUE]")
        asserts.assert_is_instance(esa_type, Clusters.DeviceEnergyManagement.Enums.ESATypeEnum, "Invalid type for ESAType")

        self.step("3")
        esa_can_generate = await self.read_dem_attribute_expect_success(attribute="ESACanGenerate")
        logger.info(f"ESACanGenerate is: {esa_can_generate} [CHECK THIS IS EXPECTED VALUE]")
        asserts.assert_is_instance(esa_can_generate, bool)

        self.step("4")
        esa_state = await self.read_dem_attribute_expect_success(attribute="ESAState")
        logger.info(f"ESAState is {esa_state}")
        asserts.assert_is_instance(esa_state, Clusters.DeviceEnergyManagement.Enums.ESAStateEnum)

        self.step("5")
        abs_min_power = await self.read_dem_attribute_expect_success(attribute="AbsMinPower")
        logger.info(f"AbsMinPower is {abs_min_power/1000000.0} W [CHECK THIS IS EXPECTED VALUE]")
        asserts.assert_is_instance(abs_min_power, int)
        if not esa_can_generate:
            # ESA's that can't generate must have positive values
            asserts.assert_greater_equal(abs_min_power, 0)

        self.step("6")
        abs_max_power = await self.read_dem_attribute_expect_success(attribute="AbsMaxPower")
        logger.info(f"AbsMaxPower is {abs_max_power/1000000.0} W [CHECK THIS IS EXPECTED VALUE]")
        asserts.assert_is_instance(abs_max_power, int)
        asserts.assert_greater_equal(abs_max_power, abs_min_power)

        self.step("7")
        if Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerAdjustment & feature_map:
            power_adjustment_capability = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
            logger.info(f"PowerAdjustmentCapability is {power_adjustment_capability}")

            if power_adjustment_capability is not NullValue:
                asserts.assert_is_instance(power_adjustment_capability,
                                           Clusters.DeviceEnergyManagement.Structs.PowerAdjustStruct)

        self.step("8")
        if Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerForecastReporting & feature_map or \
                Clusters.DeviceEnergyManagement.Bitmaps.Feature.kStateForecastReporting & feature_map:
            forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
            logger.info(f"Forecast is {forecast}")

            if forecast is not NullValue:
                asserts.assert_is_instance(forecast,
                                           Clusters.DeviceEnergyManagement.Structs.ForecastStruct)

        self.step("9")
        if Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerAdjustment & feature_map or \
                Clusters.DeviceEnergyManagement.Bitmaps.Feature.kStartTimeAdjustment & feature_map or \
                Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPausable & feature_map or \
                Clusters.DeviceEnergyManagement.Bitmaps.Feature.kForecastAdjustment & feature_map or \
                Clusters.DeviceEnergyManagement.Bitmaps.Feature.kConstraintBasedAdjustment & feature_map:
            opt_out_state = await self.read_dem_attribute_expect_success(attribute="OptOutState")
            logger.info(f"OptOutState is {opt_out_state}")
            asserts.assert_equal(opt_out_state,
                                 Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)


if __name__ == "__main__":
    default_matter_test_main()
