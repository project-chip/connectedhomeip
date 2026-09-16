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
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep

log = logging.getLogger(__name__)

cluster = Clusters.ElectricalProtectionAlarm
AlarmBitmap = cluster.Bitmaps.AlarmBitmap


class ElectricalProtectionAlarmTestBaseHelper(MatterBaseTest):
    """Shared TestEventTrigger handling for the TC-EPALM cases.

    The trigger codes are namespaced by cluster id in the top two bytes. In the low byte, 0x00
    clears every alarm, 0x01 through 0x07 raise a single alarm, and 0x11 through 0x17 clear that
    same alarm. See ElectricalProtectionAlarmTestEventTriggerHandler.h.
    """

    test_event_clear_all = 0x00A3_0000_0000_0000

    _set_triggers = {
        AlarmBitmap.kShortCircuitFault: 0x00A3_0000_0000_0001,
        AlarmBitmap.kOverLoadFault: 0x00A3_0000_0000_0002,
        AlarmBitmap.kOverVoltageFault: 0x00A3_0000_0000_0003,
        AlarmBitmap.kVoltageSurgeFault: 0x00A3_0000_0000_0004,
        AlarmBitmap.kResidualCurrentFault: 0x00A3_0000_0000_0005,
        AlarmBitmap.kArcFault: 0x00A3_0000_0000_0006,
        AlarmBitmap.kSelfTest: 0x00A3_0000_0000_0007,
    }

    _clear_triggers = {
        AlarmBitmap.kShortCircuitFault: 0x00A3_0000_0000_0011,
        AlarmBitmap.kOverLoadFault: 0x00A3_0000_0000_0012,
        AlarmBitmap.kOverVoltageFault: 0x00A3_0000_0000_0013,
        AlarmBitmap.kVoltageSurgeFault: 0x00A3_0000_0000_0014,
        AlarmBitmap.kResidualCurrentFault: 0x00A3_0000_0000_0015,
        AlarmBitmap.kArcFault: 0x00A3_0000_0000_0016,
        AlarmBitmap.kSelfTest: 0x00A3_0000_0000_0017,
    }

    async def send_test_event_trigger_set_alarm(self, alarm: AlarmBitmap) -> None:
        """Raise a single alarm."""
        await self.send_test_event_triggers(eventTrigger=self._set_triggers[alarm])

    async def send_test_event_trigger_clear_alarm(self, alarm: AlarmBitmap) -> None:
        """Lower a single alarm, leaving the others untouched."""
        await self.send_test_event_triggers(eventTrigger=self._clear_triggers[alarm])

    async def send_test_event_trigger_clear_all(self) -> None:
        """Lower every alarm."""
        await self.send_test_event_triggers(eventTrigger=self.test_event_clear_all)

    async def read_state(self, endpoint: int) -> int:
        """Read the State attribute."""
        return int(await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.State))

    async def await_notify(self, sub: EventSubscriptionHandler,
                           timeout_sec: float = 10.0) -> cluster.Events.Notify:
        """Return the next Notify event, failing the test if it could not be decoded.

        wait_for_event_report returns a ValueDecodeFailure rather than raising when the payload
        will not decode, so the attribute check is load-bearing and not dead code.
        """
        report = sub.wait_for_event_report(cluster.Events.Notify, timeout_sec=timeout_sec)
        asserts.assert_true(hasattr(report, "active") and hasattr(report, "state"),
                            f"Notify event did not decode into an event object: {report}")
        return report

    def alarm_notify_steps(self, alarm_bit: int, alarm_name: str) -> list[TestStep]:
        """The nine steps of the parameterized TC-EPALM-3.x Notify case.

        Mirrors epalm_alarm_testcase.adoc, which the plan includes once per alarm with
        ALARM_NAME and ALARM_BIT substituted. Expectations are that file's text with the
        macros expanded.
        """
        success = "Verify DUT responds w/ status SUCCESS(0x00)."
        awaits = "TH awaits subscription report of"
        return [
            TestStep("1a", "Commission DUT to TH (already done)", is_commissioning=True),
            TestStep("1b", "TH sets up a subscription to the Notify event"),
            TestStep("1c", "TH reads the Supported attribute",
                     f"Bit {alarm_bit} of the response SHALL be 1."),
            TestStep("1d", "TH reads TestEventTriggersEnabled from General Diagnostics on endpoint 0",
                     "Verify that TestEventTriggersEnabled has a value of 1 (True)."),
            TestStep("1e", f"TH sends the TestEventTrigger clearing {alarm_name}, returning the DUT "
                     "to its no-fault baseline", success),
            TestStep("2a", f"TH sends the TestEventTrigger setting {alarm_name}",
                     f"{success} {awaits} a Notify event with bit {alarm_bit} set in Active and set in State."),
            TestStep("2b", "TH reads the State attribute",
                     f"Bit {alarm_bit} of the response SHALL be 1."),
            TestStep("3a", f"TH sends the TestEventTrigger clearing {alarm_name}",
                     f"{success} {awaits} a Notify event with bit {alarm_bit} set in Inactive and clear in State."),
            TestStep("3b", "TH reads the State attribute",
                     f"Bit {alarm_bit} of the response SHALL be 0."),
        ]

    async def run_alarm_notify_test(self, alarm: AlarmBitmap, alarm_name: str, alarm_bit: int) -> None:
        """Run the parameterized TC-EPALM-3.x body for one alarm."""
        endpoint = self.get_endpoint()
        mask = int(alarm)

        self.step("1a")

        self.step("1b")
        sub = EventSubscriptionHandler(expected_cluster=cluster)
        await sub.start(self.default_controller, self.dut_node_id, endpoint)

        try:
            self.step("1c")
            supported = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Supported)
            asserts.assert_true(int(supported) & mask,
                                f"Supported must set bit {alarm_bit} for {alarm_name}")

            self.step("1d")
            await self.check_test_event_triggers_enabled()

            self.step("1e")
            state_before = await self.read_state(endpoint)
            await self.send_test_event_trigger_clear_alarm(alarm)
            if state_before & mask:
                # Already raised, so this trigger emits an inactive Notify. Consume it here rather
                # than letting it arrive after the flush, where step 2a would read it instead of
                # the set-transition it is waiting for.
                report = await self.await_notify(sub)
                asserts.assert_true(int(report.inactive) & mask,
                                    f"Baseline clear must report {alarm_name} in Inactive")
            asserts.assert_false(await self.read_state(endpoint) & mask,
                                 f"{alarm_name} must be clear before the set trigger")
            sub.flush_events()

            self.step("2a")
            await self.send_test_event_trigger_set_alarm(alarm)
            report = await self.await_notify(sub)
            log.info("Notify on set: active=0x%02X inactive=0x%02X state=0x%02X",
                     report.active, report.inactive, report.state)
            asserts.assert_true(int(report.active) & mask,
                                f"Notify.Active must set bit {alarm_bit} when {alarm_name} is raised")
            asserts.assert_true(int(report.state) & mask,
                                f"Notify.State must set bit {alarm_bit} when {alarm_name} is raised")

            self.step("2b")
            asserts.assert_true(await self.read_state(endpoint) & mask,
                                f"State bit {alarm_bit} must be 1 after the set trigger")

            self.step("3a")
            await self.send_test_event_trigger_clear_alarm(alarm)
            report = await self.await_notify(sub)
            log.info("Notify on clear: active=0x%02X inactive=0x%02X state=0x%02X",
                     report.active, report.inactive, report.state)
            asserts.assert_true(int(report.inactive) & mask,
                                f"Notify.Inactive must set bit {alarm_bit} when {alarm_name} is cleared")
            asserts.assert_false(int(report.state) & mask,
                                 f"Notify.State must clear bit {alarm_bit} when {alarm_name} is cleared")

            self.step("3b")
            asserts.assert_false(await self.read_state(endpoint) & mask,
                                 f"State bit {alarm_bit} must be 0 after the clear trigger")
        finally:
            # Leave the DUT in its no-fault baseline even if a step above failed. A second clear
            # after step 3a is a no-op: no transition means no report.
            try:
                await self.send_test_event_trigger_clear_alarm(alarm)
            except Exception:  # noqa: BLE001 - never mask the original failure
                log.exception("Could not clear %s during cleanup", alarm_name)
            sub.cancel()
