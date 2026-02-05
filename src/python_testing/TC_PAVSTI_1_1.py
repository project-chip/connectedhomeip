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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/pavsti_1_1_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --string-arg th_server_app_path:${PUSH_AV_SERVER}
#       --string-arg host_ip:127.0.0.1
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --app-pipe /tmp/pavsti_1_1_fifo
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_AVSMTestBase import AVSMTestBase
from TC_PAVSTI_Utils import PAVSTIUtils, PushAvServerProcess

import matter.clusters as Clusters
from matter.clusters import Globals
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_PAVSTI_1_1(MatterBaseTest, AVSMTestBase, PAVSTIUtils):
    def desc_TC_PAVSTI_1_1(self) -> str:
        return "[TC-PAVSTI-1.1] Verify transmission when trigger type is Manual."

    def pics_TC_PAVSTI_1_1(self):
        return ["PAVST.S"]

    @async_test_body
    async def setup_class(self):
        th_server_app = self.user_params.get("th_server_app_path", None)
        self.host_ip = self.user_params.get("host_ip", None)
        if self.host_ip is None:
            self.host_ip = self.get_private_ip()
        self.server = PushAvServerProcess(server_path=th_server_app, server_ip=self.host_ip)
        self.server.start(
            expected_output="Running on https://0.0.0.0:1234",
            timeout=30,
        )
        super().setup_class()

    def teardown_class(self):
        if self.server is not None:
            self.server.terminate()
        super().teardown_class()

    def steps_TC_PAVSTI_1_1(self) -> list[TestStep]:
        return [
            TestStep(
                "precondition", "Commissioning, already done", is_commissioning=True
            ),
            TestStep(
                1,
                "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                "Verify the number of PushAV Connections in the list is 0. If not 0, issue DeAllocatePushAVTransport with `ConnectionID to remove any connections.",
            ),
            TestStep(
                2,
                "TH Reads SupportedFormats attribute from PushAV Stream Transport Cluster on DUT",
                "Verify that list is not empty. Store value as aSupportedFormats.",
            ),
            TestStep(
                3,
                "TH Reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify that list is not empty.",
            ),
            TestStep(
                4,
                "TH Reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify that list is not empty.",
            ),
            TestStep(
                5,
                "TH sends the AllocatePushTransport command with valid parameters and TriggerType = Command",
                "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID, TransportOptions, and TransportStatus in the TransportConfigurationStruct.",
            ),
            TestStep(
                6,
                "TH establishes a subscription to all of the Events from the Cluster",
            ),
            TestStep(
                7,
                "TH sends the SetTransportStatus command with ConnectionID = aConnectionID and TransportStatus = Active.",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                8,
                "Turn ON HardPrivacyModeOn through physical switch.",
                "HardPrivacyModeOn in DUT is set to True",
            ),
            TestStep(
                9,
                "If the DUT supports HardPrivacyModeOn attribute, TH sends the ManuallyTriggerTransport command with ConnectionID = aConnectionID.",
                "DUT responds with INVALID_IN_STATE status code.",
            ),
            TestStep(
                10,
                "Turn OFF HardPrivacyModeOn through physical switch.",
                "HardPrivacyModeOn in DUT is set to False.",
            ),
            TestStep(
                11,
                "TH writes SoftRecordingPrivacyModeEnabled as True",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                12,
                "TH sends the ManuallyTriggerTransport command with ConnectionID = aConnectionID",
                "DUT responds with INVALID_IN_STATE status code.",
            ),
            TestStep(
                13,
                "TH writes SoftRecordingPrivacyModeEnabled as False",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                14,
                "TH sends ModifyPushTransport with StreamUsage = LiveView",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                15,
                "TH writes SoftLivestreamPrivacyModeEnabled as True",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                16,
                "TH sends the ManuallyTriggerTransport command with ConnectionID = aConnectionID",
                "DUT responds with INVALID_IN_STATE status code.",
            ),
            TestStep(
                17,
                "TH writes SoftLivestreamPrivacyModeEnabled as False",
                "DUT responds with SUCCESS status code and begins transmission.",
            ),
            TestStep(
                18,
                "TH sends the ManuallyTriggerTransport command with ConnectionID = aConnectionID.",
                "DUT responds with SUCCESS status code and begins transmission.",
            ),
            TestStep(
                19,
                "TH verifies that a PushTransportBegin Event was received.",
                "TH validates that the connectionID = aConnectionID, triggerType = Command, and activationReason = UserInitiated.",
            ),
            TestStep(
                20,
                "View the video stream in TH UI",
                "Verify the transmitted video stream is of CMAF Format.",
            ),
            TestStep(
                21,
                "TH sends the SetTransportStatus command with ConnectionID = aConnectionID and TransportStatus = Inactive.",
                "DUT responds with SUCCESS status code",
            ),
            TestStep(
                22,
                "View the video stream in TH UI",
                "Verify the transmission of video stream has stopped.",
            ),
            TestStep(
                23,
                "TH verifies that a PushTransportEnd Event was received.",
                "TH validates that the connectionID = aConnectionID.",
            ),
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_PAVSTI_1_1(self):
        PICS_PRIVACY = "AVSM.S.F03"
        endpoint = self.get_endpoint()
        pushavCluster = Clusters.PushAvStreamTransport
        avsmCluster = Clusters.CameraAvStreamManagement
        pushavAttr = Clusters.PushAvStreamTransport.Attributes
        avsmAttr = Clusters.CameraAvStreamManagement.Attributes

        self.step("precondition")
        # Commission DUT - already done
        await self.precondition_one_allocated_video_stream(streamUsage=Globals.Enums.StreamUsageEnum.kRecording)
        await self.precondition_one_allocated_audio_stream(streamUsage=Globals.Enums.StreamUsageEnum.kRecording)
        tlsEndpointId, _ = await self.precondition_provision_tls_endpoint(endpoint=endpoint, server=self.server, host_ip=self.host_ip)
        uploadStreamId = self.server.create_stream()

        self.step(1)
        currentConnections = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=pushavCluster,
            attribute=pushavAttr.CurrentConnections,
        )
        log.info(f"Rx'd CurrentConnections: {currentConnections}")
        if len(currentConnections) > 0:
            for connectionId in currentConnections:
                await self.send_single_cmd(
                    cmd=pushavCluster.Commands.DeallocatePushTransport(
                        connectionID=connectionId
                    ),
                    endpoint=endpoint,
                )

        self.step(2)
        supportedFormats = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=pushavCluster,
            attribute=pushavAttr.SupportedFormats,
        )
        log.info(f"Rx'd SupportedFormats: {supportedFormats}")
        asserts.assert_greater_equal(
            len(supportedFormats), 1, "SupportedFormats must not be empty"
        )
        for format in supportedFormats:
            validContainerformat = (
                format.containerFormat == pushavCluster.Enums.ContainerFormatEnum.kCmaf
            )
            isValidIngestMethod = (
                format.ingestMethod == pushavCluster.Enums.IngestMethodsEnum.kCMAFIngest
            )
            asserts.assert_true(
                (validContainerformat and isValidIngestMethod),
                "(ContainerFormat and IngestMethod) must be defined values",
            )

        self.step(3)
        allocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=avsmCluster,
            attribute=avsmAttr.AllocatedVideoStreams,
        )
        log.info(f"Rx'd AllocatedVideoStreams: {allocatedVideoStreams}")
        asserts.assert_true(
            len(allocatedVideoStreams) != 0, "AllocatedVideoStreams must not be empty"
        )
        allocatedVideoStream = allocatedVideoStreams[0]
        videoStreamId = allocatedVideoStream.videoStreamID

        self.step(4)
        allocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=avsmCluster,
            attribute=avsmAttr.AllocatedAudioStreams,
        )
        log.info(f"Rx'd AllocatedAudioStreams: {allocatedAudioStreams}")
        asserts.assert_true(
            len(allocatedAudioStreams) != 0, "AllocatedAudioStreams must not be empty"
        )
        allocatedAudioStream = allocatedAudioStreams[0]
        audioStreamId = allocatedAudioStream.audioStreamID

        self.step(5)
        trackName = "media"
        self.server.update_track_name(uploadStreamId, trackName)
        transportOptions = {
            "streamUsage": Globals.Enums.StreamUsageEnum.kRecording,
            "videoStreamID": videoStreamId,
            "audioStreamID": audioStreamId,
            "TLSEndpointID": tlsEndpointId,
            "url": f"https://{self.host_ip}:1234/streams/{uploadStreamId}/",
            "triggerOptions": {"triggerType": pushavCluster.Enums.TransportTriggerTypeEnum.kCommand, "maxPreRollLen": 10000},
            "ingestMethod": pushavCluster.Enums.IngestMethodsEnum.kCMAFIngest,
            "containerFormat": pushavCluster.Enums.ContainerFormatEnum.kCmaf,
            "containerOptions": {
                "containerType": pushavCluster.Enums.ContainerFormatEnum.kCmaf,
                "CMAFContainerOptions": {"CMAFInterface": 0, "segmentDuration": 4000, "chunkDuration": 2000, "sessionGroup": 1, "trackName": trackName},
            },
        }
        allocatePushTransportResponse = await self.send_single_cmd(
            cmd=pushavCluster.Commands.AllocatePushTransport(
                transportOptions=transportOptions
            ),
            endpoint=endpoint,
        )
        log.info(
            f"Rx'd allocatePushTransportResponse = {allocatePushTransportResponse}"
        )

        self.step(6)
        event_callback = EventSubscriptionHandler(expected_cluster=pushavCluster)
        await event_callback.start(self.default_controller,
                                   self.dut_node_id,
                                   self.get_endpoint())

        self.step(7)
        aConnectionID = (
            allocatePushTransportResponse.transportConfiguration.connectionID
        )
        await self.send_single_cmd(
            cmd=pushavCluster.Commands.SetTransportStatus(
                connectionID=aConnectionID, transportStatus=pushavCluster.Enums.TransportStatusEnum.kActive),
            endpoint=endpoint,
        )

        if await self.attribute_guard(endpoint=endpoint, attribute=avsmAttr.HardPrivacyModeOn):
            self.step(8)
            # For CI: Use app pipe to simulate physical privacy switch being turned on
            # For manual testing: User should physically turn on the privacy switch
            if self.is_pics_sdk_ci_only:
                self.write_to_app_pipe({"Name": "SetHardPrivacyModeOn", "Value": True})
            else:
                user_response = self.wait_for_user_input(
                    prompt_msg="Please turn ON the physical privacy switch on the device, then press Enter to continue..."
                )

            # Verify the attribute reflects the privacy switch state
            hard_privacy_mode = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=Clusters.CameraAvStreamManagement,
                attribute=Clusters.CameraAvStreamManagement.Attributes.HardPrivacyModeOn
            )
            asserts.assert_true(hard_privacy_mode, "HardPrivacyModeOn should be True when privacy switch is on")

            self.step(9)
            try:
                await self.send_single_cmd(
                    cmd=pushavCluster.Commands.ManuallyTriggerTransport(
                        connectionID=aConnectionID,
                        activationReason=pushavCluster.Enums.TriggerActivationReasonEnum.kUserInitiated,
                        timeControl={
                            "initialDuration": 30,
                            "augmentationDuration": 10,
                            "maxDuration": 120,
                            "blindDuration": 1,
                        }
                    ),
                    endpoint=endpoint,
                )
                asserts.fail(
                    "Unexpected success when expecting INVALID_IN_STATE due to HardPrivacyModeOn set to True"
                )
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.InvalidInState,
                    "Unexpected error returned when expecting INVALID_IN_STATE due to HardPrivacyModeOn set to True",
                )

            self.step(10)
            # For CI: Use app pipe to simulate physical privacy switch being turned off
            # For manual testing: User should physically turn off the privacy switch
            if self.is_pics_sdk_ci_only:
                self.write_to_app_pipe({"Name": "SetHardPrivacyModeOn", "Value": False})
            else:
                user_response = self.wait_for_user_input(
                    prompt_msg="Please turn OFF the physical privacy switch on the device, then press Enter to continue..."
                )
        else:
            self.skip_step(8)
            self.skip_step(9)
            self.skip_step(10)

        if self.pics_guard(self.check_pics(PICS_PRIVACY)):
            self.step(11)
            result = await self.write_single_attribute(
                avsmAttr.SoftRecordingPrivacyModeEnabled(True), endpoint_id=endpoint
            )
            asserts.assert_equal(
                result,
                Status.Success,
                "Error when trying to write SoftRecordingPrivacyModeEnabled",
            )
            self.step(12)
            try:
                await self.send_single_cmd(
                    cmd=pushavCluster.Commands.ManuallyTriggerTransport(
                        connectionID=aConnectionID,
                        activationReason=pushavCluster.Enums.TriggerActivationReasonEnum.kUserInitiated,
                        timeControl={
                            "initialDuration": 30,
                            "augmentationDuration": 10,
                            "maxDuration": 120,
                            "blindDuration": 1,
                        }
                    ),
                    endpoint=endpoint,
                )
                asserts.fail(
                    "Unexpected success when expecting INVALID_IN_STATE due to SoftRecordingPrivacyModeEnabled being set to True"
                )
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.InvalidInState,
                    "Unexpected error returned when expecting INVALID_IN_STATE due to SoftRecordingPrivacyModeEnabled being set to True",
                )
            self.step(13)
            result = await self.write_single_attribute(
                avsmAttr.SoftRecordingPrivacyModeEnabled(False), endpoint_id=endpoint
            )
        else:
            self.skip_step(11)
            self.skip_step(12)
            self.skip_step(13)

        # Modify transport to set streamUsage to Livestream and validate privacy mode
        if self.pics_guard(self.check_pics(PICS_PRIVACY)):
            self.step(14)
            transportOptions['streamUsage'] = Globals.Enums.StreamUsageEnum.kLiveView
            await self.send_single_cmd(
                cmd=pushavCluster.Commands.ModifyPushTransport(
                    connectionID=aConnectionID,
                    transportOptions=transportOptions
                ),
                endpoint=endpoint,
            )
            self.step(15)
            result = await self.write_single_attribute(
                avsmAttr.SoftLivestreamPrivacyModeEnabled(True), endpoint_id=endpoint
            )
            asserts.assert_equal(
                result,
                Status.Success,
                "Error when trying to write SoftLivestreamPrivacyModeEnabled",
            )
            self.step(16)
            try:
                await self.send_single_cmd(
                    cmd=pushavCluster.Commands.ManuallyTriggerTransport(
                        connectionID=aConnectionID,
                        activationReason=pushavCluster.Enums.TriggerActivationReasonEnum.kUserInitiated,
                        timeControl={
                            "initialDuration": 30,
                            "augmentationDuration": 10,
                            "maxDuration": 120,
                            "blindDuration": 1,
                        }
                    ),
                    endpoint=endpoint,
                )
                asserts.fail(
                    "Unexpected success when expecting INVALID_IN_STATE due to SoftLivestreamPrivacyModeEnabled being set to True"
                )
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.InvalidInState,
                    "Unexpected error returned when expecting INVALID_IN_STATE due to SoftLivestreamPrivacyModeEnabled being set to True",
                )
            self.step(17)
            result = await self.write_single_attribute(
                avsmAttr.SoftLivestreamPrivacyModeEnabled(False), endpoint_id=endpoint
            )
        else:
            self.skip_step(14)
            self.skip_step(15)
            self.skip_step(16)
            self.skip_step(17)

        self.step(18)
        await self.send_single_cmd(
            cmd=pushavCluster.Commands.ManuallyTriggerTransport(
                connectionID=aConnectionID,
                activationReason=pushavCluster.Enums.TriggerActivationReasonEnum.kUserInitiated,
                timeControl={
                    "initialDuration": 30,
                    "augmentationDuration": 10,
                    "maxDuration": 120,
                    "blindDuration": 1,
                }
            ),
            endpoint=endpoint,
        )

        self.step(19)
        # Verify event received
        event_data = event_callback.wait_for_event_report(pushavCluster.Events.PushTransportBegin, timeout_sec=5)
        log.info(f"Event data {event_data}")
        asserts.assert_equal(event_data.connectionID, aConnectionID, "Unexpected value for ConnectionID returned")
        asserts.assert_equal(event_data.triggerType, pushavCluster.Enums.TransportTriggerTypeEnum.kCommand,
                             "Unexpected value for TriggerType returned")
        asserts.assert_equal(event_data.activationReason, pushavCluster.Enums.TriggerActivationReasonEnum.kUserInitiated,
                             "Unexpected value for ActivationReason returned")

        self.step(20)
        if not self.check_pics("PICS_SDK_CI_ONLY"):
            skipped = self.user_verify_push_av_stream(
                prompt_msg="Verify the video stream is being transmitted by playing the live video and viewing the uploaded contents."
            )
            if skipped:
                # For when running in CLI
                prompt = ("Verify the video segments are being received by the server by viewing the logs with [PUSH_AV_SERVER] tag.\n"
                          "The uploaded content must be accepted by the server without any errors.\n"
                          "Enter 'y' to confirm.")
                user_response = self.wait_for_user_input(
                    prompt_msg=prompt,
                    prompt_msg_placeholder="y",
                    default_value="y",
                )
                asserts.assert_equal(user_response.lower(), "y")

        self.step(21)
        await self.send_single_cmd(
            cmd=pushavCluster.Commands.SetTransportStatus(
                connectionID=aConnectionID, transportStatus=pushavCluster.Enums.TransportStatusEnum.kInactive),
            endpoint=endpoint,
        )

        self.step(22)
        if not self.check_pics("PICS_SDK_CI_ONLY"):
            prompt = ("Verify the video stream uploaded can be played. Verify that DUT has stopped uploading by viewing the uploaded content and ensure no new files are received.\n"
                      "Click on the 'Refresh Streams' button to view the latest uploaded contents"
                      "The uploaded segment's extended path must conform to the Matter's extended path format")
            skipped = self.user_verify_push_av_stream(
                prompt_msg=prompt
            )
            if skipped:
                # For when running in CLI
                prompt = ("Verify that DUT has stopped transmitting content by viewing the server logs with [PUSH_AV_SERVER] tag."
                          "No new segments should be received."
                          "Enter 'y' to confirm.")
                user_response = self.wait_for_user_input(
                    prompt_msg=prompt,
                    prompt_msg_placeholder="y",
                    default_value="y",
                )
                asserts.assert_equal(user_response.lower(), "y")

        self.step(23)
        # Verify event received
        event_data = event_callback.wait_for_event_report(pushavCluster.Events.PushTransportEnd, timeout_sec=5)
        log.info(f"Event data {event_data}")
        asserts.assert_equal(event_data.connectionID, aConnectionID, "Unexpected value for ConnectionID returned")


if __name__ == "__main__":
    default_matter_test_main()
