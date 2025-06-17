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

import chip.clusters as Clusters
from chip.clusters import Globals
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_AVSM_2_7(MatterBaseTest):
    def desc_TC_AVSM_2_7(self) -> str:
        return "[TC-AVSM-2.7] Validate Video Stream Allocation functionality with Server as DUT"

    def pics_TC_AVSM_2_7(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_7(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(
                1, "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on TH_SERVER", "Verify VDO is supported."
            ),
            TestStep(
                2,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated video streams in the list is 0.",
            ),
            TestStep(
                3,
                "TH reads StreamUsagePriorities attribute from CameraAVStreamManagement Cluster on TH_SERVER.",
                "Store this value in aStreamUsagePriorities.",
            ),
            TestStep(
                4,
                "TH reads RateDistortionTradeOffPoints attribute from CameraAVStreamManagement Cluster on TH_SERVER.",
                "Store this value in aRateDistortionTradeOffPoints.",
            ),
            TestStep(
                5,
                "TH reads MinViewport attribute from CameraAVStreamManagement Cluster on TH_SERVER.",
                "Store this value in aMinViewport.",
            ),
            TestStep(
                6,
                "TH reads VideoSensorParams attribute from CameraAVStreamManagement Cluster on TH_SERVER.",
                "Store this value in aVideoSensorParams.",
            ),
            TestStep(
                7,
                "TH reads MaxEncodedPixelRate attribute from CameraAVStreamManagement Cluster on TH_SERVER.",
                "Store this value in aMaxEncodedPixelRate.",
            ),
            TestStep(
                8,
                "TH sets StreamUsage from aStreamUsagePriorities. TH sets VideoCodec, MinResolution, MaxResolution, MinBitRate, MaxBitRate conforming with aRateDistortionTradeOffPoints. TH sets MinFrameRate, MaxFrameRate conforming with aVideoSensorParams. TH sets the MinKeyFrameInterval and MaxKeyFrameInterval = 4000. TH sends the VideoStreamAllocate command with these arguments.",
                "DUT responds with VideoStreamAllocateResponse command with a valid VideoStreamID.",
            ),
            TestStep(
                9,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated video streams in the list is 1.",
            ),
            TestStep(
                10,
                "TH sends the VideoStreamAllocate command with the same arguments from step 7 except StreamUsage set to a value not in aStreamUsagePriorities.",
                "DUT responds with a INVALID IN STATE status code.",
            ),
            TestStep(
                11,
                "TH sends the VideoStreamAllocate command with the same arguments from step 7 except MinFrameRate set to 0(outside of valid range).",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                12,
                "TH sends the VideoStreamAllocate command with the same arguments from step 7 except MinFrameRate > MaxFrameRate.",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                13,
                "TH sends the VideoStreamAllocate command with the same arguments from step 7 except MinBitRate set to 0(outside of valid range).",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                14,
                "TH sends the VideoStreamAllocate command with the same arguments from step 7 except MinBitRate > MaxBitRate.",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                15,
                "TH sends the VideoStreamAllocate command with the same arguments from step 7 except MinKeyFrameInterval > MaxKeyFrameInterval.",
                "DUT responds with a CONSTRAINT_ERROR status code.",
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

        # Check for watermark and OSD features
        watermark = True if (aFeatureMap & cluster.Bitmaps.Feature.kWatermark) != 0 else None
        osd = True if (aFeatureMap & cluster.Bitmaps.Feature.kOnScreenDisplay) != 0 else None

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
        aMinViewport = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.MinViewport
        )
        logger.info(f"Rx'd MinViewport: {aMinViewport}")

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

        self.step(8)
        try:
            asserts.assert_greater(len(aStreamUsagePriorities), 0, "StreamUsagePriorities is empty")
            asserts.assert_greater(len(aRateDistortionTradeOffPoints), 0, "RateDistortionTradeOffPoints is empty")
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=30,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewport,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                minKeyFrameInterval=4000,
                maxKeyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd
            )
            videoStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            logger.info(f"Rx'd VideoStreamAllocateResponse: {videoStreamAllocateResponse}")
            asserts.assert_is_not_none(
                videoStreamAllocateResponse.videoStreamID, "VideoStreamAllocateResponse does not contain StreamID"
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(9)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        logger.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 1, "The number of allocated video streams in the list is not 1")

        self.step(10)
        try:
            notSupportedStreamUsage = next(
                (e for e in Globals.Enums.StreamUsageEnum if e not in aStreamUsagePriorities),
                Globals.Enums.StreamUsageEnum.kUnknownEnumValue,
            )
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=notSupportedStreamUsage,
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=30,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewport,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                minKeyFrameInterval=4000,
                maxKeyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            asserts.assert_true(
                False,
                "Unexpected success when expecting INVALID_IN_STATE due to StreamUsage set to a value not in aStreamUsagePriorities",
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.InvalidInState,
                "Unexpected error returned when expecting InvalidInState due to StreamUsage set to a value not in aStreamUsagePriorities",
            )
            pass

        self.step(11)
        try:
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=0,
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewport,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                minKeyFrameInterval=4000,
                maxKeyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            asserts.assert_true(
                False, "Unexpected success when expecting CONSTRAIN_ERROR due to MinFrameRate set to 0(outside of valid range)"
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected error returned when expecting CONSTRAIN_ERROR due to MinFrameRate set to 0(outside of valid range)",
            )
            pass

        self.step(12)
        try:
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=16,
                maxFrameRate=15,
                minResolution=aMinViewport,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                minKeyFrameInterval=4000,
                maxKeyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            asserts.assert_true(False, "Unexpected success when expecting CONSTRAIN_ERROR due to MinFrameRate > MaxFrameRate")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected error returned when expecting CONSTRAIN_ERROR due to MinFrameRate > MaxFrameRate",
            )
            pass

        self.step(13)
        try:
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=30,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewport,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=0,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                minKeyFrameInterval=4000,
                maxKeyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            asserts.assert_true(
                False, "Unexpected success when expecting CONSTRAIN_ERROR due to MinBitRate set to 0(outside of valid range)"
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected error returned when expecting CONSTRAIN_ERROR due to MinBitRate set to 0(outside of valid range)",
            )
            pass

        self.step(14)
        try:
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=30,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewport,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate + 1,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                minKeyFrameInterval=4000,
                maxKeyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            asserts.assert_true(False, "Unexpected success when expecting CONSTRAIN_ERROR due to MinBitRate > MaxBitRate")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected error returned when expecting CONSTRAIN_ERROR due to MinBitRate > MaxBitRate",
            )
            pass

        self.step(15)
        try:
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=30,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewport,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate + 1,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                minKeyFrameInterval=4000 + 1,
                maxKeyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            asserts.assert_true(
                False, "Unexpected success when expecting CONSTRAIN_ERROR due to MinKeyFrameInterval > MaxKeyFrameInterval")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected error returned when expecting CONSTRAIN_ERROR due to MinKeyFrameInterval > MaxKeyFrameInterval",
            )
            pass


if __name__ == "__main__":
    default_matter_test_main()
