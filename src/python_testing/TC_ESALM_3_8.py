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

from TC_ESALM_TestBase import AlarmBitmap, ElectricalAlarmTestBaseHelper, cluster

from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main

ALARM_NAME = "UnderCurrent"
ALARM_BIT = int(AlarmBitmap.kUnderCurrent)

# Set and clear codes for this alarm, from the PIXIT Variable Values table of the test plan.
TRIGGER_SET = 0x00A100000000000F
TRIGGER_CLEAR = 0x00A1000000000010


class TC_ESALM_3_8(ElectricalAlarmTestBaseHelper):

    def pics_TC_ESALM_3_8(self) -> list[str]:
        return ["ESALM.S", "ESALM.S.F28", "ESALM.S.E0000"]

    def steps_TC_ESALM_3_8(self) -> list[TestStep]:
        return self.alarm_lifecycle_steps(ALARM_NAME)

    @run_if_endpoint_matches(has_feature(cluster, cluster.Bitmaps.Feature.kUnderCurrent))
    async def test_TC_ESALM_3_8(self):
        """[TC-ESALM-3.8] UnderCurrent alarm lifecycle with Server as DUT

        Verifies the lifecycle of the UnderCurrent alarm: the condition sets its bit in State and
        delivers a subscription report, a Notify event carries the correct fields, a latched
        alarm persists until Reset, and a non-latched alarm clears when the condition goes away.
        """
        await self.run_alarm_lifecycle_test(ALARM_NAME, ALARM_BIT, TRIGGER_SET, TRIGGER_CLEAR)


if __name__ == "__main__":
    default_matter_test_main()
