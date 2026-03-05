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
#     app: ${LIT_ICD_APP}
#     app-args: >
#       --discriminator 1234
#       --passcode 20202021
#       --KVS kvs1
#       --trace-to json:${TRACE_TEST_JSON}-app.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import os

from mobly import asserts
from support_modules.icd_support import ICDTestEventTriggerOperations, uat_bit_name, uat_set_hints

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

'''
Purpose
Test validates that an ICD will send Check-In messages when entering Active mode if no subscription is active.
This test validates device enters active mode via the User Active Mode Trigger (UAT).

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/icdbehavior.adoc#tc-icdb-1-2-icd-check-in-protocol-register-client-user-active-mode-trigger-dut_server
'''

'''
Notes/Considerations
In CI, the UAT is simulated via the kAddActiveModeReq test event trigger.
On a real DUT, the UAT is performed physically per UserActiveModeTriggerHint/Instruction.
For a real DUT, use the --timeout <seconds> script argument if wait times are large to prevent the test from timing out.
'''

ROOT_ENDPOINT_ID = 0
MAX_CI_IDLE_CYCLE_WAIT_S = 10

cluster = Clusters.Objects.IcdManagement
attributes = cluster.Attributes
commands = cluster.Commands
clientTypeEnum = cluster.Enums.ClientTypeEnum
features = cluster.Bitmaps.Feature


class TC_ICDB_1_2(MatterBaseTest):

    def desc_TC_ICDB_1_2(self) -> str:
        return "[TC-ICDB-1.2] ICD Check-In Protocol - Register client - user active mode trigger [DUT as Server]"

    def steps_TC_ICDB_1_2(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads from the DUT the RegisteredClients attribute.",
                     "If clients are registered, unregister them."),
            TestStep(2, "TH reads from the DUT the IdleModeDuration, ActiveModeDuration, ActiveModeThreshold, UserActiveModeTriggerHint, and UserActiveModeTriggerInstruction attributes.",
                     "Store all values for later use."),
            TestStep(3, "TH sends RegisterClient command with parameters: CheckInNodeID: <th_node_id>, MonitoredSubject: <th_node_id>, and Key: <any_16_byte_octstr>.", """
                     DUT ActiveModeDuration timer is reset.
                     DUT responds with an ICDCounter value, store the value as icd_counter_at_registration."""),
            TestStep(4, "Wait for ActiveModeDuration plus a 1-second buffer.",
                     "DUT transitions from Active Mode to Idle Mode."),
            TestStep(5, "Use UAT hint/instructions to transition DUT from Idle Mode to Active Mode.", """
                     For each UAT hint, the DUT transitions from Idle to Active Mode and sends back a check-in message.
                     Verify the ICDCounter value increased.
                     Verify the ActiveModeThreshold value is unchanged."""),
            TestStep(6, "TH reads from the DUT the RegisteredClients attribute.",
                     "If clients are registered, unregister them."),
        ]

    async def _read_icdm_attribute_expect_success(self, attribute):
        return await self.read_single_attribute_check_success(endpoint=ROOT_ENDPOINT_ID, cluster=cluster, attribute=attribute)

    async def _send_single_icdm_command(self, command):
        return await self.send_single_cmd(command, endpoint=ROOT_ENDPOINT_ID)

    async def unregister_all_clients(self):
        """Unregisters all entries in the DUT's RegisteredClients attribute."""
        registeredClients = await self._read_icdm_attribute_expect_success(attributes.RegisteredClients)

        if not registeredClients:
            log.info("RegisteredClients is empty.")
            return

        log.info("RegisteredClients is not empty; unregistering all clients...")
        for client in registeredClients:
            try:
                log.info(f"Unregistering client: {client}...")
                await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=client.checkInNodeID))
            except InteractionModelError as e:
                asserts.assert_fail(f"Unexpected error returned when unregistering client: {e}")

    def pics_TC_ICDB_1_2(self) -> list[str]:
        return [
            "ICDB.S",
            "ICDM.S.F00",
        ]

    @async_test_body
    async def test_TC_ICDB_1_2(self):

        # *** PRECONDITION ***
        # Commissioning, already done
        self.step("precondition")

        # *** STEP 1 ***
        # TH reads from the DUT the RegisteredClients attribute
        # If clients are registered, unregister them
        self.step(1)
        await self.unregister_all_clients()

        # *** STEP 2 ***
        # TH reads from the DUT the IdleModeDuration, ActiveModeDuration, ActiveModeThreshold,
        # UserActiveModeTriggerHint, and UserActiveModeTriggerInstruction attributes
        self.step(2)
        idle_mode_duration_s = await self._read_icdm_attribute_expect_success(attributes.IdleModeDuration)
        active_mode_duration_ms = await self._read_icdm_attribute_expect_success(attributes.ActiveModeDuration)
        active_mode_threshold_ms = await self._read_icdm_attribute_expect_success(attributes.ActiveModeThreshold)
        user_active_mode_trigger_hint = await self._read_icdm_attribute_expect_success(attributes.UserActiveModeTriggerHint)
        user_active_mode_trigger_instruction = await self._read_icdm_attribute_expect_success(attributes.UserActiveModeTriggerInstruction)
        log.info(f"IdleModeDuration: {idle_mode_duration_s}s")
        log.info(f"ActiveModeDuration: {active_mode_duration_ms}ms")
        log.info(f"ActiveModeThreshold: {active_mode_threshold_ms}ms")
        log.info(f"UserActiveModeTriggerHint: 0x{user_active_mode_trigger_hint:08X}")
        log.info(f"UserActiveModeTriggerInstruction: {user_active_mode_trigger_instruction}")

        # *** STEP 3 ***
        # TH sends RegisterClient command with parameters: CheckInNodeID: <th_node_id>, MonitoredSubject: <th_node_id>,
        # and Key: <any_16_byte_octstr>. This resets the DUT's ActiveModeDuration timer and increases the ICDCounter.
        self.step(3)
        try:
            cmd = commands.RegisterClient(
                checkInNodeID=self.default_controller.nodeId,
                monitoredSubject=self.default_controller.nodeId,
                key=bytes.fromhex(os.urandom(16).hex()),
                clientType=clientTypeEnum.kPermanent)
            response = await self._send_single_icdm_command(cmd)
        except InteractionModelError as e:
            asserts.assert_fail(f"Unexpected error returned when registering client: {e}, command: {cmd}")

        icd_counter_at_registration = response.ICDCounter
        log.info(f"RegisterClient response ICDCounter: {icd_counter_at_registration}")

        # *** STEP 4 ***
        # Wait for ActiveModeDuration plus a 1-second buffer so DUT transitions from Active Mode to Idle Mode
        self.step(4)
        wait_time_for_idle_s = (active_mode_duration_ms / 1000.0) + 1.0
        log.info(f"Waiting {wait_time_for_idle_s}s for DUT to transition to Idle Mode...")
        await asyncio.sleep(wait_time_for_idle_s)

        # *** STEP 5 ***
        # Use UAT hint/instructions to transition DUT from Idle Mode to Active Mode.
        self.step(5)
        is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        # For CI, we use a generic trigger regardless of hint bit, so
        # iterating through each hint is unnecessary, limit to one
        # For real DUTs, test all available hints
        set_hints = uat_set_hints(user_active_mode_trigger_hint)
        hints_to_test = [set_hints[0]] if is_ci else set_hints

        previous_icd_counter = icd_counter_at_registration

        # Iterate through UAT hints
        for i, bit in enumerate(hints_to_test, start=1):
            # Get current hint name
            bit_name = uat_bit_name(bit)
            log.info(f"UAT hint {i}/{len(hints_to_test)}: {bit_name}...")

            # For the first hint, we don’t need to wait for the DUT to transition back to Idle Mode
            # because it’s already in Idle Mode from the previous step. For hints after the first,
            # wait for the DUT to transition back to Idle Mode.
            if i > 1:
                log.info(f"Waiting {wait_time_for_idle_s}s for DUT to transition to Idle Mode...")
                await asyncio.sleep(wait_time_for_idle_s)

            # Transition DUT from Idle Mode to Active Mode
            if is_ci:
                # Extra settling time so the lit-icd-app is fully in Idle Mode before the kAddActiveModeReq exchange
                # arrives and triggers a transition from Idle Mode to Active Mode (which sends the check-in)
                # NOTE: This wait value was derived empirically; it was enough for GitHub CI to pass reliably
                # given the CI environment and the hardware used for the app + controller.
                log.info(f"Waiting {MAX_CI_IDLE_CYCLE_WAIT_S}s for lit-icd-app to fully settle into Idle Mode...")
                await asyncio.sleep(MAX_CI_IDLE_CYCLE_WAIT_S)

                # Send AddActiveModeReq event trigger to transition lit-icd-app from Idle Mode to Active Mode
                await self.send_test_event_triggers(
                    eventTrigger=ICDTestEventTriggerOperations.kAddActiveModeReq)
            else:
                # In a real DUT scenario, we wait for user to perform the UAT
                self.wait_for_user_input(
                    prompt_msg=f" > \n"
                    f" > Perform UAT method '{bit_name}' on the DUT.\n"
                    f" > Follow UAT Instruction: {user_active_mode_trigger_instruction}\n"
                    f" > ")  # looks good in the logs, I promise

            # Verify the ICDCounter value increased
            current_icd_counter = await self._read_icdm_attribute_expect_success(attributes.ICDCounter)
            log.info(f"ICDCounter after '{bit_name}': {current_icd_counter} (was {previous_icd_counter})")
            asserts.assert_greater(current_icd_counter, previous_icd_counter,
                                   f"ICDCounter should have incremented after '{bit_name}'. Previous: {previous_icd_counter}, Current: {current_icd_counter}")

            # Verify the ActiveModeThreshold value is unchanged
            current_active_mode_threshold_ms = await self._read_icdm_attribute_expect_success(attributes.ActiveModeThreshold)
            log.info(
                f"ActiveModeThreshold after '{bit_name}': {current_active_mode_threshold_ms}ms (expected {active_mode_threshold_ms}ms)")
            asserts.assert_equal(current_active_mode_threshold_ms, active_mode_threshold_ms,
                                 f"ActiveModeThreshold value must be unchanged but changed after '{bit_name}'. Expected: {active_mode_threshold_ms}ms, Current: {current_active_mode_threshold_ms}ms")

            previous_icd_counter = current_icd_counter

        # *** STEP 6 ***
        # TH reads from the DUT the RegisteredClients attribute
        # If clients are registered, unregister them
        self.step(6)
        await self.unregister_all_clients()


if __name__ == "__main__":
    default_matter_test_main()
