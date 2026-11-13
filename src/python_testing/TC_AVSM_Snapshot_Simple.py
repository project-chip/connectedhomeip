#
#    Copyright (c) 2026 Project CHIP Authors
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
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --camera-test-videosrc
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

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVSM_Snapshot_Simple(MatterTestCommissionedDevice):
    disable_wildcard_subscription = True

    @async_test_body
    async def test_snapshot(self):
        endpoint = self.user_params.get("endpoint", 1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        # Read FeatureMap
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap
        )
        if not (feature_map & cluster.Bitmaps.Feature.kSnapshot):
            asserts.skip("Snapshot feature not supported, skipping test")
        watermark_supported = feature_map & cluster.Bitmaps.Feature.kWatermark
        osd_supported = feature_map & cluster.Bitmaps.Feature.kOnScreenDisplay

        # Read Capabilities
        caps = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.SnapshotCapabilities
        )
        if not caps:
            asserts.skip("SnapshotCapabilities list is empty, skipping snapshot test")

        # Allocate Snapshot Stream
        stream_id = None
        selected_cap = None
        last_error = None
        for cap in reversed(caps):
            try:
                args = {
                    "imageCodec": cap.imageCodec,
                    "maxFrameRate": cap.maxFrameRate,
                    "minResolution": cap.resolution,
                    "maxResolution": cap.resolution,
                    "quality": 90,
                }
                if watermark_supported:
                    args["watermarkEnabled"] = False
                if osd_supported:
                    args["OSDEnabled"] = False
                allocate_cmd = commands.SnapshotStreamAllocate(**args)
                response = await self.send_single_cmd(endpoint=endpoint, cmd=allocate_cmd)
                stream_id = response.snapshotStreamID
                selected_cap = cap
                log.info("Allocated snapshot stream ID: %s", stream_id)
                break
            except InteractionModelError as e:
                last_error = e
                log.warning("SnapshotStreamAllocate failed for capability %s: %s", cap, e)

        if stream_id is None:
            asserts.fail(f"Failed to allocate snapshot stream using any advertised capability: {last_error}")

        # Capture Snapshot with explicit stream_id and with NullValue stream_id
        try:
            capture_cases = [
                (stream_id, selected_cap.resolution),
                (NullValue, selected_cap.resolution),
            ]
            if caps[0].resolution != selected_cap.resolution:
                capture_cases.append((NullValue, caps[0].resolution))

            for test_stream_id, req_resolution in capture_cases:
                capture_cmd = commands.CaptureSnapshot(
                    snapshotStreamID=test_stream_id,
                    requestedResolution=req_resolution,
                )
                capture_response = await self.send_single_cmd(
                    cmd=capture_cmd,
                    endpoint=endpoint,
                    payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
                )
                log.info("CaptureSnapshotResponse (stream_id=%s) resolution: %s", test_stream_id, capture_response.resolution)
                log.info("CaptureSnapshotResponse (stream_id=%s) codec: %s", test_stream_id, capture_response.imageCodec)
                log.info("CaptureSnapshotResponse (stream_id=%s) data size: %d", test_stream_id, len(capture_response.data))
                asserts.assert_greater(len(capture_response.data), 0, "Captured image data is empty")
                asserts.assert_equal(capture_response.imageCodec, selected_cap.imageCodec, "Image codec mismatch")
                asserts.assert_equal(capture_response.resolution, selected_cap.resolution, "Image resolution mismatch")
                if capture_response.imageCodec == cluster.Enums.ImageCodecEnum.kJpeg:
                    asserts.assert_true(capture_response.data.startswith(b'\xff\xd8'),
                                        "Captured image does not start with JPEG SOI marker")
                    asserts.assert_true(capture_response.data.endswith(b'\xff\xd9'),
                                        "Captured image does not end with JPEG EOI marker")
        except InteractionModelError as e:
            asserts.fail(f"CaptureSnapshot failed: {e}")
        finally:
            # Deallocate Stream
            if stream_id is not None:
                try:
                    await self.send_single_cmd(
                        endpoint=endpoint,
                        cmd=commands.SnapshotStreamDeallocate(snapshotStreamID=stream_id),
                    )
                except Exception as e:
                    asserts.fail(f"Failed to deallocate snapshot stream {stream_id}: {e}")

        # Verify deallocation cleanup and stream range-narrowing reuse with CaptureSnapshot(NullValue)
        if (
            len(caps) > 1
            and caps[0].imageCodec == selected_cap.imageCodec
            and caps[0].resolution.width <= selected_cap.resolution.width
            and caps[0].resolution.height <= selected_cap.resolution.height
            and caps[0].resolution != selected_cap.resolution
        ):
            range_stream_id = None
            try:
                range_args = {
                    "imageCodec": selected_cap.imageCodec,
                    "maxFrameRate": min(caps[0].maxFrameRate, selected_cap.maxFrameRate),
                    "minResolution": caps[0].resolution,
                    "maxResolution": selected_cap.resolution,
                    "quality": 90,
                }
                if watermark_supported:
                    range_args["watermarkEnabled"] = False
                if osd_supported:
                    range_args["OSDEnabled"] = False

                range_resp = await self.send_single_cmd(
                    endpoint=endpoint, cmd=commands.SnapshotStreamAllocate(**range_args)
                )
                range_stream_id = range_resp.snapshotStreamID
                asserts.assert_equal(
                    range_stream_id,
                    stream_id,
                    "Deallocated snapshot stream ID should be reclaimed on next allocation",
                )

                # Narrow the allocated stream's resolution range to selected_cap.resolution via stream reuse
                narrow_args = dict(range_args)
                narrow_args["minResolution"] = selected_cap.resolution
                narrow_args["maxResolution"] = selected_cap.resolution
                narrow_resp = await self.send_single_cmd(
                    endpoint=endpoint, cmd=commands.SnapshotStreamAllocate(**narrow_args)
                )
                asserts.assert_equal(
                    narrow_resp.snapshotStreamID,
                    range_stream_id,
                    "Narrower SnapshotStreamAllocate should reuse existing snapshot stream",
                )

                capture_response = await self.send_single_cmd(
                    cmd=commands.CaptureSnapshot(
                        snapshotStreamID=NullValue,
                        requestedResolution=caps[0].resolution,
                    ),
                    endpoint=endpoint,
                    payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
                )
                asserts.assert_greater(len(capture_response.data), 0, "Captured image data after range narrowing is empty")
                asserts.assert_equal(
                    capture_response.resolution,
                    selected_cap.resolution,
                    "CaptureSnapshot after range narrowing did not use updated HAL stream resolution",
                )

                # Verify persisted snapshot stream reload + CaptureSnapshot(NullValue) across reboot
                if self.get_restart_flag_file():
                    await self.request_device_reboot()
                    reboot_capture_response = await self.send_single_cmd(
                        cmd=commands.CaptureSnapshot(
                            snapshotStreamID=NullValue,
                            requestedResolution=caps[0].resolution,
                        ),
                        endpoint=endpoint,
                        payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
                    )
                    asserts.assert_greater(
                        len(reboot_capture_response.data), 0, "Captured image data after reboot is empty"
                    )
                    asserts.assert_equal(
                        reboot_capture_response.resolution,
                        selected_cap.resolution,
                        "CaptureSnapshot after reboot did not match persisted allocated stream resolution",
                    )
            finally:
                if range_stream_id is not None:
                    try:
                        await self.send_single_cmd(
                            endpoint=endpoint,
                            cmd=commands.SnapshotStreamDeallocate(snapshotStreamID=range_stream_id),
                        )
                    except InteractionModelError as e:
                        log.error("Failed to deallocate snapshot stream %s: %s", range_stream_id, e)

        # Verify concurrent Video and Snapshot stream execution in both allocation and deallocation orders
        if feature_map & cluster.Bitmaps.Feature.kVideo:
            stream_usages = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities
            )
            trade_offs = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.RateDistortionTradeOffPoints
            )
            min_viewport = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MinViewportResolution
            )
            sensor_params = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.VideoSensorParams
            )

            def make_video_alloc_cmd(fps: int):
                v_args = {
                    "streamUsage": stream_usages[0],
                    "videoCodec": trade_offs[0].codec,
                    "minFrameRate": fps,
                    "maxFrameRate": fps,
                    "minResolution": min_viewport,
                    "maxResolution": cluster.Structs.VideoResolutionStruct(
                        width=sensor_params.sensorWidth, height=sensor_params.sensorHeight
                    ),
                    "minBitRate": trade_offs[0].minBitRate,
                    "maxBitRate": trade_offs[0].minBitRate,
                    "keyFrameInterval": 4000,
                }
                if watermark_supported:
                    v_args["watermarkEnabled"] = False
                if osd_supported:
                    v_args["OSDEnabled"] = False
                return commands.VideoStreamAllocate(**v_args)

            def make_snapshot_alloc_cmd(cap_entry):
                s_args = {
                    "imageCodec": cap_entry.imageCodec,
                    "maxFrameRate": cap_entry.maxFrameRate,
                    "minResolution": cap_entry.resolution,
                    "maxResolution": cap_entry.resolution,
                    "quality": 90,
                }
                if watermark_supported:
                    s_args["watermarkEnabled"] = False
                if osd_supported:
                    s_args["OSDEnabled"] = False
                return commands.SnapshotStreamAllocate(**s_args)

            async def dealloc_quietly(cmd):
                try:
                    await self.send_single_cmd(endpoint=endpoint, cmd=cmd)
                except InteractionModelError as e:
                    log.error("Cleanup command %s failed: %s", cmd, e)

            def extract_jpeg_dimensions(jpeg_bytes: bytes):
                idx = 2
                n = len(jpeg_bytes)
                while idx + 4 <= n:
                    if jpeg_bytes[idx] != 0xFF:
                        idx += 1
                        continue
                    while idx < n and jpeg_bytes[idx] == 0xFF:
                        idx += 1
                    if idx >= n:
                        break
                    marker = jpeg_bytes[idx]
                    idx += 1
                    if marker in (0xD8, 0xD9) or (0xD0 <= marker <= 0xD7) or marker == 0x01:
                        continue
                    if idx + 2 > n:
                        break
                    seg_len = int.from_bytes(jpeg_bytes[idx:idx + 2], "big")
                    if seg_len < 2 or idx + seg_len > n:
                        break
                    if marker in (0xC0, 0xC1, 0xC2) and seg_len >= 7:
                        height = int.from_bytes(jpeg_bytes[idx + 3:idx + 5], "big")
                        width = int.from_bytes(jpeg_bytes[idx + 5:idx + 7], "big")
                        return width, height
                    idx += seg_len
                return None, None

            async def assert_valid_snapshot_capture(target_stream_id, cap_entry):
                resp = await self.send_single_cmd(
                    cmd=commands.CaptureSnapshot(
                        snapshotStreamID=target_stream_id,
                        requestedResolution=cap_entry.resolution,
                    ),
                    endpoint=endpoint,
                    payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
                )
                asserts.assert_greater(len(resp.data), 0, "Captured image data is empty during concurrent streaming")
                asserts.assert_equal(resp.imageCodec, cap_entry.imageCodec, "Image codec mismatch")
                asserts.assert_equal(resp.resolution, cap_entry.resolution, "Image resolution mismatch")
                if resp.imageCodec == cluster.Enums.ImageCodecEnum.kJpeg:
                    asserts.assert_true(resp.data.startswith(b"\xff\xd8"), "JPEG SOI marker missing")
                    asserts.assert_true(resp.data.endswith(b"\xff\xd9"), "JPEG EOI marker missing")
                    jpeg_width, jpeg_height = extract_jpeg_dimensions(resp.data)
                    asserts.assert_equal(jpeg_width, cap_entry.resolution.width, "Encoded JPEG width mismatch")
                    asserts.assert_equal(jpeg_height, cap_entry.resolution.height, "Encoded JPEG height mismatch")

            # Case 1: Allocate lower-res Video stream first, then higher-res Snapshot stream second; deallocate Video first, then Snapshot
            video_id = None
            snap_id = None
            try:
                video_resp = await self.send_single_cmd(endpoint=endpoint, cmd=make_video_alloc_cmd(30))
                video_id = video_resp.videoStreamID
                current_fps = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.CurrentFrameRate
                )
                asserts.assert_equal(current_fps, 30, "Video stream failed to start (CurrentFrameRate != 30)")

                snap_resp = await self.send_single_cmd(endpoint=endpoint, cmd=make_snapshot_alloc_cmd(selected_cap))
                snap_id = snap_resp.snapshotStreamID

                await assert_valid_snapshot_capture(snap_id, selected_cap)
                await assert_valid_snapshot_capture(NullValue, selected_cap)

                # Stop Video stream while Snapshot stream stays active, and verify CaptureSnapshot still succeeds
                await self.send_single_cmd(
                    endpoint=endpoint, cmd=commands.VideoStreamDeallocate(videoStreamID=video_id)
                )
                video_id = None

                await assert_valid_snapshot_capture(snap_id, selected_cap)
            finally:
                if video_id is not None:
                    await dealloc_quietly(commands.VideoStreamDeallocate(videoStreamID=video_id))
                if snap_id is not None:
                    await dealloc_quietly(commands.SnapshotStreamDeallocate(snapshotStreamID=snap_id))

            # Case 2: Allocate lower-res Snapshot stream first, then higher-fps Video stream second, then higher-res Snapshot stream;
            # deallocate Snapshot streams first, then Video
            video_id = None
            snap_id = None
            snap_hd_id = None
            try:
                snap_resp = await self.send_single_cmd(endpoint=endpoint, cmd=make_snapshot_alloc_cmd(caps[0]))
                snap_id = snap_resp.snapshotStreamID
                await assert_valid_snapshot_capture(snap_id, caps[0])

                target_fps = min(60, sensor_params.maxFPS)
                video_resp = await self.send_single_cmd(endpoint=endpoint, cmd=make_video_alloc_cmd(target_fps))
                video_id = video_resp.videoStreamID
                current_fps = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.CurrentFrameRate
                )
                asserts.assert_equal(
                    current_fps,
                    target_fps,
                    "Video stream failed to start while snapshot stream was active",
                )

                if selected_cap.resolution != caps[0].resolution:
                    snap_hd_resp = await self.send_single_cmd(endpoint=endpoint, cmd=make_snapshot_alloc_cmd(selected_cap))
                    snap_hd_id = snap_hd_resp.snapshotStreamID
                    await assert_valid_snapshot_capture(snap_hd_id, selected_cap)

                # Capture snapshot while both Snapshot and Video streams are running concurrently
                await assert_valid_snapshot_capture(snap_id, caps[0])

                if snap_hd_id is not None:
                    await self.send_single_cmd(
                        endpoint=endpoint, cmd=commands.SnapshotStreamDeallocate(snapshotStreamID=snap_hd_id)
                    )
                    snap_hd_id = None

                # Deallocate Snapshot stream while Video stream remains active, then re-allocate Snapshot stream
                await self.send_single_cmd(
                    endpoint=endpoint, cmd=commands.SnapshotStreamDeallocate(snapshotStreamID=snap_id)
                )
                snap_id = None

                snap_resp = await self.send_single_cmd(endpoint=endpoint, cmd=make_snapshot_alloc_cmd(selected_cap))
                snap_id = snap_resp.snapshotStreamID
                await assert_valid_snapshot_capture(snap_id, selected_cap)
            finally:
                if snap_hd_id is not None:
                    await dealloc_quietly(commands.SnapshotStreamDeallocate(snapshotStreamID=snap_hd_id))
                if snap_id is not None:
                    await dealloc_quietly(commands.SnapshotStreamDeallocate(snapshotStreamID=snap_id))
                if video_id is not None:
                    await dealloc_quietly(commands.VideoStreamDeallocate(videoStreamID=video_id))


if __name__ == "__main__":
    default_matter_test_main()
