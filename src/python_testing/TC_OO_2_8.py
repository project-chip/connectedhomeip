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
# FIXME: https://github.com/project-chip/connectedhomeip/issues/36885
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
#   run2:
#     app: ${ALL_DEVICES_APP}
#     app-args: --device on-off-light --discriminator 1234 --KVS kvs1
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body, pics
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_OO_2_8(MatterBaseTest):

    @pics('OO.S', 'OO.S.F00')
    @async_test_body
    async def test_TC_OO_2_8(self):
        """[TC-OO-2.8] Reporting requirements with (DUT as Server)"""
        # Commissioning - already done
        self.step(1, "Commission DUT if not already done", is_commissioning=True)

        self.step(2, "TH sends the command Off command")
        await self.send_single_cmd(Clusters.OnOff.Commands.Off())

        # NOTE: added this sleep to let the DUT have some time to move
        log.info("Test waits for 1 seconds")
        await asyncio.sleep(1)

        self.step(3, "Set up a subscription wildcard subscription for the On/Off Cluster, with MinIntervalFloor set to 0, MaxIntervalCeiling set to 30 and KeepSubscriptions set to false")
        sub_handler = AttributeSubscriptionHandler(expected_cluster=Clusters.OnOff)
        await sub_handler.start(self.default_controller, self.dut_node_id, self.get_endpoint(), min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        self.step(4, "TH sends the command OnWithTimedOff command with AcceptOnlyWhenOn field set to 0, OnTime field set to 5 (0.5s) and OffWaitTime set to 5 (0.5s)")
        cmd = Clusters.OnOff.Commands.OnWithTimedOff(onOffControl=0, onTime=5, offWaitTime=5)
        await self.send_single_cmd(cmd)

        self.step(5, "TH sends the command OnWithTimedOff command with AcceptOnlyWhenOn field set to 0, OnTime field set to 100 (10s) and OffWaitTime set to 20 (2s)")
        cmd = Clusters.OnOff.Commands.OnWithTimedOff(onOffControl=0, onTime=100, offWaitTime=20)
        await self.send_single_cmd(cmd)

        self.step(6, "Wait for 5 seconds")
        log.info("Test waits for 5 seconds")
        await asyncio.sleep(5)

        self.step(7, "TH sends the command OnWithTimedOff command with AcceptOnlyWhenOn field set to 0, OnTime field set to 150 (15s) and OffWaitTime set to 100 (10s)")
        cmd = Clusters.OnOff.Commands.OnWithTimedOff(onOffControl=0, onTime=150, offWaitTime=100)
        await self.send_single_cmd(cmd)

        self.step(8, "Wait for 20 seconds")
        log.info("Test waits for 20 seconds")
        await asyncio.sleep(20)

        self.step(9, "TH stores the reported values of OnTime in all incoming reports for OnTime attribute, that contains data in reportedOnTimeValuesList and verifies reportedOnTimeValueList contains three entries")
        count = sub_handler.attribute_report_counts[Clusters.OnOff.Attributes.OnTime]
        reportedOnTimeValueList = sub_handler.attribute_reports[Clusters.OnOff.Attributes.OnTime]
        asserts.assert_equal(count, 3, "Unexpected number of OnTime reports")

        self.step(10, "TH verifies the first entry in reportedOnTimeValueList is approximately 100")
        asserts.assert_less_equal(reportedOnTimeValueList[0].value, 100, "Unexpected first OnTime report")
        asserts.assert_greater_equal(reportedOnTimeValueList[0].value, 95, msg="Unexpected first OnTime report")

        self.step(11, "TH verifies the second entry in reportedOnTimeValueList is approximately 150")
        asserts.assert_less_equal(reportedOnTimeValueList[1].value, 150, "Unexpected second OnTime report")
        asserts.assert_greater_equal(reportedOnTimeValueList[1].value, 145, msg="Unexpected second OnTime report")

        self.step(12, "TH verifies the third entry in reportedOnTimeValueList is 0")
        asserts.assert_equal(reportedOnTimeValueList[2].value, 0, "Unexpected last OnTime report")

        self.step(13, "TH stores the reported values of OffWaitTime in all incoming reports for OffWaitTime attribute, that contains data in reportedOffWaitTimeValueList and verifies reportedOffWaitTimeValueList contains three entries")
        count = sub_handler.attribute_report_counts[Clusters.OnOff.Attributes.OffWaitTime]
        reportedOffWaitTimeValueList = sub_handler.attribute_reports[Clusters.OnOff.Attributes.OffWaitTime]
        asserts.assert_equal(count, 3, "Unexpected number of OffWaitTime reports")

        self.step(14, "TH verifies the first entry in reportedOffWaitTimeValueList is approximately 20")
        asserts.assert_less_equal(reportedOffWaitTimeValueList[0].value, 20, "Unexpected first OffWaitTime report")
        asserts.assert_greater_equal(reportedOffWaitTimeValueList[0].value, 15, msg="Unexpected first OffWaitTime report")

        self.step(15, "TH verifies the second entry in reportedOffWaitTimeValueList is approximately 100")
        asserts.assert_less_equal(reportedOffWaitTimeValueList[1].value, 100, "Unexpected second OffWaitTime report")
        asserts.assert_greater_equal(reportedOffWaitTimeValueList[1].value, 95, msg="Unexpected second OffWaitTime report")

        self.step(16, "TH verifies the third entry in reportedOffWaitTimeValueList is 0")
        asserts.assert_equal(reportedOffWaitTimeValueList[2].value, 0, "Unexpected last OffWaitTime report")

        self.step(17, "Clear the received reports on the subscription")
        sub_handler.reset()
        reportedOnTimeValueList = []
        reportedOffWaitTimeValueList = []

        self.step(18, "TH sends the command OnWithTimedOff command with AcceptOnlyWhenOn field set to 0, OnTime field set to 100 (10s) and OffWaitTime set to 50 (5s)")
        cmd = Clusters.OnOff.Commands.OnWithTimedOff(onOffControl=0, onTime=100, offWaitTime=50)
        await self.send_single_cmd(cmd)

        self.step(19, "Wait for 5 seconds")
        log.info("Test waits for 5 seconds")
        await asyncio.sleep(5)

        self.step(20, "TH sends the command Off command")
        await self.send_single_cmd(Clusters.OnOff.Commands.Off())

        self.step(21, "Wait for 10 seconds")
        log.info("Test waits for 10 seconds")
        await asyncio.sleep(10)

        self.step(22, "TH stores the reported values of OnTime in all incoming reports for OnTime attribute, that contains data in reportedOnTimeValuesList and verifies reportedOnTimeValueList contains two entries")
        count = sub_handler.attribute_report_counts[Clusters.OnOff.Attributes.OnTime]
        reportedOnTimeValueList = sub_handler.attribute_reports[Clusters.OnOff.Attributes.OnTime]
        asserts.assert_equal(count, 2, "Unexpected number of OnTime reports")

        self.step(23, "TH verifies the first entry in reportedOnTimeValueList is approximately 100")
        asserts.assert_less_equal(reportedOnTimeValueList[0].value, 100, "Unexpected first OnTime report")
        asserts.assert_greater_equal(reportedOnTimeValueList[0].value, 95, msg="Unexpected first OnTime report")

        self.step(24, "TH verifies the second entry in reportedOnTimeValueList is 0")
        asserts.assert_equal(reportedOnTimeValueList[1].value, 0, "Unexpected last OnTime report")

        self.step(25, "TH stores the reported values of OffWaitTime in all incoming reports for OffWaitTime attribute, that contains data in reportedOffWaitTimeValueList and verifies reportedOffWaitTimeValueList contains two entries")
        count = sub_handler.attribute_report_counts[Clusters.OnOff.Attributes.OffWaitTime]
        reportedOffWaitTimeValueList = sub_handler.attribute_reports[Clusters.OnOff.Attributes.OffWaitTime]
        asserts.assert_equal(count, 2, "Unexpected number of OffWaitTime reports")

        self.step(26, "TH verifies the first entry in reportedOffWaitTimeValueList is approximately 50")
        asserts.assert_less_equal(reportedOffWaitTimeValueList[0].value, 50, "Unexpected first OffWaitTime report")
        asserts.assert_greater_equal(reportedOffWaitTimeValueList[0].value, 45, msg="Unexpected first OffWaitTime report")

        self.step(27, "TH verifies the second entry in reportedOffWaitTimeValueList is 0")
        asserts.assert_equal(reportedOffWaitTimeValueList[1].value, 0, "Unexpected last OffWaitTime report")


if __name__ == "__main__":
    default_matter_test_main()
