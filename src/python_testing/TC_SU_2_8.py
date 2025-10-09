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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import re
from subprocess import run

from mobly import asserts
from TC_SUTestBase import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.testing.apps import OtaImagePath, OTAProviderSubprocess
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

ANSI_RE = re.compile(r"\x1B\[[0-?]*[-/]*[@-~]]")
HEX_OR_DEC = re.compile(r"(0x[0-9a-fA-F]+|\d+)")
VENDOR_RE = re.compile(r"VendorID:\s*(0x[0-9a-fA-F]+|\d+)")
PRODUCT_RE = re.compile(r"ProductID:\s*(0x[0-9a-fA-F]+|\d+)")
SWVER_RE = re.compile(r"SoftwareVersion:\s*(0x[0-9a-fA-F]+|\d+)")
PROTO_RE = re.compile(r"ProtocolsSupported:\s*\[(.*?)\]", re.DOTALL)
HWVER_RE = re.compile(r"HardwareVersion:\s*(0x[0-9a-fA-F]+|\d+)")
LOC_RE = re.compile(r"Location:\s*([A-Za-z0.9]+)")
RCC_RE = re.compile(r"RequestorCanConsent:\s*(0x[0-9a-fA-F]+|\d+)")

FIELDS = {
    "vendor_id": VENDOR_RE,
    "product_id": PRODUCT_RE,
    "software_version": SWVER_RE,
    "hardware_version": HWVER_RE,
    "location": LOC_RE,
    "requestor_can_consent": RCC_RE
}


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
                     "Subscribe to events for OtaSoftwareUpdateRequestor cluster and verify StateTransition reaches downloading state. Also check if the targetSoftwareVersion is 3."),
        ]
        return steps

    def parse_query_block(self, block: dict) -> dict:
        text = ANSI_RE.sub("", '\n'.join(block.get("after", [])))

        out = {}

        for name, rx in FIELDS.items():
            m = rx.search(text)
            if m:
                val = m.group(1) if m.lastindex else m.group(0)
                if name == "location":
                    out[name] = val
                else:
                    out[name] = int(val, 0)

        m = PROTO_RE.search(text)
        out["protocols_supported"] = (
            [int(n, 0) for n in HEX_OR_DEC.findall(m.group(1))] if m else []
        )

        return out

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

        provider_port = 5540

        app_path = "./out/debug/chip-ota-provider-app"
        provider_ota_file = OtaImagePath(path="firmware_v2.ota")

        # States
        idle = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
        querying = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
        downloading = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading
        applying = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying

        target_version = 3

        endpoint = self.get_endpoint(default=0)
        dut_node_id = self.dut_node_id
        requestor_node_id = self.dut_node_id
        th1 = self.default_controller
        fabric_id_th2 = th1.fabricId + 1
        vendor_id = self.matter_test_config.admin_vendor_id
        product_id = 32769  # Default value

        logging.info(f"Endpoint: {endpoint}.")
        logging.info(f"DUT Node ID: {dut_node_id}.")
        logging.info(f"Requestor Node ID: {requestor_node_id}.")
        logging.info(f"Vendor ID: {vendor_id}.")
        logging.info(f"Product ID: {product_id}.")

        logging.info(f"TH1 fabric id: {th1.fabricId}.")
        logging.info(f"TH2 fabric id: {fabric_id_th2}.")

        # Commissioning step
        self.step(0)

        # Start OTA Provider
        logging.info("Starting OTA Provider 1")

        provider_1 = OTAProviderSubprocess(
            app=app_path,
            storage_dir='/tmp',
            port=provider_port,
            discriminator=p1_disc,
            passcode=p_pass,
            ota_source=provider_ota_file,
            log_file='/tmp/provider_1.log',
            err_log_file='/tmp/provider_1.log'
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
        await self.write_ota_providers(th1, p1_node, endpoint)

        # Announce after subscription
        await self.announce_ota_provider(controller=th1, provider_node_id=p1_node, requestor_node_id=requestor_node_id, vendor_id=vendor_id, endpoint=endpoint)

        await asyncio.sleep(5)

        blocks = provider_1.read_from_logs("QueryImage", before=0, after=15)

        parsed = {}
        for b in blocks:
            info = self.parse_query_block(b)
            for k, v in info.items():
                parsed.setdefault(k, v)

        logging.info(f"Parsed fields: {parsed}")

        # Check VendorID
        vendor_id_basic_information = await self.read_single_attribute_check_success(
            dev_ctrl=th1,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.VendorID
        )

        asserts.assert_equal(vendor_id_basic_information,
                             parsed['vendor_id'], f"Vendor ID is {parsed['vendor_id']} and it should be {vendor_id_basic_information}")

        # Check ProductID
        product_id_basic_information = await self.read_single_attribute_check_success(
            dev_ctrl=th1,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.ProductID
        )

        asserts.assert_equal(product_id_basic_information,
                             parsed['product_id'], f"Product ID is {parsed['product_id']} and it should be {product_id_basic_information}")

        # Check HardwareVersion
        hardware_version_basic_information = await self.read_single_attribute_check_success(
            dev_ctrl=th1,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.HardwareVersion
        )

        asserts.assert_equal(hardware_version_basic_information,
                             parsed['hardware_version'], f"Hardware Version is {parsed['hardware_version']} and it should be {hardware_version_basic_information}")

        # Check SoftwareVersion
        software_version_basic_information = await self.read_single_attribute_check_success(
            dev_ctrl=th1,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.SoftwareVersion
        )

        asserts.assert_equal(software_version_basic_information,
                             parsed['software_version'], f"Software Version is {parsed['software_version']} and it should be {software_version_basic_information}")

        # Check ProtocolsSupported protocols_supported
        asserts.assert_true(Clusters.OtaSoftwareUpdateProvider.Enums.DownloadProtocolEnum.kBDXSynchronous in parsed['protocols_supported'],
                            f"kBDXSynchronous: {Clusters.OtaSoftwareUpdateProvider.Enums.DownloadProtocolEnum.kBDXSynchronous} is not part of ProtocolsSupporter: {parsed['protocols_supported']}")

        # Check MCORE.OTA.HTTPS
        if self.check_pics("MCORE.OTA.HTTPS"):
            asserts.assert_true(Clusters.OtaSoftwareUpdateProvider.Enums.DownloadProtocolEnum.kHttps in parsed['protocols_supported'],
                                f"kHttps: {Clusters.OtaSoftwareUpdateProvider.Enums.DownloadProtocolEnum.kHttps} is not part of ProtocolsSupporter: {parsed['protocols_supported']}")

        # Check RequestorCanConsent
        asserts.assert_equal(parsed['requestor_can_consent'], 0,
                             f"Requestor Can Consent is {parsed['requestor_can_consent']} instead of 0")

        # Check Location
        if parsed['location']:
            location_basic_information = await self.read_single_attribute_check_success(
                dev_ctrl=th1,
                cluster=Clusters.BasicInformation,
                attribute=Clusters.BasicInformation.Attributes.Location
            )

            asserts.assert_equal(location_basic_information,
                                 parsed['location'], f"Location is {parsed['location']} and it should be {location_basic_information}")

        # Event Handler
        event_cb = EventSubscriptionHandler(expected_cluster=Clusters.Objects.OtaSoftwareUpdateRequestor)
        await event_cb.start(dev_ctrl=th1, node_id=requestor_node_id, endpoint=endpoint,
                             fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)

        applying_event = event_cb.wait_for_event_report(
            Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 5000)
        asserts.assert_equal(applying, applying_event.newState,
                             f"New state is {applying_event.newState} and it should be {applying}")

        event_cb.reset()
        await event_cb.cancel()

        # Stop provider
        await asyncio.sleep(2)
        try:
            provider_1.terminate()
        except Exception as e:
            logging.warning(f"Provider termination raised: {e}")
        run("rm -rf /tmp/chip_kvs", shell=True)
        await asyncio.sleep(2)

        th1.ExpireSessions(nodeid=p1_node)

        # Configure DefaultOTAProviders with invalid node ID. DUT tries to send a QueryImage command to TH1/OTA-P. DUT sends QueryImage command to TH2/OTA-P
        self.step(2)

        provider_ota_file = OtaImagePath(path="firmware_v3.ota")

        logging.info("Starting OTA Provider 2")
        provider_2 = OTAProviderSubprocess(
            app=app_path,
            storage_dir='/tmp',
            port=provider_port,
            discriminator=p2_disc,
            passcode=p_pass,
            ota_source=provider_ota_file,
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
        await self.write_ota_providers(th1, p1_node_invalid, endpoint)

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
        await self.write_ota_providers(th2, p2_node, endpoint)

        # Announce after subscription
        await self.announce_ota_provider(controller=th2, provider_node_id=p2_node, requestor_node_id=requestor_node_id, vendor_id=vendor_id, endpoint=endpoint)

        event_idle_to_querying = event_cb.wait_for_event_report(
            Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 5000)

        self.verify_state_transition_event(event_report=event_idle_to_querying, previous_state=idle, new_state=querying)

        event_querying_to_downloading = event_cb.wait_for_event_report(
            Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 5000)

        self.verify_state_transition_event(event_report=event_querying_to_downloading,
                                           previous_state=querying, new_state=downloading, target_version=target_version)

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

        await asyncio.sleep(2)
        th2.ExpireSessions(nodeid=p2_node)


if __name__ == "__main__":
    default_matter_test_main()
