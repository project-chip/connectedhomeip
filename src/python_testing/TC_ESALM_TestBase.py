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

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, MatterBaseTest
from matter.testing.runner import TestStep

cluster = Clusters.ElectricalAlarm
AlarmBitmap = cluster.Bitmaps.AlarmBitmap

# All-alarms-cleared trigger from the PIXIT Variable Values table of the test plan. Unlike a
# per-alarm clear, this one drops latched alarms too, which is what the cleanup step needs.
TRIGGER_ALL_CLEAR = 0x00A1000000000000


class ElectricalAlarmTestBaseHelper(MatterBaseTest):
    """Shared procedure for TC-ESALM-3.1 through 3.10.

    The test plan defines those cases as one alarm lifecycle per alarm class, all including the
    same procedure with per-case parameters. This mirrors that: the steps and the body live here
    once, and each test case supplies its alarm, its bit, and its two trigger codes.
    """

    async def send_test_event_trigger(self, code: int) -> None:
        await self.send_test_event_triggers(eventTrigger=code)

    async def read_state(self, endpoint: int) -> int:
        return int(await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.State))

    def alarm_lifecycle_steps(self, alarm_name: str) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "TH reads TestEventTriggersEnabled from General Diagnostics",
                     "Value is 1 (True). If 0, skip remaining steps."),
            TestStep(3, "TH reads Supported, Mask, Latch, and State",
                     f"SUCCESS. The {alarm_name} bit is set in Supported and enabled in Mask."),
            TestStep(4, "TH establishes subscription to State with MinIntervalFloor=0, MaxIntervalCeiling=30",
                     "Subscription established; initial priming report received."),
            TestStep(5, f"TH sends TestEventTrigger to simulate the {alarm_name} alarm condition",
                     f"SUCCESS. Subscription report received with the {alarm_name} bit set in State."),
            TestStep(6, "TH waits up to 30 seconds for Notify event",
                     f"Notify received. Active has the {alarm_name} bit, Inactive does not, State does, Mask matches."),
            TestStep(7, "TH reads State",
                     f"DUT returns an AlarmBitmap with the {alarm_name} bit set."),
            TestStep(8, f"TH sends TestEventTrigger to clear the {alarm_name} alarm condition", "SUCCESS."),
            TestStep(9, f"If non-latched: TH awaits subscription report with the {alarm_name} bit cleared",
                     f"Report received with the {alarm_name} bit at 0."),
            TestStep(10, "If non-latched: TH waits up to 30 seconds for Notify event",
                     f"Notify received. Inactive has the {alarm_name} bit, Active does not, State does not."),
            TestStep(11, "If latched: TH reads State",
                     f"The {alarm_name} bit remains set until Reset."),
            TestStep(12, f"If latched and Reset supported: TH sends Reset for the {alarm_name} alarm",
                     f"SUCCESS. Subscription report received with the {alarm_name} bit cleared."),
            TestStep(13, "TH sends the all-alarms-cleared TestEventTrigger", "SUCCESS. State reads back as 0."),
        ]

    async def run_alarm_lifecycle_test(self, alarm_name: str, alarm_bit: int,
                                       trigger_set: int, trigger_clear: int) -> None:
        endpoint = self.get_endpoint()
        attrs = cluster.Attributes
        cmds = cluster.Commands

        self.step(1)

        self.step(2)
        gen_diag = Clusters.GeneralDiagnostics
        triggers_enabled = await self.read_single_attribute_check_success(
            endpoint=0, cluster=gen_diag, attribute=gen_diag.Attributes.TestEventTriggersEnabled)
        if not triggers_enabled:
            self.mark_all_remaining_steps_skipped(3)
            return

        self.step(3)
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

        asserts.assert_true(int(supported) & alarm_bit,
                            f"{alarm_name} is not set in Supported, so this alarm cannot be tested")
        asserts.assert_true(int(mask) & alarm_bit,
                            f"{alarm_name} is not enabled in Mask, so the alarm would be suppressed")

        is_latched = bool(int(latch) & alarm_bit)

        # A trigger only reports if it changes State, so the alarm must start inactive.
        asserts.assert_false(await self.read_state(endpoint) & alarm_bit,
                             f"{alarm_name} is already active before the test starts")

        self.step(4)
        state_sub = AttributeSubscriptionHandler(cluster, attrs.State)
        await state_sub.start(self.default_controller, self.dut_node_id,
                              endpoint=endpoint, min_interval_sec=0,
                              max_interval_sec=30, keepSubscriptions=False)
        event_sub = EventSubscriptionHandler(expected_cluster=cluster)
        await event_sub.start(self.default_controller, self.dut_node_id,
                              endpoint=endpoint, min_interval_sec=0, max_interval_sec=30)

        self.step(5)
        state_sub.reset()
        event_sub.reset()
        await self.send_test_event_trigger(trigger_set)
        state_sub.await_all_expected_report_matches(
            [AttributeMatcher.from_callable(f"State has the {alarm_name} bit set",
                                            lambda report: bool(report.value & alarm_bit))],
            timeout_sec=30)

        self.step(6)
        notify_event = event_sub.wait_for_event_report(cluster.Events.Notify, timeout_sec=30)
        asserts.assert_true(notify_event.active & alarm_bit, f"Notify event: {alarm_name} not set in Active")
        asserts.assert_false(notify_event.inactive & alarm_bit, f"Notify event: {alarm_name} set in Inactive")
        asserts.assert_true(notify_event.state & alarm_bit, f"Notify event: {alarm_name} not set in State")
        current_mask = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
        asserts.assert_equal(notify_event.mask, current_mask,
                             "Notify event Mask does not match the Mask attribute")

        self.step(7)
        asserts.assert_true(await self.read_state(endpoint) & alarm_bit,
                            f"State does not have the {alarm_name} bit set")

        self.step(8)
        state_sub.reset()
        event_sub.reset()
        await self.send_test_event_trigger(trigger_clear)

        if not is_latched:
            self.step(9)
            state_sub.await_all_expected_report_matches(
                [AttributeMatcher.from_callable(f"State has the {alarm_name} bit cleared",
                                                lambda report: not bool(report.value & alarm_bit))],
                timeout_sec=30)

            self.step(10)
            clear_event = event_sub.wait_for_event_report(cluster.Events.Notify, timeout_sec=30)
            asserts.assert_true(clear_event.inactive & alarm_bit,
                                f"Notify event: {alarm_name} not set in Inactive on clear")
            asserts.assert_false(clear_event.active & alarm_bit,
                                 f"Notify event: {alarm_name} set in Active on clear")
            asserts.assert_false(clear_event.state & alarm_bit,
                                 f"Notify event: {alarm_name} still set in State on clear")

            self.step(11)
            self.mark_current_step_skipped()
            self.step(12)
            self.mark_current_step_skipped()
        else:
            self.step(9)
            self.mark_current_step_skipped()
            self.step(10)
            self.mark_current_step_skipped()

            self.step(11)
            asserts.assert_true(await self.read_state(endpoint) & alarm_bit,
                                f"Latched alarm: {alarm_name} should still be set after the condition clears")

            self.step(12)
            accepted_cmds = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.AcceptedCommandList)
            if cmds.Reset.command_id in accepted_cmds:
                state_sub.reset()
                await self.send_single_cmd(cmd=cmds.Reset(alarms=alarm_bit), endpoint=endpoint)
                state_sub.await_all_expected_report_matches(
                    [AttributeMatcher.from_callable("State cleared after Reset",
                                                    lambda report: not bool(report.value & alarm_bit))],
                    timeout_sec=30)
            else:
                self.mark_current_step_skipped()

        self.step(13)
        await self.send_test_event_trigger(TRIGGER_ALL_CLEAR)
        # Read rather than await a report: State may already be 0 by now, and a DUT does not
        # report an unchanged value.
        asserts.assert_equal(await self.read_state(endpoint), 0,
                             "State should be 0 after the cleanup trigger")
