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

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_AVSM_2_1(MatterBaseTest):
    def desc_TC_AVSM_2_1(self) -> str:
        return "[TC-AVSM-2.1] Attributes with Server as DUT"

    def pics_TC_AVSM_2_1(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads MaxConcurrentEncoders attribute.", "Verify that the DUT response contains an uint8 value."),
            TestStep(3, "TH reads MaxEncodedPixelRate attribute.", "Verify that the DUT response contains an uint32 value."),
            TestStep(
                4, "TH reads VideoSensorParams attribute.", "Verify that the DUT response contains a VideoSensorParamsStruct value."
            ),
            TestStep(5, "TH reads NightVisionUsesInfrared attribute.", "Verify that the DUT response contains a bool value."),
            TestStep(6, "TH reads MinViewportResolution attribute.",
                     "Verify that the DUT response contains a VideoResolutionStruct value."),
            TestStep(
                7,
                "TH reads RateDistortionTradeOffPoints attribute.",
                "Verify that the DUT response contains a list of RateDistortionTradeOffPointsStruct entries.",
            ),
            TestStep(8, "TH reads MaxContentBufferSize attribute.", "Verify that the DUT response contains an uint32 value."),
            TestStep(
                9,
                "TH reads MicrophoneCapabilities attribute.",
                "Verify that the DUT response contains an AudioCapabilitiesStruct value.",
            ),
            TestStep(
                10,
                "TH reads SpeakerCapabilities attribute.",
                "Verify that the DUT response contains an AudioCapabilitiesStruct value.",
            ),
            TestStep(
                11,
                "TH reads TwoWayTalkSupport attribute.",
                "Verify that the DUT response contains a TwoWayTalkSupportTypeEnum value.",
            ),
            TestStep(
                12,
                "TH reads SnapshotCapabilities attribute.",
                "Verify that the DUT response contains a list of SnapshotCapabilitiesStruct entries.",
            ),
            TestStep(13, "TH reads MaxNetworkBandwidth attribute.", "Verify that the DUT response contains an uint32 value."),
            TestStep(14, "TH reads CurrentFrameRate attribute.", "Verify that the DUT response contains an uint16 value."),
            TestStep(15, "TH reads HDRModeEnabled attribute.", "Verify that the DUT response contains a bool value."),
            TestStep(
                16,
                "TH reads SupportedStreamUsages attribute.",
                "Verify that the DUT response contains a list of StreamUsageEnum entries.",
            ),
            TestStep(
                17,
                "TH reads AllocatedVideoStreams attribute.",
                "Verify that the DUT response contains a list of VideoStreamStruct entries.",
            ),
            TestStep(
                18,
                "TH reads AllocatedAudioStreams attribute.",
                "Verify that the DUT response contains a list of AudioStreamStruct entries.",
            ),
            TestStep(
                19,
                "TH reads AllocatedSnapshotStreams attribute.",
                "Verify that the DUT response contains a list of SnapshotStreamStruct entries.",
            ),
            TestStep(
                20,
                "TH reads StreamUsagePriorities attribute.",
                "Verify that the DUT response contains a list of StreamUsageEnum entries.",
            ),
            TestStep(
                21, "TH reads SoftRecordingPrivacyModeEnabled attribute.", "Verify that the DUT response contains a bool value."
            ),
            TestStep(
                22, "TH reads SoftLivestreamPrivacyModeEnabled attribute.", "Verify that the DUT response contains a bool value."
            ),
            TestStep(23, "TH reads HardPrivacyModeOn attribute.", "Verify that the DUT response contains a bool value."),
            TestStep(24, "TH reads NightVision attribute.", "Verify that the DUT response contains a TriStateAutoEnum value."),
            TestStep(25, "TH reads NightVisionIllum attribute.", "Verify that the DUT response contains a TriStateAutoEnum value."),
            TestStep(26, "TH reads Viewport attribute.", "Verify that the DUT response contains a ViewportStruct value."),
            TestStep(27, "TH reads SpeakerMuted attribute.", "Verify that the DUT response contains a bool value."),
            TestStep(28, "TH reads SpeakerVolumeLevel attribute.", "Verify that the DUT response contains an uint8 value."),
            TestStep(29, "TH reads SpeakerMaxLevel attribute.", "Verify that the DUT response contains an uint8 value."),
            TestStep(30, "TH reads SpeakerMinLevel attribute.", "Verify that the DUT response contains an uint8 value."),
            TestStep(31, "TH reads MicrophoneMuted attribute.", "Verify that the DUT response contains a bool value."),
            TestStep(32, "TH reads MicrophoneVolumeLevel attribute.", "Verify that the DUT response contains an uint8 value."),
            TestStep(33, "TH reads MicrophoneMaxLevel attribute.", "Verify that the DUT response contains an uint8 value."),
            TestStep(34, "TH reads MicrophoneMinLevel attribute.", "Verify that the DUT response contains an uint8 value."),
            TestStep(35, "TH reads MicrophoneAGCEnabled attribute.", "Verify that the DUT response contains a bool value."),
            TestStep(36, "TH reads ImageRotation attribute.", "Verify that the DUT response contains an uint16 value."),
            TestStep(37, "TH reads ImageFlipHorizontal attribute.", "Verify that the DUT response contains a bool value."),
            TestStep(38, "TH reads ImageFlipVertical attribute.", "Verify that the DUT response contains a bool value."),
            TestStep(39, "TH reads LocalVideoRecordingEnabled attribute.", "Verify that the DUT response contains a bool value."),
            TestStep(
                40, "TH reads LocalSnapshotRecordingEnabled attribute.", "Verify that the DUT response contains a bool value."
            ),
            TestStep(41, "TH reads StatusLightEnabled attribute.", "Verify that the DUT response contains a bool value."),
            TestStep(
                42, "TH reads StatusLightBrightness attribute.", "Verify that the DUT response contains a ThreeLevelAutoEnum value."
            ),
        ]

    @async_test_body
    async def test_TC_AVSM_2_1(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes

        self.step(1)
        # Commission DUT - already done

        self.step(2)
        if self.pics_guard(self.check_pics("AVSM.S.A0000")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MaxConcurrentEncoders
            )
            logger.info(f"Rx'd MaxConcurrentEncoders: {value}")

        self.step(3)
        if self.pics_guard(self.check_pics("AVSM.S.A0001")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MaxEncodedPixelRate
            )
            logger.info(f"Rx'd MaxEncodedPixelRate: {value}")

        self.step(4)
        if self.pics_guard(self.check_pics("AVSM.S.A0002")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.VideoSensorParams
            )
            logger.info(f"Rx'd VideoSensorParams: {value}")

        self.step(5)
        if self.pics_guard(self.check_pics("AVSM.S.A0003")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.NightVisionUsesInfrared
            )
            logger.info(f"Rx'd NightVisionUsesInfrared: {value}")

        self.step(6)
        if self.pics_guard(self.check_pics("AVSM.S.A0004")):
            value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.MinViewportResolution)
            logger.info(f"Rx'd MinViewportResolution: {value}")

        self.step(7)
        if self.pics_guard(self.check_pics("AVSM.S.A0005")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.RateDistortionTradeOffPoints
            )
            logger.info(f"Rx'd RateDistortionTradeOffPoints: {value}")

        self.step(8)
        if self.pics_guard(self.check_pics("AVSM.S.A0006")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MaxContentBufferSize
            )
            logger.info(f"Rx'd MaxContentBufferSize: {value}")

        self.step(9)
        if self.pics_guard(self.check_pics("AVSM.S.A0007")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneCapabilities
            )
            logger.info(f"Rx'd MicrophoneCapabilities: {value}")

        self.step(10)
        if self.pics_guard(self.check_pics("AVSM.S.A0008")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerCapabilities
            )
            logger.info(f"Rx'd SpeakerCapabilities: {value}")

        self.step(11)
        if self.pics_guard(self.check_pics("AVSM.S.A0009")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.TwoWayTalkSupport
            )
            logger.info(f"Rx'd TwoWayTalkSupport: {value}")

        self.step(12)
        if self.pics_guard(self.check_pics("AVSM.S.A000A")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SnapshotCapabilities
            )
            logger.info(f"Rx'd SnapshotCapabilities: {value}")

        self.step(13)
        if self.pics_guard(self.check_pics("AVSM.S.A000B")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MaxNetworkBandwidth
            )
            logger.info(f"Rx'd MaxNetworkBandwidth: {value}")

        self.step(14)
        if self.pics_guard(self.check_pics("AVSM.S.A000C")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.CurrentFrameRate
            )
            logger.info(f"Rx'd CurrentFrameRate: {value}")

        self.step(15)
        if self.pics_guard(self.check_pics("AVSM.S.A000D")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.HDRModeEnabled
            )
            logger.info(f"Rx'd HDRModeEnabled: {value}")

        self.step(16)
        if self.pics_guard(self.check_pics("AVSM.S.A000E")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SupportedStreamUsages
            )
            logger.info(f"Rx'd SupportedStreamUsages: {value}")

        self.step(17)
        if self.pics_guard(self.check_pics("AVSM.S.A000F")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
            )
            logger.info(f"Rx'd AllocatedVideoStreams: {value}")

        self.step(18)
        if self.pics_guard(self.check_pics("AVSM.S.A0010")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
            )
            logger.info(f"Rx'd AllocatedAudioStreams: {value}")

        self.step(19)
        if self.pics_guard(self.check_pics("AVSM.S.A0011")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams
            )
            logger.info(f"Rx'd AllocatedSnapshotStreams: {value}")

        self.step(20)
        if self.pics_guard(self.check_pics("AVSM.S.A0012")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities
            )
            logger.info(f"Rx'd StreamUsagePrioritiesList: {value}")

        self.step(21)
        if self.pics_guard(self.check_pics("AVSM.S.A0013")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SoftRecordingPrivacyModeEnabled
            )
            logger.info(f"Rx'd SoftRecordingPrivacyModeEnabled: {value}")

        self.step(22)
        if self.pics_guard(self.check_pics("AVSM.S.A0014")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SoftLivestreamPrivacyModeEnabled
            )
            logger.info(f"Rx'd SoftLivestreamPrivacyModeEnabled: {value}")

        self.step(23)
        if self.pics_guard(self.check_pics("AVSM.S.A0015")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.HardPrivacyModeOn
            )
            logger.info(f"Rx'd HardPrivacyModeOn: {value}")

        self.step(24)
        if self.pics_guard(self.check_pics("AVSM.S.A0016")):
            value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.NightVision)
            logger.info(f"Rx'd NightVision: {value}")

        self.step(25)
        if self.pics_guard(self.check_pics("AVSM.S.A0017")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.NightVisionIllum
            )
            logger.info(f"Rx'd NightVisionIllum: {value}")

        self.step(26)
        if self.pics_guard(self.check_pics("AVSM.S.A0018")):
            value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.Viewport)
            logger.info(f"Rx'd Viewport: {value}")

        self.step(27)
        if self.pics_guard(self.check_pics("AVSM.S.A0019")):
            value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerMuted)
            logger.info(f"Rx'd SpeakerMuted: {value}")

        self.step(28)
        if self.pics_guard(self.check_pics("AVSM.S.A001A")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerVolumeLevel
            )
            logger.info(f"Rx'd SpeakerVolumeLevel: {value}")

        self.step(29)
        if self.pics_guard(self.check_pics("AVSM.S.A001B")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerMaxLevel
            )
            logger.info(f"Rx'd SpeakerMaxLevel: {value}")

        self.step(30)
        if self.pics_guard(self.check_pics("AVSM.S.A001C")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerMinLevel
            )
            logger.info(f"Rx'd SpeakerMinLevel: {value}")

        self.step(31)
        if self.pics_guard(self.check_pics("AVSM.S.A001D")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneMuted
            )
            logger.info(f"Rx'd MicrophoneMuted: {value}")

        self.step(32)
        if self.pics_guard(self.check_pics("AVSM.S.A001E")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneVolumeLevel
            )
            logger.info(f"Rx'd MicrophoneVolumeLevel: {value}")

        self.step(33)
        if self.pics_guard(self.check_pics("AVSM.S.A001F")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneMaxLevel
            )
            logger.info(f"Rx'd MicrophoneMaxLevel: {value}")

        self.step(34)
        if self.pics_guard(self.check_pics("AVSM.S.A0020")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneMinLevel
            )
            logger.info(f"Rx'd MicrophoneMinLevel: {value}")

        self.step(35)
        if self.pics_guard(self.check_pics("AVSM.S.A0021")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneAGCEnabled
            )
            logger.info(f"Rx'd MicrophoneAGCEnabled: {value}")

        self.step(36)
        if self.pics_guard(self.check_pics("AVSM.S.A0022")):
            value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.ImageRotation)
            logger.info(f"Rx'd ImageRotation: {value}")

        self.step(37)
        if self.pics_guard(self.check_pics("AVSM.S.A0023")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.ImageFlipHorizontal
            )
            logger.info(f"Rx'd ImageFlipHorizontal: {value}")

        self.step(38)
        if self.pics_guard(self.check_pics("AVSM.S.A0024")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.ImageFlipVertical
            )
            logger.info(f"Rx'd ImageFlipVertical: {value}")

        self.step(39)
        if self.pics_guard(self.check_pics("AVSM.S.A0025")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.LocalVideoRecordingEnabled
            )
            logger.info(f"Rx'd LocalVideoRecordingEnabled: {value}")

        self.step(40)
        if self.pics_guard(self.check_pics("AVSM.S.A0026")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.LocalSnapshotRecordingEnabled
            )
            logger.info(f"Rx'd LocalSnapshotRecordingEnabled: {value}")

        self.step(41)
        if self.pics_guard(self.check_pics("AVSM.S.A0027")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.StatusLightEnabled
            )
            logger.info(f"Rx'd StatusLightEnabled: {value}")

        self.step(42)
        if self.pics_guard(self.check_pics("AVSM.S.A0028")):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.StatusLightBrightness
            )
            logger.info(f"Rx'd StatusLightBrightness: {value}")


if __name__ == "__main__":
    default_matter_test_main()
