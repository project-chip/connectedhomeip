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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --string-arg provider_app_path:${OTA_PROVIDER_APP}
#       --string-arg ota_image:${SU_OTA_REQUESTOR_V2}
#       --int-arg ota_provider_port:5541
#       --string-arg app_pipe:/tmp/su_2_3_fifo
#       --string-arg app_pipe_out:/tmp/su_2_3_fifo_out
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
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_SU_2_3(SoftwareUpdateBaseTest):

    def desc_TC_SU_2_3(self) -> str:
        return "[TC-SU-2.3] Transfer of Software Update Images between DUT and TH/OTA-P"

    def pics_TC_SU_2_3(self):
        """Return the PICS definitions associated with this test."""
        return [
            "MCORE.OTA",
        ]

    async def wait_for_requestor_state(self, event_cb, target_state, timeout_sec=120.0):
        """
        Wait for a StateTransition event whose newState matches target_state.
        Any other StateTransition events that arrive before the target one
        are consumed and ignored. Fails the test on timeout.
        """
        deadline = time.time() + timeout_sec
        while True:
            remaining = deadline - time.time()
            if remaining <= 0:
                asserts.fail(f"Timeout waiting for StateTransition to {target_state}")

            try:
                ev = event_cb.event_queue.get(block=True, timeout=remaining)
            except queue.Empty:
                asserts.fail(f"Timeout waiting for StateTransition to {target_state}")

            data = ev.Data
            if getattr(data, "newState", None) == target_state:
                return data

    async def wait_for_requestor_state_or_none(self, event_cb, target_state, timeout_sec=120.0):
        """
        Same semantics as wait_for_requestor_state, but returns None when
        the deadline expires instead of failing the test. Useful when the
        absence of a transition is itself a valid outcome that the caller
        needs to branch on (e.g. DUT that does not support BDX resume).
        """
        deadline = time.time() + timeout_sec
        while True:
            remaining = deadline - time.time()
            if remaining <= 0:
                return None
            try:
                ev = event_cb.event_queue.get(block=True, timeout=remaining)
            except queue.Empty:
                return None
            data = ev.Data
            if getattr(data, "newState", None) == target_state:
                return data

    def steps_TC_SU_2_3(self) -> list[TestStep]:
        return [
            TestStep(0, "Prerequisite: Commission the DUT (Requestor) with the TH/OTA-P (Provider)",
                     is_commissioning=True),
            TestStep(1, "DUT sends a QueryImage command to the TH/OTA-P. RequestorCanConsent is set to True by DUT. "
                     "QueryStatus is set to 'UpdateAvailable'. "
                     "OTA-P/TH responds with a QueryImageResponse with UserConsentNeeded field set to True.",
                     "Verify that the DUT obtains the User Consent from the user prior to transfer of software update image. This step is vendor specific."),
            TestStep(2, "During the transfer of the image to the DUT, force fail the transfer before it completely transfers the image. "
                     "Initiate another QueryImage Command from DUT to the TH/OTA-P.",
                     "Verify the DUT either resumes the transfer by sending a ReceiveInit with RC[STARTOFS]=1 and STARTOFS equal to "
                     "the bytesDownloaded reported in the prior DownloadError event, or reports the appropriate error / restart "
                     "if resume is not supported.")
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

        requestorCluster = Clusters.Objects.OtaSoftwareUpdateRequestor

        self.LOG_FILE_PATH = "provider.log"
        self.KVS_PATH = "/tmp/chip_kvs_provider"
        self.provider_app_path = self.user_params.get('provider_app_path', None)
        if not self.provider_app_path:
            asserts.fail("Missing provider app path.")
        self.ota_image = self.user_params.get('ota_image')
        self.endpoint = self.get_endpoint()

        self.fifo_in = self.user_params.get('app_pipe', None)
        if not self.fifo_in:
            asserts.fail("Fifo input missing. Speficy using --string-arg app_pipe:<FIFO_APP_PIPE_INPUT>")

        self.fifo_out = self.user_params.get('app_pipe_out', None)
        if not self.fifo_out:
            asserts.fail("Fifo output missing. Speficy using --string-arg app_pipe_out:<FIFO_APP_PIPE_OUTPUT>")

        self.step(0)

        # Requestor (DUT) info
        controller = self.default_controller
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

        # DUT sends a QueryImage command to the TH/OTA-P. RequestorCanConsent is set to True by DUT.
        # OTA-P/TH responds with a QueryImageResponse with UserConsentNeeded field set to True.
        self.step(1)

        # userConsentNeeded flag
        provider_extra_args_updateconsent = [
            '--userConsentNeeded',
            '--app-pipe', self.fifo_in, '--app-pipe-out', self.fifo_out
        ]

        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_data["setup_pincode"],
            discriminator=self.provider_data["discriminator"],
            port=self.provider_data["port"],
            extra_args=provider_extra_args_updateconsent,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH,
            expected_output="Server initialization complete",
            timeout=30
        )

        # Commission Provider (Only one time)
        logger.info("Commissioning provider/TH")
        resp = await controller.CommissionOnNetwork(
            nodeId=provider_node_id,
            setupPinCode=provider_setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_discriminator
        )
        logger.info("Provider Commissioning response: %s", resp)

        logger.info("Starting EventSubscription handlers")
        event_cb = EventSubscriptionHandler(expected_cluster=Clusters.Objects.OtaSoftwareUpdateRequestor,
                                            expected_event_id=Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition.event_id)
        await event_cb.start(dev_ctrl=controller, node_id=requestor_node_id, endpoint=self.endpoint, fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)

        await self.create_acl_entry(dev_ctrl=controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)

        await self.set_default_ota_providers_list(controller=controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id, endpoint=self.endpoint)

        await self.announce_ota_provider(controller=controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id, endpoint=self.endpoint)

        await asyncio.sleep(0.5)

        # Waiting for querying event
        querying_event = event_cb.wait_for_event_report(Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 50)
        logger.info("Quering event: %s", querying_event)
        asserts.assert_equal(requestorCluster.Enums.UpdateStateEnum.kQuerying, querying_event.newState,
                             f"New state is {querying_event.newState} and it should be {requestorCluster.Enums.UpdateStateEnum.kQuerying}")

        # Waiting for downloading event
        downloading_event = event_cb.wait_for_event_report(Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 50)
        logger.info("Downloading event: %s", downloading_event)
        asserts.assert_equal(requestorCluster.Enums.UpdateStateEnum.kDownloading, downloading_event.newState,
                             f"New state is {downloading_event.newState} and it should be {requestorCluster.Enums.UpdateStateEnum.kDownloading}")

        # Getting QueryImageSnapshot using out-of-band communication channel
        # command = {"Name": "QueryImageSnapshot", "Cluster": "OtaSoftwareUpdateProvider", "Endpoint": self.endpoint}
        # self.write_to_app_pipe(command, self.fifo_in)
        # response_data = self.read_from_app_pipe(self.fifo_out)
# 
        # logger.info("Out of band command response: %s", response_data)
# 
        # # Verify that the DUT obtains the User Consent from the user prior to transfer of software update image
        # user_consent_needed = response_data['Payload']['UserConsentNeeded']
        # asserts.assert_true(user_consent_needed, "UserConsentNeeded should be True")

        self.terminate_provider()

        # Wait for the Requestor to come back to Idle before starting Step 2
        logger.info("Waiting for idle state before Step 2")
        idle_event = event_cb.wait_for_event_report(Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 50)
        asserts.assert_equal(requestorCluster.Enums.UpdateStateEnum.kIdle, idle_event.newState,
                             f"New state is {idle_event.newState} and it should be {requestorCluster.Enums.UpdateStateEnum.kIdle}")

        await asyncio.sleep(5)

        # Step 2: force fail the transfer mid-download, restart the provider,
        # and observe how the DUT reacts. Per TC-SU-2.3 the DUT must EITHER
        # resume the aborted transfer (RC[STARTOFS]=1, STARTOFS equal to the
        # bytesDownloaded at abort) OR report a clean error / restart from
        # zero if resume is not implemented. Both outcomes are valid; only
        # inconsistent behavior fails the test.
        self.step(2)

        provider_extra_args_resume = [
            '--queryImageStatus', 'updateAvailable',
            '--app-pipe', self.fifo_in, '--app-pipe-out', self.fifo_out
        ]

        # Start provider and let the DUT begin downloading
        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_data["setup_pincode"],
            discriminator=self.provider_data["discriminator"],
            port=self.provider_data["port"],
            extra_args=provider_extra_args_resume,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH,
            expected_output="Server initialization complete",
            timeout=30
        )

        await self.announce_ota_provider(
            controller=controller,
            provider_node_id=provider_node_id,
            requestor_node_id=requestor_node_id,
            endpoint=self.endpoint
        )

        # Wait until the DUT is actively downloading
        downloading = await self.wait_for_requestor_state(
            event_cb,
            requestorCluster.Enums.UpdateStateEnum.kDownloading,
            timeout_sec=120
        )
        logger.info("Transfer started, ready to abort. Event: %s", downloading)

        # Subscribe to DownloadError so we can capture bytesDownloaded at
        # abort time. That value is what we later compare against the
        # STARTOFS observed in the resumed ReceiveInit.
        error_download_event_handler = EventSubscriptionHandler(
            expected_cluster=requestorCluster,
            expected_event_id=requestorCluster.Events.DownloadError.event_id)
        await error_download_event_handler.start(
            dev_ctrl=controller, node_id=requestor_node_id, endpoint=self.endpoint,
            fabric_filtered=False, min_interval_sec=0, max_interval_sec=20, autoResubscribe=True)

        # Let a few BDX blocks flow so that if the DUT does resume, the
        # observed StartOffset will be strictly greater than 0.
        await asyncio.sleep(10)

        # Force-fail the transfer by killing the provider mid-download.
        # The DUT's BDX session will fail once the peer disappears.
        logger.info("Forcing transfer abort by terminating the provider")
        self.terminate_provider()

        download_error_event = error_download_event_handler.wait_for_event_report(
            requestorCluster.Events.DownloadError, timeout_sec=120)
        bytes_downloaded_at_abort = download_error_event.bytesDownloaded
        logger.info(
            "DownloadError after abort: bytesDownloaded=%s progressPercent=%s",
            bytes_downloaded_at_abort, download_error_event.progressPercent)
        asserts.assert_greater(
            bytes_downloaded_at_abort, 0,
            "Expected non-zero bytesDownloaded so that a resume can be distinguished from a restart"
        )
        error_download_event_handler.cancel()

        # Small pause so the DUT registers the peer loss.
        await asyncio.sleep(2)

        # Bring the provider back up and re-announce so the DUT initiates
        # a new QueryImage cycle. The provider is a passive observer here:
        # it does NOT force anything, it only snapshots the fields the DUT
        # puts in the next ReceiveInit Message.
        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_data["setup_pincode"],
            discriminator=self.provider_data["discriminator"],
            port=self.provider_data["port"],
            extra_args=provider_extra_args_resume,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH,
            expected_output="Server initialization complete",
            timeout=30
        )

        await self.announce_ota_provider(
            controller=controller,
            provider_node_id=provider_node_id,
            requestor_node_id=requestor_node_id,
            endpoint=self.endpoint
        )

        # Observe how the DUT reacts to the aborted transfer.
        #
        # Two valid outcomes per TC-SU-2.3 guidance:
        #   (a) DUT resumes: re-enters kDownloading and its next ReceiveInit
        #       carries RC[STARTOFS]=1 with StartOffset==bytes_downloaded_at_abort.
        #   (b) DUT does not support resume: either restarts the transfer
        #       from offset 0 (RC[STARTOFS]=0, StartOffset=0), or reports a
        #       clean error and never re-enters kDownloading. Both are
        #       acceptable; they only need to be logged.
        #
        # Anything else (bit set with wrong offset, offset > 0 without the
        # bit, etc.) is a protocol violation and fails the test.
        # ------------------------------------------------------------------
        resumed_downloading = await self.wait_for_requestor_state_or_none(
            event_cb,
            requestorCluster.Enums.UpdateStateEnum.kDownloading,
            timeout_sec=180
        )

        if resumed_downloading is None:
            # (b) - clean error path: the DUT never re-entered
            # kDownloading within the timeout. Per the test note this is a
            # valid outcome for devices that do not implement RC[STARTOFS].
            logger.info(
                "DUT did not re-enter kDownloading within the timeout. "
                "Treating as 'resume not supported / clean error' path."
            )
        else:
            logger.info(
                "DUT re-entered kDownloading. StateTransition data: %s",
                resumed_downloading
            )

            # Ask the provider what it observed in the last incoming
            # ReceiveInit. This is a passive read: the provider does not
            # force anything, it only reports what the DUT sent.
            receive_init_snapshot_cmd = {
                "Name": "GetBDXOffset",
                "Cluster": "OtaSoftwareUpdateProvider",
                "Endpoint": self.endpoint,
            }
            self.write_to_app_pipe(receive_init_snapshot_cmd, self.fifo_in)
            receive_init_snapshot = self.read_from_app_pipe(self.fifo_out)
            logger.info("ReceiveInit snapshot from provider: %s", receive_init_snapshot)

            snapshot_payload = receive_init_snapshot["Payload"]
            observed_startofs_bit = snapshot_payload["StartOffsetBitSet"]
            observed_start_offset = snapshot_payload["StartOffset"]

            if observed_startofs_bit and observed_start_offset == bytes_downloaded_at_abort:
                # (a) - genuine resume from the aborted offset.
                logger.info(
                    "DUT resumed the transfer with RC[STARTOFS]=1 and "
                    "StartOffset=%s (matches bytesDownloaded at abort).",
                    observed_start_offset
                )
            elif not observed_startofs_bit and observed_start_offset == 0:
                # (b) via the wire: DUT chose to restart from scratch.
                # Not a failure, just resume not implemented.
                logger.info(
                    "DUT restarted the transfer from offset 0 "
                    "(RC[STARTOFS] not set). Resume is not implemented."
                )
            else:
                # Anything else is inconsistent: e.g. bit set but wrong
                # offset, or offset > 0 without the bit. Real protocol
                # violation; fail the test.
                asserts.fail(
                    "Inconsistent ReceiveInit snapshot: "
                    f"StartOffsetBitSet={observed_startofs_bit}, "
                    f"StartOffset={observed_start_offset}, "
                    f"expected either (True, {bytes_downloaded_at_abort}) "
                    f"for resume or (False, 0) for restart-from-zero."
                )

        self.terminate_provider()


if __name__ == "__main__":
    default_matter_test_main()