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
from datetime import UTC, datetime
from enum import Enum, auto

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_asserts import assert_valid_bool, assert_valid_uint32
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.timeoperations import utc_datetime_from_matter_epoch_us

log = logging.getLogger(__name__)


class EventDataCheck(Enum):
    MATCH_REPORT_DATA = auto()
    IGNORE = auto()


class SmokeCoBaseTest(MatterBaseTest):

    smokeco_cluster = Clusters.SmokeCoAlarm
    smokeco_enums = Clusters.SmokeCoAlarm.Enums
    gd_cluster = Clusters.GeneralDiagnostics
    # Default Expiry Date : utc_time_in_matter_epoch(datetime(2126, 1, 1, 0, 0, 0, 0, tzinfo=timezone.utc)) / 1000000
    default_ci_expiry_date = 3976214400

    # SmokeAlarm triggers
    pixit_test_event_warning_smoke_alarm = 0x005c000000000090
    pixit_test_event_critical_smoke_alarm = 0x005c00000000009c
    pixit_test_event_clear_smoke_alarm = 0x005c0000000000a0

    # Smoke CO triggers
    pixit_test_event_warning_co_alarm = 0x005c000000000091
    pixit_test_event_critical_co_alarm = 0x005c00000000009d
    pixit_test_event_clear_co_alarm = 0x005c0000000000a1

    # Interconnect smoke alarm
    pixit_test_event_interconnected_smoke_alarm = 0x005c000000000092
    pixit_test_event_interconnected_smoke_alarm_clear = 0x005c0000000000a2

    # Interconnect co alarm triggers
    pixit_test_event_interconnected_co_alarm = 0x005c000000000094
    pixit_test_event_interconnected_co_alarm_clear = 0x005c0000000000a4

    # Contamination State triggers
    pixit_test_event_contamination_state_high = 0x005c000000000096
    pixit_test_event_contamination_state_low = 0x005c000000000097
    pixit_test_event_contamination_state_clear = 0x005c0000000000a6

    # Smoke Sensitivity triggers
    pixit_test_event_smokesensitivity_high = 0x005c000000000098
    pixit_test_event_smokesensitivity_low = 0x005c000000000099
    pixit_test_event_smokesensitivity_clear = 0x005c0000000000a8

    # Manual Device Mute
    pixit_test_event_manual_device_mute = 0x005c00000000009b
    pixit_test_event_manual_device_mute_clear = 0x005c0000000000ab

    # Battery triggers
    pixit_test_event_battery_warning = 0x005c000000000095
    pixit_test_event_battery_critical = 0x005c00000000009e
    pixit_test_event_battery_clear = 0x005c0000000000a5

    # Hardware triggers
    pixit_test_event_hardware_alert = 0x005c000000000093
    pixit_test_event_hardware_clear = 0x005c0000000000a3

    # Service Triggers
    pixit_test_event_service_alert = 0x005c00000000009a
    pixit_test_event_service_clear = 0x005c0000000000aa

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
        # Read and match events
        for event in events_response:
            if event.Header.EventId == smokeco_event.event_id:
                log.info(f"Event retrieved {event}")
                smoke_alarm_event_data = event.Data
        if smoke_alarm_event_data is None:
            asserts.fail(f"Failed to retrieve event for {smokeco_event}")
        return smoke_alarm_event_data

    async def read_attribute_check_range(self, attribute, enum):
        """Reads an attribute from the SmokeCluster and validate against a range."""
        attr = await self.read_smokeco_attribute_expect_success(attribute=attribute)
        is_valid = any(attr == item.value and str(item.name).lower() != "unknown" for item in enum)
        asserts.assert_true(is_valid, f"Value {attr} is not in the range for the Enum {enum}")

    async def read_attribute_check_bool(self, attribute):
        """Reads an attribute from the SmokeCluster and validate against a boolean value."""
        attr = await self.read_smokeco_attribute_expect_success(attribute=attribute)
        assert_valid_bool(value=attr, description=f"Attribute {attribute} is not a bool instance {attr}")

    async def read_attribute_check_epoch(self, attribute, check_expired: bool = False):
        """Reads an attribute from the SmokeCluster and validate is a int value represeting the seconds of matter epoch."""
        attr = await self.read_smokeco_attribute_expect_success(attribute=attribute)
        log.info(f"Reading attribte with value {attr} and checking the matter epoch ")
        # Number of seconds representing the matter epoch
        assert_valid_uint32(attr, "Attribute is not in uint range")
        if check_expired:
            self.is_valid_expired_date(attr)
        return attr

    def is_valid_expired_date(self, matter_epoch: int):
        """Asserts if the value proviced is in the future (Not expired)."""
        # Convert the epoch time from the device into UTC to compare it to current date
        device_utc_datetime = utc_datetime_from_matter_epoch_us(matter_epoch * 1000000)
        current_date = datetime.now(tz=UTC)
        log.info(f"Current matter epoch  {device_utc_datetime}")
        asserts.assert_true(device_utc_datetime > current_date,
                            f"Current matter_epoch is lower than current date. {device_utc_datetime} is an expired date.")

    async def read_general_diagnostics_test_event_triggers_enabled(self):
        "Read and return the TestEventTriggersEnabled from the GeneralDiagnosticsCluster"
        return await self.read_single_attribute_check_success(
            cluster=self.gd_cluster,
            attribute=self.gd_cluster.Attributes.TestEventTriggersEnabled,
            dev_ctrl=self.default_controller,
            endpoint=0)

    def start_device_self_test(self):
        """Start the device Self Test. Ask the user to manually start the self test. On CI it uses out of band communication."""
        if self.is_pics_sdk_ci_only:
            # LongPress will trigger the SelfTest in the SmokeCo cluster
            command_dict = {"Name": "LongPress", "EndpointId": self.get_endpoint(), "NewPosition": 0}
            self.write_to_app_pipe(command_dict=command_dict)
        else:
            self.wait_for_user_input(prompt_msg="Start manually DUT self-test", prompt_msg_placeholder="Enter 'y' when done")

    async def assert_steps_event_trigger_attr_report_actions(self,
                                                             steps: list,
                                                             pixit_event_trigger,
                                                             smoke_report_handler,
                                                             expected_report_data,
                                                             smoke_handler_timeout: int = 300,
                                                             ):
        """Verify attribute report data after eventTrigger and execute 2 steps

        Args:
            steps (list): List of steps to execute in this block
            pixit_event_trigger (int): hex code for event trigger
            smoke_report_handler (AttributeSubscriptionHandler): Attribute Report Handler for attribute under test
            expected_report_data (list,int): Expected report data
            smoke_handler_timeout (int, optional): Timeout for attribute report handler. Defaults to 300.
        """
        self.step(steps[0])
        await self.send_test_event_triggers(eventTrigger=pixit_event_trigger)

        self.step(steps[1])
        report_data = smoke_report_handler.wait_for_attribute_report(timeout_sec=smoke_handler_timeout)
        if isinstance(expected_report_data, list):
            asserts.assert_in(report_data.value, expected_report_data)
        else:
            asserts.assert_equal(report_data.value, expected_report_data)

    async def assert_steps_event_trigger_report_event_actions(self,
                                                              steps: list,
                                                              pixit_event_trigger,
                                                              smoke_report_handler,
                                                              smoke_event,
                                                              expected_report_data,
                                                              expected_event_data: EventDataCheck,
                                                              expected_expressed_state,
                                                              smoke_handler_timeout: int = 300
                                                              ):
        """Verify AttributeReport and EventReport after event trigger. This execute 4 steps.

        Args:
            steps (list): List of the 4 steps to execute
            pixit_event_trigger (int): Hex code of the event trigger
            smoke_report_handler (AttributeSubscriptionHandler): Attribute Report Handler for attribute under test
            smoke_event (Event): Event to read after the eventTrigger
            expected_report_data (list,Any): Expected value from the attribute report.
            expected_event_data (Any): Expected value from the Event. If set to REPORT_DATA match against AttrReport if IGNORE_DATA ignore check.
            expected_expressed_state (int): Expected value for the ExpressedState attribute
            smoke_handler_timeout (int, optional): Timeout for wait_for_attribute_report. Defaults to 300.
        """
        self.step(steps[0])
        await self.send_test_event_triggers(eventTrigger=pixit_event_trigger)

        self.step(steps[1])
        report_data = smoke_report_handler.wait_for_attribute_report(timeout_sec=smoke_handler_timeout)
        if isinstance(expected_report_data, list):
            asserts.assert_in(report_data.value, expected_report_data)
        else:
            asserts.assert_equal(report_data.value, expected_report_data)

        self.step(steps[2])
        event_data = await self.read_smokeco_event(smoke_event)
        if expected_event_data == EventDataCheck.MATCH_REPORT_DATA:
            asserts.assert_equal(event_data.alarmSeverityLevel, report_data.value)
        elif expected_event_data == EventDataCheck.IGNORE:
            # On ClearEvent test just check the event was recorded
            asserts.assert_is_not_none(event_data)

        self.step(steps[3])
        expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, expected_expressed_state)

    async def alarm_primary_functionality_base_test(self, state_attribute, alarm_event, expressed_state_enum_value, pixit_warning, pixit_critical, pixit_clear):
        """Define what attributes,events,enum values and pixit to use depending if is smoke alarm or co alarm for tests SMOKECO 2.2 and SMOKECO 2.3."""
        # Step 1, "Commission DUT to TH."
        self.step(1)  # Commissioning already done

        self.step(2)
        # Create Attribute Subscription
        smoke_state_handler = AttributeSubscriptionHandler(
            expected_cluster=self.smokeco_cluster, expected_attribute=state_attribute)
        await smoke_state_handler.start(dev_ctrl=self.default_controller, node_id=self.dut_node_id, endpoint=self.get_endpoint(), max_interval_sec=30)

        # Read attribute
        smoke_state = await self.read_smokeco_attribute_expect_success(attribute=state_attribute)
        asserts.assert_equal(smoke_state, self.smokeco_cluster.Enums.AlarmStateEnum.kNormal)

        self.step(3)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        self.step(4)
        # Reads General Diagnostic Cluster
        test_event_trigger_enabled = await self.read_single_attribute_check_success(
            cluster=self.gd_cluster,
            attribute=self.gd_cluster.Attributes.TestEventTriggersEnabled,
            dev_ctrl=self.default_controller,
            endpoint=0)
        asserts.assert_true(test_event_trigger_enabled, "Event Triggers are not enabled")

        self.step(5)
        # By default on endpoint 0
        await self.send_test_event_triggers(eventTrigger=pixit_warning)

        self.step(6)
        smoke_state_report = smoke_state_handler.wait_for_attribute_report(timeout_sec=300)
        log.info(f"Smoke state report {smoke_state_report} with value {smoke_state_report.value}")
        asserts.assert_equal(smoke_state_report.value, self.smokeco_cluster.Enums.AlarmStateEnum.kWarning)

        self.step(7)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, expressed_state_enum_value)

        self.step(8)
        smoke_alarm_event_data = await self.read_smokeco_event(alarm_event)
        log.info(f"SmokeAlarm Event {smoke_alarm_event_data}")
        asserts.assert_equal(smoke_alarm_event_data.alarmSeverityLevel, self.smokeco_cluster.Enums.AlarmStateEnum.kWarning)

        self.step(9)
        # When the Device has one Active Alarm SMOKE/CO or InterconnectedDevice Test should not start as the device is BUSY
        # Also ExpressedState should stay the same and not change to kTesting
        self.start_device_self_test()

        self.step(10)
        test_in_progress = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.TestInProgress)
        asserts.assert_false(test_in_progress, "Test is not in progress")
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, expressed_state_enum_value)

        # Gather these steps
        self.step(11)
        self.step(12)
        try:
            self_test_cmd = self.smokeco_cluster.Commands.SelfTestRequest()
            await self.send_single_cmd(self_test_cmd, dev_ctrl=self.default_controller, endpoint=self.get_endpoint(), timedRequestTimeoutMs=5000)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Busy, "Unexpected error returned")
            expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
            asserts.assert_equal(expressed_state, expressed_state_enum_value)

        self.step(13)
        test_in_progress = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.TestInProgress)
        asserts.assert_false(test_in_progress, "Test is not in progress")

        self.step(14)
        await self.send_test_event_triggers(eventTrigger=pixit_critical)

        self.step(15)
        smoke_state_report = smoke_state_handler.wait_for_attribute_report(timeout_sec=300)
        asserts.assert_equal(smoke_state_report.value, self.smokeco_cluster.Enums.AlarmStateEnum.kCritical)

        self.step(16)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, expressed_state_enum_value)

        self.step(17)
        smoke_alarm_event_data = await self.read_smokeco_event(alarm_event)
        # Critical is 2
        asserts.assert_equal(smoke_alarm_event_data.alarmSeverityLevel, self.smokeco_cluster.Enums.AlarmStateEnum.kCritical)

        # Smoke Alarm Test Event Clear
        self.step(18)
        await self.send_test_event_triggers(eventTrigger=pixit_clear)

        self.step(19)
        smoke_state_report_clear = smoke_state_handler.wait_for_attribute_report(timeout_sec=300)
        asserts.assert_equal(smoke_state_report_clear.value, self.smokeco_cluster.Enums.AlarmStateEnum.kNormal)
        smoke_state_handler.cancel()

        self.step(20)
        expressed_state_clear = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state_clear, self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        self.step(21)
        # This will fail if AllClearEvent is not retrieved
        await self.read_smokeco_event(self.smokeco_cluster.Events.AllClear)
