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

import test_plan_support
from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches


class TC_I_2_4(MatterBaseTest):
    def desc_TC_I_2_4(self) -> str:
        return "[TC-I-2.4] Reporting requirements with server as DUT"

    def steps_TC_I_2_4(self) -> list[TestStep]:
        THwrite = "TH writes to the DUT the"
        THcommand = "TH sends the command"
        return [TestStep(1, test_plan_support.commission_if_required(), is_commissioning=True),
                TestStep(2, "Set up a subscription wildcard subscription for the Identify Cluster, with MinIntervalFloor set to 0, MaxIntervalCeiling set to 30 and KeepSubscriptions set to false",
                         "Subscription successfully established"),
                TestStep(3, f"{THwrite} IdentifyTime attribute to 30.", test_plan_support.verify_success()),
                TestStep(4, "TH waits for the report and verifies the received report has a value of 30",
                         "The received report is equal to 30 (+/- 1)"),
                TestStep(5, f"{THwrite} IdentifyTime attribute to 35.", test_plan_support.verify_success()),
                TestStep(6, "TH waits for the report and verifies the received report has a value of 35",
                         "The received report is equal to 35 (+/- 1)"),
                TestStep(7, f"{THwrite} IdentifyTime attribute to 5.", test_plan_support.verify_success()),
                TestStep(8, "TH waits for the report and verifies the received report has a value of 5",
                         "The received report is equal to 5 (+/- 1)"),
                TestStep(9, "TH waits for the report and verifies the received report has a value of 0",
                         "The received report is equal to 0"),
                TestStep(10, f"{THwrite} IdentifyTime attribute to 35.", test_plan_support.verify_success()),
                TestStep(11, "TH waits for the report and verifies the received report has a value of 35",
                         "The received report is equal to 35 (+/- 1)"),
                TestStep(12, f"{THwrite} IdentifyTime attribute to 0.", test_plan_support.verify_success()),
                TestStep(13, "TH waits for the report and verifies the received report has a value of 0",
                         "The received report is equal to 0"),
                TestStep(14, f"{THcommand} Identify command with the value of IdentifyTime field set to 30",
                         test_plan_support.verify_success()),
                TestStep(15, "TH waits for the report and verifies the received report has a value of 30",
                         "The received report is equal to 30 (+/- 1)"),
                TestStep(16, f"{THcommand} Identify command with the value of IdentifyTime field set to 35",
                         test_plan_support.verify_success()),
                TestStep(17, "TH waits for the report and verifies the received report has a value of 35",
                         "The received report is equal to 35 (+/- 1)"),
                TestStep(18, f"{THcommand} Identify command with the value of IdentifyTime field set to 5",
                         test_plan_support.verify_success()),
                TestStep(19, "TH waits for the report and verifies the received report has a value of 5",
                         "The received report is equal to 5 (+/- 1)"),
                TestStep(20, "TH waits for the report and verifies the received report has a value of 0",
                         "The received report is equal to 0"),
                TestStep(21, f"{THcommand} Identify command with the value of IdentifyTime field set to 35",
                         test_plan_support.verify_success()),
                TestStep(22, "TH waits for the report and verifies the received report has a value of 35",
                         "The received report is equal to 35 (+/- 1)"),
                TestStep(23, f"{THcommand} Identify command with the value of IdentifyTime field set to 0",
                         test_plan_support.verify_success()),
                TestStep(24, "TH waits for the report and verifies the received report has a value of 0",
                         "The received report is equal to 0"),
                ]

    def pics_TC_I_2_4(self) -> list[str]:
        return [
            "I.S",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.Identify))
    async def test_TC_I_2_4(self):
        endpoint = self.get_endpoint()

        # Commissioning - already done
        self.step(1)

        cluster = Clusters.Identify

        self.step(2)
        sub_handler = AttributeSubscriptionHandler(expected_cluster=cluster, expected_attribute=cluster.Attributes.IdentifyTime)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint, max_interval_sec=30)

        # Verify Q requirements for IdentifyTime attribute by write to IdentifyTime

        self.step(3)
        await self.write_single_attribute(cluster.Attributes.IdentifyTime(30), endpoint_id=endpoint, expect_success=True)

        self.step(4)
        sub_handler.wait_for_attribute_report(timeout_sec=30)

        # Verify the received value is as expected
        asserts.assert_almost_equal(sub_handler.attribute_reports[cluster.Attributes.IdentifyTime][0].value,
                                    30, delta=1, msg="Received unexpected value for IdentifyTime")
        sub_handler.reset()

        self.step(5)
        await self.write_single_attribute(cluster.Attributes.IdentifyTime(35), endpoint_id=endpoint, expect_success=True)

        self.step(6)
        sub_handler.wait_for_attribute_report(timeout_sec=30)

        # Verify the received value is as expected
        asserts.assert_almost_equal(sub_handler.attribute_reports[cluster.Attributes.IdentifyTime][0].value,
                                    35, delta=1, msg="Received unexpected value for IdentifyTime")
        sub_handler.reset()

        self.step(7)
        await self.write_single_attribute(cluster.Attributes.IdentifyTime(5), endpoint_id=endpoint, expect_success=True)

        self.step(8)
        sub_handler.wait_for_attribute_report(timeout_sec=30)

        # Verify the received value is as expected
        asserts.assert_almost_equal(sub_handler.attribute_reports[cluster.Attributes.IdentifyTime][0].value,
                                    5, delta=1, msg="Received unexpected value for IdentifyTime")
        sub_handler.reset()

        self.step(9)
        logging.info("Test waits for report of IdentifyTime at 0")
        sub_handler.wait_for_attribute_report(timeout_sec=30)

        asserts.assert_equal(sub_handler.attribute_reports[cluster.Attributes.IdentifyTime]
                             [0].value, 0, "Received unexpected value for IdentifyTime")
        sub_handler.reset()

        self.step(10)
        await self.write_single_attribute(cluster.Attributes.IdentifyTime(35), endpoint_id=endpoint, expect_success=True)

        self.step(11)
        sub_handler.wait_for_attribute_report(timeout_sec=30)

        # Verify the received value is as expected
        asserts.assert_almost_equal(sub_handler.attribute_reports[cluster.Attributes.IdentifyTime][0].value,
                                    35, delta=1, msg="Received unexpected value for IdentifyTime")
        sub_handler.reset()

        self.step(12)
        await self.write_single_attribute(cluster.Attributes.IdentifyTime(0), endpoint_id=endpoint, expect_success=True)

        self.step(13)
        sub_handler.wait_for_attribute_report(timeout_sec=30)

        # Verify the received value is as expected
        asserts.assert_equal(sub_handler.attribute_reports[cluster.Attributes.IdentifyTime][0].value,
                             0, msg="Received unexpected value for IdentifyTime")
        sub_handler.reset()

        # Verify Q requirements for IdentifyTime attribute by invoke of Identify command

        self.step(14)
        try:
            await self.send_single_cmd(cmd=cluster.Commands.Identify(identifyTime=30), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        self.step(15)
        sub_handler.wait_for_attribute_report(timeout_sec=30)

        # Verify the received value is as expected
        asserts.assert_almost_equal(sub_handler.attribute_reports[cluster.Attributes.IdentifyTime][0].value,
                                    30, delta=1, msg="Received unexpected value for IdentifyTime")
        sub_handler.reset()

        self.step(16)
        try:
            await self.send_single_cmd(cmd=cluster.Commands.Identify(identifyTime=35), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        self.step(17)
        sub_handler.wait_for_attribute_report(timeout_sec=30)

        # Verify the received value is as expected
        asserts.assert_almost_equal(sub_handler.attribute_reports[cluster.Attributes.IdentifyTime][0].value,
                                    35, delta=1, msg="Received unexpected value for IdentifyTime")
        sub_handler.reset()

        self.step(18)
        try:
            await self.send_single_cmd(cmd=cluster.Commands.Identify(identifyTime=5), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        self.step(19)
        sub_handler.wait_for_attribute_report(timeout_sec=30)

        # Verify the received value is as expected
        asserts.assert_almost_equal(sub_handler.attribute_reports[cluster.Attributes.IdentifyTime][0].value,
                                    5, delta=1, msg="Received unexpected value for IdentifyTime")
        sub_handler.reset()

        self.step(20)
        logging.info("Test waits for report of IdentifyTime at 0")
        sub_handler.wait_for_attribute_report(timeout_sec=30)

        asserts.assert_equal(sub_handler.attribute_reports[cluster.Attributes.IdentifyTime]
                             [0].value, 0, "Received unexpected value for IdentifyTime")
        sub_handler.reset()

        self.step(21)
        try:
            await self.send_single_cmd(cmd=cluster.Commands.Identify(identifyTime=35), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        self.step(22)

        sub_handler.wait_for_attribute_report(timeout_sec=30)

        # Verify the received value is as expected
        asserts.assert_almost_equal(sub_handler.attribute_reports[cluster.Attributes.IdentifyTime][0].value,
                                    35, delta=1, msg="Received unexpected value for IdentifyTime")
        sub_handler.reset()

        self.step(23)
        try:
            await self.send_single_cmd(cmd=cluster.Commands.Identify(identifyTime=0), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        self.step(24)
        sub_handler.wait_for_attribute_report(timeout_sec=30)

        # Verify the received value is as expected
        asserts.assert_equal(sub_handler.attribute_reports[cluster.Attributes.IdentifyTime][0].value,
                             0, msg="Received unexpected value for IdentifyTime")
        sub_handler.reset()


if __name__ == "__main__":
    default_matter_test_main()
