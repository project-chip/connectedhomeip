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
from chip import ChipDeviceCtrl
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

    async def configure_acl_permissions(self, controller, endpoint: int):
        """
        Configure ACL entries required for OTA communication.
        """

        fabric_id = controller.fabricId

        acl_entries = [
            Clusters.Objects.AccessControl.Structs.AccessControlEntryStruct(
                fabricIndex=fabric_id,
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[],
                targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                    endpoint=endpoint,
                    cluster=Clusters.OtaSoftwareUpdateRequestor.id
                )],
            ),
            Clusters.Objects.AccessControl.Structs.AccessControlEntryStruct(
                fabricIndex=fabric_id,
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[],
                targets=[]
            ),
            Clusters.Objects.AccessControl.Structs.AccessControlEntryStruct(
                fabricIndex=fabric_id,
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[],
                targets=[]
            )
        ]

        acl_attr = Clusters.Objects.AccessControl.Attributes.Acl(value=acl_entries)
        resp = await controller.WriteAttribute(self.dut_node_id, [(endpoint, acl_attr)])
        asserts.assert_equal(resp[0].Status, Status.Success, "ACL write failed.")
        logging.info("ACL permissions configured successfully.")

    async def write_ota_providers(self, controller, providers, endpoint):
        """
        Write DefaultOtaProviders list to DUT.
        """

        resp = await controller.WriteAttribute(
            self.dut_node_id,
            [(endpoint, Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders(providers))]
        )
        asserts.assert_equal(resp[0].Status, Status.Success, "Write OTA providers failed.")
        logging.info(f"OTA providers written: {providers}.")

    async def announce_provider(self, controller, provider_node_id: int, vendor_id: int, endpoint: int):
        """
        Send AnnounceOTAProvider command from given controller.
        """

        cmd = Clusters.Objects.OtaSoftwareUpdateRequestor.Commands.AnnounceOTAProvider(
            providerNodeID=provider_node_id,
            vendorID=vendor_id,
            announcementReason=Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kUpdateAvailable,
            metadataForNode=None,
            endpoint=endpoint
        )

        resp = await self.send_single_cmd(cmd=cmd, dev_ctrl=controller)
        logging.info(f"AnnounceOTAProvider sent from node {controller.nodeId} to DUT.")

    async def wait_for_valid_update_state(self, endpoint: int, valid_states: set):
        """
        Poll UpdateState until it enters a valid one.
        """

        max_wait = 30
        interval = 5
        elapsed = 0

        while elapsed < max_wait:
            state = await self.read_single_attribute_check_success(
                node_id=self.dut_node_id,
                endpoint=endpoint,
                attribute=Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.UpdateState,
                cluster=Clusters.Objects.OtaSoftwareUpdateRequestor
            )

            logging.info(f"[{elapsed}s] UpdateState = {state.name}.")

            if state in valid_states:
                return state

            time.sleep(interval)
            elapsed += interval

        raise AssertionError(f"DUT did not reach expected OTA state. Final state: {state.name}.")

    @async_test_body
    async def test_TC_SU_2_8(self):

        # Commissioning TH1 (CLI) and TH2 (python test)
        self.step(0)

        endpoint = self.get_endpoint(default=0)
        dut_node_id = self.dut_node_id
        controller = self.default_controller  # TH1
        fabric_id_th2 = controller.fabricId + 1
        vendor_id = 0xFFF1  # from CLI

        logging.info(f"Setting up TH2.")
        th2_certificate_auth = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_auth.NewFabricAdmin(vendorId=vendor_id, fabricId=fabric_id_th2)
        th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        logging.info(f"Openning commissioning window on DUT.")
        params = await self.open_commissioning_window(controller, dut_node_id)

        resp = await th2.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=params.commissioningParameters.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=params.randomDiscriminator
        )

        logging.info(f"TH2 commissioned: {resp}.")

        await self.configure_acl_permissions(controller, endpoint)

        # DUT sends a QueryImage command to TH1/OTA-P.
        self.step(1)

        provider_th1 = Clusters.OtaSoftwareUpdateRequestor.Structs.ProviderLocation(
            providerNodeID=1,
            endpoint=endpoint,
            fabricIndex=controller.fabricId
        )

        provider_th2 = Clusters.Objects.OtaSoftwareUpdateRequestor.Structs.ProviderLocation(
            providerNodeID=self.dut_node_id,
            endpoint=endpoint,
            fabricIndex=fabric_id_th2
        )

        logging.info(f"Provider TH1: {provider_th1}.")
        logging.info(f"Provider TH2: {provider_th2}.")

        if fabric_id_th2 == controller.fabricId:
            raise AssertionError(f"Fabric IDs are the same for TH1: {controller.fabricId} and TH2: {fabric_id_th2}.")

        await self.write_ota_providers(controller=controller, providers=[provider_th1], endpoint=endpoint)
        await self.write_ota_providers(controller=th2, providers=[provider_th2], endpoint=endpoint)

        await self.announce_provider(controller=th2, provider_node_id=dut_node_id, vendor_id=vendor_id, endpoint=endpoint)

        # TH1/OTA-P does not respond with QueryImageResponse.
        self.step(2)

        valid_states = {
            Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying,
            Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading,
            Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying,
            Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply,
            Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnUserConsent
        }

        await self.wait_for_valid_update_state(endpoint, valid_states)


if __name__ == "__main__":
    default_matter_test_main()
