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
from TC_AVANALYTestBase import AVANALYTestBase

import matter.clusters as Clusters
from matter.clusters import Globals
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVANALY_2_3(MatterBaseTest, AVANALYTestBase):

    def desc_TC_AVANALY_2_3(self) -> str:
        return "[TC-AVANALY-2.3] EnableContextTriggers and DisableContextTriggers functionality with local context."

    def steps_TC_AVANALY_2_3(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads the SupportedAmbientContexts attribute, save as `supported_ambient_contexts`."),
            TestStep(3, "If the DUT has feature PerZoneDetect, TH reads the Zones attribute from the ZoneManagement cluster.",
                        "If there are no zones defined, TH creates one. Save the ZoneIDs as 'zoneIDs'."),
            TestStep(4, "TH sends an EnableContextTriggers command with `ContextTriggers` containing a context NOT in `supported_ambient_contexts`. Verify DynamicConstraing error."),
            TestStep(5, "If `zoneIDs` is not empty, TH sends an EnableContextTriggers command with `ContextTriggers` containing a `ZoneID` NOT in `existing_zones` Verify NotFound error."),
            TestStep(6, "TH sends an EnableContextTriggers command with `ContextTriggers` containing a valid subset of `supported_contexts` and valid `ZoneIDs` (or null if no 'zoneIDs'). Verify Success."),
            TestStep(7, "TH reads the ActiveAmbientContextTriggers attribute. Verify it contains the contexts provided in step 6."),
            TestStep(8, "TH sends an EnableContextTriggers command with `ContextTriggers` set to null. Verify success"),
            TestStep(9, "TH reads the ActiveAmbientContextTriggers attribute. Verify it matches `supported_ambient_contexts`."),
            TestStep(10, "If 'zoneIDs' is not empty, TH sends a DisableContextTriggers command with `ContextTriggers` containing a subset of enabled contexts and a specified zoneID. Verify DynamicConstraint error."),
            TestStep(11, "TH sends a DisableContextTriggers command with `ContextTriggers` containing a subset of enabled contexts and Null for zoneIDs. Verify Success."),
            TestStep(12, "TH reads the ActiveAmbientContextTriggers attribute. Verify the disabled contexts are no longer present."),
            TestStep(13, "TH sends a DisableContextTriggers command with `ContextTriggers` set to null. Verify Success"),
            TestStep(14, "TH reads the ActiveAmbientContextTriggers attribute. Verify the list is empty."),
        ]

    def pics_TC_AVANALY_2_3(self) -> list[str]:
        return [
            "AVANALY.S",
        ]

    @run_if_endpoint_matches(has_feature(Clusters.AvAnalysis,
                                         Clusters.AvAnalysis.Bitmaps.Feature.kLocalContextDetection))
    async def test_TC_AVANALY_2_3(self):
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        structs = cluster.Structs
        endpoint = self.get_endpoint()

        feature_map = await self.read_avanaly_attribute_expect_success(endpoint, attributes.FeatureMap)
        self.has_feature_perzonedetect = (feature_map & cluster.Bitmaps.Feature.kPerZoneContextDetection) != 0

        self.step(1)  # Already done, immediately go to step 2

        self.step(2)
        supported_ambient_contexts_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.SupportedAmbientContexts)

        zoneIDs = []
        if self.has_feature_perzonedetect:
            self.step(3)
            zoneIDs = await self.get_zoneids_from_zone_management(endpoint)
        else:
            self.skip_step(3)

        self.step(4)
        # Send a context trigger with an invalid context (namespace), ensure CONSTRAINT_ERROR
        semantic_tag = Globals.Structs.SemanticTagStruct(
            mfgCode=NullValue,
            namespaceID=0x47,
            tag=0x01,
            label=None)

        # set the ZoneIDs to None or Null depending on feature setting
        invalid_context_zoneID = NullValue if self.has_feature_perzonedetect else None
        invalid_context_triggers = [structs.ContextTriggerStruct(context=semantic_tag, zoneIDs=invalid_context_zoneID)]
        await self.send_enable_context_triggers_command(endpoint, invalid_context_triggers, expected_status=Status.ConstraintError)

        self.step(5)
        # If we have ZoneIDs, try one not in the list
        if len(zoneIDs) >= 1:
            invalid_zone_context_triggers = []
            invalid_zone_context_trigger = structs.ContextTriggerStruct(
                context=supported_ambient_contexts_dut[0], zoneIDs=[zoneIDs[-1] + 1])
            invalid_zone_context_triggers.append(invalid_zone_context_trigger)

            await self.send_enable_context_triggers_command(endpoint, invalid_zone_context_triggers, expected_status=Status.NotFound)

        self.step(6)
        # Send an enable with the first from the set of available triggers
        valid_context_triggers = []

        # Set ZoneIDs to None if no feature, Null if feature and no zone IDs, or the first zoneID if we have those
        valid_context_zoneID = None
        if self.has_feature_perzonedetect:
            valid_context_zoneID = zoneIDs[0] if len(zoneIDs) >= 1 else NullValue

        context_trigger = structs.ContextTriggerStruct(context=supported_ambient_contexts_dut[0], zoneIDs=[valid_context_zoneID])
        valid_context_triggers.append(context_trigger)

        await self.send_enable_context_triggers_command(endpoint, valid_context_triggers)

        self.step(7)
        active_ambient_context_triggers_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.ActiveAmbientContextTriggers)
        asserts.assert_equal(valid_context_triggers, active_ambient_context_triggers_dut,
                             "Active triggers should equate to the enabled triggers.")

        self.step(8)
        full_set_of_context_triggers = []
        for ambient_context in supported_ambient_contexts_dut:
            zoneID = NullValue if self.has_feature_perzonedetect else None
            context_trigger = structs.ContextTriggerStruct(context=ambient_context, zoneIDs=zoneID)
            full_set_of_context_triggers.append(context_trigger)

        await self.send_enable_context_triggers_command(endpoint, NullValue)

        self.step(9)
        active_ambient_context_triggers_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.ActiveAmbientContextTriggers)
        asserts.assert_equal(full_set_of_context_triggers, active_ambient_context_triggers_dut,
                             "Active triggers should equate to the full set of supported contexts.")

        self.step(10)
        # Disable the first item in the full set, as we've set all contexts all zones (Null) as active, we have to have zones as null.
        # First provide a Zone, this should have a DynamicConstraint error.  Only do this if we have zoneIDs
        if len(zoneIDs) >= 1:
            await self.send_disable_context_triggers_command(endpoint, valid_context_triggers, expected_status=Status.DynamicConstraintError)

        self.step(11)
        valid_context_triggers[0].zoneIDs = NullValue
        await self.send_disable_context_triggers_command(endpoint, valid_context_triggers)

        self.step(12)
        del full_set_of_context_triggers[0]

        active_ambient_context_triggers_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.ActiveAmbientContextTriggers)
        asserts.assert_equal(full_set_of_context_triggers, active_ambient_context_triggers_dut,
                             "Active triggers should equate to the reduced set of supported contexts.")

        self.step(13)
        await self.send_disable_context_triggers_command(endpoint, NullValue)

        self.step(14)
        active_ambient_context_triggers_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.ActiveAmbientContextTriggers)
        asserts.assert_equal(len(active_ambient_context_triggers_dut), 0, "Active triggers should be empty.")


if __name__ == "__main__":
    default_matter_test_main()
