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
#       --secured-device-port 5540
#       --autoApplyImage
#       --KVS /tmp/chip_kvs_requestor
#       --requestorCanConsent true
#       --userConsentState granted
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --nodeId 2
#       --admin-vendor-id 65521
#       --int-arg product-id:32769
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --string-arg provider_app_path:${OTA_PROVIDER_APP}
#       --string-arg ota_image:${SU_OTA_REQUESTOR_V2}
#       --int-arg ota_provider_port:5541
#     factory-reset: true
#     quiet: false
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import queue
import time

from mobly import asserts
from TC_SUTestBase import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_SU_2_3(SoftwareUpdateBaseTest):

    async def setup_provider(self, provider_app_path: str, ota_image_path: str, extra_args: list[str]):
        """
        Launch an OTA Provider process with the specified image and extra arguments.

        Args:
            provider_app_path (str): Path to the OTA provider application executable.
            ota_image_path (str): Path to the OTA image file.
            extra_args (list[str]): Additional command-line arguments to pass to the provider.

        Returns:
            None
        """

        logger.info(f"""Prerequisite - Provider info:
            provider_app_path: {provider_app_path},
            ota_image_path: {ota_image_path},
            discriminator: {self.provider_data["discriminator"]},
            setupPinCode: {self.provider_data["setup_pincode"]},
            port: {self.provider_data["port"]},
            extra_args: {extra_args},
            kvs_path: {self.KVS_PATH},
            log_file: {self.LOG_FILE_PATH}""")

        self.start_provider(
            provider_app_path=provider_app_path,
            ota_image_path=ota_image_path,
            setup_pincode=self.provider_data["setup_pincode"],
            discriminator=self.provider_data["discriminator"],
            port=self.provider_data["port"],
            extra_args=extra_args,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH,
            expected_output="Server initialization complete",
            timeout=30
        )

    def desc_TC_SU_2_3(self) -> str:
        return "[TC-SU-2.3] Transfer of Software Update Images between DUT and TH/OTA-P"

    def pics_TC_SU_2_3(self):
        """Return the PICS definitions associated with this test."""
        return [
            "MCORE.OTA.Requestor"
        ]

    def steps_TC_SU_2_3(self) -> list[TestStep]:
        return [
            TestStep(0, "Prerequisite: Commission the DUT (Requestor) with the TH/OTA-P (Provider)",
                     is_commissioning=True),
            TestStep(1, "DUT sends a QueryImage command to the TH/OTA-P. RequestorCanConsent is set to True by DUT. "
                     "QueryStatus is set to 'UpdateAvailable'. "
                     "OTA-P/TH responds with a QueryImageResponse with UserConsentNeeded field set to True.",
                     "Verify that the DUT obtains the User Consent from the user prior to transfer of software update image. This step is vendor specific."),
            TestStep(2, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'UpdateAvailable'. Set ImageURI to the location where the image is located.",
                     "Verify that there is a transfer of the software image from the TH/OTA-P to the DUT. "
                     "Verify that the Maximum Block Size requested by DUT should be: "
                     "- no larger than 1024 (2^10) bytes over non-TCP transports. "
                     "- no larger than 8192 (2^13) bytes over TCP transport."),
            TestStep(3, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'UpdateAvailable'. Set ImageURI with the https url of the software image.",
                     "Verify that there is a transfer of the software image from the TH/OTA-P to the DUT "
                     "from the https url and not from the OTA-P."),
            TestStep(4, "During the transfer of the image to the DUT, force fail the transfer before it completely transfers the image. "
                     "Wait for the Idle timeout so that reading the UpdateState Attribute of the OTA Requestor returns the value as Idle. "
                     "Initiate another QueryImage Command from DUT to the TH/OTA-P.",
                     "Verify that the BDX Idle timeout should be no less than 5 minutes. "
                     "Verify that the DUT starts a new transfer of software image when sending another QueryImage request."),
            TestStep(5, "During the transfer of the image to the DUT, force fail the transfer before it completely transfers the image. "
                     "Initiate another QueryImage Command from DUT to the TH/OTA-P. "
                     "Set the RC[STARTOFS] bit and associated STARTOFS field in the ReceiveInit Message to indicate the resumption of a transfer previously aborted.",
                     "ImageURI should have the https url from where the image can be downloaded.",
                     "Verify that the DUT starts receiving the rest of the software image after resuming the image transfer.")
        ]

    @async_test_body
    async def teardown_test(self):
        if hasattr(self, "current_provider_app_proc") and self.current_provider_app_proc is not None:
            logger.info("Terminating existing OTA Provider")
            self.current_provider_app_proc.terminate()
            self.current_provider_app_proc = None
        super().teardown_test()

    @async_test_body
    async def test_TC_SU_2_3(self):

        self.LOG_FILE_PATH = "provider.log"
        self.KVS_PATH = "/tmp/chip_kvs_provider"
        self.provider_app_path = self.user_params.get('provider_app_path', None)
        self.ota_image_v2 = self.user_params.get('ota_image')
        self.endpoint = self.get_endpoint()

        if not self.provider_app_path:
            asserts.fail("Missing provider app path.")

        # States
        self.querying = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
        self.downloading = Clusters.Objects.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading

        self.step(0)

        # Requestor (DUT) info
        controller = self.default_controller
        fabric_id = controller.fabricId
        requestor_node_id = self.dut_node_id

        # Provider info
        provider_node_id = 1
        provider_discriminator = 1111
        provider_setupPinCode = 20202021
        provider_port = self.user_params.get('ota_provider_port', 5541)

        self.provider_data = {
            "discriminator": provider_discriminator,
            "setup_pincode": provider_setupPinCode,
            "port": provider_port,
        }

        self.step(1)

        # userConsentNeeded flag
        provider_extra_args_updateconsent = [
            "--requestorCanConsent", "true"
        ]

        # Start provider
        await self.setup_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image_v2,
            extra_args=provider_extra_args_updateconsent
        )

        # Commission Provider (Only one time)
        resp = await controller.CommissionOnNetwork(
            nodeId=provider_node_id,
            setupPinCode=provider_setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_discriminator
        )
        logger.info(f'Provider Commissioning response: {resp}')

        await self.create_acl_entry(dev_ctrl=controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)

        await self.set_default_ota_providers_list(controller=controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id, endpoint=self.endpoint)

        await self.announce_ota_provider(controller=controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id, endpoint=self.endpoint)

        await asyncio.sleep(2)

        command = {"Name": "QueryImageSnapshot", "Cluster": "OtaSoftwareUpdateProvider", "Endpoint": self.endpoint}
        self.write_to_app_pipe(command, self.fifo_in)
        response_data = self.read_from_app_pipe(self.fifo_out)

        logger.info(f"Out of band command response: {response_data}")

        user_consent_needed = response_data['Payload']['UserConsentNeeded']

        asserts.assert_true(user_consent_needed, "UserConsentNeeded should be True")

        self.terminate_provider()

        self.step(2)

        provider_extra_args_updateAvailable = [
            "-q", "updateAvailable"
        ]

        # Start provider
        await self.setup_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image_v2,
            extra_args=provider_extra_args_updateAvailable
        )

        # Commission Provider (Only one time)
        resp = await controller.CommissionOnNetwork(
            nodeId=provider_node_id,
            setupPinCode=provider_setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_discriminator
        )
        logger.info(f'Provider Commissioning response: {resp}')

        await self.create_acl_entry(dev_ctrl=controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)

        await self.set_default_ota_providers_list(controller=controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id, endpoint=self.endpoint)

        event_cb = EventSubscriptionHandler(expected_cluster=Clusters.Objects.OtaSoftwareUpdateRequestor)
        await event_cb.start(dev_ctrl=controller, node_id=requestor_node_id, endpoint=self.endpoint, fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)

        await self.announce_ota_provider(controller=controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id, endpoint=self.endpoint)

        querying_event = event_cb.wait_for_event_report(Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 50)
        asserts.assert_equal(self.querying, querying_event.newState,
                             f"New state is {querying_event.newState} and it should be {self.querying}")

        downloading_event = event_cb.wait_for_event_report(Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 50)
        asserts.assert_equal(self.downloading, downloading_event.newState,
                             f"New state is {downloading_event.newState} and it should be {self.downloading}")

        command = {"Name": "QueryImageSnapshot", "Cluster": "OtaSoftwareUpdateProvider", "Endpoint": self.endpoint}
        self.write_to_app_pipe(command, self.fifo_in)
        response_data = self.read_from_app_pipe(self.fifo_out)

        logger.info(f"Out of band command response: {response_data}")

        block_size = response_data['Payload']['BlockSize']

        tcp = True
        max_block_size_tcp = 8192
        max_block_size_non_tcp = 1024

        if tcp:
            asserts.assert_less(block_size, max_block_size_tcp,
                                f"{block_size} should be less than {max_block_size_tcp} bytes in TCP transport")
        else:
            asserts.assert_less(block_size, max_block_size_non_tcp,
                                f"{block_size} should be less than {max_block_size_non_tcp} bytes in non TCP transport")

        event_cb.reset()
        event_cb.cancel()

        self.terminate_provider()

        self.step(3)
        # NOTE: Step skipped not implemented in spec.
        self.step(4)
        # NOTE: Step 4 behavior is already covered by TC_SU_2_2 Step #1 (idle transition after cancel)
        # and TC_SU_2_7 Step #4 (forced cancel and 5-min timeout validation).
        # Reference PR: #685 for the provider cancelation implementation.
        self.step(5)
        # NOTE: Step skipped not implemented in spec.


if __name__ == "__main__":
    default_matter_test_main()
