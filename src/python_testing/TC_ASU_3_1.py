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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_DEVICES_APP}
#     app-args: --device ambient-context-sensor:1 --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/asu_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --app-pipe /tmp/asu_fifo
#       --bool-arg simulate_ambientsensing:true
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body, has_cluster, pics, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

# Script Function Call Example
# rm -rf admin_storage1.json && rm -rf kvs1 &&
# python3 ./scripts/tests/run_python_test.py --app out/linux-x64-all-devices-clang/all-devices-app
# --factory-reset --app-args "--device ambient-context-sensor:1 --KVS kvs1 --discriminator 1234 --app-pipe /tmp/asu_fifo"
# --script src/python_testing/TC_ASU_3_1.py
# --script-args "--storage-path admin_storage1.json --discriminator 1234 --passcode 20202021
#   --commissioning-method on-network --endpoint 1 --app-pipe /tmp/asu_fifo --bool-arg simulate_ambientsensing:true"


class TC_ASU_3_1(MatterBaseTest):

    def setup_test(self):
        super().setup_test()
        self.is_ci = self.matter_test_config.global_test_params.get('simulate_ambientsensing', False)

    @pics('ASU.S')
    @async_test_body
    @run_if_endpoint_matches(has_cluster(Clusters.AmbientSensingUnion))
    async def test_TC_ASU_3_1(self):
        """[TC-ASU-3.1] Cluster endpoint"""

        node_id = self.dut_node_id
        dev_ctrl = self.default_controller
        endpoint = self.get_endpoint()
        cluster = Clusters.AmbientSensingUnion
        attr = Clusters.AmbientSensingUnion.Attributes
        ci_wait_time = 3.0

        self.step("1", "Commissioning, already done", is_commissioning=True)

        self.step("2", "TH establishes a wildcard subscription to all attributes on Ambient Sensing Union Cluster on the endpoint under test.")
        attrib_listener = AttributeSubscriptionHandler(expected_cluster=cluster)
        await attrib_listener.start(dev_ctrl, node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        # CI test values
        contnode_str_1 = "0x123456789ABCDEF"
        contend_1 = 1234
        contstatus_online = Clusters.AmbientSensingUnion.Enums.UnionContributorStatusEnum.kUnionContributorOnline

        contnode_str_2 = "0x123456789AAAAAA"
        contnode_2 = int(contnode_str_2, 16)
        contend_2 = 1235
        contstatus_2 = Clusters.AmbientSensingUnion.Enums.UnionContributorStatusEnum.kUnionContributorOnline

        self.step("3", "Change UnionName attribute.")
        union_name_write = "TestUnionName"
        await self.write_single_attribute(attr.UnionName(union_name_write), endpoint_id=endpoint)
        # Allow time for the subscription report to arrive
        await asyncio.sleep(ci_wait_time)

        self.step("4", "TH awaits a ReportDataMessage containing an attribute report for UnionName attribute. Verify that the value of UnionName attribute reflects the change made in step 3.")
        reports = attrib_listener.attribute_reports.get(cluster.Attributes.UnionName)
        asserts.assert_true(reports is not None and len(reports) > 0,
                            "No subscription report received for UnionName after writing.")
        union_name_sub = reports[-1].value
        asserts.assert_equal(union_name_sub, union_name_write,
                             "UnionName subscription report does not reflect the written value.")
        log.info("Verified UnionName subscription report: %s", union_name_sub)
        attrib_listener.reset()

        self.step("5", "Change UnionHealth attribute by adding an offline contributor to affect the union health.")
        # Read the current UnionHealth before the mutation so we can verify the report reflects the change.
        union_health_before = await self.read_single_attribute_check_success(
            cluster=cluster, attribute=attr.UnionHealth, endpoint=endpoint)
        log.info("UnionHealth before contributor add: %s", union_health_before)
        # Adding an online contributor is expected to transition health to kFullyFunctional.
        expected_union_health = Clusters.AmbientSensingUnion.Enums.UnionHealthEnum.kFullyFunctional
        # UnionHealth is read-only and derived from contributor statuses. Adding an online contributor
        # will cause the cluster to recalculate and report a new UnionHealth value.
        if self.is_ci:
            self.write_to_app_pipe({
                "Name": "AddAmbientSensingContributor",
                "EndpointId": endpoint,
                "NodeId": contnode_str_1,
                "ContributorEndpointId": contend_1,
                "Status": contstatus_online.value,
            })
            await asyncio.sleep(ci_wait_time)
        else:
            self.wait_for_user_input(
                prompt_msg="Change the UnionHealth attribute (e.g. by adding/removing contributors), then type any letter and press ENTER.")

        self.step("6", "TH awaits a ReportDataMessage containing an attribute report for UnionHealth attribute. Verify that the value of UnionHealth attribute reflects the change made in step 5.")
        reports = attrib_listener.attribute_reports.get(cluster.Attributes.UnionHealth)
        asserts.assert_true(reports is not None and len(reports) > 0,
                            "No subscription report received for UnionHealth after the change.")
        union_health_sub = reports[-1].value
        log.info("UnionHealth subscription report after contributor add: %s", union_health_sub)
        asserts.assert_equal(union_health_sub, expected_union_health,
                             f"UnionHealth subscription report ({union_health_sub}) does not match expected value "
                             f"({expected_union_health}) after adding an online contributor.")
        attrib_listener.reset()

        self.step("7", "Change UnionContributorList attribute by adding a contributor.")
        if self.is_ci:
            self.write_to_app_pipe({
                "Name": "AddAmbientSensingContributor",
                "EndpointId": endpoint,
                "NodeId": contnode_str_2,
                "ContributorEndpointId": contend_2,
                "Status": contstatus_2.value,
            })
            await asyncio.sleep(ci_wait_time)
        else:
            self.wait_for_user_input(
                prompt_msg="Add a contributor to UnionContributorList, then type any letter and press ENTER.")

        self.step("8", "TH awaits a ReportDataMessage containing an attribute report for UnionContributorList attribute. Verify that the value of UnionContributorList attribute reflects the contributor added in step 7.")
        reports = attrib_listener.attribute_reports.get(cluster.Attributes.UnionContributorList)
        asserts.assert_true(reports is not None and len(reports) > 0,
                            "No subscription report received for UnionContributorList after adding contributor.")
        reported_list = reports[-1].value

        exist_flag = False
        for contributor in reported_list:
            if contributor.contributorNodeID != Clusters.Types.NullValue and contributor.contributorNodeID == contnode_2:
                asserts.assert_equal(contributor.contributorEndpointID, contend_2,
                                     "ContributorEndpointID does not match the added contributor.")
                asserts.assert_equal(contributor.contributorStatus, contstatus_2,
                                     "ContributorStatus does not match the added contributor.")
                exist_flag = True

        asserts.assert_true(exist_flag, "The added contributor is not found in the UnionContributorList subscription report.")
        attrib_listener.reset()

        self.step("9", "Change UnionContributorList attribute by removing a contributor.")
        if self.is_ci:
            self.write_to_app_pipe({
                "Name": "RemoveAmbientSensingContributor",
                "EndpointId": endpoint,
                "NodeId": contnode_str_2,
                "ContributorEndpointId": contend_2,
            })
            await asyncio.sleep(ci_wait_time)
        else:
            self.wait_for_user_input(
                prompt_msg="Remove a contributor from UnionContributorList, then type any letter and press ENTER.")

        self.step("10", "TH awaits a ReportDataMessage containing an attribute report for UnionContributorList attribute. Verify that the value of UnionContributorList attribute reflects the contributor removed in step 9.")
        reports = attrib_listener.attribute_reports.get(cluster.Attributes.UnionContributorList)
        asserts.assert_true(reports is not None and len(reports) > 0,
                            "No subscription report received for UnionContributorList after removing contributor.")
        reported_list = reports[-1].value

        for contributor in reported_list:
            if contributor.contributorNodeID != Clusters.Types.NullValue and contributor.contributorNodeID == contnode_2:
                asserts.fail(
                    f"Removed contributor (NodeID={contnode_str_2}) is still found in UnionContributorList subscription report.")

        log.info("Verified removed contributor is absent from UnionContributorList subscription report.")
        attrib_listener.reset()


if __name__ == "__main__":
    default_matter_test_main()
