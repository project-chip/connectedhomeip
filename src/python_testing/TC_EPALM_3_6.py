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

from TC_EPALM_TestBase import ElectricalProtectionAlarmTestBaseHelper

import matter.clusters as Clusters
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.ElectricalProtectionAlarm
ALARM = cluster.Bitmaps.AlarmBitmap.kArcFault
ALARM_NAME = "ArcFault"
ALARM_BIT = 5


class TC_EPALM_3_6(ElectricalProtectionAlarmTestBaseHelper):

    def pics_TC_EPALM_3_6(self) -> list[str]:
        return ["EPALM.S", "EPALM.S.F25", "EPALM.S.E00"]

    def steps_TC_EPALM_3_6(self) -> list[TestStep]:
        return self.alarm_notify_steps(ALARM_BIT, ALARM_NAME)

    @run_if_endpoint_matches(has_feature(cluster, cluster.Bitmaps.Feature.kArcFault))
    async def test_TC_EPALM_3_6(self):
        """[TC-EPALM-3.6] Notify event for ArcFault with Server as DUT

        This test case verifies Notify event reporting and the corresponding State
        transitions for the ArcFault alarm of the Electrical Protection Alarm
        Cluster server.
        """
        await self.run_alarm_notify_test(ALARM, ALARM_NAME, ALARM_BIT)


if __name__ == "__main__":
    default_matter_test_main()
