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

import logging

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_DISHALM_3_4(MatterBaseTest):

    async def read_da_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.DishwasherAlarm
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def subscribe_event(self, endpoint):
        return await self.default_controller.SubscribeEvent(self.dut_node_id, [(endpoint, Clusters.DishwasherAlarm.Events.Notify)])

    async def read_event_notify_state(self, endpoint):
        return await self.default_controller.ReadEvent(self.dut_node_id, [(endpoint, Clusters.DishwasherAlarm.Events.Notify.state)])

    async def read_event_notify(self, endpoint):
        return await self.default_controller.ReadEvent(self.dut_node_id, [(endpoint, Clusters.DishwasherAlarm.Events.Notify)])

    async def read_state_attribute(self, endpoint):
        return await self.read_da_attribute_expect_success(endpoint, Clusters.DishwasherAlarm.Attributes.State)

    async def read_latch_attribute(self, endpoint):
        return await self.read_da_attribute_expect_success(endpoint, Clusters.DishwasherAlarm.Attributes.Latch)

    async def send_reset_cmd(self, setAlarm) -> Clusters.Objects.DishwasherAlarm.Commands.Reset:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.DishwasherAlarm.Commands.Reset.alarms(setAlarm=setAlarm), endpoint=self.endpoint)
        return ret

    @async_test_body
    async def test_TC_DISHALM_3_4(self):

        endpoint = self.user_params.get("endpoint", 1)

        asserts.assert_true(self.check_pics("DISHALM.S.A0001"), "DISHALM.S.A0001 must be supported")

        self.print_step("1a", "Commissioning, already done")

        self.print_step("1b", "Set up subscription to Notify Event")
        await self.subscribe_event(endpoint=endpoint)

        if self.check_pics("DISHALM.S.A0002"):
            self.print_step("1c", "TH reads from the DUT the Latch attribute")
            latch_response = self.read_latch_attribute(endpoint=endpoint)
            logging.info("Latch: %s" % (latch_response))
        else:
            self.print_step("1d", "Optional Latch attribute not supported, set LatchResponse to 0")
            latch_response = 0

        self.print_step("2a", "Operate Device to set the condition to raise the TempTooLow alarm and wait a few seconds")
        input("Press Enter when done.\n")

        notify = self.read_event_notify(endpoint=endpoint)
        notify_state = self.read_event_notify_state(endpoint=endpoint)

        logging.info("Notify: %s" % (notify))
        logging.info("Notify State: %s" % (notify_state))

        asserts.assert_true(notify_state & Clusters.DishwasherAlarm.Bitmaps.AlarmMap.kTempTooLow, "State bit 3 is not set to TRUE")

        self.print_step("2b", "After a few seconds, read from the DUT the State Attribute")
        state = self.read_state_attribute(endpoint=endpoint)

        logging.info("State: %s" % (state))

        asserts.assert_true(state & Clusters.DishwasherAlarm.Bitmaps.AlarmMap.kTempTooLow, "Bit 3 of State is not set to 1")

        self.print_step("2c", "Operate Device to set the condition to lower the TempTooLow alarm and wait a few seconds")
        input("Press Enter when done.\n")

        notify = self.read_event_notify(endpoint=endpoint)
        notify_state = self.read_event_notify_state(endpoint=endpoint)

        logging.info("Notify: %s" % (notify))
        logging.info("Notify State: %s" % (notify_state))
        logging.info("Latch: %s" % (latch_response))

        if not latch_response & Clusters.DishwasherAlarm.Bitmaps.AlarmMap.kTempTooLow:
            asserts.assert_false(notify_state & Clusters.DishwasherAlarm.Bitmaps.AlarmMap.kTempTooLow,
                                 "State bit 3 is not set to FALSE")

        self.print_step("2d", "After a few seconds, read from the DUT the State Attribute")
        state = self.read_state_attribute(endpoint=endpoint)

        logging.info("State: %s" % (state))
        logging.info("Latch: %s" % (latch_response))

        if latch_response & Clusters.DishwasherAlarm.Bitmaps.AlarmMap.kTempTooLow:
            asserts.assert_true(state & Clusters.DishwasherAlarm.Bitmaps.AlarmMap.kTempTooLow, "Bit 3 of State is not set to 1 while bit 3 of Latch is 1")
        else:
            asserts.assert_false(state & Clusters.DishwasherAlarm.Bitmaps.AlarmMap.kTempTooLow, "Bit 3 of State is not set to 0 while bit 3 of Latch is 0")

        self.print_step("3a", "Send to the DUT the Reset Command with bit 3 of Alarms set to 1")
        alarm = Clusters.DishwasherAlarm.Bitmaps.AlarmMap.kTempTooLow

        await self.send_reset_cmd(setAlarm=alarm)

        notify = self.read_event_notify(endpoint=endpoint)
        notify_state = self.read_event_notify_state(endpoint=endpoint)

        logging.info("Notify: %s" % (notify))
        logging.info("Notify State: %s" % (notify_state))

        logging.info("Latch: %s" % (latch_response))

        if latch_response & Clusters.DishwasherAlarm.Bitmaps.AlarmMap.kTempTooLow:
            asserts.assert_false(notify_state & Clusters.DishwasherAlarm.Bitmaps.AlarmMap.kTempTooLow,
                                 "State bit 3 is not set to FALSE")

        self.print_step("3b", "After a few seconds, read from the DUT the State Attribute")
        state = self.read_state_attribute(endpoint=endpoint)

        logging.info("State: %s" % (state))

        asserts.assert_false(state & Clusters.DishwasherAlarm.Bitmaps.AlarmMap.kTempTooLow, "Bit 3 of State is not set to 0")


if __name__ == "__main__":
    default_matter_test_main()
