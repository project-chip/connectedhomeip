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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --camera-test-videosrc
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

import contextlib
import logging
import subprocess
from dataclasses import dataclass

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


@dataclass(frozen=True)
class _RGBFrame:
    data: bytes
    width: int
    height: int


def _decode_jpeg_to_rgb(jpeg_bytes: bytes, expected_width: int, expected_height: int) -> _RGBFrame:
    """Decode JPEG bytes into an _RGBFrame containing raw RGB24 bytes (height * width * 3)."""
    asserts.assert_true(jpeg_bytes.startswith(b"\xff\xd8"), "Captured snapshot does not start with JPEG SOI marker")
    asserts.assert_true(jpeg_bytes.endswith(b"\xff\xd9"), "Captured snapshot does not end with JPEG EOI marker")
    expected_len = expected_width * expected_height * 3

    with contextlib.suppress(ImportError):
        import io

        from PIL import Image

        with Image.open(io.BytesIO(jpeg_bytes)) as img:
            rgb = img.convert("RGB")
            asserts.assert_equal(rgb.size, (expected_width, expected_height), "Decoded JPEG dimensions mismatch")
            raw = rgb.tobytes()
            asserts.assert_equal(len(raw), expected_len, "Decoded RGB24 byte length mismatch")
            return _RGBFrame(data=raw, width=expected_width, height=expected_height)

    # Fallback 1: ffmpeg rawvideo rgb24 pipe
    with contextlib.suppress(FileNotFoundError, subprocess.CalledProcessError):
        proc = subprocess.run(
            [
                "ffmpeg",
                "-v",
                "error",
                "-f",
                "image2pipe",
                "-vcodec",
                "mjpeg",
                "-i",
                "pipe:0",
                "-f",
                "rawvideo",
                "-pix_fmt",
                "rgb24",
                "pipe:1",
            ],
            input=jpeg_bytes,
            capture_output=True,
            check=True,
        )
        if len(proc.stdout) == expected_len:
            return _RGBFrame(data=proc.stdout, width=expected_width, height=expected_height)

    # Fallback 2: gst-launch-1.0 jpegdec pipe
    proc = subprocess.run(
        [
            "gst-launch-1.0",
            "-q",
            "fdsrc",
            "!",
            "jpegdec",
            "!",
            "videoconvert",
            "!",
            f"video/x-raw,format=RGB,width={expected_width},height={expected_height}",
            "!",
            "fdsink",
        ],
        input=jpeg_bytes,
        capture_output=True,
        check=True,
    )
    asserts.assert_equal(len(proc.stdout), expected_len, "Decoded RGB24 byte length mismatch from gst-launch-1.0")
    return _RGBFrame(data=proc.stdout, width=expected_width, height=expected_height)


class TC_ZONEMGMT_Privacy(MatterTestCommissionedDevice):
    def desc_TC_ZONEMGMT_Privacy(self) -> str:
        return "[TC-ZONEMGMT-Privacy] Verify Privacy Zone polygonal pixel black-out and lifecycle with Server as DUT"

    def pics_TC_ZONEMGMT_Privacy(self) -> list[str]:
        return ["ZONEMGMT.S", "AVSM.S"]

    def steps_TC_ZONEMGMT_Privacy(self) -> list[TestStep]:
        return [
            TestStep(
                "1",
                "Read ZoneManagement and CameraAVStreamManagement attributes and allocate a snapshot stream.",
                "TwoDCartesianMax is valid and a snapshot stream is allocated.",
                is_commissioning=True,
            ),
            TestStep(
                "2",
                "Capture baseline snapshot with no Privacy Zones active and decode JPEG pixels.",
                "Pixels in target regions A, B, and C are non-black.",
            ),
            TestStep(
                "3",
                "Create a rectangular Privacy Zone over Region A, capture a snapshot, and inspect decoded pixels.",
                "All pixels strictly inside Region A are black (R, G, B <= 8) while pixels outside (Regions B and C) remain non-black.",
            ),
            TestStep(
                "4",
                "Verify negative trigger and focus interactions on Privacy Zones (CreateOrUpdateTrigger on Privacy Zone, UpdateTwoDCartesianZone to Focus without FocusZones feature, and UpdateTwoDCartesianZone on triggered Motion Zone to Privacy).",
                "DUT returns ConstraintError and InvalidInState as required by specification.",
            ),
            TestStep(
                "5",
                "Update the Privacy Zone via UpdateTwoDCartesianZone to disjoint Region B, capture a snapshot, and inspect decoded pixels.",
                "Pixels inside Region B are black (R, G, B <= 8) while pixels inside former Region A and control Region C are non-black.",
            ),
            TestStep(
                "6",
                "Create a second concave L-shaped Privacy Zone over Region A, capture a snapshot, and inspect decoded pixels.",
                "Both L-shape arms in Region A and Region B are black (R, G, B <= 8), while the concave cutout in Region A and control Region C remain non-black.",
            ),
            TestStep(
                "7",
                "Remove all Privacy Zones via RemoveZone, capture a snapshot, and verify unmasked restoration.",
                "Pixels in Regions A, B, and C are restored to non-black.",
            ),
        ]

    @staticmethod
    def _pixel_to_zone_vertex(px: int, py: int, width: int, height: int, max_x: int, max_y: int):
        zx = int(round(px * max_x / (width - 1)))
        zy = int(round(py * max_y / (height - 1)))
        zx = max(0, min(max_x, zx))
        zy = max(0, min(max_y, zy))
        return Clusters.ZoneManagement.Structs.TwoDCartesianVertexStruct(x=zx, y=zy)

    @staticmethod
    def _assert_region_is_black(frame: _RGBFrame, x0: int, y0: int, x1: int, y1: int, label: str):
        asserts.assert_greater(x1, x0, f"Empty X range for {label}")
        asserts.assert_greater(y1, y0, f"Empty Y range for {label}")
        max_val = 0
        stride = frame.width * 3
        for y in range(y0, y1):
            row_start = y * stride + x0 * 3
            row_end = y * stride + x1 * 3
            row_max = max(frame.data[row_start:row_end])
            if row_max > max_val:
                max_val = row_max
        log.info("Region '%s' [x=%d..%d, y=%d..%d] max RGB channel value = %d", label, x0, x1, y0, y1, max_val)
        asserts.assert_less_equal(
            max_val,
            8,
            f"Expected all pixels in '{label}' [x={x0}..{x1}, y={y0}..{y1}] to be black (R,G,B <= 8), but found max channel {max_val}",
        )

    @staticmethod
    def _assert_region_is_non_black(frame: _RGBFrame, x0: int, y0: int, x1: int, y1: int, label: str):
        asserts.assert_greater(x1, x0, f"Empty X range for {label}")
        asserts.assert_greater(y1, y0, f"Empty Y range for {label}")
        min_brightest_channel = 255
        total_sum = 0
        total_channels = (x1 - x0) * (y1 - y0) * 3
        stride = frame.width * 3
        mv = memoryview(frame.data)
        for y in range(y0, y1):
            row_offset = y * stride
            for x in range(x0, x1):
                idx = row_offset + x * 3
                r = mv[idx]
                g = mv[idx + 1]
                b = mv[idx + 2]
                total_sum += r + g + b
                px_max = r if r >= g and r >= b else (g if g >= b else b)
                if px_max < min_brightest_channel:
                    min_brightest_channel = px_max
        mean_val = total_sum / total_channels
        log.info(
            "Region '%s' [x=%d..%d, y=%d..%d] min(max(R,G,B)) = %d, mean = %.1f",
            label,
            x0,
            x1,
            y0,
            y1,
            min_brightest_channel,
            mean_val,
        )
        asserts.assert_greater(
            min_brightest_channel,
            20,
            f"Expected pixels in '{label}' [x={x0}..{x1}, y={y0}..{y1}] to be non-black (> 20), but found min brightest channel {min_brightest_channel}",
        )

    async def _capture_and_decode_rgb(self, endpoint: int, stream_id: int, resolution) -> _RGBFrame:
        avsm_commands = Clusters.CameraAvStreamManagement.Commands
        response = await self.send_single_cmd(
            cmd=avsm_commands.CaptureSnapshot(
                snapshotStreamID=stream_id,
                requestedResolution=resolution,
            ),
            endpoint=endpoint,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )
        asserts.assert_greater(len(response.data), 0, "CaptureSnapshot returned empty data")
        asserts.assert_equal(response.resolution.width, resolution.width, "Snapshot width mismatch")
        asserts.assert_equal(response.resolution.height, resolution.height, "Snapshot height mismatch")
        return _decode_jpeg_to_rgb(response.data, resolution.width, resolution.height)

    @run_if_endpoint_matches(
        lambda wildcard, endpoint: has_cluster(Clusters.ZoneManagement)(wildcard, endpoint)
        and has_cluster(Clusters.CameraAvStreamManagement)(wildcard, endpoint)
    )
    async def test_TC_ZONEMGMT_Privacy(self):
        endpoint = self.get_endpoint()
        zm_cluster = Clusters.ZoneManagement
        zm_attr = zm_cluster.Attributes
        zm_commands = zm_cluster.Commands
        zm_enums = zm_cluster.Enums

        avsm_cluster = Clusters.CameraAvStreamManagement
        avsm_attr = avsm_cluster.Attributes
        avsm_commands = avsm_cluster.Commands

        self.step("1")
        zm_feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zm_cluster, attribute=zm_attr.FeatureMap
        )
        two_d_cart_supported = bool(zm_feature_map & zm_cluster.Bitmaps.Feature.kTwoDimensionalCartesianZone)
        user_defined_supported = bool(zm_feature_map & zm_cluster.Bitmaps.Feature.kUserDefined)
        focus_zones_supported = bool(zm_feature_map & zm_cluster.Bitmaps.Feature.kFocusZones)
        per_zone_sense_supported = bool(zm_feature_map & zm_cluster.Bitmaps.Feature.kPerZoneSensitivity)

        asserts.assert_true(
            two_d_cart_supported and user_defined_supported,
            "TwoDCartesianZone and UserDefined features are required for TC_ZONEMGMT_Privacy",
        )

        two_d_max = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zm_cluster, attribute=zm_attr.TwoDCartesianMax
        )
        max_x = two_d_max.x
        max_y = two_d_max.y
        log.info("TwoDCartesianMax: (%d, %d)", max_x, max_y)
        asserts.assert_greater(max_x, 0, "TwoDCartesianMax.x must be > 0")
        asserts.assert_greater(max_y, 0, "TwoDCartesianMax.y must be > 0")

        # Clean up any pre-existing triggers and user-defined zones
        existing_triggers = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zm_cluster, attribute=zm_attr.Triggers
        )
        for trig in existing_triggers:
            with contextlib.suppress(InteractionModelError):
                await self.send_single_cmd(endpoint=endpoint, cmd=zm_commands.RemoveTrigger(zoneID=trig.zoneID))

        existing_zones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zm_cluster, attribute=zm_attr.Zones
        )
        for z in existing_zones:
            if z.zoneSource == zm_enums.ZoneSourceEnum.kUser:
                with contextlib.suppress(InteractionModelError):
                    await self.send_single_cmd(endpoint=endpoint, cmd=zm_commands.RemoveZone(zoneID=z.zoneID))

        avsm_feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=avsm_cluster, attribute=avsm_attr.FeatureMap
        )
        asserts.assert_true(
            bool(avsm_feature_map & avsm_cluster.Bitmaps.Feature.kSnapshot),
            "Snapshot feature is required on CameraAVStreamManagement",
        )
        watermark_supported = bool(avsm_feature_map & avsm_cluster.Bitmaps.Feature.kWatermark)
        osd_supported = bool(avsm_feature_map & avsm_cluster.Bitmaps.Feature.kOnScreenDisplay)

        snapshot_caps = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=avsm_cluster, attribute=avsm_attr.SnapshotCapabilities
        )
        asserts.assert_true(len(snapshot_caps) > 0, "SnapshotCapabilities must not be empty")

        stream_id = None
        selected_cap = None
        for cap in snapshot_caps:
            try:
                alloc_args = {
                    "imageCodec": cap.imageCodec,
                    "maxFrameRate": cap.maxFrameRate,
                    "minResolution": cap.resolution,
                    "maxResolution": cap.resolution,
                    "quality": 90,
                }
                if watermark_supported:
                    alloc_args["watermarkEnabled"] = False
                if osd_supported:
                    alloc_args["OSDEnabled"] = False
                resp = await self.send_single_cmd(
                    endpoint=endpoint, cmd=avsm_commands.SnapshotStreamAllocate(**alloc_args)
                )
                stream_id = resp.snapshotStreamID
                selected_cap = cap
                break
            except InteractionModelError as e:
                log.warning("SnapshotStreamAllocate failed for capability %s: %s", cap, e)

        asserts.assert_is_not_none(stream_id, "Failed to allocate snapshot stream")
        snap_w = selected_cap.resolution.width
        snap_h = selected_cap.resolution.height
        log.info("Allocated snapshot stream ID %d at resolution %dx%d", stream_id, snap_w, snap_h)

        # Define test regions in pixel coordinates (for 640x480 or scaled proportionally for other resolutions)
        # Region A: top-left quadrant [80..260, 80..260] (for 640x480)
        # Region B: bottom-right quadrant [360..540, 200..380] (for 640x480)
        # Region C: top-right control region [360..540, 40..140] (for 640x480)
        ax0, ay0 = int(round(80 * snap_w / 640)), int(round(80 * snap_h / 480))
        ax1, ay1 = int(round(260 * snap_w / 640)), int(round(260 * snap_h / 480))
        bx0, by0 = int(round(360 * snap_w / 640)), int(round(200 * snap_h / 480))
        bx1, by1 = int(round(540 * snap_w / 640)), int(round(380 * snap_h / 480))
        cx0, cy0 = int(round(360 * snap_w / 640)), int(round(40 * snap_h / 480))
        cx1, cy1 = int(round(540 * snap_w / 640)), int(round(140 * snap_h / 480))
        inset = max(8, int(round(12 * snap_w / 640)))

        created_zone_ids = []

        try:
            self.step("2")
            baseline_rgb = await self._capture_and_decode_rgb(endpoint, stream_id, selected_cap.resolution)
            self._assert_region_is_non_black(baseline_rgb, ax0 + inset, ay0 + inset, ax1 - inset, ay1 - inset, "Baseline Region A")
            self._assert_region_is_non_black(baseline_rgb, bx0 + inset, by0 + inset, bx1 - inset, by1 - inset, "Baseline Region B")
            self._assert_region_is_non_black(baseline_rgb, cx0 + inset, cy0 + inset, cx1 - inset, cy1 - inset, "Baseline Region C")

            self.step("3")
            region_a_vertices = [
                self._pixel_to_zone_vertex(ax0, ay0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(ax1, ay0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(ax1, ay1, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(ax0, ay1, snap_w, snap_h, max_x, max_y),
            ]
            create_privacy_cmd = zm_commands.CreateTwoDCartesianZone(
                zone=zm_cluster.Structs.TwoDCartesianZoneStruct(
                    name="PrivacyZoneA",
                    use=zm_enums.ZoneUseEnum.kPrivacy,
                    vertices=region_a_vertices,
                    color="#000000",
                )
            )
            create_resp = await self.send_single_cmd(endpoint=endpoint, cmd=create_privacy_cmd)
            privacy_zone_id = create_resp.zoneID
            created_zone_ids.append(privacy_zone_id)
            log.info("Created Privacy Zone ID: %d", privacy_zone_id)

            zones_after_create = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=zm_cluster, attribute=zm_attr.Zones
            )
            matching_zone = next((z for z in zones_after_create if z.zoneID == privacy_zone_id), None)
            asserts.assert_is_not_none(matching_zone, f"Privacy zone {privacy_zone_id} not found in Zones attribute")
            asserts.assert_equal(matching_zone.zoneType, zm_enums.ZoneTypeEnum.kTwoDCARTZone, "Unexpected zoneType")
            asserts.assert_equal(matching_zone.zoneSource, zm_enums.ZoneSourceEnum.kUser, "Unexpected zoneSource")
            asserts.assert_equal(
                matching_zone.twoDCartesianZone.use, zm_enums.ZoneUseEnum.kPrivacy, "Expected ZoneUseEnum.kPrivacy"
            )

            masked_a_rgb = await self._capture_and_decode_rgb(endpoint, stream_id, selected_cap.resolution)
            self._assert_region_is_black(masked_a_rgb, ax0 + inset, ay0 + inset, ax1 - inset, ay1 - inset, "Masked Region A")
            self._assert_region_is_non_black(
                masked_a_rgb, bx0 + inset, by0 + inset, bx1 - inset, by1 - inset, "Unmasked Region B"
            )
            self._assert_region_is_non_black(
                masked_a_rgb, cx0 + inset, cy0 + inset, cx1 - inset, cy1 - inset, "Unmasked Control Region C"
            )

            self.step("4")
            sensitivity = None
            if per_zone_sense_supported:
                sensitivity_max = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=zm_cluster, attribute=zm_attr.SensitivityMax
                )
                sensitivity = min(4, sensitivity_max)
            privacy_trigger = zm_cluster.Structs.ZoneTriggerControlStruct(
                zoneID=privacy_zone_id,
                initialDuration=10,
                augmentationDuration=2,
                maxDuration=30,
                blindDuration=5,
                sensitivity=sensitivity,
            )
            try:
                await self.send_single_cmd(
                    endpoint=endpoint, cmd=zm_commands.CreateOrUpdateTrigger(trigger=privacy_trigger)
                )
                asserts.fail("Expected ConstraintError when creating a trigger on a Privacy Zone")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.ConstraintError,
                    "CreateOrUpdateTrigger on a Privacy Zone must fail with ConstraintError",
                )

            if not focus_zones_supported:
                try:
                    await self.send_single_cmd(
                        endpoint=endpoint,
                        cmd=zm_commands.UpdateTwoDCartesianZone(
                            zoneID=privacy_zone_id,
                            zone=zm_cluster.Structs.TwoDCartesianZoneStruct(
                                name="FocusInvalid",
                                use=zm_enums.ZoneUseEnum.kFocus,
                                vertices=region_a_vertices,
                                color="#000000",
                            ),
                        ),
                    )
                    asserts.fail("Expected ConstraintError when updating zone to Focus without FocusZones feature")
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status,
                        Status.ConstraintError,
                        "UpdateTwoDCartesianZone to Focus without FocusZones feature must fail with ConstraintError",
                    )

            # Create a temporary Motion zone, attach an active trigger, and verify updating it to Privacy fails with InvalidInState
            motion_vertices = [
                self._pixel_to_zone_vertex(cx0, cy0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(cx1, cy0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(cx1, cy1, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(cx0, cy1, snap_w, snap_h, max_x, max_y),
            ]
            motion_resp = await self.send_single_cmd(
                endpoint=endpoint,
                cmd=zm_commands.CreateTwoDCartesianZone(
                    zone=zm_cluster.Structs.TwoDCartesianZoneStruct(
                        name="TempMotionZone",
                        use=zm_enums.ZoneUseEnum.kMotion,
                        vertices=motion_vertices,
                        color="#00FFFF",
                    )
                ),
            )
            motion_zone_id = motion_resp.zoneID
            created_zone_ids.append(motion_zone_id)

            motion_trigger = zm_cluster.Structs.ZoneTriggerControlStruct(
                zoneID=motion_zone_id,
                initialDuration=10,
                augmentationDuration=2,
                maxDuration=30,
                blindDuration=5,
                sensitivity=sensitivity,
            )
            await self.send_single_cmd(
                endpoint=endpoint, cmd=zm_commands.CreateOrUpdateTrigger(trigger=motion_trigger)
            )
            try:
                await self.send_single_cmd(
                    endpoint=endpoint,
                    cmd=zm_commands.UpdateTwoDCartesianZone(
                        zoneID=motion_zone_id,
                        zone=zm_cluster.Structs.TwoDCartesianZoneStruct(
                            name="TempMotionToPrivacy",
                            use=zm_enums.ZoneUseEnum.kPrivacy,
                            vertices=motion_vertices,
                            color="#000000",
                        ),
                    ),
                )
                asserts.fail("Expected InvalidInState when updating a triggered Motion zone to Privacy")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.InvalidInState,
                    "Updating a triggered Motion zone to Privacy must fail with InvalidInState",
                )
            finally:
                await self.send_single_cmd(endpoint=endpoint, cmd=zm_commands.RemoveTrigger(zoneID=motion_zone_id))
                await self.send_single_cmd(endpoint=endpoint, cmd=zm_commands.RemoveZone(zoneID=motion_zone_id))
                created_zone_ids.remove(motion_zone_id)

            self.step("5")
            region_b_vertices = [
                self._pixel_to_zone_vertex(bx0, by0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(bx1, by0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(bx1, by1, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(bx0, by1, snap_w, snap_h, max_x, max_y),
            ]
            await self.send_single_cmd(
                endpoint=endpoint,
                cmd=zm_commands.UpdateTwoDCartesianZone(
                    zoneID=privacy_zone_id,
                    zone=zm_cluster.Structs.TwoDCartesianZoneStruct(
                        name="PrivacyZoneB",
                        use=zm_enums.ZoneUseEnum.kPrivacy,
                        vertices=region_b_vertices,
                        color="#000000",
                    ),
                ),
            )

            masked_b_rgb = await self._capture_and_decode_rgb(endpoint, stream_id, selected_cap.resolution)
            self._assert_region_is_black(masked_b_rgb, bx0 + inset, by0 + inset, bx1 - inset, by1 - inset, "Masked Region B")
            self._assert_region_is_non_black(
                masked_b_rgb, ax0 + inset, ay0 + inset, ax1 - inset, ay1 - inset, "Restored Region A after Update"
            )
            self._assert_region_is_non_black(
                masked_b_rgb, cx0 + inset, cy0 + inset, cx1 - inset, cy1 - inset, "Unmasked Control Region C"
            )

            self.step("6")
            # Create a 6-vertex concave L-shaped Privacy Zone over Region A with cutout in bottom-right [amid_x..ax1, amid_y..ay1]
            amid_x = (ax0 + ax1) // 2
            amid_y = (ay0 + ay1) // 2
            concave_l_vertices = [
                self._pixel_to_zone_vertex(ax0, ay0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(ax1, ay0, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(ax1, amid_y, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(amid_x, amid_y, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(amid_x, ay1, snap_w, snap_h, max_x, max_y),
                self._pixel_to_zone_vertex(ax0, ay1, snap_w, snap_h, max_x, max_y),
            ]
            create_l_resp = await self.send_single_cmd(
                endpoint=endpoint,
                cmd=zm_commands.CreateTwoDCartesianZone(
                    zone=zm_cluster.Structs.TwoDCartesianZoneStruct(
                        name="PrivacyConcaveL",
                        use=zm_enums.ZoneUseEnum.kPrivacy,
                        vertices=concave_l_vertices,
                        color="#000000",
                    )
                ),
            )
            privacy_zone_2_id = create_l_resp.zoneID
            created_zone_ids.append(privacy_zone_2_id)

            multi_rgb = await self._capture_and_decode_rgb(endpoint, stream_id, selected_cap.resolution)
            # Vertical arm of L-shape: [ax0 + inset .. amid_x - inset, ay0 + inset .. ay1 - inset]
            self._assert_region_is_black(
                multi_rgb, ax0 + inset, ay0 + inset, amid_x - inset, ay1 - inset, "L-shape Vertical Arm"
            )
            # Horizontal arm of L-shape: [ax0 + inset .. ax1 - inset, ay0 + inset .. amid_y - inset]
            self._assert_region_is_black(
                multi_rgb, ax0 + inset, ay0 + inset, ax1 - inset, amid_y - inset, "L-shape Horizontal Arm"
            )
            # Concave cutout of L-shape: [amid_x + inset .. ax1 - inset, amid_y + inset .. ay1 - inset] must remain unmasked
            self._assert_region_is_non_black(
                multi_rgb, amid_x + inset, amid_y + inset, ax1 - inset, ay1 - inset, "L-shape Concave Cutout"
            )
            # Region B (first privacy zone) must also remain black
            self._assert_region_is_black(
                multi_rgb, bx0 + inset, by0 + inset, bx1 - inset, by1 - inset, "Simultaneous Masked Region B"
            )

            self.step("7")
            for zid in list(created_zone_ids):
                await self.send_single_cmd(endpoint=endpoint, cmd=zm_commands.RemoveZone(zoneID=zid))
                created_zone_ids.remove(zid)

            restored_rgb = await self._capture_and_decode_rgb(endpoint, stream_id, selected_cap.resolution)
            self._assert_region_is_non_black(
                restored_rgb, ax0 + inset, ay0 + inset, ax1 - inset, ay1 - inset, "Restored Region A after RemoveZone"
            )
            self._assert_region_is_non_black(
                restored_rgb, bx0 + inset, by0 + inset, bx1 - inset, by1 - inset, "Restored Region B after RemoveZone"
            )
            self._assert_region_is_non_black(
                restored_rgb, cx0 + inset, cy0 + inset, cx1 - inset, cy1 - inset, "Control Region C after RemoveZone"
            )
        finally:
            for zid in list(created_zone_ids):
                with contextlib.suppress(InteractionModelError):
                    await self.send_single_cmd(endpoint=endpoint, cmd=zm_commands.RemoveZone(zoneID=zid))
            if stream_id is not None:
                with contextlib.suppress(InteractionModelError):
                    await self.send_single_cmd(
                        endpoint=endpoint,
                        cmd=avsm_commands.SnapshotStreamDeallocate(snapshotStreamID=stream_id),
                    )


if __name__ == "__main__":
    default_matter_test_main()
