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
#       --KVS /tmp/chip_kvs_requestor
#       --autoApplyImage
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 0
#       --int-arg ota_image_expected_version:2
#       --int-arg ota_provider_port:5541
#       --string-arg provider_app_path:${OTA_PROVIDER_APP}
#       --string-arg ota_image:${SU_OTA_REQUESTOR_V2}
#       --string-arg app_pipe:/tmp/su_2_8_fifo
#       --string-arg app_pipe_out:/tmp/su_2_8_fifo_out
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
    async def setup_test(self):

        # Variables for TH1-OTA Provider
        self.p1_node = 10
        self.p1_node_invalid = 13
        self.p1_disc = 1112

        # Variables for TH2-OTA Provider
        self.p2_node = 11
        self.p2_disc = 1111

        self.p_pass = 20202021

        # States
        self.idle = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
        self.querying = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
        self.downloading = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading

        self.app_path = self.user_params.get('provider_app_path', None)
        self.image = self.user_params.get('ota_image')
        self.provider_port = self.user_params.get('ota_provider_port', 5541)

        self.target_version = self.user_params.get('ota_image_expected_version')
        if not self.target_version:
            asserts.fail("Missing target version. Speficy using --int-arg target_version:<TARGET_VERSION>")

        self.fifo_in = self.user_params.get('app_pipe', None)
        if not self.fifo_in:
            asserts.fail("Fifo input missing. Speficy using --string-arg app_pipe:<FIFO_APP_PIPE_INPUT>")

        self.fifo_out = self.user_params.get('app_pipe_out', None)
        if not self.fifo_out:
            asserts.fail("Fifo output missing. Speficy using --string-arg app_pipe_out:<FIFO_APP_PIPE_OUTPUT>")

        self.endpoint = self.get_endpoint()
        self.requestor_node_id = self.dut_node_id
        self.th1 = self.default_controller
        self.fabric_id_th2 = self.th1.fabricId + 1
        self.vendor_id = self.matter_test_config.admin_vendor_id

        logging.info(f"Endpoint: {self.endpoint}.")
        logging.info(f"DUT Node ID: {self.dut_node_id}.")
        logging.info(f"Requestor Node ID: {self.requestor_node_id}.")
        logging.info(f"Vendor ID: {self.vendor_id}.")

        logging.info(f"TH1 fabric id: {self.th1.fabricId}.")
        logging.info(f"TH2 fabric id: {self.fabric_id_th2}.")

        # Start OTA Provider
        logging.info("Starting OTA Provider 1")

        extra_arguments = ['--app-pipe', self.fifo_in, '--app-pipe-out', self.fifo_out]
        self.start_provider(
            provider_app_path=self.app_path,
            ota_image_path=self.image,
            setup_pincode=self.p_pass,
            discriminator=self.p1_disc,
            port=self.provider_port,
            kvs_path='/tmp/chip_kvs_provider_1',
            log_file='/tmp/provider_1.log',
            expected_output=None,
            extra_args=extra_arguments
        )

        # Commissioning Provider-TH1
        logging.info("Commissioning OTA Provider to TH1")

        resp = await self.th1.CommissionOnNetwork(
            nodeId=self.p1_node,
            setupPinCode=self.p_pass,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.p1_disc
        )

        logging.info(f"Commissioning response: {resp}.")
        super().setup_test()

    @async_test_body
    async def teardown_test(self):
        self.terminate_provider()
        super().teardown_test()

    @async_test_body
    async def test_TC_SU_2_8(self):

        # Commissioning step
        self.step(0)

        # DUT sends a QueryImage commands to TH1/OTA-P
        self.step(1)

        # Define ACL entry
        await self.create_acl_entry(dev_ctrl=self.th1, provider_node_id=self.p1_node, requestor_node_id=self.requestor_node_id)

        # Write default OTA providers
        await self.set_default_ota_providers_list(self.th1, self.p1_node, self.requestor_node_id, self.endpoint)

        # Announce after subscription
        await self.announce_ota_provider(controller=self.th1, provider_node_id=self.p1_node, requestor_node_id=self.requestor_node_id, vendor_id=self.vendor_id, endpoint=self.endpoint)

        await asyncio.sleep(2)

        command = {"Name": "QueryImageSnapshot", "Cluster": "OtaSoftwareUpdateProvider", "Endpoint": self.endpoint}
        self.write_to_app_pipe(command, self.fifo_in, is_subprocess=True)
        response_data = self.read_from_app_pipe(self.fifo_out, is_subprocess=True)

        logging.info(f"Out of band command response: {response_data}")

        # Check VendorID
        vendor_id_basic_information = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.VendorID
        )

        vid = response_data['Payload']['VendorID']
        asserts.assert_equal(vendor_id_basic_information, vid, f"Vendor ID is {vid} and it should be {vendor_id_basic_information}")

        # Check ProductID
        product_id_basic_information = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.ProductID
        )

        pid = response_data['Payload']['ProductID']
        asserts.assert_equal(product_id_basic_information, pid,
                             f"Product ID is {pid} and it should be {product_id_basic_information}")

        # Check HardwareVersion
        hardware_version_basic_information = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.HardwareVersion
        )

        hwv = response_data['Payload']['HardwareVersion']
        asserts.assert_equal(hardware_version_basic_information, hwv,
                             f"Hardware Version is {hwv} and it should be {hardware_version_basic_information}")

        # Check SoftwareVersion
        software_version_basic_information = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1,
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
                dev_ctrl=self.th1,
                cluster=Clusters.BasicInformation,
                attribute=Clusters.BasicInformation.Attributes.Location
            )

            asserts.assert_equal(location_basic_information, location,
                                 f"Location is {location} and it should be {location_basic_information}")

        # Stop provider
        await asyncio.sleep(2)
        self.terminate_provider()

        self.th1.ExpireSessions(nodeId=self.p1_node)
        await asyncio.sleep(2)

        # Configure DefaultOTAProviders with invalid node ID. DUT tries to send a QueryImage command to TH1/OTA-P. DUT sends QueryImage command to TH2/OTA-P
        self.step(2)

        logging.info("Starting OTA Provider 2")
        self.start_provider(
            provider_app_path=self.app_path,
            ota_image_path=self.image,
            setup_pincode=self.p_pass,
            discriminator=self.p2_disc,
            port=self.provider_port,
            kvs_path='/tmp/chip_kvs_provider_2',
            log_file='/tmp/provider_2.log',
            expected_output=None
        )

        # Create TH2
        logging.info("Setting up TH2.")
        th2_certificate_auth = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_auth.NewFabricAdmin(vendorId=self.vendor_id, fabricId=self.fabric_id_th2)
        th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        logging.info("Opening commissioning window on DUT.")
        params = await self.open_commissioning_window(self.th1, self.dut_node_id)

        # Commission TH2/DUT (requestor)
        resp = await th2.CommissionOnNetwork(
            nodeId=self.requestor_node_id,
            setupPinCode=params.commissioningParameters.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=params.randomDiscriminator
        )

        logging.info(f"TH2 commissioned: {resp}.")

        # Commissioning Provider-TH2
        logging.info("Commissioning OTA Provider to TH2")

        resp = await th2.CommissionOnNetwork(
            nodeId=self.p2_node,
            setupPinCode=self.p_pass,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.p2_disc
        )

        logging.info(f"Commissioning response: {resp}.")

        # ACL permissions are not required

        if self.fabric_id_th2 == self.th1.fabricId:
            raise AssertionError(f"Fabric IDs are the same for TH1: {self.th1.fabricId} and TH2: {self.fabric_id_th2}.")

        # Event Handler
        event_cb = EventSubscriptionHandler(expected_cluster=Clusters.Objects.OtaSoftwareUpdateRequestor)
        await event_cb.start(dev_ctrl=self.th1, node_id=self.requestor_node_id, endpoint=self.endpoint,
                             fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)

        # Write default OTA providers TH1 with p1_node which does not exist
        await self.set_default_ota_providers_list(self.th1, self.p1_node_invalid, self.requestor_node_id, self.endpoint)

        # Announce TH1-OTA Provider
        await self.announce_ota_provider(controller=self.th1, provider_node_id=self.p1_node_invalid, requestor_node_id=self.requestor_node_id, vendor_id=self.vendor_id, endpoint=self.endpoint)

        # Expect events idle to querying, downloadError and then back to idle
        querying_event = event_cb.wait_for_event_report(
            Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 50)
        asserts.assert_equal(self.querying, querying_event.newState,
                             f"New state is {querying_event.newState} and it should be {self.querying}")

        download_error = event_cb.wait_for_event_report(
            Clusters.Objects.OtaSoftwareUpdateRequestor.Events.DownloadError, 50)

        logging.info(f"Download Error: {download_error}")

        idle_event = event_cb.wait_for_event_report(
            Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 50)
        asserts.assert_equal(self.idle, idle_event.newState,
                             f"New state is {idle_event.newState} and it should be {self.idle}")

        event_cb.reset()
        await event_cb.cancel()
        await asyncio.sleep(2)

        # Subscribe to events
        event_cb = EventSubscriptionHandler(expected_cluster=Clusters.Objects.OtaSoftwareUpdateRequestor)
        await event_cb.start(dev_ctrl=th2, node_id=self.requestor_node_id, endpoint=self.endpoint,
                             fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)

        # Define ACL entry
        await self.create_acl_entry(dev_ctrl=th2, provider_node_id=self.p2_node, requestor_node_id=self.requestor_node_id)

        # Write default OTA providers TH2
        await self.set_default_ota_providers_list(th2, self.p2_node, self.requestor_node_id, self.endpoint)

        # Announce after subscription
        await self.announce_ota_provider(controller=th2, provider_node_id=self.p2_node, requestor_node_id=self.requestor_node_id, vendor_id=self.vendor_id, endpoint=self.endpoint)

        event_idle_to_querying = event_cb.wait_for_event_report(
            Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 50)

        self.verify_state_transition_event(event_report=event_idle_to_querying,
                                           expected_previous_state=self.idle, expected_new_state=self.querying)

        event_querying_to_downloading = event_cb.wait_for_event_report(
            Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 50)

        self.verify_state_transition_event(event_report=event_querying_to_downloading,
                                           expected_previous_state=self.querying, expected_new_state=self.downloading, expected_target_version=self.target_version)

        event_cb.reset()
        await event_cb.cancel()

        await self.clear_ota_providers(controller=th2, requestor_node_id=self.requestor_node_id)

        th2.ExpireSessions(nodeId=self.p2_node)


if __name__ == "__main__":
    default_matter_test_main()
