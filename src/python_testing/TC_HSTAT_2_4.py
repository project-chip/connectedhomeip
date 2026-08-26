#    Copyright (c) 2026 Project CHIP Authors
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
#
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --endpoint 1
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_HSTAT_common import HSTATBase

from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# Auto-generated from test specification: [TC-HSTAT-2.4] Optional functionality with DUT as Server


class TC_HSTAT_2_4(HSTATBase):

    def pics_TC_HSTAT_2_4(self) -> list[str]:
        return [
            "HSTAT.S",
        ]

    def desc_TC_HSTAT_2_4(self) -> str:
        return "[TC-HSTAT-2.4] Optional functionality with DUT as Server"

    def steps_TC_HSTAT_2_4(self):
        return [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep(2, "TH sends command On to the On/Off cluster on the same endpoint as this cluster.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(3, "TH sends command SetSettings with the Mode field set to Humidifier if supported or otherwise to Dehumidifier",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(4, "TH sends command SetSettings with the Continuous, Sleep, and Optimal fields set to False",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(5, "Individually subscribe to the attributes Continuous if supported, Sleep if supported, and Optimal if supported.",
                     "This will receive updates when these attributes change value."),
            TestStep(6, "TH sends command SetSettings with the Continuous field set to True.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(7, "TH reads from the DUT the Continuous attribute.", "Verify that the DUT response contains a value of True"),
            TestStep(8, "TH writes to the DUT the Continuous attribute with False.",
                     "Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Continuous with the first having a value of True and the second having a value of False."),
            TestStep(9, "TH sends command SetSettings with the Sleep field set to True.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(10, "TH reads from the DUT the Sleep attribute.", "Verify that the DUT response contains a value of True"),
            TestStep(11, "TH writes to the DUT the Sleep attribute with False.",
                     "Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Sleep with the first having a value of True and the second having a value of False."),
            TestStep(12, "TH sends command SetSettings with the Optimal field set to True.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(13, "TH reads from the DUT the Optimal attribute.", "Verify that the DUT response contains a value of True"),
            TestStep(14, "TH writes to the DUT the Optimal attribute with False.",
                     "Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Optimal with the first having a value of True and the second having a value of False."),
            TestStep(15, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowContinuous event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(16, "TH sends command SetSettings with the Continuous field set to True",
                     "Verify DUT responds w/ status INVALID_IN_STATE(0xcb)"),
            TestStep(17, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowContinuous event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(18, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowSleep event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(19, "TH sends command SetSettings with the Sleep field set to True",
                     "Verify DUT responds w/ status INVALID_IN_STATE(0xcb)"),
            TestStep(20, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowSleep event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(21, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowOptimal event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(22, "TH sends command SetSettings with the Optimal field set to True",
                     "Verify DUT responds w/ status INVALID_IN_STATE(0xcb)"),
            TestStep(23, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowOptimal event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    async def disallow_continuous_event(self):
        await self.send_test_event_triggers(eventTrigger=0x0205000000000000)

    async def allow_continuous_event(self):
        await self.send_test_event_triggers(eventTrigger=0x0205000000000001)

    async def disallow_sleep_event(self):
        await self.send_test_event_triggers(eventTrigger=0x0205000000000002)

    async def allow_sleep_event(self):
        await self.send_test_event_triggers(eventTrigger=0x0205000000000003)

    async def disallow_optimal_event(self):
        await self.send_test_event_triggers(eventTrigger=0x0205000000000004)

    async def allow_optimal_event(self):
        await self.send_test_event_triggers(eventTrigger=0x0205000000000005)

    @async_test_body
    async def test_TC_HSTAT_2_4(self):
        self.step(1)
        # Commission DUT to TH (can be skipped if done in a preceding test).
        await self.setup()

        # Not sure how to gate this test case with the 'def pics_' statement, so doing that here.
        # If any of these attributes are supported, the tests should continue, otherwise they should be skipped.
        if self.attributes.Continuous.attribute_id not in self.supported_attributes:
            if self.attributes.Sleep.attribute_id not in self.supported_attributes:
                if self.attributes.Optimal.attribute_id not in self.supported_attributes:
                    log.info("No needed attributes are supported, so skipping all tests.")
                    self.mark_all_remaining_steps_skipped(2)
                    return

        self.step(2)
        # TH sends command On to the On/Off cluster on the same endpoint as this cluster.
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_onoff_on_cmd_expect_success()

        self.step(3)
        # TH sends command SetSettings with the Mode field set to Humidifier if supported or otherwise to Dehumidifier
        # Verify DUT responds w/ status SUCCESS(0x00)
        if self.humidifierFeatureSupported:
            await self.send_SetSettingsCommand_expect_success(mode=self.modeHumidifier)
        else:
            await self.send_SetSettingsCommand_expect_success(mode=self.modeDehumidifier)

        self.step(4)
        # TH sends command SetSettings with the Continuous, Sleep, and Optimal fields set to False
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_SetSettingsCommand_expect_success(continuous=False, sleep=False, optimal=False)

        self.step(5)
        # Individually subscribe to the attributes Continuous if supported, Sleep if supported, and Optimal if supported.
        # This will receive updates when these attributes change value.
        if self.continuousFeatureSupported:
            continuousSubscription = AttributeSubscriptionHandler(self.cluster, self.attributes.Continuous)
            await continuousSubscription.start(self.default_controller, self.dut_node_id, self.endpoint)
            continuousReportsReceived = []
        if self.attributes.Sleep.attribute_id in self.supported_attributes:
            sleepSubscription = AttributeSubscriptionHandler(self.cluster, self.attributes.Sleep)
            await sleepSubscription.start(self.default_controller, self.dut_node_id, self.endpoint)
            sleepReportsReceived = []
        if self.optimalFeatureSupported:
            optimalSubscription = AttributeSubscriptionHandler(self.cluster, self.attributes.Optimal)
            await optimalSubscription.start(self.default_controller, self.dut_node_id, self.endpoint)
            optimalReportsReceived = []

        if self.continuousFeatureSupported:
            self.step(6)
            # TH sends command SetSettings with the Continuous field set to True.
            # Verify DUT responds w/ status SUCCESS(0x00)
            await self.send_SetSettingsCommand_expect_success(continuous=True)
            continuousReportsReceived.append(continuousSubscription.wait_for_attribute_report().value)

            self.step(7)
            # TH reads from the DUT the Continuous attribute.
            # Verify that the DUT response contains a value of True
            dut_Continuous = await self.read_attribute_expect_success(attribute=self.attributes.Continuous)
            asserts.assert_true(dut_Continuous, "Continuous attribute was not True")

            self.step(8)
            # TH writes to the DUT the Continuous attribute with False.
            # Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Continuous with the first having a value of True and the second having a value of False.
            await self.write_single_attribute(attribute_value=self.attributes.Continuous(False), expect_success=True)
            continuousReportsReceived.append(continuousSubscription.wait_for_attribute_report().value)
            asserts.assert_equal(len(continuousReportsReceived), 2, "Wrong number of reports received for Continuous")
            asserts.assert_equal(continuousReportsReceived[0], True, "First report for Continuous is not True")
            asserts.assert_equal(continuousReportsReceived[1], False, "Second report for Continuous is not False")
        else:
            self.mark_step_range_skipped(6, 8)

        if self.attributes.Sleep.attribute_id in self.supported_attributes:
            self.step(9)
            # TH sends command SetSettings with the Sleep field set to True.
            # Verify DUT responds w/ status SUCCESS(0x00)
            await self.send_SetSettingsCommand_expect_success(sleep=True)
            sleepReportsReceived.append(sleepSubscription.wait_for_attribute_report().value)

            self.step(10)
            # TH reads from the DUT the Sleep attribute.
            # Verify that the DUT response contains a value of True
            dut_Sleep = await self.read_attribute_expect_success(attribute=self.attributes.Sleep)
            asserts.assert_true(dut_Sleep, "Sleep attribute was not True")

            self.step(11)
            # TH writes to the DUT the Sleep attribute with False.
            # Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Sleep with the first having a value of True and the second having a value of False.
            await self.write_single_attribute(attribute_value=self.attributes.Sleep(False), expect_success=True)
            sleepReportsReceived.append(sleepSubscription.wait_for_attribute_report().value)
            asserts.assert_equal(len(sleepReportsReceived), 2, "Wrong number of reports received for Sleep")
            asserts.assert_equal(sleepReportsReceived[0], True, "First report for Sleep is not True")
            asserts.assert_equal(sleepReportsReceived[1], False, "Second report for Sleep is not False")
        else:
            self.mark_step_range_skipped(9, 11)

        if self.optimalFeatureSupported:
            self.step(12)
            # TH sends command SetSettings with the Optimal field set to True.
            # Verify DUT responds w/ status SUCCESS(0x00)
            await self.send_SetSettingsCommand_expect_success(optimal=True)
            optimalReportsReceived.append(optimalSubscription.wait_for_attribute_report().value)

            self.step(13)
            # TH reads from the DUT the Optimal attribute.
            # Verify that the DUT response contains a value of True
            dut_Optimal = await self.read_attribute_expect_success(attribute=self.attributes.Optimal)
            asserts.assert_true(dut_Optimal, "Optimal attribute was not True")

            self.step(14)
            # TH writes to the DUT the Optimal attribute with False.
            # Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Optimal with the first having a value of True and the second having a value of False.
            await self.write_single_attribute(attribute_value=self.attributes.Optimal(False), expect_success=True)
            optimalReportsReceived.append(optimalSubscription.wait_for_attribute_report().value)
            asserts.assert_equal(len(optimalReportsReceived), 2, "Wrong number of reports received for Optimal")
            asserts.assert_equal(optimalReportsReceived[0], True, "First report for Optimal is not True")
            asserts.assert_equal(optimalReportsReceived[1], False, "Second report for Optimal is not False")
        else:
            self.mark_step_range_skipped(12, 14)

        if (self.attributes.Continuous.attribute_id in self.supported_attributes) and (self.check_pics("HSTAT.S.M.ContinuousError")):
            self.step(15)
            # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowContinuous event.
            # Verify DUT responds w/ status SUCCESS(0x00).
            await self.disallow_continuous_event()

            self.step(16)
            # TH sends command SetSettings with the Continuous field set to True
            # Verify DUT responds w/ status INVALID_IN_STATE(0xcb)
            await self.send_SetSettingsCommand_expect_error(continuous=True, error=Status.InvalidInState)

            self.step(17)
            # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowContinuous event.
            # Verify DUT responds w/ status SUCCESS(0x00).
            await self.allow_continuous_event()
        else:
            self.mark_step_range_skipped(15, 17)

        if (self.attributes.Sleep.attribute_id in self.supported_attributes) and (self.check_pics("HSTAT.S.M.SleepError")):
            self.step(18)
            # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowSleep event.
            # Verify DUT responds w/ status SUCCESS(0x00).
            await self.disallow_sleep_event()

            self.step(19)
            # TH sends command SetSettings with the Sleep field set to True
            # Verify DUT responds w/ status INVALID_IN_STATE(0xcb)
            await self.send_SetSettingsCommand_expect_error(sleep=True, error=Status.InvalidInState)

            self.step(20)
            # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowSleep event.
            # Verify DUT responds w/ status SUCCESS(0x00).
            await self.allow_sleep_event()
        else:
            self.mark_step_range_skipped(18, 20)

        if (self.attributes.Optimal.attribute_id in self.supported_attributes) and (self.check_pics("HSTAT.S.M.OptimalError")):
            self.step(21)
            # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowOptimal event.
            # Verify DUT responds w/ status SUCCESS(0x00).
            await self.disallow_optimal_event()

            self.step(22)
            # TH sends command SetSettings with the Optimal field set to True
            # Verify DUT responds w/ status INVALID_IN_STATE(0xcb)
            await self.send_SetSettingsCommand_expect_error(optimal=True, error=Status.InvalidInState)

            self.step(23)
            # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowOptimal event.
            # Verify DUT responds w/ status SUCCESS(0x00).
            await self.allow_optimal_event()
        else:
            self.mark_step_range_skipped(21, 23)


if __name__ == '__main__':
    default_matter_test_main()
