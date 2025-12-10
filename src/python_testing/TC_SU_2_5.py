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
#       --int-arg ota_image_expected_version:2
#       --int-arg ota_provider_port:5541
#       --int-arg ota_image_download_timeout:300
#       --timeout 600
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import contextlib
import logging

from mobly import asserts
from TC_SUTestBase import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)


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
    ota_image_download_timeout = None

    @async_test_body
    async def teardown_test(self):
        await self.clear_ota_providers(self.controller, self.requestor_node_id)
        self.terminate_provider()
        self.clear_kvs(kvs_prefix=self.provider_kvs_path)
        super().teardown_test()

    @async_test_body
    async def setup_test(self):
        # Set up Provider configuration and values for step1
        self.ota_image = self.user_params.get('ota_image')
        self.expected_software_version = self.user_params.get('ota_image_expected_version')
        self.provider_app_path = self.user_params.get('provider_app_path')
        self.ota_provider_port = self.user_params.get('ota_provider_port', 5541)
        self.provider_kvs_path = self.user_params.get('provider_kvs_path', '/tmp/chip_kvs_provider')
        self.provider_log = self.user_params.get('provider_log_path', '/tmp/provider_log_2_5.log')
        # On average the ota image build for the CI is 1.8 MB which takes 4-5 min to download. Adjust if needed.
        self.ota_image_download_timeout = self.user_params.get('ota_image_download_timeout', 60*5)
        logger.info(f"Image download timeout is set to {self.ota_image_download_timeout} seconds")

        if self.ota_image_download_timeout <= 0:
            asserts.fail("Invalid value for --int-arg ota_image_download_timeout:<seconds> value provided, must be equal or greater than 1.")

        if not self.expected_software_version:
            asserts.fail("Missing OTA image software version. Speficy using --int-arg ota_image_expected_version:<ota_image_expected_version>")

        if not self.provider_app_path:
            asserts.fail("Missing provider app path . Speficy using --string-arg provider_app_path:<provider_app_path>")

        if not self.ota_image:
            asserts.fail("Missing ota image path . Speficy using --string-arg ota_image:<ota_image>")

        self.requestor_node_id = self.dut_node_id  # 123 with discriminator 123
        self.requestor_passcode = self.matter_test_config.setup_passcodes[0]
        self.controller = self.default_controller
        # Extra Arguments required for the step 1
        extra_arguments = ['--applyUpdateAction', 'proceed', '--delayedApplyActionTimeSec', '0']

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
            TestStep(1, "OTA-P/TH sends the ApplyUpdateResponse Command to the DUT. Action field is set to \"Proceed\", DelayedActionTime is set to 0.", "Verify that the DUT starts updating its software."
                     "Once the update is finished, verify the SoftwareVersion attribute from the Basic Information cluster on the DUT to match the version downloaded for the software update."
                     "Verify on the OTA-P/TH that there is no other ApplyUpdateRequest from the DUT."),
            TestStep(2, "OTA-P/TH sends the ApplyUpdateResponse Command to the DUT. Action field is set to \"Proceed\", DelayedActionTime is set to 3 minutes.",
                     "Verify that the DUT starts updating its software after 3 minutes. Once the update is finished, verify the SoftwareVersion attribute from the Basic Information cluster on the DUT to match the version downloaded for the software update."),
            TestStep(3, "OTA-P/TH sends the ApplyUpdateResponse Command to the DUT. Action field is set to \"AwaitNextAction\", DelayedActionTime is set to 1 minute.", "Verify that the DUT waits for the minimum interval defined by spec which is 2 minutes before re-sending the ApplyUpdateRequest to the OTA-P."
                     "Verify that the DUT does not apply the software update within this time."),
            TestStep(4, "OTA-P/TH sends the ApplyUpdateResponse Command to the DUT. Action field is set to \"AwaitNextAction\", DelayedActionTime is set to 3 minutes. On the subsequent ApplyUpdateRequest command, TH/OTA-P sends the ApplyUpdateResponse back to DUT. Action field is set to \"Proceed\".", "Verify that the DUT waits for 3 minutes before sending the ApplyUpdateRequest to the OTA-P."
                     "Verify that the DUT starts updating its software after the second ApplyUpdateResponse with Proceed action."
                     "Once the update is finished, verify the SoftwareVersion attribute from the Basic Information cluster on the DUT to match the version downloaded for the software update."),
            TestStep(5, "OTA-P/TH sends the ApplyUpdateResponse Command to the DUT. Action field is set to \"Discontinue\".", "Verify that the DUT clears its previously downloaded software image, and resets the UpdateState Attribute to Idle."
                     "Verify that the DUT does not send the NotifyUpdateApplied within a reasonable time."
                     "Verify the SoftwareVersion attribute from the Basic Information cluster of the DUT to be the same as it was previously."),
        ]

    @async_test_body
    async def test_TC_SU_2_5(self):

        self.step(0)

        self.step(1)
        update_state_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )
        await update_state_attr_handler.start(dev_ctrl=self.controller, node_id=self.requestor_node_id, endpoint=0,
                                              fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)
        await self.announce_ota_provider(self.controller, self.provider_node_id, self.requestor_node_id)

        update_state_match = AttributeMatcher.from_callable(
            "Update state is Downloading",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading)
        update_state_attr_handler.await_all_expected_report_matches([update_state_match], timeout_sec=60)

        update_state_match = AttributeMatcher.from_callable(
            "Update state is Applying",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying)
        update_state_attr_handler.await_all_expected_report_matches(
            [update_state_match], timeout_sec=self.ota_image_download_timeout)

        # Wait to the idle status for good period of time like 600 seconds.
        update_state_match = AttributeMatcher.from_callable(
            "Update state is Idle",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle)
        update_state_attr_handler.await_all_expected_report_matches(
            [update_state_match], timeout_sec=600)

        update_state_attr_handler.flush_reports()
        update_state_attr_handler.cancel()

        # Once in idle verify the version match the expected software version
        await self.verify_version_applied_basic_information(
            controller=self.controller, node_id=self.requestor_node_id, target_version=self.expected_software_version)
        update_state = await self.read_single_attribute_check_success(
            Clusters.OtaSoftwareUpdateRequestor, Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState, self.controller, self.requestor_node_id, 0)
        asserts.assert_equal(update_state, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle,
                             "Update state should be idle")
        self.terminate_provider()
        self.restart_requestor()

        self.step(2)
        # Set values for step 2
        delayed_apply_action_time = 60*3
        current_sw_version = await self.read_single_attribute_check_success(
            dev_ctrl=self.controller,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.SoftwareVersion,
            node_id=self.requestor_node_id)
        extra_arguments = ['--applyUpdateAction', 'proceed', '--delayedApplyActionTimeSec', str(delayed_apply_action_time)]
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

        # Software Version Attr Handler
        software_version_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=Clusters.BasicInformation.Attributes.SoftwareVersion
        )

        # UpdateState Handler
        update_state_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )
        await software_version_attr_handler.start(dev_ctrl=self.controller, node_id=self.requestor_node_id, endpoint=0,
                                                  fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)

        await update_state_attr_handler.start(dev_ctrl=self.controller, node_id=self.requestor_node_id, endpoint=0,
                                              fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)

        await self.announce_ota_provider(self.controller, self.provider_node_id, self.requestor_node_id)

        update_state_match = AttributeMatcher.from_callable(
            "Update state is Downloading",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading)
        update_state_attr_handler.await_all_expected_report_matches([update_state_match], timeout_sec=60)

        update_state_match = AttributeMatcher.from_callable(
            "Update state is Applying",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying)
        update_state_attr_handler.await_all_expected_report_matches(
            [update_state_match], timeout_sec=self.ota_image_download_timeout)

        # Device should stay in ApplyingState During 180 seconds and not Apply the software Update after the 60 seconds.
        software_version_match = AttributeMatcher.from_callable(
            f"Sofware Version should be: {current_sw_version}",
            lambda report: report.value == current_sw_version)
        software_version_attr_handler.await_all_period_expected_report_matches(
            expected_matchers=[software_version_match], timeout_sec=delayed_apply_action_time)

        software_version_attr_handler.flush_reports()
        software_version_attr_handler.cancel()
        update_state_match = AttributeMatcher.from_callable(
            "Update state is kIdle",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle)
        update_state_attr_handler.await_all_expected_report_matches(
            [update_state_match], timeout_sec=600)
        update_state_attr_handler.flush_reports()
        update_state_attr_handler.cancel()

        await self.verify_version_applied_basic_information(
            controller=self.controller, node_id=self.requestor_node_id, target_version=self.expected_software_version)
        # Terminate the provider
        self.terminate_provider()
        self.restart_requestor()

        self.step(3)
        delayed_apply_action_time = 60
        spec_wait_time = 120
        extra_arguments = ['--applyUpdateAction', 'awaitNextAction', '--delayedApplyActionTimeSec', str(delayed_apply_action_time)]
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
        current_sw_version = await self.read_single_attribute_check_success(
            dev_ctrl=self.controller,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.SoftwareVersion,
            node_id=self.requestor_node_id)
        # Software Version Attr Handler
        software_version_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=Clusters.BasicInformation.Attributes.SoftwareVersion
        )
        # StateUpdate Attr Handler
        update_state_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )
        await software_version_attr_handler.start(dev_ctrl=self.controller, node_id=self.requestor_node_id, endpoint=0,
                                                  fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)
        await update_state_attr_handler.start(dev_ctrl=self.controller, node_id=self.requestor_node_id, endpoint=0,
                                              fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)
        await self.announce_ota_provider(self.controller, self.provider_node_id, self.requestor_node_id)

        update_state_match = AttributeMatcher.from_callable(
            "Update state is Downloading",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading)
        update_state_attr_handler.await_all_expected_report_matches([update_state_match], timeout_sec=60)

        # Waits for nextAction
        update_state_match = AttributeMatcher.from_callable(
            "Update state is kDelayedOnApply",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply)
        update_state_attr_handler.await_all_expected_report_matches(
            [update_state_match], timeout_sec=self.ota_image_download_timeout)

        # Wwitches to Applying
        update_state_match = AttributeMatcher.from_callable(
            "Update state is kApplying",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying)
        update_state_attr_handler.await_all_expected_report_matches(
            [update_state_match], timeout_sec=5)

        # Device should stay in ApplyingState During 120 seconds and not Apply the software Update after the 60 seconds.
        software_version_match = AttributeMatcher.from_callable(
            f"Sofware Version should be: {current_sw_version}",
            lambda report: report.value == current_sw_version)
        software_version_attr_handler.await_all_period_expected_report_matches(
            expected_matchers=[software_version_match], timeout_sec=spec_wait_time)

        # Lets wait the device to switch idle state (After Update)
        update_state_match = AttributeMatcher.from_callable(
            "Update state is kIdle",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle)
        update_state_attr_handler.await_all_expected_report_matches(
            [update_state_match], timeout_sec=600)
        update_state_attr_handler.reset()
        # Cancel handlers
        update_state_attr_handler.cancel()
        software_version_attr_handler.reset()
        software_version_attr_handler.cancel()

        # Now software version should be in the expected software version
        await self.verify_version_applied_basic_information(controller=self.controller, node_id=self.requestor_node_id, target_version=self.expected_software_version)
        self.terminate_provider()
        self.restart_requestor()

        self.step(4)
        delayed_apply_action_time = 180
        extra_arguments = ['--applyUpdateAction', 'awaitNextAction', '--delayedApplyActionTimeSec', str(delayed_apply_action_time)]
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
        current_sw_version = await self.read_single_attribute_check_success(
            dev_ctrl=self.controller,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.SoftwareVersion,
            node_id=self.requestor_node_id)
        # Software Version attr handler
        software_version_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation,
            expected_attribute=Clusters.BasicInformation.Attributes.SoftwareVersion
        )
        #  UpdateState attr handler
        update_state_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )
        await update_state_attr_handler.start(dev_ctrl=self.controller, node_id=self.requestor_node_id, endpoint=0,
                                              fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)
        await software_version_attr_handler.start(dev_ctrl=self.controller, node_id=self.requestor_node_id, endpoint=0,
                                                  fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)
        await self.announce_ota_provider(self.controller, self.provider_node_id, self.requestor_node_id)

        update_state_match = AttributeMatcher.from_callable(
            "Update state is Downloading",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading)
        update_state_attr_handler.await_all_expected_report_matches([update_state_match], timeout_sec=60)

        update_state_match = AttributeMatcher.from_callable(
            "Update state is kDelayedOnApply",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply)
        update_state_attr_handler.await_all_expected_report_matches(
            [update_state_match], timeout_sec=self.ota_image_download_timeout)
        logger.info(f"Waiting the time of DelayedApplyAction of {delayed_apply_action_time} seconds.")

        # Device should stay in ApplyingState and not apply the update during the 180 seconds. Only after this timeframe.
        software_version_match = AttributeMatcher.from_callable(
            f"Sofware Version should be: {current_sw_version}",
            lambda report: report.value == current_sw_version)
        software_version_attr_handler.await_all_period_expected_report_matches(
            expected_matchers=[software_version_match], timeout_sec=delayed_apply_action_time)

        # Lets wait the device to switch idle state (After Update)
        update_state_match = AttributeMatcher.from_callable(
            "Update state is kIdle",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle)
        update_state_attr_handler.await_all_expected_report_matches(
            [update_state_match], timeout_sec=600)

        update_state_attr_handler.reset()
        update_state_attr_handler.cancel()
        software_version_attr_handler.reset()
        software_version_attr_handler.cancel()

        # Verify the version is the same
        await self.verify_version_applied_basic_information(controller=self.controller, node_id=self.requestor_node_id, target_version=self.expected_software_version)
        self.terminate_provider()
        self.restart_requestor()

        self.step(5)
        extra_arguments = ['--applyUpdateAction', 'discontinue']
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
        current_sw_version = await self.read_single_attribute_check_success(
            dev_ctrl=self.controller,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.SoftwareVersion,
            node_id=self.requestor_node_id)
        update_state_attr_handler = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )
        await update_state_attr_handler.start(dev_ctrl=self.controller, node_id=self.requestor_node_id, endpoint=0,
                                              fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)
        await self.announce_ota_provider(self.controller, self.provider_node_id, self.requestor_node_id)
        # Wait Until Downloading
        update_state_match = AttributeMatcher.from_callable(
            "Waiting Update state is Downloading",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading)

        update_state_attr_handler.await_all_expected_report_matches([update_state_match], timeout_sec=90)
        # State is Downloading, let it run a few secnods to have some data to check.
        await asyncio.sleep(3)
        # Verify the default download path and the file size
        # Read file for /tmp/test.bin should exists and greater than 0
        ota_file_data = self.get_downloaded_ota_image_info()
        logger.info(f"Downloaded ota image data {str(ota_file_data)}")
        asserts.assert_equal(True, ota_file_data['exists'], f"File is was not downloaded  at {ota_file_data['path']}")
        asserts.assert_greater(ota_file_data['size'], 0, "Downloaded file is still at 0")

        # Applying
        update_state_match = AttributeMatcher.from_callable(
            "Waiting Update state is Applying",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying)

        update_state_attr_handler.await_all_expected_report_matches(
            [update_state_match], timeout_sec=self.ota_image_download_timeout)
        update_state_match = AttributeMatcher.from_callable(
            "Waiting Update state is Idle",
            lambda report: report.value == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle)
        update_state_attr_handler.await_all_expected_report_matches([update_state_match], timeout_sec=60)
        update_state_attr_handler.reset()
        update_state_attr_handler.cancel()

        # Make sure attr is Idle
        update_state = await self.read_single_attribute_check_success(
            Clusters.OtaSoftwareUpdateRequestor, Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState, self.controller, self.requestor_node_id, 0)
        asserts.assert_equal(update_state, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle,
                             "Update state is not idle")
        ota_file_data = self.get_downloaded_ota_image_info()
        logger.info(f"Downloaded ota image data {str(ota_file_data)}")
        asserts.assert_equal(ota_file_data['exists'], False, f"Downloaded file is still present {ota_file_data['path']}")
        asserts.assert_equal(ota_file_data['size'], 0, "File size is greater than 0")
        update_state_progress = await self.read_single_attribute_check_success(
            Clusters.OtaSoftwareUpdateRequestor, Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress, self.controller, self.requestor_node_id, 0)
        asserts.assert_equal(update_state_progress, NullValue, "Progress is not Null")
        logger.info(f"Progress is {update_state_progress}")
        # Verify version is the same as when it  started
        await self.verify_version_applied_basic_information(self.controller, self.requestor_node_id, current_sw_version)


if __name__ == "__main__":
    default_matter_test_main()
