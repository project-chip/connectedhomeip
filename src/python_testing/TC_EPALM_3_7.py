#
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
#     app: ${ELECTRICAL_PROTECTION_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --enable-key 000102030405060708090a0b0c0d0e0f
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.ElectricalProtectionAlarm

ALARM_BIT = 6
ALARM_MASK = 1 << ALARM_BIT

# TestEventTrigger codes implemented by ElectricalProtectionAlarmTestEventTriggerHandler.
# The low byte selects the alarm: 0x01 through 0x07 raise one, 0x11 through 0x17 lower it.
SET_TRIGGER = 0x00A3_0000_0000_0007
CLEAR_TRIGGER = 0x00A3_0000_0000_0017

EVENT_TIMEOUT_SEC = 10.0
SETTLE_SEC = 2.0


class TC_EPALM_3_7(MatterBaseTest):

    def desc_TC_EPALM_3_7(self) -> str:
        return "[TC-EPALM-3.7] Notify event for SelfTest with DUT as Server"

    def pics_TC_EPALM_3_7(self) -> list[str]:
        return ["EPALM.S", "EPALM.S.F26", "EPALM.S.E00"]

    def steps_TC_EPALM_3_7(self) -> list[TestStep]:
        return [
            TestStep("1a", "Commission DUT to TH (already done)", is_commissioning=True),
            TestStep("1b", "TH sets up a subscription to the Notify event"),
            TestStep("1c", "TH reads the Supported attribute",
                     "Bit 6 is set, so the DUT supports SelfTest."),
            TestStep("1d", "TH reads TestEventTriggersEnabled from General Diagnostics on endpoint 0",
                     "Value is 1 (True)."),
            TestStep("1e", "TH sends the TestEventTrigger clearing SelfTest, returning the DUT to its "
                           "no-fault baseline", "DUT returns SUCCESS."),
            TestStep("2a", "TH sends the TestEventTrigger setting SelfTest",
                     "DUT returns SUCCESS and reports a Notify event with bit 6 set in Active and "
                     "set in State."),
            TestStep("2b", "TH reads the State attribute", "Bit 6 is 1."),
            TestStep("3a", "TH sends the TestEventTrigger clearing SelfTest",
                     "DUT returns SUCCESS and reports a Notify event with bit 6 set in Inactive and "
                     "clear in State."),
            TestStep("3b", "TH reads the State attribute", "Bit 6 is 0."),
        ]

    async def _await_notify(self, sub: EventSubscriptionHandler):
        """Return the next Notify event, failing the test if it could not be decoded."""
        report = sub.wait_for_event_report(cluster.Events.Notify, timeout_sec=EVENT_TIMEOUT_SEC)
        # wait_for_event_report hands back a ValueDecodeFailure rather than raising when the payload
        # will not decode, so the attribute check below is load-bearing and not dead code.
        asserts.assert_true(hasattr(report, "active") and hasattr(report, "state"),
                            f"Notify event did not decode into an event object: {report}")
        return report

    async def _read_state(self, endpoint: int) -> int:
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.State)

    @run_if_endpoint_matches(has_feature(cluster, cluster.Bitmaps.Feature.kSelfTest))
    async def test_TC_EPALM_3_7(self):
        endpoint = self.get_endpoint()

        self.step("1a")

        self.step("1b")
        sub = EventSubscriptionHandler(expected_cluster=cluster)
        await sub.start(self.default_controller, self.dut_node_id, endpoint)

        self.step("1c")
        supported = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Supported)
        asserts.assert_true(int(supported) & ALARM_MASK,
                            "Supported must set bit 6 for SelfTest")

        self.step("1d")
        await self.check_test_event_triggers_enabled()

        self.step("1e")
        state_before = await self._read_state(endpoint)
        await self.send_test_event_triggers(eventTrigger=CLEAR_TRIGGER)
        if state_before & ALARM_MASK:
            # The alarm was already raised, so this trigger produces an inactive Notify. Consume it
            # here rather than letting it arrive after the flush, where step 2a would read it
            # instead of the set-transition it is waiting for.
            report = await self._await_notify(sub)
            asserts.assert_true(int(report.inactive) & ALARM_MASK,
                                "Baseline clear must report SelfTest in Inactive")
        asserts.assert_false(int(await self._read_state(endpoint)) & ALARM_MASK,
                             "SelfTest must be clear before the set trigger")
        sub.flush_events()

        try:
            self.step("2a")
            await self.send_test_event_triggers(eventTrigger=SET_TRIGGER)
            report = await self._await_notify(sub)
            log.info("Notify on set: active=0x%02X inactive=0x%02X state=0x%02X",
                     report.active, report.inactive, report.state)
            asserts.assert_true(int(report.active) & ALARM_MASK,
                                "Notify.Active must set bit 6 when SelfTest is raised")
            asserts.assert_true(int(report.state) & ALARM_MASK,
                                "Notify.State must set bit 6 when SelfTest is raised")

            self.step("2b")
            state = await self._read_state(endpoint)
            asserts.assert_true(int(state) & ALARM_MASK, "State bit 6 must be 1 after the set trigger")

            self.step("3a")
            await self.send_test_event_triggers(eventTrigger=CLEAR_TRIGGER)
            report = await self._await_notify(sub)
            log.info("Notify on clear: active=0x%02X inactive=0x%02X state=0x%02X",
                     report.active, report.inactive, report.state)
            asserts.assert_true(int(report.inactive) & ALARM_MASK,
                                "Notify.Inactive must set bit 6 when SelfTest is cleared")
            asserts.assert_false(int(report.state) & ALARM_MASK,
                                 "Notify.State must clear bit 6 when SelfTest is cleared")

            self.step("3b")
            state = await self._read_state(endpoint)
            asserts.assert_false(int(state) & ALARM_MASK, "State bit 6 must be 0 after the clear trigger")
        finally:
            # Return the DUT to its no-fault baseline even if a step above failed, so a
            # failure here does not leave the alarm latched for the test cases that follow.
            # A second clear after step 3a is a no-op: no transition means no report.
            try:
                await self.send_test_event_triggers(eventTrigger=CLEAR_TRIGGER)
            except Exception:  # noqa: BLE001 - never mask the original failure
                log.exception('Could not clear SelfTest during cleanup')
            sub.cancel()


if __name__ == "__main__":
    default_matter_test_main()
