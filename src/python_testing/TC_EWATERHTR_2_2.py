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
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --featureSet 0x00 --application water-heater
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===


import logging
import time

import chip.clusters as Clusters
from matter_testing_support import EventChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
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
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)",
                     is_commissioning=True),
            TestStep("2", "Set up a subscription to all WaterHeaterManagement cluster events"),
            TestStep("3", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Basic installation Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("4a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("4b", "TH reads from the DUT the BoostState",
                     "Value has to be 0 (Inactive)"),
            TestStep("4c", "TH reads from the DUT the HeaterTypes",
                     "Value has to be greater than 0x00 (at least one type supported) and Store the value as HeaterTypes"),
            TestStep("5", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Manual mode Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("5a", "TH reads from the DUT the HeatDemand",
                     "Value has to be greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Water Temperature 61C Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("6a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("7", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Water Temperature 20C Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("7a", "TH reads from the DUT the HeatDemand",
                     "Value has to be greater than 0x00 (demand on at least one source)"),
            TestStep("8", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Off mode Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("8a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("9", "TH sends command Boost with Duration=5s,OneShot=True",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EWATERHTR.S.E00(BoostStarted) sent with Duration=5 and OneShot=True"),
            TestStep("9a", "TH reads from the DUT the HeatDemand",
                     "Value has to be greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("9b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("10", "Wait 6 seconds",
                     "Event EWATERHTR.S.E00(BoostEnded) sent"),
            TestStep("10a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("10b", "TH reads from the DUT the BoostState",
                     "Value has to be 0 (Inactive)"),
            TestStep("11", "TH sends command Boost with Duration=600s,OneShot=True",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EWATERHTR.S.E00(BoostStarted) sent with Duration=600 and OneShot=True"),
            TestStep("11a", "TH reads from the DUT the HeatDemand",
                     "Value has to be greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("11b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("12", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Water Temperature 61C Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EWATERHTR.S.E00(BoostEnded) sent"),
            TestStep("12a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("12b", "TH reads from the DUT the BoostState",
                     "Value has to be 0 (Inactive)"),
            TestStep("13", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Water Temperature 20C Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("13a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("14", "TH sends command Boost with Duration=600s",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EWATERHTR.S.E00(BoostStarted) sent with Duration=600 and OneShot=None"),
            TestStep("14a", "TH reads from the DUT the HeatDemand",
                     "Value has to be greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("14b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("15", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Water Temperature 61C Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("15a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("15b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("16", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Water Temperature 20C Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("16a", "TH reads from the DUT the HeatDemand",
                     "Value has to be greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("16b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("17", "TH sends command CancelBoost",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EWATERHTR.S.E00(BoostEnded) sent"),
            TestStep("17a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("17b", "TH reads from the DUT the BoostState",
                     "Value has to be 0 (Inactive)"),
            TestStep("18", "TH sends command Boost with Duration=500s,TemporarySetpoint=65C",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EWATERHTR.S.E00(BoostStarted) sent with Duration=500 and TemporarySetpoint=6500"),
            TestStep("18a", "TH reads from the DUT the HeatDemand",
                     "Value has to be greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("18b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("19", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Water Temperature 61C Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("19a", "TH reads from the DUT the HeatDemand",
                     "Value has to be greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("19b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("20", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Water Temperature 66C Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("20a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("20b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("21", "TH sends command Boost with Duration=600s,TemporarySetpoint=70C",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EWATERHTR.S.E00(BoostStarted) sent with Duration=600 and TemporarySetpoint=7000"),
            TestStep("21a", "TH reads from the DUT the HeatDemand",
                     "Value has to be greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("21b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("22", "TH sends command CancelBoost",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EWATERHTR.S.E00(BoostEnded) sent"),
            TestStep("22a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("22b", "TH reads from the DUT the BoostState",
                     "Value has to be 0 (Inactive)"),
            TestStep("23", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Water Temperature 20C Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("23a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("24", "TH sends command Boost with Duration=200s,EmergencyBoost=True",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EWATERHTR.S.E00(BoostStarted) sent with Duration=200, EmergencyBoost=True and TemporarySetpoint=None"),
            TestStep("24a", "TH reads from the DUT the HeatDemand",
                     "Value has to be greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("24b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("25", "TH sends command CancelBoost",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EWATERHTR.S.E00(BoostEnded) sent"),
            TestStep("25a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("25b", "TH reads from the DUT the BoostState",
                     "Value has to be 0 (Inactive)"),
            TestStep("26", "TH sends command CancelBoost",
                     "Verify DUT responds w/ status SUCCESS(0x00) and no event sent"),
            TestStep("27", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Basic installation Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
        ]

        return steps

    @async_test_body
    async def test_TC_EWATERHTR_2_2(self):

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        # Subscribe to Events and when they are sent push them to a queue for checking later
        events_callback = EventChangeCallback(Clusters.WaterHeaterManagement)
        await events_callback.start(self.default_controller,
                                    self.dut_node_id,
                                    self.matter_test_config.endpoint)

        self.step("3")
        await self.check_test_event_triggers_enabled()

        self.step("4")
        await self.send_test_event_trigger_basic_installation_test_event()

        self.step("4a")
        await self.check_whm_attribute("HeatDemand", 0)

        self.step("4b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)

        self.step("4c")
        heaterTypes = await self.read_whm_attribute_expect_success(attribute="HeaterTypes")
        asserts.assert_greater(heaterTypes, 0,
                               f"Unexpected HeaterTypes value - expected {heaterTypes} > 0")
        asserts.assert_less_equal(heaterTypes, Clusters.WaterHeaterManagement.Bitmaps.WaterHeaterHeatSourceBitmap.kOther,
                                  f"Unexpected HeaterTypes value - expected {heaterTypes} <= WaterHeaterHeatSourceBitmap.kOther")

        self.step("5")
        await self.send_test_event_trigger_manual_mode_test_event()

        self.step("5a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("6")
        await self.send_test_event_trigger_water_temperature61C_test_event()

        self.step("6a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("7")
        await self.send_test_event_trigger_water_temperature20C_test_event()

        self.step("7a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("8")
        await self.send_test_event_trigger_off_mode_test_event()

        self.step("8a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("9")
        await self.send_boost_command(duration=5, one_shot=True)

        event_data = events_callback.wait_for_event_report(Clusters.WaterHeaterManagement.Events.BoostStarted)
        asserts.assert_equal(event_data.boostInfo.duration, 5)
        asserts.assert_equal(event_data.boostInfo.oneShot, True)

        self.step("9a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("9b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("10")
        time.sleep(6)
        event_data = events_callback.wait_for_event_report(Clusters.WaterHeaterManagement.Events.BoostEnded)

        self.step("10a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("10b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)

        self.step("11")
        await self.send_boost_command(duration=600, one_shot=True)

        event_data = events_callback.wait_for_event_report(Clusters.WaterHeaterManagement.Events.BoostStarted)
        asserts.assert_equal(event_data.boostInfo.duration, 600)
        asserts.assert_equal(event_data.boostInfo.oneShot, True)

        self.step("11a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("11b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("12")
        await self.send_test_event_trigger_water_temperature61C_test_event()

        event_data = events_callback.wait_for_event_report(Clusters.WaterHeaterManagement.Events.BoostEnded)

        self.step("12a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("12b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)

        self.step("13")
        await self.send_test_event_trigger_water_temperature20C_test_event()

        self.step("13a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("14")
        await self.send_boost_command(duration=600)

        event_data = events_callback.wait_for_event_report(Clusters.WaterHeaterManagement.Events.BoostStarted)
        asserts.assert_equal(event_data.boostInfo.duration, 600)
        asserts.assert_true(event_data.boostInfo.oneShot is None, "oneShot should be None")

        self.step("14a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("14b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("15")
        await self.send_test_event_trigger_water_temperature61C_test_event()

        self.step("15a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("15b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("16")
        await self.send_test_event_trigger_water_temperature20C_test_event()

        self.step("16a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("16b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("17")
        await self.send_cancel_boost_command()

        event_data = events_callback.wait_for_event_report(Clusters.WaterHeaterManagement.Events.BoostEnded)

        self.step("17a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("17b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)

        self.step("18")
        await self.send_boost_command(duration=500, temporary_setpoint=6500)

        event_data = events_callback.wait_for_event_report(Clusters.WaterHeaterManagement.Events.BoostStarted)
        asserts.assert_equal(event_data.boostInfo.duration, 500)
        asserts.assert_equal(event_data.boostInfo.temporarySetpoint, 6500)

        self.step("18a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("18b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("19")
        await self.send_test_event_trigger_water_temperature61C_test_event()

        self.step("19a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("19b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("20")
        await self.send_test_event_trigger_water_temperature66C_test_event()

        self.step("20a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("20b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("21")
        await self.send_boost_command(duration=600, temporary_setpoint=7000)

        event_data = events_callback.wait_for_event_report(Clusters.WaterHeaterManagement.Events.BoostStarted)
        asserts.assert_equal(event_data.boostInfo.duration, 600)
        asserts.assert_equal(event_data.boostInfo.temporarySetpoint, 7000)

        self.step("21a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("21b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("22")
        await self.send_cancel_boost_command()

        event_data = events_callback.wait_for_event_report(Clusters.WaterHeaterManagement.Events.BoostEnded)

        self.step("22a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("22b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)

        self.step("23")
        await self.send_test_event_trigger_water_temperature20C_test_event()

        self.step("23a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("24")
        await self.send_boost_command(duration=200, emergency_boost=True)

        event_data = events_callback.wait_for_event_report(Clusters.WaterHeaterManagement.Events.BoostStarted)
        asserts.assert_equal(event_data.boostInfo.duration, 200)
        asserts.assert_equal(event_data.boostInfo.emergencyBoost, True)
        asserts.assert_is_none(event_data.boostInfo.temporarySetpoint)

        self.step("24a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("24b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("25")
        await self.send_cancel_boost_command()

        event_data = events_callback.wait_for_event_report(Clusters.WaterHeaterManagement.Events.BoostEnded)

        self.step("25a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("25b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)

        self.step("26")
        await self.send_cancel_boost_command()

        event_data = events_callback.wait_for_event_expect_no_report()

        self.step("27")
        await self.send_test_event_trigger_basic_installation_test_event_clear()


if __name__ == "__main__":
    default_matter_test_main()
