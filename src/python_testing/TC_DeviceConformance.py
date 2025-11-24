#
#    Copyright (c) 2023 Project CHIP Authors
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
#     app: ${CHIP_LOCK_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --manual-code 10054912339
#       --bool-arg ignore_in_progress:True allow_provisional:True
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --tests test_TC_IDM_10_2 test_TC_IDM_10_6 test_TC_DESC_2_3 test_TC_IDM_14_1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

# TODO: Enable 10.5 in CI once the door lock OTA requestor problem is sorted.
from test_testing.DeviceConformanceTests import DeviceConformanceTests

from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_DeviceConformance(DeviceConformanceTests, MatterBaseTest):
    @async_test_body
    async def setup_class(self):
        super().setup_class()
        await self.setup_class_helper()

    @async_test_body
    async def teardown_class(self):
        super().teardown_class()

    def test_TC_IDM_10_2(self):
        # TODO: Turn this off after TE2
        # https://github.com/project-chip/connectedhomeip/issues/34615
        ignore_in_progress = self.user_params.get("ignore_in_progress", True)
        allow_provisional = self.user_params.get("allow_provisional", False)
        success, problems = self.check_conformance(ignore_in_progress, self.is_pics_sdk_ci_only, allow_provisional)
        self.problems.extend(problems)
        if not success:
            self.fail_current_test("Problems with conformance")

    def test_TC_IDM_10_3(self):
        ignore_in_progress = self.user_params.get("ignore_in_progress", False)
        success, problems = self.check_revisions(ignore_in_progress)
        self.problems.extend(problems)
        if not success:
            self.fail_current_test("Problems with cluster revision on at least one cluster")

    def test_TC_IDM_10_5(self):
        fail_on_extra_clusters = self.user_params.get("fail_on_extra_clusters", True)
        allow_provisional = self.user_params.get("allow_provisional", False)
        success, problems = self.check_device_type(fail_on_extra_clusters, allow_provisional)
        self.problems.extend(problems)
        if not success:
            self.fail_current_test("Problems with Device type conformance on one or more endpoints")

    def test_TC_IDM_10_6(self):
        success, problems = self.check_device_type_revisions()
        self.problems.extend(problems)
        if not success:
            self.fail_current_test("Problems with Device type revisions on one or more endpoints")

    def steps_TC_IDM_14_1(self):
        return [TestStep(0, "TH performs a wildcard read of all attributes and endpoints on the device"),
                TestStep(1, """ For each root-node-restricted cluster in the list, ensure the cluster does not appear on any endpoint that is not the root node.
                                List of root-node-restricted clusters:

                                * ACL
                                * Time Synchronization
                                * TLS Certificate Management
                                * TLS Client Management
                         """, "No root-node-restricted clusters appear on non-root endpoints"),
                TestStep(2, "Ensure the complex device type composition and conformance rules related to closure device types are met",
                         "Closure cluster device type rules are met"),
                TestStep(3, "Ensure the rules related to semantic tags for the closure device types are met",
                         "Closure semantic tags rules are met")
                ]

    def test_TC_IDM_14_1(self):
        self.step(0)  # wildcard read - done in setup
        self.step(1)
        problems = self.check_root_node_restricted_clusters()
        self.step(2)
        problems.extend(self.check_closure_restricted_clusters())
        self.step(3)
        problems.extend(self.check_closure_restricted_sem_tags())

        if problems:
            self.problems.extend(problems)
            self.fail_current_test("One or more device conformance violations were found")

    def steps_TC_DESC_2_3(self):
        return [TestStep(0, "TH performs a wildcard read of all attributes on all endpoints on the device"),
                TestStep(1, "TH checks the Root node endpoint and ensures no application device types are listed",
                         "No Application device types on EP0"),
                TestStep(2, "For each non-root endpoint on the device, TH checks the DeviceTypeList of the Descriptor cluster and verifies that all the listed application device types are part of the same superset, and that no two device types are unrelated supersets of any device type."),
                TestStep(3, "Fail test if either of the above steps failed.")]
        # TODO: add check that at least one endpoint has an application endpoint or an aggregator

    def desc_TC_DESC_2_3(self):
        return "[TC-DESC-2.3] Test for superset application device types"

    def test_TC_DESC_2_3(self):
        self.step(0)  # done in setup class
        problems = []

        self.step(1)
        problems.extend(self.check_root_endpoint_for_application_device_types())

        self.step(2)
        problems.extend(self.check_all_application_device_types_superset())

        self.step(3)
        self.problems.extend(problems)
        if problems:
            self.fail_current_test("One or more application device type endpoint violations")


if __name__ == "__main__":
    default_matter_test_main()
