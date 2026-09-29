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
#       --discriminator 123
#       --passcode 2123
#       --KVS /tmp/chip_kvs_requestor
#       --autoApplyImage
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 123
#       --passcode 2123
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --string-arg provider_app_path:${OTA_PROVIDER_APP}
#       --string-arg ota_image:${SU_OTA_REQUESTOR_V2}
#       --string-arg provider_app_pipe_out:/tmp/provider_app_pipe_out_2_5
#       --string-arg provider_app_pipe:/tmp/provider_app_pipe_2_5
#       --int-arg ota_provider_port:5541
#       --timeout 2100
#     factory-reset: true
#     app-ready-pattern: Server initialization complete
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import time

from mobly import asserts
from TC_SUTestBase import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

STEP_RESERVE_SEC = 60

class TC_SU_2_5(SoftwareUpdateBaseTest):
    "This test case verifies that the DUT behaves according to the spec when it is applying the software update."
    provider_kvs_path = None
    provider_log = None
    current_requestor_app_pid = None
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
        # Used only when no --timeout is passed on the command line (CI passes
        # --timeout 2100 via the test-runner args above). Real devices have unbounded
        # download/apply times, so default to a generous budget: every long wait is
        # event-driven and ends early on fast devices, so an oversized budget only
        # costs time when something is genuinely wrong.
        return 7200

    @async_test_body
    async def teardown_test(self):
        await self.clear_ota_providers(self.controller, self.requestor_node_id)
        self.terminate_provider()
        self.clear_kvs(kvs_path_prefix=self.provider_kvs_path)
        super().teardown_test()

    @async_test_body
    async def setup_test(self):

        # Start budget
        self.start_test_budget_clock()

        # Set up Provider configuration and values for step1
        self.ota_image = self.user_params.get('ota_image')

        self.provider_app_path = self.user_params.get('provider_app_path')
        self.ota_provider_port = self.user_params.get('ota_provider_port', 5541)
        self.provider_kvs_path = self.user_params.get('provider_kvs_path', '/tmp/chip_kvs_provider')
        self.provider_log = self.user_params.get('provider_log_path', '/tmp/provider_log_2_5.log')
        self.provider_app_pipe_out = self.user_params.get('provider_app_pipe_out', '/tmp/provider_app_pipe_out_2_5')
        self.provider_app_pipe = self.user_params.get('provider_app_pipe', '/tmp/provider_app_pipe_2_5')
        # On average the ota image build for the CI is 1.8 MB which takes 4-5 min to download. Adjust if needed.

        if not self.provider_kvs_path.startswith('/tmp'):
            asserts.fail("Provider KVS path must be placed in the /tmp directory.")

        if not self.provider_app_path:
            asserts.fail("Missing provider app path. Specify using --string-arg provider_app_path:<provider_app_path>")

        if not self.ota_image:
            asserts.fail("Missing ota image path. Specify using --string-arg ota_image:<ota_image>")

        self.requestor_node_id = self.dut_node_id  # 123 with discriminator 123
        self.controller = self.default_controller

        # pipe out arguments
        self.provider_pipe_arguments = ['--app-pipe-out', self.provider_app_pipe_out, '--app-pipe', self.provider_app_pipe]
        logger.info("PROVIDER PIPE OUT %s, PROVIDER PIPE %s", self.provider_app_pipe_out, self.provider_app_pipe)
        # Extra Arguments required for the step 3
        delayed_apply_action_time = 60
        extra_arguments = ['--applyUpdateAction', 'awaitNextAction',
                           '--delayedApplyActionTimeSec', str(delayed_apply_action_time)] + self.provider_pipe_arguments

        # Check ota image and running software version, this will fail if is not suited to update the device else return the version to update
        self.expected_software_version = await self.check_ota_image_version(
            controller=self.controller, requestor_node_id=self.requestor_node_id, ota_image_path=self.ota_image)

        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_setup_pincode,
            discriminator=self.provider_discriminator,
            port=self.ota_provider_port,
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            extra_args=extra_arguments,
        )
        logger.info("About to start commissioning")
        await self.controller.CommissionOnNetwork(
            nodeId=self.provider_node_id,
            setupPinCode=self.provider_setup_pincode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.provider_discriminator
        )
        logger.info("Create ACL Entries")
        await self.create_acl_entry(dev_ctrl=self.controller,
                                    provider_node_id=self.provider_node_id, requestor_node_id=self.requestor_node_id)
        logger.info("Write OTA Providers")
        await self.set_default_ota_providers_list(controller=self.controller, provider_node_id=self.provider_node_id, requestor_node_id=self.requestor_node_id, endpoint=0)
        super().setup_test()

    def desc_TC_SU_2_5(self) -> str:
        return " [TC-SU-2.5] Handling Different ApplyUpdateResponse Scenarios on Requestor"

    def pics_TC_SU_2_5(self):
        """Return the PICS definitions associated with this test."""
        return ["MCORE.OTA.Requestor"]

    def steps_TC_SU_2_5(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "OTA-P/TH sends the ApplyUpdateResponse Command to the DUT. Action field is set to \"AwaitNextAction\", DelayedActionTime is set to 1 minute.", "Verify that the DUT waits for the minimum interval defined by spec which is 2 minutes before re-sending the ApplyUpdateRequest to the OTA-P."
                     "Verify that the DUT does not apply the software update within this time."),
            TestStep(2, "OTA-P/TH sends the ApplyUpdateResponse Command to the DUT. Action field is set to \"AwaitNextAction\", DelayedActionTime is set to 3 minutes. On the subsequent ApplyUpdateRequest command, TH/OTA-P sends the ApplyUpdateResponse back to DUT. Action field is set to \"Proceed\".", "Verify that the DUT waits for 3 minutes before sending the ApplyUpdateRequest to the OTA-P."
                     "Verify that the DUT does not apply the software update within this time."),
            TestStep(3, "OTA-P/TH sends the ApplyUpdateResponse Command to the DUT. Action field is set to \"Discontinue\".", "Verify that the DUT clears its previously downloaded software image, and resets the UpdateState Attribute to Idle."
                     "Verify that the DUT does not send the NotifyUpdateApplied within a reasonable time."
                     "Verify the SoftwareVersion attribute from the Basic Information cluster of the DUT to be the same as it was previously."),
        ]

    async def _wait_for_idle_after_software_update(self, update_state_handler):
        # Waits for Idle after the provider was cancelled

        update_state_match = AttributeMatcher.from_callable(
            "UpdateState is Idle",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle)
        update_state_handler.await_all_expected_report_matches([update_state_match], timeout_sec=600)
        update_state_handler.cancel()

    @async_test_body
    async def test_TC_SU_2_5(self):

        # Commissioning already done
        self.step(0)

        self.step(1)
        # This step the DUT should download the full ota image without applying the update
        # The transitions go as follow: kIdle->KQuerying->KDownloading (full OTA download)
        # After full ota download the DUT go to kDelayedOnapply and stay in this state during 120 seconds
        # The provider process is killed once the DUT is on kDelayedOnApply
        # The DUT must not install the software update in those 120 seconds
        # The DUT must not apply the software update as the provider was terminated and there is not a re request to apply the ota image
        # After the asserts wait the DUT to go back to kIdle to start the new step

        step_number_s1 = "[STEP 1]"
        spec_wait_time = 120
        current_sw_version = await self.read_single_attribute_check_success(
            dev_ctrl=self.controller,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.SoftwareVersion,
            node_id=self.requestor_node_id)

        subscription_attr_cluster = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=None  # receive all attributes
        )

        software_version_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=Clusters.BasicInformation.Attributes.SoftwareVersion
        )

        await self._start_subscription_bounded(
            subscription_attr_cluster, step_number_s1,
            dev_ctrl=self.controller,
            node_id=self.requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=True
        )

        await self._start_subscription_bounded(
            software_version_attr_handler, step_number_s1,
            dev_ctrl=self.controller,
            node_id=self.requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=True
        )

        # Announce the provider
        logger.info('%s: Step #1.0 - Controller sends AnnounceOTAProvider command', step_number_s1)
        await self.announce_ota_provider(self.controller, self.provider_node_id, self.requestor_node_id)

        state_sequence = []
        progress_values = []
        downloading_seen = False
        progress_seen = False
        download_completed = False

        def matcher_combined(report):
            """
            Combined matcher for Step 5:

            - Validates UpdateState reaches kDownloading
            - UpdateStateProgress has any value 1-100
            """
            nonlocal state_sequence, progress_values, downloading_seen, progress_seen, download_completed
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
                    if progress_seen and  val is not None and isinstance(val, int) and val ==  99:
                        download_completed = True

            return downloading_seen and progress_seen and download_completed

        matcher_combined_obj = AttributeMatcher.from_callable(
            description=f"{step_number_s1} - Step 1 matcher: Downloading and progress reach  99",
            matcher=matcher_combined
        )

        self.write_to_app_pipe(command_dict={"Name": "GetApplyUpdateRequestStatus"}, app_pipe=self.provider_app_pipe)
        pipe_data = self.read_from_app_pipe(self.provider_app_pipe_out)
        logger.info("%s Provider pipe status after Announce %s", step_number_s1, pipe_data)
        
        # Track the Download Progress and wait until the progress is completed
        matcher_combined_obj = AttributeMatcher.from_callable(
            description=f"{step_number_s1} - Step 1 matcher: Downloading + progress 1-100",
            matcher=matcher_combined
        )

        # Wait Download to Complete
        logger.info('%s: Step #1.0 - Wait OTA Download to complete', step_number_s1)
        subscription_attr_cluster.await_all_expected_report_matches(
            [matcher_combined_obj],
            timeout_sec=self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC))

        logger.info('%s: Step #1.0 - OTA Download abount to complete!', step_number_s1)
         
        logger.info('%s: Step #1.0 - Waiting for kDelayedOnApply', step_number_s1)
        # Wait Device to reach the status KDelayedOnApply (60seconds value 120 seconds spec)
        time_for_kDelayed_apply = subscription_attr_cluster.await_first_value_asserting_no_forbidden(
            target_value=Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply,
            forbidden_values=set(),
            timeout_sec=self.remaining_test_budget_sec(
            reserve_sec=STEP_RESERVE_SEC),
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )
        logger.info('%s: Step #1.0 - kDelayedOnApply value found after the complete download', step_number_s1)
        await asyncio.sleep(0.1)
        self.write_to_app_pipe(command_dict={"Name": "GetApplyUpdateRequestStatus"}, app_pipe=self.provider_app_pipe)
        pipe_data = self.read_from_app_pipe(self.provider_app_pipe_out)
        logger.info("%s Provider pipe status after kDelayedOnApply %s", step_number_s1, pipe_data)
        # Assert the values from the Provider named pipes
        asserts.assert_equal(pipe_data['Payload']['ApplyUpdateRequestActionResponse'],
                             Clusters.OtaSoftwareUpdateProvider.Enums.ApplyUpdateActionEnum.kAwaitNextAction, "Error on TH: the action from the Provider is not AwaitNextAction")
        asserts.assert_equal(pipe_data['Payload']['ApplyUpdateRequestCount'],
                             1, "Error on TH: Only one request should be sent from the Provider to DUT (Requestor).")

        # The provider needs to be terminated just before try to send the second ApplyUpdateRequest
        logger.info("%s Terminate the provider to avoid fully update the device ", step_number_s1)
        self.terminate_provider()

        # Device should stay in UpdateState:KApplying during 120 seconds and must not Apply the software update after the 60 seconds defined at delayedApplyActionTimeSec.
        software_version_match = AttributeMatcher.from_callable(
            f"Software Version should be: {current_sw_version}",
            lambda report: report.value == current_sw_version)
        # Guard the software version attribute
        logger.info("%s About to start guard of the Software Version for 120 seconds with no changes", step_number_s1)
        software_version_attr_handler.wait_all_final_values_reported_persisted(
            expected_matchers=[software_version_match], timeout_sec=spec_wait_time)
        logger.info("%s Completed guard of the Software Version for 120 seconds with no changes!", step_number_s1)

        software_version_attr_handler.reset()
        software_version_attr_handler.cancel()
        
        time_for_kApplying = subscription_attr_cluster.await_first_value_asserting_no_forbidden(
            target_value=Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying,
            forbidden_values={Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply},
            timeout_sec=STEP_RESERVE_SEC,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )
        time_taken_from_delay_to_apply_s1 = time_for_kApplying - time_for_kDelayed_apply
        logger.info("%s Time taken from kDelayedOnApply to KApplying %s",step_number_s1,time_taken_from_delay_to_apply_s1)
        asserts.assert_greater_equal(time_taken_from_delay_to_apply_s1, 120 , "Time for kApplying is lower than 120 seconds")


        logger.info("%s Waiting device go back to kIdle after kApplying and provider is terminated.", step_number_s1)
        # Requestor did not receive the ApplyUpdateResponse from the Provider as it was terminated before resending.
        await self._wait_for_idle_after_software_update(update_state_handler=subscription_attr_cluster)
        subscription_attr_cluster.cancel()

        
        # Software version should stay the same as the second ApplyUpdateRequest was not sent when the provider was terminated before re-sending the ApplyUpdateRequest
        new_software_version = await self.verify_version_applied_basic_information(controller=self.controller, node_id=self.requestor_node_id, target_version=current_sw_version)
        logger.info("%s Sofware version is still in the same from the start of the test Start Version:%d, End Version:%d! and is back to kIdle", step_number_s1, current_sw_version,new_software_version)

        self.step(2)
        step_number_s2 = "[STEP 2]"
        delayed_apply_action_time = 180
        extra_arguments = ['--applyUpdateAction', 'awaitNextAction',
                           '--delayedApplyActionTimeSec', str(delayed_apply_action_time)] + self.provider_pipe_arguments
        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_setup_pincode,
            discriminator=self.provider_discriminator,
            port=self.ota_provider_port,
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            extra_args=extra_arguments,
        )
        
        # Software Version attribute handler
        software_version_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=Clusters.BasicInformation.Attributes.SoftwareVersion
        )
        #  OTA Requestor attribute handler
        subscription_attr_cluster = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=None
        )

        await self._start_subscription_bounded(
            subscription_attr_cluster, step_number_s2,
            dev_ctrl=self.controller,
            node_id=self.requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=True
        )

        await self._start_subscription_bounded(
            software_version_attr_handler, step_number_s2,
            dev_ctrl=self.controller,
            node_id=self.requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=True
        )

        # Actions before annouce
        await self._wait_until_idle_before_announce(
            controller=self.controller,
            requestor_node_id=self.requestor_node_id,
            subscription=subscription_attr_cluster,
            #timeout_sec=IDLE_BEFORE_ANNOUNCE_TIMEOUT_SEC,
            timeout_sec=120,
            step_name=step_number_s2,
        )

        await self._announce_until_provider_queried(
            controller=self.controller,
            provider_node_id=self.provider_node_id,
            requestor_node_id=self.requestor_node_id,
            timeout_sec=self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC),
            step_name=step_number_s2,
        )

        # Variables for matcher_combined from step1 that need to be reset in order to use them for step2
        state_sequence = []
        progress_values = []
        downloading_seen = False
        progress_seen = False
        download_completed = False

        # Uses the matcher combined method to track the value until 99
        matcher_combined_obj = AttributeMatcher.from_callable(
            description=f"{step_number_s2} - Step 2 matcher: Downloading and progress reach  99",
            matcher=matcher_combined
        )

        self.write_to_app_pipe(command_dict={"Name": "GetApplyUpdateRequestStatus"}, app_pipe=self.provider_app_pipe)
        pipe_data = self.read_from_app_pipe(self.provider_app_pipe_out)
        logger.info("%s Provider pipe status after Announce %s", step_number_s2, pipe_data)
        
        # Track the Download Progress and wait until the progress is completed
        matcher_combined_obj = AttributeMatcher.from_callable(
            description=f"{step_number_s2} - Step 2 matcher: Downloading + progress 1-100",
            matcher=matcher_combined
        )

        # Wait device to to start the download and complete
        logger.info('%s: Step #2.0 - Wait OTA Download to complete', step_number_s2)
        subscription_attr_cluster.await_all_expected_report_matches(
            [matcher_combined_obj],
            timeout_sec=self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC))
        
        logger.info("%s Waiting the time of DelayedApplyAction of %s seconds.", step_number_s2, delayed_apply_action_time)
        update_state_match = AttributeMatcher.from_callable(
            "UpdateState is kDelayedOnApply",
            lambda report: report.attribute == Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState and report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply)
        subscription_attr_cluster.await_all_expected_report_matches(
            [update_state_match], timeout_sec=STEP_RESERVE_SEC)
        time_for_kdelayed_apply_s2 = time.time()

        # Avoid race condition
        await asyncio.sleep(0.1)

        # Read the named pipe
        self.write_to_app_pipe(command_dict={"Name": "GetApplyUpdateRequestStatus"}, app_pipe=self.provider_app_pipe)
        pipe_data = self.read_from_app_pipe(self.provider_app_pipe_out)
        logger.info("%s Provider pipe status after kDelayedOnApply %s", step_number_s2, pipe_data)
        
        asserts.assert_equal(pipe_data['Payload']['ApplyUpdateRequestActionResponse'],
                                Clusters.OtaSoftwareUpdateProvider.Enums.ApplyUpdateActionEnum.kAwaitNextAction, "Action from the provider is not AwaitNextAction")
        asserts.assert_equal(pipe_data['Payload']['ApplyUpdateRequestCount'],
                                1, "Only one request should be sent from the Provider")

        # Kill the provider before it resends the ApplyUpdateRequest
        logger.info("%s Terminate the provider to avoid fully update the device ", step_number_s1)
        self.terminate_provider()


        # Device should stay kDelayedOnApply and do not apply the software update during this time (180 seconds).
        software_version_match = AttributeMatcher.from_callable(
            f"Sofware Version should be: {current_sw_version}",
            lambda report: report.value == current_sw_version)
        software_version_attr_handler.wait_all_final_values_reported_persisted(
            expected_matchers=[software_version_match], timeout_sec=delayed_apply_action_time)

        # Wait for the kApplying State
        update_state_match = AttributeMatcher.from_callable(
            "UpdateState is kApplying",
            lambda report: Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState and report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying)
        subscription_attr_cluster.await_all_expected_report_matches(
            [update_state_match], timeout_sec=STEP_RESERVE_SEC)
        time_to_kapplying_s2 = time.time()
        time_taken_from_delay_to_apply_s2 = time_to_kapplying_s2 - time_for_kdelayed_apply_s2

        logger.info("%s Time taken from kDelayedOnApply to KApplying %s",step_number_s2,time_taken_from_delay_to_apply_s2)
        asserts.assert_greater_equal(time_taken_from_delay_to_apply_s2, delayed_apply_action_time , "Time for kApplying is lower than 180 seconds")

        software_version_attr_handler.reset()
        software_version_attr_handler.cancel()

        # Requestor did not receive the second ApplyUpdateRequest from the Provider as it was terminated before resending (ApplyUpdateRequest)
        await self._wait_for_idle_after_software_update(update_state_handler=subscription_attr_cluster)
        subscription_attr_cluster.cancel()

        # Verify the version is the same as the second ApplyUpdateRequest was not sent.
        software_version = await self.verify_version_applied_basic_information(controller=self.controller, node_id=self.requestor_node_id, target_version=current_sw_version)
        logger.info("%s Sofware version is still in the same from the start of the test Start Version:%d, End Version:%d! and is back to kIdle", step_number_s2, current_sw_version,new_software_version)

        self.step(3)
        step_number_s3 = "[STEP 3]"
        extra_arguments = ['--applyUpdateAction', 'discontinue'] + self.provider_pipe_arguments
        
        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_setup_pincode,
            discriminator=self.provider_discriminator,
            port=self.ota_provider_port,
            kvs_path=self.provider_kvs_path,
            log_file=self.provider_log,
            extra_args=extra_arguments,
        )
               
        # Software Version attribute handler
        software_version_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=Clusters.BasicInformation.Attributes.SoftwareVersion
        )
        #  OTA Requestor attribute handler
        subscription_attr_cluster = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=None
        )

        await self._start_subscription_bounded(
            subscription_attr_cluster, step_number_s3,
            dev_ctrl=self.controller,
            node_id=self.requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=True
        )

        await self._start_subscription_bounded(
            software_version_attr_handler, step_number_s3,
            dev_ctrl=self.controller,
            node_id=self.requestor_node_id,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=True
        )

        # Actions before annouce
        await self._wait_until_idle_before_announce(
            controller=self.controller,
            requestor_node_id=self.requestor_node_id,
            subscription=subscription_attr_cluster,
            #timeout_sec=IDLE_BEFORE_ANNOUNCE_TIMEOUT_SEC,
            timeout_sec=120,
            step_name=step_number_s3,
        )

        await self._announce_until_provider_queried(
            controller=self.controller,
            provider_node_id=self.provider_node_id,
            requestor_node_id=self.requestor_node_id,
            timeout_sec=self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC),
            step_name=step_number_s3,
        )

        
        # Wait fort the DUT to start downloading
        logger.info("%s Waiting for DUT to reach the kDownloading state.",step_number_s3)
        update_state_match = AttributeMatcher.from_callable(
            "Waiting UpdateState is Downloading",
            lambda report: report.attribute == Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState and  report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading)
        subscription_attr_cluster.await_all_expected_report_matches([update_state_match], timeout_sec=600)

        # This can be only be tested on CI or Locally, real devices might not have this path available.
        if self.is_pics_sdk_ci_only:
            logger.info("%s Verify ota image info.",step_number_s3)
            # State is Downloading, let it run a few seconds to have some data to check.
            await asyncio.sleep(3)
            # Verify the default download path and the file size
            # Read file for /tmp/test.bin should exists and greater than 0 bytes
            ota_file_data = self.get_downloaded_ota_image_info()
            logger.info("Downloaded ota image data %s", ota_file_data)
            asserts.assert_equal(True, ota_file_data['exists'], f"File was not downloaded  at {ota_file_data['path']}")
            asserts.assert_greater(ota_file_data['size'], 0, "Downloaded file is still at 0")

        # Device is downloading the image
        progress_seen = False
        last_progress = 0
        download_completed = False

        def check_ota_download_matcher(report):
            """Check for the UpdateStateProgress and confirms it downloaded the image when the
                status reach to NullValue
            Args:
                report : Report value
            """
            nonlocal progress_seen, last_progress, download_completed
            attribute = report.attribute
            value = report.value
        
            # UpdateStateProgress
            if attribute == Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress:
                if value is not None and isinstance(value, int) and 1 <= value <= 100:
                    if not progress_seen:
                        progress_seen = True
                if progress_seen and  value ==  NullValue:
                        download_completed = True
            return download_completed

        logger.info("%s Waiting OTA download to complete.",step_number_s3)
        download_progress_attr_matcher_obj = AttributeMatcher.from_callable(
            description="Waiting Download to Complete ", matcher=check_ota_download_matcher)
        subscription_attr_cluster.await_all_expected_report_matches(
            [download_progress_attr_matcher_obj], timeout_sec=self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC))
    
        logger.info("%s OTA download completed, read the ApplyUpdateRequestActionResponse set to the Provider",step_number_s3)
        # Use named pipes to confirm the ApplyUpdateAction
        await asyncio.sleep(0.1)
        self.write_to_app_pipe(command_dict={"Name": "GetApplyUpdateRequestStatus"}, app_pipe=self.provider_app_pipe)
        pipe_data = self.read_from_app_pipe(self.provider_app_pipe_out)
        logger.info("%s Provider pipe status after kDownload and kDiscontinue enabled %s", step_number_s3 ,pipe_data)
        asserts.assert_equal(pipe_data['Payload']['ApplyUpdateRequestActionResponse'],
                             Clusters.OtaSoftwareUpdateProvider.Enums.ApplyUpdateActionEnum.kDiscontinue, "Action from the provider is not kDiscontinue")
        asserts.assert_equal(pipe_data['Payload']['ApplyUpdateRequestCount'],
                             1, "Only one request should be sent from the Provider")

        # DUT did not apply and goes to kIdle as the action was set to Discontinue.
        logger.info("%s Waiting DUT go back to kIdle as the ApplyUpdateAction is Discontinue",step_number_s3)
        subscription_attr_cluster.await_all_expected_report_matches(
            [update_state_match], timeout_sec=self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC))
        update_state_match = AttributeMatcher.from_callable(
            "Waiting UpdateState is Idle",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle)
        subscription_attr_cluster.await_all_expected_report_matches(
            [update_state_match], self.remaining_test_budget_sec(reserve_sec=STEP_RESERVE_SEC))
        
        subscription_attr_cluster.cancel()

        if self.is_pics_sdk_ci_only:
            ota_file_data = self.get_downloaded_ota_image_info()
            logger.info("Downloaded ota image data %s", ota_file_data)
            asserts.assert_equal(ota_file_data['exists'], False, f"Downloaded file is still present {ota_file_data['path']}")
            asserts.assert_equal(ota_file_data['size'], 0, "File size is greater than 0")

        
        update_state_progress = await self.read_single_attribute_check_success(
            Clusters.OtaSoftwareUpdateRequestor, Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress, self.controller, self.requestor_node_id, 0)
        asserts.assert_equal(update_state_progress, NullValue, "Progress is not Null")
        logger.info("%s Current UpdateStateProgres is: %s as expected",step_number_s3, update_state_progress)
        # Verify version is the same as when it  started
        new_software_version = await self.verify_version_applied_basic_information(self.controller, self.requestor_node_id, current_sw_version)
        logger.info("%s Sofware version is still in the same from the start of the test Start Version:%d, End Version:%d! and is back to kIdle", step_number_s3, current_sw_version, new_software_version)


if __name__ == "__main__":
    default_matter_test_main()
