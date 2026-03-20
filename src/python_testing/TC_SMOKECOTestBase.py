#
#    Copyright (c) 2026 Project CHIP Authors
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

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_asserts import assert_int_in_range, assert_is_unixtimestamp, assert_valid_bool, assert_valid_uint32
from matter.testing.matter_testing import MatterBaseTest

log = logging.getLogger(__name__)


class SmokeCoBaseTest(MatterBaseTest):

    smokeco_cluster = Clusters.SmokeCoAlarm

    async def read_smokeco_attribute_expect_success(self, attribute):
        return await self.read_single_attribute_check_success(cluster=self.smokeco_cluster, endpoint=self.get_endpoint(), attribute=attribute)

    async def read_smokeco_event(self, smokeco_event):
        urgent = 1
        events_response = await self.default_controller.ReadEvent(
            self.dut_node_id,
            events=[(self.get_endpoint(), smokeco_event, urgent)],
            fabricFiltered=True
        )
        log.info(f"Events gathered {events_response}")
        if len(events_response) == 0:
            asserts.fail("Failed to read events")
        smoke_alarm_event_data = None
        # Read tand match events
        for event in events_response:
            if event.Header.EventId == smokeco_event.event_id:
                log.info(f"Event retrieved {event}")
                smoke_alarm_event_data = event.Data
        if smoke_alarm_event_data is None:
            asserts.fail(f"Failed to retrieve event for {smokeco_event}")
        return smoke_alarm_event_data

    async def read_attribute_check_range(self, attribute, range_low: int, range_high: int):
        """Reads an attribute from the SmokeCluster and validate against a range."""
        attr = await self.read_smokeco_attribute_expect_success(attribute=attribute)
        assert_int_in_range(attr, range_low, range_high, f"Attribute {attribute} is out of range {attr}")

    async def read_attribute_check_bool(self, attribute):
        """Reads an attribute from the SmokeCluster and validate against a boolean value."""
        attr = await self.read_smokeco_attribute_expect_success(attribute=attribute)
        assert_valid_bool(value=attr, description=f"Attribute {attribute} is not a bool instance {attr}")

    async def read_attribute_check_epoch(self, attribute):
        """Reads an attribute from the SmokeCluster and validate against a valid timestmap value."""
        attr = await self.read_smokeco_attribute_expect_success(attribute=attribute)
        assert_valid_uint32(attr, "Attribute is not in uint range")
        assert_is_unixtimestamp(attr, f"Attribute with value: {attr}")

    async def alarm_primary_functionality_base_test(self,state_attribute,alarm_event,expressed_state_enum_value,pixit_warning,pixit_critical,pixit_clear):
        """Define what attribtues,events,enum values and pixit to use depending if is smoke alarm or co alarm for tests SMOKECO 2.2 and SMOKECO 2.3."""
        # Step 1, "Commission DUT to TH."
        self.step(1)  # Commissioning already done

        self.step(2)
        # Create Attribute Subscription
        smoke_state_handler = AttributeSubscriptionHandler(expected_cluster=self.smokeco_cluster, expected_attribute=state_attribute)
        await smoke_state_handler.start(dev_ctrl=self.default_controller, node_id=self.dut_node_id, endpoint=self.get_endpoint(), max_interval_sec=30)

        # Read attribute
        smoke_state = await self.read_smokeco_attribute_expect_success(attribute=state_attribute)
        asserts.assert_equal(smoke_state,self.smokeco_cluster.Enums.AlarmStateEnum.kNormal)

        self.step(3)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state,self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        self.step(4)
        # Reads General Diagnostic Cluster
        test_event_trigger_enabled = await self.read_single_attribute_check_success(
            cluster=self.gd_cluster,
            attribute=self.gd_cluster.Attributes.TestEventTriggersEnabled,
            dev_ctrl=self.default_controller,
            endpoint=0)
        asserts.assert_equal(test_event_trigger_enabled,True,"TestEventTriggersEnabled is not True")

        self.step(5)
        # By defalt on endpoint 0
        await self.send_test_event_triggers(eventTrigger=pixit_warning)

        self.step(6)
        smoke_state_report = smoke_state_handler.wait_for_attribute_report(timeout_sec=300)
        log.info(f"Smoke state report {smoke_state_report} with value {smoke_state_report.value}")
        asserts.assert_equal(smoke_state_report.value,self.smokeco_cluster.Enums.AlarmStateEnum.kWarning)

        self.step(7)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state,expressed_state_enum_value)

        self.step(8)
        smoke_alarm_event_data = await self.read_smokeco_event(alarm_event)
        log.info(f"SmokeAlarm Event {smoke_alarm_event_data}")
        asserts.assert_equal(smoke_alarm_event_data.alarmSeverityLevel,self.smokeco_cluster.Enums.AlarmStateEnum.kWarning)

        self.step(9)
        # Manually Start the Seft Test
        if self.is_pics_sdk_ci_only:
            # LongPress will trigger the SelfTest in the SmokeCo cluster
            command_dict = {"Name": "LongPress", "EndpointId": self.get_endpoint(),"NewPosition": 0}
            self.write_to_app_pipe(command_dict=command_dict)
        else:
            self.wait_for_user_input(prompt_msg="Start manually DUT self-test",prompt_msg_placeholder="Enter 'y' when done")

        self.step(10)
        test_in_progress = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.TestInProgress)
        asserts.assert_equal(test_in_progress,False)

        # Gather these steps
        self.step(11)
        self.step(12)
        try:
            self_test_cmd = self.smokeco_cluster.Commands.SelfTestRequest()
            await self.send_single_cmd(self_test_cmd,dev_ctrl=self.default_controller,endpoint=self.get_endpoint(),timedRequestTimeoutMs=5000)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Busy, "Unexpected error returned")

        self.step(13)
        test_in_progress = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.TestInProgress)
        asserts.assert_equal(test_in_progress,False)

        self.step(14)
        await self.send_test_event_triggers(eventTrigger=pixit_critical)

        self.step(15)
        smoke_state_report = smoke_state_handler.wait_for_attribute_report(timeout_sec=300)
        asserts.assert_equal(smoke_state_report.value,self.smokeco_cluster.Enums.AlarmStateEnum.kCritical)

        self.step(16)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state,expressed_state_enum_value)

        self.step(17)
        smoke_alarm_event_data = await self.read_smokeco_event(alarm_event)
        # Critical is 2
        asserts.assert_equal(smoke_alarm_event_data.alarmSeverityLevel,self.smokeco_cluster.Enums.AlarmStateEnum.kCritical)

        # Smoke Alarm Test Event Clear
        self.step(18)
        await self.send_test_event_triggers(eventTrigger=pixit_clear)

        self.step(19)
        smoke_state_report_clear = smoke_state_handler.wait_for_attribute_report(timeout_sec=300)
        asserts.assert_equal(smoke_state_report_clear.value,self.smokeco_cluster.Enums.AlarmStateEnum.kNormal)
        smoke_state_handler.cancel()

        self.step(20)
        expressed_state_clear = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state_clear,self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        self.step(21)
        # This will fail if AllClearEvent is not retrieved
        await self.read_smokeco_event(self.smokeco_cluster.Events.AllClear)
