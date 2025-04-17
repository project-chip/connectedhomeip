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


class TC_CLCTRL_5_1(MatterBaseTest):
    async def read_clctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_5_1(self) -> str:
        return "[TC_CLCTRL_5_1] Stop Command Primary Functionality with DUT as Server"

    def steps_TC_CLCTRL_5_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).", is_commissioning=True),
            TestStep("2a", "TH sends command MoveTo to DUT, with Position = OpenInFull"),
            TestStep("2b", "after 1 seconds, TH reads from the DUT the MainState attribute"),
            TestStep("2c", "TH sends command Stop to DUT"),
            TestStep("2d", "TH waits for PIXIT.CLCTRL.StopDuration seconds"),
            TestStep("2e", "TH reads from the DUT the MainState attribute"),
            TestStep("3a", "TH sends command Calibrate to DUT"),
            TestStep("3b", "after 2 seconds, TH reads from the DUT the MainState attribute"),
            TestStep("3c", "TH sends command Stop to DUT"),
            TestStep("3d", "TH waits for PIXIT.CLCTRL.StopDuration seconds"),
            TestStep("3e", "TH reads from the DUT the MainState attribute"),
            TestStep("4a", "TH sends command Stop to DUT"),
            TestStep("4b", "TH waits for PIXIT.CLCTRL.StopDuration seconds"),
            TestStep("4c", "TH reads from the DUT the MainState attribute"),
            TestStep("4d", "TH sends command Stop to DUT"),
            TestStep("5a", "TH reads TestEventTriggersEnabled attribute from General Diagnostic Cluster"),
            TestStep("5b", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is SetupRequired(7) Test Event"),
            TestStep("5c", "TH reads from the DUT the MainState attribute"),
            TestStep("5d", "TH sends command Stop to DUT"),
            TestStep("5e", "TH waits for PIXIT.CLCTRL.StopDuration seconds"),
            TestStep("5f", "TH reads from the DUT the MainState attribute"),
            TestStep("6a", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Protected(5) Test Event"),
            TestStep("6b", "TH reads from the DUT the MainState attribute"),
            TestStep("6c", "TH sends command Stop to DUT"),
            TestStep("6d", "TH waits for PIXIT.CLCTRL.StopDuration seconds"),
            TestStep("6e", "TH reads from the DUT the MainState attribute"),
            TestStep("7a", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Disengaged(6) Test Event"),
            TestStep("7b", "TH reads from the DUT the MainState attribute"),
            TestStep("7c", "TH sends command Stop to DUT"),
            TestStep("7d", "TH waits for PIXIT.CLCTRL.StopDuration seconds"),
            TestStep("7e", "TH reads from the DUT the MainState attribute"),
            TestStep("8", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear"),
        ]
        return steps

    def pics_TC_CLCTRL_5_1(self) -> list[str]:
        pics = [
            "CLCTRL.S",
            "CLCTRL.S.C00.Rsp(Stop)"
            "CLCTRL.S.F02(IS)"
        ]
        return pics

    @async_test_body
    async def test_TC_CLCTRL_5_1(self):
        endpoint = self.get_endpoint(default=1)

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)
        attributes = Clusters.ClosureControl.Attributes

        # STEP 2a: TH sends command MoveTo to DUT, with Position = OpenInFull
        self.step("2a")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kOpenInFull
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

        # STEP 2b: after 1 seconds, TH reads from the DUT the MainState attribute
        self.step("2b")

        sleep(1)
        mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
        if mainstate is NullValue:
            logging.error("Failed to read MainState attribute")
            return
        else:
            # Check if the MainState attribute has the expected values
            is_moving = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kMoving
            is_waiting = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kWaiting
            asserts.assert_true(is_moving or is_waiting, "MainState is not in the expected state")
            if is_moving or is_waiting:
                logging.info("MainState is in the moving or waiting state")
        # Check if the MainState attribute was read successfully
        if not type_matches(mainstate.status, Status.SUCCESS):
            logging.error(f"Failed to read MainState attribute: {mainstate.status}")
            return

        # STEP 2c: TH sends command Stop to DUT
        self.step("2c")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.Stop)
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command Stop: {e.status}")
            return
        else:
            logging.info("Command Stop sent successfully")

        # STEP 2d: TH waits for PIXIT.CLCTRL.StopDuration seconds
        self.step("2d")

        stop_duration = self.matter_test_config.global_test_params['PIXIT.CLCTRL.StopDuration']
        if stop_duration is None:
            logging.error("PIXIT.CLCTRL.StopDuration is not defined")
            return
        if stop_duration < 0:
            logging.error("PIXIT.CLCTRL.StopDuration is negative")
            return
        if stop_duration > 0:
            sleep(stop_duration)
        else:
            logging.info("Stop duration is 0, skipping wait")

        # STEP 2e: TH reads from the DUT the MainState attribute
        self.step("2e")

        # Read the MainState attribute from the DUT
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

        # STEP 3a: TH sends command Calibrate to DUT
        self.step("3a")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.Calibrate)
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command Calibrate: {e.status}")
            return
        else:
            logging.info("Command Calibrate sent successfully")

        # STEP 3b: after 1 seconds, TH reads from the DUT the MainState attribute
        self.step("3b")

        sleep(1)
        mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
        if mainstate is NullValue:
            logging.error("Failed to read MainState attribute")
            return
        else:
            # Check if the MainState attribute has the expected values
            is_calibrating = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kCalibrating
            asserts.assert_true(is_calibrating, "MainState is not in the expected state")
            if is_calibrating:
                logging.info("MainState is in the calibrating state")
        # Check if the MainState attribute was read successfully
        if not type_matches(mainstate.status, Status.SUCCESS):
            logging.error(f"Failed to read MainState attribute: {mainstate.status}")
            return

        # STEP 3c: TH sends command Stop to DUT
        self.step("3c")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.Stop)
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command Stop: {e.status}")
            return
        else:
            logging.info("Command Stop sent successfully")

        # STEP 3d: TH waits for PIXIT.CLCTRL.StopDuration seconds
        self.step("3d")

        stop_duration = self.matter_test_config.global_test_params['PIXIT.CLCTRL.StopDuration']
        if stop_duration is None:
            logging.error("PIXIT.CLCTRL.StopDuration is not defined")
            return
        if stop_duration < 0:
            logging.error("PIXIT.CLCTRL.StopDuration is negative")
            return
        if stop_duration > 0:
            sleep(stop_duration)
        else:
            logging.info("Stop duration is 0, skipping wait")

        # STEP 3e: TH reads from the DUT the MainState attribute
        self.step("3e")

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

        # STEP 4a: TH sends command Stop to DUT
        self.step("4a")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.Stop)
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command Stop: {e.status}")
            return
        else:
            logging.info("Command Stop sent successfully")

        # STEP 4b: TH waits for PIXIT.CLCTRL.StopDuration seconds
        self.step("4b")

        stop_duration = self.matter_test_config.global_test_params['PIXIT.CLCTRL.StopDuration']
        if stop_duration is None:
            logging.error("PIXIT.CLCTRL.StopDuration is not defined")
            return
        if stop_duration < 0:
            logging.error("PIXIT.CLCTRL.StopDuration is negative")
            return
        if stop_duration > 0:
            sleep(stop_duration)
        else:
            logging.info("Stop duration is 0, skipping wait")

        # STEP 4c: TH reads from the DUT the MainState attribute
        self.step("4c")

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

        # STEP 4d: TH sends command Stop to DUT
        self.step("4d")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.Stop)
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command Stop: {e.status}")
            return
        else:
            logging.info("Command Stop sent successfully")

        # STEP 5a: TH reads TestEventTriggersEnabled attribute from General Diagnostic Cluster
        self.step("5a")

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

        # STEP 5b: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is SetupRequired(7) Test Event
        self.step("5b")

        try:
            await self.send_command(endpoint=0, cluster=Clusters.Objects.GeneralDiagnostic, command=Clusters.GeneralDiagnostic.Commands.TestEventTrigger, arguments=Clusters.GeneralDiagnostic.TestEventTriggerRequest(
                enable_key=Clusters.GeneralDiagnostic.Bitmaps.EnableKey.kTestEventTriggerKey,
                event_trigger=Clusters.GeneralDiagnostic.Bitmaps.EventTrigger.kMainStateSetupRequired
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

        # STEP 5c: TH reads from the DUT the MainState attribute
        self.step("5c")

        mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
        if mainstate is NullValue:
            logging.error("Failed to read MainState attribute")
            return
        else:
            # Check if the MainState attribute has the expected values
            is_setup_required = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kSetupRequired
            asserts.assert_true(is_setup_required, "MainState is not in the expected state")
            if is_setup_required:
                logging.info("MainState is in the setup required state")
        # Check if the MainState attribute was read successfully
        if not type_matches(mainstate.status, Status.SUCCESS):
            logging.error(f"Failed to read MainState attribute: {mainstate.status}")
            return

        # STEP 5d: TH sends command Stop to DUT
        self.step("5d")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.Stop)
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command Stop: {e.status}")
            return
        else:
            logging.info("Command Stop sent successfully")

        # STEP 5e: TH waits for PIXIT.CLCTRL.StopDuration seconds
        self.step("5e")

        stop_duration = self.matter_test_config.global_test_params['PIXIT.CLCTRL.StopDuration']
        if stop_duration is None:
            logging.error("PIXIT.CLCTRL.StopDuration is not defined")
            return
        if stop_duration < 0:
            logging.error("PIXIT.CLCTRL.StopDuration is negative")
            return
        if stop_duration > 0:
            sleep(stop_duration)
        else:
            logging.info("Stop duration is 0, skipping wait")

        # STEP 5f: TH reads from the DUT the MainState attribute
        self.step("5f")

        mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
        if mainstate is NullValue:
            logging.error("Failed to read MainState attribute")
            return
        else:
            # Check if the MainState attribute has the expected values
            is_setup_required = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kSetupRequired
            asserts.assert_true(is_setup_required, "MainState is not in the expected state")
            if is_setup_required:
                logging.info("MainState is in the setup required state")
        # Check if the MainState attribute was read successfully
        if not type_matches(mainstate.status, Status.SUCCESS):
            logging.error(f"Failed to read MainState attribute: {mainstate.status}")
            return

        # STEP 6a: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Protected(5) Test Event
        self.step("6a")

        try:
            await self.send_command(endpoint=0, cluster=Clusters.Objects.GeneralDiagnostic, command=Clusters.GeneralDiagnostic.Commands.TestEventTrigger, arguments=Clusters.GeneralDiagnostic.TestEventTriggerRequest(
                enable_key=Clusters.GeneralDiagnostic.Bitmaps.EnableKey.kTestEventTriggerKey,
                event_trigger=Clusters.GeneralDiagnostic.Bitmaps.EventTrigger.kMainStateProtected
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

        # STEP 6b: TH reads from the DUT the MainState attribute
        self.step("6b")

        mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
        if mainstate is NullValue:
            logging.error("Failed to read MainState attribute")
            return
        else:
            # Check if the MainState attribute has the expected values
            is_protected = mainstate == Clusters.ClosureControl.Bitmaps.MainState
            asserts.assert_true(is_protected, "MainState is not in the expected state")
            if is_protected:
                logging.info("MainState is in the protected state")
        # Check if the MainState attribute was read successfully
        if not type_matches(mainstate.status, Status.SUCCESS):
            logging.error(f"Failed to read MainState attribute: {mainstate.status}")
            return

        # STEP 6c: TH sends command Stop to DUT
        self.step("6c")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.Stop)
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command Stop: {e.status}")
            return
        else:
            logging.info("Command Stop sent successfully")

        # STEP 6d: TH waits for PIXIT.CLCTRL.StopDuration seconds
        self.step("6d")

        stop_duration = self.matter_test_config.global_test_params['PIXIT.CLCTRL.StopDuration']
        if stop_duration is None:
            logging.error("PIXIT.CLCTRL.StopDuration is not defined")
            return
        if stop_duration < 0:
            logging.error("PIXIT.CLCTRL.StopDuration is negative")
            return
        if stop_duration > 0:
            sleep(stop_duration)
        else:
            logging.info("Stop duration is 0, skipping wait")

        # STEP 6e: TH reads from the DUT the MainState attribute
        self.step("6e")

        mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
        if mainstate is NullValue:
            logging.error("Failed to read MainState attribute")
            return
        else:
            # Check if the MainState attribute has the expected values
            is_protected = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kProtected
            asserts.assert_true(is_protected, "MainState is not in the expected state")
            if is_protected:
                logging.info("MainState is in the protected state")
        # Check if the MainState attribute was read successfully
        if not type_matches(mainstate.status, Status.SUCCESS):
            logging.error(f"Failed to read MainState attribute: {mainstate.status}")
            return

        # STEP 7a: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Disengaged(6) Test Event
        self.step("7a")

        try:
            await self.send_command(endpoint=0, cluster=Clusters.Objects.GeneralDiagnostic, command=Clusters.GeneralDiagnostic.Commands.TestEventTrigger, arguments=Clusters.GeneralDiagnostic.TestEventTriggerRequest(
                enable_key=Clusters.GeneralDiagnostic.Bitmaps.EnableKey.kTestEventTriggerKey,
                event_trigger=Clusters.GeneralDiagnostic.Bitmaps.EventTrigger.kMainStateDisengaged
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

        # STEP 7b: TH reads from the DUT the MainState attribute
        self.step("7b")

        mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
        if mainstate is NullValue:
            logging.error("Failed to read MainState attribute")
            return
        else:
            # Check if the MainState attribute has the expected values
            is_disengaged = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kDisengaged
            asserts.assert_true(is_disengaged, "MainState is not in the expected state")
            if is_disengaged:
                logging.info("MainState is in the disengaged state")
        # Check if the MainState attribute was read successfully
        if not type_matches(mainstate.status, Status.SUCCESS):
            logging.error(f"Failed to read MainState attribute: {mainstate.status}")
            return

        # STEP 7c: TH sends command Stop to DUT
        self.step("7c")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.Stop)
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command Stop: {e.status}")
            return
        else:
            logging.info("Command Stop sent successfully")

        # STEP 7d: TH waits for PIXIT.CLCTRL.StopDuration seconds
        self.step("7d")

        stop_duration = self.matter_test_config.global_test_params['PIXIT.CLCTRL.StopDuration']
        if stop_duration is None:
            logging.error("PIXIT.CLCTRL.StopDuration is not defined")
            return
        if stop_duration < 0:
            logging.error("PIXIT.CLCTRL.StopDuration is negative")
            return
        if stop_duration > 0:
            sleep(stop_duration)
        else:
            logging.info("Stop duration is 0, skipping wait")

        # STEP 7e: TH reads from the DUT the MainState attribute
        self.step("7e")

        mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
        if mainstate is NullValue:
            logging.error("Failed to read MainState attribute")
            return
        else:
            # Check if the MainState attribute has the expected values
            is_disengaged = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kDisengaged
            asserts.assert_true(is_disengaged, "MainState is not in the expected state")
            if is_disengaged:
                logging.info("MainState is in the disengaged state")
        # Check if the MainState attribute was read successfully
        if not type_matches(mainstate.status, Status.SUCCESS):
            logging.error(f"Failed to read MainState attribute: {mainstate.status}")
            return

        # STEP 8: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear
        self.step("8")

        try:
            await self.send_command(endpoint=0, cluster=Clusters.Objects.GeneralDiagnostic, command=Clusters.GeneralDiagnostic.Commands.TestEventTrigger, arguments=Clusters.GeneralDiagnostic.TestEventTriggerRequest(
                enable_key=Clusters.GeneralDiagnostic.Bitmaps.EnableKey.kTestEventTriggerKey,
                event_trigger=Clusters.GeneralDiagnostic.Bitmaps.EventTrigger.kMainStateTestEventClear
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


if __name__ == "__main__":
    default_matter_test_main()
