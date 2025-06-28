#
# Copyright (c) 2025 Project CHIP Authors
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
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/refalm_2_2_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --int-arg PIXIT.REFALM.AlarmThreshold:1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --app-pipe /tmp/refalm_2_2_fifo
# === END CI TEST ARGUMENTS ===

import logging
import typing
from dataclasses import dataclass
from time import sleep

import chip.clusters as Clusters
from chip import ChipUtility
from chip.clusters.ClusterObjects import ClusterCommand, ClusterObjectDescriptor, ClusterObjectFieldDescriptor
from chip.interaction_model import InteractionModelError, Status
from chip.testing import matter_asserts
from chip.testing.event_attribute_reporting import EventChangeCallback
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from chip.tlv import uint
from mobly import asserts

logger = logging.getLogger(__name__)

# TODO(#37217)  Refactor using generic method.
# Implement fake commands for the RefrigeratorAlarm Cluster
# These comands are Disallowed (X) in the spec.
# When running those commands must return 0x81 (UNSUPPORTED COMMAND)


@dataclass
class FakeReset(ClusterCommand):
    cluster_id: typing.ClassVar[int] = 0x00000057
    command_id: typing.ClassVar[int] = 0x00000000
    is_client: typing.ClassVar[bool] = True
    response_type: typing.ClassVar[typing.Optional[str]] = None

    @ChipUtility.classproperty
    def descriptor(cls) -> ClusterObjectDescriptor:
        return ClusterObjectDescriptor(
            Fields=[
                ClusterObjectFieldDescriptor(Label="alarms", Tag=0, Type=uint),
            ])

    alarms: uint = 0


@dataclass
class FakeModifyEnabledAlarms(ClusterCommand):
    cluster_id: typing.ClassVar[int] = 0x00000057
    command_id: typing.ClassVar[int] = 0x00000001
    is_client: typing.ClassVar[bool] = True
    response_type: typing.ClassVar[typing.Optional[str]] = None

    @ChipUtility.classproperty
    def descriptor(cls) -> ClusterObjectDescriptor:
        return ClusterObjectDescriptor(
            Fields=[
                ClusterObjectFieldDescriptor(Label="mask", Tag=0, Type=uint),
            ])

    mask: uint = 0


class TC_REFALM_2_2(MatterBaseTest):
    """Implementation of test case TC_REFALM_2_2."""

    def desc_TC_REFALM_2_2(self) -> str:
        return "223.2.2. [TC-REFALM-2.2] Primary functionality with DUT as Server"

    def pics_TC_REFALM_2_2(self):
        """Return PICS definitions asscociated with this test."""
        pics = [
            "REFALM.S"
        ]
        return pics

    def steps_TC_REFALM_2_2(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep(2, "Ensure that the door on the DUT is closed"),
            TestStep(3, "TH reads from the DUT the State attribute",
                     "Verify that the DUT response contains a 32-bit value with bit 0 set to 0"),
            TestStep(4, "Manually open the door on the DUT"),
            TestStep(5, "Wait for the time defined in PIXIT.REFALM.AlarmThreshold"),
            TestStep(6, "TH reads from the DUT the State attribute",
                     "Verify that the DUT response contains a 32-bit value with bit 0 set to 1"),
            TestStep(7, "Ensure that the door on the DUT is closed"),
            TestStep(8, "TH reads from the DUT the State attribute",
                     "Verify that the DUT response contains a 32-bit value with bit 0 set to 0"),
            TestStep(9, "TH sends Reset command to the DUT", "Verify DUT responds w/ status UNSUPPORTED_COMMAND(0x81)"),
            TestStep(10, "TH sends ModifyEnabledAlarms command to the DUT",
                     "Verify DUT responds w/ status UNSUPPORTED_COMMAND(0x81)"),
            TestStep(11, "Set up subscription to the Notify event"),
            TestStep(12, "Repeat steps 4 and then 5",
                     "After step 5 (repeated), receive a Notify event with the State attribute bit 0 set to 1."),
            TestStep(13, "Repeat step 7",
                     "Receive a Notify event with the State attribute bit 0 set to 0."),
        ]

        return steps

    async def _get_command_status(self, cmd: ClusterCommand):
        """Return the status of the executed command. By default the status is 0x0 unless a different 
        status on InteractionModel is returned. For this test we consider the status 0x0 as not succesfull."""
        cmd_status = Status.Success
        try:
            await self.default_controller.SendCommand(nodeid=self.dut_node_id, endpoint=self.endpoint, payload=cmd)
        except InteractionModelError as uc:
            cmd_status = uc.status
        return cmd_status

    def _ask_for_closed_door(self):
        if self.is_pics_sdk_ci_only:
            self._send_close_door_commnad()
        else:
            user_response = self.wait_for_user_input(prompt_msg="Ensure that the door on the DUT is closed. Enter 'y' or 'n' after completition",
                                                     default_value="y")
            asserts.assert_equal(user_response.lower(), "y")

    def _ask_for_open_door(self):
        if self.is_pics_sdk_ci_only:
            self._send_open_door_command()
        else:
            user_response = self.wait_for_user_input(prompt_msg="Manually open the door on the DUT. Enter 'y' or 'n' after completition",
                                                     default_value="y")
            asserts.assert_equal(user_response.lower(), "y")

    async def _read_refalm_state_attribute(self):
        cluster = Clusters.Objects.RefrigeratorAlarm
        return await self.read_single_attribute_check_success(
            endpoint=self.endpoint,
            cluster=cluster,
            attribute=Clusters.RefrigeratorAlarm.Attributes.State
        )

    def _wait_thresshold(self):
        """Wait the defined time at the PIXIT.REFALM.AlarmThreshold to trigger it."""
        logger.info(f"Sleeping for {self.refalm_threshold_seconds} seconds defined at PIXIT.REFALM.AlarmThreshold")
        sleep(self.refalm_threshold_seconds)

    def _send_open_door_command(self):
        command_dict = {"Name": "SetRefrigeratorDoorStatus", "EndpointId": self.endpoint,
                        "DoorOpen": Clusters.RefrigeratorAlarm.Bitmaps.AlarmBitmap.kDoorOpen}
        self.write_to_app_pipe(command_dict)

    def _send_close_door_commnad(self):
        command_dict = {"Name": "SetRefrigeratorDoorStatus", "EndpointId": self.endpoint, "DoorOpen": 0}
        self.write_to_app_pipe(command_dict)

    @async_test_body
    async def test_TC_REFALM_2_2(self):
        """Run the test steps."""
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.RefrigeratorAlarm
        logger.info(f"Default endpoint {self.endpoint}")
        # Commision the device.
        # Read required variables.
        self.step(1)
        asserts.assert_true('PIXIT.REFALM.AlarmThreshold' in self.matter_test_config.global_test_params, "PIXIT.REFALM.AlarmThreshold must be included on the command line in "
                            "the --int-arg flag as PIXIT.REFALM.AlarmThreshold:<AlarmThreshold> in seconds.")
        self.refalm_threshold_seconds = self.matter_test_config.global_test_params['PIXIT.REFALM.AlarmThreshold']

        # check is closed
        self.step(2)
        self._ask_for_closed_door()

        self.step(3)
        # reads the state attribute , must be a bitMap32 ( list wich values are 32 bits)
        device_state = await self._read_refalm_state_attribute()
        logger.info(f"The device state is {device_state}")
        matter_asserts.assert_valid_uint32(device_state, "State")
        asserts.assert_equal(device_state, 0)

        # # open the door manually
        self.step(4)
        self._ask_for_open_door()

        # wait  PIXIT.REFALM.AlarmThreshold (1s)
        self.step(5)
        self._wait_thresshold()

        # # read the status
        self.step(6)
        device_state = await self._read_refalm_state_attribute()
        logger.info(f"The device state is {device_state}")
        matter_asserts.assert_valid_uint32(device_state, "State")
        asserts.assert_equal(device_state, 1)

        # # # ensure the door is closed
        self.step(7)
        self._ask_for_closed_door()

        # # read from the state attr
        self.step(8)
        device_status = await self._read_refalm_state_attribute()
        logger.info(f"The device state is {device_state}")
        asserts.assert_equal(device_status, 0)
        matter_asserts.assert_valid_uint32(device_state, "State")

        self.step(9)
        cmd_status = await self._get_command_status(cmd=FakeReset())
        asserts.assert_equal(Status.UnsupportedCommand, cmd_status,
                             msg=f"Command status is not {Status.UnsupportedCommand}, is {cmd_status}")

        self.step(10)
        cmd_status = await self._get_command_status(cmd=FakeModifyEnabledAlarms())
        asserts.assert_equal(Status.UnsupportedCommand, cmd_status,
                             msg=f"Command status is not {Status.UnsupportedCommand}, is {cmd_status}")

        # Subscribe to Notify Event
        self.step(11)
        event_callback = EventChangeCallback(Clusters.RefrigeratorAlarm)
        await event_callback.start(self.default_controller,
                                   self.dut_node_id,
                                   self.get_endpoint(1))

        self.step(12)
        # repeat step 4 and 5
        logger.info("Manually open the door on the DUT")
        self._ask_for_open_door()
        logger.info("Wait for the time defined in PIXIT.REFALM.AlarmThreshold")
        self._wait_thresshold()
        # Wait for the Notify event with the State value.
        event_data = event_callback.wait_for_event_report(cluster.Events.Notify, timeout_sec=5)
        logger.info(f"Event data {event_data}")
        asserts.assert_equal(event_data.state, 1, "Unexpected value for State returned")

        self.step(13)
        logger.info("Ensure that the door on the DUT is closed")
        self._ask_for_closed_door()
        # Wait for the Notify event with the State value.
        event_data = event_callback.wait_for_event_report(cluster.Events.Notify, timeout_sec=5)
        logger.info(f"Event data {event_data}")
        asserts.assert_equal(event_data.state, 0, "Unexpected value for State returned")


if __name__ == "__main__":
    default_matter_test_main()
