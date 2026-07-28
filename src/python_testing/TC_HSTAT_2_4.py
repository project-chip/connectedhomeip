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
#       --endpoint 0
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
from TC_HSTAT_Test_Base import HSTATBase

from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# Auto-generated from test specification: [TC-HSTAT-2.4] Optional functionality with DUT as Server


class TC_HSTAT_2_4(MatterBaseTest, HSTATBase):

    def pics_TC_HSTAT_2_4(self) -> list[str]:
        return [
            "HSTAT.S",
        ]

    def desc_TC_HSTAT_2_4(self) -> str:
        return "[TC-HSTAT-2.4] Optional functionality with DUT as Server"

    def steps_TC_HSTAT_2_4(self):
        return [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).", ""),
            TestStep(2, "TH sends command On to the On/Off cluster on the same endpoint as this cluster.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(3, "TH sends command SetSettings with the Mode field set to Humidifier or Dehumidifier",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(4, "TH sends command SetSettings with the Continuous, Sleep, and Optimal fields set to False",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(5, "Individually subscribe to the attributes Continuous if supported, Sleep if supported, and Optimal if supported.",
                     "This will receive updates when these attributes change value."),
            TestStep(5, "TH sends command SetSettings with the Continuous field set to True.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(6, "TH reads from the DUT the Continuous attribute.", "Verify that the DUT response contains a value of True"),
            TestStep(7, "TH writes to the DUT the Continuous attribute with False.",
                     "Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Continuous with the first having a value of True and the second having a value of False."),
            TestStep(8, "TH sends command SetSettings with the Sleep field set to True.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(9, "TH reads from the DUT the Sleep attribute.", "Verify that the DUT response contains a value of True"),
            TestStep(10, "TH writes to the DUT the Sleep attribute with False.",
                     "Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Sleep with the first having a value of True and the second having a value of False."),
            TestStep(11, "TH sends command SetSettings with the Optimal field set to True.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(12, "TH reads from the DUT the Optimal attribute.", "Verify that the DUT response contains a value of True"),
            TestStep(13, "TH writes to the DUT the Optimal attribute with False.",
                     "Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Optimal with the first having a value of True and the second having a value of False."),
            TestStep(14, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowContinuous event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(15, "TH sends command SetSettings with the Continuous field set to True",
                     "Verify DUT responds w/ status INVALID_IN_STATE(0xcb)"),
            TestStep(16, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowContinuous event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(17, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowSleep event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(18, "TH sends command SetSettings with the Sleep field set to True",
                     "Verify DUT responds w/ status INVALID_IN_STATE(0xcb)"),
            TestStep(19, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowSleep event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(20, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowOptimal event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(21, "TH sends command SetSettings with the Optimal field set to True",
                     "Verify DUT responds w/ status INVALID_IN_STATE(0xcb)"),
            TestStep(22, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowOptimal event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_HSTAT_2_4(self):
        endpoint = self.get_endpoint()

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
        await self.send_onoff_on_cmd_expect_success(endpoint=endpoint)

        self.step(3)
        # TH sends command SetSettings with the Mode field set to Humidifier or Dehumidifier
        # Verify DUT responds w/ status SUCCESS(0x00)
        if self.humidifierFeatureSupported:
            await self.write_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.Mode(self.modeHumidifier))
        else:
            await self.write_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.Mode(self.modeDehumidifier))

        self.step(4)
        # TH sends command SetSettings with the Continuous, Sleep, and Optimal fields set to False
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_SetSettingsCommand_expect_success(endpoint=endpoint, continuous=False, sleep=False, optimal=False)

        self.step(5)
        # Individually subscribe to the attributes Continuous if supported, Sleep if supported, and Optimal if supported.
        # This will receive updates when these attributes change value.
        if self.continuousFeatureSupported:
            continuousSubscription = AttributeSubscriptionHandler(self.cluster, self.attributes.Continuous)
            await continuousSubscription.start(self.default_controller, self.dut_node_id, endpoint)
            continuousReportsReceived = []
        if self.attributes.Sleep.attribute_id in self.supported_attributes:
            sleepSubscription = AttributeSubscriptionHandler(self.cluster, self.attributes.Sleep)
            await sleepSubscription.start(self.default_controller, self.dut_node_id, endpoint)
            sleepReportsReceived = []
        if self.optimalFeatureSupported:
            optimalSubscription = AttributeSubscriptionHandler(self.cluster, self.attributes.Optimal)
            await optimalSubscription.start(self.default_controller, self.dut_node_id, endpoint)
            optimalReportsReceived = []

        if self.continuousFeatureSupported:
            self.step(5)
            # TH sends command SetSettings with the Continuous field set to True.
            # Verify DUT responds w/ status SUCCESS(0x00)
            await self.send_SetSettingsCommand_expect_success(endpoint=endpoint, continuous=True)
            continuousReportsReceived.append(continuousSubscription.wait_for_attribute_report().value)

            self.step(6)
            # TH reads from the DUT the Continuous attribute.
            # Verify that the DUT response contains a value of True
            dut_Continuous = await self.read_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.Continuous)
            asserts.assert_true(dut_Continuous, "Continuous attribute was not True")

            self.step(7)
            # TH writes to the DUT the Continuous attribute with False.
            # Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Continuous with the first having a value of True and the second having a value of False.
            await self.send_SetSettingsCommand_expect_success(endpoint=endpoint, continuous=False)
            continuousReportsReceived.append(continuousSubscription.wait_for_attribute_report().value)
            asserts.assert_equal(len(continuousReportsReceived), 2, "Wrong number of reports received for Continuous")
            asserts.assert_equal(continuousReportsReceived[0], True, "First report for Continuous is not True")
            asserts.assert_equal(continuousReportsReceived[1], False, "Second report for Continuous is not False")
        else:
            self.mark_step_range_skipped(5, 7)

        if self.attributes.Sleep.attribute_id in self.supported_attributes:
            self.step(8)
            # TH sends command SetSettings with the Sleep field set to True.
            # Verify DUT responds w/ status SUCCESS(0x00)
            await self.send_SetSettingsCommand_expect_success(endpoint=endpoint, sleep=True)
            sleepReportsReceived.append(sleepSubscription.wait_for_attribute_report().value)

            self.step(9)
            # TH reads from the DUT the Sleep attribute.
            # Verify that the DUT response contains a value of True
            dut_Sleep = await self.read_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.Sleep)
            asserts.assert_true(dut_Sleep, "Sleep attribute was not True")

            self.step(10)
            # TH writes to the DUT the Sleep attribute with False.
            # Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Sleep with the first having a value of True and the second having a value of False.
            await self.send_SetSettingsCommand_expect_success(endpoint=endpoint, sleep=False)
            sleepReportsReceived.append(sleepSubscription.wait_for_attribute_report().value)
            asserts.assert_equal(len(sleepReportsReceived), 2, "Wrong number of reports received for Sleep")
            asserts.assert_equal(sleepReportsReceived[0], True, "First report for Sleep is not True")
            asserts.assert_equal(sleepReportsReceived[1], False, "Second report for Sleep is not False")
        else:
            self.mark_step_range_skipped(8, 10)

        if self.optimalFeatureSupported:
            self.step(11)
            # TH sends command SetSettings with the Optimal field set to True.
            # Verify DUT responds w/ status SUCCESS(0x00)
            await self.send_SetSettingsCommand_expect_success(endpoint=endpoint, optimal=True)
            optimalReportsReceived.append(optimalSubscription.wait_for_attribute_report().value)

            self.step(12)
            # TH reads from the DUT the Optimal attribute.
            # Verify that the DUT response contains a value of True
            dut_Optimal = await self.read_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.Optimal)
            asserts.assert_true(dut_Optimal, "Optimal attribute was not True")

            self.step(13)
            # TH writes to the DUT the Optimal attribute with False.
            # Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Optimal with the first having a value of True and the second having a value of False.
            await self.send_SetSettingsCommand_expect_success(endpoint=endpoint, optimal=False)
            optimalReportsReceived.append(optimalSubscription.wait_for_attribute_report().value)
            asserts.assert_equal(len(optimalReportsReceived), 2, "Wrong number of reports received for Optimal")
            asserts.assert_equal(optimalReportsReceived[0], True, "First report for Sleep is not Optimal")
            asserts.assert_equal(optimalReportsReceived[1], False, "Second report for Sleep is not Optimal")
        else:
            self.mark_step_range_skipped(11, 12)

        #
        # Need to fix the remaining test cases. They should only be run if the appropriate trigger PICS are indicated.
        #

        if self.attributes.Continuous.attribute_id not in self.supported_attributes:
            self.mark_step_range_skipped(14, 16)
        else:
            self.step(14)
            # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowContinuous event.
            # Verify DUT responds w/ status SUCCESS(0x00).

            self.step(15)
            # TH sends command SetSettings with the Continuous field set to True
            # Verify DUT responds w/ status INVALID_IN_STATE(0xcb)

            self.step(16)
            # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowContinuous event.
            # Verify DUT responds w/ status SUCCESS(0x00).

        if self.attributes.Sleep.attribute_id not in self.supported_attributes:
            self.mark_step_range_skipped(17, 19)
        else:
            self.step(17)
            # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowSleep event.
            # Verify DUT responds w/ status SUCCESS(0x00).

            self.step(18)
            # TH sends command SetSettings with the Sleep field set to True
            # Verify DUT responds w/ status INVALID_IN_STATE(0xcb)

            self.step(19)
            # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowSleep event.
            # Verify DUT responds w/ status SUCCESS(0x00).

        if self.attributes.Optimal.attribute_id not in self.supported_attributes:
            self.mark_step_range_skipped(20, 22)
        else:
            self.step(20)
            # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowOptimal event.
            # Verify DUT responds w/ status SUCCESS(0x00).

            self.step(21)
            # TH sends command SetSettings with the Optimal field set to True
            # Verify DUT responds w/ status INVALID_IN_STATE(0xcb)

            self.step(22)
            # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowOptimal event.
            # Verify DUT responds w/ status SUCCESS(0x00).


if __name__ == '__main__':
    default_matter_test_main()
