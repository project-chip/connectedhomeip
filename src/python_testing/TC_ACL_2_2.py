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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     factoryreset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_ACL_2_2(MatterBaseTest):

    def desc_TC_ACL_2_2(self) -> str:
        return "[TC-ACL-2.2] Cluster endpoint"

    def steps_TC_ACL_2_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH1 reads DUT Descriptor cluster ServerList attribute from Endpoint 0"),
            TestStep(3, "TH1 reads DUT Descriptor cluster ServerList attribute from every Endpoint except 0"),
        ]
        return steps

    @async_test_body
    async def test_TC_ACL_2_2(self):
        self.step(1)
        self.step(2)
        data = await self.default_controller.ReadAttribute(nodeid=self.dut_node_id, attributes=[(Clusters.Descriptor.Attributes.ServerList)])
        asserts.assert_true(Clusters.AccessControl.id in data[0][Clusters.Descriptor]
                            [Clusters.Descriptor.Attributes.ServerList], "ACL cluster not on EP0")
        self.step(3)
        for endpoint, ep_data in data.items():
            if endpoint == 0:
                continue
            asserts.assert_false(Clusters.AccessControl.id in ep_data[Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList],
                                 f"ACL cluster incorrectly present on endpoint {endpoint}")


if __name__ == "__main__":
    default_matter_test_main()
