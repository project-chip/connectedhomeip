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
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --endpoint 0
#       --networkID ens4
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

import chip.clusters as Clusters
import test_plan_support
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts
from chip.testing import matter_asserts


class TC_CNET_4_3(MatterBaseTest):

    def desc_TC_CNET_4_3(self) -> str:
        return "[TC-CNET-4.3] [Ethernet] Verification for attributes check [DUT-Server]"

    def pics_TC_CNET_4_3(self) -> list[str]:
        """Return the PICS definitions associated with this test."""
        pics = [
            "CNET.S.F02"
        ]
        return pics

    def steps_TC_CNET_4_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, test_plan_support.commission_if_required(), "", is_commissioning=True),
            TestStep(2, "TH reads Descriptor Cluster from the DUT with EP0 TH reads ServerList from the DUT"),
            TestStep(3, "TH reads the MaxNetworks attribute from the DUT"),
            TestStep(4, "TH reads the Networks attribute list from the DUT")
        ]
        return steps

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning,
                                         Clusters.NetworkCommissioning.Bitmaps.Feature.kEthernetNetworkInterface))
    async def test_TC_CNET_4_3(self):
        # Comissioning already done
        self.step(1)

        self.step(2)
        server_list = await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList)
        asserts.assert_true(49 in server_list, msg="Verify for the presence of an element with value 49 (0x0031) in the ServerList")

        self.step(3)
        max_networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.MaxNetworks)
        matter_asserts.assert_int_in_range(max_networks, min_value=1, max_value=255, description="MaxNetworks")

        self.step(4)
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks)
        # [NetworkCommissioning.Structs.NetworkInfoStruct(networkID=b'ens4', connected=True, networkIdentifier=None, clientIdentifier=None)]
        print(networks)


if __name__ == "__main__":
    default_matter_test_main()
