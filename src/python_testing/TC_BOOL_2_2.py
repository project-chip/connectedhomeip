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
#

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --app-pipe /tmp/boolean_state_2_2_fifo
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
import time

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_BOOL_2_2(MatterBaseTest):

    def desc_TC_BOOL_2_2(self) -> str:
        return "[TC-BOOL-2.2] Primary Functionality with Server as DUT"

    def steps_TC_BOOL_2_2(self) -> list[TestStep]:
        return [
            TestStep("1", "Commission DUT to TH", is_commissioning=True),
            TestStep("2", "TH reads FeatureMap attribute.", "DUT replies with FeatureMap attribute."),
            TestStep("3", "Bring the DUT into a state so StateValue is FALSE"),
            TestStep("4", "TH prompt operator to verify the device is in FALSE state"),
            TestStep(
                "5",
                "Set up a wildcard subscription for attributes and events of the Boolean State Cluster, "
                "with MinIntervalFloor set to 0, MaxIntervalCeiling set to 30 and KeepSubscriptions set to false",
                "Subscription successfully established",
            ),
            TestStep("6", "Start accumulating all attribute and event reports on the subscription."),
            TestStep("7", "Prompt operator to bring the DUT into a state so StateValue is TRUE"),
            TestStep("8", "TH reads StateValue attribute from the DUT", "DUT responds success and value is TRUE"),
            TestStep(
                "9",
                "Wait for up to 30 seconds for TH to have received an attribute data report",
                "Attribute report received within 30 seconds and StateValue == TRUE",
            ),
            TestStep(
                "10",
                "If either CHGEVENT featIsSupported or BOOL.S.E00(StateChange) is set, "
                "TH checks if an event data report is received in the previous wait; if not, "
                "TH waits to receive an event data report for up to 30 seconds",
                "Event report received within 30 seconds and StateChange.StateValue == TRUE",
            ),
            TestStep("11", "TH clears the accumulated subscription reports and restart accumulating."),
            TestStep("12", "Prompt operator to bring the DUT into a state so StateValue is FALSE"),
            TestStep("13", "TH reads StateValue attribute from the DUT", "DUT responds success and value is FALSE"),
            TestStep(
                "14",
                "Wait for up to 30 seconds for TH to have received an attribute data report",
                "Attribute report received within 30 seconds and StateValue == FALSE",
            ),
            TestStep(
                "15",
                "If either CHGEVENT featIsSupported or BOOL.S.E00(StateChange) is set, "
                "TH checks if an event data report is received in the previous wait; if not, "
                "TH waits to receive an event data report for up to 30 seconds",
                "Event report received within 30 seconds and StateChange.StateValue == FALSE",
            ),
        ]

    def pics_TC_BOOL_2_2(self) -> list[str]:
        return [
            "BOOL.S",
            "BOOL.S.M.ManuallyControlled",
        ]

    def _feature_map_has_chgevent(self, feature_map: int) -> bool:
        CHGEVENT_MASK = 0x01
        return (int(feature_map) & CHGEVENT_MASK) != 0

    def _should_test_state_change_event(self, feature_map: int) -> bool:
        """
        Determines whether StateChange event should be tested.

        Behavior:
          - PICS BOOL.S.E00(StateChange) OR
          - FeatureMap CHGEVENT bit (when defined for BooleanState)
        """
        return self.check_pics("BOOL.S.E00") or self._feature_map_has_chgevent(feature_map)

    async def _set_dut_state_value(self, endpoint: int, state: bool) -> None:
        """
        In CI we drive the all-clusters-app via the named pipe.
        In manual runs, we prompt the operator.
        """
        logger.info("Setting DUT StateValue to %s", "TRUE" if state else "FALSE")
        if self.is_pics_sdk_ci_only:
            command_dict = {"Name": "SetBooleanState", "EndpointId": endpoint, "NewState": state}
            self.write_to_app_pipe(command_dict)
        else:
            self.wait_for_user_input(
                prompt_msg=f"Bring the DUT into a state so StateValue is {'TRUE' if state else 'FALSE'}."
            )

    async def _read_state_value(self, dev_ctrl, node_id: int, endpoint: int) -> bool:
        cbool = Clusters.BooleanState
        return await self.read_single_attribute_check_success(
            dev_ctrl=dev_ctrl,
            node_id=node_id,
            endpoint=endpoint,
            cluster=cbool,
            attribute=cbool.Attributes.StateValue,
        )

    def wait_for_statevalue_report(self, attr_cb, endpoint, expected: bool, timeout_sec: float):
        deadline = time.time() + timeout_sec
        seen = []

        while True:
            remaining = deadline - time.time()
            if remaining <= 0:
                asserts.fail(f"Did not observe StateValue == {expected} within {timeout_sec}s. Seen: {seen}")

            item = attr_cb.wait_for_attribute_report(timeout_sec=remaining)

            if item.endpoint_id != endpoint or item.attribute != Clusters.BooleanState.Attributes.StateValue:
                continue

            seen.append(item.value)

            if bool(item.value) == expected:
                return item

    @run_if_endpoint_matches(has_cluster(Clusters.BooleanState))
    async def test_TC_BOOL_2_2(self) -> None:
        cbool = Clusters.BooleanState
        endpoint = self.get_endpoint()
        node_id = self.dut_node_id
        dev_ctrl = self.default_controller

        # Step 1: Commissioning
        self.step("1")

        # Step 2: Read FeatureMap
        self.step("2")

        # Check if StateChange event is supported
        feature_map = await self.read_single_attribute_check_success(
            dev_ctrl=dev_ctrl,
            node_id=node_id,
            endpoint=endpoint,
            cluster=cbool,
            attribute=cbool.Attributes.FeatureMap,
        )
        logger.info(f"FeatureMap attribute: {feature_map}")

        should_test_state_change_event = self._should_test_state_change_event(feature_map)
        logger.info(f"StateChange event test enabled: {should_test_state_change_event}")

        # Step 3: Put DUT in FALSE
        self.step("3")
        await self._set_dut_state_value(endpoint, state=False)

        # Step 4: Verify FALSE
        self.step("4")
        state_value = await self._read_state_value(dev_ctrl=dev_ctrl, node_id=node_id, endpoint=endpoint)
        logger.info(f"StateValue: {state_value}")
        asserts.assert_false(state_value, "StateValue should be FALSE")

        # Step 5/6: Subscription + accumulate reports
        self.step("5")

        attr_cb = AttributeSubscriptionHandler(expected_cluster=cbool)

        event_cb = EventSubscriptionHandler(expected_cluster=cbool)

        self.step("6")
        await attr_cb.start(
            dev_ctrl=dev_ctrl,
            node_id=node_id,
            endpoint=endpoint,
            min_interval_sec=0,
            max_interval_sec=30,
            keepSubscriptions=False,
        )

        if should_test_state_change_event:
            await event_cb.start(
                dev_ctrl=dev_ctrl,
                node_id=node_id,
                endpoint=endpoint,
                min_interval_sec=0,
                max_interval_sec=30,
            )

        # Step 7: Set TRUE
        self.step("7")
        await self._set_dut_state_value(endpoint, state=True)

        # Step 8: Read StateValue == TRUE
        self.step("8")
        state_value = await self._read_state_value(dev_ctrl=dev_ctrl, node_id=node_id, endpoint=endpoint)
        logger.info(f"StateValue: {state_value}")
        asserts.assert_true(state_value, "StateValue should be TRUE")

        # Step 9: Wait attribute report within 30s and verify it indicates TRUE
        self.step("9")

        item = self.wait_for_statevalue_report(attr_cb, endpoint, expected=True, timeout_sec=30.0)

        logger.info(f"State value report: {item}")

        # Step 10: Event report if supported
        self.step("10")

        if should_test_state_change_event:
            data = event_cb.wait_for_event_report(
                cbool.Events.StateChange,
                timeout_sec=30
            )

            asserts.assert_true(
                data.stateValue,
                "StateChange event should have StateValue == TRUE"
            )
        else:
            logger.info("Skipping test step 10")

        # Step 11: Clear accumulated reports
        self.step("11")
        attr_cb.reset()
        event_cb.reset()

        # Step 12: Set FALSE
        self.step("12")
        await self._set_dut_state_value(endpoint, state=False)

        # Step 13: Read StateValue == FALSE
        self.step("13")
        state_value = await self._read_state_value(dev_ctrl=dev_ctrl, node_id=node_id, endpoint=endpoint)
        logger.info(f"StateValue: {state_value}")
        asserts.assert_false(state_value, "StateValue should be FALSE at step 13")

        # Step 14: Wait attribute report within 30s and verify it indicates FALSE
        self.step("14")

        item = self.wait_for_statevalue_report(attr_cb, endpoint, expected=False, timeout_sec=30.0)

        logger.info(f"State value report: {item}")

        # Step 15: Event report if supported
        self.step("15")

        if should_test_state_change_event:
            data = event_cb.wait_for_event_report(
                cbool.Events.StateChange,
                timeout_sec=30
            )

            asserts.assert_false(
                data.stateValue,
                "StateChange event should have StateValue == FALSE"
            )


if __name__ == "__main__":
    default_matter_test_main()
