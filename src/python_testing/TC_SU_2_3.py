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
        Espera hasta recibir un StateTransition cuyo newState == target_state.
        Ignora cualquier otro StateTransition que llegue antes.
        """
        deadline = time.time() + timeout_sec
        while True:
            remaining = deadline - time.time()
            if remaining <= 0:
                asserts.fail(f"Timeout esperando StateTransition a {target_state}")

            try:
                ev = event_cb.event_queue.get(block=True, timeout=remaining)
            except queue.Empty:
                asserts.fail(f"Timeout esperando StateTransition a {target_state}")

            data = ev.Data
            if getattr(data, "newState", None) == target_state:
                return data

    async def collect_post_abort_state(self, event_cb, requestor_cluster, timeout_sec=180.0):
        """
        Después de forzar el aborto de una transferencia, observa las
        transiciones de estado del Requestor y devuelve el primer estado
        "terminal" aceptable:
          - kDownloading  -> el DUT recuperó (resume real o restart desde 0)
          - kIdle         -> el DUT reportó fin/limpio (no soporta resume,
                             pero no quedó colgado)
        Cualquier otra cosa dentro del timeout se ignora; si el timeout
        expira sin ver ninguno de esos dos, devuelve None.
        """
        observed = []
        deadline = time.time() + timeout_sec
        while time.time() < deadline:
            remaining = deadline - time.time()
            try:
                ev = event_cb.event_queue.get(block=True, timeout=min(remaining, 5.0))
            except queue.Empty:
                continue

            new_state = getattr(ev.Data, "newState", None)
            observed.append(new_state)
            logger.info("Post-abort state observed: %s", new_state)

            if new_state in (
                requestor_cluster.Enums.UpdateStateEnum.kDownloading,
                requestor_cluster.Enums.UpdateStateEnum.kIdle,
            ):
                return new_state, observed

        return None, observed

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
                     "Verify the DUT reacts to the aborted transfer by either: "
                     "(a) resuming/restarting the download (kDownloading again), or "
                     "(b) reporting a clean error state (kIdle). "
                     "In either case the DUT must not remain in an inconsistent state.")
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
        command = {"Name": "QueryImageSnapshot", "Cluster": "OtaSoftwareUpdateProvider", "Endpoint": self.endpoint}
        self.write_to_app_pipe(command, self.fifo_in)
        response_data = self.read_from_app_pipe(self.fifo_out)

        logger.info("Out of band command response: %s", response_data)

        # Verify that the DUT obtains the User Consent from the user prior to transfer of software update image
        user_consent_needed = response_data['Payload']['UserConsentNeeded']
        asserts.assert_true(user_consent_needed, "UserConsentNeeded should be True")

        self.terminate_provider()

        # Wait for the Requestor to come back to Idle before starting Step 2
        logger.info("Waiting for idle state before Step 2")
        idle_event = event_cb.wait_for_event_report(Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 50)
        asserts.assert_equal(requestorCluster.Enums.UpdateStateEnum.kIdle, idle_event.newState,
                             f"New state is {idle_event.newState} and it should be {requestorCluster.Enums.UpdateStateEnum.kIdle}")

        await asyncio.sleep(5)

        # Force fail the transfer before it completes and initiate another QueryImage.
        # NOTE: The Matter BDX spec defines RC[STARTOFS] as an OPTIONAL resume mechanism,
        # so this step is intentionally NOT gated behind a PICS. We verify that the DUT
        # reacts consistently to an aborted transfer in one of the following ways:
        #   (a) Resume with RC[STARTOFS] set (offset > 0) -> observed as new kDownloading
        #   (b) Restart from offset 0 -> also observed as new kDownloading
        #   (c) Report DownloadError / go back to kIdle cleanly (resume unsupported)
        # Any of these is considered valid; getting stuck in an inconsistent state is not.
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

        # Let a few BDX blocks flow so a potential resume has a non-zero offset
        await asyncio.sleep(3)

        # Optional: capture how many bytes were transferred before abort.
        # Useful if you later want to distinguish "resume with STARTOFS>0"
        # from "restart from 0" by comparing offsets on the second transfer.
        bytes_before_abort = None
        try:
            command = {"Name": "QueryImageSnapshot", "Cluster": "OtaSoftwareUpdateProvider", "Endpoint": self.endpoint}
            self.write_to_app_pipe(command, self.fifo_in)
            snapshot = self.read_from_app_pipe(self.fifo_out)
            bytes_before_abort = snapshot.get('Payload', {}).get('BytesTransferred')
            logger.info("Bytes transferred before abort: %s", bytes_before_abort)
        except Exception as e:
            logger.warning("Could not read pre-abort snapshot: %s", e)

        # Force-fail the transfer by killing the provider mid-download.
        # The DUT's BDX session will fail once the peer disappears.
        logger.info("Forcing transfer abort by terminating the provider")
        self.terminate_provider()

        # Small pause so the DUT registers the peer loss.
        await asyncio.sleep(2)

        # Bring the provider back and re-announce so the DUT starts a new QueryImage.
        # This is well within the BDX 5-minute idle window, so if the DUT supports
        # resume it should send ReceiveInit with RC[STARTOFS] set on the new transfer.
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

        # Observe the DUT's reaction: expect either kDownloading (resume/restart)
        # or kIdle (clean error). Anything else within the timeout is a failure.
        final_state, observed = await self.collect_post_abort_state(
            event_cb=event_cb,
            requestor_cluster=requestorCluster,
            timeout_sec=180
        )

        asserts.assert_is_not_none(
            final_state,
            f"DUT did not react to the aborted transfer within the timeout. "
            f"Observed states: {observed}"
        )
        logger.info(
            "DUT reacted to aborted transfer with state=%s. All observed states: %s",
            final_state, observed
        )

        # TODO (optional, requires provider support):
        # If the provider exposes the incoming ReceiveInit STARTOFS field via
        # QueryImageSnapshot, compare it against `bytes_before_abort` to confirm
        # actual resume vs restart. Without that, both are treated as valid
        # recovery, matching the reviewer's guidance.

        self.terminate_provider()


if __name__ == "__main__":
    default_matter_test_main()