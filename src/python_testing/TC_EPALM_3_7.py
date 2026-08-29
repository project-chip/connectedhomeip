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
from TC_EPALM_TestBase import ElectricalProtectionAlarmTestBaseHelper

import matter.clusters as Clusters
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.ElectricalProtectionAlarm
ALARM = cluster.Bitmaps.AlarmBitmap.kSelfTest
ALARM_MASK = int(ALARM)


class TC_EPALM_3_7(ElectricalProtectionAlarmTestBaseHelper):

    def pics_TC_EPALM_3_7(self) -> list[str]:
        return ["EPALM.S", "EPALM.S.F26", "EPALM.S.E00"]

    @run_if_endpoint_matches(has_feature(cluster, cluster.Bitmaps.Feature.kSelfTest))
    async def test_TC_EPALM_3_7(self):
        """[TC-EPALM-3.7] Notify event for SelfTest with Server as DUT

        This test case verifies Notify event reporting and the corresponding State
        transitions for the SelfTest alarm of the Electrical Protection Alarm
        Cluster server.
        """
        endpoint = self.get_endpoint()

        self.step("1a", "Commission DUT to TH (already done)", is_commissioning=True)

        self.step("1b", "TH sets up a subscription to the Notify event")
        sub = EventSubscriptionHandler(expected_cluster=cluster)
        await sub.start(self.default_controller, self.dut_node_id, endpoint)

        try:

            self.step("1c", "TH reads the Supported attribute",
                      expectation="Bit 6 of the response SHALL be 1.")
            supported = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Supported)
            asserts.assert_true(int(supported) & ALARM_MASK,
                                "Supported must set bit 6 for SelfTest")

            self.step("1d", "TH reads TestEventTriggersEnabled from General Diagnostics on endpoint 0",
                      expectation="Verify that TestEventTriggersEnabled has a value of 1 (True).")
            await self.check_test_event_triggers_enabled()

            self.step("1e", "TH sends the TestEventTrigger clearing SelfTest, returning the DUT to its "
                            "no-fault baseline",
                      expectation="Verify DUT responds w/ status SUCCESS(0x00).")
            state_before = await self.read_state(endpoint)
            await self.send_test_event_trigger_clear_alarm(ALARM)
            if state_before & ALARM_MASK:
                # Already raised, so this trigger emits an inactive Notify. Consume it here rather than
                # letting it arrive after the flush, where step 2a would read it instead of the
                # set-transition it is waiting for.
                report = await self.await_notify(sub)
                asserts.assert_true(int(report.inactive) & ALARM_MASK,
                                    "Baseline clear must report SelfTest in Inactive")
            asserts.assert_false(await self.read_state(endpoint) & ALARM_MASK,
                                 "SelfTest must be clear before the set trigger")
            sub.flush_events()

            self.step("2a", "TH sends the TestEventTrigger setting SelfTest",
                      expectation="Verify DUT responds w/ status SUCCESS(0x00). TH awaits subscription report of a Notify "
                      "event with bit 6 set in Active and set in State.")
            await self.send_test_event_trigger_set_alarm(ALARM)
            report = await self.await_notify(sub)
            log.info("Notify on set: active=0x%02X inactive=0x%02X state=0x%02X",
                     report.active, report.inactive, report.state)
            asserts.assert_true(int(report.active) & ALARM_MASK,
                                "Notify.Active must set bit 6 when SelfTest is raised")
            asserts.assert_true(int(report.state) & ALARM_MASK,
                                "Notify.State must set bit 6 when SelfTest is raised")

            self.step("2b", "TH reads the State attribute", expectation="Bit 6 of the response SHALL be 1.")
            asserts.assert_true(await self.read_state(endpoint) & ALARM_MASK,
                                "State bit 6 must be 1 after the set trigger")

            self.step("3a", "TH sends the TestEventTrigger clearing SelfTest",
                      expectation="Verify DUT responds w/ status SUCCESS(0x00). TH awaits subscription report of a Notify "
                      "event with bit 6 set in Inactive and clear in State.")
            await self.send_test_event_trigger_clear_alarm(ALARM)
            report = await self.await_notify(sub)
            log.info("Notify on clear: active=0x%02X inactive=0x%02X state=0x%02X",
                     report.active, report.inactive, report.state)
            asserts.assert_true(int(report.inactive) & ALARM_MASK,
                                "Notify.Inactive must set bit 6 when SelfTest is cleared")
            asserts.assert_false(int(report.state) & ALARM_MASK,
                                 "Notify.State must clear bit 6 when SelfTest is cleared")

            self.step("3b", "TH reads the State attribute", expectation="Bit 6 of the response SHALL be 0.")
            asserts.assert_false(await self.read_state(endpoint) & ALARM_MASK,
                                 "State bit 6 must be 0 after the clear trigger")
        finally:
            # Leave the DUT in its no-fault baseline even if a step above failed. A second clear
            # after step 3a is a no-op: no transition means no report.
            try:
                await self.send_test_event_trigger_clear_alarm(ALARM)
            except Exception:  # noqa: BLE001 - never mask the original failure
                log.exception("Could not clear SelfTest during cleanup")
            sub.cancel()


if __name__ == "__main__":
    default_matter_test_main()
