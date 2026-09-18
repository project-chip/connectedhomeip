#
#    Copyright (c) 2026 Project CHIP Authors
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
#       --discriminator 123
#       --passcode 2123
#       --KVS /tmp/chip_kvs_requestor
#       --trace-to json:${TRACE_APP}.json
#       --autoApplyImage
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 123
#       --passcode 2123
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --string-arg provider_app_path:${OTA_PROVIDER_APP}
#       --string-arg provider_app_pipe:/tmp/provider_2_4_fifo
#       --string-arg provider_app_pipe_out:/tmp/provider_2_4_fifo_out
#       --string-arg ota_image:${SU_OTA_REQUESTOR_V2}
#       --int-arg ota_image_download_timeout:360
#       --timeout 2100
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     app-ready-pattern: Server initialization complete
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging

from mobly import asserts
from TC_SUTestBase import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_SU_2_4(SoftwareUpdateBaseTest):
    """TC-SU-2.4: Verifies that after the DUT (OTA-R) downloads a software image
    from the OTA-P, it sends an ApplyUpdateRequest carrying the correct fields:
    UpdateToken must match the token the OTA-P issued in the preceding
    QueryImageResponse, and NewVersion must match the software version of the
    image that was downloaded."""
    # Reference variable for the OTA Software Update Provider cluster.
    provider_port = None
    provider_kvs_path = None
    provider_log = None
    ota_prov = Clusters.OtaSoftwareUpdateProvider
    ota_req = Clusters.OtaSoftwareUpdateRequestor
    controller = None
    provider_node_id = 321
    provider_discriminator = 321
    provider_setup_pincode = 2321
    requestor_node_id = None
    ota_image_download_timeout = 0
    disable_wildcard_subscription = True

    @property
    def default_timeout(self) -> int:
        # Used by the framework when --timeout is not passed on the command line.
        # OTA needs a long wall-clock budget: the image download can run 4-6 minutes,
        # plus apply and reboot on top. CI still passes --timeout 2100 explicitly in
        # the test-runner-runs block, which takes precedence over this default.
        return 2100

    @async_test_body
    async def setup_test(self):
        super().setup_test()

        self.ota_prov = Clusters.OtaSoftwareUpdateProvider
        self.ota_req = Clusters.OtaSoftwareUpdateRequestor
        self.requestor_node_id = self.dut_node_id
        self.controller = self.default_controller

        self.ota_image = self.user_params.get('ota_image')
        self.provider_app_path = self.user_params.get('provider_app_path')
        self.provider_port = self.user_params.get('ota_provider_port', 5541)
        self.provider_kvs_path = self.user_params.get('provider_kvs_path', '/tmp/chip_kvs_provider')
        self.provider_log = self.user_params.get('provider_log_path', '/tmp/provider_2_4.log')
        self.provider_app_pipe = self.user_params.get('provider_app_pipe', "")
        self.provider_app_pipe_out = self.user_params.get('provider_app_pipe_out', "")
        # On average the ota image build for the CI is 1.8 MB which takes 4-6 min to download. Adjust time if needed.
        self.ota_image_download_timeout = self.user_params.get('ota_image_download_timeout', 60*6)
        logger.info("Image download timeout is set to %s seconds", self.ota_image_download_timeout)

        # Safety guard: teardown clears files whose path starts with provider_kvs_path
        # via clear_kvs, so restrict the prefix to /tmp to make sure a stray value on
        # --string-arg provider_kvs_path can't take out arbitrary files elsewhere on the host.
        if not self.provider_kvs_path.startswith('/tmp'):
            asserts.fail("Provider KVS path must be placed in the /tmp directory.")

        if self.ota_image_download_timeout <= 0:
            asserts.fail("Invalid value for --int-arg ota_image_download_timeout:<seconds> value provided, must be equal or greater than 1.")

        if not self.provider_app_path:
            asserts.fail("Missing provider app path . Specify using --string-arg provider_app_path:<provider_app_path>")

        if not self.ota_image:
            asserts.fail("Missing ota image path . Specify using --string-arg ota_image:<ota_image>")

        # TC-SU-2.4 needs the provider named pipes in order to read back both the last
        # QueryImageResponse (to get the UpdateToken issued to the DUT) and the last
        # ApplyUpdateRequest (to verify UpdateToken and NewVersion). Require them always,
        # not just on CI.
        if not self.provider_app_pipe or not self.provider_app_pipe_out:
            asserts.fail("Missing argument provider_app_pipe or provider_app_pipe_out. Specify using --string-arg provider_app_pipe:<path> and --string-arg provider_app_pipe_out:<path>")

        # Check OTA image and running software version; this will fail if it is not suited to update the device, else return the version to update
        self.expected_software_version = await self.check_ota_image_version(
            controller=self.controller, requestor_node_id=self.requestor_node_id, ota_image_path=self.ota_image)

        # Start the provider normally (no --queryImageStatus busy, no --userConsentNeeded, no
        # --applyUpdateAction override) so the full flow Query -> Download -> Apply proceeds.
        # The --app-pipe / --app-pipe-out extra args are required for the pipe reads later.
        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_setup_pincode,
            discriminator=self.provider_discriminator,
            port=self.provider_port,
            extra_args=['--app-pipe', self.provider_app_pipe, '--app-pipe-out', self.provider_app_pipe_out],
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            timeout=20
        )

        await self.controller.CommissionOnNetwork(
            nodeId=self.provider_node_id,
            setupPinCode=self.provider_setup_pincode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.provider_discriminator
        )
        await self.set_default_ota_providers_list(controller=self.controller, provider_node_id=self.provider_node_id, endpoint=0, requestor_node_id=self.requestor_node_id)
        await self.create_acl_entry(dev_ctrl=self.controller, provider_node_id=self.provider_node_id, requestor_node_id=self.requestor_node_id)

    @async_test_body
    async def teardown_test(self):
        await self.clear_ota_providers(self.controller, self.requestor_node_id)
        self.terminate_provider()
        self.clear_kvs(kvs_path_prefix=self.provider_kvs_path)
        super().teardown_test()

    def desc_TC_SU_2_4(self) -> str:
        return "[TC-SU-2.4] ApplyUpdateRequest Command from DUT to OTA-P"

    def pics_TC_SU_2_4(self):
        """Return the PICS definitions associated with this test."""
        return ["MCORE.OTA.Requestor"]

    def steps_TC_SU_2_4(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to \"UpdateAvailable\". Set ImageURI to the location where the image is located. After the DUT transfers the image, the DUT should send ApplyUpdateRequest to the OTA-P.",
                     "Verify that the request received on the OTA-P has the following mandatory fields."
                     "UpdateToken - verify that it is same as the one sent in the QueryImageResponse."
                     "NewVersion - verify that this is the same as the software version that was downloaded."),
        ]

    @async_test_body
    async def test_TC_SU_2_4(self):
        # Requestor is the DUT
        # Requestor has the flag --autoApplyImage so it will automatically send ApplyUpdateRequest
        # to the OTA-P once the image transfer completes.
        self.step(0)
        controller = self.default_controller

        self.step(1)
        # Subscribe to StateTransition events so we can confirm each stage of the flow.
        state_transition_event_handler = EventSubscriptionHandler(
            expected_cluster=self.ota_req, expected_event_id=self.ota_req.Events.StateTransition.event_id)
        await state_transition_event_handler.start(controller, self.requestor_node_id, endpoint=0,
                                                   min_interval_sec=0, max_interval_sec=20, autoResubscribe=True)

        await self.announce_ota_provider(controller, self.provider_node_id, self.requestor_node_id)

        # Querying
        logger.info("Waiting for StateTransition -> Querying")
        event_report = state_transition_event_handler.wait_for_event_report(
            self.ota_req.Events.StateTransition, timeout_sec=600)
        logger.info("Event report Querying: %s", event_report)
        self.verify_state_transition_event(
            event_report,
            expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kIdle,
            expected_new_state=self.ota_req.Enums.UpdateStateEnum.kQuerying)

        # Downloading
        logger.info("Waiting for StateTransition -> Downloading")
        event_report = state_transition_event_handler.wait_for_event_report(
            self.ota_req.Events.StateTransition, timeout_sec=60)
        logger.info("Event report Downloading: %s", event_report)
        self.verify_state_transition_event(
            event_report,
            expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kQuerying,
            expected_new_state=self.ota_req.Enums.UpdateStateEnum.kDownloading,
            expected_target_version=self.expected_software_version)

        # Applying: reaching this state means the download completed and the DUT has already
        # sent ApplyUpdateRequest to the OTA-P (and received an ApplyUpdateResponse).
        logger.info("Waiting for StateTransition -> Applying")
        event_report = state_transition_event_handler.wait_for_event_report(
            self.ota_req.Events.StateTransition, timeout_sec=self.ota_image_download_timeout)
        logger.info("Event report Applying: %s", event_report)
        self.verify_state_transition_event(
            event_report,
            expected_previous_state=self.ota_req.Enums.UpdateStateEnum.kDownloading,
            expected_new_state=self.ota_req.Enums.UpdateStateEnum.kApplying,
            expected_target_version=self.expected_software_version)

        state_transition_event_handler.cancel()

        # Small pause to avoid a race against the named pipes after the state transition.
        await asyncio.sleep(2)

        # Read the last QueryImageResponse the OTA-P sent, in order to capture the UpdateToken
        # that was handed to the DUT. Same pipe pattern used in TC-SU-2.7 step 4.
        query_command = {"Name": "QueryImageSnapshot", "Cluster": "OtaSoftwareUpdateProvider",
                         "Endpoint": self.get_endpoint()}
        self.write_to_app_pipe(query_command, self.provider_app_pipe)
        query_response_data = self.read_from_app_pipe(self.provider_app_pipe_out)
        logger.info("Provider QueryImageResponse snapshot: %s", query_response_data)
        expected_update_token = query_response_data['Payload']["UpdateToken"]

        # Read the last ApplyUpdateRequest that the OTA-P received from the DUT.
        # The provider exposes this snapshot via the "GetApplyUpdateRequestStatus" pipe command,
        # which returns UpdateToken and NewVersion under Payload alongside the existing fields.
        apply_command = {"Name": "GetApplyUpdateRequestStatus", "Cluster": "OtaSoftwareUpdateProvider",
                         "Endpoint": self.get_endpoint()}
        self.write_to_app_pipe(apply_command, self.provider_app_pipe)
        apply_response_data = self.read_from_app_pipe(self.provider_app_pipe_out)
        logger.info("Provider ApplyUpdateRequest snapshot: %s", apply_response_data)
        received_update_token = apply_response_data['Payload']["UpdateToken"]
        received_new_version = apply_response_data['Payload']["NewVersion"]

        # Sanity check: the DUT must actually have sent an ApplyUpdateRequest by now, otherwise
        # the token/version fields are still empty defaults and the assertions below would be
        # comparing meaningless data. We check the UpdateToken (which persists) rather than
        # ApplyUpdateRequestSentStatus / ApplyUpdateRequestCount, because on --autoApplyImage
        # runs the DUT can complete the full apply + reboot + NotifyUpdateApplied cycle before
        # we get to read the pipe, and NotifyUpdateApplied resets those two counters on the
        # provider side while leaving UpdateToken and NewVersion intact.
        asserts.assert_true(
            len(received_update_token) > 0,
            "Provider did not record an ApplyUpdateRequest from the DUT before the snapshot was read (UpdateToken is empty).")

        # UpdateToken in ApplyUpdateRequest must match the one the provider issued in QueryImageResponse.
        asserts.assert_equal(
            received_update_token, expected_update_token,
            f"UpdateToken from ApplyUpdateRequest ({received_update_token}) does not match the UpdateToken from QueryImageResponse ({expected_update_token}).")

        # NewVersion in ApplyUpdateRequest must match the software version that was downloaded
        # (i.e. the target version advertised by the OTA image).
        asserts.assert_equal(
            received_new_version, self.expected_software_version,
            f"NewVersion from ApplyUpdateRequest ({received_new_version}) does not match the expected software version ({self.expected_software_version}).")


if __name__ == "__main__":
    default_matter_test_main()
