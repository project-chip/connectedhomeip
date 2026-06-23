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

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import InteractionModelError
from matter.testing.matter_testing import MatterBaseTest, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVSM_Snapshot_Simple(MatterBaseTest):
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

        # Read Capabilities
        caps = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.SnapshotCapabilities
        )
        if not caps:
            asserts.skip("SnapshotCapabilities list is empty, skipping snapshot test")

        # Allocate Snapshot Stream
        try:
            allocate_cmd = commands.SnapshotStreamAllocate(
                imageCodec=caps[0].imageCodec,
                maxFrameRate=caps[0].maxFrameRate,
                minResolution=caps[0].resolution,
                maxResolution=caps[0].resolution,
                quality=90
            )
            response = await self.send_single_cmd(endpoint=endpoint, cmd=allocate_cmd)
            stream_id = response.snapshotStreamID
            log.info("Allocated snapshot stream ID: %s", stream_id)
        except InteractionModelError as e:
            asserts.fail(f"Failed to allocate snapshot stream: {e}")

        # Capture Snapshot
        try:
            capture_cmd = commands.CaptureSnapshot(
                snapshotStreamID=stream_id,
                requestedResolution=caps[0].resolution
            )
            capture_response = await self.send_single_cmd(
                cmd=capture_cmd,
                endpoint=endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD
            )
            log.info("CaptureSnapshotResponse resolution: %s", capture_response.resolution)
            log.info("CaptureSnapshotResponse codec: %s", capture_response.imageCodec)
            log.info("CaptureSnapshotResponse data size: %d", len(capture_response.data))
            asserts.assert_greater(len(capture_response.data), 0, "Captured image data is empty")
        except InteractionModelError as e:
            asserts.fail(f"CaptureSnapshot failed: {e}")
        finally:
            # Deallocate Stream
            if 'stream_id' in locals():
                try:
                    await self.send_single_cmd(
                        endpoint=endpoint,
                        cmd=commands.SnapshotStreamDeallocate(snapshotStreamID=stream_id)
                    )
                except Exception as e:
                    log.warning("Failed to deallocate snapshot stream %s: %s", stream_id, e)


if __name__ == "__main__":
    default_matter_test_main()
