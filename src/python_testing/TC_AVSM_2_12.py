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
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts

logger = logging.getLogger(__name__)


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
            TestStep(14, "TH reads Viewport attribute.",
                     "Verify that the DUT response contains a ViewportStruct value. Store value as aViewport"),
            TestStep(15, "TH writes Viewport attribute with a new value.", "DUT responds with a SUCCESS status code."),
            TestStep(16, "TH reads Viewport attribute.", "Verify that the value is the same as was written in Step 15."),
            TestStep(17, "TH reads SpeakerMuted attribute.",
                     "Verify that the DUT response contains a bool value. Store value as aSpeakerMuted"),
            TestStep(18, "TH writes SpeakerMuted attribute with value set to !aSpeakerMuted.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(19, "TH reads SpeakerMuted attribute.", "Verify that the value is the same as was written in Step 18."),
            TestStep(20, "TH reads MicrophoneMuted attribute.",
                     "Verify that the DUT response contains a bool value. Store value as aMicrophoneMuted"),
            TestStep(21, "TH writes MicrophoneMuted attribute with value set to !aMicrophoneMuted.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(22, "TH reads MicrophoneMuted attribute.", "Verify that the value is the same as was written in Step 21."),
            TestStep(23, "TH reads LocalVideoRecordingEnabled attribute.",
                     "Verify that the DUT response contains a bool value. Store value as aLocalVideoRecordingEnabled"),
            TestStep(24, "TH writes LocalVideoRecordingEnabled attribute with value set to !aLocalVideoRecordingEnabled.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(25, "TH reads LocalVideoRecordingEnabled attribute.",
                     "Verify that the value is the same as was written in Step 24."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.CameraAvStreamManagement))
    async def test_TC_AVSM_2_12(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes

        self.step(1)
        # Commission DUT - already done

        # Implicit step to get the feature map to ensure attribute operations
        # are performed on supported features
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        self.adoSupported = aFeatureMap & cluster.Bitmaps.Feature.kAudio
        self.vdoSupported = aFeatureMap & cluster.Bitmaps.Feature.kVideo
        self.privSupported = aFeatureMap & cluster.Bitmaps.Feature.kPrivacy
        self.speakerSupported = aFeatureMap & cluster.Bitmaps.Feature.kSpeaker
        self.localStorageSupported = aFeatureMap & cluster.Bitmaps.Feature.kLocalStorage
        self.hdrSupported = aFeatureMap & cluster.Bitmaps.Feature.kHighDynamicRange
        self.nightVisionSupported = aFeatureMap & cluster.Bitmaps.Feature.kNightVision

        if self.hdrSupported:
            self.step(2)
            hdrEnabled = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.HDRModeEnabled
            )
            logger.info(f"Rx'd HDRModeEnabled: {hdrEnabled}")

            self.step(3)
            result = await self.write_single_attribute(attr.HDRModeEnabled(not hdrEnabled),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write HDRModeEnabled")
            logger.info(f"Tx'd HDRModeEnabled: {not hdrEnabled}")

            self.step(4)
            hdrEnabledNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.HDRModeEnabled
            )
            logger.info(f"Rx'd HDRModeEnabled: {hdrEnabledNew}")
            asserts.assert_equal(hdrEnabledNew, not hdrEnabled, "Value does not match what was written in step 3")
        else:
            logging.info("HDR Feature not supported. Test steps skipped")
            self.skip_step(2)
            self.skip_step(3)
            self.skip_step(4)

        if self.privSupported:
            self.step(5)
            softRecordingPrivMode = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SoftRecordingPrivacyModeEnabled
            )
            logger.info(f"Rx'd SoftRecordingPrivacyModeEnabled: {softRecordingPrivMode}")

            self.step(6)
            result = await self.write_single_attribute(attr.SoftRecordingPrivacyModeEnabled(not softRecordingPrivMode),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write SoftRecordingPrivacyModeEnabled")
            logger.info(f"Tx'd SoftRecordingPrivacyModeEnabled: {not softRecordingPrivMode}")

            self.step(7)
            softRecordingPrivModeNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SoftRecordingPrivacyModeEnabled
            )
            logger.info(f"Rx'd SoftRecordingPrivacyModeEnabled: {softRecordingPrivModeNew}")
            asserts.assert_equal(softRecordingPrivModeNew, not softRecordingPrivMode,
                                 "Value does not match what was written in step 6")

            self.step(8)
            softLivestreamPrivMode = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SoftLivestreamPrivacyModeEnabled
            )
            logger.info(f"Rx'd SoftLivestreamPrivacyModeEnabled: {softLivestreamPrivMode}")

            self.step(9)
            result = await self.write_single_attribute(attr.SoftLivestreamPrivacyModeEnabled(not softLivestreamPrivMode),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write SoftLivestreamPrivacyModeEnabled")
            logger.info(f"Tx'd SoftLivestreamPrivacyModeEnabled: {not softLivestreamPrivMode}")

            self.step(10)
            softLivestreamPrivModeNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SoftLivestreamPrivacyModeEnabled
            )
            logger.info(f"Rx'd SoftLivestreamPrivacyModeEnabled: {softLivestreamPrivModeNew}")
            asserts.assert_equal(softLivestreamPrivModeNew, not softLivestreamPrivMode,
                                 "Value does not match what was written in step 9")
        else:
            logging.info("Privacy Feature not supported. Test steps skipped")
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
            logger.info(f"Rx'd NightVision: {nightVision}")

            self.step(12)
            nightVisionToWrite = (nightVision + 1) % 3
            result = await self.write_single_attribute(attr.NightVision(nightVisionToWrite),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write NightVision")
            logger.info(f"Tx'd NightVision: {nightVisionToWrite}")

            self.step(13)
            nightVisionNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.NightVision
            )
            logger.info(f"Rx'd NightVision: {nightVisionNew}")
            asserts.assert_equal(nightVisionNew, nightVisionToWrite, "Value does not match what was written in step 12")
        else:
            self.skip_step(11)
            self.skip_step(12)
            self.skip_step(13)

        if self.vdoSupported:
            self.step(14)
            viewport = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Viewport
            )
            logger.info(f"Rx'd Viewport: {viewport}")

            self.step(15)
            viewportToWrite = Globals.Structs.ViewportStruct(viewport.x1+1, viewport.y1+1, viewport.x2+1, viewport.y2+1)
            result = await self.write_single_attribute(attr.Viewport(viewportToWrite),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write viewportToWrite")
            logger.info(f"Tx'd Viewport: {viewportToWrite}")

            self.step(16)
            viewportNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.Viewport
            )
            logger.info(f"Rx'd Viewport: {viewportNew}")
            asserts.assert_equal(viewportNew, viewportToWrite, "Value does not match what was written in step 15")
        else:
            self.skip_step(14)
            self.skip_step(15)
            self.skip_step(16)

        if self.speakerSupported:
            self.step(17)
            speakerMuted = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerMuted
            )
            logger.info(f"Rx'd SpeakerMuted: {speakerMuted}")

            self.step(18)
            result = await self.write_single_attribute(attr.SpeakerMuted(not speakerMuted),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write SpeakerMuted")
            logger.info(f"Tx'd SpeakerMuted: {not speakerMuted}")

            self.step(19)
            speakerMutedNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SpeakerMuted
            )
            logger.info(f"Rx'd SpeakerMuted: {speakerMutedNew}")
            asserts.assert_equal(speakerMutedNew, not speakerMuted, "Value does not match what was written in step 18")
        else:
            self.skip_step(17)
            self.skip_step(18)
            self.skip_step(19)

        if self.adoSupported:
            self.step(20)
            micMuted = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneMuted
            )
            logger.info(f"Rx'd MicrophoneMuted: {micMuted}")

            self.step(21)
            result = await self.write_single_attribute(attr.MicrophoneMuted(not micMuted),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write MicrophoneMuted")
            logger.info(f"Tx'd MicrophoneMuted: {not micMuted}")

            self.step(22)
            micMutedNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneMuted
            )
            logger.info(f"Rx'd MicrophoneMuted: {micMutedNew}")
            asserts.assert_equal(micMutedNew, not micMuted, "Value does not match what was written in step 21")
        else:
            self.skip_step(20)
            self.skip_step(21)
            self.skip_step(22)

        if self.vdoSupported and self.localStorageSupported:
            self.step(23)
            localVideoRecordingEnabled = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.LocalVideoRecordingEnabled
            )
            logger.info(f"Rx'd LocalVideoRecordingEnabled: {localVideoRecordingEnabled}")

            self.step(24)
            result = await self.write_single_attribute(attr.LocalVideoRecordingEnabled(not localVideoRecordingEnabled),
                                                       endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write LocalVideoRecordingEnabled")
            logger.info(f"Tx'd LocalVideoRecordingEnabled: {not localVideoRecordingEnabled}")

            self.step(25)
            localVideoRecordingEnabledNew = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.LocalVideoRecordingEnabled
            )
            logger.info(f"Rx'd LocalVideoRecordingEnabled: {localVideoRecordingEnabledNew}")
            asserts.assert_equal(localVideoRecordingEnabledNew, not localVideoRecordingEnabled,
                                 "Value does not match what was written in step 24")
        else:
            self.skip_step(23)
            self.skip_step(24)
            self.skip_step(25)


if __name__ == "__main__":
    default_matter_test_main()
