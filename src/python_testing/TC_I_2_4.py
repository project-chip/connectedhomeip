#
#    Copyright (c) 2024 Project CHIP Authors
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
# === END CI TEST ARGUMENTS ===

import logging
import time

import chip.clusters as Clusters
import test_plan_support
from chip.interaction_model import InteractionModelError, Status
from chip.testing.event_attribute_reporting import ClusterAttributeChangeAccumulator
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts


class TC_I_2_4(MatterBaseTest):
    def desc_TC_I_2_4(self) -> str:
        return "[TC-I-2.4] Reporting requirements with server as DUT"

    def steps_TC_I_2_4(self) -> list[TestStep]:
        THwrite = "TH writes to the DUT the"
        THcommand = "TH sends the command"
        return [TestStep(1, test_plan_support.commission_if_required(), is_commissioning=True),
                TestStep(2, "Set up a subscription wildcard subscription for the Identify Cluster, with MinIntervalFloor set to 0, MaxIntervalCeiling set to 30 and KeepSubscriptions set to false",
                         "Subscription successfully established"),
                TestStep(3, "TH stores the reported values of IdentifyTime in all incoming reports for IdentifyTime attribute, that contains data in reportedIdentifyTimeValuesList, over a period of 15 seconds."),
                TestStep(4, f"{THwrite} IdentifyTime attribute to 5.", test_plan_support.verify_success()),
                TestStep(5, f"{THwrite} IdentifyTime attribute to 10.", test_plan_support.verify_success()),
                TestStep(6, "Wait for 12 seconds"),
                TestStep(7, "TH verifies reportedIdentifyTimeValuesList contains three entries",
                         "reportedIdentifyTimeValuesList has 3 entries in the list"),
                TestStep(8, "TH verifies the first entry in reportedIdentifyTimeValuesList is 5",
                         "The first entry in reportedIdentifyTimeValuesList is equal to 5 (+/- 1)"),
                TestStep(9, "TH verifies the second entry in reportedIdentifyTimeValuesList is 10",
                         "The second entry in reportedIdentifyTimeValuesList is equal to 10 (+/- 1)"),
                TestStep(10, "TH verifies the third entry in reportedIdentifyTimeValuesList is 0",
                         "The third entry in reportedIdentifyTimeValuesList is equal to 0"),
                TestStep(11, "TH clears the reportedIdentifyTimeValuesList"),
                TestStep(12, f"{THwrite} IdentifyTime attribute to 5.", test_plan_support.verify_success()),
                TestStep(13, f"{THwrite} IdentifyTime attribute to 0.", test_plan_support.verify_success()),
                TestStep(14, "Wait for 1 seconds"),
                TestStep(15, "TH verifies reportedIdentifyTimeValuesList contains two entries",
                         "reportedIdentifyTimeValuesList has 2 entries in the list"),
                TestStep(16, "TH verifies the first entry in reportedIdentifyTimeValuesList is 5",
                         "The first entry in reportedIdentifyTimeValuesList is equal to 5 (+/- 1)"),
                TestStep(17, "TH verifies the second entry in reportedIdentifyTimeValuesList is 0",
                         "The second entry in reportedIdentifyTimeValuesList is equal to 0"),
                TestStep(18, "TH clears the reportedIdentifyTimeValuesList"),
                TestStep(19, f"{THcommand} Identify command with the value of IdentifyTime field set to 5",
                         test_plan_support.verify_success()),
                TestStep(20, f"{THcommand} Identify command with the value of IdentifyTime field set to 10",
                         test_plan_support.verify_success()),
                TestStep(21, "Wait for 12 seconds"),
                TestStep(22, "TH verifies reportedIdentifyTimeValuesList contains three entries",
                         "reportedIdentifyTimeValuesList has 3 entries in the list"),
                TestStep(23, "TH verifies the first entry in reportedIdentifyTimeValuesList is 5",
                         "The first entry in reportedIdentifyTimeValuesList is equal to 5 (+/- 1)"),
                TestStep(24, "TH verifies the second entry in reportedIdentifyTimeValuesList is 10"),
                TestStep(25, "TH verifies the third entry in reportedIdentifyTimeValuesList is 0",
                         "The third entry in reportedIdentifyTimeValuesList is equal to 0"),
                TestStep(26, "TH clears the reportedIdentifyTimeValuesList"),
                TestStep(27, f"{THcommand} Identify command with the value of IdentifyTime field set to 5",
                         test_plan_support.verify_success()),
                TestStep(28, f"{THcommand} Identify command with the value of IdentifyTime field set to 0",
                         test_plan_support.verify_success()),
                TestStep(29, "Wait for 1 seconds"),
                TestStep(30, "TH verifies reportedIdentifyTimeValuesList contains two entries",
                         "reportedIdentifyTimeValuesList has 2 entries in the list"),
                TestStep(31, "TH verifies the first entry in reportedIdentifyTimeValuesList is 5",
                         "The first entry in reportedIdentifyTimeValuesList is equal to 5 (+/- 1)"),
                TestStep(32, "TH verifies the second entry in reportedIdentifyTimeValuesList is 0",
                         "The second entry in reportedIdentifyTimeValuesList is equal to 0"),
                ]

    def pics_TC_I_2_4(self) -> list[str]:
        pics = [
            "I.S",
        ]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.Identify))
    async def test_TC_I_2_4(self):
        endpoint = self.get_endpoint(default=1)

        # Commissioning - already done
        self.step(1)

        cluster = Clusters.Identify

        self.step(2)
        sub_handler = ClusterAttributeChangeAccumulator(cluster)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint)

        # Verify Q requirements for IdentifyTime attribute by write to IdentifyTime

        self.step(3)
        reportedIdentifyTimeValuesList = []

        self.step(4)
        result = await self.write_single_attribute(cluster.Attributes.IdentifyTime(5), endpoint_id=endpoint)
        asserts.assert_equal(result, Status.Success, "Error when trying to write a IdentifyTime value")

        self.step(5)
        result = await self.write_single_attribute(cluster.Attributes.IdentifyTime(10), endpoint_id=endpoint)
        asserts.assert_equal(result, Status.Success, "Error when trying to write a IdentifyTime value")

        self.step(6)
        logging.info("Test waits for 12 seconds")
        time.sleep(12)

        self.step(7)
        count = sub_handler.attribute_report_counts[cluster.Attributes.IdentifyTime]
        asserts.assert_equal(count, 3, "Unexpected number of IdentifyTime reports")

        self.step(8)
        reportedIdentifyTimeValuesList = sub_handler.attribute_reports[cluster.Attributes.IdentifyTime]
        logging.info(f'IdentifyTime reports: {reportedIdentifyTimeValuesList}')
        asserts.assert_almost_equal(reportedIdentifyTimeValuesList[0].value, 5, delta=1, msg="Unexpected first IdentifyTime report")

        self.step(9)
        asserts.assert_almost_equal(reportedIdentifyTimeValuesList[1].value,
                                    10, delta=1, msg="Unexpected second IdentifyTime report")

        self.step(10)
        asserts.assert_equal(reportedIdentifyTimeValuesList[2].value, 0, "Unexpected last IdentifyTime report")

        self.step(11)
        # reports are stored by the handler, so just reset so we get a clean look
        sub_handler.reset()
        reportedIdentifyTimeValuesList.clear()

        self.step(12)
        result = await self.write_single_attribute(cluster.Attributes.IdentifyTime(5), endpoint_id=endpoint)
        asserts.assert_equal(result, Status.Success, "Error when trying to write a IdentifyTime value")

        self.step(13)
        result = await self.write_single_attribute(cluster.Attributes.IdentifyTime(0), endpoint_id=endpoint)
        asserts.assert_equal(result, Status.Success, "Error when trying to write a IdentifyTime value")

        self.step(14)
        logging.info("Test waits for 1 seconds")
        time.sleep(1)

        self.step(15)
        count = sub_handler.attribute_report_counts[cluster.Attributes.IdentifyTime]
        asserts.assert_equal(count, 2, "Unexpected number of IdentifyTime reports")

        self.step(16)
        reportedIdentifyTimeValuesList = sub_handler.attribute_reports[cluster.Attributes.IdentifyTime]
        logging.info(f'IdentifyTime reports: {reportedIdentifyTimeValuesList}')
        asserts.assert_almost_equal(reportedIdentifyTimeValuesList[0].value, 5, delta=1, msg="Unexpected first IdentifyTime report")

        self.step(17)
        asserts.assert_equal(reportedIdentifyTimeValuesList[1].value, 0, "Unexpected last IdentifyTime report")

        # Verify Q requirements for IdentifyTime attribute by invoke of Identify command

        self.step(18)
        # reports are stored by the handler, so just reset so we get a clean look
        sub_handler.reset()
        reportedIdentifyTimeValuesList.clear()

        self.step(19)
        try:
            await self.send_single_cmd(cmd=cluster.Commands.Identify(identifyTime=5), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(20)
        try:
            await self.send_single_cmd(cmd=cluster.Commands.Identify(identifyTime=10), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(21)
        logging.info("Test waits for 12 seconds")
        time.sleep(12)

        self.step(22)
        count = sub_handler.attribute_report_counts[cluster.Attributes.IdentifyTime]
        asserts.assert_equal(count, 3, "Unexpected number of IdentifyTime reports")

        self.step(23)
        reportedIdentifyTimeValuesList = sub_handler.attribute_reports[cluster.Attributes.IdentifyTime]
        logging.info(f'IdentifyTime reports: {reportedIdentifyTimeValuesList}')
        asserts.assert_almost_equal(reportedIdentifyTimeValuesList[0].value, 5, delta=1, msg="Unexpected first IdentifyTime report")

        self.step(24)
        asserts.assert_almost_equal(reportedIdentifyTimeValuesList[1].value,
                                    10, delta=1, msg="Unexpected second IdentifyTime report")

        self.step(25)
        asserts.assert_equal(reportedIdentifyTimeValuesList[2].value, 0, "Unexpected last IdentifyTime report")

        self.step(26)
        # reports are stored by the handler, so just reset so we get a clean look
        sub_handler.reset()
        reportedIdentifyTimeValuesList.clear()

        self.step(27)
        try:
            await self.send_single_cmd(cmd=cluster.Commands.Identify(identifyTime=5), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(28)
        try:
            await self.send_single_cmd(cmd=cluster.Commands.Identify(identifyTime=0), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(29)
        logging.info("Test waits for 1 seconds")
        time.sleep(1)

        self.step(30)
        count = sub_handler.attribute_report_counts[cluster.Attributes.IdentifyTime]
        asserts.assert_equal(count, 2, "Unexpected number of IdentifyTime reports")

        self.step(31)
        reportedIdentifyTimeValuesList = sub_handler.attribute_reports[cluster.Attributes.IdentifyTime]
        logging.info(f'IdentifyTime reports: {reportedIdentifyTimeValuesList}')
        asserts.assert_almost_equal(reportedIdentifyTimeValuesList[0].value, 5, delta=1, msg="Unexpected first IdentifyTime report")

        self.step(32)
        asserts.assert_equal(reportedIdentifyTimeValuesList[1].value, 0, "Unexpected last IdentifyTime report")


if __name__ == "__main__":
    default_matter_test_main()
