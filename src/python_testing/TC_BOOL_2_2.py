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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/boolean_state_2_2_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --app-pipe /tmp/boolean_state_2_2_fifo
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_BOOL_2_2(MatterBaseTest):

    async def set_dut_state_value(self, endpoint: int, state: bool):
        """Set the DUT's BooleanState value via named pipe command."""
        logger.info(f" --- Setting DUT StateValue to {'TRUE' if state else 'FALSE'}")
        if self.is_pics_sdk_ci_only:    # for running in CI
            command_dict = {"Name": "SetBooleanState", "EndpointId": endpoint, "NewState": state}
            self.write_to_app_pipe(command_dict)
        else:                           # for manual testing
            self.wait_for_user_input(
                prompt_msg=f"Bring the DUT into a state so StateValue is {'TRUE' if state else 'FALSE'}.")

    async def read_state_value_from_dut(self, endpoint):
        """Read the StateValue attribute from the DUT."""
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.BooleanState,
            attribute=Clusters.BooleanState.Attributes.StateValue)

    async def is_state_change_event_supported(self) -> bool:
        """Check if StateChange event is supported via PICS or feature map."""
        # TODO: In future spec version also check feature map for event support
        # when that capability is added. See https://github.com/project-chip/connectedhomeip/issues/42425
        # feature_map = await self.read_single_attribute_check_success(
        #     cluster=Clusters.BooleanState,
        #     attribute=Clusters.BooleanState.Attributes.FeatureMap
        # )
        # return self.check_pics("BOOL.S.E00") or bool(feature_map & Clusters.BooleanState.Bitmaps.Feature.kEventSupport)

        return self.check_pics("BOOL.S.E00")

    def desc_TC_BOOL_2_2(self) -> str:
        return "[TC-BOOL-2.2] Primary Functionality with Server as DUT"

    def steps_TC_BOOL_2_2(self) -> list[TestStep]:
        return [
            TestStep("1", "Commission DUT to TH", is_commissioning=True),
            TestStep("1a", "Set up subscription to StateValue attribute and StateChange event."),
            TestStep("2a", "Bring the DUT into a state so StateValue is FALSE."),
            TestStep("2b", "TH reads the StateValue attribute from the DUT.",
                     "Verify that the value in the response is FALSE."),
            TestStep("3a", "Bring the DUT into a state so StateValue is TRUE."),
            TestStep("3b", "TH waits for a StateChange event report from the DUT.",
                     "Verify that StateChange event has StateValue set to TRUE."),
            TestStep("3c", "TH waits for a StateValue attribute report from the DUT.",
                     "Verify that StateValue in the report is TRUE."),
            TestStep("4a", "Bring the DUT into a state so StateValue is FALSE."),
            TestStep("4b", "TH waits for a StateChange event report from the DUT.",
                     "Verify that StateChange event has StateValue set to FALSE."),
            TestStep("4c", "TH waits for a StateValue attribute report from the DUT.",
                     "Verify that StateValue in the report is FALSE."),
        ]

    def pics_TC_BOOL_2_2(self) -> list[str]:
        return [
            "BOOL.S",
            "BOOL.S.E00",  # StateChange event
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.BooleanState))
    async def test_TC_BOOL_2_2(self):

        # Commission DUT to TH done
        self.step("1")

        cbool = Clusters.BooleanState
        endpoint = self.get_endpoint()
        node_id = self.dut_node_id
        dev_ctrl = self.default_controller

        # Check if the StateChange event is supported via PICS or feature map
        has_event = await self.is_state_change_event_supported()

        if not has_event:
            logger.info("StateChange event (BOOL.S.E00) is not supported. Skipping remaining steps.")
            self.mark_all_remaining_steps_skipped("1a")
            return

        # Set up subscription to StateValue attribute and StateChange event.
        self.step("1a")

        event_listener = EventSubscriptionHandler(expected_cluster=cbool)
        await event_listener.start(dev_ctrl, node_id, endpoint=endpoint)

        # Bring the DUT into a state so StateValue is FALSE.
        self.step("2a")

        await self.set_dut_state_value(endpoint, state=False)

        # TH reads the StateValue attribute from the DUT.
        self.step("2b")

        state_value = await self.read_state_value_from_dut(endpoint)
        logger.info(f" --- Step 2b: state_value: {state_value}")

        # Verify that value in the response is FALSE.
        asserts.assert_false(state_value, " --- Step 2b: state_value should be False.")

        # Bring the DUT into a state so StateValue is TRUE.
        self.step("3a")

        await self.set_dut_state_value(endpoint, state=True)

        # TH waits for a StateChange event report from the DUT.
        self.step("3b")

        post_prompt_settle_delay_seconds = 1.0 if self.is_pics_sdk_ci_only else 10.0  # longer delay for manual testing
        event = event_listener.wait_for_event_report(
            cbool.Events.StateChange, timeout_sec=post_prompt_settle_delay_seconds)
        # Verify that StateChange event has StateValue set to TRUE.
        asserts.assert_true(event.stateValue, "Unexpected stateValue on StateChange")

        # Bring the DUT into a state so StateValue is FALSE.
        self.step("4a")

        await self.set_dut_state_value(endpoint, state=False)

        # TH waits for a StateChange event report from the DUT.
        self.step("4b")

        event = event_listener.wait_for_event_report(
            cbool.Events.StateChange, timeout_sec=post_prompt_settle_delay_seconds)
        # Verify that StateChange event has StateValue set to FALSE.
        asserts.assert_false(event.stateValue, "Unexpected stateValue on StateChange")


if __name__ == "__main__":
    default_matter_test_main()
