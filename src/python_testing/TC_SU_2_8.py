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


import logging

from mobly import asserts
from TC_SUBase import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_SU_2_8(MatterBaseTest, SoftwareUpdateBaseTest):
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
            TestStep(1, "Configure DefaultOTAProviders with invalid node ID. DUT tries to send a QueryImage command to TH1/OTA-P.",
                     "TH1/OTA-P does not respond with QueryImage response command. StateTransition goes from idle to querying, then a download error happens and finally it goes back to idle."),
            TestStep(2, "DUT sends QueryImage command to TH2/OTA-P.",
                     "Subscribe to events for OtaSoftwareUpdateRequestor cluster and verify StateTransition reaches downloading state. Also check if the targetSoftwareVersion is 2."),
        ]
        return steps

    def _make_provider_location(self, controller, provider_node_id: int, endpoint: int):
        ProviderLoc = Clusters.Objects.OtaSoftwareUpdateRequestor.Structs.ProviderLocation
        fabric_index = controller.fabricId
        return ProviderLoc(providerNodeID=provider_node_id, endpoint=endpoint, fabricIndex=fabric_index)

    async def _write_default_providers(self, controller, endpoint, provider_node_id, dut_node_id):
        """
        Write default OTA providers.
        """

        prov = self._make_provider_location(controller, provider_node_id, endpoint)

        logging.info("Writing DefaultOTAProviders.")
        resp = await controller.WriteAttribute(
            dut_node_id,
            [(endpoint, Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders([prov]))]
        )
        asserts.assert_equal(resp[0].Status, Status.Success, "Write DefaultOTAProviders failed.")
        logging.info(f"DefaultOTAProviders = [{prov}].")

    async def _announce(self, controller, vendor_id: int, node_id: int, endpoint: int):
        """
        Announce OTA provider.
        """

        cmd = Clusters.Objects.OtaSoftwareUpdateRequestor.Commands.AnnounceOTAProvider(
            providerNodeID=node_id,
            vendorID=vendor_id,
            announcementReason=Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kUpdateAvailable,
            metadataForNode=None,
            endpoint=endpoint
        )
        resp = await self.send_single_cmd(cmd=cmd, dev_ctrl=controller)
        logging.info(f"Announce resp: {resp}.")

    async def check_event_status(self, event, previous_state, next_state, software_version):
        logging.info(f"State Transition: {event}")

        asserts.assert_equal(previous_state, event.previousState,
                             f"Previous state is {event.previousState} and it should be {previous_state}")
        asserts.assert_equal(next_state, event.newState,
                             f"New state is {event.newState} and it should be {next_state}")

        asserts.assert_equal(software_version, event.targetSoftwareVersion,
                             f"Target version is {event.targetSoftwareVersion} and it should be {software_version}")

    @async_test_body
    async def test_TC_SU_2_8(self):

        # Variables for TH1-OTA Provider
        p1_node = 10

        # Variables for TH2-OTA Provider
        p2_node = 11
        p2_disc = 1111

        p_pass = 20202021

        # States
        idle = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
        querying = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
        downloading = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading

        target_version = 2

        # Commissioning step
        self.step(0)

        endpoint = self.get_endpoint(default=0)
        dut_node_id = self.dut_node_id
        dut_node_id_th2 = self.dut_node_id
        th1 = self.default_controller
        fabric_id_th2 = th1.fabricId + 1
        vendor_id = 0xFFF1

        logging.info(f"Endpoint: {endpoint}.")
        logging.info(f"DUT Node ID: {dut_node_id}.")
        logging.info(f"Vendor ID: {vendor_id}.")

        logging.info(f"TH1 fabric id: {th1.fabricId}.")
        logging.info(f"TH2 fabric id: {fabric_id_th2}.")

        # Create TH2
        logging.info("Setting up TH2.")
        th2_certificate_auth = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_auth.NewFabricAdmin(vendorId=vendor_id, fabricId=fabric_id_th2)
        th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        logging.info("Opening commissioning window on DUT.")
        params = await self.open_commissioning_window(th1, dut_node_id)

        # Commission TH2/DUT (requestor)
        resp = await th2.CommissionOnNetwork(
            nodeId=dut_node_id_th2,
            setupPinCode=params.commissioningParameters.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=params.randomDiscriminator
        )

        logging.info(f"TH2 commissioned: {resp}.")

        # Do not commissioning Provider-TH1

        # Commissioning Provider-TH2

        logging.info("Commissioning OTA Provider to TH2")

        resp = await th2.CommissionOnNetwork(
            nodeId=p2_node,
            setupPinCode=p_pass,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=p2_disc
        )

        logging.info(f"Commissioning response: {resp}.")

        # ACL permissions are not required

        if fabric_id_th2 == th1.fabricId:
            raise AssertionError(f"Fabric IDs are the same for TH1: {th1.fabricId} and TH2: {fabric_id_th2}.")

        # Configure DefaultOTAProviders with invalid node ID. DUT tries to send a QueryImage command to TH1/OTA-P.
        self.step(1)

        event_cb = EventSubscriptionHandler(expected_cluster=Clusters.Objects.OtaSoftwareUpdateRequestor)
        await event_cb.start(dev_ctrl=th2, node_id=dut_node_id_th2, endpoint=endpoint,
                             fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)

        # Write default OTA providers TH1 with p1_node which does not exist
        await self._write_default_providers(th1, endpoint, p1_node, dut_node_id)

        default_ota_providers = await self.read_single_attribute_check_success(
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders,
            cluster=Clusters.Objects.OtaSoftwareUpdateRequestor,
            dev_ctrl=th1
        )

        logging.info(f"Default OTA Providers: {default_ota_providers}.")

        # Do not announce TH1-OTA Provider

        # Expect events idle to querying, downloadError and then back to idle
        querying_event = event_cb.wait_for_event_report(
            Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 5000)
        asserts.assert_equal(querying, querying_event.newState,
                             f"New state is {querying_event.newState} and it should be {querying}")

        download_error = event_cb.wait_for_event_report(
            Clusters.Objects.OtaSoftwareUpdateRequestor.Events.DownloadError, 50000)

        logging.info(f"Download Error: {download_error}")

        idle_event = event_cb.wait_for_event_report(
            Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 50000)
        asserts.assert_equal(idle, idle_event.newState,
                             f"New state is {idle_event.newState} and it should be {idle}")

        logging.info("Cleaning DefaultOTAProviders.")
        resp = await th1.WriteAttribute(
            dut_node_id,
            [(endpoint, Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders([]))]
        )
        asserts.assert_equal(resp[0].Status, Status.Success, "Clean DefaultOTAProviders failed.")

        # DUT sends QueryImage command to TH2/OTA-P.
        self.step(2)

        # Subscribe to events
        event_cb = EventSubscriptionHandler(expected_cluster=Clusters.Objects.OtaSoftwareUpdateRequestor)
        await event_cb.start(dev_ctrl=th2, node_id=dut_node_id_th2, endpoint=endpoint,
                             fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)

        # Write default OTA providers TH2
        await self._write_default_providers(th2, endpoint, p2_node, dut_node_id_th2)

        # Write default OTA providers TH2
        default_ota_providers = await self.read_single_attribute_check_success(
            node_id=dut_node_id_th2,
            endpoint=endpoint,
            attribute=Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders,
            cluster=Clusters.Objects.OtaSoftwareUpdateRequestor,
            dev_ctrl=th2
        )

        logging.info(f"Default OTA Providers: {default_ota_providers}.")

        # Announce after subscription
        await self._announce(th2, vendor_id, p2_node, endpoint)

        event_idle_to_querying = event_cb.wait_for_event_report(
            Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 5000)

        await self.check_event_status(event=event_idle_to_querying, previous_state=idle, next_state=querying, software_version=NullValue)

        event_querying_to_downloading = event_cb.wait_for_event_report(
            Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 5000)

        await self.check_event_status(event=event_querying_to_downloading, previous_state=querying, next_state=downloading, software_version=target_version)


if __name__ == "__main__":
    default_matter_test_main()
