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
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_PWRTL_2_1(MatterBaseTest):

    def pics_TC_PWRTL_2_1(self) -> list[str]:
        return ["PWRTL.S"]

    @async_test_body
    async def test_TC_PWRTL_2_1(self):

        attributes = Clusters.PowerTopology.Attributes

        endpoint = self.user_params.get("endpoint", 1)

        self.print_step(1, "Commissioning, already done")

        if not self.check_pics("PWRTL.S.A0000"):
            logging.info("Test skipped because PICS PWRTL.S.A0000 is not set")
            return

        self.print_step(2, "Read AvailableAttributes attribute")
        available_endpoints = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=Clusters.Objects.PowerTopology, attribute=attributes.AvailableEndpoints)

        asserts.assert_less_equal(len(available_endpoints), 20,
                                  "AvailableEndpoints length %d must be less than 21!" % len(available_endpoints))

        if not self.check_pics("PWRTL.S.A0001"):
            logging.info("Test skipped because PICS PWRTL.S.A0001 is not set")
            return

        self.print_step(3, "Read ActiveEndpoints attribute")
        active_endpoints = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=Clusters.Objects.PowerTopology,  attribute=attributes.ActiveEndpoints)
        logging.info("ActiveEndpoints: %s" % (active_endpoints))
        asserts.assert_less_equal(len(active_endpoints), 20,
                                  "ActiveEndpoints length %d must be less than 21!" % len(active_endpoints))
        # Verify that ActiveEndpoints is a subset of AvailableEndpoints
        asserts.assert_true(set(active_endpoints).issubset(set(available_endpoints)),
                            "ActiveEndpoints should be a subset of AvailableEndpoints")


if __name__ == "__main__":
    default_matter_test_main()
