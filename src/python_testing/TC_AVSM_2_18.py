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
#     app-ready-pattern: "APP STATUS: Starting event loop"
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
from TC_AVSMTestBase import AVSMTestBase

import matter.clusters as Clusters
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVSM_2_18(MatterBaseTest, AVSMTestBase):

    def desc_TC_AVSM_2_18(self) -> str:
        return "[TC-AVSM-2.18] Validate persistence of allocated video streams with DUT"

    def steps_TC_AVSM_2_18(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on DUT. Verify F_VDO is supported."),
            TestStep(3, "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated video streams in the list is 0."),
            TestStep(4, "TH reads StreamUsagePriorities attribute from CameraAVStreamManagement Cluster on DUT. Store this value in aStreamUsagePriorities."),
            TestStep(5, "TH reads RateDistortionTradeOffPoints attribute from CameraAVStreamManagement Cluster on DUT. Store this value in aRateDistortionTradeOffPoints."),
            TestStep(6, "TH reads MinViewportResolution attribute from CameraAVStreamManagement Cluster on DUT. Store this value in aMinViewportResolution."),
            TestStep(7, "TH reads VideoSensorParams attribute from CameraAVStreamManagement Cluster on DUT. Store this value in aVideoSensorParams."),
            TestStep(8, "TH reads MaxEncodedPixelrate attribute from CameraAVStreamManagement Cluster on DUT. Store this value in aMaxEncodedPixelRate."),
            TestStep(9, "If F_WMARK is supported, TH sets it's local aWatermark to True, otherwise this is Null."),
            TestStep(10, "If F_OSD is supported, TH sets its local aOSD to True, otherwise this is Null."),
            TestStep(11, "TH sets StreamUsage from aStreamUsagePriorities. TH sets VideoCodec, MinResolution, MaxResolution, MinBitRate, MaxBitRate conforming with aRateDistortionTradeOffPoints. TH sets MinFrameRate, MaxFrameRate conforming with aVideoSensorParams. TH sets the KeyFrameInterval = 4000. TH sets WatermarkEnabled to aWatermark, TH also sets OSDEnabled to aOSD. TH sends the VideoStreamAllocate command with these arguments."),
            TestStep(12, "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated video streams in the list is 1."),
            TestStep(13, "TH reboots the DUT."),
            TestStep(14, "TH waits for the DUT to come back online."),
            TestStep(15, "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated video streams in the list is 1 and the stream info is identical to what was provided in step 11."),
            TestStep(16, "TH sends the VideoStreamDeallocate command with VideoStreamID set to myStreamID."),
            TestStep(17, "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated video streams in the list is 0."),
            TestStep(18, "TH reboots the DUT."),
            TestStep(19, "TH waits for the DUT to come back online."),
            TestStep(20, "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated video streams in the list is 0."),
        ]

    def pics_TC_AVSM_2_18(self) -> list[str]:
        return ["AVSM.S", "AVSM.S.F_VDO"]

    @run_if_endpoint_matches(
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kVideo)
    )
    async def test_TC_AVSM_2_18(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step(1)

        self.step(2)
        feature_map = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        has_f_vdo = (feature_map & cluster.Bitmaps.Feature.kVideo) != 0
        asserts.assert_true(has_f_vdo, "FeatureMap F_VDO is not set")

        self.step(3)
        allocated_video_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams)
        asserts.assert_equal(len(allocated_video_streams), 0, "AllocatedVideoStreams should be empty")

        self.step(4)
        stream_usage_priorities = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities)
        asserts.assert_greater(len(stream_usage_priorities), 0, "StreamUsagePriorities should not be empty")

        self.step(5)
        rate_distortion_trade_off_points = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.RateDistortionTradeOffPoints)
        asserts.assert_greater(len(rate_distortion_trade_off_points), 0, "RateDistortionTradeOffPoints should not be empty")

        self.step(6)
        min_viewport_resolution = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.MinViewportResolution)
        log.info(f"Rx'd MinViewportResolution: {min_viewport_resolution}")

        self.step(7)
        video_sensor_params = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.VideoSensorParams)
        log.info(f"Rx'd VideoSensorParams: {video_sensor_params}")

        self.step(8)
        max_encoded_pixel_rate = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.MaxEncodedPixelRate)
        log.info(f"Rx'd MaxEncodedPixelRate: {max_encoded_pixel_rate}")

        self.step(9)
        has_f_wmark = (feature_map & cluster.Bitmaps.Feature.kWatermark) != 0
        watermark_enabled = True if has_f_wmark else None

        self.step(10)
        has_f_osd = (feature_map & cluster.Bitmaps.Feature.kOnScreenDisplay) != 0
        osd_enabled = True if has_f_osd else None

        self.step(11)
        # Select valid parameters based on device capabilities
        stream_usage = stream_usage_priorities[0]
        trade_off_point = rate_distortion_trade_off_points[0]
        video_codec = trade_off_point.codec
        min_resolution = min_viewport_resolution
        min_bit_rate = trade_off_point.minBitRate
        max_bit_rate = trade_off_point.minBitRate
        min_frame_rate = 30
        max_frame_rate = video_sensor_params.maxFPS
        key_frame_interval = 4000

        cmd = commands.VideoStreamAllocate(
            streamUsage=stream_usage,
            videoCodec=video_codec,
            minFrameRate=min_frame_rate,
            maxFrameRate=max_frame_rate,
            minResolution=min_resolution,
            maxResolution=cluster.Structs.VideoResolutionStruct(
                width=video_sensor_params.sensorWidth, height=video_sensor_params.sensorHeight
            ),
            minBitRate=min_bit_rate,
            maxBitRate=max_bit_rate,
            keyFrameInterval=key_frame_interval,
            watermarkEnabled=watermark_enabled,
            OSDEnabled=osd_enabled
        )

        resp = await self.send_single_cmd(endpoint=endpoint, cmd=cmd)
        my_stream_id = resp.videoStreamID
        asserts.assert_is_not_none(my_stream_id, "VideoStreamID should not be None")

        # Store the allocated stream info for comparison after reboot
        allocated_stream_info = cluster.Structs.VideoStreamStruct(
            videoStreamID=my_stream_id,
            streamUsage=stream_usage,
            videoCodec=video_codec,
            minResolution=min_resolution,
            maxResolution=cluster.Structs.VideoResolutionStruct(
                width=video_sensor_params.sensorWidth, height=video_sensor_params.sensorHeight
            ),
            minBitRate=min_bit_rate,
            maxBitRate=max_bit_rate,
            minFrameRate=min_frame_rate,
            maxFrameRate=max_frame_rate,
            keyFrameInterval=key_frame_interval,
            watermarkEnabled=watermark_enabled,
            OSDEnabled=osd_enabled,
            referenceCount=0
        )

        self.step(12)
        allocated_video_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams)
        asserts.assert_equal(len(allocated_video_streams), 1, "AllocatedVideoStreams should have 1 entry")

        self.step(13)
        await self.request_device_reboot()
        self.step(14)
        # Wait for device to be online is handled by request_device_reboot

        self.step(15)
        allocated_video_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams)
        asserts.assert_equal(len(allocated_video_streams), 1, "AllocatedVideoStreams should have 1 entry after reboot")
        # Compare the structs, fields should be the same, except referenceCount, which is not persisted
        retrieved_stream = allocated_video_streams[0]
        asserts.assert_equal(retrieved_stream.videoStreamID, allocated_stream_info.videoStreamID, "videoStreamID mismatch")
        asserts.assert_equal(retrieved_stream.streamUsage, allocated_stream_info.streamUsage, "streamUsage mismatch")
        asserts.assert_equal(retrieved_stream.videoCodec, allocated_stream_info.videoCodec, "videoCodec mismatch")
        asserts.assert_equal(retrieved_stream.minResolution, allocated_stream_info.minResolution, "minResolution mismatch")
        asserts.assert_equal(retrieved_stream.maxResolution, allocated_stream_info.maxResolution, "maxResolution mismatch")
        asserts.assert_equal(retrieved_stream.minBitRate, allocated_stream_info.minBitRate, "minBitRate mismatch")
        asserts.assert_equal(retrieved_stream.maxBitRate, allocated_stream_info.maxBitRate, "maxBitRate mismatch")
        asserts.assert_equal(retrieved_stream.minFrameRate, allocated_stream_info.minFrameRate, "minFrameRate mismatch")
        asserts.assert_equal(retrieved_stream.maxFrameRate, allocated_stream_info.maxFrameRate, "maxFrameRate mismatch")
        asserts.assert_equal(retrieved_stream.keyFrameInterval, allocated_stream_info.keyFrameInterval, "keyFrameInterval mismatch")
        asserts.assert_equal(retrieved_stream.watermarkEnabled, allocated_stream_info.watermarkEnabled, "watermarkEnabled mismatch")
        asserts.assert_equal(retrieved_stream.OSDEnabled, allocated_stream_info.OSDEnabled, "OSDEnabled mismatch")

        self.step(16)
        await self.send_single_cmd(cmd=commands.VideoStreamDeallocate(videoStreamID=my_stream_id), endpoint=endpoint)

        self.step(17)
        allocated_video_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams)
        asserts.assert_equal(len(allocated_video_streams), 0, "AllocatedVideoStreams should be empty after deallocate")

        self.step(18)
        await self.request_device_reboot()
        self.step(19)

        self.step(20)
        allocated_video_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams)
        asserts.assert_equal(len(allocated_video_streams), 0, "AllocatedVideoStreams should be empty after reboot")


if __name__ == "__main__":
    default_matter_test_main()
