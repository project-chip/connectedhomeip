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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
from time import sleep
import queue
import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, SimpleEventCallback
from mobly import asserts

logger = logging.getLogger(__name__)

class TC_REFALM_2_2(MatterBaseTest):


    def TC_REFALM_2_2(self) -> str :
        return "223.2.2. [TC-REFALM-2.2] Primary functionality with DUT as Server"

    def pics_TC_REFALM_2_2(self):
        """Return PICS definitions asscociated with this test."""
        pics = [
            "REFALM.S"
        ]
        return pics

    def steps_TC_REFALM_2_2(self) -> list[TestStep]:
        steps = [
            TestStep( 1, "Commission DUT to TH (can be skipped if done in a preceding test)",is_commissioning=True),
            TestStep( 2, "Ensure that the door on the DUT is closed"),
            TestStep( 3, "TH reads from the DUT the State attribute"),
            TestStep( 4, "Manually open the door on the DUT"),
            # TestStep( 5, "Wait for the time defined in PIXIT.REFALM.AlarmThreshold"),
            TestStep( 6, "TH reads from the DUT the State attribute"),
            TestStep( 7, "Ensure that the door on the DUT is closed"),
            TestStep( 8, "TH reads from the DUT the State attribute"),
            TestStep( 9, "TH sends Reset command to the DUT"),
            TestStep( 10, "TH sends ModifyEnabledAlarms command to the DUT"),
            TestStep( 11, "Set up subscription to the Notify event"),
            TestStep( "12.a", "Repeating step 4 Manually open the door on the DUT"),
            #TestStep( "12.b", "Step 12b: Repeat step 5 Wait for the time defined in PIXIT.REFALM.AlarmThreshold"),
            TestStep( "12.c", "After step 5 (repeated), receive a Notify event with the State attribute bit 0 set to 1."),
            # TestStep( "13.a", "Repeat step 7 Ensure that the door on the DUT is closed"),
            # TestStep( "13.b", "Receive a Notify event with the State attribute bit 0 set to 0"),
        ]

        return steps

    def _ask_for_closed_door(self):
        user_response = self.wait_for_user_input(prompt_msg=f"Ensure that the door on the DUT is closed. Enter 'y' or 'n' after completition",
            prompt_msg_placeholder="y",
            default_value="y")
        asserts.assert_equal(user_response.lower(), "y")

    def _ask_for_open_door(self):
        user_response = self.wait_for_user_input(prompt_msg=f"Manually open the door on the DUT. Enter 'y' or 'n' after completition",
            prompt_msg_placeholder="y",
            default_value="y")
        asserts.assert_equal(user_response.lower(), "y")

    async def read_refalm_state_attribute(self):
        cluster = Clusters.Objects.RefrigeratorAlarm
        return await self.read_single_attribute_check_success(
            endpoint=self.endpoint,
            cluster=cluster,
            attribute=Clusters.RefrigeratorAlarm.Attributes.State
        )
    
    async def wait_thresshold(self,alarm_threshold):
        sleep(alarm_threshold)

    @async_test_body
    async def test_TC_REFALM_2_2(self):
        
        self.endpoint = self.get_endpoint(default=1)
        logger.info(f"Default endpoint {self.endpoint}")
        self.step(1)

        # # check is closed
        self.step(2)
        self._ask_for_closed_door()

        self.step(3)
        # reads the state attribute , must be a bitMap32 ( list wich values are 32 bits)
        device_state = await self.read_refalm_state_attribute()
        logger.info(f"The device state is {device_state}")
        asserts.assert_equal(device_state,0)


        # # open the door manually
        self.step(4)
        self._ask_for_open_door()
        # I think this will 
        Clusters.Objects.RefrigeratorAlarm.Bitmaps.AlarmBitmap.kDoorOpen = 1


        # # wait  PIXIT.REFALM.AlarmThreshold (5s)
        # self.step(5)

        # Result [MatterTest] 01-13 18:01:36.897 INFO Attr str [0, 2, 3, 65528, 65529, 65531, 65532, 65533]
        attr_list = await self.read_single_attribute_check_success(
            endpoint=self.endpoint,
            cluster=Clusters.Objects.RefrigeratorAlarm,
            attribute=Clusters.RefrigeratorAlarm.Attributes.AttributeList
        )
        logger.info(f"Attr list  {attr_list}")

        # Result [MatterTest] 01-13 18:01:36.897 INFO Attr str []
        accepted_command_list = await self.read_single_attribute_check_success(
            endpoint=self.endpoint,
            cluster=Clusters.Objects.RefrigeratorAlarm,
            attribute=Clusters.RefrigeratorAlarm.Attributes.AcceptedCommandList
        )        
        logger.info(f"Accepted commands {accepted_command_list}")


        # Result is empty[]
        generated_command_list = await self.read_single_attribute_check_success(
            endpoint=self.endpoint,
            cluster=Clusters.Objects.RefrigeratorAlarm,
            attribute=Clusters.RefrigeratorAlarm.Attributes.GeneratedCommandList
        )        
        logger.info(f"Generated commands {generated_command_list}")

        A0002_status  = self.check_pics('REFALM.S.A0002')
        logger.info(f"A0002 commands {A0002_status}")

        c00_status  = self.check_pics('REFALM.S.C00.Rsp')
        logger.info(f"c00 commands {c00_status}")

        c01_status  = self.check_pics('REFALM.S.C0.Rsp')
        logger.info(f"c01 commands {c01_status}")

        Clusters.Objects.RefrigeratorAlarm.Events.Notify

        # # read the status
        self.step(6)
        device_state = await self.read_refalm_state_attribute()
        asserts.assert_equal(device_state,1)

        # # # ensure the door is closed
        self.step(7)
        self._ask_for_closed_door()

        # # read from the state attr
        self.step(8)
        device_status = await self.read_refalm_state_attribute()
        asserts.assert_equal(device_status,0)

        # step 9
        self.step(9)
        asserts.assert_true(self.check_pics('REFALM.S.C00.Rsp'),"UNSUPPORTED_COMMAND(0x81)")

        if self.pics_guard(self.check_pics('REFALM.S .C00.Rsp') ):
            ret = await self.send_single_cmd(cmd=Clusters.Objects.RefrigeratorAlarm.acceptedCommandList.Reset,endpoint=self.endpoint)
            logger.info(f"Reset: {ret}")
         

        # step 10
        self.step(10)
        asserts.assert_true(self.check_pics('REFALM.S.C01.Rsp')," UNSUPPORTED_COMMAND(0x81)")

        if self.pics_guard(self.check_pics('REFALM.S.C01.Rsp')):
            ret = await self.send_single_cmd(cmd=Clusters.Objects.RefrigeratorAlarm.acceptedCommandList.ModifyEnabledAlarms,endpoint=self.endpoint)
            logger.info(f"ModifyEnabledAlarms: {ret}")

        # step 11
        self.step(11)
        notify_event = Clusters.RefrigeratorAlarm.Events.Notify
        self.q = queue.Queue()
        cb = SimpleEventCallback("Notify", notify_event.cluster_id, notify_event.event_id, self.q)
        urgent = 1
        subscription = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=[(self.endpoint, notify_event, urgent)], reportInterval=[1, 3])
        subscription.SetEventUpdateCallback(callback=cb)

        # step(12)
        self.step("12.a")
        # self._ask_for_open_door()

        # step(12)
        self.step("12.a")
        self._ask_for_open_door()


        # step(12.c)
        self.step("12.c")
        device_state = await self.read_refalm_state_attribute()
        logger.info(f"The device state is {device_state}")
        asserts.assert_equal(device_state,1)

        self.step("13.a")
        self._ask_for_closed_door()

        # check for StateAttribute
        self.step("13.b")
        device_state = await self.read_refalm_state_attribute()
        logger.info(f"The device state is {device_state}")
        asserts.assert_equal(device_state,0)

if __name__ == "__main__":
    default_matter_test_main()