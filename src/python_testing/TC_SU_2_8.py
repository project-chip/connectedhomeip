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

import logging
import time

import chip.clusters as Clusters
from chip.interaction_model import Status
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
            TestStep(2, "TH1/OTA-P does not respond with QueryImageResponse."),
        ]
        return steps

    @async_test_body
    async def test_TC_SU_2_8(self):

        endpoint = self.get_endpoint(default=0)
        dut_node_id = self.dut_node_id
        th2 = self.default_controller
        cluster = Clusters.Objects.BasicInformation
        attr = Clusters.Objects.BasicInformation.Attributes

        provider_th1 = Clusters.OtaSoftwareUpdateRequestor.Structs.ProviderLocation(
            providerNodeID=0xDEAD,
            endpoint=0,
            fabricIndex=1
        )

        provider_th2 = Clusters.Objects.OtaSoftwareUpdateRequestor.Structs.ProviderLocation(
            providerNodeID=self.dut_node_id,
            endpoint=endpoint,
            fabricIndex=th2.fabricId
        )

        # Commissioning
        self.step(0)

        # DUT sends a QueryImage command to TH1/OTA-P.
        self.step(1)

        logging.info(f"TH1 provider: {provider_th1}")

        resp = await self.write_single_attribute(
            attribute_value=Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders(
                value=[provider_th1]
            ),
            endpoint_id=endpoint
        )

        logging.info(f"Response from TH1: {resp}")

        asserts.assert_equal(resp, Status.Success, "Failed to write DefaultOTAProviders for TH1")

        update_state = await self.read_single_attribute_check_success(
            node_id=dut_node_id,
            endpoint=endpoint,
            attribute=Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.UpdateState,
            cluster=Clusters.Objects.OtaSoftwareUpdateRequestor
        )

        logging.info(f"State: {update_state}")

        th1_state = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
        asserts.assert_equal(update_state, th1_state,
                             f"UpdateState is {update_state} and it should be {th1_state} after setting TH1")

        query_image_server = await self.read_single_attribute_check_success(
            node_id=dut_node_id,
            endpoint=endpoint,
            attribute=attr,
            cluster=cluster
        )

        bdx_protocol = Clusters.Objects.OtaSoftwareUpdateProvider.Enums.kBDXSynchronous

        # asserts.assert_equal(query_image_server.vendorID, , f"Vendor ID in server side is {query_image_server.vendorID} and it should be {}.")
        # asserts.assert_equal(query_image_server.productID, , f"Product ID in server side is {query_image_server.productID} and it should be {}.")
        # asserts.assert_equal(query_image_server.softwareVersion, , f"Software version in server side is {query_image_server.softwareVersion} and it should be {}.")
        # asserts.assert_equal(query_image_server.hardwareVersion, , f"Hardware version in server side is {query_image_server.hardwareVersion} and it should be {}.")
        # asserts.assert_true(bdx_protocol in query_image_server.protocolsSupported,
        #                     f"BDX protocol: {bdx_protocol} is not in protocolsSupported list: {query_image_server.protocolsSupported}")

        # MCORE.OTA.HTTPS validation

        # RequestorCanConsent validation

        # asserts.assert_equal(query_image_server.location, , f"Location in server side is {query_image_server.location} and it should be {}")

        # TH1/OTA-P does not respond with QueryImageResponse.
        self.step(2)

        resp = await self.write_single_attribute(
            attribute_value=Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders(
                value=[provider_th2]
            ),
            endpoint_id=endpoint
        )

        asserts.assert_equal(resp, Status.Success, "Failed to write DefaultOTAProviders for TH2")


if __name__ == "__main__":
    default_matter_test_main()
