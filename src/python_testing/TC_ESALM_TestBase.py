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

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, MatterBaseTest
from matter.testing.runner import TestStep

log = logging.getLogger(__name__)

cluster = Clusters.ElectricalAlarm
AlarmBitmap = cluster.Bitmaps.AlarmBitmap

# The complete TestEventTrigger table for this cluster, from the PIXIT Variable Values section
# of the Electrical Alarm test plan. Keeping every code in one place means a case file names
# its alarm and nothing else, and the table can be checked against the plan in one read.
#
# The all-alarms-cleared code differs from a per-alarm clear: it drops latched alarms too,
# which is what the cleanup step needs and what a per-alarm clear deliberately does not do.
TRIGGER_ALL_CLEAR = 0x00A1000000000000

_BASE = 0x00A1000000000000

# alarm name -> (simulate, clear)
TRIGGERS: dict[str, tuple[int, int]] = {
    "OverVoltage": (_BASE | 0x01, _BASE | 0x02),
    "UnderVoltage": (_BASE | 0x03, _BASE | 0x04),
    "OverFrequency": (_BASE | 0x05, _BASE | 0x06),
    "UnderFrequency": (_BASE | 0x07, _BASE | 0x08),
    "OverPower": (_BASE | 0x09, _BASE | 0x0A),
    "UnderPower": (_BASE | 0x0B, _BASE | 0x0C),
    "OverCurrent": (_BASE | 0x0D, _BASE | 0x0E),
    "UnderCurrent": (_BASE | 0x0F, _BASE | 0x10),
    "PowerImport": (_BASE | 0x11, _BASE | 0x12),
    "PowerExport": (_BASE | 0x13, _BASE | 0x14),
}


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

    def alarm_lifecycle_steps(self, alarm_name: str, alarm_bit: int) -> list[TestStep]:
        """The step table of esalm_alarm_testcase.adoc, rendered for one alarm.

        Descriptions and expectations reproduce the plan's text with its macros expanded, so a
        test house can match a step in the log to the same-numbered row of the test plan.
        """
        bit = int(alarm_bit).bit_length() - 1
        trig_set, trig_clear = (f"0x{c:016X}" for c in TRIGGERS[alarm_name])
        trigger_cmd = ("TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 "
                       "with EnableKey field set to PIXIT.ESALM.TEST_EVENT_TRIGGER_KEY and EventTrigger "
                       "field set to PIXIT.ESALM.TEST_EVENT_TRIGGER")
        notify_cleared = (f"Receive Notify event. The Inactive field has bit {bit} set. The Active field "
                          f"does not have bit {bit} set. The State field does not have bit {bit} set. "
                          "The Mask field equals the current Mask attribute value.")
        report_cleared = f"Subscription report received with bit {bit} = 0 in State."
        return [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster.",
                     "Value has to be 1 (True). If 0, skip the remaining steps and end the test case."),
            TestStep(3, "TH reads from the DUT the Supported. Store the value as Supported.",
                     f"Verify that the DUT response contains a map32 AlarmBitmap with bit {bit} set "
                     "(alarm is supported)."),
            TestStep("3a", "TH reads from the DUT the Mask. Store the value as Mask.",
                     f"Verify that the DUT response contains a map32 AlarmBitmap with bit {bit} set "
                     "(alarm is enabled in Mask)."),
            TestStep("3b", "TH reads from the DUT the Latch. Store the value as Latch.",
                     "Verify that the DUT response contains a map32 AlarmBitmap. Record whether bit "
                     f"{bit} is 0 or 1 as Latch for use in steps 9-12b."),
            TestStep("3c", "TH reads from the DUT the State. Store the value as InitialState.",
                     "Verify that the DUT response contains a map32 AlarmBitmap. Bit "
                     f"{bit} is 0 (alarm not yet active before test begins)."),
            TestStep(4, "TH establishes a subscription to State with MinIntervalFloor=0 and "
                     "MaxIntervalCeiling=30.",
                     "Subscription is established successfully."),
            TestStep("4a", "TH awaits subscription report of an initial priming report for State.",
                     "Priming report received carrying the current State value."),
            TestStep(5, f"{trigger_cmd} with EventTrigger set to {trig_set} to simulate the "
                     f"{alarm_name} alarm condition.",
                     "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep("5a", f"TH awaits subscription report of a State value with bit {bit} set.",
                     f"Subscription report received with bit {bit} set in State."),
            TestStep(6, "TH waits up to 30 seconds for a Notify event.",
                     f"Receive Notify event. The Active field has bit {bit} set. The Inactive field does "
                     f"not have bit {bit} set. The State field has bit {bit} set. The Mask field equals "
                     "the current Mask attribute value."),
            TestStep(7, "TH reads from the DUT the State.",
                     f"Verify that the DUT response contains a map32 AlarmBitmap with bit {bit} set."),
            TestStep(8, f"{trigger_cmd} with EventTrigger set to {trig_clear} to clear the "
                     f"{alarm_name} alarm condition.",
                     "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(9, f"IF bit {bit} of Latch is 0 (non-latched): TH awaits subscription report of a "
                     f"State value with bit {bit} cleared. Otherwise skip to step 11.",
                     report_cleared),
            TestStep(10, f"IF bit {bit} of Latch is 0 (non-latched): TH waits up to 30 seconds for a "
                     "Notify event. Otherwise skip to step 11.",
                     notify_cleared),
            TestStep(11, f"IF bit {bit} of Latch is 1 (latched): TH reads from the DUT the State. "
                     "Otherwise skip to step 12c.",
                     f"Bit {bit} remains set in State (latched alarm persists until Reset)."),
            TestStep(12, f"IF bit {bit} is latched and Reset is supported: TH sends command Reset with "
                     f"bit {bit} set in the Alarms field. Otherwise skip to step 12c.",
                     "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep("12a", f"IF bit {bit} is latched and Reset is supported: TH awaits subscription "
                     f"report of a State value with bit {bit} cleared. Otherwise skip to step 12c.",
                     report_cleared),
            TestStep("12b", f"IF bit {bit} is latched and Reset is supported: TH waits up to 30 seconds "
                     "for a Notify event. Otherwise skip to step 12c.",
                     notify_cleared),
            TestStep("12c", "TH reads from the DUT the State.",
                     f"Verify that the DUT response contains a map32 AlarmBitmap with bit {bit} = 0."),
            TestStep(13, f"{trigger_cmd} for All Alarms Test Event Clear "
                     "(PIXIT.ESALM.TEST_EVENT_TRIGGER = 0x00A1_0000_0000_0000).",
                     "Verify DUT responds w/ status SUCCESS(0x00)."),
        ]

    async def run_alarm_lifecycle_test(self, alarm_name: str, alarm_bit: int) -> None:
        """Run the shared alarm lifecycle. Trigger codes are looked up from TRIGGERS."""
        asserts.assert_in(alarm_name, TRIGGERS, f"No TestEventTrigger codes defined for {alarm_name}")
        trigger_set, trigger_clear = TRIGGERS[alarm_name]
        endpoint = self.get_endpoint()
        attrs = cluster.Attributes
        cmds = cluster.Commands

        # Commissioning; declared as is_commissioning=True in alarm_lifecycle_steps, so the
        # framework performs it and this call only advances to it.
        self.step(1)

        self.step(2)
        gen_diag = Clusters.GeneralDiagnostics
        if not await self.read_single_attribute_check_success(
                endpoint=0, cluster=gen_diag, attribute=gen_diag.Attributes.TestEventTriggersEnabled):
            self.mark_all_remaining_steps_skipped(3)
            return

        # The plan makes the case not applicable, rather than failed, when the alarm is absent from
        # Supported or disabled in Mask: "Steps 3 and 3a verify that bit {ALARM_BIT} is set in
        # Supported and enabled in Mask before proceeding. If not, the test case is not applicable."
        # Mask is writable server state, so a server may legitimately ship with an alarm disabled.
        self.step(3)
        supported = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Supported)
        if not int(supported) & alarm_bit:
            log.info("%s is not set in Supported; the test case is not applicable", alarm_name)
            self.mark_all_remaining_steps_skipped("3a")
            return

        self.step("3a")
        mask = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.Mask)
        if not int(mask) & alarm_bit:
            log.info("%s is not enabled in Mask; the test case is not applicable", alarm_name)
            self.mark_all_remaining_steps_skipped("3b")
            return

        self.step("3b")
        attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.AttributeList)
        latch = 0
        if attrs.Latch.attribute_id in attribute_list:
            latch = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.Latch)
        is_latched = bool(int(latch) & alarm_bit)

        self.step("3c")
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

        self.step("4a")
        # AttributeSubscriptionHandler.start() registers its callback only after ReadAttribute
        # returns, so the priming report never reaches the queue. Read for the baseline instead;
        # every later report here is change-driven and does arrive.
        asserts.assert_false(await self.read_state(endpoint) & alarm_bit,
                             f"Priming value of State already has the {alarm_name} bit set")

        self.step(5)
        state_sub.reset()
        event_sub.reset()
        await self.send_test_event_trigger(trigger_set)

        self.step("5a")
        state_sub.await_all_expected_report_matches(
            [AttributeMatcher.from_callable(f"State has the {alarm_name} bit set",
                                            lambda report: bool(report.value & alarm_bit))],
            timeout_sec=30)

        self.step(6)
        notify_event = event_sub.wait_for_event_report(cluster.Events.Notify, timeout_sec=30)
        asserts.assert_true(notify_event.active & alarm_bit, f"Notify: {alarm_name} not set in Active")
        asserts.assert_false(notify_event.inactive & alarm_bit, f"Notify: {alarm_name} set in Inactive")
        asserts.assert_true(notify_event.state & alarm_bit, f"Notify: {alarm_name} not set in State")
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

        cleared = AttributeMatcher.from_callable(f"State has the {alarm_name} bit cleared",
                                                 lambda report: not bool(report.value & alarm_bit))
        has_reset = False

        if not is_latched:
            self.step(9)
            state_sub.await_all_expected_report_matches([cleared], timeout_sec=30)

            self.step(10)
            clear_event = event_sub.wait_for_event_report(cluster.Events.Notify, timeout_sec=30)
            asserts.assert_true(clear_event.inactive & alarm_bit,
                                f"Notify: {alarm_name} not set in Inactive on clear")
            asserts.assert_false(clear_event.active & alarm_bit,
                                 f"Notify: {alarm_name} set in Active on clear")
            asserts.assert_false(clear_event.state & alarm_bit,
                                 f"Notify: {alarm_name} still set in State on clear")
            asserts.assert_equal(clear_event.mask, current_mask,
                                 "Notify event Mask does not match the Mask attribute")

            for skipped in (11, 12, "12a", "12b"):
                self.step(skipped)
                self.mark_current_step_skipped()
        else:
            for skipped in (9, 10):
                self.step(skipped)
                self.mark_current_step_skipped()

            self.step(11)
            asserts.assert_true(await self.read_state(endpoint) & alarm_bit,
                                f"Latched alarm: {alarm_name} should still be set after the condition clears")

            accepted_cmds = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attrs.AcceptedCommandList)
            has_reset = cmds.Reset.command_id in accepted_cmds

            self.step(12)
            if has_reset:
                state_sub.reset()
                event_sub.reset()
                await self.send_single_cmd(cmd=cmds.Reset(alarms=alarm_bit), endpoint=endpoint)
            else:
                self.mark_current_step_skipped()

            self.step("12a")
            if has_reset:
                state_sub.await_all_expected_report_matches([cleared], timeout_sec=30)
            else:
                self.mark_current_step_skipped()

            self.step("12b")
            if has_reset:
                reset_event = event_sub.wait_for_event_report(cluster.Events.Notify, timeout_sec=30)
                asserts.assert_true(reset_event.inactive & alarm_bit,
                                    f"Notify: {alarm_name} not set in Inactive on Reset")
                asserts.assert_false(reset_event.active & alarm_bit,
                                     f"Notify: {alarm_name} set in Active on Reset")
                asserts.assert_false(reset_event.state & alarm_bit,
                                     f"Notify: {alarm_name} still set in State on Reset")
                asserts.assert_equal(reset_event.mask, current_mask,
                                     "Notify event Mask does not match the Mask attribute")
            else:
                self.mark_current_step_skipped()

        # Latch and the Reset command share the RESET feature as their conformance, so an alarm can
        # only be latched on a DUT that also offers Reset. The alarm has therefore been cleared by
        # one path or the other by now, and a DUT that still reports it set is not conformant.
        self.step("12c")
        asserts.assert_false(await self.read_state(endpoint) & alarm_bit,
                             f"{alarm_name} should be 0 in State by this point")

        self.step(13)
        await self.send_test_event_trigger(TRIGGER_ALL_CLEAR)
