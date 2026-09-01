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
from matter.clusters.Types import NullValue
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVANALY_2_9(MatterBaseTest, AVANALYTestBase):

    def desc_TC_AVANALY_2_9(self) -> str:
        return "[TC-AVANALY-2.9] Validate context tracking across zone boundaries with Server as DUT"

    def steps_TC_AVANALY_2_9(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Simulate entity detection in Zone 1."),
            TestStep(3, "TH receives PerceivedContext event. Verify CurrentZone is Zone 1, PreviousZone is Null."),
            TestStep(4, "Simulate entity moving from Zone 1 to Zone 2."),
            TestStep(5, "TH receives PerceivedContext event. Verify CurrentZone is Zone 2, PreviousZone is Zone 1."),
        ]

    def pics_TC_AVANALY_2_9(self) -> list[str]:
        return [
            "AVANALY.S",
            "AVANALY.S.F02",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AvAnalysis))
    async def test_TC_AVANALY_2_9(self):
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        self.step(1)  # Already done, immediately go to step 2

        await self.read_avanaly_features(endpoint)
        if not self.has_feature_perzonedetect:
            log.info("PERZONEDETECT feature not supported, skipping TC-AVANALY-2.9")
            self.skip_step(2)
            self.skip_step(3)
            self.skip_step(4)
            self.skip_step(5)
            return

        supported_contexts = await self.read_avanaly_attribute_expect_success(endpoint, attributes.SupportedAmbientContexts)
        asserts.assert_greater_equal(len(supported_contexts), 1, "SupportedAmbientContexts must not be empty")

        # Discover or define Zone 1 and Zone 2 in Zone Management
        zone_1_id = 1
        zone_2_id = 2
        zone_cluster = Clusters.Objects.ZoneManagement
        try:
            existing_zones = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=zone_cluster, attribute=zone_cluster.Attributes.Zones
            )
            if len(existing_zones) >= 2:
                zone_1_id = existing_zones[0].zoneID
                zone_2_id = existing_zones[1].zoneID
            elif len(existing_zones) == 1:
                zone_1_id = existing_zones[0].zoneID
                zone_2_id = zone_1_id + 1
        except Exception as e:
            log.info("ZoneManagement cluster query exception: %s", e)

        log.info("Using Zone 1 ID: %d, Zone 2 ID: %d", zone_1_id, zone_2_id)

        # Enable TrackingEnabled and enable context triggers for all zones
        await self.write_single_attribute(attributes.TrackingEnabled(True), endpoint_id=endpoint)
        await self.send_enable_context_triggers_cmd(endpoint, context_triggers=NullValue)

        # Set up event subscription handler
        event_callback = EventSubscriptionHandler(expected_cluster=cluster)
        await event_callback.start(self.default_controller, self.dut_node_id, endpoint)

        self.step(2)
        if not self.is_ci:
            self.wait_for_user_input(
                prompt_msg=f"Simulate entity detection in Zone 1 (zoneID={zone_1_id}) on the DUT. Press Enter once initiated."
            )

        self.step(3)
        if not self.is_ci:
            event1 = event_callback.wait_for_event_report(cluster.Events.PerceivedContext, timeout_sec=30)
            log.info("PerceivedContext event 1: %s", event1)
            asserts.assert_is_not_none(event1, "Expected PerceivedContext event")
            asserts.assert_is_not_none(event1.newIdentifiedContexts, "newIdentifiedContexts must be present in event 1")
            tc1 = event1.newIdentifiedContexts[0]
            asserts.assert_equal(tc1.currentZone, zone_1_id, f"Expected currentZone {zone_1_id}, got {tc1.currentZone}")
            asserts.assert_true(tc1.previousZone is None or tc1.previousZone is NullValue,
                                f"Expected previousZone to be Null, got {tc1.previousZone}")
        else:
            log.info("CI mode: skipping blocking event wait in Step 3")

        self.step(4)
        if not self.is_ci:
            self.wait_for_user_input(
                prompt_msg=f"Simulate entity moving from Zone 1 (zoneID={zone_1_id}) to Zone 2 (zoneID={zone_2_id}) on the DUT. Press Enter once initiated."
            )

        self.step(5)
        if not self.is_ci:
            event2 = event_callback.wait_for_event_report(cluster.Events.PerceivedContext, timeout_sec=30)
            log.info("PerceivedContext event 2: %s", event2)
            asserts.assert_is_not_none(event2, "Expected PerceivedContext event")
            tracked_list = event2.newIdentifiedContexts or event2.currentIdentifiedContexts
            asserts.assert_is_not_none(tracked_list, "TrackedContexts must be present in event 2")
            tc2 = tracked_list[0]
            asserts.assert_equal(tc2.currentZone, zone_2_id, f"Expected currentZone {zone_2_id}, got {tc2.currentZone}")
            asserts.assert_equal(tc2.previousZone, zone_1_id, f"Expected previousZone {zone_1_id}, got {tc2.previousZone}")
        else:
            log.info("CI mode: skipping blocking event wait in Step 5")

        # Cleanup
        await self.write_single_attribute(attributes.TrackingEnabled(False), endpoint_id=endpoint)
        await self.send_disable_context_triggers_cmd(endpoint, context_triggers=NullValue)


if __name__ == "__main__":
    default_matter_test_main()
