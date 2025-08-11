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
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_PAVST_2_2(MatterBaseTest):
    def desc_TC_PAVST_2_2(self) -> str:
        return "[TC-PAVST-2.2] Attributes with Server as DUT"

    def pics_TC_PAVST_2_2(self):
        return ["PAVST.S"]

    def steps_TC_PAVST_2_2(self) -> list[TestStep]:
        return [
            TestStep(1, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                     "Verify the number of PushAV Connections in the list is 0. If not 0, issue DeAllocatePushAVTransport with `ConnectionID to remove any connections."),
            TestStep(2, "TH Reads SupportedFormats attribute from PushAV Stream Transport Cluster on DUT",
                     "Store value as aSupportedFormats."),
            TestStep(3, "TH Reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                     "Verify that list is not empty. Store value as aAllocatedVideoStreams."),
            TestStep(4, "TH Reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT",
                     "Verify that list is not empty. Store value as aAllocatedAudioStreams."),
            TestStep(5, "TH sends the AllocatePushTransport command with valid parameters",
                     "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID, TransportOptions, and TransportStatus in the TransportConfigurationStruct."),
            TestStep(6, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT over a MRP-based secure session",
                     "Verify the number of PushAV Connections is 1. Verify that the TransportOptions field in the TransportConfiguration struct is absent."),
            TestStep(7, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT over a large payload session",
                     "Verify the number of PushAV Connections is 1. Verify that the TransportOptions field in the TransportConfiguration struct is present."),
        ]

    @async_test_body
    async def test_TC_PAVST_2_2(self):
        endpoint = self.get_endpoint(default=1)
        pvcluster = Clusters.PushAvStreamTransport
        avcluster = Clusters.Objects.CameraAvStreamManagement
        pvattr = Clusters.PushAvStreamTransport.Attributes
        avattr = Clusters.Objects.CameraAvStreamManagement.Attributes

        self.step(1)
        if self.pics_guard(self.check_pics("PAVST.S")):
            transport_configs = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=pvcluster, attribute=pvattr.CurrentConnections
            )
            for config in transport_configs:
                if config.ConnectionID != 0:
                    try:
                        await self.send_single_cmd(cmd=pvcluster.Commands.DeallocatePushTransport(ConnectionID=config.ConnectionID),
                                                   endpoint=endpoint)
                    except InteractionModelError as e:
                        asserts.assert_true(e.status == Status.Success, "Unexpected error returned")

        self.step(2)
        if self.pics_guard(self.check_pics("PAVST.S")):
            supported_formats = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=pvcluster, attribute=pvattr.SupportedFormats
            )
            asserts.assert_greater_equal(len(supported_formats, 1), "SupportedFormats must not be empty!")
            for format in supported_formats:
                validContainerformat = format.ContainerFormat == pvcluster.ContainerFormatEnum.kCmaf
                isValidIngestMethod = format.IngestMethod == pvcluster.IngestMethodEnum.kCMAFIngest
                asserts.assert_true((validContainerformat & isValidIngestMethod),
                                    "(ContainerFormat & IngestMethod) must be defined values!")

        self.step(3)
        if self.pics_guard(self.check_pics("AVSM.S")):
            await self.send_single_cmd(cmd=Clusters.CameraAvStreamManagement.Commands.VideoStreamAllocate(
                streamUsage=0,
                videoCodec=0,
                minFrameRate=30,
                maxFrameRate=120,
                minResolution=Clusters.CameraAvStreamManagement.Structs.VideoResolutionStruct(width=400, height=300),
                maxResolution=Clusters.CameraAvStreamManagement.Structs.VideoResolutionStruct(width=1920, height=1080),
                minBitRate=20000,
                maxBitRate=150000,
                minFragmentLen=2000,
                maxFragmentLen=8000
            ),
                endpoint=endpoint)

            aAllocatedVideoStreams = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=avcluster, attribute=avattr.AllocatedVideoStreams
            )
            asserts.assert_greater_equal(len(aAllocatedVideoStreams), 1, "AllocatedVideoStreams must not be empty")

        self.step(4)
        if self.pics_guard(self.check_pics("AVSM.S")):
            await self.send_single_cmd(cmd=Clusters.CameraAvStreamManagement.Commands.AudioStreamAllocate(
                streamUsage=0,
                audioCodec=0,
                channelCount=2,
                sampleRate=48000,
                bitRate=96000,
                bitDepth=16
            ),
                endpoint=endpoint)

            aAllocatedAudioStreams = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=avcluster, attribute=avattr.AllocatedAudioStreams
            )
            asserts.assert_greater_equal(len(aAllocatedAudioStreams), 1, "AllocatedAudioStreams must not be empty")

        self.step(5)
        if self.pics_guard(self.check_pics("PAVST.S")):
            await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 2},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)

        self.step(6)
        if self.pics_guard(self.check_pics("PAVST.S")):
            current_connections = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=pvcluster, attribute=pvcluster.Attributes.CurrentConnections
            )
            asserts.assert_equal(len(current_connections), 1, "TransportConfigurations must be 1")
        self.step(7)
        if self.pics_guard(self.check_pics("PAVST.S")):
            current_connections = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=pvcluster, attribute=pvcluster.Attributes.CurrentConnections
            )
            asserts.assert_equal(len(current_connections), 1, "TransportConfigurations must be 1")


if __name__ == "__main__":
    default_matter_test_main()
