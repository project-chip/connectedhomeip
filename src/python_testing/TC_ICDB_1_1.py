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
#       --icdIdleModeDuration 10
#       --icdActiveModeDurationMs 10000
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

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError
from matter.testing.decorators import async_test_body
from support_modules.icd_support import ICDBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

'''
Purpose
Test validates that an ICD will send Check-In messages when entering Active mode if no subscription is active.
This test validates device enters active mode after idle mode duration.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/icdbehavior.adoc#tc-icdb-1-1-icd-check-in-protocol-register-client-idle-mode-duration-dut_server
'''

'''
Notes/Considerations
In CI, --icdIdleModeDuration is set to a short value (10s) so the DUT naturally wakes up quickly.
On a real DUT, the full IdleModeDuration is used.
On a real DUT, use the --timeout <seconds> script argument if wait times are large to prevent the test from timing out.
'''

ROOT_ENDPOINT_ID = 0

cluster = Clusters.Objects.IcdManagement
attributes = cluster.Attributes
commands = cluster.Commands
clientTypeEnum = cluster.Enums.ClientTypeEnum
features = cluster.Bitmaps.Feature


class TC_ICDB_1_1(ICDBaseTest):

    def desc_TC_ICDB_1_1(self) -> str:
        return "[TC-ICDB-1.1] ICD Check-In Protocol - Register client - idle mode duration [DUT as Server]"

    def steps_TC_ICDB_1_1(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads from the DUT the RegisteredClients attribute.",
                     "If clients are registered, unregister them."),
            TestStep(2, "TH reads from the DUT the IdleModeDuration, ActiveModeDuration, and ActiveModeThreshold attributes.",
                     "Store all values for later use."),
            TestStep(3, "TH sends RegisterClient command with parameters: CheckInNodeID: <th_node_id>, MonitoredSubject: <th_node_id>, and Key: <any_16_byte_octstr>.", """
                     DUT ActiveModeDuration timer is reset.
                     DUT responds with an ICDCounter value, store the value as icd_counter_at_registration."""),
            TestStep(4, "Wait for ActiveModeDuration plus a 1-second buffer.",
                     "DUT transitions from Active Mode to Idle Mode."),
            TestStep(5, "Wait for at least one IdleModeDuration cycle.", """
                     DUT sends a check-in message upon transitioning from Idle Mode to Active Mode.
                     Verify the ICDCounter value increased.
                     Verify the ActiveModeThreshold value is unchanged."""),
            TestStep(6, "TH reads from the DUT the RegisteredClients attribute.",
                     "If clients are registered, unregister them."),
        ]

    async def _read_icdm_attribute_expect_success(self, attribute):
        return await self.read_single_attribute_check_success(endpoint=self.ROOT_NODE_ENDPOINT_ID, cluster=cluster, attribute=attribute)

    async def _send_single_icdm_command(self, command):
        return await self.send_single_cmd(command, endpoint=self.ROOT_NODE_ENDPOINT_ID)

    def pics_TC_ICDB_1_1(self) -> list[str]:
        return [
            "ICDB.S",
            "ICDM.S.F00",
        ]

    @async_test_body
    async def test_TC_ICDB_1_1(self):

        # *** PRECONDITION ***
        # Commissioning, already done
        self.step("precondition")

        # *** STEP 1 ***
        # TH reads from the DUT the RegisteredClients attribute
        # If clients are registered, unregister them
        self.step(1)
        await self.unregister_all_clients()

        # *** STEP 2 ***
        # TH reads from the DUT the IdleModeDuration, ActiveModeDuration, and ActiveModeThreshold attributes
        self.step(2)
        idle_mode_duration_s = await self._read_icdm_attribute_expect_success(attributes.IdleModeDuration)
        active_mode_duration_ms = await self._read_icdm_attribute_expect_success(attributes.ActiveModeDuration)
        active_mode_threshold_ms = await self._read_icdm_attribute_expect_success(attributes.ActiveModeThreshold)
        log.info(f"IdleModeDuration: {idle_mode_duration_s}s")
        log.info(f"ActiveModeDuration: {active_mode_duration_ms}ms")
        log.info(f"ActiveModeThreshold: {active_mode_threshold_ms}ms")

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
        # Wait for at least one IdleModeDuration cycle.
        # DUT sends a check-in message upon transitioning from Idle Mode to Active Mode.
        self.step(5)
        # Wait for the full IdleModeDuration so the DUT naturally
        # transitions from Idle Mode to Active Mode and sends its check-in message
        # In CI, --icdIdleModeDuration is set to a short value (10s) so this completes quickly.
        wait_time_for_checkin_s = idle_mode_duration_s + 1.0
        log.info(f"Waiting {wait_time_for_checkin_s}s for DUT to complete idle cycle (IdleModeDuration: {idle_mode_duration_s}s)...")
        await asyncio.sleep(wait_time_for_checkin_s)

        # Verify the ICDCounter value increased
        current_icd_counter = await self._read_icdm_attribute_expect_success(attributes.ICDCounter)
        log.info(f"ICDCounter after idle cycle: {current_icd_counter} (was {icd_counter_at_registration})")
        asserts.assert_greater(current_icd_counter, icd_counter_at_registration,
                               f"ICDCounter should have incremented after idle cycle. Previous: {icd_counter_at_registration}, Current: {current_icd_counter}")

        # Verify the ActiveModeThreshold value is unchanged
        current_active_mode_threshold_ms = await self._read_icdm_attribute_expect_success(attributes.ActiveModeThreshold)
        log.info(
            f"ActiveModeThreshold after idle cycle: {current_active_mode_threshold_ms}ms (expected {active_mode_threshold_ms}ms)")
        asserts.assert_equal(current_active_mode_threshold_ms, active_mode_threshold_ms,
                             f"ActiveModeThreshold value must be unchanged but changed after idle cycle. Expected: {active_mode_threshold_ms}ms, Current: {current_active_mode_threshold_ms}ms")

        # *** STEP 6 ***
        # TH reads from the DUT the RegisteredClients attribute
        # If clients are registered, unregister them
        self.step(6)
        await self.unregister_all_clients()


if __name__ == "__main__":
    default_matter_test_main()
