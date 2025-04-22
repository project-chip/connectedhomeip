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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
from random import choice

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts


class TC_CLCTRL_6_1(MatterBaseTest):
    async def read_clctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_6_1(self) -> str:
        return "[TC_CLCTRL_6_1] Event Functionality with DUT as Server"

    def setup_test(self):
        super().setup_test()
        self.is_ci = self._use_button_simulator()

    def _ask_for_manual_latching(self, endpoint_id: int, latched: bool):
        if not self._use_button_simulator():
            self.wait_for_user_input(prompt_msg=f"Latch the DUT manually to set OverallState.Latch to True.")
        else:
            self._send_latching_named_pipe_command(endpoint_id, latched)

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

    def _send_latching_named_pipe_command(self, endpoint_id: int, latched: bool):
        command_dict = {"Name": "SimulateLatching", "EndpointId": endpoint_id, "latched": latched}
        self._send_named_pipe_command(command_dict)

    def steps_TC_CLCTRL_6_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).", is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggerEnabled attribute from General Diagnostics Cluster."),
            TestStep("3a", "Set up a subscription to the OperationalError event."),
            TestStep("3b", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Error(3) Test Event."),
            TestStep("3c", "Verify that the DUT has emitted the OperationalError event."),
            TestStep("3d", "TH reads from the DUT the CurrentErrorList attribute."),
            TestStep("3e", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear."),
            TestStep("4a", "If PS feature is not supported on the cluster or IS feature is supported on the cluster, skip steps 4b to 4e."),
            TestStep("4b", "Set up a subscription to the MovementCompleted event."),
            TestStep("4c", "TH sends command MoveTo with Position = CloseInFull."),
            TestStep("4d", "Verify that the DUT has emitted the MovementCompleted event."),
            TestStep("4e", "TH reads from the DUT the Mainstate attribute."),
            TestStep("5a", "If MO feature is not supported on the cluster, skip steps 5b to 5f."),
            TestStep("5b", "Set up a subscription to the EngagedStateChanged event."),
            TestStep("5c", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Disengaged(6) Test Event."),
            TestStep("5d", "Verify that the DUT has emitted the EngagedStateChanged event."),
            TestStep("5e", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear."),
            TestStep("5f", "Verify that the DUT has emitted the EngagedStateChanged event."),
            TestStep("6a", "Set up a subscription to the SecureStateChanged event."),
            TestStep("6b", "If LT feature is supported on the cluster or PS feature is not supported on the cluster, skip steps 6c to 6f."),
            TestStep("6c", "TH sends command MoveTo with Position = OpenInFull."),
            TestStep("6d", "Verify that the DUT has emitted the SecureStateChanged event."),
            TestStep("6e", "TH sends command MoveTo with Position = CloseInFull."),
            TestStep("6f", "Verify that the DUT has emitted the SecureStateChanged event."),
            TestStep("6g", "If LT feature is not supported on the cluster or PS feature is supported on the cluster, skip steps 6h to 6o."),
            TestStep("6h", "TH sends command MoveTo to DUT with Latch = False."),
            TestStep("6i", "Verify that the DUT has emitted the SecureStateChanged event."),
            TestStep("6j", "If !CLCTRL.M.ManualLatching, skip steps 6k to 6l."),
            TestStep("6k", "Latch the DUT manually to set OverallState.Latch to True."),
            TestStep("6l", "Verify that the DUT has emitted the SecureStateChanged event."),
            TestStep("6m", "If CLCTRL.M.ManualLatching, skip steps 6n to 6o."),
            TestStep("6n", "TH sends command MoveTo to DUT with Latch = True."),
            TestStep("6o", "Verify that the DUT has emitted the SecureStateChanged event."),
            TestStep("6p", "If LT feature is not supported on the cluster or PS feature is not supported on the cluster, skip steps 6q to 6y."),
            TestStep("6q", "TH sends command MoveTo with Position = OpenInFull and Latch = False."),
            TestStep("6r", "Verify that the DUT has emitted the SecureStateChanged event."),
            TestStep("6s", "If !CLCTRL.M.ManualLatching, skip steps 6t to 6v."),
            TestStep("6t", "TH sends command MoveTo with Position = CloseInFull."),
            TestStep("6u", "Latch the DUT manually to set OverallState.Latch to True."),
            TestStep("6v", "Verify that the DUT has emitted the SecureStateChanged event."),
            TestStep("6w", "If CLCTRL.M.ManualLatching, skip steps 6y to 6z."),
            TestStep("6x", "TH sends command MoveTo with Position = CloseInFull and Latch = True."),
            TestStep("6y", "Verify that the DUT has emitted the SecureStateChanged event."),
        ]
        return steps

    def pics_TC_CLCTRL_6_1(self) -> list[str]:
        pics = [
            "CLCTRL.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLCTRL_6_1(self):
        endpoint = self.get_endpoint(default=1)
        is_skipped = False
        is_manual_latching_skipped = False

        feature_map = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        logging.info(f"FeatureMap: {feature_map}")
        is_ps_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kPosition
        is_mo_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kMotion
        is_is_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kIs
        is_lt_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kLatch

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)
        attributes = Clusters.ClosureControl.Attributes

        # STEP 2: TH reads TestEventTriggerEnabled attribute from General Diagnostics Cluster
        self.step(2)

        try:
            await self.read_single_attribute_check_success(endpoint=0, cluster=Clusters.Objects.GeneralDiagnostic, attribute=Clusters.GeneralDiagnostic.Attributes.TestEventTriggersEnabled)
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the attribute was read successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to read TestEventTriggersEnabled attribute: {e.status}")
            return
        else:
            logging.info("TestEventTriggersEnabled attribute read successfully")

        # Check if the attribute is enabled
        test_event_triggers_enabled = await self.read_single_attribute_check_success(endpoint=0, cluster=Clusters.Objects.GeneralDiagnostic, attribute=Clusters.GeneralDiagnostic.Attributes.TestEventTriggersEnabled)
        if test_event_triggers_enabled is NullValue:
            logging.error("Failed to read TestEventTriggersEnabled attribute")
            return
        else:
            asserts.assert_true(test_event_triggers_enabled ==
                                Clusters.GeneralDiagnostic.Bitmaps.TestEventTriggersEnabled.kEnabled, "TestEventTriggersEnabled is not enabled")
            if test_event_triggers_enabled == Clusters.GeneralDiagnostic.Bitmaps.TestEventTriggersEnabled.kEnabled:
                logging.info("TestEventTriggersEnabled is enabled")
        # Check if the attribute was read successfully
        if not type_matches(test_event_triggers_enabled.status, Status.SUCCESS):
            logging.error(f"Failed to read TestEventTriggersEnabled attribute: {test_event_triggers_enabled.status}")
            return

        # STEP 3a: Set up a subscription to the OperationalError event
        self.step("3a")

        # Subscribe to the OperationalError event
        self.subscribe_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl,
                             event=Clusters.ClosureControl.Events.OperationalError)
        # Check if the subscription was successful
        if not type_matches(self.status, Status.SUCCESS):
            logging.error(f"Failed to subscribe to OperationalError event: {self.status}")
            return
        else:
            logging.info("Subscribed to OperationalError event successfully")

        # STEP 3b: TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Error(3) Test Event
        self.step("3b")

        try:
            await self.send_command(endpoint=0, cluster=Clusters.Objects.GeneralDiagnostic, command=Clusters.GeneralDiagnostic.Commands.TestEventTrigger, arguments=Clusters.GeneralDiagnostic.TestEventTriggerRequest(
                enable_key=Clusters.GeneralDiagnostic.Bitmaps.EnableKey.kTestEventTriggerKey,
                event_trigger=Clusters.GeneralDiagnostic.Bitmaps.EventTrigger.kMainStateError,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command TestEventTrigger: {e.status}")
            return
        else:
            logging.info("Command TestEventTrigger sent successfully")

        # STEP 3c: Verify that the DUT has emitted the OperationalError event
        self.step("3c")

        # Wait for the OperationalError event to be emitted
        event = await self.wait_for_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, event=Clusters.ClosureControl.Events.OperationalError)
        # Check if the event was received successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to receive OperationalError event: {event.status}")
            return
        else:
            logging.info("OperationalError event received successfully")
        # Check if the event data is correct
        if event.data.count == 0:
            logging.error(f"Unexpected event data count: {event.data.count}")
            return
        else:
            logging.info("Event data is correct")
        # Check if the event was emitted successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to emit OperationalError event: {event.status}")
            return
        else:
            logging.info("OperationalError event emitted successfully")

        # STEP 3d: TH reads from the DUT the CurrentErrorList attribute
        self.step("3d")

        # Read the CurrentErrorList attribute
        current_error_list = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, attribute=Clusters.ClosureControl.Attributes.CurrentErrorList)
        # Check if the attribute was read successfully
        if not type_matches(current_error_list.status, Status.SUCCESS):
            logging.error(f"Failed to read CurrentErrorList attribute: {current_error_list.status}")
            return
        else:
            logging.info("CurrentErrorList attribute read successfully")
        # Check if the attribute data is correct
        if current_error_list.count == 0:
            logging.error(f"Unexpected attribute count: {current_error_list.count}")
            return
        else:
            logging.info("Attribute data is correct")

        # STEP 3e: TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear
        self.step("3e")

        try:
            await self.send_command(endpoint=0, cluster=Clusters.Objects.GeneralDiagnostic, command=Clusters.GeneralDiagnostic.Commands.TestEventTrigger, arguments=Clusters.GeneralDiagnostic.TestEventTriggerRequest(
                enable_key=Clusters.GeneralDiagnostic.Bitmaps.EnableKey.kTestEventTriggerKey,
                event_trigger=Clusters.GeneralDiagnostic.Bitmaps.EventTrigger.kMainStateClear,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command TestEventTrigger: {e.status}")
            return
        else:
            logging.info("Command TestEventTrigger sent successfully")

        # STEP 4a: If PS feature is not supported on the cluster or IS feature is supported on the cluster, skip steps 4b to 4e
        self.step("4a")

        if not is_ps_feature_supported or is_is_feature_supported:
            logging.info("Skipping steps 4b to 4e")
            is_skipped = True

        # STEP 4b: Set up a subscription to the MovementCompleted event
        self.step("4b")

        if is_skipped == True:
            logging.info("Test step skipped")
            return

        # Subscribe to the MovementCompleted event
        self.subscribe_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl,
                             event=Clusters.ClosureControl.Events.MovementCompleted)
        # Check if the subscription was successful
        if not type_matches(self.status, Status.SUCCESS):
            logging.error(f"Failed to subscribe to MovementCompleted event: {self.status}")
            return
        else:
            logging.info("Subscribed to MovementCompleted event successfully")

        # STEP 4c: TH sends command MoveTo with Position = CloseInFull
        self.step("4c")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kCloseInFull,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 4d: Verify that the DUT has emitted the MovementCompleted event
        self.step("4d")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        # Wait for the MovementCompleted event to be emitted
        event = await self.wait_for_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, event=Clusters.ClosureControl.Events.MovementCompleted)
        # Check if the event was received successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to receive MovementCompleted event: {event.status}")
            return
        else:
            logging.info("MovementCompleted event received successfully")

        # STEP 4e: TH reads from the DUT the Mainstate attribute
        self.step("4e")

        if is_skipped == True:
            logging.info("Test step skipped")
            is_skipped = False
            return
        # Read the Mainstate attribute
        mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
        if mainstate is NullValue:
            logging.error("Failed to read MainState attribute")
            return
        else:
            # Check if the MainState attribute has the expected values
            is_stopped = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kStopped
            asserts.assert_true(is_stopped, "MainState is not in the expected state")
            if is_stopped:
                logging.info("MainState is in the stopped state")
        # Check if the MainState attribute was read successfully
        if not type_matches(mainstate.status, Status.SUCCESS):
            logging.error(f"Failed to read MainState attribute: {mainstate.status}")
            return

        is_skipped = False

        # STEP 5a: If MO feature is not supported on the cluster, skip steps 5b to 5f
        self.step("5a")

        if not is_mo_feature_supported:
            logging.info("Skipping steps 5b to 5f")
            is_skipped = True

        # STEP 5b: Set up a subscription to the EngagedStateChanged event
        self.step("5b")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        # Subscribe to the EngagedStateChanged event
        self.subscribe_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl,
                             event=Clusters.ClosureControl.Events.EngagedStateChanged)
        # Check if the subscription was successful
        if not type_matches(self.status, Status.SUCCESS):
            logging.error(f"Failed to subscribe to EngagedStateChanged event: {self.status}")
            return
        else:
            logging.info("Subscribed to EngagedStateChanged event successfully")

        # STEP 5c: TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Disengaged(6) Test Event
        self.step("5c")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        try:
            await self.send_command(endpoint=0, cluster=Clusters.Objects.GeneralDiagnostic, command=Clusters.GeneralDiagnostic.Commands.TestEventTrigger, arguments=Clusters.GeneralDiagnostic.TestEventTriggerRequest(
                enable_key=Clusters.GeneralDiagnostic.Bitmaps.EnableKey.kTestEventTriggerKey,
                event_trigger=Clusters.GeneralDiagnostic.Bitmaps.EventTrigger.kMainStateDisengaged,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command TestEventTrigger: {e.status}")
            return
        else:
            logging.info("Command TestEventTrigger sent successfully")

        # STEP 5d: Verify that the DUT has emitted the EngagedStateChanged event
        self.step("5d")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        # Wait for the EngagedStateChanged event to be emitted
        event = await self.wait_for_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, event=Clusters.ClosureControl.Events.EngagedStateChanged)
        # Check if the event was received successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to receive EngagedStateChanged event: {event.status}")
            return
        else:
            logging.info("EngagedStateChanged event received successfully")
        # Check if the event data is correct
        if event.data == True:
            logging.error(f"Unexpected event data: {event.data}")
            return
        else:
            logging.info("Event data is correct")
        # Check if the event was emitted successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to emit EngagedStateChanged event: {event.status}")
            return
        else:
            logging.info("EngagedStateChanged event emitted successfully")

        # STEP 5e: TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_ENABLE_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear
        self.step("5e")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        try:
            await self.send_command(endpoint=0, cluster=Clusters.Objects.GeneralDiagnostic, command=Clusters.GeneralDiagnostic.Commands.TestEventTrigger, arguments=Clusters.GeneralDiagnostic.TestEventTriggerRequest(
                enable_key=Clusters.GeneralDiagnostic.Bitmaps.EnableKey.kTestEventTriggerKey,
                event_trigger=Clusters.GeneralDiagnostic.Bitmaps.EventTrigger.kMainStateClear,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command TestEventTrigger: {e.status}")
            return
        else:
            logging.info("Command TestEventTrigger sent successfully")

        # STEP 5f: Verify that the DUT has emitted the EngagedStateChanged event
        self.step("5f")

        if is_skipped == True:
            logging.info("Test step skipped")
            is_skipped = False
            return
        # Wait for the EngagedStateChanged event to be emitted
        event = await self.wait_for_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, event=Clusters.ClosureControl.Events.EngagedStateChanged)
        # Check if the event was received successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to receive EngagedStateChanged event: {event.status}")
            return
        else:
            logging.info("EngagedStateChanged event received successfully")
        # Check if the event data is correct
        if event.data == False:
            logging.error(f"Unexpected event data: {event.data}")
            return
        else:
            logging.info("Event data is correct")
        # Check if the event was emitted successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to emit EngagedStateChanged event: {event.status}")
            return
        else:
            logging.info("EngagedStateChanged event emitted successfully")

        is_skipped = False

        # STEP 6a: Set up a subscription to the SecureStateChanged event
        self.step("6a")

        # Subscribe to the SecureStateChanged event
        self.subscribe_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl,
                             event=Clusters.ClosureControl.Events.SecureStateChanged)
        # Check if the subscription was successful
        if not type_matches(self.status, Status.SUCCESS):
            logging.error(f"Failed to subscribe to SecureStateChanged event: {self.status}")
            return
        else:
            logging.info("Subscribed to SecureStateChanged event successfully")

        # STEP 6b: If LT feature is supported on the cluster or PS feature is not supported on the cluster, skip steps 6c to 6f
        self.step("6b")

        if is_lt_feature_supported or not is_ps_feature_supported:
            logging.info("Skipping steps 6c to 6f")
            is_skipped = True

        # STEP 6c: TH sends command MoveTo with Position = OpenInFull
        self.step("6c")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kOpenInFull,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 6d: Verify that the DUT has emitted the SecureStateChanged event
        self.step("6d")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        # Wait for the SecureStateChanged event to be emitted
        event = await self.wait_for_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, event=Clusters.ClosureControl.Events.SecureStateChanged)
        # Check if the event was received successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to receive SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event received successfully")
        # Check if the event data is correct
        if event.data == True:
            logging.error(f"Unexpected event data: {event.data}")
            return
        else:
            logging.info("Event data is correct")
        # Check if the event was emitted successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to emit SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event emitted successfully")

        # STEP 6e: TH sends command MoveTo with Position = CloseInFull
        self.step("6e")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kCloseInFull,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 6f: Verify that the DUT has emitted the SecureStateChanged event
        self.step("6f")

        if is_skipped == True:
            logging.info("Test step skipped")
            is_skipped = False
            return
        # Wait for the SecureStateChanged event to be emitted
        event = await self.wait_for_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, event=Clusters.ClosureControl.Events.SecureStateChanged)
        # Check if the event was received successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to receive SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event received successfully")
        # Check if the event data is correct
        if event.data == False:
            logging.error(f"Unexpected event data: {event.data}")
            return
        else:
            logging.info("Event data is correct")
        # Check if the event was emitted successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to emit SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event emitted successfully")

        is_skipped = False

        # STEP 6g: If LT feature is not supported on the cluster or PS feature is supported on the cluster, skip steps 6h to 6o
        self.step("6g")

        if not is_lt_feature_supported or is_ps_feature_supported:
            logging.info("Skipping steps 6h to 6o")
            is_skipped = True

        # STEP 6h: TH sends command MoveTo to DUT with Latch = False
        self.step("6h")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                latch=False,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 6i: Verify that the DUT has emitted the SecureStateChanged event
        self.step("6i")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        # Wait for the SecureStateChanged event to be emitted
        event = await self.wait_for_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, event=Clusters.ClosureControl.Events.SecureStateChanged)
        # Check if the event was received successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to receive SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event received successfully")
        # Check if the event data is correct
        if event.data == True:
            logging.error(f"Unexpected event data: {event.data}")
            return
        else:
            logging.info("Event data is correct")
        # Check if the event was emitted successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to emit SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event emitted successfully")

        # STEP 6j: If !CLCTRL.M.ManualLatching, skip steps 6k to 6l
        self.step("6j")

        if is_skipped == True:
            logging.info("Test step skipped")
            return

        manual_latching = self.check_pics('CLCTRL.M.ManualLatching')
        if not manual_latching:
            logging.info("Skipping steps 6k to 6l")
            is_manual_latching_skipped = True

        # STEP 6k: Latch the DUT manually to set OverallState.Latch to True
        self.step("6k")

        if is_skipped == True or is_manual_latching_skipped == True:
            logging.info("Test step skipped")
            return

        # Latch the DUT manually
        self._ask_for_manual_latching(endpoint, latched=True)

        # Check if the DUT was latched successfully
        if not type_matches(self.status, Status.SUCCESS):
            logging.error(f"Failed to latch DUT: {self.status}")
            return
        else:
            logging.info("DUT latched successfully")

        # STEP 6l: Verify that the DUT has emitted the SecureStateChanged event
        self.step("6l")

        if is_skipped == True or is_manual_latching_skipped == True:
            logging.info("Test step skipped")
            is_manual_latching_skipped = False
            return

        # Wait for the SecureStateChanged event to be emitted
        event = await self.wait_for_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, event=Clusters.ClosureControl.Events.SecureStateChanged)
        # Check if the event was received successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to receive SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event received successfully")
        # Check if the event data is correct
        if event.data == False:
            logging.error(f"Unexpected event data: {event.data}")
            return
        else:
            logging.info("Event data is correct")
        # Check if the event was emitted successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to emit SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event emitted successfully")
        is_manual_latching_skipped = False

        # STEP 6m: If CLCTRL.M.ManualLatching, skip steps 6n to 6o
        self.step("6m")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        if manual_latching:
            logging.info("Skipping steps 6n to 6o")
            is_manual_latching_skipped = True

        # STEP 6n: TH sends command MoveTo to DUT with Latch = True
        self.step("6n")

        if is_skipped == True or is_manual_latching_skipped == True:
            logging.info("Test step skipped")
            return
        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                latch=True,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(self.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {self.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 6o: Verify that the DUT has emitted the SecureStateChanged event
        self.step("6o")

        if is_skipped == True or is_manual_latching_skipped == True:
            logging.info("Test step skipped")
            is_manual_latching_skipped = False
            is_skipped = False
            return
        # Wait for the SecureStateChanged event to be emitted
        event = await self.wait_for_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, event=Clusters.ClosureControl.Events.SecureStateChanged)
        # Check if the event was received successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to receive SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event received successfully")
        # Check if the event data is correct
        if event.data == True:
            logging.error(f"Unexpected event data: {event.data}")
            return
        else:
            logging.info("Event data is correct")
        # Check if the event was emitted successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to emit SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event emitted successfully")
        is_manual_latching_skipped = False
        is_skipped = False

        # STEP 6p: If LT feature is not supported on the cluster or PS feature is not supported on the cluster, skip steps 6q to 6y
        self.step("6p")

        if not is_lt_feature_supported or not is_ps_feature_supported:
            logging.info("Skipping steps 6q to 6y")
            is_skipped = True

        # STEP 6q: TH sends command MoveTo to DUT with Position = OpenInFull and Latch = False
        self.step("6q")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kOpenInFull,
                latch=False,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 6r: Verify that the DUT has emitted the SecureStateChanged event
        self.step("6r")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        # Wait for the SecureStateChanged event to be emitted
        event = await self.wait_for_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, event=Clusters.ClosureControl.Events.SecureStateChanged)
        # Check if the event was received successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to receive SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event received successfully")
        # Check if the event data is correct
        if event.data == True:
            logging.error(f"Unexpected event data: {event.data}")
            return
        else:
            logging.info("Event data is correct")
        # Check if the event was emitted successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to emit SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event emitted successfully")

        # STEP 6s: If !CLCTRL.M.ManualLatching, skip steps 6t to 6v
        self.step("6s")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        if not manual_latching:
            logging.info("Skipping steps 6t to 6v")
            is_manual_latching_skipped = True

        # STEP 6t: TH sends command MoveTo to DUT with Position = CloseInFull
        self.step("6t")

        if is_skipped == True or is_manual_latching_skipped == True:
            logging.info("Test step skipped")
            return
        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kCloseInFull,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 6u: Latch the DUT manually to set OverallState.Latch to True
        self.step("6u")

        if is_skipped == True or is_manual_latching_skipped == True:
            logging.info("Test step skipped")
            return
        # Latch the DUT manually
        self._ask_for_manual_latching(endpoint, latched=True)
        # Check if the DUT was latched successfully
        if not type_matches(self.status, Status.SUCCESS):
            logging.error(f"Failed to latch DUT: {self.status}")
            return
        else:
            logging.info("DUT latched successfully")

        # STEP 6v: Verify that the DUT has emitted the SecureStateChanged event
        self.step("6v")

        if is_skipped == True or is_manual_latching_skipped == True:
            logging.info("Test step skipped")
            return
        # Wait for the SecureStateChanged event to be emitted
        event = await self.wait_for_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, event=Clusters.ClosureControl.Events.SecureStateChanged)
        # Check if the event was received successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to receive SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event received successfully")
        # Check if the event data is correct
        if event.data == False:
            logging.error(f"Unexpected event data: {event.data}")
            return
        else:
            logging.info("Event data is correct")
        # Check if the event was emitted successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to emit SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event emitted successfully")
        is_manual_latching_skipped = False

        # STEP 6w: If CLCTRL.M.ManualLatching, skip steps 6x to 6y
        self.step("6w")

        if is_skipped == True:
            logging.info("Test step skipped")
            return
        if manual_latching:
            logging.info("Skipping steps 6x to 6y")
            is_manual_latching_skipped = True

        # STEP 6x: TH sends command MoveTo to DUT with Position = CloseInFull and Latch = True
        self.step("6x")

        if is_skipped == True or is_manual_latching_skipped == True:
            logging.info("Test step skipped")
            return
        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kCloseInFull,
                latch=True,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 6y: Verify that the DUT has emitted the SecureStateChanged event
        self.step("6y")

        if is_skipped == True or is_manual_latching_skipped == True:
            logging.info("Test step skipped")
            is_manual_latching_skipped = False
            is_skipped = False
            return
        # Wait for the SecureStateChanged event to be emitted
        event = await self.wait_for_event(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, event=Clusters.ClosureControl.Events.SecureStateChanged)
        # Check if the event was received successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to receive SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event received successfully")
        # Check if the event data is correct
        if event.data == False:
            logging.error(f"Unexpected event data: {event.data}")
            return
        else:
            logging.info("Event data is correct")
        # Check if the event was emitted successfully
        if not type_matches(event.status, Status.SUCCESS):
            logging.error(f"Failed to emit SecureStateChanged event: {event.status}")
            return
        else:
            logging.info("SecureStateChanged event emitted successfully")
        is_manual_latching_skipped = False
        is_skipped = False


if __name__ == "__main__":
    default_matter_test_main()
