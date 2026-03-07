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
#     app: ${ALL_CLUSTERS_APP}
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

import numpy as np
from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

min_value_uint8 = np.iinfo(np.uint8).min
max_value_uint8 = np.iinfo(np.uint8).max
min_value_uint16 = np.iinfo(np.uint16).min
max_value_uint16 = np.iinfo(np.uint16).max
min_value_uint32 = np.iinfo(np.uint32).min
max_value_uint32 = np.iinfo(np.uint32).max

HUMANACTIVITYNAMESPACEID = 0x4B
HUMANACTIVITYMAXTAGNUMBER = 0X09
OBJECTIDENTIFICATIONNAMESPACEID = 0x49
OBJECTIDENTIFICATIONMAXTAGNUMBER = 0X0C
SOUNDIDENTIFICATIONNAMESPACEID = 0x4A
SOUNDIDENTIFICATIONMAXTAGNUMBER = 0X15


class TC_ACS_2_1(MatterBaseTest):
    def desc_TC_ACS_2_1(self) -> str:
        return "[TC-ACS-2.1] Attributes with DUT as a server"

    def pics_TC_ACS_2_1(self):
        return ["ACS.S"]

    def steps_TC_ACS_2_1(self) -> list[TestStep]:
        return [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads the HumanActivityDetected attribute.",
                     "If DUT supports HumanActivity feature, then TH reads the HumanActivityDetected attribute containing Boolean True or False."),
            TestStep("3", "TH reads the ObjectIdentified attribute.",
                     "If DUT supports ObjectIdentification feature, then TH reads the ObjectIdentified attribute containing Boolean True or False."),
            TestStep("4", "TH reads the AudioContextDetected attribute.",
                     "If DUT supports SoundIdentification feature, then TH reads the AudioContextDetected attribute containing Boolean True or False."),
            TestStep("5", "TH reads the AmbientContextType attribute.",
                     "Verify that DUT response contains the list size is less than SimultaneousDetectionLimit and AmbientContextSensed struct data field including namespace ID and tag ID from IdentifiedObject or IdentifiedHumanActivity or IdentifiedSound namespaces."),
            TestStep("5a", "If DUT supports DetectionStartTime field, TH reads DetectedStartTime field.", "An epoch-s data type is read"),
            TestStep("6", "If DUT supports HumanActivity or ObjectIdentification or SoundIdentification, TH reads the AmbientContextTypeSupported attribute.",
                     "Verify that the DUT response contains SemanticTag struct data field including namespace ID and tag ID from IdentifiedObject or IdentifiedHumanActivity or IdentifiedSound namespaces.",
                     "Verify that the list size is less than equal to 50."),
            TestStep("7", "If DUT supports ObjectCounting and ObjectIdentification feature, then TH reads the ObjectCountReached attribute containing Boolean True or False."),
            TestStep("8", "If DUT supports ObjectCounting and ObjectIdentification feature, then TH reads the ObjectCountConfig attribute.",
                     "Verify that DUT response contains the list of ObjectCountDataStruct entries and its CountingObject field is SemanticTagStruct data type containing namespace ID and tag ID from IdentifiedObject.",
                     "Verify that the ObjectCountThreshold field is an uint16 value."),
            TestStep("9", "If DUT supports ObjectCounting and ObjectIdentification feature and ObjectCount attribute, verity that DUT response contains uint16 value."),
            TestStep("10", "TH reads the SimultaneousDetectionLimit attribute.",
                     "Verify that the DUT response contains a value less than equal to 10."),
            TestStep("11", "TH reads the HoldTime attribute.",
                     "Verify that DUT response contains an uint16 value ranging between HoldTimeLimits.HoldTimeMin and HoldTimeLimits.HoldTimeMax"),
            TestStep("12", "TH reads the HoldTimeLimits attribute.",
                     "Verify that DUT response contains HoldTimeMin (>=1), HolTimeMax (min maxOf(HoldTimeMin, 10)) and HoldTimeDefault (between HoldTimeMin and HoldTimeMax)"),
            TestStep("13", "If DUT supports PredictedActivity feature, then TH reads the PredictedActivity attribute.",
                     "Verify that DUT response contains StartTimestamp that is epoch-s data and less than equal to EndTimestamp-1, and EndTimestamp that is epoch-s data and greater than equal to StartTimestamp-1, and verify that DUT response contains Confidence field that is a percentage data between 0 and 100."),
            TestStep("13a", "If DUT supports HumanActivity or ObjectIdentification or SoundIdentification, then TH reads a list of SemanticTagStruct data.",
                     "They include namespace ID and tag ID from IdentifiedObject or IdentifiedHumanActivity or IdentifiedSound namespaces."),
            TestStep("13b", "If DUT supports ObjectCounting feature, then then TH reads CrowdDetected field.", "Boolean true or false is read.",
                     "If DUT supports ObjectCounting feature and CrowdCount field, then TH reads an uint8 value between 1 and 254.")
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AmbientContextSensing))
    async def test_TC_ACS_2_1(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.AmbientContextSensing
        attr = Clusters.AmbientContextSensing.Attributes

        self.step("1")
        # Commission DUT - already done
        # Implicit step to get the feature map to ensure attribute operations
        # are performed on supported features
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        log.info(f"Rx'd FeatureMap: {aFeatureMap}")
        self.HumanActivitySupported = aFeatureMap & cluster.Bitmaps.Feature.kHumanActivity
        log.info(f"Rx'd HumanActivitySupported: {self.HumanActivitySupported}")
        self.ObjectCountingSupported = aFeatureMap & cluster.Bitmaps.Feature.kObjectCounting
        log.info(f"Rx'd ObjectCountingSupported: {self.ObjectCountingSupported}")
        self.ObjectIdentificationSupported = aFeatureMap & cluster.Bitmaps.Feature.kObjectIdentification
        log.info(f"Rx'd ObjectIdentificationSupported: {self.ObjectIdentificationSupported}")
        self.SoundIdentificationSupported = aFeatureMap & cluster.Bitmaps.Feature.kSoundIdentification
        log.info(f"Rx'd SoundIdentificationSupported: {self.SoundIdentificationSupported}")
        self.PredictedActivitySupported = aFeatureMap & cluster.Bitmaps.Feature.kPredictedActivity
        log.info(f"Rx'd PredictedActivitySupported: {self.PredictedActivitySupported}")

        if self.HumanActivitySupported:
            self.step("2")
            humanActivityDetected = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.HumanActivityDetected
            )
            log.info(f"Rx'd HumanActivityDetected: {humanActivityDetected}")
            asserts.assert_true(humanActivityDetected is True | humanActivityDetected is False,
                                "Expected True or False Boolean value.")
        else:
            log.info("HumanActivity Feature not supported. Test steps skipped")
            self.skip_step("2")

        if self.ObjectIdentificationSupported:
            self.step("3")
            objectIdentified = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.ObjectIdentified
            )
            log.info(f"Rx'd ObjectIdentified: {objectIdentified}")
            asserts.assert_true(objectIdentified is True | objectIdentified is False,
                                "Expected True or False Boolean value.")
        else:
            log.info("ObjectIdentification Feature not supported. Test steps skipped")
            self.skip_step("3")

        if self.SoundIdentificationSupported:
            self.step("4")
            audioContextDetected = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AudioContextDetected
            )
            log.info(f"Rx'd AudioContextDetected: {audioContextDetected}")
            asserts.assert_true(audioContextDetected is True | audioContextDetected is False,
                                "Expected True or False Boolean value.")
        else:
            log.info("SoundIdentification Feature not supported. Test steps skipped")
            self.skip_step("4")

        if self.HumanActivitySupported or self.ObjectIdentificationSupported or self.SoundIdentificationSupported:
            self.step("5")
            ambientContextType = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AmbientContextType
            )
            log.info(f"Rx'd AmbientContextType: {ambientContextType}")
            nsID = ambientContextType.ambientContextSensed.namespaceID
            tagID = ambientContextType.ambientContextSensed.tag

            if self.HumanActivitySupported:
                asserts.assert_equal(nsID, HUMANACTIVITYNAMESPACEID, "Not Identified Human Activity Namespace ID")
                asserts.assert_less_equal(tagID, HUMANACTIVITYMAXTAGNUMBER, "Tag number doesn't exit.")

            if self.ObjectIdentificationSupported:
                asserts.assert_equal(nsID, OBJECTIDENTIFICATIONNAMESPACEID, "Not Identified Object Namespace ID")
                asserts.assert_less_equal(tagID, OBJECTIDENTIFICATIONMAXTAGNUMBER, "Tag number doesn't exit.")

            if self.SoundIdentificationSupported:
                asserts.assert_equal(nsID, SOUNDIDENTIFICATIONNAMESPACEID, "Not Identifid Sound Namespace ID")
                asserts.assert_less_equal(tagID, SOUNDIDENTIFICATIONMAXTAGNUMBER, "Tag number doesn't exit.")

            if "detectionStartTime" in ambientContextType:
                self.step("5a")
                asserts.assert_less_equal(min_value_uint32, ambientContextType.detectionStartTime,
                                          "DetectionStartTime is not uint32.")
                asserts.assert_less_equal(ambientContextType.detectionStartTime, max_value_uint32,
                                          "DetectionStartTime is not uint32.")
        else:
            log.info("HumanActivity, ObjectIdentification, SoundIdentification Feature not supported. Test steps skipped")
            self.skip_step("5")
            self.skip_step("5a")

        if self.HumanActivitySupported | self.ObjectIdentificationSupported | self.SoundIdentificationSupported:
            self.step("6")
            ambientContextTypeSupported = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AmbientContextTypeSupported
            )
            log.info(f"Rx'd AmbientContextTypeSupported: {ambientContextTypeSupported}")
            asserts.assert_less_equal(len(ambientContextTypeSupported), 50,
                                      "AmbientContextTypeSupported should be less than equalt to 50.")

            for acts in ambientContextTypeSupported:
                nsID = acts.namespaceID
                tagID = acts.tag

                if self.HumanActivitySupported:
                    asserts.assert_equal(nsID, HUMANACTIVITYNAMESPACEID, "Not Identified Human Activity Namespace ID")
                    asserts.assert_less_equal(tagID, HUMANACTIVITYMAXTAGNUMBER, "Tag number doesn't exit.")

                if self.ObjectIdentificationSupported:
                    asserts.assert_equal(nsID, OBJECTIDENTIFICATIONNAMESPACEID, "Not Identified Object Namespace ID")
                    asserts.assert_less_equal(tagID, OBJECTIDENTIFICATIONMAXTAGNUMBER, "Tag number doesn't exit.")

                if self.SoundIdentificationSupported:
                    asserts.assert_equal(nsID, SOUNDIDENTIFICATIONNAMESPACEID, "Not Identifid Sound Namespace ID")
                    asserts.assert_less_equal(tagID, SOUNDIDENTIFICATIONMAXTAGNUMBER, "Tag number doesn't exit.")
        else:
            log.info("HumanActivity, ObjectIdentification, SoundIdentification Feature not supported. Test steps skipped")
            self.skip_step("6")

        if self.ObjectCountingSupported & self.ObjectIdentificationSupported:
            self.step("7")
            objectCountReached = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.ObjectCountReached
            )
            log.info(f"Rx'd ObjectCountReached: {objectCountReached}")
            asserts.assert_true(objectCountReached is True | objectCountReached is False,
                                "Expected True or False Boolean value.")
        else:
            log.info("Object Counting & Object Identification Feature not supported. Test steps skipped")
            self.skip_step("7")

        if self.ObjectCountingSupported & self.ObjectIdentificationSupported:
            self.step("8")
            objectCountConfig = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.ObjectCountConfig
            )
            nsID = objectCountConfig.countingObject.namespaceID
            tagID = objectCountConfig.countingObject.tag

            # object should come from Identified Object namespace
            log.info(f"Rx'd ObjectCountConfig: {objectCountConfig}")
            asserts.assert_equal(nsID, OBJECTIDENTIFICATIONNAMESPACEID, "Not Identified Object Namespace ID")
            asserts.assert_less_equal(tagID, OBJECTIDENTIFICATIONMAXTAGNUMBER, "Tag number doesn't exit.")

            # ObjectCountThreshold should be uint16
            asserts.assert_true((type(objectCountConfig.objectCountThreshold) is int), "Threshold value should be uint16 data.")
            asserts.assert_less_equal(min_value_uint16, objectCountConfig.objectCountThreshold,
                                      "Threshold value should be uint16 data.")
            asserts.assert_less_equal(objectCountConfig.objectCountThreshold, max_value_uint16,
                                      "Threshold value should be uint16 data.")
        else:
            log.info("Object Counting & Object Identification are not supported. Test steps skipped")
            self.skip_step("8")

        if self.ObjectCountingSupported & self.ObjectIdentificationSupported:
            self.step("9")

            # ObjectCount should be uint16
            if "objectCount" in objectCountConfig:
                asserts.assert_true((type(objectCountConfig.objectCount) is int), "ObjectCount value should be uint16 data.")
                asserts.assert_less_equal(min_value_uint16, objectCountConfig.objectCount,
                                          "ObjectCount value should be uint16 data.")
                asserts.assert_less_equal(objectCountConfig.objectCount, max_value_uint16,
                                          "ObjectCount value should be uint16 data.")
        else:
            log.info("Object Counting & Object Identification are not supported. Test steps skipped")
            self.skip_step("9")

        self.step("10")
        simultaneousDetectionLimit = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.SimultaneousDetectionLimit
        )
        log.info(f"Rx'd AudioContextDetected: {simultaneousDetectionLimit}")
        asserts.assert_less_equal(1, simultaneousDetectionLimit, "SimultaneousDetectionLimit is not within 1 and 10.")
        asserts.assert_less_equal(simultaneousDetectionLimit, 10, "SimultaneousDetectionLimit is not within 1 and 10.")

        self.step("11")
        holdTime = await self.read_single_attribute_expect_success(endpoint=endpoint, attribute=attr.HoldTime)
        holdTimeLimits = await self.read_single_attribute_expect_success(endpoint=endpoint, attribute=attr.HoldTimeLimits)
        log.info(f"Rx'd HoldTime: {holdTime}")
        asserts.assert_less_equal(holdTimeLimits.holdTimeMin, holdTime, "Expected to be between HoldTimeMin and HoldTimeMax.")
        asserts.assert_less_equal(holdTime, holdTimeLimits.holdTimeMax, "Expected to be between HoldTimeMin and HoldTimeMax.")

        self.step("12")
        log.info(f"Rx'd HoldTimeLimits: {holdTimeLimits}")
        asserts.assert_less_equal(holdTimeLimits.holdTimeMin, 1,
                                  "Expected HoldTimeMin to be greater than equal to 1.")

        minformax = max(holdTimeLimits.holdTimeMin, 10)
        asserts.assert_less_equal(holdTimeLimits.holdTimeMax, minformax,
                                  "Expected HoldTimeMax to be greater than equal to max(holdTimeLimits.holdTimeMin, 10).")

        asserts.assert_less_equal(holdTimeLimits.holdTimeMin, holdTimeLimits.holdTimeDefault,
                                  "Expected to be between HoldTimeMin and HoldTimeMax.")
        asserts.assert_less_equal(holdTimeLimits.holdTimeDefault, holdTimeLimits.holdTimeMax,
                                  "Expected to be between HoldTimeMin and HoldTimeMax.")

        if self.PredictedActivitySupported:
            self.step("13")
            predictedActivity = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.PredictedActivity
            )
            log.info(f"Rx'd PredictedActivity: {predictedActivity}")

            # less than 20
            asserts.assert_less_equal(len(predictedActivity), 20, "PredictedActivity should be less than 20.")

            startTime = predictedActivity.startTimestamp
            endTime = predictedActivity.endTimestamp

            # StartTimestamp should be epoch-s (uint32)
            asserts.assert_less_equal(startTime, endTime-1, "StartTimestamp must be less than EndTimestamp.")

            # EndTimestamp should be epoch-s (uint32)
            asserts.assert_less_equal(endTime, startTime+1, "EndTimestamp must be greater than StartTimestamp.")

            # Confidence
            asserts.assert_less_equal(0, predictedActivity.confidence,
                                      "Expected the percentage to be between 0 and 100.")
            asserts.assert_less_equal(predictedActivity.confidence, 100,
                                      "Expected the percentage to be between 0 and 100.")

            if self.HumanActivitySupported | self.ObjectIdentificationSupported | self.SoundIdentificationSupported:
                self.step("13a")

                # AmbientContextType
                asserts.assert_less_equal(len(predictedActivity.ambientContextType), "AmbientContextType should be less than 100.")

                for acts in predictedActivity.ambientContextType:
                    nsID = acts.namespaceID
                    tagID = acts.tag

                    if self.HumanActivitySupported:
                        asserts.assert_equal(nsID, HUMANACTIVITYNAMESPACEID, "Not Identified Human Activity Namespace ID")
                        asserts.assert_less_equal(tagID, HUMANACTIVITYMAXTAGNUMBER, "Tag number doesn't exit.")

                    if self.ObjectIdentificationSupported:
                        asserts.assert_equal(nsID, OBJECTIDENTIFICATIONNAMESPACEID, "Not Identified Object Namespace ID")
                        asserts.assert_less_equal(tagID, OBJECTIDENTIFICATIONMAXTAGNUMBER, "Tag number doesn't exit.")

                    if self.SoundIdentificationSupported:
                        asserts.assert_equal(nsID, SOUNDIDENTIFICATIONNAMESPACEID, "Not Identifid Sound Namespace ID")
                        asserts.assert_less_equal(tagID, SOUNDIDENTIFICATIONMAXTAGNUMBER, "Tag number doesn't exit.")
            else:
                log.info("Test step 13a skipped")
                self.skip_step("13a")

            if self.ObjectCountingSupported:
                self.step("13b")
                # CrowdDetected
                log.info(f"Rx'd CrowdDetected: {predictedActivity.crowdDetected}")
                asserts.assert_true(predictedActivity.crowdDetected is True |
                                    predictedActivity.crowdDetected is False, "Expected True or False Boolean value.")

                # CrowdCount
                if "crowdCount" in predictedActivity:
                    log.info(f"Rx'd CrowdCount: {predictedActivity.crowdCount}")
                    asserts.assert_less_equal(min_value_uint8, predictedActivity.crowdCount,
                                              "CrowdCount is expected to be between 1 and 254.")
                    asserts.assert_less_equal(predictedActivity.crowdCount, max_value_uint8,
                                              "CrowdCount is expected to be between 1 and 254.")
            else:
                log.info("Test step 13b skipped")
                self.skip_step("13b")

        else:
            log.info("PredictedActivity Feature not supported. Test steps skipped")
            self.skip_step("13")
            self.skip_step("13a")
            self.skip_step("13b")


if __name__ == "__main__":
    default_matter_test_main()
