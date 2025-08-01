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

    async def write_ota_providers(self, controller, providers, endpoint):
        resp = await controller.WriteAttribute(
            self.dut_node_id,
            [(endpoint, Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders(providers))]
        )
        asserts.assert_equal(resp[0].Status, Status.Success, "Write OTA providers failed.")

    @async_test_body
    async def test_TC_SU_2_8(self):

        endpoint = self.get_endpoint(default=0)
        dut_node_id = self.dut_node_id
        controller = self.default_controller
        fabric_id_th2 = 2

        valid_states = {
            Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying,
            Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading,
            Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying,
            Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply,
            Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnUserConsent
        }

        provider_th1 = Clusters.OtaSoftwareUpdateRequestor.Structs.ProviderLocation(
            providerNodeID=0xDEAD,
            endpoint=0,
            fabricIndex=controller.fabricId
        )

        provider_th2 = Clusters.Objects.OtaSoftwareUpdateRequestor.Structs.ProviderLocation(
            providerNodeID=self.dut_node_id,
            endpoint=endpoint,
            fabricIndex=fabric_id_th2
        )

        if fabric_id_th2 == controller.fabricId:
            raise AssertionError(f"Fabric IDs are the same for TH1: {controller.fabricId} and TH2: {fabric_id_th2}.")

        # Commissioning
        self.step(0)

        # DUT sends a QueryImage command to TH1/OTA-P.
        self.step(1)

        logging.info(f"TH1 provider: {provider_th1}")

        await self.write_ota_providers(controller=controller, providers=[provider_th1], endpoint=endpoint)
        await self.write_ota_providers(controller=controller, providers=[provider_th2], endpoint=endpoint)  # Is this ok?

        self.step(2)

        max_wait = 30
        interval = 5
        elapsed = 0

        update_state = None
        while elapsed < max_wait:
            update_state = await self.read_single_attribute_check_success(
                node_id=dut_node_id,
                endpoint=endpoint,
                attribute=Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.UpdateState,
                cluster=Clusters.Objects.OtaSoftwareUpdateRequestor
            )

            logging.info(f"UpdateState after {elapsed}s: {update_state.name}")

            if update_state in valid_states:
                break

            time.sleep(interval)
            elapsed += interval

        asserts.assert_true(update_state in valid_states,
                            f"DUT did not reach {valid_states} state after fallback to TH2. The state is {update_state}.")


if __name__ == "__main__":
    default_matter_test_main()
