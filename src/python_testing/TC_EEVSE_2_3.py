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
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --application evse
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
from datetime import datetime, timedelta, timezone

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from matter_testing_support import EventChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_EEVSE_Utils import EEVSEBaseTestHelper

logger = logging.getLogger(__name__)


class TC_EEVSE_2_3(MatterBaseTest, EEVSEBaseTestHelper):

    def desc_TC_EEVSE_2_3(self) -> str:
        """Returns a description of this test"""
        return "5.1.4. [TC-EEVSE-2.3] Optional ChargingPreferences feature functionality with DUT as Server\n" \
            "This test case verifies the primary functionality of the Energy EVSE cluster server with the optional ChargingPreferences feature supported."

    def pics_TC_EEVSE_2_3(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["EEVSE.S", "EEVSE.S.F00"]

    def steps_TC_EEVSE_2_3(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)",
                     is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for Basic Functionality Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EVSE TimeOfUse Mode Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("5", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Plugged-in Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E00(EVConnected) sent"),
            TestStep("6", "TH sends command ClearTargets",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("6a", "TH reads from the DUT the NextChargeStartTime",
                     "Value has to be null."),
            TestStep("6b", "TH reads from the DUT the NextChargeTargetTime",
                     "Value has to be null."),
            TestStep("6c", "TH reads from the DUT the NextChargeRequiredEnergy",
                     "Value has to be null."),
            TestStep("6d", "TH reads from the DUT the NextChargeTargetSoC",
                     "Value has to be null."),
            TestStep("7", "TH sends command GetTargets",
                     "Response EEVSE.S.C00.Tx(GetTargetsResponse) sent with no targets defined."),
            TestStep("8", "TH sends command SetTargets with DayOfTheWeekforSequence=0x7F (i.e. having all days set) and a single ChargingTargets={TargetTime=1439, TargetSoC=null, AddedEnergy=25000000}",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("8a", "TH reads from the DUT the NextChargeStartTime",
                     "Value has to be null."),
            TestStep("8b", "TH reads from the DUT the NextChargeTargetTime",
                     "Value has to be null."),
            TestStep("8c", "TH reads from the DUT the NextChargeRequiredEnergy",
                     "Value has to be null."),
            TestStep("8d", "TH reads from the DUT the NextChargeTargetSoC",
                     "Value has to be null."),
            TestStep("9", "TH sends command EnableCharging with ChargingEnabledUntil=null, minimumChargeCurrent=6000, maximumChargeCurrent=60000",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("9a", "TH reads from the DUT the NextChargeStartTime",
                     "Value has to be before the next TargetTime above."),
            TestStep("9b", "TH reads from the DUT the NextChargeTargetTime",
                     "Value has to be TargetTime above."),
            TestStep("9c", "TH reads from the DUT the NextChargeRequiredEnergy",
                     "Value has to be AddedEnergy above."),
            TestStep("9d", "TH reads from the DUT the NextChargeTargetSoC",
                     "Value has to be null."),
            TestStep("10", "TH sends command GetTargets",
                     "Response EEVSE.S.C00.Tx(GetTargetsResponse) sent with targets equivalent to the above (Note 1)."),
            TestStep("11", "TH sends command SetTargets with DayOfTheWeekforSequence=0x7F (i.e. having all days set) and a single ChargingTargets={TargetTime=1, TargetSoC=100, AddedEnergy=null}",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("11a", "TH reads from the DUT the NextChargeStartTime",
                     "Value has to be before the next TargetTime above."),
            TestStep("11b", "TH reads from the DUT the NextChargeTargetTime",
                     "Value has to be TargetTime above."),
            TestStep("11c", "TH reads from the DUT the NextChargeRequiredEnergy",
                     "Value has to be null."),
            TestStep("11d", "TH reads from the DUT the NextChargeTargetSoC",
                     "Value has to be 100."),
            TestStep("12", "TH sends command GetTargets",
                     "Response EEVSE.S.C00.Tx(GetTargetsResponse) sent with targets equivalent to the above (Note 1)."),
            TestStep("13", "TH sends command SetTargets with DayOfTheWeekforSequence=0x40 (i.e. having Saturday set) and 10 ChargingTargets with TargetTimes=60,180,300,420,540,660,780,900,1020,1140 and all with TargetSoC=null, AddedEnergy=2500000",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("14", "TH sends command SetTargets with DayOfTheWeekforSequence=0x01 (i.e. having Sunday set) and no ChargingTargets",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("15", "TH sends command GetTargets",
                     "Response EEVSE.S.C00.Tx(GetTargetsResponse) sent with 1 target for each day Monday to Friday equivalent to step 9 (Note 1), 10 targets for Saturday as step 11, and no targets for Sunday."),
            TestStep("16", "TH sends command ClearTargets",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("16a", "TH reads from the DUT the NextChargeStartTime",
                     "Value has to be null."),
            TestStep("16b", "TH reads from the DUT the NextChargeTargetTime",
                     "Value has to be null."),
            TestStep("16c", "TH reads from the DUT the NextChargeRequiredEnergy",
                     "Value has to be null."),
            TestStep("16d", "TH reads from the DUT the NextChargeTargetSoC",
                     "Value has to be null."),
            TestStep("17", "TH sends command GetTargets",
                     "Response EEVSE.S.C00.Tx(GetTargetsResponse) sent with no targets defined."),
            TestStep("18", "TH sends command SetTargets with two identical ChargingTargetSchedules={DayOfTheWeekforSequence=0x01,ChargingTarget[0]={TargetTime=60,TargetSoC=null,AddedEnergy=2500000}}",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("19", "TH sends command SetTargets with DayOfTheWeekforSequence=0x40 and 11 ChargingTargets with TargetTimes=60,180,300,420,540,660,780,900,1020,1140,1260 and all with TargetSoC=null, AddedEnergy=2500000",
                     "Verify DUT responds w/ status RESOURCE_EXHAUSTED(0x89)"),
            TestStep("20", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Plugged-in Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E01(EVNotDetected) sent"),
            TestStep("21", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for Basic Functionality Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("22", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EVSE TimeOfUse Mode Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
        ]

        return steps

    def log_get_targets_response(self, get_targets_response):
        logger.info(f" Rx'd: {get_targets_response}")
        for index, entry in enumerate(get_targets_response.chargingTargetSchedules):
            logger.info(
                f"   [{index}] DayOfWeekForSequence: {entry.dayOfWeekForSequence:02x}")
            for sub_index, sub_entry in enumerate(entry.chargingTargets):
                logger.info(
                    f"    - [{sub_index}] TargetTime: {sub_entry.targetTimeMinutesPastMidnight} TargetSoC: {sub_entry.targetSoC} AddedEnergy: {sub_entry.addedEnergy}")

    def convert_epoch_s_to_time(self, epoch_s, tz=timezone.utc):
        delta_from_epoch = timedelta(seconds=epoch_s)
        matter_epoch = datetime(2000, 1, 1, 0, 0, 0, 0, tz)

        return matter_epoch + delta_from_epoch

    def compute_expected_target_time_as_epoch_s(self, minutes_past_midnight):
        """Takes minutes past midnight and assumes local timezone, returns the value in Matter Epoch_S"""
        # Matter epoch is 0 hours, 0 minutes, 0 seconds on Jan 1, 2000 UTC
        # Get the current midnight + minutesPastMidnight as epoch_s
        # NOTE that MinutesPastMidnight is in LOCAL time not UTC so it reflects
        # the charging time based on where the consumer is.
        target_time = datetime.now()     # Get time in local time
        target_time = target_time.replace(hour=int(minutes_past_midnight / 60),
                                          minute=(minutes_past_midnight % 60), second=0,
                                          microsecond=0)  # Align to minutes past midnight

        if (target_time < datetime.now()):
            # This is in the past - so we need to add 1 day
            # We can get away with this in this test scenario - but should
            # really look at the next target on this day to see if that is in the future
            target_time = target_time + timedelta(days=1)

        # Shift to UTC so we can use timezone aware subtraction from Matter epoch in UTC
        target_time = target_time.astimezone(timezone.utc)

        logger.info(
            f"minutesPastMidnight = {minutes_past_midnight} => "
            f"{int(minutes_past_midnight/60)}:{int(minutes_past_midnight%60)}"
            f" Expected target_time = {target_time}")

        target_time_delta = target_time - \
            datetime(2000, 1, 1, 0, 0, 0, 0).astimezone(timezone.utc)
        expected_target_time_epoch_s = int(target_time_delta.total_seconds())
        return expected_target_time_epoch_s

    @async_test_body
    async def test_TC_EEVSE_2_3(self):

        self.step("1")
        # Commission DUT - already done

        # Subscribe to Events and when they are sent push them to a queue for checking later
        events_callback = EventChangeCallback(Clusters.EnergyEvse)
        await events_callback.start(self.default_controller,
                                    self.dut_node_id,
                                    self.matter_test_config.endpoint)

        self.step("2")
        await self.check_test_event_triggers_enabled()

        self.step("3")
        await self.send_test_event_trigger_basic()

        self.step("4")
        await self.send_test_event_trigger_time_of_use_mode()

        self.step("5")
        await self.send_test_event_trigger_pluggedin()
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EVConnected)
        session_id = event_data.sessionID

        self.step("6")
        await self.send_clear_targets_command()

        self.step("6a")
        await self.check_evse_attribute("NextChargeStartTime", NullValue)

        self.step("6b")
        await self.check_evse_attribute("NextChargeTargetTime", NullValue)

        self.step("6c")
        await self.check_evse_attribute("NextChargeRequiredEnergy", NullValue)

        self.step("6d")
        await self.check_evse_attribute("NextChargeTargetSoC", NullValue)

        self.step("7")
        get_targets_response = await self.send_get_targets_command()
        self.log_get_targets_response(get_targets_response)
        empty_targets_response = Clusters.EnergyEvse.Commands.GetTargetsResponse(
            chargingTargetSchedules=[])
        asserts.assert_equal(get_targets_response, empty_targets_response,
                             f"Unexpected 'GetTargets' response value - expected {empty_targets_response}, was {get_targets_response}")

        self.step("8")
        # The targets is a list of up to 7x ChargingTargetScheduleStruct's (one per day)
        # each containing a list of up to 10x targets per day
        minutes_past_midnight = 1439
        dailyTargets = [Clusters.EnergyEvse.Structs.ChargingTargetStruct(targetTimeMinutesPastMidnight=minutes_past_midnight,
                                                                         # targetSoc not sent
                                                                         addedEnergy=25000000)]
        targets = [Clusters.EnergyEvse.Structs.ChargingTargetScheduleStruct(
            dayOfWeekForSequence=0x7F, chargingTargets=dailyTargets)]
        # This should be all days Sun-Sat (0x7F) with an TargetTime 1439 and added Energy 25kWh
        await self.send_set_targets_command(chargingTargetSchedules=targets)

        self.step("8a")
        await self.check_evse_attribute("NextChargeStartTime", NullValue)

        self.step("8b")
        await self.check_evse_attribute("NextChargeTargetTime", NullValue)

        self.step("8c")
        await self.check_evse_attribute("NextChargeRequiredEnergy", NullValue)

        self.step("8d")
        await self.check_evse_attribute("NextChargeTargetSoC", NullValue)

        self.step("9")
        await self.send_enable_charge_command(charge_until=NullValue, min_charge=6000, max_charge=60000)

        self.step("9a")
        next_start_time_epoch_s = await self.read_evse_attribute_expect_success(attribute="NextChargeStartTime")

        expected_next_start_time_epoch_s = self.compute_expected_target_time_as_epoch_s(
            minutes_past_midnight)
        asserts.assert_less(next_start_time_epoch_s,
                            expected_next_start_time_epoch_s)

        self.step("9b")
        await self.check_evse_attribute("NextChargeTargetTime", expected_next_start_time_epoch_s)

        self.step("9c")
        await self.check_evse_attribute("NextChargeRequiredEnergy", 25000000)

        self.step("9d")
        await self.check_evse_attribute("NextChargeTargetSoC", NullValue)

        self.step("10")
        get_targets_response = await self.send_get_targets_command()
        self.log_get_targets_response(get_targets_response)
        asserts.assert_equal(get_targets_response.chargingTargetSchedules, targets,
                             f"Unexpected 'GetTargets' response value - expected {targets}, was {get_targets_response}")

        self.step("11")
        # This should be all days Sun-Sat (0x7F) with an TargetTime 1 and SoC of 100%, AddedEnergy= NullValue
        minutes_past_midnight = 1
        daily_targets_step_11 = [Clusters.EnergyEvse.Structs.ChargingTargetStruct(targetTimeMinutesPastMidnight=minutes_past_midnight,
                                                                                  targetSoC=100)]
        targets_step_11 = [Clusters.EnergyEvse.Structs.ChargingTargetScheduleStruct(
            dayOfWeekForSequence=0x7F, chargingTargets=daily_targets_step_11)]

        await self.send_set_targets_command(chargingTargetSchedules=targets_step_11)

        self.step("11a")
        next_start_time_epoch_s = await self.read_evse_attribute_expect_success(attribute="NextChargeStartTime")
        logger.info(
            f"Received NextChargeStartTime: {next_start_time_epoch_s} = {self.convert_epoch_s_to_time(next_start_time_epoch_s, tz=None)}")

        self.step("11b")
        next_target_time_epoch_s = await self.read_evse_attribute_expect_success(attribute="NextChargeTargetTime")
        logger.info(
            f"Received NextChargeTargetTime: {next_target_time_epoch_s} = {self.convert_epoch_s_to_time(next_target_time_epoch_s, tz=None)}")

        # This should be the next MinutesPastMidnight converted to realtime as epoch_s
        expected_target_time_epoch_s = self.compute_expected_target_time_as_epoch_s(
            minutes_past_midnight)

        asserts.assert_less(next_start_time_epoch_s, next_target_time_epoch_s,
                            f"Unexpected 'NextChargeStartTime' response value - expected this to be < {next_target_time_epoch_s}, was {next_start_time_epoch_s}")

        asserts.assert_equal(next_target_time_epoch_s, expected_target_time_epoch_s,
                             f"Unexpected 'NextChargeTargetTime' response value - expected {expected_target_time_epoch_s} = {self.convert_epoch_s_to_time(expected_target_time_epoch_s, tz=None)}, was {next_target_time_epoch_s} = {self.convert_epoch_s_to_time(next_target_time_epoch_s, tz=None)}")

        self.step("11c")
        await self.check_evse_attribute("NextChargeRequiredEnergy", NullValue)

        self.step("11d")
        await self.check_evse_attribute("NextChargeTargetSoC", 100)

        self.step("12")
        get_targets_response = await self.send_get_targets_command()
        self.log_get_targets_response(get_targets_response)
        # This should be the same as targets_step_11
        asserts.assert_equal(get_targets_response.chargingTargetSchedules, targets_step_11,
                             f"Unexpected 'GetTargets' response value - expected {targets_step_11}, was {get_targets_response}")

        self.step("13")
        # This should modify Sat (0x40) with 10 targets throughout the day
        daily_targets_step_13 = [
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=60, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=180, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=300, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=420, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=540, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=660, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=780, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=900, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=1020, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=1140, addedEnergy=25000000),
        ]
        targets_step_13 = [Clusters.EnergyEvse.Structs.ChargingTargetScheduleStruct(
            dayOfWeekForSequence=0x40, chargingTargets=daily_targets_step_13)]
        await self.send_set_targets_command(chargingTargetSchedules=targets_step_13)

        self.step("14")
        # This should modify Sun (0x01) with NO targets on that day
        daily_targets_step_14 = []
        targets_step_14 = [Clusters.EnergyEvse.Structs.ChargingTargetScheduleStruct(
            dayOfWeekForSequence=0x01, chargingTargets=daily_targets_step_14)]
        await self.send_set_targets_command(chargingTargetSchedules=targets_step_14)

        self.step("15")
        get_targets_response = await self.send_get_targets_command()
        self.log_get_targets_response(get_targets_response)
        # We should expect that there should be 3 entries:
        # [0] This should be all days (except Sun & Sat) = 0x3e with an TargetTime 1439 and added Energy 25kWh (from step 9)
        # [1] This should be (Sat) = 0x40 with 10 TargetTimes and added Energy 25kWh (from step 11)
        # [2] This should be (Sun) = 0x01 with NO Targets (from step 12)
        # TODO - it would be better to iterate through each day and check it matches
        asserts.assert_equal(len(get_targets_response.chargingTargetSchedules), 3,
                             "'GetTargets' response should have 3 entries")
        asserts.assert_equal(get_targets_response.chargingTargetSchedules[0].dayOfWeekForSequence, 0x3e,
                             "'GetTargets' response entry 0 should have DayOfWeekForSequence = 0x3e (62)")
        asserts.assert_equal(get_targets_response.chargingTargetSchedules[0].chargingTargets, daily_targets_step_11,
                             f"'GetTargets' response entry 0 should have chargingTargets = {daily_targets_step_11})")
        asserts.assert_equal(get_targets_response.chargingTargetSchedules[1], targets_step_13[0],
                             f"'GetTargets' response entry 1 should be {targets_step_13})")
        asserts.assert_equal(get_targets_response.chargingTargetSchedules[2], targets_step_14[0],
                             f"'GetTargets' response entry 2 should be {targets_step_14})")

        self.step("16")
        await self.send_clear_targets_command()

        self.step("16a")
        await self.check_evse_attribute("NextChargeStartTime", NullValue)

        self.step("16b")
        await self.check_evse_attribute("NextChargeTargetTime", NullValue)

        self.step("16c")
        await self.check_evse_attribute("NextChargeRequiredEnergy", NullValue)

        self.step("16d")
        await self.check_evse_attribute("NextChargeTargetSoC", NullValue)

        self.step("17")
        get_targets_response = await self.send_get_targets_command()
        self.log_get_targets_response(get_targets_response)
        asserts.assert_equal(get_targets_response, empty_targets_response,
                             f"Unexpected 'GetTargets' response value - expected {empty_targets_response}, was {get_targets_response}")

        self.step("18")
        daily_targets_step_18 = [Clusters.EnergyEvse.Structs.ChargingTargetStruct(
            targetTimeMinutesPastMidnight=60, addedEnergy=25000000)]
        targets_step_18 = [Clusters.EnergyEvse.Structs.ChargingTargetScheduleStruct(dayOfWeekForSequence=0x1, chargingTargets=daily_targets_step_18),
                           Clusters.EnergyEvse.Structs.ChargingTargetScheduleStruct(dayOfWeekForSequence=0x1, chargingTargets=daily_targets_step_18)]
        await self.send_set_targets_command(chargingTargetSchedules=targets_step_18, expected_status=Status.ConstraintError)

        self.step("19")
        daily_targets_step_19 = [
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=60, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=180, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=300, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=420, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=540, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=660, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=780, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=900, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=1020, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=1140, addedEnergy=25000000),
            Clusters.EnergyEvse.Structs.ChargingTargetStruct(
                targetTimeMinutesPastMidnight=1260, addedEnergy=25000000),
        ]

        targets_step_19 = [Clusters.EnergyEvse.Structs.ChargingTargetScheduleStruct(
            dayOfWeekForSequence=0x40, chargingTargets=daily_targets_step_19)]
        await self.send_set_targets_command(chargingTargetSchedules=targets_step_19, expected_status=Status.ResourceExhausted)

        self.step("20")
        await self.send_test_event_trigger_pluggedin_clear()
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EVNotDetected)
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kPluggedInNoDemand
        self.validate_ev_not_detected_event(
            event_data, session_id, expected_state, expected_duration=0, expected_charged=0)

        self.step("21")
        await self.send_test_event_trigger_basic_clear()

        self.step("22")
        await self.send_test_event_trigger_time_of_use_mode_clear()


if __name__ == "__main__":
    default_matter_test_main()
