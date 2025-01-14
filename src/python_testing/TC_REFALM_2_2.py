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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --app-pid ${CLUSTER_PID}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
from time import sleep
from typing import Any
import queue
import json
import chip.clusters as Clusters
import typing
from  chip.clusters.ClusterObjects import ClusterObjectDescriptor, ClusterCommand, ClusterObjectFieldDescriptor
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, SimpleEventCallback,type_matches
from mobly import asserts
from dataclasses import dataclass
from chip import ChipUtility
from chip.tlv import  uint


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

    def TC_REFALM_2_2(self) -> str :
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
            TestStep(3, "TH reads from the DUT the State attribute"),
            TestStep(4, "Manually open the door on the DUT"),
            TestStep(5, "Wait for the time defined in PIXIT.REFALM.AlarmThreshold"),
            TestStep(6, "TH reads from the DUT the State attribute"),
            TestStep(7, "Ensure that the door on the DUT is closed"),
            TestStep(8, "TH reads from the DUT the State attribute"),
            TestStep(9, "TH sends Reset command to the DUT"),
            TestStep(10, "TH sends ModifyEnabledAlarms command to the DUT"),
            TestStep(11, "Set up subscription to the Notify event"),
            TestStep("12.a", "Repeating step 4 Manually open the door on the DUT"),
            TestStep("12.b", "Step 12b: Repeat step 5 Wait for the time defined in PIXIT.REFALM.AlarmThreshold"),
            TestStep("12.c", "After step 5 (repeated), receive a Notify event with the State attribute bit 0 set to 1."),
            TestStep("13.a", "Repeat step 7 Ensure that the door on the DUT is closed"),
            TestStep("13.b", "Receive a Notify event with the State attribute bit 0 set to 0"),
        ]

        return steps

    async def _get_command_status(self,cmd: ClusterCommand,cmd_str:str=""):
        """Return the status of the executed command. By default the status is 0x0 unless a different 
        status on InteractionModel is returned. For this test we consider the status 0x0 as not succesfull."""
        cmd_status = Status.Success
        if self.is_ci:
            try:
                await self.default_controller.SendCommand(nodeid=self.dut_node_id,endpoint=self.endpoint,payload=cmd)
            except InteractionModelError as uc:
                cmd_status = uc.status
        else:
            user_response = self.wait_for_user_input(prompt_msg=f"{cmd_str} command is implemented in the DUT?. Enter 'y' or 'n' to confirm.",
                default_value="n")
            asserts.assert_equal(user_response.lower(), "n")
            if user_response.lower() == "n":
                cmd_status = Status.UnsupportedCommand
            
        return cmd_status


    def _ask_for_closed_door(self):
        if self.is_ci:
            self._send_close_door_commnad()
            sleep(1)
        else:
            user_response = self.wait_for_user_input(prompt_msg=f"Ensure that the door on the DUT is closed.",
                default_value="y")
            asserts.assert_equal(user_response.lower(), "y")

    def _ask_for_open_door(self):
        if self.is_ci:
            self._send_open_door_command()
        else:
            user_response = self.wait_for_user_input(prompt_msg=f"Manually open the door on the DUT. Enter 'y' or 'n' after completition",
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
        sleep(self.wait_thresshold_v/1000)

    def _send_named_pipe_command(self, command_dict: dict[str, Any]):
        app_pid = self.matter_test_config.app_pid
        if app_pid == 0:
            asserts.fail("The --app-pid flag must be set when usage of door state simulation named pipe is required (e.g. CI)")

        app_pipe = f"/tmp/chip_all_clusters_fifo_{app_pid}"
        command = json.dumps(command_dict)

        # Sends an out-of-band command to the sample app
        with open(app_pipe, "w") as outfile:
            logging.info(f"Sending named pipe command to {app_pipe}: '{command}'")
            outfile.write(command + "\n")
        # Delay for pipe command to be processed (otherwise tests may be flaky).
        sleep(0.1)

    def _send_open_door_command(self):
        command_dict  = {"Name":"SetRefrigeratorDoorStatus", "EndpointId": self.endpoint, "DoorOpen": Clusters.RefrigeratorAlarm.Bitmaps.AlarmBitmap.kDoorOpen}
        self._send_named_pipe_command(command_dict)

    def _send_close_door_commnad(self):
        command_dict  = {"Name":"SetRefrigeratorDoorStatus", "EndpointId": self.endpoint, "DoorOpen": 0}
        self._send_named_pipe_command(command_dict)

    @async_test_body
    async def test_TC_REFALM_2_2(self):
        """Run the test steps."""
        self.wait_thresshold_v = 5000
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.RefrigeratorAlarm
        
        logger.info(f"Default endpoint {self.endpoint}")
        self.step(1)

        # check is closed
        self.step(2)
        self._ask_for_closed_door()

        self.step(3)
        # reads the state attribute , must be a bitMap32 ( list wich values are 32 bits)
        device_state = await self._read_refalm_state_attribute()
        logger.info(f"The device state is {device_state}")
        asserts.assert_equal(device_state,0)


        # # open the door manually
        self.step(4)
        self._ask_for_open_door()

        # wait  PIXIT.REFALM.AlarmThreshold (5s)
        self.step(5)
        self._wait_thresshold()

        # # read the status
        self.step(6)
        device_state = await self._read_refalm_state_attribute()
        logger.info(f"The device state is {device_state}")
        asserts.assert_equal(device_state,1)

        # # # ensure the door is closed
        self.step(7)
        self._ask_for_closed_door()

        # # read from the state attr
        self.step(8)
        device_status = await self._read_refalm_state_attribute()
        logger.info(f"The device state is {device_state}")
        asserts.assert_equal(device_status,0)

        self.step(9)
        cmd_status  = await self._get_command_status(cmd=FakeReset(),cmd_str="Reset")
        asserts.assert_equal(Status.UnsupportedCommand,cmd_status, msg=f"Command status is not {Status.UnsupportedCommand}, is {cmd_status}")

        self.step(10)
        cmd_status = await self._get_command_status(cmd=FakeModifyEnabledAlarms(),cmd_str="ModifyEnabledAlarms")
        asserts.assert_equal(Status.UnsupportedCommand,cmd_status,msg=f"Command status is not {Status.UnsupportedCommand}, is {cmd_status}")

        
        # Subscribe to Notify Event
        self.step(11)
        self.q = queue.Queue()
        notify_event = Clusters.RefrigeratorAlarm.Events.Notify
        cb = SimpleEventCallback("Notify", notify_event.cluster_id, notify_event.event_id, self.q)
        urgent = 1
        subscription = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=[(self.endpoint, notify_event, urgent)], reportInterval=[2, 10])
        subscription.SetEventUpdateCallback(callback=cb)
        
        self.step("12.a")
        self._ask_for_open_door()

        self.step("12.b")
        self._wait_thresshold()

        self.step("12.c")
        # Wait for the Notify event with the State value.
        try:
            ret = self.q.get(block=True, timeout=5)
            logger.info(f"Event data {ret}")
            asserts.assert_true(type_matches(ret.Data, cluster.Events.Notify ),"Unexpected event type returned")
            asserts.assert_equal(ret.Data.state, 1,"Unexpected value for State returned")
        except queue.Empty:
            asserts.fail("Did not receive Notify event")

        self.step("13.a")
        self._ask_for_closed_door()

        self.step("13.b")
        # Wait for the Notify event with the State value.
        try:
            ret = self.q.get(block=True, timeout=5)
            logger.info(f"Event data {ret}")
            asserts.assert_true(type_matches(ret.Data, cluster.Events.Notify ),"Unexpected event type returned")
            asserts.assert_equal(ret.Data.state, 0,"Unexpected value for State returned")
        except queue.Empty:
            asserts.fail("Did not receive Notify event")

if __name__ == "__main__":
    default_matter_test_main()
