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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
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

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, MatterBaseTest
from matter.testing.runner import default_matter_test_main

# TestEventTrigger code: all alarms cleared (cleanup trigger, PIXIT.ESALM.TEST_EVENT_TRIGGER = 0x00A1_0000_0000_0000)
_TRIGGER_ALL_CLEAR = 0x00A1000000000000

cluster = Clusters.ElectricalAlarm


class TC_ESALM_3_1(MatterBaseTest):

    @run_if_endpoint_matches(has_cluster(cluster))
    async def test_TC_ESALM_3_1(self):
        """[TC-ESALM-3.1] Notify Event Reporting with Server as DUT

        Verify that the Electrical Alarm cluster sends Notify events when alarm conditions
        are triggered and cleared, with correct Active, Inactive, State, and Mask fields,
        and that subscription reports reflect the State attribute changes.
        """
        endpoint = self.get_endpoint()
        attrs = cluster.Attributes
        cmds = cluster.Commands

        self.step(1, "Commission DUT to TH", is_commissioning=True)

        self.step(2, "TH reads TestEventTriggersEnabled from General Diagnostics",
                  "Value is 1 (True). If 0, skip remaining steps.")
        gen_diag = Clusters.GeneralDiagnostics
        triggers_enabled = await self.read_single_attribute_check_success(
            endpoint=0, cluster=gen_diag, attribute=gen_diag.Attributes.TestEventTriggersEnabled)
        if not triggers_enabled:
            for step_num in [3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13]:
                self.step(step_num)
                self.mark_current_step_skipped()
            return

        self.step(3, "TH reads Supported, Mask, Latch, and State",
                  "SUCCESS. At least one alarm bit is set in Supported and enabled in Mask.")
        supported = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Supported)
        mask = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
        attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.AttributeList)
        has_latch_attr = attrs.Latch.attribute_id in attribute_list
        latch = 0
        if has_latch_attr:
            latch = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Latch)

        triggered_bit = None
        for bit in range(64):
            candidate = 1 << bit
            if (supported & candidate) and (mask & candidate):
                triggered_bit = candidate
                break
        asserts.assert_is_not_none(triggered_bit,
                                   "No alarm bit is both supported and enabled in Mask — cannot proceed")

        is_latched = bool(latch & triggered_bit)

        trigger_active = self.user_params.get("PIXIT.ESALM.TEST_EVENT_TRIGGER_ACTIVE", None)
        trigger_clear = self.user_params.get("PIXIT.ESALM.TEST_EVENT_TRIGGER_CLEAR", None)

        self.step(4, "TH establishes subscription to State with MinIntervalFloor=0, MaxIntervalCeiling=30",
                  "Subscription established; initial priming report received.")
        state_sub = AttributeSubscriptionHandler(cluster, attrs.State)
        await state_sub.start(self.default_controller, self.dut_node_id,
                              endpoint=endpoint, min_interval_sec=0,
                              max_interval_sec=30, keepSubscriptions=False)
        event_sub = EventSubscriptionHandler(expected_cluster=cluster)
        await event_sub.start(self.default_controller, self.dut_node_id,
                              endpoint=endpoint, min_interval_sec=0,
                              max_interval_sec=30)

        self.step(5, "TH sends TestEventTrigger to simulate an alarm condition for a supported and enabled alarm bit (TRIGGERED_BIT)",
                  "SUCCESS. Subscription report received with TRIGGERED_BIT set in State.")
        if trigger_active is None:
            self.mark_current_step_skipped()
            for step_num in [6, 7, 8, 9, 10, 11, 12, 13]:
                self.step(step_num)
                self.mark_current_step_skipped()
            return

        state_sub.reset()
        event_sub.reset()
        await self.send_test_event_triggers(eventTrigger=int(trigger_active, 0))

        def state_has_triggered_bit(report):
            return bool(report.value & triggered_bit)

        state_sub.await_all_expected_report_matches(
            [AttributeMatcher.from_callable("State has TRIGGERED_BIT set", state_has_triggered_bit)],
            timeout_sec=30)

        self.step(6, "TH waits up to 30 seconds for Notify event",
                  "Notify event received. Active has TRIGGERED_BIT set. Inactive does not. State has TRIGGERED_BIT set. Mask equals current Mask attribute.")
        notify_event = event_sub.wait_for_event_report(cluster.Events.Notify, timeout_sec=30)
        asserts.assert_true(notify_event.active & triggered_bit,
                            "Notify event: TRIGGERED_BIT not set in Active field")
        asserts.assert_false(notify_event.inactive & triggered_bit,
                             "Notify event: TRIGGERED_BIT unexpectedly set in Inactive field")
        asserts.assert_true(notify_event.state & triggered_bit,
                            "Notify event: TRIGGERED_BIT not set in State field")
        current_mask = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
        asserts.assert_equal(notify_event.mask, current_mask,
                             "Notify event Mask field does not match current Mask attribute")

        self.step(7, "TH reads State",
                  "DUT returns AlarmBitmap with TRIGGERED_BIT set.")
        state_val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.State)
        asserts.assert_true(state_val & triggered_bit, "State does not have TRIGGERED_BIT set")

        self.step(8, "TH sends TestEventTrigger to clear the alarm condition for TRIGGERED_BIT",
                  "SUCCESS.")
        state_sub.reset()
        event_sub.reset()
        if trigger_clear is not None:
            await self.send_test_event_triggers(eventTrigger=int(trigger_clear, 0))
        else:
            self.mark_current_step_skipped()

        if not is_latched:
            self.step(9, "If non-latched: TH awaits subscription report with TRIGGERED_BIT cleared in State",
                      "Report received with TRIGGERED_BIT = 0.")
            if trigger_clear is not None:
                def state_cleared_triggered_bit(report):
                    return not bool(report.value & triggered_bit)
                state_sub.await_all_expected_report_matches(
                    [AttributeMatcher.from_callable("State has TRIGGERED_BIT cleared", state_cleared_triggered_bit)],
                    timeout_sec=30)
            else:
                self.mark_current_step_skipped()

            self.step(10, "If non-latched: TH waits up to 30 seconds for Notify event",
                      "Notify event received. Inactive has TRIGGERED_BIT set. Active does not. State does not have TRIGGERED_BIT set.")
            if trigger_clear is not None:
                clear_event = event_sub.wait_for_event_report(cluster.Events.Notify, timeout_sec=30)
                asserts.assert_true(clear_event.inactive & triggered_bit,
                                    "Notify event: TRIGGERED_BIT not set in Inactive field on clear")
                asserts.assert_false(clear_event.active & triggered_bit,
                                     "Notify event: TRIGGERED_BIT unexpectedly set in Active field on clear")
                asserts.assert_false(clear_event.state & triggered_bit,
                                     "Notify event: TRIGGERED_BIT still set in State field on clear")
            else:
                self.mark_current_step_skipped()

            self.step(11, "If latched: TH reads State",
                      "TRIGGERED_BIT remains set (latched alarm persists until Reset).")
            self.mark_current_step_skipped()

            self.step(12, "If latched and Reset supported: TH sends Reset with TRIGGERED_BIT",
                      "SUCCESS. Subscription report received with TRIGGERED_BIT cleared.")
            self.mark_current_step_skipped()
        else:
            self.step(9, "If non-latched: TH awaits subscription report with TRIGGERED_BIT cleared in State",
                      "Report received with TRIGGERED_BIT = 0.")
            self.mark_current_step_skipped()

            self.step(10, "If non-latched: TH waits up to 30 seconds for Notify event",
                      "Notify event received. Inactive has TRIGGERED_BIT set. Active does not. State does not have TRIGGERED_BIT set.")
            self.mark_current_step_skipped()

            self.step(11, "If latched: TH reads State",
                      "TRIGGERED_BIT remains set (latched alarm persists until Reset).")
            state_val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.State)
            asserts.assert_true(state_val & triggered_bit,
                                "Latched alarm: TRIGGERED_BIT should still be set in State after condition clears")

            self.step(12, "If latched and Reset supported: TH sends Reset with TRIGGERED_BIT",
                      "SUCCESS. Subscription report received with TRIGGERED_BIT cleared.")
            accepted_cmds = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.AcceptedCommandList)
            if 0x00 in accepted_cmds:  # Reset command code = 0x00
                state_sub.reset()
                await self.send_single_cmd(cmd=cmds.Reset(alarms=triggered_bit), endpoint=endpoint)

                def state_cleared(report):
                    return not bool(report.value & triggered_bit)
                state_sub.await_all_expected_report_matches(
                    [AttributeMatcher.from_callable("State has TRIGGERED_BIT cleared after Reset", state_cleared)],
                    timeout_sec=30)
            else:
                self.mark_current_step_skipped()

        self.step(13, "TH sends TestEventTrigger for all-alarms-cleared (0x00A1_0000_0000_0000)",
                  "SUCCESS. Subscription report received with State = 0 (cleanup).")
        state_sub.reset()
        await self.send_test_event_triggers(eventTrigger=_TRIGGER_ALL_CLEAR)

        def state_is_zero(report):
            return report.value == 0
        state_sub.await_all_expected_report_matches(
            [AttributeMatcher.from_callable("State = 0 after cleanup trigger", state_is_zero)],
            timeout_sec=30)


if __name__ == "__main__":
    default_matter_test_main()
