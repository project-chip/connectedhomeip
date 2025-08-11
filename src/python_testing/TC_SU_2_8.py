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
import subprocess

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

    def _make_provider_location(self, controller, provider_node_id: int, endpoint: int):
        ProviderLoc = Clusters.Objects.OtaSoftwareUpdateRequestor.Structs.ProviderLocation
        fabric_index = getattr(controller, "fabricIndex", None)
        if fabric_index is None:
            return ProviderLoc(providerNodeID=provider_node_id, endpoint=endpoint, fabricIndex=0)
        return ProviderLoc(providerNodeID=provider_node_id, endpoint=endpoint, fabricIndex=fabric_index)

    async def _write_default_providers_both_fabrics(self, controller_th1, controller_th2, endpoint, p1_node_id, p2_node_id):
        prov_th1 = self._make_provider_location(controller_th1, p1_node_id, endpoint)
        prov_th2 = self._make_provider_location(controller_th2, p2_node_id, endpoint)

        resp = await controller_th1.WriteAttribute(
            self.dut_node_id,
            [(endpoint, Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders([prov_th1]))]
        )
        asserts.assert_equal(resp[0].Status, Status.Success, "Write DefaultOTAProviders (TH1) failed.")
        logging.info(f"DefaultOTAProviders (TH1) = [{prov_th1}].")

        resp = await controller_th2.WriteAttribute(
            self.dut_node_id,
            [(endpoint, Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders([prov_th2]))]
        )
        asserts.assert_equal(resp[0].Status, Status.Success, "Write DefaultOTAProviders (TH2) failed.")
        logging.info(f"DefaultOTAProviders (TH2) = [{prov_th1}].")

    async def _announce(self, controller, vendor_id: int, p1_node_id: int, endpoint: int):
        cmd = Clusters.Objects.OtaSoftwareUpdateRequestor.Commands.AnnounceOTAProvider(
            providerNodeID=p1_node_id,
            vendorID=vendor_id,
            announcementReason=Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kUpdateAvailable,
            metadataForNode=None,
            endpoint=endpoint
        )
        resp = await self.send_single_cmd(cmd=cmd, dev_ctrl=controller)
        logging.info(f"Announce resp: {resp}.")

    def kill_provider1(self):
        patterns = [
            r"chip-ota-provider-app.*--discriminator\s*1111",
            r"ota-provider.*--discriminator\s*1111",
            r"chip-ota-provider-app.*--secured-device-port\s*5543"
        ]

        for pat in patterns:
            try:
                subprocess.run(["pkill", "-9", "-f", pat], check=False)
            except Exception:
                pass
        subprocess.run(["pkill", "-9", "chip-ota-provider-app"], check=False)
        logging.info("Provider-1 (TH1) killed.")

    async def configure_acl_permissions(self, controller, endpoint: int, node_id):
        """
        Configure ACL entries required for OTA communication.
        """

        logging.info("Configuring ACL permissions.")

        acl_entries = [
            Clusters.Objects.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[],
                targets=[]
            ),
            Clusters.Objects.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[],
                targets=[]
            ),
            Clusters.Objects.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[],
                targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                    endpoint=endpoint,
                    cluster=Clusters.OtaSoftwareUpdateRequestor.id
                )]
            )
        ]

        acl_attr = Clusters.Objects.AccessControl.Attributes.Acl(value=acl_entries)
        resp = await controller.WriteAttribute(node_id, [(endpoint, acl_attr)])
        asserts.assert_equal(resp[0].Status, Status.Success, "ACL write failed.")
        logging.info("ACL permissions configured successfully.")

    async def wait_for_valid_update_state(self, endpoint: int, valid_states):
        """
        Poll UpdateState until it enters a valid one.
        """

        max_wait = 60
        interval = 3
        elapsed = 0

        while elapsed < max_wait:
            state = await self.read_single_attribute_check_success(
                node_id=self.dut_node_id,
                endpoint=endpoint,
                attribute=Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.UpdateState,
                cluster=Clusters.Objects.OtaSoftwareUpdateRequestor
            )

            logging.info(f"[{elapsed}s] UpdateState = {state.name}.")

            if state == valid_states:
                return state

            time.sleep(interval)
            elapsed += interval

        raise AssertionError(f"DUT did not reach expected OTA state. Final state: {state.name}.")

    @async_test_body
    async def test_TC_SU_2_8(self):

        # Commissioning TH1 (CLI) and TH2 (python test)
        self.step(0)

        endpoint = 0  # self.get_endpoint(default=0)
        dut_node_id = self.dut_node_id
        th1 = self.default_controller  # TH1
        fabric_id_th2 = th1.fabricId + 1
        vendor_id = 0xFFF1  # from CLI

        logging.info("Setting up TH2.")
        th2_certificate_auth = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_auth.NewFabricAdmin(vendorId=vendor_id, fabricId=fabric_id_th2)
        th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        logging.info("Openning commissioning window on DUT.")
        params = await self.open_commissioning_window(th1, dut_node_id)

        resp = await th2.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=params.commissioningParameters.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=params.randomDiscriminator
        )

        logging.info(f"TH2 commissioned: {resp}.")

        # Commissioning Provider-TH1

        p1_node = 10
        p1_disc = 1111
        p_pass = 20202021

        p2_node = 11
        p2_disc = 2222

        logging.info("Commissioning OTA Provider 1 to TH1")

        resp = await th1.CommissionOnNetwork(
            nodeId=p1_node,
            setupPinCode=p_pass,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=p1_disc
        )

        logging.info(f"Commissioning response: {resp}.")

        # Commissioning Provider-TH2

        logging.info("Commissioning OTA Provider 2 to TH2")

        resp = await th2.CommissionOnNetwork(
            nodeId=p2_node,
            setupPinCode=p_pass,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=p2_disc
        )

        logging.info(f"Commissioning response: {resp}.")

        await self.configure_acl_permissions(th1, endpoint, p1_node)
        # await self.configure_acl_permissions(th2, endpoint, p2_node)

        if fabric_id_th2 == th1.fabricId:
            raise AssertionError(f"Fabric IDs are the same for TH1: {th1.fabricId} and TH2: {fabric_id_th2}.")

        await self._write_default_providers_both_fabrics(th1, th2, endpoint, p1_node, p2_node)

        # DUT sends a QueryImage command to TH1/OTA-P.
        self.step(1)

        # await self._announce(th1, vendor_id, p1_node, endpoint)

        self.kill_provider1()

        time.sleep(2)

        # TH1/OTA-P does not respond with QueryImageResponse.
        self.step(2)

        await self._announce(th2, vendor_id, p2_node, endpoint)

        valid_states = {
            Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying,
            Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading,
            Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying,
            Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply,
            Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnUserConsent
        }

        valid_states = list(valid_states)

        await self.wait_for_valid_update_state(endpoint, valid_states[0])

        await self.wait_for_valid_update_state(endpoint, valid_states[1])


# CHECK VERIFICATION STEPS: PROVIDER 1 DOES NOT RECEIVE A QUERY IMAGE WHEN FAILING

if __name__ == "__main__":
    default_matter_test_main()
