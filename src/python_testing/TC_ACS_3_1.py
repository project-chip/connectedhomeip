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

import asyncio
import logging
import time

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import Globals
from matter.clusters.Types import NullValue
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import AttributeValue, MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

HUMANACTIVITYNAMESPACEID = 0x4B
OBJECTIDENTIFICATIONNAMESPACEID = 0x49
SOUNDIDENTIFICATIONNAMESPACEID = 0x4A


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
            TestStep("3", "TH writes 2 to the SimultaneousDetectionLimit attribute.",
                     "Verify that TH reads the SimultaneousDetectionLimit attribute with 2."),
            TestStep("4", "TH writes 50 to the HoldTimeMax of HoldTimeLimits attribute.",
                     "Verify that TH reads the HoldTimeMax of HoldTime attribute with a value of 50."),
            TestStep("5", "TH writes DUT HoldTime attribute with with 30,"
                     "Verify TH reads the HoldTime attribute with a value of 30."),
            TestStep("6", "Trigger DUT with its ambient sensing supporting feature via PIXIT.ACS.AmbientContextSensed_1."),
            TestStep("7", "TH reads the AmbientContextType attribute.",
                     "Verify that DUT response contains the AmbientContextSensed struct data including the namespace ID and its tag ID that match the triggering sensing context from the step 6."),
            TestStep("8", "Within HoldTime seconds from the step 6, trigger DUT with its ambient sensing supporting feature via PIXIT.ACS.AmbientContextSensed_2, which is different from the step 6."),
            TestStep("9", "TH reads the AmbientContextType attribute.",
                     "Verify that Verify that the AmbientContextSensed field data of The AmbientContextType attribute contains the list size of 2.",
                     "Verify that the first list entry contains the namespace ID and tag ID of PIXIT.ACS.AmbientContextSensed_2 from the step 8, and the second list entry contains the namespace ID and tag ID of PIXIT.ACS.AmbientContextSensed_1 from the step 6."),
            TestStep("10", "Still within HoldTime seconds from the step 6, trigger DUT with its ambient sensing supporting feature via PIXIT.ACS.AmbientContextSensed_3, which is different from the step 8."),
            TestStep("11", "TH reads the AmbientContextType attribute.",
                     "Verify that DUT response contains the AmbientContextSensed struct data list of size 2.",
                     "Verify that the first list entry contains the namespace ID and tag ID from PIXIT.ACS.AmbientContextSensed_3, and the second list element contains the namespace ID and tag ID from PIXIT.ACS.AmbientContextSensed_2"),
            TestStep("12", "Wait until HoldTime seconds from the step 6 expires."),
            TestStep("13", "TH reads the AmbientContextType attribute.",
                     "Verify that the list entry only contains the namespace ID and tag ID from PIXIT.ACS.AmbientContextSensed_3")
        ]

    # Sends and out-of-band command to the all-clusters-app
    def write_to_app_pipe(self, command):
        # CI app pipe id creation
        self.app_pipe = "/tmp/acs_fifo_"
        if self.is_ci:
            app_pid = self.matter_test_config.app_pid
            if app_pid == 0:
                asserts.fail("The --app-pid flag must be set when using named pipe")
            self.app_pipe = self.app_pipe + str(app_pid)

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

        self.step("2")
        simultaneousDetectionLimit = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.SimultaneousDetectionLimit
        )
        log.info(f"Rx'd SimultaneousDetectionLimit: {simultaneousDetectionLimit}")
        if simultaneousDetectionLimit != 1:

            self.step("3")
            # subscription setup for the following trigger testing
            attrib_listener = AttributeSubscriptionHandler(expected_cluster=cluster)
            await attrib_listener.start(dev_ctrl, node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

            simultaneouslimit_input = 2  # define simultaneous detection limit to 2
            await self.write_single_attribute(attr.SimultaneousDetectionLimit(simultaneouslimit_input))

            simultaneousDetectionLimit = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SimultaneousDetectionLimit
            )
            asserts.assert_equal(simultaneousDetectionLimit, simultaneouslimit_input,
                                 "Different SimultaneousDetectionLimit value is read.")

            # subscription check
            subscription_expected = simultaneouslimit_input
            attrib_listener.await_all_final_values_reported(
                expected_final_values=[AttributeValue(
                    endpoint_id=endpoint, attribute=attr.SimultaneousDetectionLimit, value=subscription_expected)],
                timeout_sec=30.0)
            log.info("Received attribute report for SimultaneousDetectionLimit.")
            attrib_listener.reset()

            self.step("4")
            holdtimemax_input = 50  # define HoldTimeMax to be 50 sec
            await self.write_single_attribute(attr.HoldTimeLimits.holTimeMax(holdtimemax_input))

            holdTimeMax = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.HoldTimeLimits.holdTimeMax
            )
            asserts.assert_equal(holdTimeMax, holdtimemax_input,
                                 "Different HoldTimeMax value is read.")

            # subscription check
            subscription_expected = cluster.Structs.HoldTimeLimitStruct(
                holdTimeMin=1, holdTimeMax=holdtimemax_input, holdTimeDefault=10)
            attrib_listener.await_all_final_values_reported(
                expected_final_values=[AttributeValue(
                    endpoint_id=endpoint, attribute=attr.HoldTimeLimit, value=subscription_expected)],
                timeout_sec=30.0)
            log.info("Received attribute report for HoldTimeLimit.")
            attrib_listener.reset()

            self.step("5")
            holdtime_input = 30  # 30 sec for HoldTime
            await self.write_single_attribute(attr.HoldTime(holdtime_input))

            holdTime = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.HoldTime
            )
            asserts.assert_eqaul(holdTime, holdtime_input, "Different HoldTime value is read.")

            # subscription check
            subscription_expected = holdtime_input
            attrib_listener.await_all_final_values_reported(
                expected_final_values=[AttributeValue(
                    endpoint_id=endpoint, attribute=attr.HoldTime, value=subscription_expected)],
                timeout_sec=30.0)
            log.info("Received attribute report for HoldTime.")
            attrib_listener.reset()

            self.step("6")  # Trigger ambient sensing
            # PIXIT.ACS.AmbientContextSensed_1 = Human activity walking
            namespaceID1 = HUMANACTIVITYNAMESPACEID
            tag1 = 0x03  # walking
            # CI call to trigger on
            # CI input number type is decimal not hexidecimal
            if self.is_ci:
                self.write_to_app_pipe(
                    '{"Name":"AddAmbientContextDetect", "EndpointId": 1, "AmbientContextType": {"TypeId":75, "TagId":3}}')
            else:
                # Trigger the ambient sensor to change AmbientContextType.AmbientContextSensed.NamespaceID
                # and AmbientContextType.AmbientContextSensed.Tag => TESTER ACTION on DUT
                self.wait_for_user_input(
                    prompt_msg="Type any letter and press ENTER after a desired ambient sensing is triggered.")

            self.step("7")
            ambientContextType = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AmbientContextType)
            log.info(f"Rx'd AmbientContextType: {ambientContextType}")

            nsID_1 = ambientContextType.ambientContextSensed.namespaceID
            tagID_1 = ambientContextType.ambientContextSensed.tag
            # check the response is the same as what is expected
            asserts.assert_equal(nsID_1, namespaceID1, "Namespace ID and Tag ID must reflect step 6 sensing context.")
            asserts.assert_equal(tagID_1, tag1, "Namespace ID and Tag ID must reflect step 6 sensing context.")

            # subscription check
            semantic_tag = Globals.Structs.SemanticTagStruct(mfgCode=NullValue, namespaceID=namespaceID1, tag=tag1, label=NullValue)
            subscription_expected = cluster.Structs.AmbientContextTypeStruct(ambientContextSensed=semantic_tag, detectionTime=None)
            attrib_listener.await_all_final_values_reported(
                expected_final_values=[AttributeValue(
                    endpoint_id=endpoint, attribute=attr.AmbientContextType, value=subscription_expected)],
                timeout_sec=30.0)
            # attrib_listener.await_all_final_values_reported(expected_final_values=[AttributeValue(endpoint_id=endpoint, attribute=attr.AmbientContextType, value=subscription_expected)], timeout_sec=30.0)
            log.info("Received attribute report for AmbientContextType.")
            attrib_listener.reset()

            self.step("8")  # Trigger another ambient sensing
            # PIXIT.ACS.AmbientContextSensed_2 = Object Identification person
            namespaceID2 = OBJECTIDENTIFICATIONNAMESPACEID
            tag2 = 0x03  # person
            # CI call to trigger on
            if self.is_ci:
                self.write_to_app_pipe(
                    '{"Name":"AddAmbientContextDetect", "EndpointId": 1, "AmbientContextType": {"TypeId":73, "TagId":3}}')
            else:
                # Trigger the ambient sensor to change AmbientContextType.AmbientContextSensed.NamespaceID
                # and AmbientContextType.AmbientContextSensed.Tag => TESTER ACTION on DUT
                self.wait_for_user_input(
                    prompt_msg="Type any letter and press ENTER after a desired ambient sensing is triggered.")

            self.step("9")
            ambientContextType = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AmbientContextType)
            log.info(f"Rx'd AmbientContextType: {ambientContextType}")

            # first entry
            nsID_2 = ambientContextType[0].ambientContextSensed.namespaceID
            tagID_2 = ambientContextType[0].ambientContextSensed.tag
            # check the response is the same as what is expected
            asserts.assert_equal(nsID_2, namespaceID2, "Namespace ID and Tag ID must reflect step 8 sensing context.")
            asserts.assert_equal(tagID_2, tag2, "Namespace ID and Tag ID must reflect step 8 sensing context.")

            # second entry
            nsID_2_1 = ambientContextType[1].ambientContextSensed.namespaceID
            tagID_2_1 = ambientContextType[1].ambientContextSensed.tag
            asserts.assert_equal(nsID_2_1, namespaceID1, "Namespace ID and Tag ID must reflect step 6 sensing context.")
            asserts.assert_equal(tagID_2_1, tag1, "Namespace ID and Tag ID must reflect step 6 sensing context.")

            # subscription check
            semantic_tag1 = Globals.Structs.SemanticTagStruct(
                mfgCode=NullValue, namespaceID=namespaceID1, tag=tag1, label=NullValue)
            semantic_tag2 = Globals.Structs.SemanticTagStruct(
                mfgCode=NullValue, namespaceID=namespaceID2, tag=tag2, label=NullValue)
            subscription_expected1 = cluster.Structs.AmbientContextTypeStruct(
                ambientContextSensed=semantic_tag1, detectionTime=None)
            subscription_expected2 = cluster.Structs.AmbientContextTypeStruct(
                ambientContextSensed=semantic_tag2, detectionTime=None)
            attrib_listener.await_all_final_values_reported(
                expected_final_values=[AttributeValue(
                    endpoint_id=endpoint, attribute=attr.AmbientContextType, value=[subscription_expected2, subscription_expected1])],
                timeout_sec=30.0)
            # attrib_listener.await_all_final_values_reported(expected_final_values=[AttributeValue(endpoint_id=endpoint, attribute=attr.AmbientContextType, value=subscription_expected)], timeout_sec=30.0)
            log.info("Received attribute report for AmbientContextType.")
            attrib_listener.reset()

            self.step("10")  # Trigger another ambient sensing
            # PIXIT.ACS.AmbientContextSensed_2 = Sound Identification barking
            namespaceID3 = SOUNDIDENTIFICATIONNAMESPACEID
            tag3 = 0x04  # barking
            # CI call to trigger on
            if self.is_ci:
                self.write_to_app_pipe(
                    '{"Name":"AddAmbientContextDetect", "EndpointId": 1, "AmbientContextType": {"TypeId":74, "TagId":4}}')
            else:
                # Trigger the ambient sensor to change AmbientContextType.AmbientContextSensed.NamespaceID
                # and AmbientContextType.AmbientContextSensed.Tag => TESTER ACTION on DUT
                self.wait_for_user_input(
                    prompt_msg="Type any letter and press ENTER after a desired ambient sensing is triggered.")

            self.step("11")
            ambientContextType = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AmbientContextType)
            log.info(f"Rx'd AmbientContextType: {ambientContextType}")

            # PIXITSimultaneousDetectionLimit = 2
            asserts.assert_equal(len(ambientContextType), simultaneousDetectionLimit,
                                 "AmbientContextType needs to be the size of 2.")

            nsID_3 = ambientContextType[0].ambientContextSensed.namespaceID
            tagID_3 = ambientContextType[0].ambientContextSensed.tag
            # check the response is the same as what is expected
            asserts.assert_equal(nsID_3, namespaceID3, "Namespace ID and Tag ID must reflect step 10 sensing context.")
            asserts.assert_equal(tagID_3, tag3, "Namespace ID and Tag ID must reflect step 10 sensing context.")

            nsID_3_1 = ambientContextType[1].ambientContextSensed.namespaceID
            tagID_3_1 = ambientContextType[1].ambientContextSensed.tag
            asserts.assert_equal(nsID_3_1, namespaceID2, "Namespace ID and Tag ID must reflect step 8 sensing context.")
            asserts.assert_equal(tagID_3_1, tag2, "Namespace ID and Tag ID must reflect step 8 sensing context.")

            # subscription check
            semantic_tag3 = Globals.Structs.SemanticTagStruct(
                mfgCode=NullValue, namespaceID=namespaceID3, tag=tag3, label=NullValue)
            subscription_expected3 = cluster.Structs.AmbientContextTypeStruct(
                ambientContextSensed=semantic_tag3, detectionTime=None)
            attrib_listener.await_all_final_values_reported(
                expected_final_values=[AttributeValue(
                    endpoint_id=endpoint, attribute=attr.AmbientContextType, value=[subscription_expected3, subscription_expected2])],
                timeout_sec=30.0)
            # attrib_listener.await_all_final_values_reported(expected_final_values=[AttributeValue(endpoint_id=endpoint, attribute=attr.AmbientContextType, value=subscription_expected)], timeout_sec=30.0)
            log.info("Received attribute report for AmbientContextType.")
            attrib_listener.reset()

            self.step("12")
            await asyncio.sleep(holdtime_input)

            self.step("13")
            ambientContextType = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AmbientContextType)
            log.info(f"Rx'd AmbientContextType: {ambientContextType}")

            nsID_4 = ambientContextType.ambientContextSensed.namespaceID
            tagID_4 = ambientContextType.ambientContextSensed.tag
            # check the response is the same as what is expected
            asserts.assert_equal(nsID_4, namespaceID3, "Namespace ID and Tag ID must reflect step 10 sensing context.")
            asserts.assert_equal(tagID_4, tag3, "Namespace ID and Tag ID must reflect step 10 sensing context.")

            # subscription check
            attrib_listener.await_all_final_values_reported(
                expected_final_values=[AttributeValue(
                    endpoint_id=endpoint, attribute=attr.AmbientContextType, value=subscription_expected3)],
                timeout_sec=30.0)
            # attrib_listener.await_all_final_values_reported(expected_final_values=[AttributeValue(endpoint_id=endpoint, attribute=attr.AmbientContextType, value=subscription_expected)], timeout_sec=30.0)
            log.info("Received attribute report for AmbientContextType.")
            attrib_listener.reset()

        else:
            log.info("Multiple Detection Feature not supported. This test case skipped")
            self.skip_step("2")
            self.skip_step("3")
            self.skip_step("4")
            self.skip_step("5")
            self.skip_step("6")
            self.skip_step("7")
            self.skip_step("8")
            self.skip_step("9")
            self.skip_step("10")
            self.skip_step("11")
            self.skip_step("12")
            self.skip_step("13")


if __name__ == "__main__":
    default_matter_test_main()
