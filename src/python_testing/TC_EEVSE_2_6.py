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
import time

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import (ClusterAttributeChangeAccumulator, EventChangeCallback, MatterBaseTest, TestStep,
                                    async_test_body, default_matter_test_main)
from mobly import asserts
from TC_EEVSE_Utils import EEVSEBaseTestHelper

logger = logging.getLogger(__name__)


class TC_EEVSE_2_6(MatterBaseTest, EEVSEBaseTestHelper):

    def desc_TC_EEVSE_2_6(self) -> str:
        """Returns a description of this test"""
        return "5.1.6. [TC-EEVSE-2.6] Test Q quality functionality with DUT as Server"

    def pics_TC_EEVSE_2_6(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["EEVSE.S"]

    def steps_TC_EEVSE_2_6(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)",
                     is_commissioning=True),
            TestStep("2", "TH reads from the DUT the FeatureMap",
                     "Verify that the DUT response contains the FeatureMap attribute. Store the value as FeatureMap."),
            TestStep("3", "Set up a subscription to all EnergyEVSE cluster events"),
            TestStep("4", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("5", "Set up a subscription to the EnergyEVSE cluster, with MinIntervalFloor set to 0, MaxIntervalCeiling set to 10 and KeepSubscriptions set to True",
                     "Subscription successfully established"),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for Basic Functionality Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("6a", "TH reads from the DUT the State",
                     "Value has to be 0x00 (NotPluggedIn)"),
            TestStep("7", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Plugged-in Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E00(EVConnected) sent"),
            TestStep("8", "TH sends command EnableCharging with ChargingEnabledUntil=null, minimumChargeCurrent=6000, maximumChargeCurrent=12000",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("9", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Charge Demand Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E02(EnergyTransferStarted) sent"),
            TestStep("9a", "TH reads from the DUT the State",
                     "Value has to be 0x03 (PluggedInCharging)"),
            TestStep("10", "Reset all accumulated report counts, then wait 12 seconds"),
            TestStep("10a", "TH counts all report transactions with an attribute report for the SessionID attribute",
                     "TH verifies that numberOfReportsReceived = 0"),
            TestStep("10b", "TH counts all report transactions with an attribute report for the SessionDuration attribute",
                     "TH verifies that numberOfReportsReceived <= 2"),
            TestStep("10c", "TH counts all report transactions with an attribute report for the SessionEnergyCharged attribute",
                     "TH verifies that numberOfReportsReceived <= 2"),
            TestStep("10d", "TH counts all report transactions with an attribute report for the SessionEnergyDischarged attribute",
                     "TH verifies that numberOfReportsReceived <= 2"),
            TestStep("11", "Reset all accumulated report counts"),
            TestStep("12", "TH sends command Disable",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EEVSE.S.E03(EnergyTransferStopped) sent with reason EvseStopped"),
            TestStep("13", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Charge Demand Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("14", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Plugged-in Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E01(EVNotDetected) sent"),
            TestStep("15", "Wait 5 seconds"),
            TestStep("15a", "TH counts all report transactions with an attribute report for the SessionID attribute",
                     "TH verifies that numberOfReportsReceived = 0"),
            TestStep("15b", "TH counts all report transactions with an attribute report for the SessionDuration attribute",
                     "TH verifies that numberOfReportsReceived >= 1"),
            TestStep("15c", "TH counts all report transactions with an attribute report for the SessionEnergyCharged attribute",
                     "TH verifies that numberOfReportsReceived >= 1"),
            TestStep("15d", "If V2X feature is supported on the cluster, TH counts all report transactions with an attribute report for the SessionEnergyDischarged attribute",
                     "TH verifies that numberOfReportsReceived >= 1"),
            TestStep("16", "Reset all accumulated report counts"),
            TestStep("17", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Plugged-in Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E00(EVConnected) sent"),
            TestStep("18", "Wait 5 seconds"),
            TestStep("18a", "TH counts all report transactions with an attribute report for the SessionID attribute",
                     "TH verifies that numberOfReportsReceived = 1"),
            TestStep("18b", "TH counts all report transactions with an attribute report for the SessionDuration attribute",
                     "TH verifies that numberOfReportsReceived >= 1"),
            TestStep("18c", "TH counts all report transactions with an attribute report for the SessionEnergyCharged attribute",
                     "TH verifies that numberOfReportsReceived >= 1"),
            TestStep("18d", "If V2X feature is supported on the cluster, TH counts all report transactions with an attribute report for the SessionEnergyDischarged attribute",
                     "TH verifies that numberOfReportsReceived >= 1"),
            TestStep("19", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Plugged-in Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E01(EVNotDetected) sent"),
            TestStep("20", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for Basic Functionality Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("21", "Cancel the subscription to the Device Energy Management cluster",
                     "The subscription is cancelled successfully"),
        ]

        return steps

    @async_test_body
    async def test_TC_EEVSE_2_6(self):
        self.step("1")
        # Commission DUT - already done

        self.step("2")
        feature_map = await self.read_evse_attribute_expect_success(attribute="FeatureMap")
        logger.info(f"FeatureMap: {feature_map}")
        has_v2x = feature_map & Clusters.EnergyEvse.Bitmaps.Feature.kV2x

        # Subscribe to Events and when they are sent push them to a queue for checking later
        self.step("3")
        events_callback = EventChangeCallback(Clusters.EnergyEvse)
        await events_callback.start(self.default_controller,
                                    self.dut_node_id,
                                    self.matter_test_config.endpoint)

        self.step("4")
        await self.check_test_event_triggers_enabled()

        self.step("5")
        sub_handler = ClusterAttributeChangeAccumulator(Clusters.EnergyEvse)
        await sub_handler.start(self.default_controller, self.dut_node_id,
                                self.matter_test_config.endpoint,
                                min_interval_sec=0,
                                max_interval_sec=10, keepSubscriptions=True)

        def accumulate_reports(wait_time):
            logging.info(f"Test will now wait {wait_time} seconds to accumulate reports")
            time.sleep(wait_time)

        self.step("6")
        await self.send_test_event_trigger_basic()

        self.step("6a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kNotPluggedIn)

        self.step("7")
        await self.send_test_event_trigger_pluggedin()
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EVConnected)

        self.step("8")
        charge_until = NullValue
        min_charge_current = 6000
        max_charge_current = 12000
        await self.send_enable_charge_command(charge_until=charge_until, min_charge=min_charge_current, max_charge=max_charge_current)

        self.step("9")
        await self.send_test_event_trigger_charge_demand()
        event_data = events_callback.wait_for_event_report(Clusters.EnergyEvse.Events.EnergyTransferStarted)

        self.step("9a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInCharging)

        self.step("10")
        wait = 12  # Wait 12 seconds - the spec says we should only get reports every 10s at most, unless a command changes it
        sub_handler.reset()
        accumulate_reports(wait)

        self.step("10a")
        count = sub_handler.attribute_report_counts[Clusters.EnergyEvse.Attributes.SessionID]
        logging.info(f"Received {count} SessionID updates in {wait} seconds")
        asserts.assert_equal(count, 0, f"Expected NO SessionID updates in {wait} seconds")

        self.step("10b")
        count = sub_handler.attribute_report_counts[Clusters.EnergyEvse.Attributes.SessionDuration]
        logging.info(f"Received {count} SessionDuration updates in {wait} seconds")
        asserts.assert_less_equal(count, 2, f"Expected <= 2 SessionDuration updates in {wait} seconds")

        self.step("10c")
        count = sub_handler.attribute_report_counts[Clusters.EnergyEvse.Attributes.SessionEnergyCharged]
        logging.info(f"Received {count} SessionEnergyCharged updates in {wait} seconds")
        asserts.assert_less_equal(count, 2, f"Expected <= 2 SessionEnergyCharged updates in {wait} seconds")

        self.step("10d")
        if has_v2x:
            count = sub_handler.attribute_report_counts[Clusters.EnergyEvse.Attributes.SessionEnergyDischarged]
            logging.info(f"Received {count} SessionEnergyDischarged updates in {wait} seconds")
            asserts.assert_less_equal(count, 2, f"Expected <= 2 SessionEnergyDischarged updates in {wait} seconds")

        self.step("11")
        sub_handler.reset()

        self.step("12")
        await self.send_disable_command()
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EnergyTransferStopped)
        expected_reason = Clusters.EnergyEvse.Enums.EnergyTransferStoppedReasonEnum.kEVSEStopped
        asserts.assert_equal(expected_reason, event_data.reason,
                             f"EnergyTransferStopped event reason was {event_data.reason}, expected {expected_reason}")

        self.step("13")
        await self.send_test_event_trigger_charge_demand_clear()

        self.step("14")
        await self.send_test_event_trigger_pluggedin_clear()
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EVNotDetected)

        self.step("15")
        wait = 5  # We expect a change to the Session attributes after the EV is unplugged, Wait 5 seconds - allow time for the report to come in
        accumulate_reports(wait)

        self.step("15a")
        count = sub_handler.attribute_report_counts[Clusters.EnergyEvse.Attributes.SessionID]
        logging.info(f"Received {count} SessionID updates in {wait} seconds")
        asserts.assert_equal(count, 0, "Expected = 0 SessionID updates after a Unplugged operation - it changes on next plug-in")

        self.step("15b")
        count = sub_handler.attribute_report_counts[Clusters.EnergyEvse.Attributes.SessionDuration]
        logging.info(f"Received {count} SessionDuration updates in {wait} seconds")
        asserts.assert_greater_equal(count, 1, "Expected >= 1 SessionDuration updates after a Unplugged operation")

        self.step("15c")
        count = sub_handler.attribute_report_counts[Clusters.EnergyEvse.Attributes.SessionEnergyCharged]
        logging.info(f"Received {count} SessionEnergyCharged updates in {wait} seconds")
        asserts.assert_greater_equal(count, 1, "Expected >= 1 SessionEnergyCharged updates after a Unplugged operation")

        self.step("15d")
        if has_v2x:
            count = sub_handler.attribute_report_counts[Clusters.EnergyEvse.Attributes.SessionEnergyDischarged]
            logging.info(f"Received {count} SessionEnergyDischarged updates in {wait} seconds")
            asserts.assert_greater_equal(count, 1, "Expected >= 1 SessionEnergyDischarged updates after a Unplugged operation")

        self.step("16")
        sub_handler.reset()

        self.step("17")
        await self.send_test_event_trigger_pluggedin()
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EVConnected)

        self.step("18")
        wait = 5  # We expect a change to the Session attributes after the EV is plugged in again, Wait 5 seconds - allow time for the report to come in
        accumulate_reports(wait)

        self.step("18a")
        count = sub_handler.attribute_report_counts[Clusters.EnergyEvse.Attributes.SessionID]
        logging.info(f"Received {count} SessionID updates in {wait} seconds")
        asserts.assert_equal(count, 1, "Expected = 1 SessionID updates after a plug-in")

        self.step("18b")
        count = sub_handler.attribute_report_counts[Clusters.EnergyEvse.Attributes.SessionDuration]
        logging.info(f"Received {count} SessionDuration updates in {wait} seconds")
        asserts.assert_greater_equal(count, 1, "Expected >= 1 SessionDuration updates after a Unplugged operation")

        self.step("18c")
        count = sub_handler.attribute_report_counts[Clusters.EnergyEvse.Attributes.SessionEnergyCharged]
        logging.info(f"Received {count} SessionEnergyCharged updates in {wait} seconds")
        asserts.assert_greater_equal(count, 1, "Expected >= 1 SessionEnergyCharged updates after a Unplugged operation")

        self.step("18d")
        if has_v2x:
            count = sub_handler.attribute_report_counts[Clusters.EnergyEvse.Attributes.SessionEnergyDischarged]
            logging.info(f"Received {count} SessionEnergyDischarged updates in {wait} seconds")
            asserts.assert_greater_equal(count, 1, "Expected >= 1 SessionEnergyDischarged updates after a Unplugged operation")

        self.step("19")
        await self.send_test_event_trigger_pluggedin_clear()
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EVNotDetected)

        self.step("20")
        await self.send_test_event_trigger_basic_clear()

        self.step("21")
        await sub_handler.cancel()


if __name__ == "__main__":
    default_matter_test_main()
