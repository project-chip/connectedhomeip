#
#    Copyright (c) 2025 Project CHIP Authors
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
# test-runner-runs:
#   run1:
#     app: ${ENERGY_MANAGEMENT_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#       --application evse
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.event_attribute_reporting import EventChangeCallback
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts
from TC_EEVSE_Utils import EEVSEBaseTestHelper

logger = logging.getLogger(__name__)
cluster = Clusters.EnergyEvse


class TC_EEVSE_2_7(MatterBaseTest, EEVSEBaseTestHelper):

    """This test case verifies the primary functionality of the Energy EVSE Cluster server
     with the optional SoCReporting feature supported. This test case can also verify the
     interaction between SoCReporting and ChargingPreferences features if ChargingPreferences
     is also supported."""

    def desc_TC_EEVSE_2_7(self) -> str:
        """Returns a description of this test"""
        return "[TC-EEVSE-2.7] Optional SoCReporting feature functionality with DUT as Server"

    def pics_TC_EEVSE_2_7(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["EEVSE.S.F01"]

    def steps_TC_EEVSE_2_7(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)",
                     is_commissioning=True),
            TestStep("2", "TH reads from the DUT the FeatureMap",
                     "Verify that the DUT response contains the FeatureMap attribute. Store the value as FeatureMap."),
            TestStep("3", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for Basic Functionality Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("5", "TH reads from the DUT the State",
                     "Value has to be 0x00 (NotPluggedIn)"),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Plugged-in Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EVConnected sent"),
            TestStep("7", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EVSE Set SoC Low Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("7a", "TH reads from the DUT the StateOfCharge",
                     "Value has to be 20% state of charge."),
            TestStep("7b", "TH reads from the DUT the BatteryCapacity",
                     "Value has to be 70,000,000 (70kWh)."),
            TestStep("8", "If ChargingPreferences feature is not supported, skip all remaining test steps."),
            TestStep("9", "TH sends command ClearTargets",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("9a", "TH reads from the DUT the NextChargeStartTime",
                     "Value has to be null."),
            TestStep("9b", "TH reads from the DUT the NextChargeTargetTime",
                     "Value has to be null."),
            TestStep("9c", "TH reads from the DUT the NextChargeRequiredEnergy",
                     "Value has to be null."),
            TestStep("9d", "TH reads from the DUT the NextChargeTargetSoC",
                     "Value has to be null."),
            TestStep("10", "TH sends command SetTargets with DayOfTheWeekforSequence=0x7F (i.e. having all days set) and a single ChargingTargets={TargetTimeMinutesPastMidnight=1439, TargetSoC=80, AddedEnergy=25000000}. Store TargetTimeMinutesPastMidnight converted from local timezone into Matter EPOCH in UTC as TargetTime, TargetSoC as TargetSoC, and AddedEnergy as AddedEnergy.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("10a", "TH reads from the DUT the NextChargeStartTime",
                     "Value has to be null."),
            TestStep("10b", "TH reads from the DUT the NextChargeTargetTime",
                     "Value has to be null."),
            TestStep("10c", "TH reads from the DUT the NextChargeRequiredEnergy",
                     "Value has to be null."),
            TestStep("10d", "TH reads from the DUT the NextChargeTargetSoC",
                     "Value has to be null."),
            TestStep("11", "TH sends command EnableCharging with ChargingEnabledUntil=null, minimumChargeCurrent=6000, maximumChargeCurrent=60000",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("11a", "TH reads from the DUT the NextChargeStartTime",
                     "Value has to be before the next TargetTime above."),
            TestStep("11b", "TH reads from the DUT the NextChargeTargetTime",
                     "Value has to be TargetTime above."),
            TestStep("11c", "TH reads from the DUT the NextChargeRequiredEnergy",
                     "Value has to be null."),
            TestStep("11d", "TH reads from the DUT the NextChargeTargetSoC",
                     "Value has to be TargetSoC above."),
            TestStep("12", "TH sends command Disable",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("12a", "TH reads from the DUT the NextChargeStartTime",
                     "Value has to be null."),
            TestStep("12b", "TH reads from the DUT the NextChargeTargetTime",
                     "Value has to be null."),
            TestStep("12c", "TH reads from the DUT the NextChargeRequiredEnergy",
                     "Value has to be null."),
            TestStep("12d", "TH reads from the DUT the NextChargeTargetSoC",
                     "Value has to be null."),
            TestStep("13", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EVSE Set SoC High Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("13a", "TH reads from the DUT the StateOfCharge",
                     "Value has to be 95% state of charge."),
            TestStep("13b", "TH reads from the DUT the BatteryCapacity",
                     "Value has to be 70,000,000 (70kWh)."),
            TestStep("14", "TH sends command EnableCharging with ChargingEnabledUntil=null, minimumChargeCurrent=6000, maximumChargeCurrent=60000",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("14a", "TH reads from the DUT the NextChargeStartTime",
                     "Value has to be null."),
            TestStep("14b", "TH reads from the DUT the NextChargeTargetTime",
                     "Value has to be TargetTime above."),
            TestStep("14c", "TH reads from the DUT the NextChargeRequiredEnergy",
                     "Value has to be null."),
            TestStep("14d", "TH reads from the DUT the NextChargeTargetSoC",
                     "Value has to be TargetSoC above."),
            TestStep("15", "TH sends command Disable",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("15a", "TH reads from the DUT the NextChargeStartTime",
                     "Value has to be null."),
            TestStep("15b", "TH reads from the DUT the NextChargeTargetTime",
                     "Value has to be null."),
            TestStep("15c", "TH reads from the DUT the NextChargeRequiredEnergy",
                     "Value has to be null."),
            TestStep("15d", "TH reads from the DUT the NextChargeTargetSoC",
                     "Value has to be null."),
            TestStep("16", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EVSE Set SoC Clear Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("16a", "TH reads from the DUT the StateOfCharge",
                     "Value has to be null."),
            TestStep("16b", "TH reads from the DUT the BatteryCapacity",
                     "Value has to be null."),
            TestStep("17", "TH sends command EnableCharging with ChargingEnabledUntil=null, minimumChargeCurrent=6000, maximumChargeCurrent=60000",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("17a", "TH reads from the DUT the NextChargeStartTime",
                     "Value has to be before the next TargetTime above."),
            TestStep("17b", "TH reads from the DUT the NextChargeTargetTime",
                     "Value has to be TargetTime above."),
            TestStep("17c", "TH reads from the DUT the NextChargeRequiredEnergy",
                     "Value has to be AddedEnergy above."),
            TestStep("17d", "TH reads from the DUT the NextChargeTargetSoC",
                     "Value has to be null."),

        ]

        return steps

    @run_if_endpoint_matches(has_feature(cluster, cluster.Bitmaps.Feature.kSoCReporting))
    async def test_TC_EEVSE_2_7(self):
        endpoint = self.get_endpoint()

        self.step("1")
        # Commission DUT - already done

        # Subscribe to Events and when they are sent push them to a queue for checking later
        events_callback = EventChangeCallback(Clusters.EnergyEvse)
        await events_callback.start(self.default_controller,
                                    self.dut_node_id,
                                    self.get_endpoint())

        self.step("2")
        feature_map = await self.read_evse_attribute_expect_success(attribute="FeatureMap")
        soc_reporting_supported = (feature_map & Clusters.EnergyEvse.Bitmaps.Feature.kSoCReporting) > 0
        charging_preferences_supported = (feature_map & Clusters.EnergyEvse.Bitmaps.Feature.kChargingPreferences) > 0

        logger.info(
            f"Received FeatureMap: {feature_map:#x} = SoCReporting ({soc_reporting_supported}), ChargingPreferences ({charging_preferences_supported})")

        self.step("3")
        await self.check_test_event_triggers_enabled()

        self.step("4")
        await self.send_test_event_trigger_basic()

        self.step("5")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kNotPluggedIn)

        self.step("6")
        await self.send_test_event_trigger_pluggedin()
        events_callback.wait_for_event_report(Clusters.EnergyEvse.Events.EVConnected)

        self.step("7")
        await self.send_test_event_trigger_evse_set_soc_low()

        self.step("7a")
        await self.check_evse_attribute("StateOfCharge", 20)

        self.step("7b")
        await self.check_evse_attribute("BatteryCapacity", 70000000)

        self.step("8")
        has_pref = self.feature_guard(endpoint=endpoint, cluster=cluster,
                                      feature_int=Clusters.EnergyEvse.Bitmaps.Feature.kChargingPreferences)

        if not has_pref:
            self.mark_all_remaining_steps_skipped("9")
        else:
            self.step("9")
            # TH sends command ClearTargets
            await self.send_clear_targets_command()

            self.step("9a")
            # TH reads from the DUT the NextChargeStartTime
            await self.check_evse_attribute("NextChargeStartTime", NullValue)

            self.step("9b")
            # TH reads from the DUT the NextChargeTargetTime
            await self.check_evse_attribute("NextChargeTargetTime", NullValue)

            self.step("9c")
            # TH reads from the DUT the NextChargeRequiredEnergy
            await self.check_evse_attribute("NextChargeRequiredEnergy", NullValue)

            self.step("9d")
            # TH reads from the DUT the NextChargeTargetSoC
            await self.check_evse_attribute("NextChargeTargetSoC", NullValue)

            self.step("10")
            # TH sends command SetTargets with DayOfTheWeekforSequence=0x7F (i.e. having all days set) and a single ChargingTargets={TargetTime=1439, TargetSoC=80, AddedEnergy=25000000}
            # The targets is a list of up to 7x ChargingTargetScheduleStruct's (one per day)
            # each containing a list of up to 10x targets per day
            minutes_past_midnight = 1439
            target_soc = 80
            added_energy = 25000000
            dailyTargets = [Clusters.EnergyEvse.Structs.ChargingTargetStruct(targetTimeMinutesPastMidnight=minutes_past_midnight,
                                                                             targetSoC=target_soc,
                                                                             addedEnergy=added_energy)]
            targets = [Clusters.EnergyEvse.Structs.ChargingTargetScheduleStruct(
                dayOfWeekForSequence=0x7F, chargingTargets=dailyTargets)]

            # This should be all days Sun-Sat (0x7F) with an TargetTime 1439 and added Energy 25kWh or targetSoc=80
            await self.send_set_targets_command(chargingTargetSchedules=targets)

            self.step("10a")
            # TH reads from the DUT the NextChargeStartTime
            await self.check_evse_attribute("NextChargeStartTime", NullValue)

            self.step("10b")
            # TH reads from the DUT the NextChargeTargetTime
            await self.check_evse_attribute("NextChargeTargetTime", NullValue)

            self.step("10c")
            # TH reads from the DUT the NextChargeRequiredEnergy
            await self.check_evse_attribute("NextChargeRequiredEnergy", NullValue)

            self.step("10d")
            # TH reads from the DUT the NextChargeTargetSoC
            await self.check_evse_attribute("NextChargeTargetSoC", NullValue)

            self.step("11")
            # TH sends command EnableCharging with ChargingEnabledUntil=null, minimumChargeCurrent=6000, maximumChargeCurrent=60000
            await self.send_enable_charge_command(charge_until=NullValue, min_charge=6000, max_charge=60000)

            self.step("11a")
            # TH reads from the DUT the NextChargeStartTime
            # Value has to be before the next TargetTime above.
            next_start_time_epoch_s = await self.read_evse_attribute_expect_success(attribute="NextChargeStartTime")
            logger.info(
                f"Received NextChargeStartTime: {next_start_time_epoch_s} = {self.convert_epoch_s_to_time(next_start_time_epoch_s, tz=None)}")

            expected_next_target_time_epoch_s = self.compute_expected_target_time_as_epoch_s(
                minutes_past_midnight)
            asserts.assert_less(next_start_time_epoch_s,
                                expected_next_target_time_epoch_s)

            self.step("11b")
            # TH reads from the DUT the NextChargeTargetTime
            # Value has to be TargetTime above.
            await self.check_evse_attribute("NextChargeTargetTime",
                                            expected_next_target_time_epoch_s)

            self.step("11c")
            # TH reads from the DUT the NextChargeRequiredEnergy
            # Value has to be null.
            await self.check_evse_attribute("NextChargeRequiredEnergy", NullValue)

            self.step("11d")
            # TH reads from the DUT the NextChargeTargetSoC
            # Value has to be TargetSoC above.
            await self.check_evse_attribute("NextChargeTargetSoC", target_soc)

            self.step("12")
            # TH sends command Disable - Verify DUT responds w/ status SUCCESS(0x00)
            await self.send_disable_command()

            self.step("12a")
            # TH reads from the DUT the NextChargeStartTime
            # Value has to be null
            await self.check_evse_attribute("NextChargeStartTime", NullValue)

            self.step("12b")
            # TH reads from the DUT the NextChargeTargetTime
            # Value has to be null
            await self.check_evse_attribute("NextChargeTargetTime", NullValue)

            self.step("12c")
            # TH reads from the DUT the NextChargeRequiredEnergy
            # Value has to be null
            await self.check_evse_attribute("NextChargeRequiredEnergy", NullValue)

            self.step("12d")
            # TH reads from the DUT the NextChargeTargetSoC
            # Value has to be null
            await self.check_evse_attribute("NextChargeTargetSoC", NullValue)

            self.step("13")
            # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EVSE Set SoC High Test Event
            await self.send_test_event_trigger_evse_set_soc_high()

            self.step("13a")
            # TH reads from the DUT the StateOfCharge
            # Value has to be 95 % state of charge
            await self.check_evse_attribute("StateOfCharge", 95)

            self.step("13b")
            # TH reads from the DUT the BatteryCapacity
            # Value has to be 70,000,000 (70kWh)
            await self.check_evse_attribute("BatteryCapacity", 70000000)

            self.step("14")
            # TH sends command EnableCharging with ChargingEnabledUntil=null, minimumChargeCurrent=6000, maximumChargeCurrent=60000
            await self.send_enable_charge_command(charge_until=NullValue, min_charge=6000, max_charge=60000)

            self.step("14a")
            # TH reads from the DUT the NextChargeStartTime
            # Value has to be null
            await self.check_evse_attribute("NextChargeStartTime", NullValue)

            self.step("14b")
            # TH reads from the DUT the NextChargeTargetTime
            # Value has to be TargetTime above
            await self.check_evse_attribute("NextChargeTargetTime", expected_next_target_time_epoch_s)

            self.step("14c")
            # TH reads from the DUT the NextChargeRequiredEnergy
            # Value has to be null
            await self.check_evse_attribute("NextChargeRequiredEnergy", NullValue)

            self.step("14d")
            # TH reads from the DUT the NextChargeTargetSoC
            # Value has to be TargetSoC above
            await self.check_evse_attribute("NextChargeTargetSoC", target_soc)

            self.step("15")
            # TH sends command Disable
            await self.send_disable_command()

            self.step("15a")
            # TH reads from the DUT the NextChargeStartTime
            # Value has to be null
            await self.check_evse_attribute("NextChargeStartTime", NullValue)

            self.step("15b")
            # TH reads from the DUT the NextChargeTargetTime
            # Value has to be null
            await self.check_evse_attribute("NextChargeTargetTime", NullValue)

            self.step("15c")
            # TH reads from the DUT the NextChargeRequiredEnergy
            # Value has to be null
            await self.check_evse_attribute("NextChargeRequiredEnergy", NullValue)

            self.step("15d")
            # TH reads from the DUT the NextChargeTargetSoC
            # Value has to be null
            await self.check_evse_attribute("NextChargeTargetSoC", NullValue)

            self.step("16")
            # TH sends TestEventTrigger command to General Diagnostics Cluster on
            # Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set
            # to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EVSE Set SoC Clear Test Event
            await self.send_test_event_trigger_evse_set_soc_clear()

            self.step("16a")
            # TH reads from the DUT the StateOfCharge
            # Value has to be null
            await self.check_evse_attribute("StateOfCharge", NullValue)

            self.step("16b")
            # TH reads from the DUT the BatteryCapacity
            # Value has to be null
            await self.check_evse_attribute("BatteryCapacity", NullValue)

            self.step("17")
            # TH sends command EnableCharging with ChargingEnabledUntil=null, minimumChargeCurrent=6000, maximumChargeCurrent=60000
            await self.send_enable_charge_command(charge_until=NullValue, min_charge=6000, max_charge=60000)

            self.step("17a")
            # TH reads from the DUT the NextChargeStartTime
            # Value has to be before the next TargetTime above
            next_start_time_epoch_s = await self.read_evse_attribute_expect_success(attribute="NextChargeStartTime")
            logger.info(
                f"Received NextChargeStartTime: {next_start_time_epoch_s} = {self.convert_epoch_s_to_time(next_start_time_epoch_s, tz=None)}")

            expected_next_target_time_epoch_s = self.compute_expected_target_time_as_epoch_s(
                minutes_past_midnight)
            asserts.assert_less(next_start_time_epoch_s,
                                expected_next_target_time_epoch_s)

            self.step("17b")
            # TH reads from the DUT the NextChargeTargetTime
            # Value has to be TargetTime above
            await self.check_evse_attribute("NextChargeTargetTime",
                                            expected_next_target_time_epoch_s)

            self.step("17c")
            # TH reads from the DUT the NextChargeRequiredEnergy
            # Value has to be AddedEnergy above
            await self.check_evse_attribute("NextChargeRequiredEnergy", added_energy)

            self.step("17d")
            # TH reads from the DUT the NextChargeTargetSoC
            # Value has to be null
            await self.check_evse_attribute("NextChargeTargetSoC", NullValue)


if __name__ == "__main__":
    default_matter_test_main()
