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
#       --secured-device-port 5540
#       --KVS /tmp/chip_kvs_requestor
#       --autoApplyImage
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 123
#       --passcode 2123
#       --endpoint 0
#       --nodeId 123
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
from os import environ, getcwd, path
from signal import SIGTERM
from subprocess import run
from time import sleep
from TC_SUBase import SoftwareUpdateBaseTest

from mobly import asserts
from typing import Union

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


logger = logging.getLogger(__name__)


class TC_SU_2_5(SoftwareUpdateBaseTest):
    "This test case verifies that the DUT behaves according to the spec when it is applying the software update."

    def desc_TC_SU_2_5(self) -> str:
        return " [TC-SU-2.5] Handling Different ApplyUpdateResponse Scenarios on Requestor"

    def pics_TC_SU_2_5(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "MCORE.OTA.Requestor",
        ]
        return pics

    def steps_TC_SU_2_5(self) -> list[TestStep]:
        steps = [
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
        return steps

    def _terminate_provider_process(self):
        if self.current_provider_app_proc is not None:
            logger.info(f"Terminating provider with pid {self.current_provider_app_proc.get_pid()}")
            self.current_provider_app_proc.terminate()
            run("rm -rf /tmp/chip_kvs_provider*", shell=True)
            self.current_provider_app_proc = None
            # Wait SIGTERM
            sleep(2)

    @async_test_body
    async def teardown_test(self):
        self._terminate_provider_process()
        # self._terminate_requestor_process()

    @async_test_body
    async def test_TC_SU_2_5(self):

        self.step(0)
        controller = self.default_controller
        requestor_node_id = self.dut_node_id  # 123 with discriminator 123

        # Provider
        provider_data = {
            "node_id": 321,
            "discriminator": 321,
            "setup_pincode": 2321
        }
        self.step(1)
        expected_software_version = 2
        self.launch_provider_app(extra_args=['--applyUpdateAction', 'proceed', '--delayedApplyActionTimeSec', '0'])
        await controller.CommissionOnNetwork(
            nodeId=provider_data['node_id'],
            setupPinCode=provider_data['setup_pincode'],
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_data['discriminator']
        )

        basicinformation_handler = EventSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation, expected_event_id=Clusters.BasicInformation.Events.ShutDown.event_id)
        await basicinformation_handler.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=6000)

        event_state_transition = EventSubscriptionHandler(expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
                                                          expected_event_id=Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition.event_id)
        await event_state_transition.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=6000)

        await self.current_provider_app_proc.create_acl_entry(dev_ctrl=controller, provider_node_id=provider_data['node_id'], requestor_node_id=requestor_node_id)
        await self.write_ota_providers(controller=controller, provider_node_id=provider_data['node_id'], endpoint=0)
        await self.announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)

        # wait for Download event this means updating has started
        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=30)
        logger.info(f"Event report for Querying : {event_report}")

        # wait for Download event this means updating has started
        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=30)
        logger.info(f"Event report for Downloading : {event_report}")

        # Wait for shutdown event ( this is triggered after the SU Completition)
        shutdown_event = basicinformation_handler.wait_for_event_report(Clusters.BasicInformation.Events.ShutDown, timeout_sec=60*6)
        logger.info(f"Event report for ShutDown : {shutdown_event}")
        # Cancel eventhandlers
        event_state_transition.flush_events()
        await event_state_transition.cancel()

        basicinformation_handler.flush_events()
        await basicinformation_handler.cancel()

        # Just wait for the device to StartUp after ShutDown
        await asyncio.sleep(5)
        # Verify software version after StartUp
        self.verify_version_applied_basic_information(
            controller=controller, node_id=requestor_node_id, target_version=expected_software_version)

        # Verify on te OTA-P Logs  and confirm there is no other ApplyUpdateRequest form the DUT
        expected_line = 'Provider received ApplyUpdateRequest'
        found_lines = self.current_provider_app_proc.read_from_logs(expected_line, regex=False)
        asserts.assert_equal(len(found_lines), 1,
                             f"Zero or more than 1 lines , but not 1 line containing {expected_line} found in the logs")

        self._terminate_provider_process()
        controller.ExpireSessions(nodeid=provider_data['node_id'])

        self.step(2)
        expected_software_version = 3
        delayed_apply_action_time = 60*3
        self.launch_provider_app(extra_args=['--applyUpdateAction', 'proceed',
                                             '--delayedApplyActionTimeSec', str(delayed_apply_action_time)], version=expected_software_version)
        await controller.CommissionOnNetwork(
            nodeId=provider_data['node_id'],
            setupPinCode=provider_data['setup_pincode'],
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_data['discriminator']
        )

        event_state_transition = EventSubscriptionHandler(expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
                                                          expected_event_id=Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition.event_id)
        await event_state_transition.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=6000)
        basicinformation_handler = EventSubscriptionHandler(
            expected_cluster=Clusters.BasicInformation, expected_event_id=Clusters.BasicInformation.Events.ShutDown.event_id)
        await basicinformation_handler.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=6000)

        await self.current_provider_app_proc.create_acl_entry(dev_ctrl=controller, provider_node_id=provider_data['node_id'], requestor_node_id=requestor_node_id)
        await self.write_ota_providers(controller=controller, provider_node_id=provider_data['node_id'], endpoint=0)
        await self.announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)

        # Report of failed and waiting on Idle
        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=30)
        logger.info(f"Event report for Idle : {event_report}")

        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=30)
        logger.info(f"Event report for Query : {event_report}")

        # Downloading
        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=30)
        logger.info(f"Event report for Downloading : {event_report}")

        update_state_prev = await self.read_single_attribute_check_success(
            Clusters.OtaSoftwareUpdateRequestor, Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState, controller, requestor_node_id)
        logger.info("UpdateState before update " + str(update_state_prev))

        # Applying this wil trigger the "delayedActionTime: 180 seconds" in the logs
        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=60*5)
        logger.info(f"Event report for Applying : {event_report}")

        # Should not get a report at least for the following 3 minutes
        logger.info("Waiting for 3 minutes without events")
        event_state_transition.wait_for_event_expect_no_report(timeout_sec=delayed_apply_action_time)

        # Cancel State Transition
        event_state_transition.flush_events()
        await event_state_transition.cancel()

        # Wait for shutdown event (this is triggered after the SU Completition)
        shutdown_event = basicinformation_handler.wait_for_event_report(Clusters.BasicInformation.Events.ShutDown, timeout_sec=30)
        logger.info(f"Event report for ShutDown : {shutdown_event}")

        basicinformation_handler.flush_events()
        await basicinformation_handler.cancel()

        # Just wait for the device to StartUp after ShutDown
        await asyncio.sleep(5)
        # Verify software version after StartUp

        update_state_after = await self.read_single_attribute_check_success(
            Clusters.OtaSoftwareUpdateRequestor, Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState, controller, requestor_node_id)
        logger.info("UpdateState after update" + str(update_state_after))
        # Just want to make sure we did change the update state meaning we applied the update
        asserts.assert_not_equal(update_state_prev, update_state_after, "Update state did not change")
        asserts.assert_equal(update_state_after, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle,
                             "UpdateState is not Idle")

        await self.verify_version_applied_basic_information(
            controller=controller, node_id=requestor_node_id, target_version=expected_software_version)

        self._terminate_provider_process()
        controller.ExpireSessions(nodeid=provider_data['node_id'])

        self.step(3)
        expected_software_version = 4
        delayed_apply_action_time = 60
        self.launch_provider_app(extra_args=['--applyUpdateAction', 'awaitNextAction',
                                             '--delayedApplyActionTimeSec', str(delayed_apply_action_time)], version=expected_software_version)
        await controller.CommissionOnNetwork(
            nodeId=provider_data['node_id'],
            setupPinCode=provider_data['setup_pincode'],
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_data['discriminator']
        )

        # Get current version
        current_sw_version = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.SoftwareVersion,
            node_id=requestor_node_id)

        event_state_transition = EventSubscriptionHandler(expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
                                                          expected_event_id=Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition.event_id)
        await event_state_transition.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=6000)

        await self.current_provider_app_proc.create_acl_entry(dev_ctrl=controller, provider_node_id=provider_data['node_id'], requestor_node_id=requestor_node_id)
        await self.write_ota_providers(controller=controller, provider_node_id=provider_data['node_id'], endpoint=0)
        await self.announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)

        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=60)
        logger.info(f"Event report for Querying : {event_report}")
        asserts.assert_equal(event_report.newState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying)

        update_state_prev = await self.read_single_attribute_check_success(
            Clusters.OtaSoftwareUpdateRequestor, Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState, controller, requestor_node_id)
        logger.info("UpdateState before update " + str(update_state_prev))

        # Applying this will trigger the "delayedActionTime: 180 seconds" in the logs
        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=10)
        logger.info(f"Event report for Downloading : {event_report}")
        asserts.assert_equal(event_report.newState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading)

        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=60*5)
        logger.info(f"Event report for Applying : {event_report}")
        asserts.assert_equal(event_report.newState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying)

        # As the action is awaitNextAction State should be DelayedOnApply
        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=30)
        logger.info(f"Event report for DelayedOnApply : {event_report}")
        asserts.assert_equal(event_report.newState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply)

        event_state_transition.flush_events()
        await event_state_transition.cancel()

        # Wait for 119 seconds 1 minute and 59 seconds and check the version in BasicInformationCluster
        # In the last second verify no update has taken place
        logger.info("Waitiing 1 minute a 59 seconds (Almost 2 minutes defined in the spec)")
        await asyncio.sleep(119)
        await self.verify_version_applied_basic_information(controller=controller, node_id=requestor_node_id, target_version=current_sw_version)

        # Should be the same version as the start before annouce
        self._terminate_provider_process()
        controller.ExpireSessions(nodeid=provider_data['node_id'])

        self.step(4)
        expected_software_version = 5
        delayed_apply_action_time = 180
        self.launch_provider_app(extra_args=['--applyUpdateAction', 'awaitNextAction',
                                             '--delayedApplyActionTimeSec', str(delayed_apply_action_time)], version=expected_software_version)
        await controller.CommissionOnNetwork(
            nodeId=provider_data['node_id'],
            setupPinCode=provider_data['setup_pincode'],
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_data['discriminator']
        )

        event_state_transition = EventSubscriptionHandler(expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
                                                          expected_event_id=Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition.event_id)
        await event_state_transition.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=6000)

        await self.current_provider_app_proc.create_acl_entry(dev_ctrl=controller, provider_node_id=provider_data['node_id'], requestor_node_id=requestor_node_id)
        await self.write_ota_providers(controller=controller, provider_node_id=provider_data['node_id'], endpoint=0)
        await self.announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)

        logger.info("Waiting for Attribute UpdateState events to complete")
        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=60)
        logger.info(f"Event report for Querying : {event_report}")
        asserts.assert_equal(event_report.newState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying)

        # Applying this will trigger the "delayedActionTime: 180 seconds" in the logs
        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=10)
        logger.info(f"Event report for Downloading : {event_report}")
        asserts.assert_equal(event_report.newState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading)

        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=60*5)
        logger.info(f"Event report for Applying : {event_report}")
        asserts.assert_equal(event_report.newState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying)

        # As the action is awaitNextAction State should be DelayedOnApply
        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=30)
        logger.info(f"Event report for DelayedOnApply : {event_report}")
        asserts.assert_equal(event_report.newState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnApply)

        logger.info(f"Expect {(delayed_apply_action_time/60)} minutes without events from StateTransition")
        event_state_transition.wait_for_event_expect_no_report(delayed_apply_action_time-1)
        logger.info(f"Waited {(delayed_apply_action_time/60)} minutes without events")

        # Inmediatly should change to Applying
        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=3)
        logger.info(f"Event report for Applying : {event_report}")
        asserts.assert_equal(event_report.newState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying)

        event_state_transition.flush_events()
        await event_state_transition.cancel()

        # Wait device to shutdown and startup and then verify the SW Version
        await asyncio.sleep(5)
        await self.verify_version_applied_basic_information(controller=controller, node_id=requestor_node_id, target_version=expected_software_version)

        self.step(5)
        expected_software_version = 6
        self.launch_provider_app(extra_args=['--applyUpdateAction', 'discontinue'], version=expected_software_version)
        await controller.CommissionOnNetwork(
            nodeId=provider_data['node_id'],
            setupPinCode=provider_data['setup_pincode'],
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_data['discriminator']
        )

        current_sw_version = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.SoftwareVersion,
            node_id=requestor_node_id)
        event_state_transition = EventSubscriptionHandler(expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
                                                          expected_event_id=Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition.event_id)
        await event_state_transition.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=6000)
        download_event_handler = EventSubscriptionHandler(expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
                                                          expected_event_id=Clusters.OtaSoftwareUpdateRequestor.Events.DownloadError.event_id)
        await download_event_handler.start(controller, requestor_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=6000)

        await self.current_provider_app_proc.create_acl_entry(dev_ctrl=controller, provider_node_id=provider_data['node_id'], requestor_node_id=requestor_node_id)
        await self.write_ota_providers(controller=controller, provider_node_id=provider_data['node_id'], endpoint=0)
        await self.announce_ota_provider(controller, provider_data['node_id'], requestor_node_id)

        # Querying
        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=60)
        logger.info(f"Event report for Querying : {event_report}")
        asserts.assert_equal(event_report.newState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying)

        # Downloading
        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=10)
        logger.info(f"Event report for Downloading : {event_report}")
        asserts.assert_equal(event_report.newState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading)

        # Wait some time to check if is downloading
        await asyncio.sleep(5)
        # Verify the default download path and the file size
        # Read file for /tmp/test.bin should exists and greater than 0
        ota_file_data = self.get_downloaded_ota_image_info()
        logger.info(f"Downloaded ota image data {str(ota_file_data)}")
        asserts.assert_equal(True, ota_file_data['exists'], f"File is not bein downloaded  at {ota_file_data['path']}")
        asserts.assert_greater(ota_file_data['size'], 0, f"Downloaded file is still at 0")

        # Applying
        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=60*6)
        logger.info(f"Event report for Downloading : {event_report}")
        asserts.assert_equal(event_report.newState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying)

        # Discontinue Change the Status to Idle
        event_report = event_state_transition.wait_for_event_report(
            Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, timeout_sec=30)
        logger.info(f"Event report for Idle : {event_report}")
        asserts.assert_equal(event_report.newState, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle)

        event_state_transition.flush_events()
        await event_state_transition.cancel()

        # Make sure attr is Idle
        update_state = await self.read_single_attribute_check_success(
            Clusters.OtaSoftwareUpdateRequestor, Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState, controller, requestor_node_id, 0)
        asserts.assert_equal(update_state, Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle,
                             "Update state is not idle")
        ota_file_data = self.get_downloaded_ota_image_info()
        logger.info(f"Downloaded ota image data {str(ota_file_data)}")
        asserts.assert_equal(ota_file_data['exists'], False, f"Downloaded file is still present {ota_file_data['path']}")
        asserts.assert_equal(ota_file_data['size'], 0, f"File size is greater than 0")
        update_state_progress = await self.read_single_attribute_check_success(
            Clusters.OtaSoftwareUpdateRequestor, Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress, controller, requestor_node_id, 0)
        asserts.assert_equal(update_state_progress, NullValue, "Progress is not Null")
        logger.info(f"Progress is {update_state_progress}")

        # Verify version is the same as when it  started
        await self.verify_version_applied_basic_information(controller, requestor_node_id, current_sw_version)


if __name__ == "__main__":
    default_matter_test_main()

# if __name__ == "__main__":
#     # Ota image
#     version = 2
#     ota_image = ""
#     if environ.get(f"SU_OTA_REQUESTOR_V{version}") is not None:
#         ota_image = environ.get(f"SU_OTA_REQUESTOR_V{version}")
#     else:
#         ota_image = f"{getcwd()}/chip-ota-requestor-app_v{version}.min.ota"

#     ota_image_path = OtaImagePath(path=ota_image)
#     ts = '1758662674'
#     provider_log = f'/tmp/provider_{ts}.log'
#     logger.info(f"WRITING LOGS AT : {provider_log}")
#     proc = OTAProviderSubprocess(
#         'ota_app',
#         storage_dir='/tmp',
#         port=5541,
#         discriminator=321,
#         passcode=2321,
#         ota_source=ota_image_path,
#         extra_args=[],
#         log_file=provider_log)
#     lines = proc.read_from_logs(
#         r"\[OTA-PROVIDER\]\[[0-9]+\.[0-9]+\]\s\[[0-9]+\:[0-9]+\:chip\]\s\[ZCL\].*ApplyUpdateRequest", regex=True)
#     print(f"LINES 1 {lines}")
#     lines = proc.read_from_logs('Provider received ApplyUpdateRequest', regex=False)
#     print(f"LINES 2 {lines}")
