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
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --featureSet 0x7c --application evse
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

"""Define Matter test case TC_DEM_2_9."""


import logging

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_DEMTestBase import DEMTestBase

logger = logging.getLogger(__name__)


class TC_DEM_2_9(MatterBaseTest, DEMTestBase):
    """Implementation of test case TC_DEM_2_9."""

    def desc_TC_DEM_2_9(self) -> str:
        """Return a description of this test."""
        return "4.1.3. [TC-DEM-2.2] Power or State Forecast Reporting feature functionality with DUT as Server"

    def pics_TC_DEM_2_9(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            # Depends on Feature 01 (PowerForecastReporting) | Feature 2 (StateForecastReporting)
            "DEM.S.F01", "DEM.S.F02",
        ]
        return pics

    def steps_TC_DEM_2_9(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)",
                     is_commissioning=True),
            TestStep("2", "TH reads from the DUT the _FeatureMap_ attribute",
                     "Verify that the DUT response contains the _FeatureMap_ attribute. Verify one of PowerForecastReporting or StateForecastReporting is supported but not both."),
            TestStep("3", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Forecast Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("4a", "TH reads from the DUT the Forecast",
                     "Value has to include a valid slots[0].ManufacturerESAState"),
            TestStep("4b", "TH reads from the DUT the Forecast",
                     "Value has to include valid slots[0].NominalPower, slots[0].MinPower, slots[0].MaxPower, slots[0].NominalEnergy"),
            TestStep("5", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Forecast Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
        ]

        return steps

    @async_test_body
    async def test_TC_DEM_2_9(self):
        # pylint: disable=too-many-locals, too-many-statements
        """Run the test steps."""
        self.step("1")
        # Commission DUT - already done

        self.step("2")
        await self.validate_pfr_or_sfr_in_feature_map()

        self.step("3")
        await self.check_test_event_triggers_enabled()

        self.step("4")
        await self.send_test_event_trigger_forecast()

        self.step("4a")
        feature_map = await self.read_dem_attribute_expect_success(attribute="FeatureMap")
        if feature_map & Clusters.DeviceEnergyManagement.Bitmaps.Feature.kStateForecastReporting:
            forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
            asserts.assert_is_not_none(forecast.slots[0].manufacturerESAState)
        else:
            logging.info('Device does not support StateForecastReporting. Skipping step 4a')

        self.step("4b")
        if feature_map & Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerForecastReporting:
            forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")

            asserts.assert_is_not_none(forecast.slots[0].nominalPower)
            asserts.assert_is_not_none(forecast.slots[0].minPower)
            asserts.assert_is_not_none(forecast.slots[0].maxPower)
            asserts.assert_is_not_none(forecast.slots[0].nominalEnergy)
        else:
            logging.info('Device does not support PowerForecastReporting. Skipping step 4b')

        self.step("5")
        await self.send_test_event_trigger_forecast_clear()


if __name__ == "__main__":
    default_matter_test_main()
