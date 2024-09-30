#
#    Copyright (c) 2024 Project CHIP Authors
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
# test-runner-runs: run1 run2 run3 run4
#
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --endpoint 1 --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto --PICS src/app/tests/suites/certification/ci-pics-values
#
# test-runner-run/run2/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run2/factoryreset: True
# test-runner-run/run2/quiet: True
# test-runner-run/run2/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run2/script-args: --endpoint 2 --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto --PICS src/app/tests/suites/certification/ci-pics-values
#
# test-runner-run/run3/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run3/factoryreset: True
# test-runner-run/run3/quiet: True
# test-runner-run/run3/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run3/script-args: --endpoint 3 --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto --PICS src/app/tests/suites/certification/ci-pics-values
#
# test-runner-run/run4/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run4/factoryreset: True
# test-runner-run/run4/quiet: True
# test-runner-run/run4/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run4/script-args: --endpoint 4 --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto --PICS src/app/tests/suites/certification/ci-pics-values
#
# === END CI TEST ARGUMENTS ===
#
# These tests run on every endpoint regardless of whether a switch is present because they are set up to auto-select.

import json
import logging
import queue
import time
from datetime import datetime, timedelta
from typing import Any

import chip.clusters as Clusters
import test_plan_support
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import EventReadResult
from chip.tlv import uint
from matter_testing_support import (AttributeValue, ClusterAttributeChangeAccumulator, EventChangeCallback, MatterBaseTest,
                                    TestStep, await_sequence_of_reports, default_matter_test_main, has_feature,
                                    run_if_endpoint_matches)
from mobly import asserts

logger = logging.getLogger(__name__)

SIMULATED_LONG_PRESS_LENGTH_SECONDS = 2.0


def bump_substep(step: str) -> str:
    """Given a string like "5a", bump it to "5b", or "6c" to "6d" """
    if len(step) == 0:
        raise ValueError("Can't bump empty steps!")

    end_char = step[-1]
    if not end_char.isalpha():
        return step + "a"

    step_prefix = step[:-1]
    next_end_char = chr(ord(end_char) + 1)
    if ord(next_end_char) > ord('z'):
        raise ValueError(f"Reached max substep for step '{step}'")
    next_step = step_prefix + next_end_char

    return next_step


class TC_SwitchTests(MatterBaseTest):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._default_pressed_position = self.user_params.get("default_pressed_position", 1)

    def setup_test(self):
        super().setup_test()
        self.is_ci = self._use_button_simulator()

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

    def _send_multi_press_named_pipe_command(self, endpoint_id: int, number_of_presses: int, pressed_position: int, feature_map: uint, multi_press_max: uint):
        command_dict = {"Name": 'SimulateMultiPress', "EndpointId": endpoint_id,
                        "ButtonId": pressed_position, "MultiPressPressedTimeMillis": 500, "MultiPressReleasedTimeMillis": 500,
                        "MultiPressNumPresses": number_of_presses, "FeatureMap": feature_map, "MultiPressMax": multi_press_max}
        self._send_named_pipe_command(command_dict)

    def _send_long_press_named_pipe_command(self, endpoint_id: int, pressed_position: int, feature_map: int):
        command_dict = {"Name": "SimulateLongPress", "EndpointId": endpoint_id,
                        "ButtonId": pressed_position, "LongPressDelayMillis": int(1000 * (SIMULATED_LONG_PRESS_LENGTH_SECONDS - 0.5)),
                        "LongPressDurationMillis": int(1000 * SIMULATED_LONG_PRESS_LENGTH_SECONDS), "FeatureMap": feature_map}
        self._send_named_pipe_command(command_dict)

    def _send_latching_switch_named_pipe_command(self, endpoint_id: int, new_position: int):
        command_dict = {"Name": "SimulateLatchPosition", "EndpointId": endpoint_id, "PositionId": new_position}
        self._send_named_pipe_command(command_dict)

    def _send_switch_idle_named_pipe_command(self, endpoint_id: int):
        command_dict = {"Name": "SimulateSwitchIdle", "EndpointId": endpoint_id}
        self._send_named_pipe_command(command_dict)

    def _ask_for_switch_idle(self, endpoint_id: int, omit_for_simulator: bool = False):
        if not self._use_button_simulator():
            self.wait_for_user_input(prompt_msg="Ensure switch is idle")
        elif not omit_for_simulator:
            self._send_switch_idle_named_pipe_command(endpoint_id)

    def _ask_for_switch_position(self, endpoint_id: int, new_position: int):
        if not self._use_button_simulator():
            self.wait_for_user_input(prompt_msg=f"Move latched switch to position {new_position}, if it is not already there.")
        else:
            self._send_latching_switch_named_pipe_command(endpoint_id, new_position)

    def _ask_for_multi_press_short_long(self, endpoint_id: int, pressed_position: int, feature_map: uint, multi_press_max: uint):
        if not self._use_button_simulator():
            msg = f"""
                Actuate the switch in the following sequence (read full sequence first):
                1. Operate switch (press briefly) associated with position {pressed_position} on the DUT then release switch from DUT
                2. Operate switch (keep pressed for long time, e.g. 5 seconds) on the DUT immediately after the previous step
                3. Release switch from the DUT
                """
            self.wait_for_user_input(msg)
        else:
            # This is just a simulator, ignore the long press instruction for now, it doesn't matter for the CI. It does for cert.
            self._send_multi_press_named_pipe_command(
                endpoint_id, number_of_presses=2, pressed_position=pressed_position, feature_map=feature_map, multi_press_max=multi_press_max)

    def _ask_for_multi_press_long_short(self, endpoint_id, pressed_position, feature_map: int):
        if not self._use_button_simulator():
            msg = f"""
                  Actuate the switch in the following sequence (read full sequence first):
                  1. Operate switch (keep pressed for long time, e.g. 5 seconds) on the DUT
                  2. Releases switch from the DUT
                  3. Immediately after the previous step completes, operate switch (press briefly) associated with position {pressed_position} on the DUT then release switch from DUT
                  """
            self.wait_for_user_input(msg)
        else:
            # This is just the start of the sequence
            # we'll need to send the short press after getting the LongRelease event because the simulator doesn't queue requests.
            self._send_long_press_named_pipe_command(endpoint_id, pressed_position, feature_map)

    def _ask_for_multi_press(self, endpoint_id: int, number_of_presses: int, pressed_position: int, feature_map: uint, multi_press_max: uint):
        if not self._use_button_simulator():
            self.wait_for_user_input(
                f'Execute {number_of_presses} separate brief press/release cycles on position {pressed_position}.')
        else:
            self._send_multi_press_named_pipe_command(endpoint_id, number_of_presses,
                                                      pressed_position, feature_map, multi_press_max)

    def _ask_for_long_press(self, endpoint_id: int, pressed_position: int, feature_map):
        if not self._use_button_simulator():
            self.wait_for_user_input(
                prompt_msg=f"Press switch position {pressed_position} for a long time (around 5 seconds) on the DUT, then release it.")
        else:
            self._send_long_press_named_pipe_command(endpoint_id, pressed_position, feature_map)

    def _ask_for_keep_pressed(self, endpoint_id: int, pressed_position: int, feature_map: int):
        if not self._use_button_simulator():
            self.wait_for_user_input(
                prompt_msg=f"Press switch position {pressed_position} for a long time (around 5 seconds) on the DUT, keep it pressed, do NOT release it.")
        else:
            self._send_long_press_named_pipe_command(endpoint_id, pressed_position, feature_map)

    def _ask_for_release(self):
        # Since we used a long press for this, "ask for release" on the button simulator just means waiting out the delay
        if not self._use_button_simulator():
            self.wait_for_user_input(
                prompt_msg="Release the button."
            )
        else:
            # This will await for the events to be generated properly. Note that there is a bit of a
            # race here for the button simulator, but this race is extremely unlikely to be lost.
            time.sleep(SIMULATED_LONG_PRESS_LENGTH_SECONDS + 0.5)

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

    def _received_event(self, event_listener: EventChangeCallback, target_event: ClusterObjects.ClusterEvent, timeout_s: int) -> bool:
        """
            Returns true if this event was received, false otherwise
        """
        remaining = timedelta(seconds=timeout_s)
        end_time = datetime.now() + remaining
        while (remaining.seconds > 0):
            try:
                event = event_listener.event_queue.get(timeout=remaining.seconds)
            except queue.Empty:
                return False

            if event.Header.EventId == target_event.event_id:
                return True
            remaining = end_time - datetime.now()
        return False

    def steps_TC_SWTCH_2_2(self):
        return [TestStep(1, test_plan_support.commission_if_required(), "", is_commissioning=True),
                TestStep(2, "Set up subscription to all events and attributes of Switch cluster on the endpoint"),
                TestStep(3, "Operator sets switch to first position on the DUT"),
                TestStep(4, "TH reads the CurrentPosition attribute from the DUT.", "Verify that the value is 0."),
                TestStep(5, "Operator sets switch to second position (one) on the DUT",
                         "Verify that the TH receives SwitchLatched event with NewPosition set to 1 from the DUT."),
                TestStep(6, "TH reads the CurrentPosition attribute from the DUT",
                         "Verify that the value is 1, and that a subscription report was received for that change."),
                TestStep(7, "If there are more than 2 positions, test subsequent positions of the DUT"),
                TestStep(8, "Operator sets switch to first position on the DUT."),
                TestStep(9, "Wait 10 seconds for event reports stable." "Verify that last SwitchLatched event received is for NewPosition 0."),
                TestStep(10, "TH reads the CurrentPosition attribute from the DUT",
                         "Verify that the value is 0, and that a subscription report was received for that change."),
                ]

    @run_if_endpoint_matches(has_feature(Clusters.Switch, Clusters.Switch.Bitmaps.Feature.kLatchingSwitch))
    async def test_TC_SWTCH_2_2(self):
        post_prompt_settle_delay_seconds = 10.0
        cluster = Clusters.Switch
        endpoint_id = self.matter_test_config.endpoint

        # Step 1: Commissioning - already done
        self.step(1)

        # Step 2: Set up subscription to all events of Switch cluster on the endpoint.
        self.step(2)
        event_listener = EventChangeCallback(cluster)
        await event_listener.start(self.default_controller, self.dut_node_id, endpoint=endpoint_id)
        attrib_listener = ClusterAttributeChangeAccumulator(cluster)
        await attrib_listener.start(self.default_controller, self.dut_node_id, endpoint=endpoint_id)

        # Pre-get number of positions for step 7 later.
        num_positions = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.NumberOfPositions)
        asserts.assert_greater(num_positions, 1, "Latching switch has only 1 position, this is impossible.")

        # Step 3: Operator sets switch to first position on the DUT.
        self.step(3)
        self._ask_for_switch_position(endpoint_id, new_position=0)

        # Step 4: TH reads the CurrentPosition attribute from the DUT.
        # Verify that the value is 0.
        self.step(4)
        button_val = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.CurrentPosition)
        asserts.assert_equal(button_val, 0, "Switch position value is not 0")

        attrib_listener.reset()
        event_listener.reset()

        for expected_switch_position in range(1, num_positions):
            # Step 5: Operator sets switch to second position (one) on the DUT",
            # Verify that the TH receives SwitchLatched event with NewPosition set to 1 from the DUT
            if expected_switch_position == 1:
                self.step(5)
            self._ask_for_switch_position(endpoint_id, expected_switch_position)

            data = event_listener.wait_for_event_report(cluster.Events.SwitchLatched, timeout_sec=post_prompt_settle_delay_seconds)
            logging.info(f"-> SwitchLatched event last received: {data}")
            asserts.assert_equal(data, cluster.Events.SwitchLatched(
                newPosition=expected_switch_position), "Did not get expected switch position")

            # Step 6: TH reads the CurrentPosition attribute from the DUT", "Verify that the value is 1
            if expected_switch_position == 1:  # Indicate step 7 only once
                self.step(6)
            button_val = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.CurrentPosition)
            asserts.assert_equal(button_val, expected_switch_position, f"Switch position is not {expected_switch_position}")
            logging.info(f"Checking to see if a report for {expected_switch_position} is received")
            attrib_listener.await_sequence_of_reports(attribute=cluster.Attributes.CurrentPosition, sequence=[
                                                      expected_switch_position], timeout_sec=post_prompt_settle_delay_seconds)

            # Step 7: If there are more than 2 positions, test subsequent positions of the DUT
            if expected_switch_position == 1:
                if num_positions > 2:  # Indicate step 7 only once
                    self.step(7)
                else:
                    self.skip_step(7)

            if num_positions > 2:
                logging.info("Looping for the other positions")

        # Step 8: Operator sets switch to first position on the DUT.
        self.step(8)
        event_listener.flush_events()
        self._ask_for_switch_position(endpoint_id, new_position=0)

        # Step 9: Wait 10 seconds for event reports stable.
        # Verify that last SwitchLatched event received is for NewPosition 0.
        self.step(9)
        time.sleep(10.0 if not self.is_ci else 1.0)

        expected_switch_position = 0
        last_event = event_listener.get_last_event()
        asserts.assert_is_not_none(last_event, "Did not get SwitchLatched events since last operator action.")
        last_event_data = last_event.Data
        logging.info(f"-> SwitchLatched event last received: {last_event_data}")
        asserts.assert_equal(last_event_data, cluster.Events.SwitchLatched(
            newPosition=expected_switch_position), "Did not get expected switch position")

        # Step 10: TH reads the CurrentPosition attribute from the DUT.
        # Verify that the value is 0
        self.step(10)

        button_val = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.CurrentPosition)
        asserts.assert_equal(button_val, 0, "Button value is not 0")

        logging.info(f"Checking to see if a report for {expected_switch_position} is received")
        expected_final_value = [AttributeValue(
            endpoint_id, attribute=cluster.Attributes.CurrentPosition, value=expected_switch_position)]
        attrib_listener.await_all_final_values_reported(expected_final_value, timeout_sec=post_prompt_settle_delay_seconds)

    def steps_TC_SWTCH_2_3(self):
        return [TestStep(1, test_plan_support.commission_if_required(), "", is_commissioning=True),
                TestStep(2, "Set up subscription to all events of Switch cluster on the endpoint"),
                TestStep(3, "Operator does not operate switch on the DUT"),
                TestStep(4, "TH reads the CurrentPosition attribute from the DUT", "Verify that the value is 0"),
                TestStep(5, "Operator operates switch (keep it pressed, and wait at least 5 seconds)",
                         "Verify that the TH receives InitialPress event with NewPosition set to 1 on the DUT"),
                TestStep(6, "TH reads the CurrentPosition attribute from the DUT", "Verify that the value is 1"),
                TestStep(7, "Operator releases switch on the DUT"),
                TestStep("8a", "If the DUT implements the MSR feature and does not implement the MSL feature, verify that the TH receives ShortRelease event with NewPosition set to 0 on the DUT", "Event received"),
                TestStep("8b", "If the DUT implements the MSR feature and the MSL feature, verify that the TH receives LongRelease event with NewPosition set to 0 on the DUT", "Event received"),
                TestStep(
                    "8c", "If the DUT implements the AS feature, verify that the TH does not receive ShortRelease event on the DUT", "No event received"),
                TestStep(9, "TH reads the CurrentPosition attribute from the DUT", "Verify that the value is 0"),
                ]

    @run_if_endpoint_matches(has_feature(Clusters.Switch, Clusters.Switch.Bitmaps.Feature.kMomentarySwitch))
    async def test_TC_SWTCH_2_3(self):
        # Commissioning - already done
        self.step(1)
        cluster = Clusters.Switch
        feature_map = await self.read_single_attribute_check_success(cluster, attribute=cluster.Attributes.FeatureMap)

        has_msr_feature = (feature_map & cluster.Bitmaps.Feature.kMomentarySwitchRelease) != 0
        has_msl_feature = (feature_map & cluster.Bitmaps.Feature.kMomentarySwitchLongPress) != 0
        has_as_feature = (feature_map & cluster.Bitmaps.Feature.kActionSwitch) != 0

        endpoint_id = self.matter_test_config.endpoint

        self.step(2)
        event_listener = EventChangeCallback(cluster)
        await event_listener.start(self.default_controller, self.dut_node_id, endpoint=endpoint_id)

        self.step(3)
        self._ask_for_switch_idle(endpoint_id)

        self.step(4)
        button_val = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.CurrentPosition)
        asserts.assert_equal(button_val, 0, "Button value is not 0")

        self.step(5)
        # We're using a long press here with a very long duration (in computer-land). This will let us check the intermediate values.
        # This is 1s larger than the subscription ceiling
        self.pressed_position = 1
        self._ask_for_keep_pressed(endpoint_id, self.pressed_position, feature_map)
        event_listener.wait_for_event_report(cluster.Events.InitialPress)

        self.step(6)
        button_val = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.CurrentPosition)
        asserts.assert_equal(button_val, self.pressed_position, f"Button value is not {self.pressed_position}")

        self.step(7)
        self._ask_for_release()

        self.step("8a")
        if has_msr_feature and not has_msl_feature:
            asserts.assert_true(self._received_event(event_listener, cluster.Events.ShortRelease, 10),
                                "Did not receive short release")
        else:
            self.mark_current_step_skipped()

        self.step("8b")
        if has_msr_feature and has_msl_feature:
            asserts.assert_true(self._received_event(event_listener, cluster.Events.LongRelease, 10),
                                "Did not receive long release")

        self.step("8c")
        if has_as_feature:
            asserts.assert_false(self._received_event(event_listener, cluster.Events.ShortRelease, 10), "Received short release")
        else:
            self.mark_current_step_skipped()

        self.step(9)
        button_val = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.CurrentPosition)
        asserts.assert_equal(button_val, 0, "Button value is not 0")

    def desc_TC_SWTCH_2_4(self) -> str:
        return "[TC-SWTCH-2.4] Momentary Switch Long Press Verification"

    def steps_TC_SWTCH_2_4(self):
        return [TestStep(1, test_plan_support.commission_if_required(), "", is_commissioning=True),
                TestStep(2, "Set up subscription to all events and attributes of Switch cluster on the endpoint"),
                TestStep(3, "Operator does not operate switch on the DUT"),
                TestStep(4, "TH reads the CurrentPosition attribute from the DUT", "Verify that the value is 0"),
                TestStep(5, "Operator operates switch (keep pressed for long time, e.g. 5 seconds) on the DUT, then release it",
                """
                * TH expects receiving a subscription report of CurrentPosition 1, followed by a report of Current Position 0.
                * TH expects receiving at InitialPress event with NewPosition = 1.
                * if MSL feature is supported, TH expect receiving LongPress/LongRelease in that order.
                * if MS & (!MSL & !AS & !MSR & !MSM) features present, TH expects receiving no further events for 10 seconds after release.
                * if (MSR & !MSL) features present, TH expects receiving ShortRelease event.
                """)
                ]

    @run_if_endpoint_matches(has_feature(Clusters.Switch, Clusters.Switch.Bitmaps.Feature.kMomentarySwitch))
    async def test_TC_SWTCH_2_4(self):
        switch_pressed_position = self._default_pressed_position
        post_prompt_settle_delay_seconds = 10.0

        endpoint_id = self.matter_test_config.endpoint
        cluster = Clusters.Objects.Switch

        # Step 1: Commission DUT - already done
        self.step(1)

        # Read feature map to set bool markers
        feature_map = await self.read_single_attribute_check_success(cluster, attribute=cluster.Attributes.FeatureMap)

        has_ms_feature = (feature_map & cluster.Bitmaps.Feature.kMomentarySwitch) != 0
        has_msr_feature = (feature_map & cluster.Bitmaps.Feature.kMomentarySwitchRelease) != 0
        has_msl_feature = (feature_map & cluster.Bitmaps.Feature.kMomentarySwitchLongPress) != 0
        has_msm_feature = (feature_map & cluster.Bitmaps.Feature.kMomentarySwitchMultiPress) != 0
        has_as_feature = (feature_map & cluster.Bitmaps.Feature.kActionSwitch) != 0

        if not has_ms_feature:
            logging.info("Skipping rest of test: SWTCH.S.F01(MS) feature not present")
            self.skip_all_remaining_steps("2")

        # Step 2: Set up subscription to all events and attributes of Switch cluster on the endpoint
        self.step(2)
        event_listener = EventChangeCallback(cluster)
        await event_listener.start(self.default_controller, self.dut_node_id, endpoint=endpoint_id)
        attrib_listener = ClusterAttributeChangeAccumulator(cluster)
        await attrib_listener.start(self.default_controller, self.dut_node_id, endpoint=endpoint_id)

        # Step 3: Operator does not operate switch on the DUT
        self.step(3)
        self._ask_for_switch_idle(endpoint_id)

        # Step 4: TH reads the CurrentPosition attribute from the DUT
        self.step(4)

        # Verify that the value is 0
        current_position = await self.read_single_attribute_check_success(cluster, attribute=cluster.Attributes.CurrentPosition)
        asserts.assert_equal(current_position, 0)

        # Step 5: Operator operates switch (keep pressed for long time, e.g. 5 seconds) on the DUT, the release it
        self.step(5)
        self._ask_for_long_press(endpoint_id, switch_pressed_position, feature_map)

        # - TH expects report of CurrentPosition 1, followed by a report of Current Position 0.
        logging.info(
            f"Starting to wait for {post_prompt_settle_delay_seconds:.1f} seconds for CurrentPosition to go {switch_pressed_position}, then 0.")
        await_sequence_of_reports(report_queue=attrib_listener.attribute_queue, endpoint_id=endpoint_id, attribute=cluster.Attributes.CurrentPosition, sequence=[
                                  switch_pressed_position, 0], timeout_sec=post_prompt_settle_delay_seconds)

        # - TH expects at least InitialPress with NewPosition = 1
        logging.info(f"Starting to wait for {post_prompt_settle_delay_seconds:.1f} seconds for InitialPress event.")
        expected_events = [cluster.Events.InitialPress(newPosition=switch_pressed_position)]
        self._await_sequence_of_events(event_queue=event_listener.event_queue, endpoint_id=endpoint_id,
                                       sequence=expected_events, timeout_sec=post_prompt_settle_delay_seconds)

        # - if MSL feature is supported, expect to see LongPress/LongRelease in that order.
        if not has_msl_feature:
            logging.info("Since MSL feature unsupported, skipping check for LongPress/LongRelease")
        else:
            # - TH expects report of LongPress, LongRelease in that order.
            logging.info(f"Starting to wait for {post_prompt_settle_delay_seconds:.1f} seconds for LongPress then LongRelease.")
            expected_events = []
            expected_events.append(cluster.Events.LongPress(newPosition=switch_pressed_position))
            expected_events.append(cluster.Events.LongRelease(previousPosition=switch_pressed_position))
            self._await_sequence_of_events(event_queue=event_listener.event_queue, endpoint_id=endpoint_id,
                                           sequence=expected_events, timeout_sec=post_prompt_settle_delay_seconds)

        # - if MS & (!MSL & !AS & !MSR & !MSM) features present, expect no further events for 10 seconds after release.
        if not has_msl_feature and not has_as_feature and not has_msr_feature and not has_msm_feature:
            self._expect_no_events_for_cluster(event_queue=event_listener.event_queue,
                                               endpoint_id=endpoint_id, expected_cluster=cluster, timeout_sec=10.0)

        # - if (MSR & !MSL) features present, expect to see ShortRelease event.
        if not has_msl_feature and has_msr_feature:
            expected_events = [cluster.Events.ShortRelease(previousPosition=switch_pressed_position)]
            self._await_sequence_of_events(event_queue=event_listener.event_queue, endpoint_id=endpoint_id,
                                           sequence=expected_events, timeout_sec=post_prompt_settle_delay_seconds)

    def steps_TC_SWTCH_2_5(self):
        return [TestStep(1, test_plan_support.commission_if_required(), "", is_commissioning=True),
                TestStep(2, "Set up a subscription to all Switch cluster events"),
                TestStep(3, "Operate does not operate the switch on the DUT"),
                TestStep("4a", "Operator operates switch (press briefly) associated with position 1 on the DUT then release switch from DUT",
                         """

                         * Verify that the TH receives InitialPress event with NewPosition set to 1 from the DUT
                         * Verify that the TH receives ShortRelease event with PreviousPosition set to 1 from the DUT
                        """),
                TestStep("4b", "Operator does not operate switch on the DUT",
                         "TH receives MultiPressComplete event with PreviousPosition set to 1 and TotalNumberOfPressesCounted set to 1 from the DUT"),
                TestStep("5a", "Operator repeat step 4a 2 times quickly",
                         """

                         * Verify that the TH receives InitialPress event with NewPosition set to 1 from the DUT
                         * Verify that the TH receives ShortRelease event with PreviousPosition set to 1 from the DUT
                         * Verify that the TH receives InitialPress event with NewPosition set to 1 from the DUT
                         * Verify that the TH receives MultiPressOngoing event with NewPosition set to 1 and CurrentNumberOfPressesCounted set to 2 from the DUT
                         * Verify that the TH receives ShortRelease event with PreviousPosition set to 1 from the DUT +

                         The events sequence SHALL follow the same sequence as above
                         """),
                TestStep("5b", "Operator does not operate switch on the DUT",
                         "Verify that the TH receives MultiPressComplete event with PreviousPosition set to 1 and TotalNumberOfPressesCounted set to 2 from the DUT"),
                TestStep("6a", "If MultiPressMax == 2 (see 2c of TC-SWTCH-2.1), skip steps 6b .. 6c"),
                TestStep("6b", "Operator repeat step 4a 3 times quickly",
                         """
                         * Verify that the TH receives InitialPress event with NewPosition set to 1 from the DUT
                         * If MSR supported, Verify that the TH receives ShortRelease event with PreviousPosition set to 1 from the DUT
                         * Verify that the TH receives InitialPress event with NewPosition set to 1 from the DUT
                         * Verify that the TH receives MultiPressOngoing event with NewPosition set to 1 and CurrentNumberOfPressesCounted set to 2 from the DUT
                         * If MSR supported, Verify that the TH receives ShortRelease event with PreviousPosition set to 1 from the DUT
                         * Verify that the TH receives InitialPress event with NewPosition set to 1 from the DUT
                         * Verify that the TH receives MultiPressOngoing event with NewPosition set to 1 and CurrentNumberOfPressesCounted set to 3 from the DUT
                         * If MSR supported, Verify that the TH receives ShortRelease event with PreviousPosition set to 1 from the DUT +

                         The events sequence from the subscription SHALL follow the same sequence as expressed above, in the exact order of events specified.
                         """),
                TestStep("6c", "Operator does not operate switch on the DUT for 5 seconds",
                         "Verify that the TH receives MultiPressComplete event with PreviousPosition set to 1 and TotalNumberOfPressesCounted set to 3 from the DUT"),
                TestStep(7, "Set up subscription to all Switch cluster events"),
                TestStep("8a",
                         """
                         Operator operates switch in below sequence:
                         1. Operator operates switch (press briefly) associated with position 1 on the DUT then release switch from DUT
                         2. Operator operates switch (keep pressed for long time, e.g. 5 seconds) on the DUT immediately after the previous step
                         3. Operator releases switch from the DUT
                         """,
                         """

                         * Verify that the TH receives InitialPress event with NewPosition set to 1 from the DUT
                         * If MSR supported, Verify that the TH receives ShortRelease event with PreviousPosition set to 1 from the DUT
                         * Verify that the TH receives InitialPress event with NewPosition set to 1 from the DUT

                         * Verify that the TH receives MultiPressOngoing event with NewPosition set to 1 and CurrentNumberOfPressesCounted set to 2 from the DUT
                         * If MSR supported, verify that the TH receives ShortRelease event with PreviousPosition set to 1 from the DUT
                         * Verify that the TH does not receive LongPress event from the DUT
                         * Verify that the TH does not receive LongRelease event from the DUT

                         The events sequence from the subscription SHALL follow the same sequence as expressed above, in the exact order of events specified.
                         """),
                TestStep("8b", "Operator does not operate switch on the DUT",
                         "TH receives MultiPressComplete event with PreviousPosition set to 1 and TotalNumberOfPressesCounted set to 2 from the DUT"),
                TestStep("9a",
                         """
                         Operator operates switch in below sequence:
                         1. Operator operates switch (keep pressed for long time, e.g. 5 seconds) on the DUT
                         2. Operator releases switch from the DUT
                         3. Immediately after the previous step completes, Operator operates switch (press briefly) associated with position 1 on the DUT then release switch from DUT
                         """,
                         """

                         * Verify that the TH receives InitialPress event with NewPosition set to 1 from the DUT
                         * Verify that the TH receives (one, not more than one) LongPress event with NewPosition set to 1 from the DUT
                         * Verify that the TH receives LongRelease event with PreviousPosition set to 1 from the DUT
                         * Verify that the TH receives InitialPress event with NewPosition set to 1 from the DUT
                         * If MSR supported, verify that the TH receives ShortRelease event with PreviousPosition set to 1 from the DUT
                         * Verify that the TH does not receive MultiPressOngoing event from the DUT

                         The events sequence from the subscription SHALL follow the same sequence as expressed above, in the exact order of events specified.
                         """),
                TestStep("9b", "Operator does not operate switch on the DUT",
                         "TH receives MultiPressComplete event with PreviousPosition set to 1 and TotalNumberOfPressesCounted set to 2 from the DUT")

                ]

    @staticmethod
    def should_run_SWTCH_2_5(wildcard, endpoint):
        msm = has_feature(Clusters.Switch, Clusters.Switch.Bitmaps.Feature.kMomentarySwitchMultiPress)
        asf = has_feature(Clusters.Switch, Clusters.Switch.Bitmaps.Feature.kActionSwitch)
        return msm(wildcard, endpoint) and not asf(wildcard, endpoint)

    @run_if_endpoint_matches(should_run_SWTCH_2_5)
    async def test_TC_SWTCH_2_5(self):
        # Commissioning - already done
        self.step(1)

        cluster = Clusters.Switch
        feature_map = await self.read_single_attribute_check_success(cluster, attribute=cluster.Attributes.FeatureMap)
        has_msl_feature = (feature_map & cluster.Bitmaps.Feature.kMomentarySwitchLongPress)
        has_msr_feature = (feature_map & cluster.Bitmaps.Feature.kMomentarySwitchRelease)
        multi_press_max = await self.read_single_attribute_check_success(cluster, attribute=cluster.Attributes.MultiPressMax)

        endpoint_id = self.matter_test_config.endpoint
        pressed_position = self._default_pressed_position

        self.step(2)
        event_listener = EventChangeCallback(cluster)
        await event_listener.start(self.default_controller, self.dut_node_id, endpoint=endpoint_id)

        self.step(3)
        self._ask_for_switch_idle(endpoint_id)

        def test_multi_press_sequence(starting_step: str, count: int, short_long: bool = False):
            step = starting_step
            self.step(step)

            if short_long:
                self._ask_for_multi_press_short_long(endpoint_id, pressed_position,
                                                     feature_map=feature_map, multi_press_max=multi_press_max)
            else:
                self._ask_for_multi_press(endpoint_id, number_of_presses=count, pressed_position=pressed_position,
                                          feature_map=feature_map, multi_press_max=multi_press_max)
            for pos_idx in range(count):
                event = event_listener.wait_for_event_report(cluster.Events.InitialPress)
                asserts.assert_equal(event.newPosition, pressed_position, "Unexpected NewPosition on InitialEvent")
                if pos_idx > 0:
                    event = event_listener.wait_for_event_report(cluster.Events.MultiPressOngoing)
                    asserts.assert_equal(event.newPosition, pressed_position, "Unexpected NewPosition on MultiPressOngoing")
                    asserts.assert_equal(event.currentNumberOfPressesCounted, pos_idx + 1,
                                         "Unexpected CurrentNumberOfPressesCounted on MultiPressOngoing")
                if has_msr_feature:
                    event = event_listener.wait_for_event_report(cluster.Events.ShortRelease)
                    asserts.assert_equal(event.previousPosition, pressed_position, "Unexpected PreviousPosition on ShortRelease")

            step = bump_substep(step)
            self.step(step)
            self._ask_for_switch_idle(endpoint_id, omit_for_simulator=True)
            event = event_listener.wait_for_event_report(cluster.Events.MultiPressComplete)
            asserts.assert_equal(event.previousPosition, pressed_position, "Unexpected PreviousPosition on MultiPressComplete")
            asserts.assert_equal(event.totalNumberOfPressesCounted, count, "Unexpected count on MultiPressComplete")

        test_multi_press_sequence("4a", count=1)

        test_multi_press_sequence("5a", count=2)

        self.step("6a")
        multi_press_max = await self.read_single_attribute_check_success(cluster=cluster, attribute=cluster.Attributes.MultiPressMax)
        if multi_press_max == 2:
            self.skip_step("6b")
            self.skip_step("6c")
        else:
            test_multi_press_sequence("6b", count=3)

        if not has_msl_feature:
            self.skip_all_remaining_steps(7)
            return

        self.step(7)
        # subscription is already set up

        test_multi_press_sequence("8a", count=2, short_long=True)

        self.step("9a")
        self._ask_for_multi_press_long_short(endpoint_id, pressed_position, feature_map)

        event = event_listener.wait_for_event_report(cluster.Events.InitialPress)
        asserts.assert_equal(event.newPosition, pressed_position, "Unexpected NewPosition on InitialEvent")
        event = event_listener.wait_for_event_report(cluster.Events.LongPress)
        asserts.assert_equal(event.newPosition, pressed_position, "Unexpected NewPosition on LongPress")
        event = event_listener.wait_for_event_report(cluster.Events.LongRelease)
        asserts.assert_equal(event.previousPosition, pressed_position, "Unexpected PreviousPosition on LongRelease")
        if self._use_button_simulator():
            # simulator can't sequence so we need to help it along here
            self._send_multi_press_named_pipe_command(endpoint_id, number_of_presses=1,
                                                      pressed_position=1, feature_map=feature_map, multi_press_max=multi_press_max)

        event = event_listener.wait_for_event_report(cluster.Events.InitialPress)
        asserts.assert_equal(event.newPosition, pressed_position, "Unexpected NewPosition on InitialEvent")

        if has_msr_feature:
            event = event_listener.wait_for_event_report(cluster.Events.ShortRelease)
            asserts.assert_equal(event.previousPosition, pressed_position, "Unexpected PreviousPosition on ShortRelease")

        # Because this is a queue, we verify that no multipress ongoing is received by verifying that the next event is the multipress complete

        self.step("9b")
        self._ask_for_switch_idle(endpoint_id, omit_for_simulator=True)
        event = event_listener.wait_for_event_report(cluster.Events.MultiPressComplete)
        asserts.assert_equal(event.previousPosition, pressed_position, "Unexpected PreviousPosition on MultiPressComplete")
        asserts.assert_equal(event.totalNumberOfPressesCounted, 1, "Unexpected count on MultiPressComplete")

    def steps_TC_SWTCH_2_6(self):
        return [TestStep(1, test_plan_support.commission_if_required(), is_commissioning=True),
                TestStep(2, "Set up subscription to all Switch cluster events"),
                TestStep(3, "Operator does not operate switch on the DUT"),
                TestStep("4a", "Operator operates switch (press briefly) associated with position 1 on the DUT then release switch from DUT",
                         """

                            * Verify that the TH receives InitialPress event with NewPosition set to 1 from the DUT
                            * Verify that the TH does not receive ShortRelease event from the DUT
                            """),
                TestStep("4b", "Operator does not operate switch on the DUT",
                         "TH receives MultiPressComplete event with PreviousPosition set to 1 and TotalNumberOfPressesCounted set to 1 from the DUT"),
                TestStep("5a", "Operator repeat step 4a 2 times quickly",
                         """

                         * Verify that the TH receives InitialPress(one, not more than one) event with NewPosition set to 1 from the DUT
                         * Verify that the TH does not receive ShortRelease event from the DUT
                         * Verify that the TH does not receive MultiPressOngoing event from the DUT
                         """),
                TestStep("5b", "Operator does not operate switch on the DUT",
                         "Verify that the TH receives MultiPressComplete event with PreviousPosition set to 1 and TotalNumberOfPressesCounted set to 2 from the DUT"),
                TestStep("6a", "Operator repeat step 4a MultiPressMax + 1(see 2c of TC-SWTCH-2.1) times quickly",
                         """

                         * Verify that the TH receives InitialPress(one, not more than one) event with NewPosition set to 1 from the DUT
                         * Verify that the TH does not receive ShortRelease event from the DUT
                         * Verify that the TH does not receive MultiPressOngoing event from the DUT
                         """
                         ),
                TestStep("6b", "Operator does not operate switch on the DUT",
                         "Verify that the TH receives MultiPressComplete event with PreviousPosition set to 1 and TotalNumberOfPressesCounted set to 0 from the DUT"),
                TestStep("7a", "If the switch cluster does not implement the MomentarySwitchLongPress (MSL) feature, skip the remaining steps"),
                TestStep("7b", "Set up subscription to all Switch cluster events"),
                TestStep("8a",
                         """
                         Operator operates switch in below sequence:
                         1. Operator operates switch (press briefly) associated with position 1 on the DUT then release switch from DUT
                         2. Operator operates switch (keep pressed for long time, e.g. 5 seconds) on the DUT immediately after the previous step
                         3. Operator releases switch from the DUT
                         """,
                         """

                         * Verify that the TH receives InitialPress(one, not more than one) event with NewPosition set to 1 from the DUT
                         * Verify that the TH does not receive ShortRelease event from the DUT
                         * Verify that the TH does not receive MultiPressOngoing event from the DUT
                         * Verify that the TH does not receive LongPress event from the DUT
                         * Verify that the TH does not receive LongRelease event from the DUT
                         """),
                TestStep("8b", "Operator does not operate switch on the DUT",
                         "TH receives MultiPressComplete event with PreviousPosition set to 1 and TotalNumberOfPressesCounted set to 2 from the DUT"),
                TestStep("9a",
                         """
                         Operator operates switch in below sequence:

                         1. Operator operates switch (keep pressed for long time, e.g. 5 seconds) on the DUT
                         2. Operator releases switch from the DUT
                         3. Immediately after the previous step complete, Operator operates switch (press briefly) associated with position 1 on the DUT then release switch from DUT
                         """,
                         """

                         * Verify that the TH receives InitialPress event with NewPosition set to 1 from the DUT
                         * Verify that the TH receives (one, not more than one) LongPress event with NewPosition set to 1 from the DUT
                         * Verify that the TH receives LongRelease event with PreviousPosition set to 1 from the DUT
                         * Verify that the TH receives InitialPress event with NewPosition set to 1 from the DUT
                         * Verify that the TH does not receive MultiPressOngoing event from the DUT
                         * Verify that the TH does not receive ShortRelease event from the DUT

                         The events sequence from the subscription SHALL follow the same sequence as expressed above, in the exact order of events specified.
                         """),
                TestStep("9b", "Operator does not operate switch on the DUT"
                         "Verify that the TH receives MultiPressComplete event with PreviousPosition set to 1 and TotalNumberOfPressesCounted set to 1 from the DUT"),
                ]

    @staticmethod
    def should_run_SWTCH_2_6(wildcard, endpoint):
        msm = has_feature(Clusters.Switch, Clusters.Switch.Bitmaps.Feature.kMomentarySwitchMultiPress)
        asf = has_feature(Clusters.Switch, 0x20)
        return msm(wildcard, endpoint) and asf(wildcard, endpoint)

    @run_if_endpoint_matches(should_run_SWTCH_2_6)
    async def test_TC_SWTCH_2_6(self):
        # Commissioning - already done
        self.step(1)

        cluster = Clusters.Switch
        feature_map = await self.read_single_attribute_check_success(cluster, attribute=cluster.Attributes.FeatureMap)
        has_msl_feature = (feature_map & cluster.Bitmaps.Feature.kMomentarySwitchLongPress)
        multi_press_max = await self.read_single_attribute_check_success(cluster, attribute=cluster.Attributes.MultiPressMax)

        endpoint_id = self.matter_test_config.endpoint
        pressed_position = self._default_pressed_position

        self.step(2)
        event_listener = EventChangeCallback(cluster)
        await event_listener.start(self.default_controller, self.dut_node_id, endpoint=endpoint_id)

        self.step(3)
        self._ask_for_switch_idle(endpoint_id)

        def test_multi_press_sequence(starting_step: str, count: int, short_long: bool = False):
            step = starting_step
            self.step(step)

            if short_long:
                self._ask_for_multi_press_short_long(endpoint_id, pressed_position,
                                                     feature_map=feature_map, multi_press_max=multi_press_max)
            else:
                self._ask_for_multi_press(endpoint_id, number_of_presses=count, pressed_position=pressed_position,
                                          feature_map=feature_map, multi_press_max=multi_press_max)

            event = event_listener.wait_for_event_report(cluster.Events.InitialPress)
            asserts.assert_equal(event.newPosition, pressed_position, "Unexpected NewPosition on InitialEvent")

            step = bump_substep(step)
            self.step(step)
            self._ask_for_switch_idle(endpoint_id, omit_for_simulator=True)
            event = event_listener.wait_for_event_report(cluster.Events.MultiPressComplete)
            asserts.assert_equal(event.previousPosition, pressed_position, "Unexpected PreviousPosition on MultiPressComplete")
            expected_count = 0 if count > multi_press_max else count
            asserts.assert_equal(event.totalNumberOfPressesCounted, expected_count, "Unexpected count on MultiPressComplete")

        test_multi_press_sequence("4a", count=1)

        test_multi_press_sequence("5a", count=2)

        test_multi_press_sequence("6a", count=(multi_press_max + 1))

        self.step("7a")
        if not has_msl_feature:
            self.skip_all_remaining_steps("7b")
            return

        # subscription is already established
        self.step("7b")

        test_multi_press_sequence("8a", count=2, short_long=True)

        self.step("9a")
        self._ask_for_multi_press_long_short(endpoint_id, pressed_position, feature_map)

        event = event_listener.wait_for_event_report(cluster.Events.InitialPress)
        asserts.assert_equal(event.newPosition, pressed_position, "Unexpected NewPosition on InitialEvent")
        event = event_listener.wait_for_event_report(cluster.Events.LongPress)
        asserts.assert_equal(event.newPosition, pressed_position, "Unexpected NewPosition on LongPress")
        event = event_listener.wait_for_event_report(cluster.Events.LongRelease)
        asserts.assert_equal(event.previousPosition, pressed_position, "Unexpected PreviousPosition on LongRelease")
        if self._use_button_simulator():
            # simulator can't sequence so we need to help it along here
            self._send_multi_press_named_pipe_command(endpoint_id, number_of_presses=1,
                                                      pressed_position=1, feature_map=feature_map, multi_press_max=multi_press_max)

        event = event_listener.wait_for_event_report(cluster.Events.InitialPress)
        asserts.assert_equal(event.newPosition, pressed_position, "Unexpected NewPosition on InitialEvent")

        # Verify that we don't receive the multi-press ongoing or short release by verifying that the next event in the sequence is the multi-press complete
        self.step("9b")
        self._ask_for_switch_idle(endpoint_id, omit_for_simulator=True)
        event = event_listener.wait_for_event_report(cluster.Events.MultiPressComplete)
        asserts.assert_equal(event.previousPosition, pressed_position, "Unexpected PreviousPosition on MultiPressComplete")
        asserts.assert_equal(event.totalNumberOfPressesCounted, 1, "Unexpected count on MultiPressComplete")


if __name__ == "__main__":
    default_matter_test_main()
