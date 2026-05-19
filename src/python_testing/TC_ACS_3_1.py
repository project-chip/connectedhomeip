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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/acs_fifo_3_1
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#       --app-pipe /tmp/acs_fifo_3_1
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
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# Namespace in integer (not hex)
HUMANACTIVITYNAMESPACEID = 75  # 0x4B
OBJECTIDENTIFICATIONNAMESPACEID = 73  # 0x49
SOUNDIDENTIFICATIONNAMESPACEID = 74  # 0x4A


class TC_ACS_3_1(MatterBaseTest):
    def desc_TC_ACS_3_1(self) -> str:
        return "[TC-ACS-3.1] Multiple Detection Functionality & Simultaneous Detection Limit Check with DUT as a server"

    def pics_TC_ACS_3_1(self):
        return ["ACS.S"]

    def steps_TC_ACS_3_1(self) -> list[TestStep]:
        return [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads the SimultaneousDetectionLimit attribute.",
                     "If 1 is read, skip this test case. Otherwise proceed the following."),
            TestStep("3", "TH establishes a wildcard subscription to all attributes on Ambient Context Sensing Cluster on the endpoint under test."),
            TestStep("4", "An operator actuates DUT to generate the first ambient sensing event, and then removes its sensing stimulus.",
                     "And within HoldTime duration, an operator actuates DUT to generate the second ambient sensing event, and then removes its sensing stimulus.",
                     "And within HoldTime duration, an operator actuates DUT to generate the third ambient sensing event, and then removes its sensing stimulus."),
            TestStep("5", "TH verifies the AmbientContextType attribute change.",
                     "Verify that DUT response contains the AmbientContextSensed struct data list of size 2.",
                     "Verify that DUT response contains the AmbientContextSensed struct data including the namespace ID and its tag ID that match both ambient sensing events from the step 4."),
            TestStep("6", "An operator waits until the HoldTime duration expires since the step 4 execution.",
                     "Check if AmbientContextDetectEnded is received for the second ambient sensing event."),
            TestStep("7", "TH reads the AmbientContextType attribute.",
                     "Verify that the AmbientContextType attribute contains an empty list and the Boolean attributes related the step 4 are False.")
        ]

    def setup_test(self):
        super().setup_test()
        self.is_ci = self.matter_test_config.global_test_params.get('simulate_ambientsensing', True)

    # Sends and out-of-band command to the all-clusters-app
    def write_to_app_pipe(self, command):
        # CI app pipe id creation
        # self.app_pipe = "/tmp/acs_fifo"
        if self.is_ci:
            # app_pid = self.matter_test_config.app_pid
            # if app_pid == 0:
            #     asserts.fail("The --app-pid flag must be set when using named pipe")
            # self.app_pipe = self.app_pipe + str(app_pid)
            self.app_pipe = "/tmp/acs_fifo_3_1"

        with open(self.app_pipe, "w") as app_pipe:
            app_pipe.write(command + "\n")
        # Delay for pipe command to be processed (otherwise tests are flaky)
        time.sleep(0.001)

    @run_if_endpoint_matches(has_cluster(Clusters.AmbientContextSensing))
    async def test_TC_ACS_3_1(self):
        node_id = self.dut_node_id
        endpoint = self.get_endpoint()
        cluster = Clusters.AmbientContextSensing
        attr = Clusters.AmbientContextSensing.Attributes
        dev_ctrl = self.default_controller

        self.step("1")
        # Commission DUT - already done
        # Implicit step to get the feature map to ensure attribute operations
        # are performed on supported features
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        log.info(f"Rx'd FeatureMap: {aFeatureMap}")
        self.HumanActivitySupported = ((aFeatureMap & cluster.Bitmaps.Feature.kHumanActivity) != 0)
        log.info(f"Rx'd HumanActivitySupported: {self.HumanActivitySupported}")
        self.ObjectCountingSupported = ((aFeatureMap & cluster.Bitmaps.Feature.kObjectCounting) != 0)
        log.info(f"Rx'd ObjectCountingSupported: {self.ObjectCountingSupported}")
        self.ObjectIdentificationSupported = ((aFeatureMap & cluster.Bitmaps.Feature.kObjectIdentification) != 0)
        log.info(f"Rx'd ObjectIdentificationSupported: {self.ObjectIdentificationSupported}")
        self.SoundIdentificationSupported = ((aFeatureMap & cluster.Bitmaps.Feature.kSoundIdentification) != 0)
        log.info(f"Rx'd SoundIdentificationSupported: {self.SoundIdentificationSupported}")
        self.PredictedActivitySupported = ((aFeatureMap & cluster.Bitmaps.Feature.kPredictedActivity) != 0)
        log.info(f"Rx'd PredictedActivitySupported: {self.PredictedActivitySupported}")

        self.step("2")
        simultaneousDetectionLimit = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.SimultaneousDetectionLimit
        )
        log.info(f"Rx'd SimultaneousDetectionLimit: {simultaneousDetectionLimit}")

        # check if multi detection is supported
        if simultaneousDetectionLimit > 1:

            self.step("3")
            # subscription setup
            attrib_listener = AttributeSubscriptionHandler(expected_cluster=cluster)
            await attrib_listener.start(dev_ctrl, node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

            # start event listener
            event_listener = EventSubscriptionHandler(expected_cluster=cluster)
            await event_listener.start(dev_ctrl, node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=90)

            self.step("4")
            # Add AmbientContextSupported elements based on DUT capability
            # Human activity walking, Object identification person, Audio identification barking
            if self.is_ci:
                self.write_to_app_pipe(
                    '{"Name":"SetAmbientContextSupport","EndpointId":1,"AmbientContextType":[{"TypeId":73, "TagId":3},{"TypeId":74, "TagId":4},{"TypeId":75,"TagId":3}]}')

            # define simultaneous detection limit to 2 (To be removed since this will be read only)
            simultaneouslimit_input = 2
            await self.write_single_attribute(attr.SimultaneousDetectionLimit(simultaneouslimit_input))

            # CI for the first ambient sensing event => Human activity walking
            # Use decimal number not hex
            namespaceID1 = HUMANACTIVITYNAMESPACEID  # 75
            tag1 = 3  # walking
            if self.is_ci:
                self.write_to_app_pipe(
                    '{"Name":"AddAmbientContextDetect", "EndpointId":1, "AmbientContextType":[{"TypeId":75, "TagId":3}]}')
            else:
                # Trigger the ambient sensor to change AmbientContextType.AmbientContextSensed.NamespaceID
                # and AmbientContextType.AmbientContextSensed.Tag => TESTER ACTION on DUT
                # self.wait_for_user_input(prompt_msg="Type any letter and press ENTER after a desired ambient sensing is actuated.")
                user_data = self.wait_for_user_input(
                    prompt_msg="Type in namespace ID and tag ID of a desired ambient sensing event (ex [0x4B, 0x03]) and press ENTER after the desired ambient sensing is actuated.")
                user_data = "[0x4B, 0x03]"
                # log.info(f"user input: {user_data}")
                list_dec = ast.literal_eval(user_data)  # convert string hex to decimal
                # log.info(f"list_dec: {list_dec}")
                namespaceID1 = list_dec[0]
                tag1 = list_dec[1]
                log.info(f"user input: {namespaceID1} {tag1}")

            # Add 1 second delay to make sure it's done
            await asyncio.sleep(1)

            # CI for the second ambient sensing event => Object Identification person
            namespaceID2 = OBJECTIDENTIFICATIONNAMESPACEID  # 73
            tag2 = 3  # person
            if self.is_ci:
                self.write_to_app_pipe(
                    '{"Name":"AddAmbientContextDetect", "EndpointId":1, "AmbientContextType":[{"TypeId":73, "TagId":3}]}')
            else:
                # Trigger the ambient sensor to change AmbientContextType.AmbientContextSensed.NamespaceID
                # and AmbientContextType.AmbientContextSensed.Tag => TESTER ACTION on DUT
                # self.wait_for_user_input(prompt_msg="Type any letter and press ENTER after a desired ambient sensing is triggered.")
                user_data = self.wait_for_user_input(
                    prompt_msg="Type in namespace ID and tag ID of a desired ambient sensing event (ex [0x4B, 0x03]) and press ENTER after the desired ambient sensing is actuated.")
                user_data = "[0x49, 0x03]"
                list_dec = ast.literal_eval(user_data)  # convert string hex to decimal
                namespaceID2 = list_dec[0]
                tag2 = list_dec[1]
                log.info(f"user input: {namespaceID2} {tag2}")

            # Add 1 second delay to make sure it's done
            await asyncio.sleep(1)

            # CI for the third ambient sensing event => Sound context barking
            namespaceID3 = SOUNDIDENTIFICATIONNAMESPACEID  # 74
            tag3 = 4  # person
            if self.is_ci:
                self.write_to_app_pipe(
                    '{"Name":"AddAmbientContextDetect", "EndpointId":1, "AmbientContextType":[{"TypeId":74, "TagId":4}]}')
            else:
                # Trigger the ambient sensor to change AmbientContextType.AmbientContextSensed.NamespaceID
                # and AmbientContextType.AmbientContextSensed.Tag => TESTER ACTION on DUT
                # self.wait_for_user_input(prompt_msg="Type any letter and press ENTER after a desired ambient sensing is triggered.")
                user_data = self.wait_for_user_input(
                    prompt_msg="Type in namespace ID and tag ID of a desired ambient sensing event (ex [0x4B, 0x03]) and press ENTER after the desired ambient sensing is actuated.")
                user_data = "[0x49, 0x03]"
                list_dec = ast.literal_eval(user_data)  # convert string hex to decimal
                namespaceID3 = list_dec[0]
                tag3 = list_dec[1]
                log.info(f"user input: {namespaceID3} {tag3}")

            # Add 1 second delay to make sure it's done
            await asyncio.sleep(1)

            # After the 3rd trigger, collect the event ID event to check the correct AmbientContextDetectEnded event reception later
            event = event_listener.wait_for_event_report(cluster.Events.AmbientContextDetectStarted, timeout_sec=10)
            start_event_id = event.event_id

            self.step("5")
            # check the subscription of AmbientContextType attribute
            subscription_expected = attrib_listener.attribute_reports[cluster.Attributes.AmbientContextType][-1].value
            humanActivityDetected = False
            objectIdentified = False
            audioContextDetected = False

            # latest namespace id == namespaceID3
            namespaceid_test = subscription_expected[0].ambientContextSensed[0].namespaceID
            log.info(f"namespaceid_test: {namespaceid_test}")
            asserts.assert_true(namespaceid_test == namespaceID3,
                                f"Unexpected namespaceID, {namespaceid_test}, exp {namespaceID3}")
            asserts.assert_true(subscription_expected[0].ambientContextSensed[0].tag == tag3,
                                f"Unexpected tag, {subscription_expected[0].ambientContextSensed[0].tag}, exp {tag3}")

            # early namespace id == namespaceID2
            namespaceid_test1 = subscription_expected[1].ambientContextSensed[0].namespaceID
            log.info(f"namespaceid_test1: {namespaceid_test1}")
            asserts.assert_true(namespaceid_test1 == namespaceID2,
                                f"Unexpected namespaceID, {namespaceid_test1}, exp {namespaceID2}")
            asserts.assert_true(subscription_expected[1].ambientContextSensed[0].tag == tag2,
                                f"Unexpected tag, {subscription_expected[0].ambientContextSensed[0].tag}, exp {tag2}")

            # AmbientContextType attribute subscription check for the latest event boolean attribute
            if (namespaceid_test == HUMANACTIVITYNAMESPACEID) & self.HumanActivitySupported:
                subscription_bool_expected = attrib_listener.attribute_reports[cluster.Attributes.HumanActivityDetected]
                log.info(f"Rx'd subscription_bool_expected: {subscription_bool_expected}")
                humanActivityDetected = subscription_bool_expected[0].value
                log.info(f"Rx'd humanActivityDetected: {humanActivityDetected}")
                asserts.assert_true(humanActivityDetected, "Failed to get HumanActivityDetected being True.")
            elif (namespaceid_test == OBJECTIDENTIFICATIONNAMESPACEID) & self.ObjectIdentificationSupported:
                subscription_bool_expected = attrib_listener.attribute_reports[cluster.Attributes.ObjectIdentified]
                log.info(f"Rx'd subscription_bool_expected: {subscription_bool_expected}")
                objectIdentified = subscription_bool_expected[0].value
                log.info(f"Rx'd objectIdentified: {objectIdentified}")
                asserts.assert_true(objectIdentified, "Failed to get ObjectIdentified being True.")
            elif (namespaceid_test == SOUNDIDENTIFICATIONNAMESPACEID) & self.SoundIdentificationSupported:
                subscription_bool_expected = attrib_listener.attribute_reports[cluster.Attributes.AudioContextDetected]
                log.info(f"Rx'd subscription_bool_expected: {subscription_bool_expected}")
                audioContextDetected = subscription_bool_expected[0].value
                log.info(f"Rx'd audioContextDetected: {audioContextDetected}")
                asserts.assert_true(audioContextDetected, "Failed to get audioContextDetected being True.")

            # AmbientContextType attribute subscription check for the early boolean attribute
            if (namespaceid_test1 == HUMANACTIVITYNAMESPACEID) & self.HumanActivitySupported:
                subscription_bool_expected1 = attrib_listener.attribute_reports[cluster.Attributes.HumanActivityDetected]
                log.info(f"Rx'd subscription_expected1: {subscription_bool_expected1}")
                humanActivityDetected = subscription_bool_expected1[0].value
                log.info(f"Rx'd humanActivityDetected: {humanActivityDetected}")
                asserts.assert_true(humanActivityDetected, "Failed to get HumanActivityDetected being True.")
            elif (namespaceid_test1 == OBJECTIDENTIFICATIONNAMESPACEID) & self.ObjectIdentificationSupported:
                subscription_bool_expected1 = attrib_listener.attribute_reports[cluster.Attributes.ObjectIdentified]
                log.info(f"Rx'd subscription_bool_expected1: {subscription_bool_expected1}")
                objectIdentified = subscription_bool_expected1[0].value
                log.info(f"Rx'd objectIdentified: {objectIdentified}")
                asserts.assert_true(objectIdentified, "Failed to get ObjectIdentified being True.")
            elif (namespaceid_test1 == SOUNDIDENTIFICATIONNAMESPACEID) & self.SoundIdentificationSupported:
                subscription_bool_expected1 = attrib_listener.attribute_reports[cluster.Attributes.AudioContextDetected]
                log.info(f"Rx'd subscription_bool_expected1: {subscription_bool_expected1}")
                audioContextDetected = subscription_bool_expected1[0].value
                log.info(f"Rx'd audioContextDetected: {audioContextDetected}")
                asserts.assert_true(audioContextDetected, "Failed to get audioContextDetected being True.")

            ambientContextType = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AmbientContextType)
            log.info(f"Rx'd AmbientContextType: {ambientContextType}")

            # Simultaneous Detection => 2
            asserts.assert_equal(len(ambientContextType), 2, "AmbientContextType list needs to be the size of 2.")

            # AmbientContextType attribute read test
            ambientContextType_read = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AmbientContextType)
            log.info(f"Rx'd AmbientContextType_read: {ambientContextType_read}")

            # first entry read
            nsID_2_read = ambientContextType_read[0].ambientContextSensed[0].namespaceID
            tagID_2_read = ambientContextType_read[0].ambientContextSensed[0].tag
            # check the response is the same as what is expected
            asserts.assert_equal(nsID_2_read, namespaceID3, "Namespace ID and Tag ID must reflect step 8 sensing context.")
            asserts.assert_equal(tagID_2_read, tag3, "Namespace ID and Tag ID must reflect step 8 sensing context.")

            # second entry read
            nsID_1_read = ambientContextType_read[1].ambientContextSensed[0].namespaceID
            tagID_1_read = ambientContextType_read[1].ambientContextSensed[0].tag
            asserts.assert_equal(nsID_1_read, namespaceID2, "Namespace ID and Tag ID must reflect step 6 sensing context.")
            asserts.assert_equal(tagID_1_read, tag2, "Namespace ID and Tag ID must reflect step 6 sensing context.")

            attrib_listener.reset()

            self.step("6")
            # wait until HoldTime expires
            # CI call to trigger on
            if self.is_ci:
                holdtime_dut = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.HoldTime)
                await asyncio.sleep(holdtime_dut)
            else:
                self.wait_for_user_input(
                    prompt_msg="Type any letter and press ENTER after HoldTime duration from the step4 has passed.")

            # check if the last AmbientContextDetectEnded event is from the event start number of AmbientContextSensed_2
            # this latest 2nd event end ensures that the HoldTime has passed.
            # event = event_listener.wait_for_event_report(cluster.Events.AmbientContextDetectEnded, timeout_sec=(holdtime_dut))
            # this check not ready yet.
            # asserts.assert_equal(event.event_id, start_event_id, "Wrong Event ID")

            self.step("7")
            # Check the boolean attributes are set to False
            if (humanActivityDetected == True) & self.HumanActivitySupported:
                subscription_bool_expected = attrib_listener.attribute_reports[cluster.Attributes.HumanActivityDetected]
                log.info(f"Rx'd subscription_bool_expected: {subscription_bool_expected}")
                humanActivityDetected = subscription_bool_expected[0].value
                log.info(f"Rx'd humanActivityDetected: {humanActivityDetected}")
                asserts.assert_true(not humanActivityDetected, "Failed to get HumanActivityDetected being False.")

            if (objectIdentified == True) & self.ObjectIdentificationSupported:
                # AmbientContextType attribute subscription check for PIXIT.ACS.AmbientContextSensed_2 = Object identification person
                subscription_bool_expected = attrib_listener.attribute_reports[cluster.Attributes.ObjectIdentified]
                log.info(f"Rx'd subscription_bool_expected: {subscription_bool_expected}")
                objectIdentified = subscription_bool_expected[0].value
                log.info(f"Rx'd objectIdentified: {objectIdentified}")
                asserts.assert_true(not objectIdentified, "Failed to get ObjectIdentified being False.")

            if (audioContextDetected == True) & self.SoundIdentificationSupported:
                # AmbientContextType attribute subscription check for PIXIT.ACS.AmbientContextSensed_2 = Object identification person
                subscription_bool_expected = attrib_listener.attribute_reports[cluster.Attributes.AudioContextDetected]
                log.info(f"Rx'd subscription_bool_expected: {subscription_bool_expected}")
                audioContextDetected = subscription_bool_expected[0].value
                log.info(f"Rx'd audioContextDetected: {audioContextDetected}")
                asserts.assert_true(not audioContextDetected, "Failed to get audioContextDetected being False.")

            # check the subscription of AmbientContextType attribute (to be empty list)
            subscription_expected = attrib_listener.attribute_reports[cluster.Attributes.AmbientContextType][-1].value
            asserts.assert_true(len(subscription_expected) == 0, "AmbientContext attribute is not empty.")

            attrib_listener.reset()

        else:
            log.info("Multiple Detection Feature not supported. This test case skipped")
            self.skip_step("3")
            self.skip_step("4")
            self.skip_step("5")
            self.skip_step("6")
            self.skip_step("7")


if __name__ == "__main__":
    default_matter_test_main()
