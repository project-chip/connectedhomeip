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
#

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CLOSURE_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --enable-key 000102030405060708090a0b0c0d0e0f --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --timeout 30
#       --endpoint 1
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import time

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import (AttributeMatcher, AttributeValue, MatterBaseTest, TestStep, async_test_body,
                                           default_matter_test_main)

triggerError = 0x0104000000000000
triggerDisengaged = 0x0104000000000002
triggerClear = 0x0104000000000004


def current_position_matcher(current_position: Clusters.ClosureControl.Enums.CurrentPositionEnum) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallCurrentState:
            return False
        return report.value.position == current_position
    return AttributeMatcher.from_callable(description=f"OverallCurrentState.Position is {current_position}", matcher=predicate)


def current_latch_matcher(current_latch: bool) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallCurrentState:
            return False
        return report.value.latch == current_latch
    return AttributeMatcher.from_callable(description=f"OverallCurrentState.Latch is {current_latch}", matcher=predicate)


class TC_CLCTRL_6_1(MatterBaseTest):
    async def read_clctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_6_1(self) -> str:
        return "[TC_CLCTRL_6_1] Event Functionality with DUT as Server"

    def steps_TC_CLCTRL_6_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).", is_commissioning=True),
            TestStep("2a", "TH reads from the DUT the (0xFFFC) FeatureMap attribute."),
            TestStep("2b", "TH reads TestEventTriggerEnabled attribute from General Diagnostics Cluster."),
            TestStep("2c", "TH reads from the DUT the (0xFFFB) AttributeList attribute."),
            TestStep("2d", "TH establishes a wildcard subscription to all attributes on the Closure Control Cluster, with MinIntervalFloor = 0, MaxIntervalCeiling = 30 and KeepSubscriptions = false."),
            TestStep("3a", "If the LT feature is not supported on the cluster, skip steps 3b to 3i."),
            TestStep("3b", "If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState.Latch attribute."),
            TestStep("3c", "If the attribute is supported on the cluster, TH reads from the DUT the LatchControlModes attribute."),
            TestStep("3d", "If CurrentLatch = False, skip steps 3e to 3i."),
            TestStep("3e", "If LatchControlModes Bit 1 = 0 (RemoteUnlatching = False), skip step 3f."),
            TestStep("3f", "TH sends command MoveTo with Latch = False."),
            TestStep("3g", "If LatchControlModes Bit 1 = 1 (RemoteUnlatching = True), skip step 3h."),
            TestStep("3h", "Unlatch the DUT manually to set OverallCurrentState.Latch to False."),
            TestStep("3i", "Wait until TH receives a subscription report with OverallCurrentState.Latch."),
            TestStep("3j", "If the PS feature is not supported on the cluster, skip steps 3k to 3n."),
            TestStep("3k", "If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute."),
            TestStep("3l", "If CurrentPosition = FullyClosed, skip steps 3m to 3n."),
            TestStep("3m", "TH sends command MoveTo with Position = MoveToFullyClosed."),
            TestStep("3n", "Wait until TH receives a subscription report with OverallCurrentState.Position."),
            TestStep("4a", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Error(3) Test Event."),
            TestStep("4b", "Verify that the DUT has emitted the OperationalError event."),
            TestStep("4c", "TH reads from the DUT the CurrentErrorList attribute."),
            TestStep("4d", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear."),
            TestStep("5a", "If PS feature is not supported on the cluster or IS feature is supported on the cluster, skip steps 5b to 5f."),
            TestStep("5b", "TH sends command MoveTo with Position = MoveToFullyOpen."),
            TestStep("5c", "Wait until TH receives a subscription report with OverallCurrentState.Position = FullyOpened."),
            TestStep("5d", "TH sends command MoveTo with Position = MoveToFullyClosed."),
            TestStep("5e", "Verify that the DUT has emitted the MovementCompleted event."),
            TestStep("5f", "TH reads from the DUT the Mainstate attribute."),
            TestStep("6a", "If MO feature is not supported on the cluster, skip steps 6b to 6e."),
            TestStep("6b", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Disengaged(6) Test Event."),
            TestStep("6c", "Verify that the DUT has emitted the EngageStateChanged event."),
            TestStep("6d", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear."),
            TestStep("6e", "Verify that the DUT has emitted the EngageStateChanged event."),
            TestStep("7a", "If LT feature is supported on the cluster or PS feature is not supported on the cluster, skip steps 7b to 7f."),
            TestStep("7b", "If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute."),
            TestStep("7c", "TH sends command MoveTo with Position = MoveToFullyOpen."),
            TestStep("7d", "Verify that the DUT has emitted the SecureStateChanged event."),
            TestStep("7e", "TH sends command MoveTo with Position = MoveToFullyClosed."),
            TestStep("7f", "Verify that the DUT has emitted the SecureStateChanged event."),
            TestStep("8a", "If LT feature is not supported on the cluster or PS feature is supported on the cluster, skip steps 8b to 8l."),
            TestStep("8b", "If LatchControlModes Bit 0 = 0 (RemoteLatching = False), skip step 8c."),
            TestStep("8c", "TH sends command MoveTo with Latch = True."),
            TestStep("8d", "If LatchControlModes Bit 0 = 1 (RemoteLatching = True), skip step 8e."),
            TestStep("8e", "Latch the DUT manually to set OverallCurrentState.Latch to True."),
            TestStep("8f", "Wait until TH receives a subscription report with OverallCurrentState.Latch = True."),
            TestStep("8g", "Verify that the DUT has emitted the SecureStateChanged event."),
            TestStep("8h", "If LatchControlModes Bit 1 = 0 (RemoteUnlatching = False), skip step 8i."),
            TestStep("8i", "TH sends command MoveTo with Latch = False."),
            TestStep("8j", "If LatchControlModes Bit 1 = 1 (RemoteUnlatching = True), skip step 8k."),
            TestStep("8k", "Unlatch the DUT manually to set OverallCurrentState.Latch to False."),
            TestStep("8l", "Verify that the DUT has emitted the SecureStateChanged event."),
            TestStep("9a", "If LT feature is not supported on the cluster or PS feature is not supported on the cluster, skip steps 9b to 9l."),
            TestStep("9b", "If LatchControlModes Bit 0 = 0 (RemoteLatching = False), skip step 9c."),
            TestStep("9c", "TH sends command MoveTo with Latch = True."),
            TestStep("9d", "If LatchControlModes Bit 0 = 1 (RemoteLatching = True), skip step 9e."),
            TestStep("9e", "Latch the DUT manually to set OverallCurrentState.Latch to True."),
            TestStep("9f", "Verify that the DUT has emitted the SecureStateChanged event."),
            TestStep("9g", "If LatchControlModes Bit 1 = 0 (RemoteUnlatching = False), skip step 9h."),
            TestStep("9h", "TH sends command MoveTo with Latch = False."),
            TestStep("9i", "If LatchControlModes Bit 1 = 1 (RemoteUnlatching = True), skip step 9j."),
            TestStep("9j", "Unlatch the DUT manually to set OverallCurrentState.Latch to False."),
            TestStep("9k", "TH sends command MoveTo with Position = MoveToFullyOpen."),
            TestStep("9l", "Verify that the DUT has emitted the SecureStateChanged event."),
        ]
        return steps

    def wait_for_secure_state_changed_and_assert(self, event_sub_handler, expected_secure_value: bool, timeout: int):
        """
        Waits for a SecureStateChanged event, allowing for one other event to arrive first.
        Asserts that the SecureStateChanged event is received and its secureValue matches the expected value.
        """
        event_queue = event_sub_handler.event_queue

        try:
            event1 = event_queue.get(block=True, timeout=timeout)
        except event_queue.Empty:
            asserts.fail("Timeout waiting for event.")

        secure_event = None
        if event1.Header.EventId == Clusters.ClosureControl.Events.SecureStateChanged.event_id:
            secure_event = event1
        else:
            logging.info(f"Received other event first: {event1.Header.EventId}, waiting for SecureStateChanged.")
            try:
                event2 = event_queue.get(block=True, timeout=timeout)
            except event_queue.Empty:
                asserts.fail("Timeout waiting for SecureStateChanged event after receiving another event first.")

            asserts.assert_equal(event2.Header.EventId, Clusters.ClosureControl.Events.SecureStateChanged.event_id,
                                 "Expected SecureStateChanged event not received as the second event.")
            secure_event = event2

        logging.info(f"SecureStateChanged event received: {secure_event}")
        if expected_secure_value:
            asserts.assert_true(secure_event.Data.secureValue, "SecureStateChanged event secureValue is not True.")
        else:
            asserts.assert_false(secure_event.Data.secureValue, "SecureStateChanged event secureValue is not False.")

    def pics_TC_CLCTRL_6_1(self) -> list[str]:
        pics = [
            "CLCTRL.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLCTRL_6_1(self):
        endpoint = self.get_endpoint(default=1)
        dev_controller = self.default_controller
        attributes = Clusters.ClosureControl.Attributes
        timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)

        attributes = Clusters.ClosureControl.Attributes

        # STEP 2a: TH reads from the DUT the (0xFFFC) FeatureMap attribute
        self.step("2a")

        # Read the FeatureMap attribute
        feature_map = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        logging.info(f"FeatureMap: {feature_map}")
        is_ps_feature_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kPositioning
        is_mo_feature_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kManuallyOperable
        is_is_feature_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kInstantaneous
        is_lt_feature_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kMotionLatching

        # STEP 2b: TH reads TestEventTriggerEnabled attribute from General Diagnostics Cluster
        self.step("2b")

        logging.info("Checking if TestEventTrigger attribute is enabled")
        self.check_test_event_triggers_enabled()

        # STEP 2c: TH reads from the DUT the (0xFFFB) AttributeList attribute
        self.step("2c")

        attribute_list = await self.read_clctrl_attribute_expect_success(endpoint, attributes.AttributeList)
        logging.info(f"AttributeList: {attribute_list}")

        # STEP 2d: TH establishes a wildcard subscription to all attributes and events on the Closure Control Cluster, with MinIntervalFloor = 0, MaxIntervalCeiling = 30 and KeepSubscriptions = false
        self.step("2d")

        sub_handler = AttributeSubscriptionHandler(expected_cluster=Clusters.ClosureControl)
        await sub_handler.start(dev_controller, self.dut_node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        event_sub_handler = EventSubscriptionHandler(expected_cluster=Clusters.ClosureControl)
        await event_sub_handler.start(self.default_controller, self.dut_node_id, endpoint=endpoint)

        # STEP 3a: If the LT feature is not supported on the cluster, skip steps 3b to 3i.
        self.step("3a")

        if not is_lt_feature_supported:
            logging.info("Motion Latching feature not supported, skipping steps 3b to 3i")

            # Skipping steps 3b to 3i
            self.mark_step_range_skipped("3b", "3i")
        else:
            logging.info("Motion Latching feature supported.")

            # STEP 3b: If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState.Latch attribute
            self.step("3b")

            if attributes.OverallCurrentState.attribute_id in attribute_list:
                overall_current_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
                logging.info(f"OverallCurrentState: {overall_current_state}")
                if overall_current_state is NullValue:
                    asserts.assert_true(False, "OverallCurrentState is NullValue.")

                CurrentLatch = overall_current_state.latch
                asserts.assert_in(CurrentLatch, [True, False], "OverallCurrentState.latch is not in the expected range")
            else:
                asserts.assert_true(False, "OverallCurrentState attribute is not supported.")

            # STEP 3c: If the attribute is supported on the cluster, TH reads from the DUT the LatchControlModes attribute
            self.step("3c")

            if attributes.LatchControlModes.attribute_id in attribute_list:
                LatchControlModes = await self.read_clctrl_attribute_expect_success(endpoint, attributes.LatchControlModes)
                logging.info(f"LatchControlModes: {LatchControlModes}")
                if LatchControlModes is NullValue:
                    asserts.assert_true(False, "LatchControlModes is NullValue.")
            else:
                asserts.assert_true(False, "LatchControlModes attribute is not supported.")

            # STEP 3d: If CurrentLatch = False, skip steps 3e to 3i
            self.step("3d")

            if not CurrentLatch:
                logging.info("CurrentLatch is False, skipping steps 3e to 3i")

                # Skipping steps 3e to 3i
                self.mark_step_range_skipped("3e", "3i")
            else:
                logging.info("CurrentLatch is True, proceeding to steps 3e to 3i")

                # STEP 3e: If LatchControlModes Bit 1 = 0 (RemoteUnlatching = False), skip step 3f
                self.step("3e")
                sub_handler.reset()

                if not LatchControlModes & Clusters.ClosureControl.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                    logging.info("LatchControlModes Bit 1 is 0 (RemoteUnlatching = False), skipping step 3f")
                    self.skip_step("3f")

                    # STEP 3g: If LatchControlModes Bit 1 = 1 (RemoteUnlatching = True), skip step 3h
                    self.step("3g")

                    logging.info("RemoteUnlatching is False, proceeding to step 3h")

                    # STEP 3h: Unlatch the DUT manually to set OverallCurrentState.Latch to False
                    self.step("3h")

                    logging.info("Unlatch the DUT manually to set OverallCurrentState.Latch to False")
                    # Simulating manual unlatching by waiting for user input
                    self.wait_for_user_input(prompt_msg="Press Enter after unlatching the DUT..")
                    logging.info("Manual unlatching completed.")
                else:
                    logging.info("LatchControlModes Bit 1 is 1 (RemoteUnlatching = True), proceeding to step 3f")

                    # STEP 3f: TH sends command MoveTo with Latch = False
                    self.step("3f")

                    try:
                        await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                            latch=False
                        ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                    except InteractionModelError as e:
                        asserts.assert_equal(
                            e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                        pass
                    self.step("3g")
                    self.skip_step("3h")

                # STEP 3i: Wait until TH receives a subscription report with OverallCurrentState.Latch.
                self.step("3i")

                logging.info("Waiting for OverallCurrentState.Latch to be False")
                sub_handler.await_all_expected_report_matches(expected_matchers=[current_latch_matcher(False)],
                                                              timeout_sec=timeout)

        # STEP 3j: If the PS feature is not supported on the cluster, skip steps 3k to 3n.
        self.step("3j")

        if not is_ps_feature_supported:
            logging.info("Positioning feature not supported, skipping steps 3k to 3n")

            # Skipping steps 3k to 3n
            self.mark_step_range_skipped("3k", "3n")
        else:
            logging.info("Positioning feature supported.")

            # STEP 3k: If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute
            self.step("3k")

            if attributes.OverallCurrentState.attribute_id in attribute_list:
                overall_current_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
                logging.info(f"OverallCurrentState: {overall_current_state}")
                if overall_current_state is NullValue:
                    asserts.assert_true(False, "OverallCurrentState is NullValue.")

                CurrentPosition = overall_current_state.position
                asserts.assert_in(CurrentPosition, Clusters.ClosureControl.Enums.CurrentPositionEnum,
                                  "OverallCurrentState.position is not in the expected range")
            else:
                asserts.assert_true(False, "OverallCurrentState attribute is not supported.")

            # STEP 3l: If CurrentPosition = FullyClosed, skip steps 3m to 3n
            self.step("3l")

            if CurrentPosition == Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed:
                logging.info("CurrentPosition is FullyClosed, skipping steps 3m to 3n")

                # Skipping steps 3m and 3n
                self.skip_step("3m")
                self.skip_step("3n")
            else:
                # STEP 3m: TH sends command MoveTo with Position = MoveToFullyClosed
                self.step("3m")

                sub_handler.reset()
                try:
                    await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                        position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed,
                    ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                    pass

                # STEP 3n: Wait until TH receives a subscription report with OverallCurrentState.Position.
                self.step("3n")

                logging.info("Waiting for OverallCurrentState.Position to be FullyClosed")
                sub_handler.await_all_expected_report_matches(expected_matchers=[current_position_matcher(Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed)],
                                                              timeout_sec=timeout)

        # STEP 4a: TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Error(3) Test Event
        self.step("4a")

        event_sub_handler.reset()

        try:
            await self.send_test_event_triggers(eventTrigger=triggerError)  # Error
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.Success, f"Failed to send command TestEventTrigger: {e.status}")
            pass

        # STEP 4b: Verify that the DUT has emitted the OperationalError event
        self.step("4b")

        # Wait for the OperationalError event to be emitted
        data = event_sub_handler.wait_for_event_report(Clusters.ClosureControl.Events.OperationalError, timeout_sec=timeout)
        logging.info(f"-> OperationalError event last received: {data.errorState}")

        asserts.assert_not_equal(data.errorState, [], "The CurrentErrorList attribute is empty.")

        # STEP 4c: TH reads from the DUT the CurrentErrorList attribute
        self.step("4c")

        # Read the CurrentErrorList attribute
        current_error_list = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, attribute=Clusters.ClosureControl.Attributes.CurrentErrorList)
        asserts.assert_not_equal(current_error_list, [], "The CurrentErrorList attribute is empty.")

        # STEP 4d: TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear
        self.step("4d")

        try:
            await self.send_test_event_triggers(eventTrigger=triggerClear)  # Test Event Clear
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.Success, f"Failed to send command TestEventTrigger: {e.status}")
            pass

        # STEP 5a: If PS feature is not supported on the cluster or IS feature is supported on the cluster, skip steps 5b to 5f
        self.step("5a")

        if not is_ps_feature_supported or is_is_feature_supported:
            logging.info("Positioning feature supported or Instantaneous feature not supported, skipping steps 5b to 5f")

            # Skipping steps 5b to 5f
            self.mark_step_range_skipped("5b", "5f")
        else:
            logging.info("Positioning feature not supported and Instantaneous feature supported processing steps 5b to 5f.")

            # STEP 5b: TH sends command MoveTo with Position = MoveToFullyOpen
            self.step("5b")

            event_sub_handler.reset()
            sub_handler.reset()

            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen,
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                pass

            # STEP 5c: Wait until TH receives a subscription report with OverallCurrentState.Position = FullyOpened and the MovementCompleted event.
            self.step("5c")

            logging.info("Waiting for OverallCurrentState.Position to be FullyOpened and the corresponding MovementCompleted event.")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_position_matcher(Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyOpened)],
                                                          timeout_sec=timeout)
            event_sub_handler.wait_for_event_report(Clusters.ClosureControl.Events.MovementCompleted, timeout_sec=timeout)

            # STEP 5d: TH sends command MoveTo with Position = MoveToFullyClosed
            self.step("5d")

            event_sub_handler.reset()
            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed,
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                pass

            # STEP 5e: Wait until TH receives a subscription report with OverallCurrentState.Position = FullyClosed and the MovementCompleted event.
            self.step("5e")

            logging.info("Waiting for OverallCurrentState.Position to be FullyClosed and the corresponding MovementCompleted event.")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_position_matcher(Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed)],
                                                          timeout_sec=timeout)
            event_sub_handler.wait_for_event_report(Clusters.ClosureControl.Events.MovementCompleted, timeout_sec=timeout)

            # STEP 5f: TH reads from the DUT the MainState attribute
            self.step("5f")

            if attributes.MainState.attribute_id in attribute_list:
                main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
                logging.info(f"MainState: {main_state}")
                asserts.assert_equal(main_state, Clusters.ClosureControl.Enums.MainStateEnum.kStopped,
                                     "MainState is not in the expected state")
            else:
                asserts.assert_true(False, "MainState attribute is not supported.")

        # STEP 6a: If MO feature is not supported on the cluster, skip steps 6b to 6e
        self.step("6a")

        if not is_mo_feature_supported:
            logging.info("Manual Operable feature not supported, skipping steps 6b to 6e")

            # Skipping steps 6b to 6e
            self.mark_step_range_skipped("6b", "6e")
        else:
            logging.info("Manual Operable feature supported processing steps 6b to 6e.")

            # STEP 6b: TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Disengaged(6) Test Event
            self.step("6b")

            event_sub_handler.reset()
            try:
                await self.send_test_event_triggers(eventTrigger=triggerDisengaged)  # Disengaged
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command TestEventTrigger: {e.status}")
                pass

            # STEP 6c: Verify that the DUT has emitted the EngageStateChanged event
            self.step("6c")

            # Wait for the EngageStateChanged event to be emitted
            data = event_sub_handler.wait_for_event_report(Clusters.ClosureControl.Events.EngageStateChanged, timeout_sec=timeout)

            asserts.assert_false(data.engageValue, f"Unexpected event data: {data.engageValue}")

            # STEP 6d: TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear
            self.step("6d")

            event_sub_handler.reset()
            try:
                await self.send_test_event_triggers(eventTrigger=triggerClear)  # Test Event Clear
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command TestEventTrigger: {e.status}")
                pass

            # STEP 6e: Verify that the DUT has emitted the EngageStateChanged event
            self.step("6e")

            # Wait for the EngageStateChanged event to be emitted
            data = event_sub_handler.wait_for_event_report(Clusters.ClosureControl.Events.EngageStateChanged, timeout_sec=timeout)

            asserts.assert_true(data.engageValue, f"Unexpected event data: {data.engageValue}")

        # STEP 7a: If LT feature is supported on the cluster or PS feature is not supported on the cluster, skip steps 7b to 7f
        self.step("7a")

        if is_lt_feature_supported or not is_ps_feature_supported:
            logging.info("Positioning feature not supported or Motion Latching feature supported, skipping steps 7b to 7f")

            # Skipping steps 7b to 7f
            self.mark_step_range_skipped("7b", "7f")
        else:
            logging.info("Positioning feature supported and Motion Latching feature not supported processing steps 7b to 7f.")

            # STEP 7b: If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute
            self.step("7b")

            if attributes.OverallCurrentState.attribute_id in attribute_list:
                overall_current_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
                logging.info(f"OverallCurrentState: {overall_current_state}")
                if overall_current_state is NullValue:
                    asserts.assert_true(False, "OverallCurrentState is NullValue.")

                asserts.assert_in(overall_current_state.position, Clusters.ClosureControl.Enums.CurrentPositionEnum,
                                  "OverallCurrentState.position is not in the expected range")
                asserts.assert_equal(overall_current_state.position,
                                     Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed, "The DUT is in an incorrect position.")
                asserts.assert_true(overall_current_state.secureState, "The DUT is in an incorrect secure state.")
            else:
                asserts.assert_true(False, "OverallCurrentState attribute is not supported.")

            # STEP 7c: TH sends command MoveTo with Position = MoveToFullyOpen.
            self.step("7c")

            event_sub_handler.reset()
            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen,
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                pass

            # STEP 7d: Verify that the DUT has emitted the SecureStateChanged event
            self.step("7d")

            # Wait for the SecureStateChanged event to be emitted
            self.wait_for_secure_state_changed_and_assert(event_sub_handler, expected_secure_value=False, timeout=timeout)

            # STEP 7e: TH sends command MoveTo with Position = MoveToFullyClosed.
            self.step("7e")

            event_sub_handler.reset()
            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed,
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                pass

            # STEP 7f: Verify that the DUT has emitted the SecureStateChanged event
            self.step("7f")

            # Wait for the SecureStateChanged event to be emitted
            self.wait_for_secure_state_changed_and_assert(event_sub_handler, expected_secure_value=True, timeout=timeout)

        # STEP 8a: If LT feature is not supported on the cluster or PS feature is supported on the cluster, skip steps 8b to 8l
        self.step("8a")

        if not is_lt_feature_supported or is_ps_feature_supported:
            logging.info("Positioning feature supported or Motion Latching feature not supported, skipping steps 8b to 8l")

            # Skipping steps 8b to 8l
            self.mark_step_range_skipped("8b", "8l")
        else:
            logging.info("Positioning feature not supported and Motion Latching feature supported processing steps 8b to 8l.")
            sub_handler.reset()
            event_sub_handler.reset()

            # STEP 8b: If LatchControlModes Bit 0 = 0 (RemoteLatching = False), skip step 8c
            self.step("8b")

            if not LatchControlModes & Clusters.ClosureControl.Bitmaps.LatchControlModesBitmap.kRemoteLatching:
                logging.info("LatchControlModes Bit 0 is 0 (RemoteLatching = False), skipping step 8c")
                self.skip_step("8c")

                # STEP 8d: If LatchControlModes Bit 0 = 1 (RemoteLatching = True), skip step 8e
                self.step("8d")

                logging.info("RemoteLatching is False, proceeding to step 8e")

                # STEP 8e: Latch the DUT manually to set OverallCurrentState.Latch to True
                self.step("8e")

                logging.info("Latch the DUT manually to set OverallCurrentState.Latch to True")
                # Simulating manual latching by waiting for user input
                self.wait_for_user_input(prompt_msg="Press Enter after latching the DUT...")
                logging.info("Manual latching completed.")
            else:
                logging.info("LatchControlModes Bit 0 is 1 (RemoteLatching = True), proceeding to step 8c")

                # STEP 8c: TH sends command MoveTo with Latch = True
                self.step("8c")

                try:
                    await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                        latch=True
                    ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                    pass
                self.step("8d")
                self.skip_step("8e")

            # STEP 8f: Wait until TH receives a subscription report with OverallCurrentState.Latch = True
            self.step("8f")

            logging.info("Waiting for OverallCurrentState.Latch to be True")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_latch_matcher(True)],
                                                          timeout_sec=timeout)

            # STEP 8g: Verify that the DUT has emitted the SecureStateChanged event
            self.step("8g")

            # Wait for the SecureStateChanged event to be emitted
            self.wait_for_secure_state_changed_and_assert(event_sub_handler, expected_secure_value=True, timeout=timeout)

            # STEP 8h: If LatchControlModes Bit 1 = 0 (RemoteUnlatching = False), skip step 8i
            self.step("8h")

            event_sub_handler.reset()

            if not LatchControlModes & Clusters.ClosureControl.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                logging.info("LatchControlModes Bit 1 is 0 (RemoteUnlatching = False), skipping step 8i")
                self.skip_step("8i")

                # STEP 8j: If LatchControlModes Bit 1 = 1 (RemoteUnlatching = True), skip step 8k
                self.step("8j")

                logging.info("RemoteUnlatching is False, proceeding to step 8k")

                # STEP 8k: Unlatch the DUT manually to set OverallCurrentState.Latch to False
                self.step("8k")

                logging.info("Unlatch the DUT manually to set OverallCurrentState.Latch to False")
                # Simulating manual unlatching by waiting for user input
                self.wait_for_user_input(prompt_msg="Press Enter after unlatching the DUT...")
                logging.info("Manual unlatching completed.")
            else:
                logging.info("LatchControlModes Bit 1 is 1 (RemoteUnlatching = True), proceeding to step 8i")

                # STEP 8i: TH sends command MoveTo with Latch = False
                self.step("8i")

                try:
                    await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                        latch=False
                    ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                    pass
                self.step("8j")
                self.skip_step("8k")

            # STEP 8l: Verify that the DUT has emitted the SecureStateChanged event
            self.step("8l")

            # Wait for the SecureStateChanged event to be emitted
            self.wait_for_secure_state_changed_and_assert(event_sub_handler, expected_secure_value=False, timeout=timeout)

        # STEP 9a: If LT feature is not supported on the cluster or PS feature is not supported on the cluster, skip steps 9b to 9l
        self.step("9a")

        if not is_lt_feature_supported or not is_ps_feature_supported:
            logging.info("Positioning feature not supported or Motion Latching feature not supported, skipping steps 9b to 9l")

            # Skipping steps 9b to 9l
            self.mark_step_range_skipped("9b", "9l")
        else:
            logging.info("Positioning feature supported and Motion Latching feature supported processing steps 9b to 9l.")
            event_sub_handler.reset()

            # STEP 9b: If LatchControlModes Bit 0 = 0 (RemoteLatching = False), skip step 9c
            self.step("9b")

            if not LatchControlModes & Clusters.ClosureControl.Bitmaps.LatchControlModesBitmap.kRemoteLatching:
                logging.info("LatchControlModes Bit 0 is 0 (RemoteLatching = False), skipping step 9c")
                self.skip_step("9c")
                # STEP 9d: If LatchControlModes Bit 0 = 1 (RemoteLatching = True), skip step 9e
                self.step("9d")

                logging.info("RemoteLatching is False, proceeding to step 9e")

                # STEP 9e: Latch the DUT manually to set OverallCurrentState.Latch to True
                self.step("9e")

                logging.info("Latch the DUT manually to set OverallCurrentState.Latch to True")
                # Simulating manual latching by waiting for user input
                self.wait_for_user_input(prompt_msg="Press Enter after latching the DUT...")
                logging.info("Manual latching completed.")
            else:
                logging.info("LatchControlModes Bit 0 is 1 (RemoteLatching = True), proceeding to step 9c")

                # STEP 9c: TH sends command MoveTo with Latch = True
                self.step("9c")

                try:
                    await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                        latch=True
                    ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                    pass
                self.step("9d")
                self.skip_step("9e")

            # STEP 9f: Verify that the DUT has emitted the SecureStateChanged event
            self.step("9f")

            # Wait for the SecureStateChanged event to be emitted
            self.wait_for_secure_state_changed_and_assert(event_sub_handler, expected_secure_value=True, timeout=timeout)

            # STEP 9g: If LatchControlModes Bit 1 = 0 (RemoteUnlatching = False), skip step 9h
            self.step("9g")

            event_sub_handler.reset()

            if not LatchControlModes & Clusters.ClosureControl.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                logging.info("LatchControlModes Bit 1 is 0 (RemoteUnlatching = False), skipping step 9h")
                self.skip_step("9h")

                # STEP 9i: If LatchControlModes Bit 1 = 1 (RemoteUnlatching = True), skip step 9j
                self.step("9i")

                logging.info("RemoteUnlatching is False, proceeding to step 9j")

                # STEP 9j: Unlatch the DUT manually to set OverallCurrentState.Latch to False
                self.step("9j")

                logging.info("Unlatch the DUT manually to set OverallCurrentState.Latch to False")
                # Simulating manual unlatching by waiting for user input
                self.wait_for_user_input(prompt_msg="Press Enter after unlatching the DUT...")
                logging.info("Manual unlatching completed.")
            else:
                logging.info("LatchControlModes Bit 1 is 1 (RemoteUnlatching = True), proceeding to step 9h")

                # STEP 9h: TH sends command MoveTo with Latch = False
                self.step("9h")

                try:
                    await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                        latch=False
                    ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                    pass
                self.step("9i")
                self.skip_step("9j")

            # STEP 9k: TH sends command MoveTo with Position = MoveToFullyOpen.
            self.step("9k")

            time.sleep(2)  # Adding a small delay to ensure the previous command is processed
            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen,
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                pass

            # STEP 9l: Verify that the DUT has emitted the SecureStateChanged event
            self.step("9l")

            # Wait for the SecureStateChanged event to be emitted
            self.wait_for_secure_state_changed_and_assert(event_sub_handler, expected_secure_value=False, timeout=timeout)


if __name__ == "__main__":
    default_matter_test_main()
