#
#    Copyright (c) 2025 Project CHIP Authors
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
#     app: ${EVSE_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     app-ready-pattern: "APP STATUS: Starting event loop"
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
#   run2:
#     app: ${ALL_DEVICES_APP}
#     app-args: --device electrical-sensor:1 --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body, pics
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_PWRTL_2_1(MatterTestCommissionedDevice):

    @property
    def default_endpoint(self) -> int:
        return 1

    @pics('PWRTL.S')
    @async_test_body
    async def test_TC_PWRTL_2_1(self):
        """[TC-PWRTL-2.1] Attributes with DUT as Server

        This test case verifies the primary functionality of the Power Topology Cluster server.
        """
        endpoint = self.get_endpoint()
        cluster = Clusters.PowerTopology
        attributes = cluster.Attributes

        self.step(1, "Commission DUT to TH (already done)", is_commissioning=True)

        attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.AttributeList)

        self.step(2, "TH reads from the DUT the AvailableEndpoints attribute.",
                  expectation="Verify that the DUT response contains a list of endpoint-no entries. "
                              "Verify that the list has no more than 20 entries.")
        avail_eps = None
        if attributes.AvailableEndpoints.attribute_id in attribute_list:
            avail_eps = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.AvailableEndpoints)
            asserts.assert_less_equal(len(avail_eps), 20,
                                      "AvailableEndpoints must have no more than 20 entries")
            log.info("AvailableEndpoints: %s", avail_eps)
        else:
            self.mark_current_step_skipped()

        self.step(3, "TH reads from the DUT the ActiveEndpoints attribute.",
                  expectation="Verify that the DUT response contains a list of endpoint-no entries. "
                              "Verify that the list has no more than 20 entries. Verify that the list "
                              "is a subset of AvailableEndpoints.")
        if attributes.ActiveEndpoints.attribute_id in attribute_list:
            active_eps = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.ActiveEndpoints)
            asserts.assert_less_equal(len(active_eps), 20,
                                      "ActiveEndpoints must have no more than 20 entries")
            if avail_eps is not None:
                for ep in active_eps:
                    asserts.assert_in(ep, avail_eps,
                                      f"ActiveEndpoint {ep} is not in AvailableEndpoints")
            log.info("ActiveEndpoints: %s", active_eps)
        else:
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
