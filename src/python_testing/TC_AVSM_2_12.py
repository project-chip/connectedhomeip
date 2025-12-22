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
import random

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import Globals
from matter.interaction_model import Status
from matter.testing import matter_asserts
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVSM_2_12(MatterBaseTest):
    def desc_TC_AVSM_2_12(self) -> str:
        return "[TC-AVSM-2.12] Validate writable attributes with Server as DUT"

    def pics_TC_AVSM_2_12(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_12(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads HDRModeEnabled attribute.",
                     "Verify that the DUT response contains a bool value. Store value as aHDREnabled"),
            TestStep(3, "TH writes HDRModeEnabled attribute with value set to !aHDREnabled.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(4, "TH reads HDRModeEnabled attribute.", "Verify that the value is the same as was written in Step 3."),
            TestStep(5, "TH reads SoftRecordingPrivacyModeEnabled attribute.",
                     "Verify that the DUT response contains a bool value. Store value as aSoftRecordingPrivacyMode"),
            TestStep(6, "TH writes SoftRecordingPrivacyModeEnabled attribute with value set to !aSoftRecordingPrivacyMode.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(7, "TH reads SoftRecordingPrivacyModeEnabled attribute.",
                     "Verify that the value is the same as was written in Step 6."),
            TestStep(8, "TH reads SoftLivestreamPrivacyModeEnabled attribute.",
                     "Verify that the DUT response contains a bool value. Store value as aSoftLivestreamPrivacyMode"),
            TestStep(9, "TH writes SoftLivestreamPrivacyModeEnabled attribute with value set to !aSoftLivestreamPrivacyMode.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(10, "TH reads SoftLivestreamPrivacyModeEnabled attribute.",
                     "Verify that the value is the same as was written in Step 9."),
            TestStep(11, "TH reads NightVision attribute.",
                     "Verify that the DUT response contains a TriStateAutoEnum value. Store value as aNightVision"),
            TestStep(12, "TH writes NightVision attribute with a new value", "DUT responds with a SUCCESS status code."),
            TestStep(13, "TH reads NightVision attribute.", "Verify that the value is the same as was written in Step 12."),
            TestStep(14, "TH reads NightVisionIllum attribute.",
                     "Verify that the DUT response contains a TriStateAutoEnum value. Store value as aNightVisionIllum"),
            TestStep(15, "TH writes NightVisionIllum attribute with a new value", "DUT responds with a SUCCESS status code."),
            TestStep(16, "TH reads NightVisionIllum attribute.", "Verify that the value is the same as was written in Step 15."),
            TestStep(17, "TH reads Viewport attribute.",
                     "Verify that the DUT response contains a ViewportStruct value. Store value as aViewport"),
            TestStep(18, "TH writes Viewport attribute with a new value.", "DUT responds with a SUCCESS status code."),
            TestStep(19, "TH reads Viewport attribute.", "Verify that the value is the same as was written in Step 18."),
            TestStep(20, "TH reads SpeakerMuted attribute.",
                     "Verify that the DUT response contains a bool value. Store value as aSpeakerMuted"),
            TestStep(21, "TH writes SpeakerMuted attribute with value set to !aSpeakerMuted.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(22, "TH reads SpeakerMuted attribute.", "Verify that the value is the same as was written in Step 21."),
            TestStep(23, "TH reads SpeakerVolumeLevel attribute.",
                     "Verify that the DUT response is in the range SpeakerMin to SpeakerMax. Store value as aSpeakerVolumeLevel"),
            TestStep(24, "TH writes SpeakerVolumeLevel attribute with a new value of aSpeakerVolumeLevel.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(25, "TH reads SpeakerVolumeLevel attribute.", "Verify that the value is the same as was written in Step 24."),
            TestStep(26, "TH reads MicrophoneMuted attribute.",
                     "Verify that the DUT response contains a bool value. Store value as aMicrophoneMuted"),
            TestStep(27, "TH writes MicrophoneMuted attribute with value set to !aMicrophoneMuted.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(28, "TH reads MicrophoneMuted attribute.", "Verify that the value is the same as was written in Step 27."),
            TestStep(29, "TH reads MicrophoneVolumeLevel attribute.",
                     "Verify that the DUT response is in the range MicrophoneMin to MicrophoneMax. Store value as aMicrophoneVolumeLevel"),
            TestStep(30, "TH writes MicrophoneVolumeLevel attribute with a new value of aMicrophoneVolumeLevel.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(31, "TH reads MicrophoneVolumeLevel attribute.",
                     "Verify that the value is the same as was written in Step 30."),
            TestStep(32, "TH reads MicrophoneAGCEnabled attribute.",
                     "Verify that the DUT response contains a bool value. Store value as aMicrophoneAGCEnabled"),
            TestStep(33, "TH writes MicrophoneAGCEnabled attribute with value set to !aMicrophoneAGCEnabled.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(34, "TH reads MicrophoneAGCEnabled attribute.",
                     "Verify that the value is the same as was written in Step 33."),
            TestStep(35, "TH reads ImageRotation attribute.",
                     "Verify that the DUT response is not greater than 359. Store value as aImageRotation"),
            TestStep(36, "TH writes ImageRotation attribute with a new value of aImageRotation.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(37, "TH reads ImageRotation attribute.", "Verify that the value is the same as was written in Step 36."),
            TestStep(38, "TH reads ImageFlipHorizontal attribute.",
                     "Verify that the DUT response contains a bool value. Store value as aImageFlipHorizontal"),
            TestStep(39, "TH writes ImageFlipHorizontal attribute with value set to !aImageFlipHorizontal.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(40, "TH reads ImageFlipHorizontal attribute.",
                     "Verify that the value is the same as was written in Step 39."),
            TestStep(41, "TH reads ImageFlipVertical attribute.",
                     "Verify that the DUT response contains a bool value. Store value as aImageFlipVertical"),
            TestStep(42, "TH writes ImageFlipVertical attribute with value set to !aImageFlipVertical.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(43, "TH reads ImageFlipVertical attribute.",
                     "Verify that the value is the same as was written in Step 42."),
            TestStep(44, "TH reads LocalVideoRecordingEnabled attribute.",
                     "Verify that the DUT response contains a bool value. Store value as aLocalVideoRecordingEnabled"),
            TestStep(45, "TH writes LocalVideoRecordingEnabled attribute with value set to !aLocalVideoRecordingEnabled.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(46, "TH reads LocalVideoRecordingEnabled attribute.",
                     "Verify that the value is the same as was written in Step 45."),
            TestStep(47, "TH reads LocalSnapshotRecordingEnabled attribute.",
                     "Verify that the DUT response contains a bool value. Store value as aLocalSnapshotRecordingEnabled"),
            TestStep(48, "TH writes LocalSnapshotRecordingEnabled attribute with value set to !aLocalSnapshotRecordingEnabled.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(49, "TH reads LocalSnapshotRecordingEnabled attribute.",
                     "Verify that the value is the same as was written in Step 48."),
            TestStep(50, "TH reads StatusLightEnabled attribute.",
                     "Verify that the DUT response contains a bool value. Store value as aStatusLightEnabled"),
            TestStep(51, "TH writes StatusLightEnabled attribute with value set to !aStatusLightEnabled.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(52, "TH reads StatusLightEnabled attribute.",
                     "Verify that the value is the same as was written in Step 51."),
            TestStep(53, "TH reads StatusLightBrightness attribute.",
                     "Verify that the DUT response contains a ThreeLevelAutoEnum value. Store value as aStatusLightBrightness"),
            TestStep(54, "TH writes StatusLightBrightness attribute with a new valud enum value.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(55, "TH reads StatusLightBrightness attribute.",
                     "Verify that the value is the same as was written in Step 54."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.CameraAvStreamManagement))
    async def test_TC_AVSM_2_12(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes

        self.step(1)
        # Commission DUT - already done

        # Implicit step to get the feature map to ensure attribute operations
        # are performed on supported features
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        log.info(f"Rx'd FeatureMap: {aFeatureMap}")
        self.adoSupported = aFeatureMap & cluster.Bitmaps.Feature.kAudio
        self.vdoSupported = aFeatureMap & cluster.Bitmaps.Feature.kVideo
        self.privSupported = aFeatureMap & cluster.Bitmaps.Feature.kPrivacy
        self.speakerSupported = aFeatureMap & cluster.Bitmaps.Feature.kSpeaker
        self.localStorageSupported = aFeatureMap & cluster.Bitmaps.Feature.kLocalStorage
        self.hdrSupported = aFeatureMap & cluster.Bitmaps.Feature.kHighDynamicRange
        self.nightVisionSupported = aFeatureMap & cluster.Bitmaps.Feature.kNightVision
        self.imagecControlSupported = aFeatureMap & cluster.Bitmaps.Feature.kImageControl
        self.snapshotSupported = aFeatureMap & cluster.Bitmaps.Feature.kSnapshot

        if self.hdrSupported:
            self.step(2)
            hdrEnabled = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.HDRModeEnabled
            )
            log.info(f"Rx'd HDRModeEnabled: {hdrEnabled}")

            self.step(3)
            result = await self.write_single_attribute(attr.HDRModeEnabled(not hdrEnabled),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write HDRModeEnabled")
            log.info(f"Tx'd HDRModeEnabled: {not hdrEnabled}")

            self.step(4)
            hdrEnabledNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.HDRModeEnabled
            )
            log.info(f"Rx'd HDRModeEnabled: {hdrEnabledNew}")
            asserts.assert_equal(hdrEnabledNew, not hdrEnabled, "Value does not match what was written in step 3")
        else:
            log.info("HDR Feature not supported. Test steps skipped")
            self.skip_step(2)
            self.skip_step(3)
            self.skip_step(4)

        if self.privSupported:
            self.step(5)
            softRecordingPrivMode = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SoftRecordingPrivacyModeEnabled
            )
            log.info(f"Rx'd SoftRecordingPrivacyModeEnabled: {softRecordingPrivMode}")

            self.step(6)
            result = await self.write_single_attribute(attr.SoftRecordingPrivacyModeEnabled(not softRecordingPrivMode),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write SoftRecordingPrivacyModeEnabled")
            log.info(f"Tx'd SoftRecordingPrivacyModeEnabled: {not softRecordingPrivMode}")

            self.step(7)
            softRecordingPrivModeNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SoftRecordingPrivacyModeEnabled
            )
            log.info(f"Rx'd SoftRecordingPrivacyModeEnabled: {softRecordingPrivModeNew}")
            asserts.assert_equal(softRecordingPrivModeNew, not softRecordingPrivMode,
                                 "Value does not match what was written in step 6")

            self.step(8)
            softLivestreamPrivMode = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SoftLivestreamPrivacyModeEnabled
            )
            log.info(f"Rx'd SoftLivestreamPrivacyModeEnabled: {softLivestreamPrivMode}")

            self.step(9)
            result = await self.write_single_attribute(attr.SoftLivestreamPrivacyModeEnabled(not softLivestreamPrivMode),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write SoftLivestreamPrivacyModeEnabled")
            log.info(f"Tx'd SoftLivestreamPrivacyModeEnabled: {not softLivestreamPrivMode}")

            self.step(10)
            softLivestreamPrivModeNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SoftLivestreamPrivacyModeEnabled
            )
            log.info(f"Rx'd SoftLivestreamPrivacyModeEnabled: {softLivestreamPrivModeNew}")
            asserts.assert_equal(softLivestreamPrivModeNew, not softLivestreamPrivMode,
                                 "Value does not match what was written in step 9")
        else:
            log.info("Privacy Feature not supported. Test steps skipped")
            self.skip_step(5)
            self.skip_step(6)
            self.skip_step(7)
            self.skip_step(8)
            self.skip_step(9)
            self.skip_step(10)

        if self.nightVisionSupported:
            self.step(11)
            nightVision = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.NightVision
            )
            log.info(f"Rx'd NightVision: {nightVision}")

            self.step(12)
            nightVisionToWrite = (nightVision + 1) % 3
            result = await self.write_single_attribute(attr.NightVision(nightVisionToWrite),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write NightVision")
            log.info(f"Tx'd NightVision: {nightVisionToWrite}")

            self.step(13)
            nightVisionNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.NightVision)
            log.info(f"Rx'd NightVision: {nightVisionNew}")
            asserts.assert_equal(nightVisionNew, nightVisionToWrite, "Value does not match what was written in step 12")

            # NightVisionIllum is optional, so ensure it is present
            if await self.attribute_guard(endpoint=endpoint, attribute=attr.NightVisionIllum):
                self.step(14)
                nightVisionIllum = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.NightVisionIllum)
                log.info(f"Rx'd NightVisionIllum: {nightVisionIllum}")

                self.step(15)
                nightVisionIllumToWrite = (nightVisionIllum + 1) % 3
                result = await self.write_single_attribute(attr.NightVisionIllum(nightVisionIllumToWrite),
                                                           endpoint_id=endpoint)
                asserts.assert_equal(result, Status.Success, "Error when trying to write NightVisionIllum")
                log.info(f"Tx'd NightVisionIllum: {nightVisionIllumToWrite}")

                self.step(16)
                nightVisionIllumNew = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.NightVisionIllum)
                log.info(f"Rx'd NightVisionIllum: {nightVisionIllumNew}")
                asserts.assert_equal(nightVisionIllumNew, nightVisionIllumToWrite,
                                     "Value does not match what was written for NightVisionIllum in step 15")
            else:
                self.skip_step(14)
                self.skip_step(15)
                self.skip_step(16)
        else:
            self.skip_step(11)
            self.skip_step(12)
            self.skip_step(13)
            self.skip_step(14)
            self.skip_step(15)
            self.skip_step(16)

        if self.vdoSupported:
            self.step(17)
            viewport = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Viewport
            )
            log.info(f"Rx'd Viewport: {viewport}")

            self.step(18)
            viewportToWrite = Globals.Structs.ViewportStruct(viewport.x1+1, viewport.y1+1, viewport.x2+1, viewport.y2+1)
            result = await self.write_single_attribute(attr.Viewport(viewportToWrite),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write viewportToWrite")
            log.info(f"Tx'd Viewport: {viewportToWrite}")

            self.step(19)
            viewportNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Viewport
            )
            log.info(f"Rx'd Viewport: {viewportNew}")
            asserts.assert_equal(viewportNew, viewportToWrite, "Value does not match what was written in step 18")
        else:
            self.skip_step(17)
            self.skip_step(18)
            self.skip_step(19)

        if self.speakerSupported:
            self.step(20)
            speakerMuted = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerMuted
            )
            log.info(f"Rx'd SpeakerMuted: {speakerMuted}")

            self.step(21)
            result = await self.write_single_attribute(attr.SpeakerMuted(not speakerMuted),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write SpeakerMuted")
            log.info(f"Tx'd SpeakerMuted: {not speakerMuted}")

            self.step(22)
            speakerMutedNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerMuted
            )
            log.info(f"Rx'd SpeakerMuted: {speakerMutedNew}")
            asserts.assert_equal(speakerMutedNew, not speakerMuted, "Value does not match what was written in step 21")

            speakerMinLevel = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerMinLevel)
            log.info(f"Rx'd SpeakerMinLevel: {speakerMinLevel}")

            speakerMaxLevel = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerMaxLevel)
            log.info(f"Rx'd SpeakerMaxLevel: {speakerMaxLevel}")

            # Only test write of new value if SpeakerMin != SpeakerMax
            if speakerMinLevel < speakerMaxLevel:
                self.step(23)
                speakerVolumeLevel = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerVolumeLevel)
                log.info(f"Rx'd SpeakerVolumeLevel: {speakerVolumeLevel}")
                matter_asserts.assert_int_in_range(speakerVolumeLevel, speakerMinLevel, speakerMaxLevel,
                                                   "Speaker Volume not between SpeakerMin and SpeakerMax")

                self.step(24)
                newSpeakerVolume = random.randint(speakerMinLevel, speakerMaxLevel)
                while newSpeakerVolume == speakerVolumeLevel:
                    newSpeakerVolume = random.randint(speakerMinLevel, speakerMaxLevel)

                result = await self.write_single_attribute(attr.SpeakerVolumeLevel(newSpeakerVolume),
                                                           endpoint_id=endpoint)
                asserts.assert_equal(result, Status.Success, "Error when trying to write SpeakerVolumeLevel")
                log.info(f"Tx'd SpeakerVolumeLevel: {newSpeakerVolume}")

                self.step(25)
                speakerVolumeLevelNew = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerVolumeLevel)
                log.info(f"Rx'd SpeakerVolumeLevel: {speakerVolumeLevelNew}")
                asserts.assert_equal(speakerVolumeLevelNew, newSpeakerVolume,
                                     "Value does not match what was written for SpeakerVolumeLevel in step 24")
            else:
                self.skip_step(23)
                self.skip_step(24)
                self.skip_step(25)
        else:
            self.skip_step(20)
            self.skip_step(21)
            self.skip_step(22)
            self.skip_step(23)
            self.skip_step(24)
            self.skip_step(25)

        if self.adoSupported:
            self.step(26)
            micMuted = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneMuted
            )
            log.info(f"Rx'd MicrophoneMuted: {micMuted}")

            self.step(27)
            result = await self.write_single_attribute(attr.MicrophoneMuted(not micMuted),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write MicrophoneMuted")
            log.info(f"Tx'd MicrophoneMuted: {not micMuted}")

            self.step(28)
            micMutedNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneMuted
            )
            log.info(f"Rx'd MicrophoneMuted: {micMutedNew}")
            asserts.assert_equal(micMutedNew, not micMuted, "Value does not match what was written in step 27")

            micMinLevel = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneMinLevel)
            log.info(f"Rx'd MicrophoneMinLevel: {micMinLevel}")

            micMaxLevel = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneMaxLevel)
            log.info(f"Rx'd MicrophoneMaxLevel: {micMaxLevel}")

            # Only test write of new value if MicMin != MicMax
            if micMinLevel < micMaxLevel:
                self.step(29)
                micVolumeLevel = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneVolumeLevel)
                log.info(f"Rx'd MicrophoneVolumeLevel: {micVolumeLevel}")
                matter_asserts.assert_int_in_range(micVolumeLevel, micMinLevel, micMaxLevel,
                                                   "Microphone Volume not between MicMin and MicMax")

                self.step(30)
                newMicVolume = random.randint(micMinLevel, micMaxLevel)
                while newMicVolume == micVolumeLevel:
                    newMicVolume = random.randint(micMinLevel, micMaxLevel)

                result = await self.write_single_attribute(attr.MicrophoneVolumeLevel(newMicVolume),
                                                           endpoint_id=endpoint)
                asserts.assert_equal(result, Status.Success, "Error when trying to write MicrophoneVolumeLevel")
                log.info(f"Tx'd MicrophoneVolumeLevel: {newMicVolume}")

                self.step(31)
                micVolumeLevelNew = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneVolumeLevel)
                log.info(f"Rx'd MicrophoneVolumeLevel: {micVolumeLevelNew}")
                asserts.assert_equal(micVolumeLevelNew, newMicVolume,
                                     "Value does not match what was written for MicrophoneVolumeLevel in step 30")
            else:
                self.skip_step(29)
                self.skip_step(30)
                self.skip_step(31)

            # MicrophoneAGCEnabled is optional so ensure it is present
            if await self.attribute_guard(endpoint=endpoint, attribute=attr.MicrophoneAGCEnabled):
                self.step(32)
                micAGCEnabled = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneAGCEnabled)
                log.info(f"Rx'd MicrophoneAGCEnabled: {micAGCEnabled}")

                self.step(33)
                result = await self.write_single_attribute(attr.MicrophoneAGCEnabled(not micAGCEnabled),
                                                           endpoint_id=endpoint)
                asserts.assert_equal(result, Status.Success, "Error when trying to write MicrophoneAGCEnabled")
                log.info(f"Tx'd MicrophoneAGCEnabled: {not micAGCEnabled}")

                self.step(34)
                micAGCEnabledNew = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneAGCEnabled)
                log.info(f"Rx'd MicrophoneAGCEnabled: {micAGCEnabledNew}")
                asserts.assert_equal(micAGCEnabledNew, not micAGCEnabled,
                                     "Value does not match what was written for MicrophoneAGCEnabled in step 33")
            else:
                self.skip_step(32)
                self.skip_step(33)
                self.skip_step(34)
        else:
            self.skip_step(26)
            self.skip_step(27)
            self.skip_step(28)
            self.skip_step(29)
            self.skip_step(30)
            self.skip_step(31)
            self.skip_step(32)
            self.skip_step(33)
            self.skip_step(34)

        if self.imagecControlSupported:
            # Attributes are all optional, one must be present, attribute guard for each
            if await self.attribute_guard(endpoint=endpoint, attribute=attr.ImageRotation):
                self.step(35)
                imageRotation = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.ImageRotation)
                log.info(f"Rx'd ImageRotation: {imageRotation}")

                self.step(36)
                newImageRotation = random.randint(0, 359)

                while newImageRotation == imageRotation:
                    newImageRotation = random.randint(0, 359)

                result = await self.write_single_attribute(attr.ImageRotation(newImageRotation),
                                                           endpoint_id=endpoint)
                asserts.assert_equal(result, Status.Success, "Error when trying to write ImageRotation")
                log.info(f"Tx'd ImageRotation: {newImageRotation}")

                self.step(37)
                imageRotationNew = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.ImageRotation)
                log.info(f"Rx'd ImageRotation: {imageRotationNew}")
                asserts.assert_equal(imageRotationNew, newImageRotation,
                                     "Value does not match what was written for ImageRotation in step 36")

            else:
                self.skip_step(35)
                self.skip_step(36)
                self.skip_step(37)

            if await self.attribute_guard(endpoint=endpoint, attribute=attr.ImageFlipHorizontal):
                self.step(38)
                imageFlipHorizontal = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.ImageFlipHorizontal)
                log.info(f"Rx'd ImageFlipHorizontal: {imageFlipHorizontal}")

                self.step(39)
                result = await self.write_single_attribute(attr.ImageFlipHorizontal(not imageFlipHorizontal),
                                                           endpoint_id=endpoint)
                asserts.assert_equal(result, Status.Success, "Error when trying to write ImageFlipHorizontal")
                log.info(f"Tx'd ImageFlipHorizontal: {not imageFlipHorizontal}")

                self.step(40)
                imageFlipHorizontalNew = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.ImageFlipHorizontal)
                log.info(f"Rx'd ImageFlipHorizontal: {imageFlipHorizontalNew}")
                asserts.assert_equal(imageFlipHorizontalNew, not imageFlipHorizontal,
                                     "Value does not match what was written for ImageFlipHorizontal in step 39")
            else:
                self.skip_step(38)
                self.skip_step(39)
                self.skip_step(40)

            if await self.attribute_guard(endpoint=endpoint, attribute=attr.ImageFlipVertical):
                self.step(41)
                imageFlipVertical = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.ImageFlipVertical)
                log.info(f"Rx'd ImageFlipVertical: {imageFlipVertical}")

                self.step(42)
                result = await self.write_single_attribute(attr.ImageFlipVertical(not imageFlipVertical),
                                                           endpoint_id=endpoint)
                asserts.assert_equal(result, Status.Success, "Error when trying to write ImageFlipVertical")
                log.info(f"Tx'd ImageFlipVertical: {not imageFlipVertical}")

                self.step(43)
                imageFlipVerticalNew = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=attr.ImageFlipVertical)
                log.info(f"Rx'd ImageFlipVertical: {imageFlipVerticalNew}")
                asserts.assert_equal(imageFlipVerticalNew, not imageFlipVertical,
                                     "Value does not match what was written for ImageFlipVertical in step 42")
            else:
                self.skip_step(41)
                self.skip_step(42)
                self.skip_step(43)
        else:
            self.skip_step(35)
            self.skip_step(36)
            self.skip_step(37)
            self.skip_step(38)
            self.skip_step(39)
            self.skip_step(40)
            self.skip_step(41)
            self.skip_step(42)
            self.skip_step(43)

        if self.vdoSupported and self.localStorageSupported:
            self.step(44)
            localVideoRecordingEnabled = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.LocalVideoRecordingEnabled
            )
            log.info(f"Rx'd LocalVideoRecordingEnabled: {localVideoRecordingEnabled}")

            self.step(45)
            result = await self.write_single_attribute(attr.LocalVideoRecordingEnabled(not localVideoRecordingEnabled),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write LocalVideoRecordingEnabled")
            log.info(f"Tx'd LocalVideoRecordingEnabled: {not localVideoRecordingEnabled}")

            self.step(46)
            localVideoRecordingEnabledNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.LocalVideoRecordingEnabled
            )
            log.info(f"Rx'd LocalVideoRecordingEnabled: {localVideoRecordingEnabledNew}")
            asserts.assert_equal(localVideoRecordingEnabledNew, not localVideoRecordingEnabled,
                                 "Value does not match what was written for LocalVideoRecordingEnabled in step 45")
        else:
            self.skip_step(44)
            self.skip_step(45)
            self.skip_step(46)

        if self.snapshotSupported and self.localStorageSupported:
            self.step(47)
            localSnapshotRecordingEnabled = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.LocalSnapshotRecordingEnabled
            )
            log.info(f"Rx'd LocalSnapshotRecordingEnabled: {localSnapshotRecordingEnabled}")

            self.step(48)
            result = await self.write_single_attribute(attr.LocalSnapshotRecordingEnabled(not localSnapshotRecordingEnabled),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write LocalSnapshotRecordingEnabled")
            log.info(f"Tx'd LocalSnapshotRecordingEnabled: {not localSnapshotRecordingEnabled}")

            self.step(49)
            localSnapshotRecordingEnabledNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.LocalSnapshotRecordingEnabled
            )
            log.info(f"Rx'd LocalSnapshotRecordingEnabled: {localSnapshotRecordingEnabledNew}")
            asserts.assert_equal(localSnapshotRecordingEnabledNew, not localSnapshotRecordingEnabled,
                                 "Value does not match what was written for LocalSnapshotRecordingEnabled in step 48")
        else:
            self.skip_step(47)
            self.skip_step(48)
            self.skip_step(49)

        if await self.attribute_guard(endpoint=endpoint, attribute=attr.StatusLightEnabled):
            self.step(50)
            statusLightEnabled = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.StatusLightEnabled)
            log.info(f"Rx'd StatusLightEnabled: {statusLightEnabled}")

            self.step(51)
            result = await self.write_single_attribute(attr.StatusLightEnabled(not statusLightEnabled),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write StatusLightEnabled")
            log.info(f"Tx'd StatusLightEnabled: {not statusLightEnabled}")

            self.step(52)
            statusLightEnabledNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.StatusLightEnabled)
            log.info(f"Rx'd StatusLightEnabled: {statusLightEnabledNew}")
            asserts.assert_equal(statusLightEnabledNew, not statusLightEnabled,
                                 "Value does not match what was written for StatusLightEnabled in step 51")
        else:
            self.skip_step(50)
            self.skip_step(51)
            self.skip_step(52)

        if await self.attribute_guard(endpoint=endpoint, attribute=attr.StatusLightBrightness):
            self.step(53)
            statusLightBrightness = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.StatusLightBrightness)
            log.info(f"Rx'd StatusLightBrightness: {statusLightBrightness}")

            self.step(54)
            statusLightBrightnessToWrite = (statusLightBrightness + 1) % 4
            result = await self.write_single_attribute(attr.StatusLightBrightness(statusLightBrightnessToWrite),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write StatusLightBrightness")
            log.info(f"Tx'd StatusLightBrightness: {statusLightBrightnessToWrite}")

            self.step(55)
            statusLightBrightnessNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.StatusLightBrightness)
            log.info(f"Rx'd StatusLightBrightness: {statusLightBrightnessNew}")
            asserts.assert_equal(statusLightBrightnessNew, statusLightBrightnessToWrite,
                                 "Value does not match what was written for StatusLightBrightness in step 54")
        else:
            self.skip_step(53)
            self.skip_step(54)
            self.skip_step(55)


if __name__ == "__main__":
    default_matter_test_main()
