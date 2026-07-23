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
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --endpoint 0
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

from mobly import asserts
import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main
from itertools import count

import logging
log = logging.getLogger(__name__)

# Auto-generated from test specification: [TC-HSTAT-2.4] Optional functionality with DUT as Server
class TC_HSTAT_2_4(MatterBaseTest):

    def pics_TC_HSTAT_2_4(self) -> list[str]:
        return [
            "HSTAT.S",
        ]

    def desc_TC_HSTAT_2_4(self) -> str:
        return "[TC-HSTAT-2.4] Optional functionality with DUT as Server"

    def steps_TC_HSTAT_2_4(self):
        return [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).", ""),
            TestStep(2, "TH sends command On to the On/Off cluster on the same endpoint as this cluster.", "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(3, "TH sends command SetSettings with the Mode field set to Humidifier or Dehumidifier", "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(4, "TH sends command SetSettings with the Continuous, Sleep, and Optimal fields set to False", "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(5, "Individually subscribe to the attributes Continuous if supported, Sleep if supported, and Optimal if supported.", "This will receive updates when these attributes change value."),
            TestStep(5, "TH sends command SetSettings with the Continuous field set to True.", "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(6, "TH reads from the DUT the Continuous attribute.", "Verify that the DUT response contains a value of True"),
            TestStep(7, "TH writes to the DUT the Continuous attribute with False.", "Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Continuous with the first having a value of True and the second having a value of False."),
            TestStep(8, "TH sends command SetSettings with the Sleep field set to True.", "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(9, "TH reads from the DUT the Sleep attribute.", "Verify that the DUT response contains a value of True"),
            TestStep(10, "TH writes to the DUT the Sleep attribute with False.", "Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Sleep with the first having a value of True and the second having a value of False."),
            TestStep(11, "TH sends command SetSettings with the Optimal field set to True.", "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(12, "TH reads from the DUT the Optimal attribute.", "Verify that the DUT response contains a value of True"),
            TestStep(13, "TH writes to the DUT the Optimal attribute with False.", "Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Optimal with the first having a value of True and the second having a value of False."),
            TestStep(14, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowContinuous event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(15, "TH sends command SetSettings with the Continuous field set to True", "Verify DUT responds w/ status INVALID_IN_STATE(0xcb)"),
            TestStep(16, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowContinuous event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(17, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowSleep event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(18, "TH sends command SetSettings with the Sleep field set to True", "Verify DUT responds w/ status INVALID_IN_STATE(0xcb)"),
            TestStep(19, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowSleep event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(20, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowOptimal event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(21, "TH sends command SetSettings with the Optimal field set to True", "Verify DUT responds w/ status INVALID_IN_STATE(0xcb)"),
            TestStep(22, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowOptimal event.", "Verify DUT responds w/ status SUCCESS(0x00)."),
        ]

    @async_test_body
    async def test_TC_HSTAT_2_4(self):
        self.step(1)
        # Commission DUT to TH (can be skipped if done in a preceding test).
        # 

        self.step(2)
        # TH sends command On to the On/Off cluster on the same endpoint as this cluster.
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(3)
        # TH sends command SetSettings with the Mode field set to Humidifier or Dehumidifier
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(4)
        # TH sends command SetSettings with the Continuous, Sleep, and Optimal fields set to False
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(5)
        # Individually subscribe to the attributes Continuous if supported, Sleep if supported, and Optimal if supported.
        # This will receive updates when these attributes change value.

        self.step(5)
        # TH sends command SetSettings with the Continuous field set to True.
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(6)
        # TH reads from the DUT the Continuous attribute.
        # Verify that the DUT response contains a value of True

        self.step(7)
        # TH writes to the DUT the Continuous attribute with False.
        # Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Continuous with the first having a value of True and the second having a value of False.

        self.step(8)
        # TH sends command SetSettings with the Sleep field set to True.
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(9)
        # TH reads from the DUT the Sleep attribute.
        # Verify that the DUT response contains a value of True

        self.step(10)
        # TH writes to the DUT the Sleep attribute with False.
        # Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Sleep with the first having a value of True and the second having a value of False.

        self.step(11)
        # TH sends command SetSettings with the Optimal field set to True.
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(12)
        # TH reads from the DUT the Optimal attribute.
        # Verify that the DUT response contains a value of True

        self.step(13)
        # TH writes to the DUT the Optimal attribute with False.
        # Verify DUT responds w/ status SUCCESS(0x00) Confirm 2 attribute reports for Optimal with the first having a value of True and the second having a value of False.

        self.step(14)
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowContinuous event.
        # Verify DUT responds w/ status SUCCESS(0x00).

        self.step(15)
        # TH sends command SetSettings with the Continuous field set to True
        # Verify DUT responds w/ status INVALID_IN_STATE(0xcb)

        self.step(16)
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowContinuous event.
        # Verify DUT responds w/ status SUCCESS(0x00).

        self.step(17)
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowSleep event.
        # Verify DUT responds w/ status SUCCESS(0x00).

        self.step(18)
        # TH sends command SetSettings with the Sleep field set to True
        # Verify DUT responds w/ status INVALID_IN_STATE(0xcb)

        self.step(19)
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowSleep event.
        # Verify DUT responds w/ status SUCCESS(0x00).

        self.step(20)
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for DisallowOptimal event.
        # Verify DUT responds w/ status SUCCESS(0x00).

        self.step(21)
        # TH sends command SetSettings with the Optimal field set to True
        # Verify DUT responds w/ status INVALID_IN_STATE(0xcb)

        self.step(22)
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.HSTAT.TEST_EVENT_TRIGGER for AllowOptimal event.
        # Verify DUT responds w/ status SUCCESS(0x00).


if __name__ == '__main__':
    default_matter_test_main()
