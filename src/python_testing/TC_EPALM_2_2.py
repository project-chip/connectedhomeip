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
#

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ELECTRICAL_PROTECTION_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --enable-key 000102030405060708090a0b0c0d0e0f
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import typing
from dataclasses import dataclass

from mobly import asserts
from TC_EPALM_TestBase import ElectricalProtectionAlarmTestBaseHelper

import matter.clusters as Clusters
from matter import ChipUtility
from matter.clusters.ClusterObjects import ClusterCommand, ClusterObjectDescriptor, ClusterObjectFieldDescriptor
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.runner import default_matter_test_main
from matter.tlv import uint

log = logging.getLogger(__name__)

cluster = Clusters.ElectricalProtectionAlarm

# Alarm Base Reset command id, inherited by EPALM.
RESET_COMMAND_ID = 0x00

REPORT_TIMEOUT_SEC = 10.0


# EPALM disallows the inherited RESET feature, so no Reset command is generated into the
# Python bindings for this cluster and it cannot be sent through Clusters.ElectricalProtectionAlarm.
# Declaring it locally is the established way to exercise the negative path; see TC_REFALM_2_2.py,
# which does the same for Refrigerator Alarm.
@dataclass
class FakeReset(ClusterCommand):
    cluster_id: typing.ClassVar[int] = 0x000000A3
    command_id: typing.ClassVar[int] = RESET_COMMAND_ID
    is_client: typing.ClassVar[bool] = True
    response_type: typing.ClassVar[str | None] = None

    @ChipUtility.classproperty
    def descriptor(cls) -> ClusterObjectDescriptor:
        return ClusterObjectDescriptor(
            Fields=[
                ClusterObjectFieldDescriptor(Label="alarms", Tag=0, Type=uint),
            ])

    alarms: uint = 0


class TC_EPALM_2_2(ElectricalProtectionAlarmTestBaseHelper):

    def pics_TC_EPALM_2_2(self) -> list[str]:
        return ["EPALM.S"]

    async def _command_status(self, command: ClusterCommand, endpoint: int) -> Status:
        """Send a command and return the resulting status, treating no exception as Success."""
        try:
            await self.default_controller.SendCommand(
                nodeId=self.dut_node_id, endpoint=endpoint, payload=command)
        except InteractionModelError as e:
            return e.status
        return Status.Success

    async def _trigger_and_expect_bit(self, sub: AttributeSubscriptionHandler,
                                      bit: cluster.Bitmaps.AlarmBitmap, name: str,
                                      endpoint: int, current_state: int) -> int:
        """Fire a fault trigger and confirm the bit ends up set. Returns the new State.

        The cluster reports only on an actual transition, so a DUT that already has this alarm
        raised emits nothing and waiting for a report would hang. Read in that case instead.
        """
        await self.send_test_event_trigger_set_alarm(bit)
        if current_state & int(bit):
            state = await self.read_state(endpoint)
            log.info('State after %s trigger: 0x%08X (already raised, no transition to report)', name, state)
        else:
            report = sub.wait_next_report(timeout_sec=REPORT_TIMEOUT_SEC)
            state = report.value
            log.info('State after %s trigger: 0x%08X', name, state)
        asserts.assert_true(int(state) & int(bit),
                            f'State must have the {name} bit set after its TestEventTrigger')
        return int(state)

    @async_test_body
    async def test_TC_EPALM_2_2(self):
        """[TC-EPALM-2.2] Primary functionality (alarm state machine) with Server as DUT

        This test case verifies the alarm state machine of the Electrical Protection Alarm
        Cluster server: that inducing each supported fault condition via the General Diagnostics
        TestEventTrigger sets the correct AlarmBitmap bit in State, and that the inherited Alarm
        Base Reset command is rejected because EPALM disallows the inherited RESET feature.
        ModifyEnabledAlarms is optional in Alarm Base and is checked against whichever behavior
        the DUT declares.
        """
        endpoint = self.get_endpoint()
        attributes = cluster.Attributes
        alarm_bits = cluster.Bitmaps.AlarmBitmap
        features = cluster.Bitmaps.Feature

        self.step(1, "Commission DUT to TH (already done)", is_commissioning=True)

        self.step(2, "TH reads TestEventTriggersEnabled from General Diagnostics on endpoint 0",
                  expectation="Value has to be 1 (True). If 0, skip the remaining steps and end the test case.")
        await self.check_test_event_triggers_enabled()

        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.FeatureMap)
        log.info('FeatureMap: 0x%08X', feature_map)

        accepted_commands = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.AcceptedCommandList)
        accepts_modify = cluster.Commands.ModifyEnabledAlarms.command_id in accepted_commands

        self.step(3, "TH establishes a subscription to the State attribute with MinIntervalFloor=0 "
                     "and MaxIntervalCeiling=30",
                  expectation="Subscription is established successfully; TH awaits subscription report of an initial "
                  "priming report carrying the current State value (in the no-fault baseline this is 0).")
        sub = AttributeSubscriptionHandler(expected_cluster=cluster, expected_attribute=attributes.State)
        await sub.start(self.default_controller, self.dut_node_id, endpoint,
                        min_interval_sec=0, max_interval_sec=30)
        # AttributeSubscriptionHandler.start() registers its update callback only after
        # ReadAttribute returns, so the priming report is delivered before the handler is
        # listening and never reaches its queue. Read the attribute for the baseline instead;
        # every later report in this test is change-driven and does reach the queue.
        priming_state = await self.read_state(endpoint)
        log.info('State at subscription time: 0x%08X', priming_state)

        current_state = int(priming_state)
        self.step(4, "TH sends the TestEventTrigger for ShortCircuitFault",
                  expectation="Verify DUT responds w/ status SUCCESS(0x00). TH awaits subscription report of a State "
                  "value with bit 0 (ShortCircuitFault) set.")
        if feature_map & features.kShortCircuit:
            current_state = await self._trigger_and_expect_bit(
                sub, alarm_bits.kShortCircuitFault, "ShortCircuitFault", endpoint, current_state)
        else:
            self.mark_current_step_skipped()

        self.step(5, "TH sends the TestEventTrigger for OverLoadFault",
                  expectation="Verify DUT responds w/ status SUCCESS(0x00). TH awaits subscription report of a State "
                  "value with bit 1 (OverLoadFault) set.")
        if feature_map & features.kOverLoad:
            current_state = await self._trigger_and_expect_bit(
                sub, alarm_bits.kOverLoadFault, "OverLoadFault", endpoint, current_state)
        else:
            self.mark_current_step_skipped()

        self.step(6, "TH sends the TestEventTrigger for OverVoltageFault",
                  expectation="Verify DUT responds w/ status SUCCESS(0x00). TH awaits subscription report of a State "
                  "value with bit 2 (OverVoltageFault) set.")
        if feature_map & features.kOverVoltage:
            current_state = await self._trigger_and_expect_bit(
                sub, alarm_bits.kOverVoltageFault, "OverVoltageFault", endpoint, current_state)
        else:
            self.mark_current_step_skipped()

        self.step(7, "TH sends the TestEventTrigger for VoltageSurgeFault",
                  expectation="Verify DUT responds w/ status SUCCESS(0x00). TH awaits subscription report of a State "
                  "value with bit 3 (VoltageSurgeFault) set.")
        if feature_map & features.kSurgeProtection:
            current_state = await self._trigger_and_expect_bit(
                sub, alarm_bits.kVoltageSurgeFault, "VoltageSurgeFault", endpoint, current_state)
        else:
            self.mark_current_step_skipped()

        self.step(8, "TH sends the TestEventTrigger for ResidualCurrentFault",
                  expectation="Verify DUT responds w/ status SUCCESS(0x00). TH awaits subscription report of a State "
                  "value with bit 4 (ResidualCurrentFault) set.")
        if feature_map & features.kResidualCurrent:
            current_state = await self._trigger_and_expect_bit(
                sub, alarm_bits.kResidualCurrentFault, "ResidualCurrentFault", endpoint, current_state)
        else:
            self.mark_current_step_skipped()

        self.step(9, "TH sends the TestEventTrigger for ArcFault",
                  expectation="Verify DUT responds w/ status SUCCESS(0x00). TH awaits subscription report of a State "
                  "value with bit 5 (ArcFault) set.")
        if feature_map & features.kArcFault:
            current_state = await self._trigger_and_expect_bit(
                sub, alarm_bits.kArcFault, "ArcFault", endpoint, current_state)
        else:
            self.mark_current_step_skipped()

        self.step(10, "TH sends the TestEventTrigger for SelfTest",
                  expectation="Verify DUT responds w/ status SUCCESS(0x00). TH awaits subscription report of a State "
                  "value with bit 6 (SelfTest) set.")
        if feature_map & features.kSelfTest:
            current_state = await self._trigger_and_expect_bit(
                sub, alarm_bits.kSelfTest, "SelfTest", endpoint, current_state)
        else:
            self.mark_current_step_skipped()

        self.step(11, "TH reads the State attribute and inspects the value",
                  expectation="For every bit b that is set: the feature corresponding to bit b is supported by the DUT "
                  "(per Supported). For every bit b that is NOT supported by the DUT: bit b MUST NOT be set "
                  "(feature gating is correctly enforced).")
        state = await self.read_state(endpoint)
        supported = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.Supported)
        log.info('State: 0x%08X  Supported: 0x%08X', state, supported)
        asserts.assert_equal(int(state) & ~int(supported), 0,
                             'State must not set any bit that is absent from Supported')

        self.step(12, "TH sends the Reset command (Alarm Base.Reset) to the DUT",
                  expectation="Verify that the DUT response contains UNSUPPORTED_COMMAND (0x81). Reason: EPALM "
                  "disallows the inherited RESET feature, so the Reset command is not callable.")
        status = await self._command_status(FakeReset(alarms=0), endpoint)
        asserts.assert_equal(status, Status.UnsupportedCommand,
                             'Reset must return UNSUPPORTED_COMMAND: EPALM disallows the RESET feature')

        self.step(13, "If the DUT does not accept ModifyEnabledAlarms, TH sends it anyway",
                  expectation="Verify that the DUT response contains UNSUPPORTED_COMMAND (0x81). If the DUT declares "
                  "EPALM.S.C01.Rsp(ModifyEnabledAlarms), the step is skipped.")
        if accepts_modify:
            self.mark_current_step_skipped()
        else:
            status = await self._command_status(
                cluster.Commands.ModifyEnabledAlarms(mask=0), endpoint)
            asserts.assert_equal(status, Status.UnsupportedCommand,
                                 'ModifyEnabledAlarms must return UNSUPPORTED_COMMAND when the DUT '
                                 'does not accept it')

        self.step("13a", "If the DUT accepts ModifyEnabledAlarms, TH sends it",
                  expectation="Verify DUT responds w/ status SUCCESS(0x00). The command is optional in Alarm Base, so a "
                  "DUT that implements it SHALL accept it rather than reject it. If the DUT does not "
                  "declare EPALM.S.C01.Rsp(ModifyEnabledAlarms), the step is skipped.")
        if not accepts_modify:
            self.mark_current_step_skipped()
        else:
            status = await self._command_status(
                cluster.Commands.ModifyEnabledAlarms(mask=int(supported)), endpoint)
            asserts.assert_equal(status, Status.Success,
                                 'ModifyEnabledAlarms is optional in Alarm Base, so a DUT that '
                                 'accepts it must succeed rather than reject')

        self.step("13b", "TH sends the ModifyEnabledAlarms command with a mask that clears at least one "
                         "bit set in Supported, then TH reads from the DUT the Mask attribute.",
                  expectation="Value has to be the mask TH sent, with the cleared bit no longer set. If the "
                  "DUT does not declare EPALM.S.C01.Rsp(ModifyEnabledAlarms), the step is skipped.")
        if not accepts_modify or not int(supported):
            self.mark_current_step_skipped()
        else:
            lowest_supported_bit = int(supported) & -int(supported)
            reduced_mask = int(supported) & ~lowest_supported_bit
            status = await self._command_status(
                cluster.Commands.ModifyEnabledAlarms(mask=reduced_mask), endpoint)
            asserts.assert_equal(status, Status.Success,
                                 'ModifyEnabledAlarms must succeed when the DUT accepts it')
            mask = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attributes.Mask)
            asserts.assert_equal(int(mask), reduced_mask,
                                 'Mask must reflect the value sent to ModifyEnabledAlarms')

        self.step(14, "TH sends the TestEventTrigger that clears all alarms",
                  expectation="Verify DUT responds w/ status SUCCESS(0x00). TH awaits subscription report of a State "
                  "value of 0 (cleanup).")
        await self.send_test_event_trigger_clear_all()
        if int(state) == 0:
            # Nothing was raised, so clearing is a no-op and the cluster reports nothing.
            final_state = await self.read_state(endpoint)
        else:
            final_state = int(sub.wait_next_report(timeout_sec=REPORT_TIMEOUT_SEC).value)
        asserts.assert_equal(final_state, 0, 'State must be 0 after the clear-all trigger')
        sub.cancel()


if __name__ == "__main__":
    default_matter_test_main()
