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

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import Globals
from matter.interaction_model import InteractionModelError, Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches

logger = logging.getLogger(__name__)


class TC_AVSM_2_7(MatterBaseTest):
    def desc_TC_AVSM_2_7(self) -> str:
        return "[TC-AVSM-2.7] Validate Video Stream Allocation functionality with Server as DUT"

    def pics_TC_AVSM_2_7(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_7(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on DUT", "Verify VDO is supported."),
            TestStep(
                2,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the number of allocated video streams in the list is 0.",
            ),
            TestStep(
                3,
                "TH reads StreamUsagePriorities attribute from CameraAVStreamManagement Cluster on DUT.",
                "Store this value in aStreamUsagePriorities.",
            ),
            TestStep(
                4,
                "TH reads RateDistortionTradeOffPoints attribute from CameraAVStreamManagement Cluster on DUT.",
                "Store this value in aRateDistortionTradeOffPoints.",
            ),
            TestStep(
                5,
                "TH reads MinViewportResolution attribute from CameraAVStreamManagement Cluster on DUT.",
                "Store this value in aMinViewportRes.",
            ),
            TestStep(
                6,
                "TH reads VideoSensorParams attribute from CameraAVStreamManagement Cluster on DUT.",
                "Store this value in aVideoSensorParams.",
            ),
            TestStep(
                7,
                "TH reads MaxEncodedPixelRate attribute from CameraAVStreamManagement Cluster on DUT.",
                "Store this value in aMaxEncodedPixelRate.",
            ),
            TestStep(
                8,
                "If the watermark feature is supported, set aWatermark to True, otherwise set this to Null.",
            ),
            TestStep(
                9,
                "If the OSD feature is supported, set aOSD to True, otherwise set this to Null.",
            ),
            TestStep(
                10,
                "TH sets StreamUsage from aStreamUsagePriorities. TH sets VideoCodec, MinResolution, MaxResolution, MinBitRate, MaxBitRate conforming with aRateDistortionTradeOffPoints. TH sets MinFrameRate, MaxFrameRate conforming with aVideoSensorParams. TH sets the KeyFrameInterval = 4000. TH sets WatermarkEnabled to aWatermark, TH also sets OSDEnabled to aOSD. TH sends the VideoStreamAllocate command with these arguments.",
                "DUT responds with VideoStreamAllocateResponse command with a valid VideoStreamID. Store this as myStreamID",
            ),
            TestStep(
                11,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the number of allocated video streams in the list is 1.",
            ),
            TestStep(
                12,
                "TH sends the VideoStreamAllocate command with the same arguments from step 10.",
                "DUT responds with VideoStreamAllocateResponse command with the same VideoStreamID as step 10.",
            ),
            TestStep(
                13,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the number of allocated video streams in the list is 1.",
            ),
            TestStep(
                14,
                "If the CameraAVSettingsUserLevelManagement cluster is present, with the DPTZ feature flag set the proceed to step 15, otherwise jump to step 17.",
            ),
            TestStep(
                15,
                "TH reads the `Viewport` attribute from CameraAVStreamManagement Cluster on DUT.",
                "Store this value in aViewport.",
            ),
            TestStep(
                16,
                "TH reads the `DPTZStreams` attribute from CameraAVSettingsUserLevelManagement Cluster on DUT. Verify there is an entry with `VideoStreamID` set to `myStreamID.",
                "Verify the `Viewport` for that entry is the same as `myViewport",
            ),
            TestStep(
                17,
                "TH sends the VideoStreamAllocate command with the same arguments from step 10 except with StreamUsage set to Internal",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                18,
                "TH sends the VideoStreamAllocate command with the same arguments from step 10 except StreamUsage set to a value not in aStreamUsagePriorities.",
                "DUT responds with a INVALID IN STATE status code.",
            ),
            TestStep(
                19,
                "TH sends the VideoStreamAllocate command with the same arguments from step 10 except MinFrameRate set to 0(outside of valid range).",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                20,
                "TH sends the VideoStreamAllocate command with the same arguments from step 10 except MinFrameRate > MaxFrameRate.",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                21,
                "TH sends the VideoStreamAllocate command with the same arguments from step 10 except MinBitRate set to 0(outside of valid range).",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                22,
                "TH sends the VideoStreamAllocate command with the same arguments from step 10 except MinBitRate > MaxBitRate.",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                23,
                "TH sends the VideoStreamAllocate command with the same arguments from step 10 except KeyFrameInterval > Max value",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                24,
                "TH sends the VideoStreamAllocate command with the same arguments from step 10 except VideoCodec is set to 10 (out of range).",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                25,
                "TH sends the VideoStreamAllocate command with the same arguments from step 10 except MaxFrameRate set to a value not in aVideoSensorParams.",
                "DUT responds with a DYNAMIC_CONSTRAINT_ERROR status code.",
            ),
        ]

    @run_if_endpoint_matches(
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kVideo)
    )
    async def test_TC_AVSM_2_7(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done

        self.step(1)
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        vdoSupport = aFeatureMap & cluster.Bitmaps.Feature.kVideo
        asserts.assert_equal(vdoSupport, cluster.Bitmaps.Feature.kVideo, "Video Feature is not supported.")

        self.step(2)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        logger.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 0, "The number of allocated video streams in the list is not 0")

        self.step(3)
        aStreamUsagePriorities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities
        )
        logger.info(f"Rx'd StreamUsagePriorities: {aStreamUsagePriorities}")

        self.step(4)
        aRateDistortionTradeOffPoints = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.RateDistortionTradeOffPoints
        )
        logger.info(f"Rx'd RateDistortionTradeOffPoints: {aRateDistortionTradeOffPoints}")

        self.step(5)
        aMinViewportRes = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.MinViewportResolution
        )
        logger.info(f"Rx'd MinViewportResolution: {aMinViewportRes}")

        self.step(6)
        aVideoSensorParams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.VideoSensorParams
        )
        logger.info(f"Rx'd VideoSensorParams: {aVideoSensorParams}")

        self.step(7)
        aMaxEncodedPixelRate = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.MaxEncodedPixelRate
        )
        logger.info(f"Rx'd MaxEncodedPixelRate: {aMaxEncodedPixelRate}")

        # Check for watermark and OSD features
        self.step(8)
        watermark = True if (aFeatureMap & cluster.Bitmaps.Feature.kWatermark) != 0 else None

        self.step(9)
        osd = True if (aFeatureMap & cluster.Bitmaps.Feature.kOnScreenDisplay) != 0 else None

        self.step(10)
        myStreamID = 0
        try:
            asserts.assert_greater(len(aStreamUsagePriorities), 0, "StreamUsagePriorities is empty")
            asserts.assert_greater(len(aRateDistortionTradeOffPoints), 0, "RateDistortionTradeOffPoints is empty")
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=30,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewportRes,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                keyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd,
            )
            videoStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            logger.info(f"Rx'd VideoStreamAllocateResponse: {videoStreamAllocateResponse}")
            asserts.assert_is_not_none(
                videoStreamAllocateResponse.videoStreamID, "VideoStreamAllocateResponse does not contain StreamID"
            )
            myStreamID = videoStreamAllocateResponse.videoStreamID
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(11)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        logger.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 1, "The number of allocated video streams in the list is not 1")

        self.step(12)
        try:
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=30,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewportRes,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                keyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd,
            )
            videoStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            logger.info(f"Rx'd VideoStreamAllocateResponse: {videoStreamAllocateResponse}")
            asserts.assert_equal(
                videoStreamAllocateResponse.videoStreamID, myStreamID, "VideoStreamAllocateResponse does not reuse the StreamID"
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(13)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        logger.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 1, "The number of allocated video streams in the list is not 1")

        # Viewport check
        descriptor = await self.default_controller.ReadAttribute(self.dut_node_id, [(endpoint, Clusters.Descriptor)])
        server_list = descriptor[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList]
        has_cameraavsettings = Clusters.CameraAvSettingsUserLevelManagement.id in server_list

        self.step(14)
        if has_cameraavsettings:
            aFeatureMap = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=Clusters.CameraAvSettingsUserLevelManagement,
                attribute=Clusters.CameraAvSettingsUserLevelManagement.Attributes.FeatureMap,
            )
            dptzSupport = (aFeatureMap & Clusters.CameraAvSettingsUserLevelManagement.Bitmaps.Feature.kDigitalPTZ) != 0

            if dptzSupport:
                self.step(15)
                aViewport = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.Viewport
                )

                self.step(16)
                aDptzStreams = await self.read_single_attribute_check_success(
                    endpoint=endpoint,
                    cluster=Clusters.CameraAvSettingsUserLevelManagement,
                    attribute=Clusters.CameraAvSettingsUserLevelManagement.Attributes.DPTZStreams,
                )

                dptzWritten = False
                for dptzStream in aDptzStreams:
                    if dptzStream.videoStreamID == myStreamID:
                        if dptzStream.viewport == aViewport:
                            dptzWritten = True

                asserts.assert_true(
                    dptzWritten, "DPTZStreams in CameraAvSettingsUserLevelManagement was not updated with the allocated stream id"
                )
            else:
                self.skip_step(15)
                self.skip_step(16)
        else:
            self.skip_step(15)
            self.skip_step(16)

        self.step(17)
        try:
            outOfConstraintStreamUsage = Globals.Enums.StreamUsageEnum.kInternal
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=outOfConstraintStreamUsage,
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=30,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewportRes,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                keyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            asserts.fail(
                "Unexpected success when expecting CONSTRAINT_ERROR due to StreamUsage set to Internal",
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected error returned when expecting CONSTRAINT_ERROR due to StreamUsage set to Internal",
            )
            pass

        self.step(18)
        try:
            notSupportedStreamUsage = next(
                (e for e in Globals.Enums.StreamUsageEnum if e not in aStreamUsagePriorities and e != Globals.Enums.StreamUsageEnum.kInternal),
                Globals.Enums.StreamUsageEnum.kUnknownEnumValue,
            )
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=notSupportedStreamUsage,
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=30,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewportRes,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                keyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            asserts.fail(
                "Unexpected success when expecting INVALID_IN_STATE due to StreamUsage set to a value not in aStreamUsagePriorities",
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.InvalidInState,
                "Unexpected error returned when expecting InvalidInState due to StreamUsage set to a value not in aStreamUsagePriorities",
            )
            pass

        self.step(19)
        try:
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=0,
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewportRes,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                keyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            asserts.fail("Unexpected success when expecting CONSTRAINT_ERROR due to MinFrameRate set to 0(outside of valid range)")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected error returned when expecting CONSTRAINT_ERROR due to MinFrameRate set to 0(outside of valid range)",
            )
            pass

        self.step(20)
        try:
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=16,
                maxFrameRate=15,
                minResolution=aMinViewportRes,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                keyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            asserts.fail("Unexpected success when expecting CONSTRAINT_ERROR due to MinFrameRate > MaxFrameRate")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected error returned when expecting CONSTRAINT_ERROR due to MinFrameRate > MaxFrameRate",
            )
            pass

        self.step(21)
        try:
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=30,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewportRes,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=0,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                keyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            asserts.fail("Unexpected success when expecting CONSTRAINT_ERROR due to MinBitRate set to 0(outside of valid range)")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected error returned when expecting CONSTRAINT_ERROR due to MinBitRate set to 0(outside of valid range)",
            )
            pass

        self.step(22)
        try:
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=30,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewportRes,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate + 1,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                keyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            asserts.fail("Unexpected success when expecting CONSTRAINT_ERROR due to MinBitRate > MaxBitRate")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected error returned when expecting CONSTRAINT_ERROR due to MinBitRate > MaxBitRate",
            )
            pass

        self.step(23)
        try:
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=30,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewportRes,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                keyFrameInterval=65500 + 1,
                watermarkEnabled=watermark,
                OSDEnabled=osd,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            asserts.fail("Unexpected success when expecting CONSTRAINT_ERROR due to MinKeyFrameInterval > MaxKeyFrameInterval")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected error returned when expecting CONSTRAINT_ERROR due to MinKeyFrameInterval > MaxKeyFrameInterval",
            )
            pass

        self.step(24)
        try:
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=10,
                minFrameRate=30,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewportRes,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                keyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            asserts.fail("Unexpected success when expecting CONSTRAINT_ERROR due to invalid codec")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected error returned when expecting CONSTRAINT_ERROR due to invalid codec",
            )
            pass

        self.step(25)
        try:
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=30,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS + 10,
                minResolution=aMinViewportRes,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                keyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            asserts.fail("Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to unsupported MaxFrameRate")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.DynamicConstraintError,
                "Unexpected error returned when expecting DYNAMIC_CONSTRAINT_ERROR due to unsupported MaxFrameRate",
            )
            pass


if __name__ == "__main__":
    default_matter_test_main()
