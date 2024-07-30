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
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --featureSet 0x00
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===


import logging
import time

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_EWATERHTRBase import EWATERHTRBase

logger = logging.getLogger(__name__)


class TC_EWATERHTR_2_2(MatterBaseTest, EWATERHTRBase):

    def desc_TC_EWATERHTR_2_2(self) -> str:
        """Returns a description of this test"""
        return "[TC-EWATERHTR-2.2] Basic functionality with attributes with DUT as Server." \
            "This test case verifies the primary functionality of the Water Heater Management cluster server."

    def pics_TC_EWATERHTR_2_2(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["EWATERHTR.S"]

    def steps_TC_EWATERHTR_2_2(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done",
                     is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster.",
                     "Verify value is 1 (True)"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Basic installation Test Event.",
                     "Verify Command response is Success"),
            TestStep("3a", "TH reads HeatDemand attribute.",
                     "Verify value is 0x00 (no demand on any source)"),
            TestStep("3b", "TH reads BoostState attribute.",
                     "Verify value is 0 (Inactive)"),
            TestStep("3c", "TH reads HeaterTypes attribute.",
                     "Verify value is greater than 0x00 (at least one type supported) and store the value as HeaterTypes"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Manual mode Test Event.",
                     "Verify Command response is Success"),
            TestStep("4a", "TH reads HeatDemand attribute.",
                     "Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("5", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 61C Test Event.",
                     "Verify Command response is Success"),
            TestStep("5a", "TH reads HeatDemand attribute.",
                     "Verify value is 0x00 (no demand on any source)"),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 20C Test Event.",
                     "Verify Command response is Success"),
            TestStep("6a", "TH reads HeatDemand attribute.",
                     "Verify value is greater than 0x00 (demand on at least one source)"),
            TestStep("7", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Off mode Test Event.",
                     "Verify Command response is Success"),
            TestStep("7a", "TH reads HeatDemand attribute.",
                     "Verify value is 0x00 (no demand on any source)"),
            TestStep("8", "TH sends Boost with Duration=5s,OneShot=True.",
                     "Verify Command response is Success"),
            TestStep("8a", "TH reads HeatDemand attribute.",
                     "Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("8b", "TH reads BoostState attribute.",
                     "Verify value is 1 (Active)"),
            TestStep("9", "Wait 6 seconds"),
            TestStep("9a", "TH reads HeatDemand attribute.",
                     "Verify value is 0x00 (no demand on any source)"),
            TestStep("9b", "TH reads BoostState attribute.",
                     "Verify value is 0 (Inactive)"),
            TestStep("10", "TH sends Boost with Duration=600s,OneShot=True.",
                     "Verify Command response is Success"),
            TestStep("10a", "TH reads HeatDemand attribute.",
                     "Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("10b", "TH reads BoostState attribute.",
                     "Verify value is 1 (Active)"),
            TestStep("11", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 61C Test Event.",
                     "Verify Command response is Success"),
            TestStep("11a", "TH reads HeatDemand attribute.",
                     "Verify value is 0x00 (no demand on any source)"),
            TestStep("11b", "TH reads BoostState attribute.",
                     "Verify value is 0 (Inactive)"),
            TestStep("12", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 20C Test Event.",
                     "Verify Command response is Success"),
            TestStep("12a", "TH reads HeatDemand attribute.",
                     "Verify value is 0x00 (no demand on any source)"),
            TestStep("13", "TH sends Boost with Duration=600s.",
                     "Verify Command response is Success"),
            TestStep("13a", "TH reads HeatDemand attribute.",
                     "Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("13b", "TH reads BoostState attribute.",
                     "Verify value is 1 (Active)"),
            TestStep("14", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 61C Test Event.",
                     "Verify Command response is Success"),
            TestStep("14a", "TH reads HeatDemand attribute.",
                     "Verify value is 0x00 (no demand on any source)"),
            TestStep("14b", "TH reads BoostState attribute.",
                     "Verify value is 1 (Active)"),
            TestStep("15", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 20C Test Event.",
                     "Verify Command response is Success"),
            TestStep("15a", "TH reads HeatDemand attribute.",
                     "Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("15b", "TH reads BoostState attribute.",
                     "Verify value is 1 (Active)"),
            TestStep("16", "TH sends CancelBoost.",
                     "Verify Command response is Success"),
            TestStep("16a", "TH reads HeatDemand attribute.",
                     "Verify value is 0x00 (no demand on any source)"),
            TestStep("16b", "TH reads BoostState attribute.",
                     "Verify value is 0 (Inactive)"),
            TestStep("17", "TH sends Boost with Duration=600s,TemporarySetpoint=65C.",
                     "Verify Command response is Success"),
            TestStep("17a", "TH reads HeatDemand attribute.",
                     "Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("17b", "TH reads BoostState attribute.",
                     "Verify value is 1 (Active)"),
            TestStep("18", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 61C Test Event.",
                     "Verify Command response is Success"),
            TestStep("18a", "TH reads HeatDemand attribute.",
                     "Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("18b", "TH reads BoostState attribute.",
                     "Verify value is 1 (Active)"),
            TestStep("19", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 66C Test Event.",
                     "Verify Command response is Success"),
            TestStep("19a", "TH reads HeatDemand attribute.",
                     "Verify value is 0x00 (no demand on any source)"),
            TestStep("19b", "TH reads BoostState attribute.",
                     "Verify value is 1 (Active)"),
            TestStep("20", "TH sends Boost with Duration=600s,TemporarySetpoint=70C.",
                     "Verify Command response is Success"),
            TestStep("20a", "TH reads HeatDemand attribute.",
                     "Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("20b", "TH reads BoostState attribute.",
                     "Verify value is 1 (Active)"),
            TestStep("21", "TH sends CancelBoost.",
                     "Verify Command response is Success"),
            TestStep("21a", "TH reads HeatDemand attribute.",
                     "Verify value is 0x00 (no demand on any source)"),
            TestStep("21b", "TH reads BoostState attribute.",
                     "Verify value is 0 (Inactive)"),
            TestStep("22", "TH sends CancelBoost.",
                     "Verify Command response is Success"),
            TestStep("23", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Basic installation Test Event Clear.",
                     "Verify Command response is Success"),
        ]

        return steps

    @async_test_body
    async def test_TC_EWATERHTR_2_2(self):

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        await self.check_test_event_triggers_enabled()

        self.step("3")
        await self.send_test_event_trigger_basic_installation_test_event()

        self.step("3a")
        await self.check_whm_attribute("HeatDemand", 0)

        self.step("3b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)

        self.step("3c")
        heaterTypes = await self.read_whm_attribute_expect_success(attribute="HeaterTypes")
        asserts.assert_greater(heaterTypes, 0,
                               f"Unexpected HeaterTypes value - expected {heaterTypes} > 0")
        asserts.assert_less_equal(heaterTypes, Clusters.WaterHeaterManagement.Bitmaps.WaterHeaterTypeBitmap.kOther,
                                  f"Unexpected HeaterTypes value - expected {heaterTypes} <= WaterHeaterTypeBitmap.kOther")

        self.step("4")
        await self.send_test_event_trigger_manual_mode_test_event()

        self.step("4a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("5")
        await self.send_test_event_trigger_water_temperature61C_test_event()

        self.step("5a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("6")
        await self.send_test_event_trigger_water_temperature20C_test_event()

        self.step("6a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("7")
        await self.send_test_event_trigger_off_mode_test_event()

        self.step("7a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("8")
        await self.send_boost_command(duration=5, one_shot=True)

        self.step("8a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("8b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("9")
        time.sleep(6)

        self.step("9a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("9b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)

        self.step("10")
        await self.send_boost_command(duration=600, one_shot=True)

        self.step("10a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("10b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("11")
        await self.send_test_event_trigger_water_temperature61C_test_event()

        self.step("11a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("11b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)

        self.step("12")
        await self.send_test_event_trigger_water_temperature20C_test_event()

        self.step("12a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("13")
        await self.send_boost_command(duration=600)

        self.step("13a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("13b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("14")
        await self.send_test_event_trigger_water_temperature61C_test_event()

        self.step("14a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("14b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("15")
        await self.send_test_event_trigger_water_temperature20C_test_event()

        self.step("15a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("15b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("16")
        await self.send_cancel_boost_command()

        self.step("16a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("16b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)

        self.step("17")
        await self.send_boost_command(duration=600, temporary_setpoint=6500)

        self.step("17a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("17b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("18")
        await self.send_test_event_trigger_water_temperature61C_test_event()

        self.step("18a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("18b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("19")
        await self.send_test_event_trigger_water_temperature66C_test_event()

        self.step("19a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("19b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("20")
        await self.send_boost_command(duration=600, temporary_setpoint=7000)

        self.step("20a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("20b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("21")
        await self.send_cancel_boost_command()

        self.step("21a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("21b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)

        self.step("22")
        await self.send_cancel_boost_command()

        self.step("23")
        await self.send_test_event_trigger_basic_installation_test_event_clear()


if __name__ == "__main__":
    default_matter_test_main()
