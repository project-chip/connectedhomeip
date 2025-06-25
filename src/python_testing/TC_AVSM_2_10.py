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
from chip import ChipDeviceCtrl
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts
from TC_AVSMTestBase import AVSMTestBase

logger = logging.getLogger(__name__)


class TC_AVSM_2_10(MatterBaseTest, AVSMTestBase):
    def desc_TC_AVSM_2_10(self) -> str:
        return "[TC-AVSM-2.10] Validate CaptureSnapshot Functionality with Server as DUT"

    def pics_TC_AVSM_2_10(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_10(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(
                1,
                "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify SNP is supported. Do not run if not.",
            ),
            TestStep(
                2,
                "TH sends the CaptureSnapshot command with SnapshotStreamID set to Null.",
                "DUT responds with NOT_FOUND status code.",
            ),
            TestStep(
                3,
                "TH reads SnapshotCapabilities attribute from CameraAVStreamManagement Cluster on DUT.",
                "Store this value in aSnapshotCapabilities.",
            ),
            TestStep(
                4,
                "If WMARK is supported, TH sets it’s local aWatermark to True, otherwise this is Null",
                "",
            ),
            TestStep(
                5,
                "If OSD is supported, TH sets its local aOSD to True, otherwise this is Null",
                "",
            ),
            TestStep(
                6,
                "TH sends the SnapshotStreamAllocate command with valid values of ImageCodec, MaxFrameRate, MinResolution=MaxResolution=Resolution from aSnapshotCapabilities, WatermarkEnabled to aWatermark, OSDEnabled to aOSD, and Quality set to 90.",
                "DUT responds with SnapshotStreamAllocateResponse command. Ensure: * the response has a valid SnapshotStreamID, store this as myStreamID. * the provided ImageCodec is the same as that provided in the allocation in step 6",
            ),
            TestStep(
                7,
                "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the number of allocated snapshot streams in the list is 1.",
            ),
            TestStep(
                8,
                "TH sends the CaptureSnapshot command with SnapshotStreamID set to aStreamID.",
                "DUT responds with CaptureSnapshotResponse command with the image in the Data field.",
            ),
            TestStep(
                9,
                "TH sends the CaptureSnapshot command with SnapshotStreamID set to aStreamID + 1.",
                "DUT responds with NOT_FOUND status code.",
            ),
            TestStep(
                10,
                "TH sends the CaptureSnapshot command with SnapshotStreamID set to Null.",
                "DUT responds with CaptureSnapshotResponse command with the image in the Data field.",
            ),
            TestStep(
                11,
                "If DUT supports Privacy feature, TH writes attribute SoftLivestreamPrivacyModeEnabled = true in the CameraAVStreamManagement Cluster on DUT",
                "DUT responds with a SUCCESS status code.",
            ),
            TestStep(
                12,
                "TH sends the CaptureSnapshot command with SnapshotStreamID set to aStreamID.",
                "DUT responds with INVALID_IN_STATE status code.",
            ),
            TestStep(
                13,
                "TH sends the SnapshotStreamDeallocate command with SnapshotStreamID set to aStreamID.",
                "DUT responds with a SUCCESS status code.",
            ),
            TestStep(
                14,
                "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the number of allocated snapshot streams in the list is 0.",
            ),
            TestStep(
                15,
                "TH sends the CaptureSnapshot command with SnapshotStreamID set to Null.",
                "DUT responds with NOT_FOUND status code.",
            ),
        ]

    @run_if_endpoint_matches(
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kSnapshot)
    )
    async def test_TC_AVSM_2_10(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done

        self.step(1)
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        snpSupport = (aFeatureMap & cluster.Bitmaps.Feature.kSnapshot) > 0
        asserts.assert_true(snpSupport, "Snapshot Feature is not supported.")
        self.privacySupport = (aFeatureMap & cluster.Bitmaps.Feature.kPrivacy) > 0

        self.step(2)
        captureSnapshotCmd = commands.CaptureSnapshot(
            requestedResolution=cluster.Structs.VideoResolutionStruct(width=640, height=480)
        )
        try:
            captureSnapshotResponse = await self.send_single_cmd(
                cmd=captureSnapshotCmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD
            )
            asserts.assert_true(False, "Unexpected success when expecting NOT_FOUND due to SnapshotStreamID set to Null")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.NotFound, "Unexpected error returned when expecting NOT_FOUND due to SnapshotStreamID set to Null"
            )
            pass

        self.step(3)
        aSnapshotCapabilities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.SnapshotCapabilities
        )
        logger.info(f"Rx'd SnapshotCapabilities: {aSnapshotCapabilities}")
        asserts.assert_is_not_none(aSnapshotCapabilities, "SnapshotCapabilities is Null")
        asserts.assert_greater(len(aSnapshotCapabilities), 0, "SnapshotCapabilities list is empty")

        self.step(4)
        aWatermark = True if (aFeatureMap & cluster.Bitmaps.Feature.kWatermark) != 0 else None

        self.step(5)
        aOSD = True if (aFeatureMap & cluster.Bitmaps.Feature.kOnScreenDisplay) != 0 else None

        self.step(6)
        try:
            snpStreamAllocateCmd = commands.SnapshotStreamAllocate(
                imageCodec=aSnapshotCapabilities[0].imageCodec,
                maxFrameRate=aSnapshotCapabilities[0].maxFrameRate,
                minResolution=aSnapshotCapabilities[0].resolution,
                maxResolution=aSnapshotCapabilities[0].resolution,
                quality=90,
                watermarkEnabled=aWatermark,
                OSDEnabled=aOSD,
            )
            snpStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=snpStreamAllocateCmd)
            logger.info(f"Rx'd SnapshotStreamAllocateResponse: {snpStreamAllocateResponse}")
            asserts.assert_is_not_none(
                snpStreamAllocateResponse.snapshotStreamID, "SnapshotStreamAllocateResponse does not contain StreamID"
            )
            # TODO check if the provided ImageCodec is the same as that provided in the allocation in step 6
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(7)
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams
        )
        logger.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        asserts.assert_equal(len(aAllocatedSnapshotStreams), 1, "The number of allocated snapshot streams in the list is not 1.")
        aStreamID = aAllocatedSnapshotStreams[0].snapshotStreamID
        aResolution = aAllocatedSnapshotStreams[0].minResolution

        self.step(8)
        try:
            captureSnapshotResponse = await self.send_single_cmd(
                cmd=commands.CaptureSnapshot(snapshotStreamID=aStreamID, requestedResolution=aResolution),
                endpoint=endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            logger.info(f"Rx'd CaptureSnapshotResponse: {captureSnapshotResponse}")
            asserts.assert_greater(len(captureSnapshotResponse.data), 0, "Image data returned by CaptureSnapshotResponse is empty")
            asserts.assert_equal(
                captureSnapshotResponse.imageCodec,
                aAllocatedSnapshotStreams[0].imageCodec,
                "ImageCodec does not match the the value found in the corresponding allocated snapshot stream",
            )
            asserts.assert_greater(
                captureSnapshotResponse.resolution.width, 0, "Image width returned by CaptureSnapshotResponse is <= 0"
            )
            asserts.assert_greater(
                captureSnapshotResponse.resolution.height, 0, "Image height returned by CaptureSnapshotResponse is <= 0"
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(9)
        try:
            await self.send_single_cmd(
                cmd=commands.CaptureSnapshot(snapshotStreamID=aStreamID + 1, requestedResolution=aResolution),
                endpoint=endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            asserts.assert_true(False, "Unexpected success when expecting NOT_FOUND due to snapshotStreamID set to aStreamID + 1")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.NotFound,
                "Unexpected error returned when expecting NOT_FOUND due to snapshotStreamID set to aStreamID + 1",
            )
            pass

        self.step(10)
        try:
            captureSnapshotResponse = await self.send_single_cmd(
                cmd=commands.CaptureSnapshot(requestedResolution=aResolution),
                endpoint=endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            logger.info(f"Rx'd CaptureSnapshotResponse: {captureSnapshotResponse}")
            asserts.assert_greater(len(captureSnapshotResponse.data), 0, "Image data returned by CaptureSnapshotResponse is empty")
            asserts.assert_equal(
                captureSnapshotResponse.imageCodec,
                aAllocatedSnapshotStreams[0].imageCodec,
                "ImageCodec does not match the the value found in the corresponding allocated snapshot stream",
            )
            asserts.assert_greater(
                captureSnapshotResponse.resolution.width, 0, "Image width returned by CaptureSnapshotResponse is <= 0"
            )
            asserts.assert_greater(
                captureSnapshotResponse.resolution.height, 0, "Image height returned by CaptureSnapshotResponse is <= 0"
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        if self.privacySupport:
            self.step(11)
            result = await self.write_single_attribute(attr.SoftLivestreamPrivacyModeEnabled(True), endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write SoftLivestreamPrivacyModeEnabled")
            logger.info(f"Tx'd : SoftLivestreamPrivacyModeEnabled{True}")

            self.step(12)
            try:
                await self.send_single_cmd(
                    cmd=commands.CaptureSnapshot(snapshotStreamID=aStreamID, requestedResolution=aResolution), endpoint=endpoint
                )
                asserts.assert_true(False, "Unexpected success when expecting INVALID_IN_STATE due to SoftPrivacy mode set to On")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.InvalidInState,
                    "Unexpected error returned when expecting INVALID_IN_STATE due to SoftPrivacy mode set to On",
                )
                pass

        else:
            self.skip_step(11)
            self.skip_step(12)

        self.step(13)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.SnapshotStreamDeallocate(snapshotStreamID=aStreamID))
        except InteractionModelError as e:
            asserts.fail(f"Expected SnapshotStreamDeallocate to succeed, but it failed with status: {e.status}")
            pass

        self.step(14)
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams
        )
        logger.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        asserts.assert_equal(len(aAllocatedSnapshotStreams), 0, "The number of allocated snapshot streams in the list is not 0.")

        self.step(15)
        try:
            captureSnapshotResponse = await self.send_single_cmd(
                cmd=commands.CaptureSnapshot(requestedResolution=aResolution), endpoint=endpoint
            )
            asserts.assert_true(False, "Unexpected success when expecting NOT_FOUND due to 0 allocated snapshot streams")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.NotFound, "Unexpected error returned when expecting NOT_FOUND due to 0 allocated snapshot streams"
            )
            pass


if __name__ == "__main__":
    default_matter_test_main()
