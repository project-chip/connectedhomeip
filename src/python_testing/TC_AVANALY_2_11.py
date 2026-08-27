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
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVANALY_2_11(MatterBaseTest, AVANALYTestBase):

    def desc_TC_AVANALY_2_11(self) -> str:
        return "[TC-AVANALY-2.11] Validate persistence of attributes and active triggers with Server as DUT"

    def steps_TC_AVANALY_2_11(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH enables a subset of context triggers. Verify success response."),
            TestStep(3, "TH sets TrackingEnabled attribute to True. Verify success response."),
            TestStep(4, "Reboot the DUT."),
            TestStep(5, "Reconnect after reboot, already done.", is_commissioning=True),
            TestStep(6, "TH reads ActiveAmbientContextTriggers attribute. Verify value is restored across reboot."),
            TestStep(7, "TH reads TrackingEnabled attribute. Verify value is restored across reboot."),
        ]

    def pics_TC_AVANALY_2_11(self) -> list[str]:
        return [
            "AVANALY.S",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AvAnalysis))
    async def test_TC_AVANALY_2_11(self):
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()

        self.step(1)  # Already done, immediately go to step 2

        await self.read_avanaly_features(endpoint)
        supported_contexts = await self.read_avanaly_attribute_expect_success(endpoint, attributes.SupportedAmbientContexts)
        asserts.assert_greater_equal(len(supported_contexts), 1, "SupportedAmbientContexts must not be empty")

        self.step(2)
        subset_context = supported_contexts[0]
        if self.has_feature_perzonedetect:
            trigger = cluster.Structs.ContextTriggerStruct(context=subset_context, zoneIDs=None)
        else:
            trigger = cluster.Structs.ContextTriggerStruct(context=subset_context)

        await self.send_enable_context_triggers_cmd(endpoint, context_triggers=[trigger])
        active_before = await self.read_avanaly_attribute_expect_success(endpoint, attributes.ActiveAmbientContextTriggers)
        asserts.assert_greater_equal(len(active_before), 1, "ActiveAmbientContextTriggers should not be empty")

        self.step(3)
        res = await self.write_single_attribute(attributes.TrackingEnabled(True), endpoint_id=endpoint)
        asserts.assert_equal(res, Status.Success, "Writing TrackingEnabled failed")

        self.step(4)
        await self.request_device_reboot()

        self.step(5)  # Reconnection handled by request_device_reboot

        self.step(6)
        active_after = await self.read_avanaly_attribute_expect_success(endpoint, attributes.ActiveAmbientContextTriggers)
        log.info("ActiveAmbientContextTriggers after reboot: %s", active_after)
        active_tags_after = {(t.context.namespaceID, t.context.tag) for t in active_after}
        asserts.assert_in((subset_context.namespaceID, subset_context.tag), active_tags_after,
                          "Enabled context trigger was not persisted across reboot")

        self.step(7)
        tracking_after = await self.read_avanaly_attribute_expect_success(endpoint, attributes.TrackingEnabled)
        log.info("TrackingEnabled after reboot: %s", tracking_after)
        asserts.assert_true(tracking_after, "TrackingEnabled was not persisted as True across reboot")

        # Cleanup
        await self.write_single_attribute(attributes.TrackingEnabled(False), endpoint_id=endpoint)
        await self.send_disable_context_triggers_cmd(endpoint, context_triggers=None)


if __name__ == "__main__":
    default_matter_test_main()
