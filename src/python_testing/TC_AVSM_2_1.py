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
#     app: ${camera}
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
from matter.testing import matter_asserts
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

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_AVSM_2_1(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes

        self.step(1)
        # Commission DUT - already done

        self.step(2)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.MaxConcurrentEncoders):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MaxConcurrentEncoders
            )
            logger.info(f"Rx'd MaxConcurrentEncoders: {value}")
            asserts.assert_is_not_none(value, "MaxConcurrentEncoders is None")
            matter_asserts.assert_valid_uint8(value, "MaxConcurrentEncoders")

        self.step(3)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.MaxEncodedPixelRate):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MaxEncodedPixelRate
            )
            logger.info(f"Rx'd MaxEncodedPixelRate: {value}")
            asserts.assert_is_not_none(value, "MaxEncodedPixelRate is None")
            matter_asserts.assert_valid_uint32(value, "MaxEncodedPixelRate")

        self.step(4)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.VideoSensorParams):
            videoSensorParams = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.VideoSensorParams
            )
            logger.info(f"Rx'd VideoSensorParams: {videoSensorParams}")
            asserts.assert_is_not_none(videoSensorParams, "VideoSensorParams is None")
            self.assert_video_sensor_params_struct(videoSensorParams)

        self.step(5)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.NightVisionUsesInfrared):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.NightVisionUsesInfrared
            )
            logger.info(f"Rx'd NightVisionUsesInfrared: {value}")
            asserts.assert_is_not_none(value, "NightVisionUsesInfrared is None")

        self.step(6)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.MinViewportResolution):
            minViewportResolution = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MinViewportResolution
            )
            logger.info(f"Rx'd MinViewportResolution: {minViewportResolution}")
            asserts.assert_is_not_none(minViewportResolution, "MinViewportResolution is None")
            self.assert_video_resolution_struct(minViewportResolution)

        self.step(7)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.RateDistortionTradeOffPoints):
            rateDistortionTradeOffPoints = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.RateDistortionTradeOffPoints
            )
            logger.info(f"Rx'd RateDistortionTradeOffPoints: {rateDistortionTradeOffPoints}")
            asserts.assert_is_not_none(rateDistortionTradeOffPoints, "RateDistortionTradeOffPoints is None")
            matter_asserts.assert_all(
                rateDistortionTradeOffPoints,
                lambda x: self.assert_rate_distortion_trade_off_point_struct(x),
                "RateDistortionTradeOffPoints has invalid entry",
            )

        self.step(8)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.MaxContentBufferSize):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MaxContentBufferSize
            )
            logger.info(f"Rx'd MaxContentBufferSize: {value}")
            matter_asserts.assert_valid_uint32(value, "MaxContentBufferSize")

        self.step(9)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.MicrophoneCapabilities):
            microphoneCapabilities = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneCapabilities
            )
            logger.info(f"Rx'd MicrophoneCapabilities: {microphoneCapabilities}")
            asserts.assert_is_not_none(microphoneCapabilities, "MicrophoneCapabilities is None")
            self.assert_audio_capabilities_struct(microphoneCapabilities)

        self.step(10)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.SpeakerCapabilities):
            speakerCapabilities = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerCapabilities
            )
            logger.info(f"Rx'd SpeakerCapabilities: {speakerCapabilities}")
            asserts.assert_is_not_none(speakerCapabilities, "SpeakerCapabilities is None")
            self.assert_audio_capabilities_struct(speakerCapabilities)

        self.step(11)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.TwoWayTalkSupport):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.TwoWayTalkSupport
            )
            logger.info(f"Rx'd TwoWayTalkSupport: {value}")
            asserts.assert_is_not_none(value, "TwoWayTalkSupport is None")
            asserts.assert_in(
                value,
                cluster.Enums.TwoWayTalkSupportTypeEnum,
                "TwoWayTalkSupport is not a valid TwoWayTalkSupportTypeEnum",
            )

        self.step(12)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.SnapshotCapabilities):
            snapshotCapabilities = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SnapshotCapabilities
            )
            logger.info(f"Rx'd SnapshotCapabilities: {snapshotCapabilities}")
            asserts.assert_is_not_none(snapshotCapabilities, "SnapshotCapabilities is None")
            matter_asserts.assert_all(
                snapshotCapabilities,
                lambda x: self.assert_snapshot_capabilities_struct(x),
                "SnapshotCapabilities has invalid entry",
            )

        self.step(13)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.MaxNetworkBandwidth):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MaxNetworkBandwidth
            )
            logger.info(f"Rx'd MaxNetworkBandwidth: {value}")
            matter_asserts.assert_valid_uint32(value, "MaxNetworkBandwidth")

        self.step(14)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.CurrentFrameRate):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.CurrentFrameRate
            )
            logger.info(f"Rx'd CurrentFrameRate: {value}")
            asserts.assert_is_not_none(value, "CurrentFrameRate is None")
            matter_asserts.assert_valid_uint16(value, "CurrentFrameRate")

        self.step(15)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.HDRModeEnabled):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.HDRModeEnabled
            )
            logger.info(f"Rx'd HDRModeEnabled: {value}")
            asserts.assert_is_not_none(value, "HDRModeEnabled is None")

        self.step(16)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.SupportedStreamUsages):
            supportedStreamUsages = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SupportedStreamUsages
            )
            logger.info(f"Rx'd SupportedStreamUsages: {supportedStreamUsages}")
            matter_asserts.assert_all(
                supportedStreamUsages,
                lambda x: self.assert_enum(
                    x, Clusters.Globals.Enums.StreamUsageEnum, "SupportedStreamUsage not a valid StreamUsageEnum"
                ),
                "StreamUsage is not a valid StreamUsageEnum",
            )

        self.step(17)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.AllocatedVideoStreams):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
            )
            logger.info(f"Rx'd AllocatedVideoStreams: {value}")
            asserts.assert_is_not_none(value, "AllocatedVideoStreams is None")
            # TODO assert struct fields of list - currently list is empty as nothing allocated

        self.step(18)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.AllocatedAudioStreams):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
            )
            logger.info(f"Rx'd AllocatedAudioStreams: {value}")
            asserts.assert_is_not_none(value, "AllocatedAudioStreams is None")
            # TODO assert struct fields of list - currently list is empty as nothing allocated

        self.step(19)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.AllocatedSnapshotStreams):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams
            )
            logger.info(f"Rx'd AllocatedSnapshotStreams: {value}")
            asserts.assert_is_not_none(value, "AllocatedSnapshotStreams is None")
            # TODO assert struct fields of list - currently list is empty as nothing allocated

        self.step(20)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.StreamUsagePriorities):
            streamUsagePriorities = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities
            )
            logger.info(f"Rx'd StreamUsagePrioritiesList: {streamUsagePriorities}")
            matter_asserts.assert_all(
                streamUsagePriorities,
                lambda x: self.assert_enum(
                    x, Clusters.Globals.Enums.StreamUsageEnum, "StreamUsagePriority not a valid StreamUsageEnum"
                ),
                "StreamUsage is not a valid StreamUsageEnum",
            )

        self.step(21)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.SoftRecordingPrivacyModeEnabled):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SoftRecordingPrivacyModeEnabled
            )
            logger.info(f"Rx'd SoftRecordingPrivacyModeEnabled: {value}")
            asserts.assert_is_not_none(value, "SoftRecordingPrivacyModeEnabled is None")

        self.step(22)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.SoftLivestreamPrivacyModeEnabled):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SoftLivestreamPrivacyModeEnabled
            )
            logger.info(f"Rx'd SoftLivestreamPrivacyModeEnabled: {value}")
            asserts.assert_is_not_none(value, "SoftLivestreamPrivacyModeEnabled is None")

        self.step(23)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.HardPrivacyModeOn):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.HardPrivacyModeOn
            )
            logger.info(f"Rx'd HardPrivacyModeOn: {value}")
            asserts.assert_is_not_none(value, "HardPrivacyModeOn is None")

        self.step(24)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.NightVision):
            value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.NightVision)
            logger.info(f"Rx'd NightVision: {value}")
            asserts.assert_is_not_none(value, "NightVision is None")
            asserts.assert_in(
                value,
                cluster.Enums.TriStateAutoEnum,
                "NightVision is not a valid TriStateAutoEnum",
            )

        self.step(25)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.NightVisionIllum):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.NightVisionIllum
            )
            logger.info(f"Rx'd NightVisionIllum: {value}")
            asserts.assert_is_not_none(value, "NightVisionIllum is None")
            asserts.assert_in(
                value,
                cluster.Enums.TriStateAutoEnum,
                "NightVisionIllum is not a valid TriStateAutoEnum",
            )

        self.step(26)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.Viewport):
            viewport = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.Viewport)
            logger.info(f"Rx'd Viewport: {viewport}")
            asserts.assert_is_not_none(viewport, "Viewport is None")
            self.assert_viewport_struct(viewport)

        self.step(27)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.SpeakerMuted):
            value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerMuted)
            logger.info(f"Rx'd SpeakerMuted: {value}")
            asserts.assert_is_not_none(value, "SpeakerMuted is None")

        self.step(28)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.SpeakerVolumeLevel):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerVolumeLevel
            )
            logger.info(f"Rx'd SpeakerVolumeLevel: {value}")
            asserts.assert_is_not_none(value, "SpeakerVolumeLevel is None")
            matter_asserts.assert_valid_uint8(value, "SpeakerVolumeLevel")
            # TODO asserts depend on reading SpeakerMaxLevel and SpeakerMinLevel first

        self.step(29)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.SpeakerMaxLevel):
            speakerMaxLevel = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerMaxLevel
            )
            logger.info(f"Rx'd SpeakerMaxLevel: {speakerMaxLevel}")
            asserts.assert_is_not_none(speakerMaxLevel, "SpeakerMaxLevel is None")
            matter_asserts.assert_valid_uint8(speakerMaxLevel, "SpeakerMaxLevel")
            asserts.assert_less_equal(speakerMaxLevel, 254, "SpeakerMaxLevel is > 254")

        self.step(30)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.SpeakerMinLevel):
            speakerMinLevel = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerMinLevel
            )
            logger.info(f"Rx'd SpeakerMinLevel: {speakerMinLevel}")
            asserts.assert_is_not_none(speakerMinLevel, "SpeakerMinLevel is None")
            matter_asserts.assert_valid_uint8(speakerMinLevel, "SpeakerMinLevel")
            asserts.assert_less_equal(speakerMinLevel, speakerMaxLevel, "SpeakerMinLevel is > SpeakerMaxLevel")

        self.step(31)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.MicrophoneMuted):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneMuted
            )
            logger.info(f"Rx'd MicrophoneMuted: {value}")
            asserts.assert_is_not_none(value, "MicrophoneMuted is None")

        self.step(32)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.MicrophoneVolumeLevel):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneVolumeLevel
            )
            logger.info(f"Rx'd MicrophoneVolumeLevel: {value}")
            asserts.assert_is_not_none(value, "MicrophoneVolumeLevel is None")
            matter_asserts.assert_valid_uint8(value, "MicrophoneVolumeLevel")
            # TODO asserts depend on reading MicrophoneMaxLevel and MicrophoneMinLevel first

        self.step(33)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.MicrophoneMaxLevel):
            microphoneMaxLevel = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneMaxLevel
            )
            logger.info(f"Rx'd MicrophoneMaxLevel: {microphoneMaxLevel}")
            asserts.assert_is_not_none(microphoneMaxLevel, "MicrophoneMaxLevel is None")
            matter_asserts.assert_valid_uint8(microphoneMaxLevel, "MicrophoneMaxLevel")
            asserts.assert_less_equal(microphoneMaxLevel, 254, "MicrophoneMaxLevel is > 254")

        self.step(34)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.MicrophoneMinLevel):
            microphoneMinLevel = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneMinLevel
            )
            logger.info(f"Rx'd MicrophoneMinLevel: {microphoneMinLevel}")
            asserts.assert_is_not_none(microphoneMinLevel, "MicrophoneMinLevel is None")
            matter_asserts.assert_valid_uint8(microphoneMinLevel, "MicrophoneMinLevel")
            asserts.assert_less_equal(microphoneMinLevel, microphoneMaxLevel, "MicrophoneMinLevel is > MicrophoneMaxLevel")

        self.step(35)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.MicrophoneAGCEnabled):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneAGCEnabled
            )
            logger.info(f"Rx'd MicrophoneAGCEnabled: {value}")
            asserts.assert_is_not_none(value, "MicrophoneAGCEnabled is None")

        self.step(36)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.ImageRotation):
            value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.ImageRotation)
            logger.info(f"Rx'd ImageRotation: {value}")
            asserts.assert_is_not_none(value, "ImageRotation is None")
            matter_asserts.assert_valid_uint16(value, "ImageRotation")
            asserts.assert_less_equal(value, 359, "ImageRotation is > 359")

        self.step(37)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.ImageFlipHorizontal):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.ImageFlipHorizontal
            )
            logger.info(f"Rx'd ImageFlipHorizontal: {value}")
            asserts.assert_is_not_none(value, "ImageFlipHorizontal is None")

        self.step(38)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.ImageFlipVertical):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.ImageFlipVertical
            )
            logger.info(f"Rx'd ImageFlipVertical: {value}")
            asserts.assert_is_not_none(value, "ImageFlipVertical is None")

        self.step(39)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.LocalVideoRecordingEnabled):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.LocalVideoRecordingEnabled
            )
            logger.info(f"Rx'd LocalVideoRecordingEnabled: {value}")
            asserts.assert_is_not_none(value, "LocalVideoRecordingEnabled is None")

        self.step(40)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.LocalSnapshotRecordingEnabled):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.LocalSnapshotRecordingEnabled
            )
            logger.info(f"Rx'd LocalSnapshotRecordingEnabled: {value}")
            asserts.assert_is_not_none(value, "LocalSnapshotRecordingEnabled is None")

        self.step(41)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.StatusLightEnabled):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.StatusLightEnabled
            )
            logger.info(f"Rx'd StatusLightEnabled: {value}")
            asserts.assert_is_not_none(value, "StatusLightEnabled is None")

        self.step(42)
        if await self.attribute_guard(endpoint=endpoint, attribute=attr.StatusLightBrightness):
            value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.StatusLightBrightness
            )
            logger.info(f"Rx'd StatusLightBrightness: {value}")
            asserts.assert_is_not_none(value, "StatusLightBrightness is None")
            asserts.assert_in(
                value,
                Clusters.Globals.Enums.ThreeLevelAutoEnum,
                "StatusLightBrightness is not a valid ThreeLevelAutoEnum",
            )

    def assert_video_sensor_params_struct(
        self, videoSensorParams: Clusters.CameraAvStreamManagement.Structs.VideoSensorParamsStruct
    ) -> bool:
        asserts.assert_greater_equal(videoSensorParams.sensorWidth, 64, "SensorWidth is less than 64")
        asserts.assert_greater_equal(videoSensorParams.sensorHeight, 64, "SensorHeight is less than 64")
        asserts.assert_greater_equal(videoSensorParams.maxFPS, 1, "MaxFPS is less than 1")
        if videoSensorParams.maxHDRFPS is not None:
            matter_asserts.assert_int_in_range(
                videoSensorParams.maxHDRFPS,
                1,
                videoSensorParams.maxFPS,
                "MaxHDRFPS is not between 1 and MaxFPS",
            )
        return True

    def assert_video_resolution_struct(
        self, videoResolution: Clusters.CameraAvStreamManagement.Structs.VideoResolutionStruct
    ) -> bool:
        asserts.assert_greater_equal(videoResolution.width, 1, "Width is less than 1")
        asserts.assert_greater_equal(videoResolution.height, 1, "Height is less than 1")
        return True

    def assert_rate_distortion_trade_off_point_struct(
        self, rateDistortionTradeOffPoints: Clusters.CameraAvStreamManagement.Structs.RateDistortionTradeOffPointsStruct
    ) -> bool:
        asserts.assert_in(
            rateDistortionTradeOffPoints.codec,
            Clusters.CameraAvStreamManagement.Enums.VideoCodecEnum,
            "Codec is not a valid VideoCodecEnum",
        )
        self.assert_video_resolution_struct(rateDistortionTradeOffPoints.resolution)
        asserts.assert_greater_equal(rateDistortionTradeOffPoints.minBitRate, 1, "MinBitRate is less than 1")
        return True

    def assert_audio_capabilities_struct(
        self, audioCapabilities: Clusters.CameraAvStreamManagement.Structs.AudioCapabilitiesStruct
    ) -> bool:
        asserts.assert_greater_equal(audioCapabilities.maxNumberOfChannels, 1, "MaxNumberOfChannels is less than 1")
        asserts.assert_greater_equal(len(audioCapabilities.supportedCodecs), 1, "SupportedCodecs list is empty")
        matter_asserts.assert_all(
            audioCapabilities.supportedCodecs,
            lambda x: self.assert_enum(
                x, Clusters.CameraAvStreamManagement.Enums.AudioCodecEnum, "SupportedCodec not a valid AudioCodecEnum"
            ),
            "SupportedCodec contains an entry that is not a valid AudioCodecEnum",
        )
        asserts.assert_greater_equal(len(audioCapabilities.supportedSampleRates), 1, "SupportedSampleRates list is empty")
        asserts.assert_greater_equal(len(audioCapabilities.supportedBitDepths), 1, "SupportedBitDepths list is empty")
        # TODO assert supportedSampleRates and supportedBitDepths list values - is this needed?
        return True

    def assert_snapshot_capabilities_struct(
        self, snapshotCapabilities: Clusters.CameraAvStreamManagement.Structs.SnapshotCapabilitiesStruct
    ) -> bool:
        self.assert_video_resolution_struct(snapshotCapabilities.resolution)
        asserts.assert_greater_equal(snapshotCapabilities.maxFrameRate, 1, "MaxFrameRate is less than 1")
        asserts.assert_in(
            snapshotCapabilities.imageCodec,
            Clusters.CameraAvStreamManagement.Enums.ImageCodecEnum,
            "ImageCodec is not a valid ImageCodecEnum",
        )
        return True

    def assert_viewport_struct(self, viewport: Clusters.Globals.Structs.ViewportStruct) -> bool:
        # No constraints
        return True

    def assert_enum(self, member, enum, msg) -> bool:
        asserts.assert_in(member, enum, msg)
        return True


if __name__ == "__main__":
    default_matter_test_main()
