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
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest

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

    async def await_notify(self, sub: EventSubscriptionHandler, timeout_sec: float = 10.0):
        """Return the next Notify event, failing the test if it could not be decoded.

        wait_for_event_report returns a ValueDecodeFailure rather than raising when the payload
        will not decode, so the attribute check is load-bearing and not dead code.
        """
        report = sub.wait_for_event_report(cluster.Events.Notify, timeout_sec=timeout_sec)
        asserts.assert_true(hasattr(report, "active") and hasattr(report, "state"),
                            f"Notify event did not decode into an event object: {report}")
        return report
