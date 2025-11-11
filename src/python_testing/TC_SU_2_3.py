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
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import TestStep, async_test_body, default_matter_test_main

# Create a logger
logger = logging.getLogger(__name__)


class TC_SU_2_3(SoftwareUpdateBaseTest):

    async def add_single_ota_provider(self, controller, requestor_node_id: int, provider_node_id: int):
        """
        Adds a single OTA provider to the Requestor's DefaultOTAProviders attribute
        only if no provider is currently registered. If a provider already exists,
        the function does nothing.

        Args:
            controller: The device controller.
            requestor_node_id (int): Node ID of the Requestor device.
            provider_node_id (int): Node ID of the OTA Provider to add.

        Returns:
            None
        """
        # Read existing DefaultOTAProviders on the Requestor
        current_providers = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=Clusters.OtaSoftwareUpdateRequestor,
            attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders
        )
        logger.info(f'Prerequisite #4.0 - Current DefaultOTAProviders on Requestor: {current_providers}')

        # If there is already a provider, skip adding
        if current_providers:
            logger.info(f'Skipping add: Requestor already has a provider registered ({current_providers})')
            return

        # Create a ProviderLocation for the new provider
        provider_location = Clusters.OtaSoftwareUpdateRequestor.Structs.ProviderLocation(
            providerNodeID=provider_node_id,
            endpoint=0,
            fabricIndex=controller.fabricId
        )
        logger.info(f'Prerequisite #4.0 - ProviderLocation to add: {provider_location}')

        # Combine with existing providers (preserving previous ones)
        updated_providers = current_providers + [provider_location]

        # Write the updated DefaultOTAProviders list back to the Requestor
        attr = Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders(value=updated_providers)
        resp = await controller.WriteAttribute(
            attributes=[(0, attr)],
            nodeId=requestor_node_id
        )
        logger.info(f'Prerequisite #4.0 - Write DefaultOTAProviders response: {resp}')
        asserts.assert_equal(resp[0].Status, Status.Success, "Failed to write DefaultOTAProviders attribute")

    async def clear_ota_providers(self, controller, requestor_node_id: int):
        """
        Clears the DefaultOTAProviders attribute on the Requestor, leaving it empty.

        Args:
            controller: The controller to use for writing attributes.
            requestor_node_id (int): Node ID of the Requestor device.

        Returns:
            None
        """
        # Set DefaultOTAProviders to empty list
        attr_clear = Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders(value=[])
        resp = await controller.WriteAttribute(
            attributes=[(0, attr_clear)],
            nodeId=requestor_node_id
        )
        logger.info('Cleanup - DefaultOTAProviders cleared')

        assert resp[0].Status == Status.Success, "Failed to clear DefaultOTAProviders"

    async def send_announce_ota_provider(self, controller, requestor_node_id, provider_node_id, vendor_id=0xFFF1, reason=None, endpoint=0):
        """
        Send the AnnounceOTAProvider command from the Requestor to the Provider.

        Args:
            controller: Controller object to send the command.
            requestor_node_id: Node ID of the OTA Requestor (DUT).
            provider_node_id: Node ID of the OTA Provider.
            vendor_id: Vendor ID associated with the Provider (default: 0xFFF1).
            reason: AnnouncementReasonEnum for the announcement (default: kUpdateAvailable).
            endpoint: Endpoint on which to send the command (default: 0).

        Returns:
            resp: Response object from the AnnounceOTAProvider command.
        """
        reason = reason or Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kUrgentUpdateAvailable
        cmd = Clusters.OtaSoftwareUpdateRequestor.Commands.AnnounceOTAProvider(
            providerNodeID=provider_node_id,
            vendorID=vendor_id,
            announcementReason=reason,
            metadataForNode=None,
            endpoint=endpoint
        )
        logger.info(f'Sending AnnounceOTAProvider to providerNodeID={provider_node_id}: {cmd}')
        resp = await self.send_single_cmd(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            cmd=cmd
        )
        logger.info(f'AnnounceOTAProvider response: {resp}')
        return resp

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

    async def extend_ota_acls(self, controller, provider_node_id, requestor_node_id):
        """
        Set ACLs for OTA interaction between a Requestor and Provider.
        Preserves existing ACLs to avoid overwriting.

        Args:
            controller: The Matter device controller instance used to read/write ACLs.
            requestor_node: Node ID of the OTA Requestor (DUT).
            provider_node: Node ID of the OTA Provider.

        Returns:
            None

        Raises:
            AssertionError: If reading or writing ACL attributes fails.
        """

        # Read existing ACLs on Requestor and Provider
        provider_existing_acls = await self.read_single_attribute(
            dev_ctrl=controller,
            node_id=provider_node_id,
            endpoint=0,
            attribute=Clusters.AccessControl.Attributes.Acl,
        )

        requestor_existing_acls = await self.read_single_attribute(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            attribute=Clusters.AccessControl.Attributes.Acl,
        )

        # Generate new ACL entries for OTA interaction (via TC_SUTestBase)
        await self.create_acl_entry(
            dev_ctrl=controller,
            provider_node_id=provider_node_id,
            requestor_node_id=requestor_node_id
        )

        await self.create_acl_entry(
            dev_ctrl=controller,
            provider_node_id=requestor_node_id,
            requestor_node_id=provider_node_id
        )

        # Read the current ACLs after adding new entries
        provider_current_acls = await self.read_single_attribute(
            dev_ctrl=controller,
            node_id=provider_node_id,
            endpoint=0,
            attribute=Clusters.AccessControl.Attributes.Acl,
        )

        requestor_current_acls = await self.read_single_attribute(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            attribute=Clusters.AccessControl.Attributes.Acl,
        )

        # Combine original + new ACLs to preserve existing entries
        combined_provider_acls = provider_existing_acls + provider_current_acls
        combined_requestor_acls = requestor_existing_acls + requestor_current_acls

        # Write back the combined ACLs
        await self.write_acl(controller, provider_node_id, combined_provider_acls)
        await self.write_acl(controller, requestor_node_id, combined_requestor_acls)

        logger.info(f'OTA ACLs extended between provider: {provider_node_id} and requestor: {requestor_node_id}')

    async def write_acl(self, controller, node_id, acl):
        """
        Writes the Access Control List (ACL) to the DUT device using the specified controller.

        Args:
            controller: The Matter controller (e.g., th1, th4) that will perform the write operation.
            acl (list): List of AccessControlEntryStruct objects defining the ACL permissions to write.
            node_id:

        Raises:
            AssertionError: If writing the ACL attribute fails (status is not Status.Success).
        """
        result = await controller.WriteAttribute(
            nodeId=node_id,
            attributes=[(0, Clusters.AccessControl.Attributes.Acl(acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")
        return True

    def desc_TC_SU_2_3(self) -> str:
        return "[TC-SU-2.3] Transfer of Software Update Images between DUT and TH/OTA-P"

    def pics_TC_SU_2_3(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "MCORE.OTA.Requestor",      # Pics
        ]
        return pics

    def steps_TC_SU_2_3(self) -> list[TestStep]:
        steps = [
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
        return steps

    @async_test_body
    async def teardown_test(self):
        self.current_provider_app_proc.terminate()
        super().teardown_test()

    @async_test_body
    async def test_TC_SU_2_3(self):

        # NOTE: The DUT (Requestor) is launched with the following flags in CI arguments:
        # Launch the requestor with flags:
        #       --requestorCanConsent true
        #       --userConsentState granted
        # These flags indicate to the Provider that the Requestor is capable of handling user consent.
        # The test simulates that consent has already been granted so the OTA flow can proceed automatically.

        self.LOG_FILE_PATH = "provider.log"
        self.KVS_PATH = "/tmp/chip_kvs_provider"
        self.provider_app_path = self.user_params.get('provider_app_path', None)
        self.ota_image_v2 = self.user_params.get('ota_image')

        self.step(0)
        # Controller has already commissioned the requestor

        # Prerequisite #1.0 - Requestor (DUT) info
        controller = self.default_controller
        fabric_id = controller.fabricId
        requestor_node_id = self.dut_node_id

        # Prerequisite #1.0 - Provider info
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
        step_number = "[STEP_1]"
        logger.info(f'{step_number}: Prerequisite #1.0 - Requestor (DUT), NodeID: {requestor_node_id}, FabricId: {fabric_id}')
        logger.info(f'{step_number}: Prerequisite #1.0 - Launched Provider')

        provider_extra_args_updateAvailable = [
            "-c"
        ]

        await self.setup_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image_v2,
            extra_args=provider_extra_args_updateAvailable
        )

        # Prerequisite #2.0 - Commission Provider (Only one time)
        resp = await controller.CommissionOnNetwork(
            nodeId=provider_node_id,
            setupPinCode=provider_setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_discriminator
        )
        logger.info(f'{step_number}: Prerequisite #2 - Provider Commissioning response: {resp}')

        await self.extend_ota_acls(
            controller=controller,
            provider_node_id=provider_node_id,
            requestor_node_id=requestor_node_id
        )

        # Prerequisite #3.0 - Add OTA Provider to the Requestor (Only if none exists, and only one time)
        logger.info(f'{step_number}: Prerequisite #4.0 - Add Provider to Requestor(DUT) DefaultOTAProviders')
        await self.add_single_ota_provider(controller, requestor_node_id, provider_node_id)

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.1 - Matcher for OTA records logs
        # Start EventSubscriptionHandler first to avoid missing any rapid OTA events (race condition)
        # Events: StateTransition (Idle > Querying > Downloading)
        # ------------------------------------------------------------------------------------
        subscription_state_download = EventSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_event_id=Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition.event_id
        )

        # Start subscriptions
        await subscription_state_download.start(
            dev_ctrl=controller,
            node_id=requestor_node_id,  # DUT
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=1,
            max_interval_sec=1
        )
        await asyncio.sleep(1)

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------

        logger.info(f'{step_number}: Step #1.0 - Controller sends AnnounceOTAProvider command')
        resp_announce = await self.send_announce_ota_provider(controller, requestor_node_id, provider_node_id=provider_node_id)
        logger.info(f'{step_number}: Step #1.0 - cmd AnnounceOTAProvider response: {resp_announce}.')

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.2 -  Track OTA StateTransition event: Idle > Querying > Downloading
        # Don't rely on kDelayedOnUserConsent since there is no UI
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number}: Step #1.1 - Create a subscription for StateTransition event '
            '(should be Idle > Querying > Downloading) '
            'before AnnounceOTAProvider to avoid missing OTA events')

        state_sequence_available = []  # Full OTA state flow

        def matcher_idle_state_download(report):
            """
            Step #1.2 matcher function to track OTA StateTransition event
            Tracks state transitions events:
            First event: Idle > Querying
            Second event: Querying > Downloading
            Records each observed transition only once and validate image transfer occurs.
            """
            nonlocal state_sequence_available
            val = report.Data.newState
            prev_state = report.Data.previousState

            logger.info(f"Previous state: {prev_state}, New state: {val}")

            if val is None:
                return False

            transition = (prev_state, val)
            state_sequence_available.append(transition)

            expected = [
                (Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle,
                 Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying),
                (Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying,
                 Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading),
            ]

            # Validate the last two transitions
            if len(state_sequence_available) >= 2:
                if state_sequence_available[-2:] == expected:
                    logger.info(
                        f"{step_number}: Step #1.2 - Validated StateTransition events: Idle > Querying (first event) > Querying > Downloading (second event)")
                    return True

            return False

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.3 - Start tasks to track OTA events:
        # StateTransition two events: Idle > Querying, Querying > Downloading, ensuring image transfer occurs.
        # ------------------------------------------------------------------------------------
        try:
            timeout_total = 60  # 1 min
            start_time = time.time()

            while time.time() - start_time < timeout_total:
                try:
                    report = subscription_state_download.get_event_from_queue(block=True, timeout=10)
                    matcher_idle_state_download(report)
                except queue.Empty:
                    continue
        finally:
            logger.info(f'{step_number}: Step #1.3 - StateTransition two events: '
                        '(1) Idle > Querying, (2) Querying > Downloading, '
                        'successfully observed.')
            await subscription_state_download.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step # 1.4 - Verify image transfer occurs from TH/OTA-P to DUT
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number}: Step #1.4 - Full OTA StateTransition observed: {state_sequence_available}')

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.5 - Close Provider Process (CLEANUP!)
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number}: Step #1.5 - Closed Provider.')

        # Kill Provider process
        self.current_provider_app_proc.terminate()

        self.step(2)
        self.step(3)
        # NOTE: Step skipped not implemented in spec.
        self.step(4)
        self.step(5)
        # NOTE: Step skipped not implemented in spec.


if __name__ == "__main__":
    default_matter_test_main()
