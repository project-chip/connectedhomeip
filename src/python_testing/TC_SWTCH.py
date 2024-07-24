#
#    Copyright (c) 2023 Project CHIP Authors
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
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --endpoint 3 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto --PICS src/app/tests/suites/certification/ci-pics-values
# === END CI TEST ARGUMENTS ===

import json
import logging
import queue
import time
from typing import Any

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import EventReadResult, TypedAttributePath
from matter_testing_support import (AttributeValue, ClusterAttributeChangeAccumulator, EventChangeCallback, MatterBaseTest,
                                    async_test_body, default_matter_test_main)
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_SwitchTests(MatterBaseTest):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def desc_TC_SWTCH_2_4(self) -> str:
        """Returns a description of this test"""
        return "[TC-SWTCH-2.4] Momentary Switch Long Press Verification"

    def pics_TC_SWTCH_2_4(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["SWTCH.S", "SWTCH.S.F01"]

    # def steps_TC_SWTCH_2_4(self) -> list[TestStep]:
    #     steps = [
    #         TestStep("0", "Commissioning, already done", is_commissioning=True),
    #         # TODO: fill when test is done
    #     ]

    #     return steps

    def _send_named_pipe_command(self, command_dict: dict[str, Any]):
        app_pid = self.matter_test_config.app_pid
        if app_pid == 0:
            asserts.fail("The --app-pid flag must be set when usage of button simulation named pipe is required (e.g. CI)")

        app_pipe = f"/tmp/chip_all_clusters_fifo_{app_pid}"
        command = json.dumps(command_dict)

        # Sends an out-of-band command to the sample app
        with open(app_pipe, "w") as outfile:
            logging.info(f"Sending named pipe command to {app_pipe}: '{command}'")
            outfile.write(command + "\n")
        # Delay for pipe command to be processed (otherwise tests may be flaky).
        time.sleep(0.1)

    def _use_button_simulator(self) -> bool:
        return self.check_pics("PICS_SDK_CI_ONLY") or self.user_params.get("use_button_simulator", False)

    def _ask_for_switch_idle(self):
        if not self._use_button_simulator():
            self.wait_for_user_input(prompt_msg="Ensure switch is idle")

    def _ask_for_long_press(self, endpoint_id: int, pressed_position: int):
        if not self._use_button_simulator():
            self.wait_for_user_input(
                prompt_msg=f"Press switch position {pressed_position} for a long time (around 5 seconds) on the DUT, then release it.")
        else:
            command_dict = {"Name": "SimulateActionSwitchLongPress", "EndpointId": endpoint_id,
                            "ButtonId": pressed_position, "LongPressDelayMillis": 5000, "LongPressDurationMillis": 5500}
            self._send_named_pipe_command(command_dict)

    def _placeholder_for_step(self, step_id: str):
        # TODO: Global search an replace of `self._placeholder_for_step` with `self.step` when done.
        logging.info(f"Step {step_id}")
        pass

    def _placeholder_for_skip(self, step_id: str):
        logging.info(f"Skipped step {step_id}")

    def _await_sequence_of_reports(self, report_queue: queue.Queue, endpoint_id: int, attribute: TypedAttributePath, sequence: list[Any], timeout_sec: float):
        start_time = time.time()
        elapsed = 0.0
        time_remaining = timeout_sec

        sequence_idx = 0
        actual_values = []

        while time_remaining > 0:
            expected_value = sequence[sequence_idx]
            logging.info(f"Expecting value {expected_value} for attribute {attribute} on endpoint {endpoint_id}")
            try:
                item: AttributeValue = report_queue.get(block=True, timeout=time_remaining)

                # Track arrival of all values for the given attribute.
                if item.endpoint_id == endpoint_id and item.attribute == attribute:
                    actual_values.append(item.value)

                    if item.value == expected_value:
                        logging.info(f"Got expected attribute change {sequence_idx+1}/{len(sequence)} for attribute {attribute}")
                        sequence_idx += 1
                    else:
                        asserts.assert_equal(item.value, expected_value,
                                             msg="Did not get expected attribute value in correct sequence.")

                    # We are done waiting when we have accumulated all results.
                    if sequence_idx == len(sequence):
                        logging.info("Got all attribute changes, done waiting.")
                        return
            except queue.Empty:
                # No error, we update timeouts and keep going
                pass

            elapsed = time.time() - start_time
            time_remaining = timeout_sec - elapsed

        asserts.fail(f"Did not get full sequence {sequence} in {timeout_sec:.1f} seconds. Got {actual_values} before time-out.")

    def _await_sequence_of_events(self, event_queue: queue.Queue, endpoint_id: int, sequence: list[ClusterObjects.ClusterEvent], timeout_sec: float):
        start_time = time.time()
        elapsed = 0.0
        time_remaining = timeout_sec

        sequence_idx = 0
        actual_events = []

        while time_remaining > 0:
            logging.info(f"Expecting event {sequence[sequence_idx]} on endpoint {endpoint_id}")
            try:
                item: EventReadResult = event_queue.get(block=True, timeout=time_remaining)
                expected_event = sequence[sequence_idx]
                event_data = item.Data

                if item.Header.EndpointId == endpoint_id and item.Header.ClusterId == event_data.cluster_id:
                    actual_events.append(event_data)

                    if event_data == expected_event:
                        logging.info(f"Got expected Event {sequence_idx+1}/{len(sequence)}: {event_data}")
                        sequence_idx += 1
                    else:
                        asserts.assert_equal(event_data, expected_event, msg="Did not get expected event in correct sequence.")

                    # We are done waiting when we have accumulated all results.
                    if sequence_idx == len(sequence):
                        logging.info("Got all expected events, done waiting.")
                        return
            except queue.Empty:
                # No error, we update timeouts and keep going
                pass

            elapsed = time.time() - start_time
            time_remaining = timeout_sec - elapsed

        asserts.fail(f"Did not get full sequence {sequence} in {timeout_sec:.1f} seconds. Got {actual_events} before time-out.")

    def _expect_no_events_for_cluster(self, event_queue: queue.Queue, endpoint_id: int, expected_cluster: ClusterObjects.Cluster, timeout_sec: float):
        start_time = time.time()
        elapsed = 0.0
        time_remaining = timeout_sec

        logging.info(f"Waiting {timeout_sec:.1f} seconds for no more events for cluster {expected_cluster} on endpoint {endpoint_id}")
        while time_remaining > 0:
            try:
                item: EventReadResult = event_queue.get(block=True, timeout=time_remaining)
                event_data = item.Data

                if item.Header.EndpointId == endpoint_id and item.Header.ClusterId == event_data.cluster_id and item.Header.ClusterId == expected_cluster.id:
                    asserts.fail(f"Got Event {event_data} when we expected no further events for {expected_cluster}")
            except queue.Empty:
                # No error, we update timeouts and keep going
                pass

            elapsed = time.time() - start_time
            time_remaining = timeout_sec - elapsed

        logging.info(f"Successfully waited for no further events on {expected_cluster} for {elapsed:.1f} seconds")

    @async_test_body
    async def test_TC_SWTCH_2_4(self):
        # TODO: Make this come from PIXIT
        switch_pressed_position = 1
        post_prompt_settle_delay_seconds = 10.0

        # Commission DUT - already done

        # Read feature map to set bool markers
        cluster = Clusters.Objects.Switch
        feature_map = await self.read_single_attribute_check_success(cluster, attribute=cluster.Attributes.FeatureMap)

        has_ms_feature = (feature_map & cluster.Bitmaps.Feature.kMomentarySwitch) != 0
        has_msr_feature = (feature_map & cluster.Bitmaps.Feature.kMomentarySwitchRelease) != 0
        has_msl_feature = (feature_map & cluster.Bitmaps.Feature.kMomentarySwitchLongPress) != 0
        has_as_feature = (feature_map & cluster.Bitmaps.Feature.kActionSwitch) != 0
        # has_msm_feature = (feature_map & cluster.Bitmaps.Feature.kMomentarySwitchMultiPress) != 0

        if not has_ms_feature:
            logging.info("Skipping rest of test: SWTCH.S.F01(MS) feature not present")
            self.skip_all_remaining_steps("2")

        endpoint_id = self.matter_test_config.endpoint

        # Step 1: Set up subscription to all Switch cluster events
        self._placeholder_for_step("1")
        event_listener = EventChangeCallback(cluster)
        attrib_listener = ClusterAttributeChangeAccumulator(cluster)
        await event_listener.start(self.default_controller, self.dut_node_id, endpoint=endpoint_id)
        await attrib_listener.start(self.default_controller, self.dut_node_id, endpoint=endpoint_id)

        # Step 2: Operator does not operate switch on the DUT
        self._placeholder_for_step("2")
        self._ask_for_switch_idle()

        # Step 3: TH reads the CurrentPosition attribute from the DUT
        self._placeholder_for_step("3")

        # Verify that the value is 0
        current_position = await self.read_single_attribute_check_success(cluster, attribute=cluster.Attributes.CurrentPosition)
        asserts.assert_equal(current_position, 0)

        # Step 4a: Operator operates switch (keep pressed for long time, e.g. 5 seconds) on the DUT, the release it
        self._placeholder_for_step("4a")
        self._ask_for_long_press(endpoint_id, switch_pressed_position)

        # Step 4b: TH expects report of CurrentPosition 1, followed by a report of Current Position 0.
        self._placeholder_for_step("4b")
        logging.info(
            f"Starting to wait for {post_prompt_settle_delay_seconds:.1f} seconds for CurrentPosition to go {switch_pressed_position}, then 0.")
        self._await_sequence_of_reports(report_queue=attrib_listener.attribute_queue, endpoint_id=endpoint_id, attribute=cluster.Attributes.CurrentPosition, sequence=[
                                        switch_pressed_position, 0], timeout_sec=post_prompt_settle_delay_seconds)

        # Step 4c: TH expects at least InitialPress with NewPosition = 1
        self._placeholder_for_step("4c")
        logging.info(f"Starting to wait for {post_prompt_settle_delay_seconds:.1f} seconds for InitialPress event.")
        expected_events = [cluster.Events.InitialPress(newPosition=switch_pressed_position)]
        self._await_sequence_of_events(event_queue=event_listener.event_queue, endpoint_id=endpoint_id,
                                       sequence=expected_events, timeout_sec=post_prompt_settle_delay_seconds)

        # Step 4d: For MSL/AS, expect to see LongPress/LongRelease in that order
        if not has_msl_feature and not has_as_feature:
            logging.info("Skipping Step 4d due to missing MSL and AS features")
            self._placeholder_for_skip("4d")
        else:
            # Steb 4d: TH expects report of LongPress, LongRelease in that order.
            self._placeholder_for_step("4d")
            logging.info(f"Starting to wait for {post_prompt_settle_delay_seconds:.1f} seconds for LongPress then LongRelease.")
            expected_events = []
            expected_events.append(cluster.Events.LongPress(newPosition=switch_pressed_position))
            expected_events.append(cluster.Events.LongRelease(previousPosition=switch_pressed_position))
            self._await_sequence_of_events(event_queue=event_listener.event_queue, endpoint_id=endpoint_id,
                                           sequence=expected_events, timeout_sec=post_prompt_settle_delay_seconds)

        # Step 4e: For MS & (!MSL & !AS & !MSR), expect no further events for 10 seconds.
        if not has_msl_feature and not has_as_feature and not has_msr_feature:
            self._placeholder_for_step("4e")
            self._expect_no_events_for_cluster(event_queue=event_listener.event_queue,
                                               endpoint_id=endpoint_id, expected_cluster=cluster, timeout_sec=10.0)

        # Step 4f: For MSR & not MSL, expect to see ShortRelease.
        if not has_msl_feature and has_msr_feature:
            self._placeholder_for_step("4f")
            expected_events = [cluster.Events.ShortRelease(previousPosition=switch_pressed_position)]
            self._await_sequence_of_events(event_queue=event_listener.event_queue, endpoint_id=endpoint_id,
                                           sequence=expected_events, timeout_sec=post_prompt_settle_delay_seconds)


if __name__ == "__main__":
    default_matter_test_main()
