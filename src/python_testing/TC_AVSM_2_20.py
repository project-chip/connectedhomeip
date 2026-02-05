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


class TC_AVSM_2_20(MatterBaseTest, AVSMTestBase):

    def desc_TC_AVSM_2_20(self) -> str:
        return "[TC-AVSM-2.20] Validate persistence of allocated snapshot streams with DUT"

    def steps_TC_AVSM_2_20(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on DUT. Verify F_SNP is supported."),
            TestStep(3, "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated snapshot streams in the list is 0."),
            TestStep(4, "TH reads SnapshotCapabilities attribute from CameraAVStreamManagement Cluster on DUT. Store this value in aSnapshotCapabilities."),
            TestStep(5, "If F_WMARK is supported, TH sets its local aWatermark to True, otherwise this is Null."),
            TestStep(6, "If F_OSD is supported, TH sets its local aOSD to True, otherwise this is Null."),
            TestStep(7, "TH sends the SnapshotStreamAllocate command with valid values of ImageCodec, MaxFrameRate, MinResolution=MaxResolution=Resolution from aSnapshotCapabilities, WatermarkEnabled to aWatermark, OSDEnabled to aOSD, and Quality set to 90."),
            TestStep(8, "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated snapshot streams in the list is 1."),
            TestStep(9, "TH reboots the DUT."),
            TestStep(10, "TH waits for the DUT to come back online."),
            TestStep(11, "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated snapshot streams in the list is 1 and the stream info is identical to what was provided in step 7."),
            TestStep(12, "TH sends the SnapshotStreamDeallocate command with SnapshotStreamID set to myStreamID."),
            TestStep(13, "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated snapshot streams in the list is 0."),
            TestStep(14, "TH reboots the DUT."),
            TestStep(15, "TH waits for the DUT to come back online."),
            TestStep(16, "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated snapshot streams in the list is 0."),
        ]

    def pics_TC_AVSM_2_20(self) -> list[str]:
        return ["AVSM.S", "AVSM.S.F_SNP"]

    @run_if_endpoint_matches(
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kSnapshot)
    )
    async def test_TC_AVSM_2_20(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step(1)

        self.step(2)
        feature_map = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        has_f_snp = (feature_map & cluster.Bitmaps.Feature.kSnapshot) != 0
        asserts.assert_true(has_f_snp, "FeatureMap F_SNP is not set")

        self.step(3)
        allocated_snapshot_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams)
        asserts.assert_equal(len(allocated_snapshot_streams), 0, "AllocatedSnapshotStreams should be empty")

        self.step(4)
        snapshot_capabilities = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.SnapshotCapabilities)
        asserts.assert_greater(len(snapshot_capabilities), 0, "SnapshotCapabilities should not be empty")

        self.step(5)
        has_f_wmark = (feature_map & cluster.Bitmaps.Feature.kWatermark) != 0
        watermark_enabled = True if has_f_wmark else None

        self.step(6)
        has_f_osd = (feature_map & cluster.Bitmaps.Feature.kOnScreenDisplay) != 0
        osd_enabled = True if has_f_osd else None

        self.step(7)
        capability = snapshot_capabilities[0]
        image_codec = capability.imageCodec
        frame_rate = capability.maxFrameRate
        resolution = capability.resolution
        quality = 90

        cmd = commands.SnapshotStreamAllocate(
            imageCodec=image_codec,
            maxFrameRate=frame_rate,
            minResolution=resolution,
            maxResolution=resolution,
            quality=quality,
            watermarkEnabled=watermark_enabled,
            OSDEnabled=osd_enabled,
        )

        resp = await self.send_single_cmd(endpoint=endpoint, cmd=cmd)
        my_stream_id = resp.snapshotStreamID
        asserts.assert_is_not_none(my_stream_id, "SnapshotStreamID should not be None")

        allocated_stream_info = cluster.Structs.SnapshotStreamStruct(
            snapshotStreamID=my_stream_id,
            imageCodec=image_codec,
            frameRate=frame_rate,
            minResolution=resolution,
            maxResolution=resolution,
            watermarkEnabled=watermark_enabled,
            OSDEnabled=osd_enabled,
            quality=quality,
            referenceCount=0
        )

        self.step(8)
        allocated_snapshot_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams)
        asserts.assert_equal(len(allocated_snapshot_streams), 1, "AllocatedSnapshotStreams should have 1 entry")

        self.step(9)
        await self.request_device_reboot()
        self.step(10)

        self.step(11)
        allocated_snapshot_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams)
        asserts.assert_equal(len(allocated_snapshot_streams), 1, "AllocatedSnapshotStreams should have 1 entry after reboot")
        retrieved_stream = allocated_snapshot_streams[0]
        asserts.assert_equal(retrieved_stream.snapshotStreamID, allocated_stream_info.snapshotStreamID, "snapshotStreamID mismatch")
        asserts.assert_equal(retrieved_stream.imageCodec, allocated_stream_info.imageCodec, "imageCodec mismatch")
        asserts.assert_equal(retrieved_stream.frameRate, allocated_stream_info.frameRate, "frameRate mismatch")
        asserts.assert_equal(retrieved_stream.minResolution, allocated_stream_info.minResolution, "minResolution mismatch")
        asserts.assert_equal(retrieved_stream.maxResolution, allocated_stream_info.maxResolution, "maxResolution mismatch")
        asserts.assert_equal(retrieved_stream.watermarkEnabled, allocated_stream_info.watermarkEnabled, "watermarkEnabled mismatch")
        asserts.assert_equal(retrieved_stream.OSDEnabled, allocated_stream_info.OSDEnabled, "OSDEnabled mismatch")
        asserts.assert_equal(retrieved_stream.quality, allocated_stream_info.quality, "quality mismatch")

        self.step(12)
        await self.send_single_cmd(cmd=commands.SnapshotStreamDeallocate(snapshotStreamID=my_stream_id), endpoint=endpoint)

        self.step(13)
        allocated_snapshot_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams)
        asserts.assert_equal(len(allocated_snapshot_streams), 0, "AllocatedSnapshotStreams should be empty after deallocate")

        self.step(14)
        await self.request_device_reboot()
        self.step(15)

        self.step(16)
        allocated_snapshot_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams)
        asserts.assert_equal(len(allocated_snapshot_streams), 0, "AllocatedSnapshotStreams should be empty after reboot")


if __name__ == "__main__":
    default_matter_test_main()
