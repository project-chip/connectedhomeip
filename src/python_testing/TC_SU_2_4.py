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
#       --timeout 2100
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
#     tags: [nightly]
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import time

from mobly import asserts
from support_modules.su_support import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, MatterTestCommissionedDevice
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_SU_2_4(MatterTestCommissionedDevice, SoftwareUpdateBaseTest):
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
        self.cleanup = True
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

        # Safety guard: teardown clears files whose path starts with provider_kvs_path
        # via clear_kvs, so restrict the prefix to /tmp to make sure a stray value on
        # --string-arg provider_kvs_path can't take out arbitrary files elsewhere on the host.
        if not self.provider_kvs_path.startswith('/tmp'):
            asserts.fail("Provider KVS path must be placed in the /tmp directory.")

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
        # Adds a delay to allow terminate the provider but read the ApplyUpdateRequest before reading the named pipe
        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_setup_pincode,
            discriminator=self.provider_discriminator,
            port=self.provider_port,
            extra_args=['--app-pipe', self.provider_app_pipe, '--app-pipe-out',
                        self.provider_app_pipe_out, '--delayedApplyActionTimeSec', '120', '--applyUpdateAction' ,'awaitNextAction'],
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

    async def _cleanup_test(self):
        if self.cleanup:
            await self.clear_ota_providers(self.controller, self.requestor_node_id)
            self.terminate_provider()

    @async_test_body
    async def teardown_test(self):
        # Once checks are completed clear the ota provider list to then terminate the provider.
        await self._cleanup_test()
        self.clear_kvs(kvs_path_prefix=self.provider_kvs_path)
        super().teardown_test()

    def desc_TC_SU_2_4(self) -> str:
        return "[TC-SU-2.4] ApplyUpdateRequest Command from DUT to OTA-P"

    def pics_TC_SU_2_4(self):
        """Return the PICS definitions associated with this test."""
        return ["MCORE.OTA.Requestor"]

    def steps_TC_SU_2_4(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done"),
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
        self.start_test_budget_clock()
        self.step(0)
        controller = self.default_controller

        self.step(1)
        step_number_s1 = '[STEP_1]'
        subscription_attr_s1 = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=None  # receive all attributes
        )

        await self._start_subscription_bounded(
            subscription_attr_s1, step_number_s1,
            dev_ctrl=controller,
            node_id=self.requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=False
        )

        await self.announce_ota_provider(controller, self.provider_node_id, self.requestor_node_id)

        # Verify if the Download has started and some values for UpdateStateProgress is seen.
        state_sequence = []
        progress_values = []
        downloading_seen = False
        progress_seen = False

        def matcher_combined(report):
            """
            Combined matcher for Step 1:

            - Validates UpdateState reaches kDownloading
            - UpdateStateProgress has any value 1-100
            """
            nonlocal state_sequence, progress_values, downloading_seen, progress_seen
            val = getattr(report.value, "value", report.value)
            current_time = time.time()
            # UpdateState
            if report.attribute == Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState:
                if val is not None and val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading:
                    if not downloading_seen:
                        downloading_seen = True
                        state_sequence.append(Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading)
                        logger.info('%s: State observed: %s at %s', step_number_s1, val, current_time)

            # UpdateStateProgress
            elif report.attribute == Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress:
                if val is not None and isinstance(val, int) and 1 <= val <= 100:
                    if not progress_seen:
                        progress_seen = True
                        progress_values.append(val)
                        logger.info('%s: Progress observed: %s at %s', step_number_s1, val, current_time)

            return downloading_seen and progress_seen

        matcher_combined_obj = AttributeMatcher.from_callable(
            description=f"{step_number_s1} -  Downloading + progress 1-100",
            matcher=matcher_combined
        )

        subscription_attr_s1.await_all_expected_report_matches(
            [matcher_combined_obj],
            timeout_sec=self.remaining_test_budget_sec(reserve_sec=60))
        logger.info('%s: UpdateState (updateAvailable sequence) matcher has completed.', step_number_s1)

        # Flow check
        expected_flows = [
            [Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading]
        ]
        if state_sequence in expected_flows:
            logger.info('%s: OTA flow is valid: %s', step_number_s1, state_sequence)
        else:
            msg = f"Observed OTA flow: {state_sequence}, Expected one of: {expected_flows}"
            asserts.fail(msg)

        # Read the last QueryImageResponse the OTA-P sent, in order to capture the UpdateToken.
        # Reads the QuerySnapshot when the Download is in progress to then retrieve the values for
        # ApplyUpdateRequest when transfer is complete.
        logger.info("About to read  the named pipe OtaSoftwareUpdateProvider from provider")
        query_command = {"Name": "QueryImageSnapshot", "Cluster": "OtaSoftwareUpdateProvider",
                         "Endpoint": self.get_endpoint()}
        self.write_to_app_pipe(query_command, self.provider_app_pipe)
        query_response_data = self.read_from_app_pipe(self.provider_app_pipe_out)
        logger.info("Provider QueryImageResponse snapshot: %s", query_response_data)
        expected_update_token = query_response_data['Payload']["UpdateToken"]

        last_progress_s1 = [None]

        def progress_liveness_s1(report):
            if report.attribute != Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress:
                return False
            val = getattr(report.value, "value", report.value)
            if val is None or val == last_progress_s1[0]:
                return False
            last_progress_s1[0] = val
            return True

        # Applying: reaching this state means the download completed and the DUT has already
        # sent ApplyUpdateRequest to the OTA-P (and received an ApplyUpdateResponse).
        subscription_attr_s1.await_first_value_asserting_no_forbidden(
            target_value=Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying,
            forbidden_values=set(),
            timeout_sec=self.remaining_test_budget_sec(reserve_sec=60),
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState,
            stall_timeout_sec=1800,
            liveness_matcher=progress_liveness_s1,
        )
        subscription_attr_s1.cancel()

        # Pool for a max of 30 seconds the read the last ApplyUpdateRequest that the OTA-P received from the DUT to avoid a race condition.
        # A delayedApplyActionTimeSec of 60 secs to cover the poll time.
        # The provider exposes this snapshot via the "GetApplyUpdateRequestStatus" pipe command,
        # which returns UpdateToken and NewVersion under Payload alongside the existing fields.
        poll_command_timeout_sec = 30
        poll_command_interval_sec = 1
        poll_command_deadline = time.time() + poll_command_timeout_sec
        attempt = 0
        # Variables to use for named pipe values
        received_update_token = None
        received_new_version = None
        # Commands for named pipes
        apply_command = {"Name": "GetApplyUpdateRequestStatus", "Cluster": "OtaSoftwareUpdateProvider",
                                 "Endpoint": self.get_endpoint()}

        while True:
            wait_poll = True
            attempt += 1
            self.write_to_app_pipe(apply_command, self.provider_app_pipe)
            try:
                apply_response_data = self.read_from_app_pipe(self.provider_app_pipe_out)
                received_update_token = apply_response_data["Payload"]["UpdateToken"]
                received_new_version = int(apply_response_data["Payload"]["NewVersion"])
                logger.info("Provider GetApplyUpdateRequestStatus retrieved from named pipe: %s at Attempt # %d",
                            apply_response_data, attempt)
            except TimeoutError:
                # Not able to read named pipe in the read_from_app_pipe timeout, do not wait more.
                wait_poll = False
                logger.info("%s Not able to read from named pipe, retrying.")

            # If the named pipe retrieved correct data, then terminate to poll the named pipe
            if received_update_token is not None and len(received_update_token) > 0 and received_new_version is not None and received_new_version > 0:
                logger.info("%s - Named pipe ApplyUpdateRequest value found at Attempt # %d", step_number_s1, attempt)
                break

            # If the values read from the named pipe are not the appropiate wait some time until is ready.
            remaining = poll_command_deadline - time.time()
            if remaining <= 0:
                # Time exhausted, warn and exit the loop. Asserts will fail as received_update_token and expected_update_token initialized as None
                logger.warning(
                    'Step #1 ApplyUpdateRequest has not been reached after %.0fs. Retrying', poll_command_interval_sec)
                break
            if wait_poll:
                await asyncio.sleep(min(poll_command_interval_sec, remaining))

        # UpdateToken in ApplyUpdateRequest must match the one the provider issued in QueryImageResponse.
        asserts.assert_equal(
            received_update_token, expected_update_token,
            f"UpdateToken from ApplyUpdateRequest ({received_update_token}) does not match the UpdateToken from QueryImageResponse ({expected_update_token}).")

        # NewVersion in ApplyUpdateRequest must match the software version that was downloaded
        # (i.e. the target version advertised by the OTA image).
        asserts.assert_equal(
            received_new_version, self.expected_software_version,
            f"NewVersion from ApplyUpdateRequest ({received_new_version}) does not match the expected software version ({self.expected_software_version}).")
        # Clean up and terminate if this is completed dont perform this action on teardown
        await self._cleanup_test()
        self.cleanup = False


if __name__ == "__main__":
    default_matter_test_main()
