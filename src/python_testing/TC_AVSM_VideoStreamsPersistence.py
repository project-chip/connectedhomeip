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
from matter.interaction_model import InteractionModelError, Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches

log = logging.getLogger(__name__)


class TC_AVSM_VideoStreamsPersistence(MatterBaseTest):
    def desc_TC_AVSM_VideoStreamsPersistence(self) -> str:
        return "[TC-AVSM-VideoStreamsPersistence] Validate Video Streams Persistence functionality with Server as DUT"

    def pics_TC_AVSM_VideoStreamsPersistence(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_VideoStreamsPersistence(self) -> list[TestStep]:
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
                "DUT responds with VideoStreamAllocateResponse command with a valid VideoStreamID.",
            ),
            TestStep(
                11,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the number of allocated video streams in the list is 1.",
            ),
            TestStep(
                12,
                "TH injects kFault_ClearInMemoryAllocatedVideoStreams on DUT.",
            ),
            TestStep(
                13,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the number of allocated video streams in the list is 0.",
            ),
            TestStep(
                14,
                "TH injects kFault_LoadPersistentCameraAVSMAttributes on DUT.",
            ),
            TestStep(
                15,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the number of allocated video streams in the list is 1.",
                "Verify the individual fields of allocated video stream is as was allocated in step 10.",
            ),
        ]

    @run_if_endpoint_matches(
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kVideo)
    )
    async def test_TC_AVSM_VideoStreamsPersistence(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done

        self.step(1)
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        log.info(f"Rx'd FeatureMap: {aFeatureMap}")
        vdoSupport = aFeatureMap & cluster.Bitmaps.Feature.kVideo
        asserts.assert_equal(vdoSupport, cluster.Bitmaps.Feature.kVideo, "Video Feature is not supported.")

        self.step(2)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        log.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 0, "The number of allocated video streams in the list is not 0")

        self.step(3)
        aStreamUsagePriorities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities
        )
        log.info(f"Rx'd StreamUsagePriorities: {aStreamUsagePriorities}")

        self.step(4)
        aRateDistortionTradeOffPoints = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.RateDistortionTradeOffPoints
        )
        log.info(f"Rx'd RateDistortionTradeOffPoints: {aRateDistortionTradeOffPoints}")

        self.step(5)
        aMinViewportRes = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.MinViewportResolution
        )
        log.info(f"Rx'd MinViewportResolution: {aMinViewportRes}")

        self.step(6)
        aVideoSensorParams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.VideoSensorParams
        )
        log.info(f"Rx'd VideoSensorParams: {aVideoSensorParams}")

        self.step(7)
        aMaxEncodedPixelRate = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.MaxEncodedPixelRate
        )
        log.info(f"Rx'd MaxEncodedPixelRate: {aMaxEncodedPixelRate}")

        # Check for watermark and OSD features
        self.step(8)
        watermark = True if (aFeatureMap & cluster.Bitmaps.Feature.kWatermark) != 0 else None

        self.step(9)
        osd = True if (aFeatureMap & cluster.Bitmaps.Feature.kOnScreenDisplay) != 0 else None

        self.step(10)
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
            log.info(f"Rx'd VideoStreamAllocateResponse: {videoStreamAllocateResponse}")
            asserts.assert_is_not_none(
                videoStreamAllocateResponse.videoStreamID, "VideoStreamAllocateResponse does not contain StreamID"
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        self.step(11)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        log.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 1, "The number of allocated video streams in the list is not 1")

        self.step(12)
        log.info("Injecting kFault_ClearInMemoryAllocatedVideoStreams on DUT")

        # --- Fault‑Injection cluster (mfg‑specific 0xFFF1_FC06) ---
        # Use FailAtFault to activate the chip‑layer fault exactly once
        #
        #  • faultType = kChipFault (0x03)  – always used for CHIP faults
        #  • id        = FaultInjection.Id.kFault_ClearInMemoryAllocatedVideoStreams
        #  • numCallsToSkip = 0  – trigger on the very next call
        #  • numCallsToFail = 1  – inject once, then auto‑clear
        #  • takeMutex      = False  – single‑threaded app, no lock needed
        #
        command = Clusters.FaultInjection.Commands.FailAtFault(
            type=Clusters.FaultInjection.Enums.FaultType.kChipFault,
            id=34,  # kFault_ClearInMemoryAllocatedVideoStreams
            numCallsToFail=1,
            takeMutex=False,
        )
        await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=0,  # Fault‑Injection cluster lives on EP0
            payload=command,
        )

        self.step(13)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        log.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 0, "Allocated video streams is not empty")

        self.step(14)
        log.info("Injecting kFault_LoadPersistentCameraAVSMAttributes on DUT")
        # --- Fault‑Injection cluster (mfg‑specific 0xFFF1_FC06) ---
        # Use FailAtFault to activate the chip‑layer fault exactly once
        #
        #  • faultType = kChipFault (0x03)  – always used for CHIP faults
        #  • id        = FaultInjection.Id.kFault_LoadPersistentCameraAVSMAttributes
        #  • numCallsToSkip = 0  – trigger on the very next call
        #  • numCallsToFail = 1  – inject once, then auto‑clear
        #  • takeMutex      = False  – single‑threaded app, no lock needed
        #
        command = Clusters.FaultInjection.Commands.FailAtFault(
            type=Clusters.FaultInjection.Enums.FaultType.kChipFault,
            id=37,  # kFault_LoadPersistentCameraAVSMAttributes
            numCallsToFail=1,
            takeMutex=False,
        )
        await self.default_controller.SendCommand(
            nodeId=self.dut_node_id,
            endpoint=0,  # Fault‑Injection cluster lives on EP0
            payload=command,
        )

        self.step(15)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        log.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 1, "Allocated video streams is not empty")

        ##### Validate fields in Video Stream that was stored #####
        asserts.assert_equal(aAllocatedVideoStreams[0].streamUsage, aStreamUsagePriorities[0], "Stream Usage does not match")
        asserts.assert_equal(aAllocatedVideoStreams[0].videoCodec,
                             aRateDistortionTradeOffPoints[0].codec, "Video codec does not match")
        asserts.assert_equal(aAllocatedVideoStreams[0].minFrameRate, 30, "MinFrameRate does not match")
        asserts.assert_equal(aAllocatedVideoStreams[0].maxFrameRate, aVideoSensorParams.maxFPS, "MaxFrameRate does not match")
        asserts.assert_equal(aAllocatedVideoStreams[0].minResolution, aMinViewportRes, "MinResolution does not match")
        asserts.assert_equal(aAllocatedVideoStreams[0].maxResolution.width,
                             aVideoSensorParams.sensorWidth, "MaxResolution does not match")
        asserts.assert_equal(aAllocatedVideoStreams[0].maxResolution.height,
                             aVideoSensorParams.sensorHeight, "MaxResolution does not match")
        asserts.assert_equal(aAllocatedVideoStreams[0].minBitRate,
                             aRateDistortionTradeOffPoints[0].minBitRate, "MinBitRate does not match")
        asserts.assert_equal(aAllocatedVideoStreams[0].maxBitRate,
                             aRateDistortionTradeOffPoints[0].minBitRate, "MaxBitRate does not match")
        asserts.assert_equal(aAllocatedVideoStreams[0].keyFrameInterval, 4000, "KeyFrameInterval does not match")

        # Clear all allocated streams
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )

        for stream in aAllocatedVideoStreams:
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=commands.VideoStreamDeallocate(videoStreamID=(stream.videoStreamID)))
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")


if __name__ == "__main__":
    default_matter_test_main()
