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
from matter_testing_support import EventChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_EWATERHTRBase import EWATERHTRBase

logger = logging.getLogger(__name__)


class TC_EWATERHTR_2_3(MatterBaseTest, EWATERHTRBase):

    def desc_TC_EWATERHTR_2_3(self) -> str:
        """Returns a description of this test"""
        return "[TC-EWATERHTR-2.3] This test case verifies the functionality of the Water Heater Management cluster server with the TankPercentage feature."

    def pics_TC_EWATERHTR_2_3(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["EWATERHTR.S", "EWATERHTR.S.F01"]

    def steps_TC_EWATERHTR_2_3(self) -> list[TestStep]:
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
            TestStep("4c", "TH reads from the DUT the TankPercentage",
                     "Value has to be 0%"),
            TestStep("4d", "TH reads from the DUT the HeaterTypes",
                     "Value has to be greater than 0x00 (at least one type supported) and Store the value as HeaterTypes"),
            TestStep("5", "TH sends command Boost with Duration=600s,TargetPercentage=100%",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EWATERHTR.S.E00(BoostStarted) sent with Duration=600 and TargetPercentage=100"),
            TestStep("5a", "TH reads from the DUT the HeatDemand",
                     "Value has to be greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("5b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Water Temperature 61C Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("6a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("6b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("6c", "TH reads from the DUT the TankPercentage",
                     "Value has to be 100%"),
            TestStep("7", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Draw off hot water Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("7a", "TH reads from the DUT the HeatDemand",
                     "Value has to be greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("7b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("7c", "TH reads from the DUT the TankPercentage",
                     "Value has to be 76%"),
            TestStep("8", "TH sends command CancelBoost",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EWATERHTR.S.E00(BoostEnded) sent"),
            TestStep("8a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("8b", "TH reads from the DUT the BoostState",
                     "Value has to be 0 (Inactive)"),
            TestStep("8c", "TH reads from the DUT the TankPercentage",
                     "Value has to be 76%"),
            TestStep("9", "TH sends command Boost with Duration=400s,TargetPercentage=100%,TargetReheat=65%",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EWATERHTR.S.E00(BoostStarted) sent with Duration=400 and TargetPercentage=100 and TargetReheat=65"),
            TestStep("9a", "TH reads from the DUT the HeatDemand",
                     "Value has to be greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("9b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("9c", "TH reads from the DUT the TankPercentage",
                     "Value has to be 76%"),
            TestStep("10", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Water Temperature 61C Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("10a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("10b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("10c", "TH reads from the DUT the TankPercentage",
                     "Value has to be 100%"),
            TestStep("11", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Draw off hot water Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("11a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("11b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("11c", "TH reads from the DUT the TankPercentage",
                     "Value has to be 76%"),
            TestStep("12", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Draw off hot water Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("12a", "TH reads from the DUT the HeatDemand",
                     "Value has to be greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("12b", "TH reads from the DUT the BoostState",
                     "Value has to be 1 (Active)"),
            TestStep("12c", "TH reads from the DUT the TankPercentage",
                     "Value has to be 57%"),
            TestStep("13", "TH sends command CancelBoost",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EWATERHTR.S.E00(BoostEnded) sent"),
            TestStep("13a", "TH reads from the DUT the HeatDemand",
                     "Value has to be 0x00 (no demand on any source)"),
            TestStep("13b", "TH reads from the DUT the BoostState",
                     "Value has to be 0 (Inactive)"),
            TestStep("13c", "TH reads from the DUT the TankPercentage",
                     "Value has to be 57%"),
            TestStep("14", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EWATERHTR.TESTEVENTTRIGGER for Basic installation Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
        ]

        return steps

    @async_test_body
    async def test_TC_EWATERHTR_2_3(self):

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
        await self.check_whm_attribute("TankPercentage", 0)

        self.step("4d")
        heaterTypes = await self.read_whm_attribute_expect_success(attribute="HeaterTypes")
        asserts.assert_greater(heaterTypes, 0)

        self.step("5")
        await self.send_boost_command(duration=600, target_percentage=100)

        event_data = events_callback.wait_for_event_report(Clusters.WaterHeaterManagement.Events.BoostStarted)
        asserts.assert_equal(event_data.boostInfo.duration, 600)
        asserts.assert_equal(event_data.boostInfo.targetPercentage, 100)

        self.step("5a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("5b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("6")
        await self.send_test_event_trigger_water_temperature61C_test_event()

        self.step("6a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_equal(heatDemand, 0)

        self.step("6b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("6c")
        await self.check_whm_attribute("TankPercentage", 100)

        self.step("7")
        await self.send_test_event_trigger_draw_off_hot_water_test_event()

        self.step("7a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("7b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("7c")
        await self.check_whm_attribute("TankPercentage", 76)

        self.step("8")
        await self.send_cancel_boost_command()

        event_data = events_callback.wait_for_event_report(Clusters.WaterHeaterManagement.Events.BoostEnded)

        self.step("8a")
        await self.check_whm_attribute("HeatDemand", 0)

        self.step("8b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)

        self.step("8c")
        await self.check_whm_attribute("TankPercentage", 76)

        self.step("9")
        await self.send_boost_command(duration=400, target_percentage=100, target_reheat=65)

        event_data = events_callback.wait_for_event_report(Clusters.WaterHeaterManagement.Events.BoostStarted)
        asserts.assert_equal(event_data.boostInfo.duration, 400)
        asserts.assert_equal(event_data.boostInfo.targetPercentage, 100)
        asserts.assert_equal(event_data.boostInfo.targetReheat, 65)

        self.step("9a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("9b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("9c")
        await self.check_whm_attribute("TankPercentage", 76)

        self.step("10")
        await self.send_test_event_trigger_water_temperature61C_test_event()

        self.step("10a")
        await self.check_whm_attribute("HeatDemand", 0)

        self.step("10b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("10c")
        await self.check_whm_attribute("TankPercentage", 100)

        self.step("11")
        await self.send_test_event_trigger_draw_off_hot_water_test_event()

        self.step("11a")
        await self.check_whm_attribute("HeatDemand", 0)

        self.step("11b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("11c")
        await self.check_whm_attribute("TankPercentage", 76)

        self.step("12")
        await self.send_test_event_trigger_draw_off_hot_water_test_event()

        self.step("12a")
        heatDemand = await self.read_whm_attribute_expect_success(attribute="HeatDemand")
        asserts.assert_greater(heatDemand, 0)
        asserts.assert_equal(heatDemand & (~heaterTypes), 0, "heatDemand should only be from declared supported types"),

        self.step("12b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kActive)

        self.step("12c")
        await self.check_whm_attribute("TankPercentage", 57)

        self.step("13")
        await self.send_cancel_boost_command()

        event_data = events_callback.wait_for_event_report(Clusters.WaterHeaterManagement.Events.BoostEnded)

        self.step("13a")
        await self.check_whm_attribute("HeatDemand", 0)

        self.step("13b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)

        self.step("13c")
        await self.check_whm_attribute("TankPercentage", 57)

        self.step("14")
        await self.send_test_event_trigger_basic_installation_test_event_clear()


if __name__ == "__main__":
    default_matter_test_main()
