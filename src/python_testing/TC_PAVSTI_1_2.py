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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_AVSMTestBase import AVSMTestBase

import matter.clusters as Clusters
from matter.clusters import Globals
from matter.interaction_model import InteractionModelError, Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_PAVSTI_1_2(MatterBaseTest, AVSMTestBase):
    def desc_TC_PAVSTI_1_2(self) -> str:
        return "[TC-PAVSTI-1.2] Verify transmission with trigger type as Continuous and ensure privacy settings are checked if supported."

    def pics_TC_PAVSTI_1_2(self):
        return ["PAVST.S"]

    def steps_TC_PAVSTI_1_2(self) -> list[TestStep]:
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
                "TH writes SoftRecordingPrivacyModeEnabled as True",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                6,
                "TH writes SoftLiveStreamPrivacyModeEnabled as True",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                7,
                "TH sends the AllocatePushTransport command with valid parameters and TriggerType = Continuous",
                "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID, TransportOptions, and TransportStatus in the TransportConfigurationStruct. Store ConnectionID as aConnectionID.",
            ),
            TestStep(
                8,
                "TH sends the SetTransportStatus command with ConnectionID = aConnectionID and TransportStatus = Active",
                "DUT responds with INVALID_IN_STATE status code.",
            ),
            TestStep(
                9,
                "TH writes SoftRecordingPrivacyModeEnabled as False",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                10,
                "TH writes SoftLiveStreamPrivacyModeEnabled as False",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                11,
                "TH sends the SetTransportStatus command with ConnectionID = aConnectionID and TransportStatus = Active.",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                12,
                "View the video stream in TH UI",
                "Verify the transmitted video stream is of CMAF Format.",
            ),
            TestStep(
                13,
                "TH sends the SetTransportStatus command with ConnectionID = aConnectionID and TransportStatus = Inactive.",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                14,
                "View the video stream in TH UI",
                "Verify the transmission of video stream has stopped.",
            ),
        ]

    @async_test_body
    async def test_TC_PAVSTI_1_2(self):
        endpoint = self.get_endpoint(default=1)
        pushavCluster = Clusters.PushAvStreamTransport
        avsmCluster = Clusters.CameraAvStreamManagement
        pushavAttr = Clusters.PushAvStreamTransport.Attributes
        avsmAttr = Clusters.CameraAvStreamManagement.Attributes

        self.step("precondition")
        # Commission DUT - already done
        await self.precondition_one_allocated_video_stream(streamUsage=Globals.Enums.StreamUsageEnum.kRecording)
        await self.precondition_one_allocated_audio_stream(streamUsage=Globals.Enums.StreamUsageEnum.kRecording)

        self.step(1)
        currentConnections = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=pushavCluster,
            attribute=pushavAttr.CurrentConnections
        )
        logger.info(f"Rx'd CurrentConnections: {currentConnections}")
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
        logger.info(f"Rx'd SupportedFormats: {supportedFormats}")
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
        logger.info(f"Rx'd AllocatedVideoStreams: {allocatedVideoStreams}")
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
        logger.info(f"Rx'd AllocatedAudioStreams: {allocatedAudioStreams}")
        asserts.assert_true(
            len(allocatedAudioStreams) != 0, "AllocatedAudioStreams must not be empty"
        )
        allocatedAudioStream = allocatedAudioStreams[0]
        audioStreamId = allocatedAudioStream.audioStreamID

        self.step(5)
        if self.pics_guard(self.check_pics("AVSM.S.F0003")):
            aFeatureMap = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=avsmCluster, attribute=avsmAttr.FeatureMap
            )
            privacySupport = (aFeatureMap & avsmCluster.Bitmaps.Feature.kPrivacy) > 0
            asserts.assert_true(privacySupport, "Privacy Feature is not supported.")

            result = await self.write_single_attribute(
                avsmAttr.SoftRecordingPrivacyModeEnabled(True), endpoint_id=endpoint
            )
            asserts.assert_equal(
                result,
                Status.Success,
                "Error when trying to write SoftRecordingPrivacyModeEnabled",
            )

        self.step(6)
        if self.pics_guard(self.check_pics("AVSM.S.F0003")):
            result = await self.write_single_attribute(
                avsmAttr.SoftLivestreamPrivacyModeEnabled(True), endpoint_id=endpoint
            )
            asserts.assert_equal(
                result,
                Status.Success,
                "Error when trying to write SoftLivestreamPrivacyModeEnabled",
            )

        self.step(7)
        allocatePushTransportResponse = await self.send_single_cmd(
            cmd=pushavCluster.Commands.AllocatePushTransport(
                transportOptions={
                    "streamUsage": Globals.Enums.StreamUsageEnum.kRecording,
                    "videoStreamID": videoStreamId,
                    "audioStreamID": audioStreamId,
                    "endpointID": 1,  # TODO: Revisit TLS arguments once TLSCM cluster is available.
                    "url": "https://localhost:1234/streams/1",
                    "triggerOptions": {"triggerType": pushavCluster.Enums.TransportTriggerTypeEnum.kContinuous},
                    "ingestMethod": pushavCluster.Enums.IngestMethodsEnum.kCMAFIngest,
                    "containerFormat": pushavCluster.Enums.ContainerFormatEnum.kCmaf,
                    "containerOptions": {
                        "containerType": pushavCluster.Enums.ContainerFormatEnum.kCmaf,
                        "CMAFContainerOptions": {"chunkDuration": 4},
                    },
                }
            ),
            endpoint=endpoint,
        )
        logger.info(
            f"Rx'd allocatePushTransportResponse = {allocatePushTransportResponse}"
        )
        aConnectionID = (
            allocatePushTransportResponse.transportConfiguration.connectionID
        )

        self.step(8)
        if self.pics_guard(self.check_pics("AVSM.S.F0003")):
            try:
                await self.send_single_cmd(
                    cmd=pushavCluster.Commands.SetTransportStatus(
                        connectionID=aConnectionID, transportStatus=pushavCluster.Enums.TransportStatusEnum.kActive
                    ),
                    endpoint=endpoint,
                )
                asserts.fail(
                    "Unexpected success when expecting INVALID_IN_STATE due to SoftPrivacy mode set to True"
                )
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.InvalidInState,
                    "Unexpected error returned when expecting INVALID_IN_STATE due to SoftPrivacy mode set to True",
                )

        self.step(9)
        if self.pics_guard(self.check_pics("AVSM.S.F0003")):
            result = await self.write_single_attribute(
                avsmAttr.SoftRecordingPrivacyModeEnabled(False), endpoint_id=endpoint
            )
            asserts.assert_equal(
                result,
                Status.Success,
                "Error when trying to write SoftRecordingPrivacyModeEnabled",
            )

        self.step(10)
        if self.pics_guard(self.check_pics("AVSM.S.F0003")):
            result = await self.write_single_attribute(
                avsmAttr.SoftLivestreamPrivacyModeEnabled(False), endpoint_id=endpoint
            )
            asserts.assert_equal(
                result,
                Status.Success,
                "Error when trying to write SoftLivestreamPrivacyModeEnabled",
            )

        self.step(11)
        await self.send_single_cmd(
            cmd=pushavCluster.Commands.SetTransportStatus(
                connectionID=aConnectionID, transportStatus=pushavCluster.Enums.TransportStatusEnum.kActive),
            endpoint=endpoint,
        )

        self.step(12)
        # TODO: Add a step to allow user to verify this through TH UI.
        if not self.check_pics("PICS_SDK_CI_ONLY"):
            user_response = self.wait_for_user_input(
                prompt_msg="Verify the video stream is being transmitted and is of CMAF format. Enter 'y' to confirm.",
                prompt_msg_placeholder="y",
                default_value="y",
            )
            asserts.assert_equal(user_response.lower(), "y")

        self.step(13)
        await self.send_single_cmd(
            cmd=pushavCluster.Commands.SetTransportStatus(
                connectionID=aConnectionID, transportStatus=pushavCluster.Enums.TransportStatusEnum.kInactive),
            endpoint=endpoint,
        )

        self.step(14)
        # TODO: Add a step to allow user to verify this through TH UI.
        if not self.check_pics("PICS_SDK_CI_ONLY"):
            user_response = self.wait_for_user_input(
                prompt_msg="Verify the transmission of video stream has stopped. Enter 'y' to confirm.",
                prompt_msg_placeholder="y",
                default_value="y",
            )
            asserts.assert_equal(user_response.lower(), "y")


if __name__ == "__main__":
    default_matter_test_main()
