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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/acs_fifo_3_2
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#       --app-pipe /tmp/acs_fifo_3_2
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import ast
import asyncio
import logging
import time

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# Namespace in integer (not hex)
HUMAN_ACTIVITY_NAMESPACE_ID = 75  # 0x4B
OBJECT_IDENTIFICATION_NAMESPACE_ID = 73  # 0x49
SOUND_IDENTIFICATION_NAMESPACE_ID = 74  # 0x4A


class TC_ACS_3_2(MatterBaseTest):
    def desc_TC_ACS_3_2(self) -> str:
        return "[TC-ACS-3.2] Same Continuous Detection and HoldTimeMax Functionality with DUT as a server"

    def pics_TC_ACS_3_2(self):
        return ["ACS.S"]

    def steps_TC_ACS_3_2(self) -> list[TestStep]:
        return [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH establishes a wildcard subscription to all attributes on Ambient Context Sensing Cluster on the endpoint under test with minIntervalFloor set to 0, MaxIntervalCeiling set to 30 and KeepSubscriptions set to false"),
            TestStep("3", "TH writes DUT HoldTime attribute with 30 seconds",
                     "Verify that TH reads the HoldTime attribute with 30."),
            TestStep("4", "Trigger one of DUT supporting ambient sensing features"),
            TestStep("5", "TH reads the AmbientContextType attribute.",
                     "Verify that DUT response contains the AmbientContextSensed struct data including the namespace ID and its tag ID of test step 4"),
            TestStep("6", "Within HoldTime duration of the step 4, trigger the DUT with the same ambient sensing feature."),
            TestStep("7", "TH reads the AmbientContextType attribute.",
                     "Verify that DUT response contains the AmbientContextSensed struct data including the namespace ID and its tag ID from the step 6.",
                     "Verify that DUT response contains the size of AmbientContextType list is 1."),
            TestStep("8", "Wait until HoldTime seconds are passed from the step 6 execution."),
            TestStep("9", "TH reads the AmbientContextType attribute and check a Boolean attribute related to the step 6.",
                     "Verify that DUT response contains the Boolean attribute (HumanActivityDetected, ObjectIdentified, AudioContextDetected) is read False.")
        ]

    def setup_test(self):
        super().setup_test()
        self.is_ci = self.matter_test_config.global_test_params.get('simulate_ambientsensing', True)

    # Sends and out-of-band command to the all-clusters-app
    def write_to_app_pipe(self, command):

        if self.is_ci:
            self.app_pipe = "/tmp/acs_fifo_3_2"

        with open(self.app_pipe, "w") as app_pipe:
            app_pipe.write(command + "\n")
        # Delay for pipe command to be processed (otherwise tests are flaky)
        time.sleep(0.001)

    @run_if_endpoint_matches(has_cluster(Clusters.AmbientContextSensing))
    async def test_TC_ACS_3_2(self):
        node_id = self.dut_node_id
        endpoint = self.get_endpoint()
        cluster = Clusters.AmbientContextSensing
        attr = Clusters.AmbientContextSensing.Attributes
        dev_ctrl = self.default_controller

        self.step("1", "Commissioning, already done", is_commissioning=True)
        # Commission DUT - already done
        # Implicit step to get the feature map to ensure attribute operations
        # are performed on supported features
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        log.info("Rx'd FeatureMap: %s", {aFeatureMap})
        self.HumanActivitySupported = ((aFeatureMap & cluster.Bitmaps.Feature.kHumanActivity) != 0)
        log.info("Rx'd HumanActivitySupported: %s", {self.HumanActivitySupported})
        self.ObjectCountingSupported = ((aFeatureMap & cluster.Bitmaps.Feature.kObjectCounting) != 0)
        log.info("Rx'd ObjectCountingSupported: %s", {self.ObjectCountingSupported})
        self.ObjectIdentificationSupported = ((aFeatureMap & cluster.Bitmaps.Feature.kObjectIdentification) != 0)
        log.info("Rx'd ObjectIdentificationSupported: %s", {self.ObjectIdentificationSupported})
        self.SoundIdentificationSupported = ((aFeatureMap & cluster.Bitmaps.Feature.kSoundIdentification) != 0)
        log.info("Rx'd SoundIdentificationSupported: %s", {self.SoundIdentificationSupported})
        self.PredictedActivitySupported = ((aFeatureMap & cluster.Bitmaps.Feature.kPredictedActivity) != 0)
        log.info("Rx'd PredictedActivitySupported: %s", {self.PredictedActivitySupported})

        self.step("2", "TH establishes a wildcard subscription to all attributes on Ambient Context Sensing Cluster on the endpoint under test with minIntervalFloor set to 0, MaxIntervalCeiling set to 30 and KeepSubscriptions set to false")
        # Add AmbientContextSupported elements based on DUT capability for ci
        # Human activity walking, Object identification person, Audio identification barking
        if self.is_ci:
            self.write_to_app_pipe(
                '{"Name":"SetAmbientContextSupport","EndpointId":1,"AmbientContextType":[{"TypeId":73, "TagId":3},{"TypeId":74, "TagId":4},{"TypeId":75,"TagId":3}]}')

        # subscription setup for the following trigger testing
        attrib_listener = AttributeSubscriptionHandler(expected_cluster=cluster)
        await attrib_listener.start(dev_ctrl, node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)
        attrib_listener.reset()

        self.step("3")
        log.info("\n\n Holdtime changed to 30 seconds. \n\n")

        holdTime_input = 30  # 30 seconds
        await self.write_single_attribute(attr.HoldTime(holdTime_input))

        # attribute subscription check (in replacement of reading)
        subscription_expected = attrib_listener.attribute_reports[cluster.Attributes.HoldTime]
        log.info("Rx'd subscription_expected: %s", {subscription_expected})
        holdTime_sub = subscription_expected[0].value
        log.info("Rx'd holdTime_sub: %s", {holdTime_sub})
        asserts.assert_true(holdTime_sub, holdTime_input, "Subscribed HoldTime is different from the input HoldTime.")
        attrib_listener.reset()

        self.step("4")
        log.info("\n\n First trigger. \n\n")

        # CI call to trigger on
        if self.is_ci:
            # Human activity walking for ci
            namespaceID1 = HUMAN_ACTIVITY_NAMESPACE_ID
            tag1 = 3  # walking
            self.write_to_app_pipe(
                '{"Name":"AddAmbientContextDetect", "EndpointId":{endpoint}, "AmbientContextType":[{"TypeId":75, "TagId":3}]}')
        else:
            # Trigger the ambient sensor to change AmbientContextType.AmbientContextSensed.NamespaceID
            # and AmbientContextType.AmbientContextSensed.Tag => TESTER ACTION on DUT
            user_data = self.wait_for_user_input(
                prompt_msg="Type in namespace ID and tag ID of a desired ambient sensing event (ex [0x4B, 0x03]) and press ENTER after the desired ambient sensing is actuated.")
            # user_data = "[0x4B, 0x03]"
            list_dec = ast.literal_eval(user_data)  # convert string hex to decimal
            namespaceID1 = list_dec[0]
            tag1 = list_dec[1]
            log.info("user input: %s %s", {namespaceID1}, {tag1})

        # Add 1 second delay to make sure it's done
        await asyncio.sleep(1)

        self.step("5")
        # Check the boolean attribute subscription
        if namespaceID1 == HUMAN_ACTIVITY_NAMESPACE_ID:
            subscription_expected1 = attrib_listener.attribute_reports[cluster.Attributes.HumanActivityDetected]
            log.info("Rx'd subscription_expected1: %s", {subscription_expected1})
            humanActivityDetected = subscription_expected1[0].value
            log.info("Rx'd humanActivityDetected: %s", {humanActivityDetected})
            asserts.assert_true(humanActivityDetected, "Failed to get HumanActivityDetected being True.")

        elif namespaceID1 == OBJECT_IDENTIFICATION_NAMESPACE_ID:
            # AmbientContextType attribute subscription check for PIXIT.ACS.AmbientContextSensed_2 = Object identification person
            subscription_expected2 = attrib_listener.attribute_reports[cluster.Attributes.ObjectIdentified]
            log.info("Rx'd subscription_expected2: %s", {subscription_expected2})
            objectIdentified = subscription_expected2[0].value
            asserts.assert_true(objectIdentified, "Failed to get ObjectIdentified being True.")

        elif namespaceID1 == SOUND_IDENTIFICATION_NAMESPACE_ID:
            # AmbientContextType attribute subscription check for PIXIT.ACS.AmbientContextSensed_2 = Object identification person
            subscription_expected3 = attrib_listener.attribute_reports[cluster.Attributes.AudioContextDetected]
            log.info("Rx'd subscription_expected3: %s", {subscription_expected3})
            audioContextDetected = subscription_expected3[0].value
            log.info("Rx'd audioContextDetected: %s", {audioContextDetected})
            asserts.assert_true(audioContextDetected, "Failed to get audioContextDetected being True.")

        # Read AmbientContextType attribute
        ambientContextType_read = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AmbientContextType)
        log.info("Rx'd AmbientContextType_read: %s", {ambientContextType_read})

        # check attribute read
        nsID_1_read = ambientContextType_read[0].ambientContextSensed[0].namespaceID
        tagID_1_read = ambientContextType_read[0].ambientContextSensed[0].tag
        asserts.assert_equal(nsID_1_read, namespaceID1, "Namespace ID and Tag ID must match.")
        asserts.assert_equal(tagID_1_read, tag1, "Namespace ID and Tag ID must match.")

        # check the subscription of AmbientContextType attribute
        subscription_expected = attrib_listener.attribute_reports[cluster.Attributes.AmbientContextType][0].value
        asserts.assert_true(subscription_expected[0].ambientContextSensed[0].namespaceID == namespaceID1,
                            f"Unexpected namespaceID, {subscription_expected[0].ambientContextSensed[0].namespaceID}, exp {namespaceID1}")
        asserts.assert_true(subscription_expected[0].ambientContextSensed[0].tag == tag1,
                            f"Unexpected tag, {subscription_expected[0].ambientContextSensed[0].tag}, exp {tag1}")

        # attrib_listener.reset()

        self.step("6")
        # Trigger the sensor with same ambient context within HoldTime duration
        # Human activity walking for ci
        log.info("\n\n Re-triggering another same type ambient sensing event \n\n")

        # CI call to trigger on
        if self.is_ci:
            self.write_to_app_pipe(
                '{"Name":"AddAmbientContextDetect", "EndpointId":{endpoint}, "AmbientContextType":[{"TypeId":75, "TagId":3}]}')
        else:
            # Trigger the ambient sensor to change AmbientContextType.AmbientContextSensed.NamespaceID
            # and AmbientContextType.AmbientContextSensed.Tag => TESTER ACTION on DUT
            user_data = self.wait_for_user_input(
                prompt_msg="Type in namespace ID and tag ID of the previous ambient sensing event (ex [0x4B, 0x03]) and press ENTER after the desired ambient sensing is actuated.")
            # user_data = "[0x4B, 0x03]"
            list_dec = ast.literal_eval(user_data)  # convert string hex to decimal
            namespaceID1 = list_dec[0]
            tag1 = list_dec[1]
            log.info("user input: %s %s", {namespaceID1}, {tag1})

            self.wait_for_user_input(
                prompt_msg="Type any letter and press ENTER after the same type ambient sensing event is re-triggered.")

        # Add 1 second delay to make sure it's done
        await asyncio.sleep(1)

        self.step("7")
        ambientContextType = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AmbientContextType)
        log.info("Rx'd AmbientContextType: %s", {ambientContextType})

        # Same trigger shouldn't add to the list
        asserts.assert_equal(len(ambientContextType), 1, "AmbientContextType needs to be the size of 1.")

        # Check the boolean attribute subscription
        if namespaceID1 == HUMAN_ACTIVITY_NAMESPACE_ID:
            subscription_expected1 = attrib_listener.attribute_reports[cluster.Attributes.HumanActivityDetected]
            log.info("Rx'd subscription_expected1: %s", {subscription_expected1})
            humanActivityDetected = subscription_expected1[0].value
            log.info("Rx'd humanActivityDetected: %s", {humanActivityDetected})
            asserts.assert_true(humanActivityDetected, "Failed to get HumanActivityDetected being True.")

        elif namespaceID1 == OBJECT_IDENTIFICATION_NAMESPACE_ID:
            # AmbientContextType attribute subscription check for PIXIT.ACS.AmbientContextSensed_2 = Object identification person
            subscription_expected2 = attrib_listener.attribute_reports[cluster.Attributes.ObjectIdentified]
            log.info("Rx'd subscription_expected2: %s", {subscription_expected2})
            objectIdentified = subscription_expected2[0].value
            log.info("Rx'd objectIdentified: %s", {objectIdentified})
            asserts.assert_true(objectIdentified, "Failed to get ObjectIdentified being True.")

        elif namespaceID1 == SOUND_IDENTIFICATION_NAMESPACE_ID:
            # AmbientContextType attribute subscription check for PIXIT.ACS.AmbientContextSensed_2 = Object identification person
            subscription_expected3 = attrib_listener.attribute_reports[cluster.Attributes.AudioContextDetected]
            log.info("Rx'd subscription_expected3: %s", {subscription_expected3})
            audioContextDetected = subscription_expected3[0].value
            log.info("Rx'd audioContextDetected: %s", {audioContextDetected})
            asserts.assert_true(audioContextDetected, "Failed to get audioContextDetected being True.")

        # check attribute read
        nsID_1_read = ambientContextType_read[0].ambientContextSensed[0].namespaceID
        tagID_1_read = ambientContextType_read[0].ambientContextSensed[0].tag
        asserts.assert_equal(nsID_1_read, namespaceID1, "Namespace ID must match.")
        asserts.assert_equal(tagID_1_read, tag1, "Tag ID must match.")

        # check the subscription of AmbientContextType attribute
        subscription_expected = attrib_listener.attribute_reports[cluster.Attributes.AmbientContextType][0].value
        asserts.assert_true(subscription_expected[0].ambientContextSensed[0].namespaceID == namespaceID1,
                            f"Unexpected namespaceID, {subscription_expected[0].ambientContextSensed[0].namespaceID}, exp {namespaceID1}")
        asserts.assert_true(subscription_expected[0].ambientContextSensed[0].tag == tag1,
                            f"Unexpected tag, {subscription_expected[0].ambientContextSensed[0].tag}, exp {tag1}")

        attrib_listener.reset()

        self.step("8")
        # make sure to wait until HoldTime duration expires
        # CI call to trigger on
        if self.is_ci:
            holdtime_dut = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.HoldTime)
            await asyncio.sleep(holdtime_dut)
        else:
            self.wait_for_user_input(
                prompt_msg="Type any letter and press ENTER after HoldTime duration from the step 4 has passed.")

        self.step("9")
        # check the subscription of AmbientContextType attribute (to be empty list)
        subscription_expected = attrib_listener.attribute_reports[cluster.Attributes.AmbientContextType][0].value
        asserts.assert_true(len(subscription_expected) == 0, "AmbientContext attribute is not empty.")

        # check boolean attributes
        if humanActivityDetected & self.HumanActivitySupported:
            # subscription check
            subscription_expected1 = attrib_listener.attribute_reports[cluster.Attributes.HumanActivityDetected]
            humanActivityDetected = subscription_expected1[0].value
            log.info("Rx'd humanActivityDetected: %s", {humanActivityDetected})
            asserts.assert_true(humanActivityDetected is False, "Failed to get HumanActivityDetected being False.")
        elif objectIdentified & self.ObjectIdentificationSupported:
            # subscription check
            subscription_expected2 = attrib_listener.attribute_reports[cluster.Attributes.ObjectIdentified]
            log.info("Rx'd subscription_expected2: %s", {subscription_expected2})
            objectIdentified = subscription_expected2[0].value
            log.info("Rx'd objectIdentified: %s", {objectIdentified})
            asserts.assert_true(objectIdentified is False, "Failed to get ObjectIdentified being False.")
        elif audioContextDetected & self.SoundIdentificationSupported:
            # subscription check
            subscription_expected3 = attrib_listener.attribute_reports[cluster.Attributes.AudioContextDetected]
            log.info("Rx'd subscription_expected3: %s", {subscription_expected3})
            audioContextDetected = subscription_expected3[0].value
            log.info("Rx'd audioContextDetected: %s", {audioContextDetected})
            asserts.assert_true(not audioContextDetected, "Failed to get audioContextDetected being False.")

        attrib_listener.reset()


if __name__ == "__main__":
    default_matter_test_main()
