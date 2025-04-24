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


class TC_CLCTRL_3_1(MatterBaseTest):
    async def read_clctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_3_1(self) -> str:
        return "[TC_CLCTRL_3_1] Calibrate Command Primary Functionality with DUT as Server"

    def steps_TC_CLCTRL_3_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).", is_commissioning=True),
            TestStep("2a", "TH sends command Calibrate to DUT"),
            TestStep("2b", "after 1 seconds, TH reads from the DUT the MainState attribute"),
            TestStep("2c", "TH waits for PIXIT.CLCTRL.CalibrationDuration seconds"),
            TestStep("2d", "TH reads from the DUT the MainState attribute"),
            TestStep("3a", "TH sends command Calibrate to DUT"),
            TestStep("3b", "after 1 seconds, TH reads from the DUT the MainState attribute"),
            TestStep("3c", "TH sends command Calibrate to DUT"),
            TestStep("3d", "TH waits for PIXIT.CLCTRL.CalibrationDuration seconds"),
            TestStep("3e", "TH reads from the DUT the MainState attribute"),
            TestStep("4a", "TH sends command MoveTo to DUT with Position = CloseInFull(0)"),
            TestStep("4b", "after 1 seconds, TH reads from the DUT the MainState attribute"),
            TestStep("4c", "TH sends command Calibrate to DUT"),
        ]
        return steps

    def pics_TC_CLCTRL_3_1(self) -> list[str]:
        pics = [
            "CLCTRL.S",
            "CLCTRL.S.C02.Rsp(Calibrate)"
        ]
        return pics

    @async_test_body
    async def test_TC_CLCTRL_3_1(self):
        endpoint = self.get_endpoint(default=1)

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)
        attributes = Clusters.ClosureControl.Attributes

        # Read the FeatureMap attribute
        feature_map = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        # Check if the FeatureMap attribute was read successfully
        if not type_matches(feature_map.status, Status.SUCCESS):
            logging.error(f"Failed to read FeatureMap attribute: {feature_map.status}")
            return
        else:
            logging.info("FeatureMap attribute read successfully")
            logging.info(f"FeatureMap: {feature_map}")
            is_CL_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kCalibrate
            if not is_CL_feature_supported:
                logging.info("The feature Calibration is not supported by the DUT")
                for step in self.get_test_steps(self.current_test_info.name)[self.current_step.index:]:
                    # Skip the test steps that are not relevant
                    self.step(step.test_plan_number)
                    logging.info(f"Skipping test step {step.test_plan_number}")
                return

        # STEP 2a: TH sends command Calibrate to DUT
        self.step("2a")

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

        # STEP 2b: after 1 seconds, TH reads from the DUT the MainState attribute
        self.step("2b")

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

        # STEP 2c: TH waits for PIXIT.CLCTRL.CalibrationDuration seconds
        self.step("2c")

        calibration_duration = self.matter_test_config.global_test_params['PIXIT.CLCTRL.CalibrationDuration']
        if calibration_duration is None:
            logging.error("PIXIT.CLCTRL.CalibrationDuration is not defined")
            return
        if calibration_duration < 0:
            logging.error("PIXIT.CLCTRL.CalibrationDuration is negative")
            return
        if calibration_duration > 0:
            sleep(calibration_duration)
        else:
            logging.info("Calibration duration is 0, skipping wait")

        # STEP 2d: TH reads from the DUT the MainState attribute
        self.step("2d")

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

        # STEP 3c: TH sends command Calibrate to DUT
        self.step("3c")

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

        # STEP 3d: TH waits for PIXIT.CLCTRL.CalibrationDuration seconds
        self.step("3d")
        calibration_duration = self.matter_test_config.global_test_params['PIXIT.CLCTRL.CalibrationDuration']
        if calibration_duration is None:
            logging.error("PIXIT.CLCTRL.CalibrationDuration is not defined")
            return
        if calibration_duration < 0:
            logging.error("PIXIT.CLCTRL.CalibrationDuration is negative")
            return
        if calibration_duration > 0:
            sleep(calibration_duration)
        else:
            logging.info("Calibration duration is 0, skipping wait")

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

        # STEP 4a: TH sends command MoveTo to DUT with Position = CloseInFull(0)
        self.step("4a")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl,
                                    command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                                        position=Clusters.ClosureControl.Bitmaps.Position.kCloseInFull
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

        # STEP 4b: after 1 seconds, TH reads from the DUT the MainState attribute
        self.step("4b")

        sleep(1)
        mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
        if mainstate is NullValue:
            logging.error("Failed to read MainState attribute")
            return
        else:
            # Check if the MainState attribute has the expected values
            is_moving = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kMoving
            asserts.assert_true(is_moving, "MainState is not in the expected state")
            if is_moving:
                logging.info("MainState is in the moving state")
        # Check if the MainState attribute was read successfully
        if not type_matches(mainstate.status, Status.SUCCESS):
            logging.error(f"Failed to read MainState attribute: {mainstate.status}")
            return
        # Check if the MainState attribute was read successfully
        if not type_matches(mainstate.status, Status.SUCCESS):
            logging.error(f"Failed to read MainState attribute: {mainstate.status}")
            return

        # STEP 4c: TH sends command Calibrate to DUT
        self.step("4c")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.Calibrate)
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent invalid state
        if e.status == Status.INVALID_IN_STATE:
            logging.info("Command Calibrate is invalid while moving")
            return
        else:
            logging.error(f"Failed to send command Calibrate: {e.status}")
            return


if __name__ == "__main__":
    default_matter_test_main()
