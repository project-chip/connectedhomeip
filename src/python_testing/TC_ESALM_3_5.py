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
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ELECTRICAL_PROTECTION_APP}
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
#       --endpoint 2
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, MatterBaseTest
from matter.testing.runner import default_matter_test_main

cluster = Clusters.ElectricalAlarm
_F = cluster.Bitmaps.Feature

# TestEventTrigger codes from the PIXIT Variable Values table of the Electrical Alarm test plan.
# The per-alarm clear removes the measured condition and leaves a latched alarm active; the
# all-clear is the cleanup path and drops latched alarms too.
_TRIGGER_SET = 0x00A1000000000009
_TRIGGER_CLEAR = 0x00A100000000000A
_TRIGGER_ALL_CLEAR = 0x00A1000000000000

_ALARM_BIT = int(cluster.Bitmaps.AlarmBitmap.kOverPower)


class TC_ESALM_3_5(MatterBaseTest):

    @run_if_endpoint_matches(has_feature(cluster, _F.kOverPower))
    async def test_TC_ESALM_3_5(self):
        """[TC-ESALM-3.5] OverPower alarm lifecycle with Server as DUT"""
        endpoint = self.get_endpoint()
        attrs = cluster.Attributes
        cmds = cluster.Commands

        self.step(1, "Commission DUT to TH (already done)")

        self.step(2, "TH reads TestEventTriggersEnabled from General Diagnostics",
                  expectation="Value is 1 (True). If 0, skip remaining steps.")
        gen_diag = Clusters.GeneralDiagnostics
        triggers_enabled = await self.read_single_attribute_check_success(
            endpoint=0, cluster=gen_diag, attribute=gen_diag.Attributes.TestEventTriggersEnabled)
        if not triggers_enabled:
            self.mark_all_remaining_steps_skipped(3)
            return

        self.step(3, "TH reads Supported, Mask, Latch, and State",
                  expectation="SUCCESS. The OverPower bit is set in Supported and enabled in Mask.")
        supported = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Supported)
        mask = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
        attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.AttributeList)
        latch = 0
        if attrs.Latch.attribute_id in attribute_list:
            latch = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Latch)

        asserts.assert_true(int(supported) & _ALARM_BIT,
                            "OverPower is not set in Supported, so this alarm cannot be tested")
        asserts.assert_true(int(mask) & _ALARM_BIT,
                            "OverPower is not enabled in Mask, so the alarm would be suppressed")

        is_latched = bool(int(latch) & _ALARM_BIT)

        # A trigger only reports if it changes State, so the alarm must start inactive.
        current_state = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.State)
        asserts.assert_false(int(current_state) & _ALARM_BIT,
                             "OverPower is already active before the test starts")

        self.step(4, "TH establishes subscription to State with MinIntervalFloor=0, MaxIntervalCeiling=30",
                  expectation="Subscription established; initial priming report received.")
        state_sub = AttributeSubscriptionHandler(cluster, attrs.State)
        await state_sub.start(self.default_controller, self.dut_node_id,
                              endpoint=endpoint, min_interval_sec=0,
                              max_interval_sec=30, keepSubscriptions=False)
        event_sub = EventSubscriptionHandler(expected_cluster=cluster)
        await event_sub.start(self.default_controller, self.dut_node_id,
                              endpoint=endpoint, min_interval_sec=0,
                              max_interval_sec=30)

        self.step(5, "TH sends TestEventTrigger to simulate the OverPower alarm condition",
                  expectation="SUCCESS. Subscription report received with the OverPower bit set in State.")
        state_sub.reset()
        event_sub.reset()
        await self.send_test_event_triggers(eventTrigger=_TRIGGER_SET)

        def state_has_alarm(report):
            return bool(report.value & _ALARM_BIT)

        state_sub.await_all_expected_report_matches(
            [AttributeMatcher.from_callable("State has the OverPower bit set", state_has_alarm)],
            timeout_sec=30)

        self.step(6, "TH waits up to 30 seconds for Notify event",
                  expectation="Notify received. Active has the OverPower bit, Inactive does not, State does, Mask matches.")
        notify_event = event_sub.wait_for_event_report(cluster.Events.Notify, timeout_sec=30)
        asserts.assert_true(notify_event.active & _ALARM_BIT,
                            "Notify event: OverPower not set in Active")
        asserts.assert_false(notify_event.inactive & _ALARM_BIT,
                             "Notify event: OverPower unexpectedly set in Inactive")
        asserts.assert_true(notify_event.state & _ALARM_BIT,
                            "Notify event: OverPower not set in State")
        current_mask = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
        asserts.assert_equal(notify_event.mask, current_mask,
                             "Notify event Mask does not match the Mask attribute")

        self.step(7, "TH reads State",
                  expectation="DUT returns an AlarmBitmap with the OverPower bit set.")
        state_val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.State)
        asserts.assert_true(int(state_val) & _ALARM_BIT, "State does not have the OverPower bit set")

        self.step(8, "TH sends TestEventTrigger to clear the OverPower alarm condition",
                  expectation="SUCCESS.")
        state_sub.reset()
        event_sub.reset()
        await self.send_test_event_triggers(eventTrigger=_TRIGGER_CLEAR)

        if not is_latched:
            self.step(9, "If non-latched: TH awaits subscription report with the OverPower bit cleared",
                      expectation="Report received with the OverPower bit at 0.")

            def state_cleared(report):
                return not bool(report.value & _ALARM_BIT)

            state_sub.await_all_expected_report_matches(
                [AttributeMatcher.from_callable("State has the OverPower bit cleared", state_cleared)],
                timeout_sec=30)

            self.step(10, "If non-latched: TH waits up to 30 seconds for Notify event",
                      expectation="Notify received. Inactive has the OverPower bit, Active does not, State does not.")
            clear_event = event_sub.wait_for_event_report(cluster.Events.Notify, timeout_sec=30)
            asserts.assert_true(clear_event.inactive & _ALARM_BIT,
                                "Notify event: OverPower not set in Inactive on clear")
            asserts.assert_false(clear_event.active & _ALARM_BIT,
                                 "Notify event: OverPower unexpectedly set in Active on clear")
            asserts.assert_false(clear_event.state & _ALARM_BIT,
                                 "Notify event: OverPower still set in State on clear")

            self.step(11, "If latched: TH reads State",
                      expectation="The OverPower bit remains set until Reset.")
            self.mark_current_step_skipped()

            self.step(12, "If latched and Reset supported: TH sends Reset for the OverPower alarm",
                      expectation="SUCCESS. Subscription report received with the OverPower bit cleared.")
            self.mark_current_step_skipped()
        else:
            self.step(9, "If non-latched: TH awaits subscription report with the OverPower bit cleared",
                      expectation="Report received with the OverPower bit at 0.")
            self.mark_current_step_skipped()

            self.step(10, "If non-latched: TH waits up to 30 seconds for Notify event",
                      expectation="Notify received. Inactive has the OverPower bit, Active does not, State does not.")
            self.mark_current_step_skipped()

            self.step(11, "If latched: TH reads State",
                      expectation="The OverPower bit remains set until Reset.")
            state_val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.State)
            asserts.assert_true(int(state_val) & _ALARM_BIT,
                                "Latched alarm: OverPower should still be set in State after the condition clears")

            self.step(12, "If latched and Reset supported: TH sends Reset for the OverPower alarm",
                      expectation="SUCCESS. Subscription report received with the OverPower bit cleared.")
            accepted_cmds = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.AcceptedCommandList)
            if cmds.Reset.command_id in accepted_cmds:
                state_sub.reset()
                await self.send_single_cmd(cmd=cmds.Reset(alarms=_ALARM_BIT), endpoint=endpoint)

                def state_cleared_after_reset(report):
                    return not bool(report.value & _ALARM_BIT)

                state_sub.await_all_expected_report_matches(
                    [AttributeMatcher.from_callable("State cleared after Reset", state_cleared_after_reset)],
                    timeout_sec=30)
            else:
                self.mark_current_step_skipped()

        self.step(13, "TH sends the all-alarms-cleared TestEventTrigger",
                  expectation="SUCCESS. State reads back as 0.")
        await self.send_test_event_triggers(eventTrigger=_TRIGGER_ALL_CLEAR)
        # Read rather than await a report: by this point State may already be 0, and a DUT does
        # not report an unchanged value.
        final_state = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.State)
        asserts.assert_equal(int(final_state), 0, "State should be 0 after the cleanup trigger")


if __name__ == "__main__":
    default_matter_test_main()
