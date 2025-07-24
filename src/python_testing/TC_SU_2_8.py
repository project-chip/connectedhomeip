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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     factory-reset: true
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
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_SU_2_8(MatterBaseTest):
    """
    This test case verifies that the DUT is able to successfully send a QueryImage command to the OTA-P in multi fabric scenario.
    """

    def desc_TC_SU_2_8(self) -> str:
        return "[TC-SU-2.8] OTA functionality in Multi Fabric scenario"

    def pics_TC_SU_2_8(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "MCORE.OTA",
        ]
        return pics

    def steps_TC_SU_2_8(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commissioning, already done.", is_commissioning=True),
            TestStep(1, "DUT sends a QueryImage command to TH1/OTA-P."),
            TestStep(2, "DUT sends a QueryImage command to TH1/OTA-P. TH1/OTA-P does not respond with QueryImageResponse."),
        ]
        return steps

    async def _get_provider_struct(self, th, endpoint=0):
        fabric_index = await self.get_fabric_index_for_node(th.node_id)
        provider_struct = Clusters.Objects.OtaSoftwareUpdateRequestor.Structs.ProviderLocation(
            providerNodeID=th.node_id,
            endpoint=endpoint,
            fabricIndex=fabric_index
        )

        return provider_struct

    @async_test_body
    async def test_TC_SU_2_8(self):

        endpoint = self.get_endpoint(default=0)
        dut_node_id = self.dut_node_id
        attr = Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.DefaultOtaProviders

        # Commissioning
        self.step(0)

        # DUT sends a QueryImage command to TH1/OTA-P.
        self.step(1)

        provider_th1 = await self._get_provider_struct(self.th1, endpoint=endpoint)
        resp = await self.write_single_attribute(
            node_id=dut_node_id,
            endpoint=endpoint,
            attribute=attr,
            value=[provider_th1]
        )

        asserts.assert_equal(resp.status, Clusters.Status.Success, "Failed to write DefaultOTAProviders for TH1")

        state = await self.read_single_attribute_check_success(
            node_id=dut_node_id,
            endpoint=endpoint,
            attribute=Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        asserts.assert_equal(state, 0, "UpdateState should be Idle after setting TH1")

        # DUT sends a QueryImage command to TH1/OTA-P. TH1/OTA-P does not respond with QueryImageResponse.
        self.step(2)

        provider_th2 = await self._get_provider_struct(self.th2, endpoint=endpoint)
        resp = await self.write_single_attribute(
            node_id=dut_node_id,
            endpoint=endpoint,
            attribute=attr,
            value=[provider_th2]
        )

        asserts.assert_equal(resp.status, Clusters.Status.Success, "Failed to write DefaultOTAProviders for TH2")


if __name__ == "__main__":
    default_matter_test_main()
