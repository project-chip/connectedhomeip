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
#     app: ${OTA_REQUESTOR_APP}
#     app-args: >
#       --discriminator 1234
#       --passcode 20202021
#       --secured-device-port 5541
#       --KVS /tmp/chip_kvs_requestor
#       --autoApplyImage
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 0
#       --int-arg target_version:2
#       --int-arg ota_provider_port:5540
#       --string-arg provider_app_path:${OTA_PROVIDER_APP}
#       --string-arg ota_image:${SU_OTA_REQUESTOR_V2}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging

from mobly import asserts
from TC_SUTestBase import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.testing.apps import OtaImagePath, OTAProviderSubprocess
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_SU_2_8(SoftwareUpdateBaseTest, MatterBaseTest):
    """
    This test case verifies that the DUT is able to successfully send a QueryImage command to the OTA-P in multi fabric scenario.
    """

    def desc_TC_SU_2_8(self) -> str:
        return "[TC-SU-2.8] OTA functionality in Multi Fabric scenario"

    def pics_TC_SU_2_8(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "MCORE.OTA.Requestor",
        ]
        return pics

    def steps_TC_SU_2_8(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commissioning, already done.", is_commissioning=True),
            TestStep(1, "DUT sends a QueryImage commands to TH1/OTA-P.",
                     "Verify the QueryImage command received on the server has the following mandatory fields."
                     "VendorId - Should match the value reported by the Basic Information Cluster VendorID attribute of the DUT."
                     "ProductId - Should match the value reported by the Basic Information Cluster ProductID attribute of the DUT."
                     "HardwareVersion - If present, verify that it matches the value reported by the Basic Information Cluster HardwareVersion attribute of the DUT."
                     "SoftwareVersion - Should match the value reported by the Basic Information Cluster SoftwareVersion attribute of the DUT."
                     "Verify the field ProtocolsSupported lists the BDX Synchronous protocol."
                     "If (MCORE.OTA.HTTPS_Supported) HTTPS protocol should be listed."
                     "Verify the default value of RequestorCanConsent is set to False unless DUT sets it to True."
                     "If the Location field is present, verify that the value is same as Basic Information Cluster Location Attribute of the DUT."),
            TestStep(2, "Configure DefaultOTAProviders with invalid node ID. DUT tries to send a QueryImage command to TH1/OTA-P. DUT sends QueryImage command to TH2/OTA-P.",
                     "TH1/OTA-P does not respond with QueryImage response command. StateTransition goes from idle to querying, then a download error happens and finally it goes back to idle."
                     "Subscribe to events for OtaSoftwareUpdateRequestor cluster and verify StateTransition reaches downloading state. Also check if the targetSoftwareVersion is 2."),
        ]
        return steps

    @async_test_body
    async def test_TC_SU_2_8(self):

        # Variables for TH1-OTA Provider
        p1_node = 10
        p1_node_invalid = 13
        p1_disc = 1112

        # Variables for TH2-OTA Provider
        p2_node = 11
        p2_disc = 1111

        p_pass = 20202021

        self.provider_port = self.user_params.get('ota_provider_port')
        if not self.provider_port:
            asserts.fail("Missing OTA provider port. Speficy using --int-arg ota_provider_port:<OTA_PROVIDER_PORT>")

        self.app_path = self.user_params.get('provider_app_path')
        if not self.app_path:
            asserts.fail("Missing OTA provider app path. Speficy using --string-arg provider_app_path:<PROVIDER_APP_PATH>")

        image = self.user_params.get('ota_image')
        if not image:
            asserts.fail("Missing OTA image. Specify using --string-arg ota_image:<OTA_IMAGE>")

        self.provider_ota_file = OtaImagePath(path=image)

        # States
        idle = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
        querying = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
        downloading = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading

        self.target_version = self.user_params.get('target_version')
        if not self.target_version:
            asserts.fail("Missing target version. Speficy using --int-arg target_version:<TARGET_VERSION>")

        self.fifo_in = self.user_params.get('app_pipe', None)
        if not self.fifo_in:
            asserts.fail("Fifo input missing. Speficy using --string-arg app_pipe:<FIFO_APP_PIPE_INPUT>")

        self.fifo_out = self.user_params.get('app_pipe_out', None)
        if not self.fifo_out:
            asserts.fail("Fifo output missing. Speficy using --string-arg app_pipe_out:<FIFO_APP_PIPE_OUTPUT>")

        endpoint = self.get_endpoint(default=0)
        dut_node_id = self.dut_node_id
        requestor_node_id = self.dut_node_id
        th1 = self.default_controller
        fabric_id_th2 = th1.fabricId + 1
        vendor_id = self.matter_test_config.admin_vendor_id

        logging.info(f"Endpoint: {endpoint}.")
        logging.info(f"DUT Node ID: {dut_node_id}.")
        logging.info(f"Requestor Node ID: {requestor_node_id}.")
        logging.info(f"Vendor ID: {vendor_id}.")

        logging.info(f"TH1 fabric id: {th1.fabricId}.")
        logging.info(f"TH2 fabric id: {fabric_id_th2}.")

        # Commissioning step
        self.step(0)

        # Start OTA Provider
        logging.info("Starting OTA Provider 1")

        provider_1 = OTAProviderSubprocess(
            app=self.app_path,
            storage_dir='/tmp',
            port=self.provider_port,
            discriminator=p1_disc,
            passcode=p_pass,
            ota_source=self.provider_ota_file,
            kvs_path='/tmp/chip_kvs_provider',
            log_file='/tmp/provider_1.log',
            err_log_file='/tmp/provider_1.log',
            extra_args=["--app-pipe", self.fifo_in, "--app-pipe-out", self.fifo_out]
        )

        provider_1.start()

        # Commissioning Provider-TH1
        logging.info("Commissioning OTA Provider to TH1")

        resp = await th1.CommissionOnNetwork(
            nodeId=p1_node,
            setupPinCode=p_pass,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=p1_disc
        )

        logging.info(f"Commissioning response: {resp}.")

        # DUT sends a QueryImage commands to TH1/OTA-P
        self.step(1)

        # Define ACL entry
        await self.create_acl_entry(dev_ctrl=th1, provider_node_id=p1_node, requestor_node_id=requestor_node_id)

        # Write default OTA providers
        await self.set_default_ota_providers_list(th1, p1_node, requestor_node_id, endpoint)

        # Announce after subscription
        await self.announce_ota_provider(controller=th1, provider_node_id=p1_node, requestor_node_id=requestor_node_id, vendor_id=vendor_id, endpoint=endpoint)

        await asyncio.sleep(2)

        command = {"Name": "QueryImageSnapshot", "Cluster": "OtaSoftwareUpdateProvider", "Endpoint": 0}
        self.write_to_app_pipe(command, self.fifo_in)
        response_data = self.read_from_app_pipe(self.fifo_out)

        logging.info(f"Out of band command response: {response_data}")

        # Check VendorID
        vendor_id_basic_information = await self.read_single_attribute_check_success(
            dev_ctrl=th1,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.VendorID
        )

        vid = response_data['Payload']['VendorID']
        asserts.assert_equal(vendor_id_basic_information, vid, f"Vendor ID is {vid} and it should be {vendor_id_basic_information}")

        # Check ProductID
        product_id_basic_information = await self.read_single_attribute_check_success(
            dev_ctrl=th1,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.ProductID
        )

        pid = response_data['Payload']['ProductID']
        asserts.assert_equal(product_id_basic_information, pid,
                             f"Product ID is {pid} and it should be {product_id_basic_information}")

        # Check HardwareVersion
        hardware_version_basic_information = await self.read_single_attribute_check_success(
            dev_ctrl=th1,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.HardwareVersion
        )

        hwv = response_data['Payload']['HardwareVersion']
        asserts.assert_equal(hardware_version_basic_information, hwv,
                             f"Hardware Version is {hwv} and it should be {hardware_version_basic_information}")

        # Check SoftwareVersion
        software_version_basic_information = await self.read_single_attribute_check_success(
            dev_ctrl=th1,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.SoftwareVersion
        )

        swv = response_data['Payload']['SoftwareVersion']
        asserts.assert_equal(software_version_basic_information, swv,
                             f"Software Version is {swv} and it should be {software_version_basic_information}")

        # Check ProtocolsSupported protocols_supported
        ps = response_data['Payload']['ProtocolsSupported']
        asserts.assert_true(Clusters.OtaSoftwareUpdateProvider.Enums.DownloadProtocolEnum.kBDXSynchronous in ps,
                            f"kBDXSynchronous: {Clusters.OtaSoftwareUpdateProvider.Enums.DownloadProtocolEnum.kBDXSynchronous} is not part of ProtocolsSupporter: {ps}")

        # Check MCORE.OTA.HTTPS
        if self.check_pics("MCORE.OTA.HTTPS"):
            asserts.assert_true(Clusters.OtaSoftwareUpdateProvider.Enums.DownloadProtocolEnum.kHttps in ps,
                                f"kHttps: {Clusters.OtaSoftwareUpdateProvider.Enums.DownloadProtocolEnum.kHttps} is not part of ProtocolsSupporter: {ps}")

        # Check RequestorCanConsent
        expected_rcc = 0
        rcc = response_data['Payload']['RequestorCanConsent']
        asserts.assert_equal(rcc, expected_rcc, f"Requestor Can Consent is {rcc} instead of {expected_rcc}")

        # Check Location
        location = response_data['Payload']['Location']
        if location:
            location_basic_information = await self.read_single_attribute_check_success(
                dev_ctrl=th1,
                cluster=Clusters.BasicInformation,
                attribute=Clusters.BasicInformation.Attributes.Location
            )

            asserts.assert_equal(location_basic_information, location,
                                 f"Location is {location} and it should be {location_basic_information}")

        # Event Handler
        event_cb = EventSubscriptionHandler(expected_cluster=Clusters.Objects.OtaSoftwareUpdateRequestor)
        await event_cb.start(dev_ctrl=th1, node_id=requestor_node_id, endpoint=endpoint,
                             fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)

        # Stop provider
        await asyncio.sleep(2)
        try:
            provider_1.terminate()
        except Exception as e:
            logging.warning(f"Provider termination raised: {e}")

        th1.ExpireSessions(nodeId=p1_node)
        await asyncio.sleep(2)

        event_cb.reset()
        await event_cb.cancel()

        # Configure DefaultOTAProviders with invalid node ID. DUT tries to send a QueryImage command to TH1/OTA-P. DUT sends QueryImage command to TH2/OTA-P
        self.step(2)

        # provider_ota_file = OtaImagePath(path="firmware_v2.ota")

        logging.info("Starting OTA Provider 2")
        provider_2 = OTAProviderSubprocess(
            app=self.app_path,
            storage_dir='/tmp',
            port=self.provider_port,
            discriminator=p2_disc,
            passcode=p_pass,
            ota_source=self.provider_ota_file,
            kvs_path='/tmp/chip_kvs_provider_2',
            log_file='/tmp/provider_2.log',
            err_log_file='/tmp/provider_2.log'
        )

        provider_2.start()

        # Create TH2
        logging.info("Setting up TH2.")
        th2_certificate_auth = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_auth.NewFabricAdmin(vendorId=vendor_id, fabricId=fabric_id_th2)
        th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        logging.info("Opening commissioning window on DUT.")
        params = await self.open_commissioning_window(th1, dut_node_id)

        # Commission TH2/DUT (requestor)
        resp = await th2.CommissionOnNetwork(
            nodeId=requestor_node_id,
            setupPinCode=params.commissioningParameters.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=params.randomDiscriminator
        )

        logging.info(f"TH2 commissioned: {resp}.")

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

        # Event Handler
        event_cb = EventSubscriptionHandler(expected_cluster=Clusters.Objects.OtaSoftwareUpdateRequestor)
        await event_cb.start(dev_ctrl=th1, node_id=requestor_node_id, endpoint=endpoint,
                             fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)

        # Write default OTA providers TH1 with p1_node which does not exist
        await self.set_default_ota_providers_list(th1, p1_node_invalid, requestor_node_id, endpoint)

        # Announce TH1-OTA Provider
        await self.announce_ota_provider(controller=th1, provider_node_id=p1_node_invalid, requestor_node_id=requestor_node_id, vendor_id=vendor_id, endpoint=endpoint)

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

        event_cb.reset()
        await event_cb.cancel()
        await asyncio.sleep(2)

        # Subscribe to events
        event_cb = EventSubscriptionHandler(expected_cluster=Clusters.Objects.OtaSoftwareUpdateRequestor)
        await event_cb.start(dev_ctrl=th2, node_id=requestor_node_id, endpoint=endpoint,
                             fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)

        # Define ACL entry
        await self.create_acl_entry(dev_ctrl=th2, provider_node_id=p2_node, requestor_node_id=requestor_node_id)

        # Write default OTA providers TH2
        await self.set_default_ota_providers_list(th2, p2_node, requestor_node_id, endpoint)

        # Announce after subscription
        await self.announce_ota_provider(controller=th2, provider_node_id=p2_node, requestor_node_id=requestor_node_id, vendor_id=vendor_id, endpoint=endpoint)

        event_idle_to_querying = event_cb.wait_for_event_report(
            Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 5000)

        self.verify_state_transition_event(event_report=event_idle_to_querying,
                                           expected_previous_state=idle, expected_new_state=querying)

        event_querying_to_downloading = event_cb.wait_for_event_report(
            Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 5000)

        self.verify_state_transition_event(event_report=event_querying_to_downloading,
                                           expected_previous_state=querying, expected_new_state=downloading, expected_target_version=self.target_version)

        event_cb.reset()
        await event_cb.cancel()

        logging.info("Cleaning DefaultOTAProviders.")
        resp = await th2.WriteAttribute(
            dut_node_id,
            [(endpoint, Clusters.Objects.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders([]))]
        )
        asserts.assert_equal(resp[0].Status, Status.Success, "Clean DefaultOTAProviders failed.")

        await asyncio.sleep(2)
        try:
            provider_2.terminate()
        except Exception as e:
            logging.warning(f"Provider termination raised: {e}")

        th2.ExpireSessions(nodeId=p2_node)


if __name__ == "__main__":
    default_matter_test_main()
