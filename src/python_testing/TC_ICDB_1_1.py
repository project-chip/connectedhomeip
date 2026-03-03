#
#    Copyright (c) 2024 Project CHIP Authors
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
from support_modules.icd_support import ICDTestEventTriggerOperations

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
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
For a real DUT, the --timeout xxxx (in seconds) script argument can be used to extend
the testing time so the test does not time out due to long IdleModeDuration values
'''

kRootEndpointId = 0

# Safety cap so CI wait time does not hang the test
kMaxCIWaitTimeSeconds = 10

cluster = Clusters.Objects.IcdManagement
attributes = cluster.Attributes
commands = cluster.Commands
clientTypeEnum = cluster.Enums.ClientTypeEnum
features = cluster.Bitmaps.Feature


class TC_ICDB_1_1(MatterBaseTest):

    def desc_TC_ICDB_1_1(self) -> str:
        return "[TC-ICDB-1.1] ICD Check-In Protocol - Register client - idle mode duration [DUT as Server]"

    def steps_TC_ICDB_1_1(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep("1a", "TH reads from the DUT the RegisteredClients attribute. ",
                     "Check if RegisteredClients is empty, if not, TH sends command UnregisterClient to clear all clients in RegisteredClients by checkInNodeID."),
            TestStep("1b", "TH reads from the DUT the IdleModeDuration and ActiveModeDuration attributes",
                     "Store values as idle_mode_duration_s and active_mode_duration_ms."),
            TestStep("2", "TH sends RegisterClient command with parameters: CheckInNodeID: <any_node_id>, MonitoredSubject: <any_monitored_subject_id>, and Key: <any_16_byte_octstr>.", """
                        Verify DUT command response is successful (no exception).
                        Store ICDCounter value from response as icd_counter_at_registration."""),
            TestStep("3", "TH sends the RemoveActiveModeReq test event trigger. "
                     "Wait for DUT to transition to Idle Mode after active_mode_duration_ms."),
            TestStep("4", "Wait for at least one IdleModeDuration cycle."
                     "DUT sends a check-in message upon transitioning from Idle Mode to Active Mode."),
            TestStep("5", "TH sends the AddActiveModeReq test event trigger to hold DUT in active mode.",
                     "TH is able to read attributes from the DUT."),
            TestStep("6", "TH reads the current ICDCounter attribute from the DUT", """
                      Store ICDCounter value as current_icd_counter.
                      Verify that current_icd_counter is greater than icd_counter_at_registration."""),
            TestStep("7", "TH sends command UnregisterClient to the DUT",
                     "All clients in RegisteredClients are cleared, if any"),
        ]

    async def _read_icdm_attribute_expect_success(self, attribute):
        return await self.read_single_attribute_check_success(endpoint=kRootEndpointId, cluster=cluster, attribute=attribute)

    async def _send_single_icdm_command(self, command):
        return await self.send_single_cmd(command, endpoint=kRootEndpointId)

    async def unregister_all_clients(self, registeredClients=None):
        """Unregisters all entries in the DUT's `RegisteredClients` attribute.

        If `registeredClients` is provided, it is treated as the list of entries to
        unregister; otherwise, the method reads `RegisteredClients` from the DUT.
        """
        if registeredClients is None:
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

        # *** STEP 1a ***
        # TH reads from the DUT the RegisteredClients attribute
        self.step("1a")
        registeredClients = await self._read_icdm_attribute_expect_success(attributes.RegisteredClients)

        # Check if RegisteredClients is empty
        # If empty, move on to the next step, if not empty, unregister all clients
        await self.unregister_all_clients(registeredClients)

        # *** STEP 1b ***
        # TH reads from the DUT the IdleModeDuration and ActiveModeDuration attributes
        self.step("1b")
        idle_mode_duration_s = await self._read_icdm_attribute_expect_success(attributes.IdleModeDuration)
        active_mode_duration_ms = await self._read_icdm_attribute_expect_success(attributes.ActiveModeDuration)
        log.info(f"IdleModeDuration: {idle_mode_duration_s}s, ActiveModeDuration: {active_mode_duration_ms}ms")

        # *** STEP 2 ***
        # TH sends RegisterClient command
        self.step("2")
        try:
            cmd = commands.RegisterClient(
                checkInNodeID=self.default_controller.nodeId,
                monitoredSubject=self.default_controller.nodeId,
                key=bytes.fromhex(os.urandom(16).hex()),
                clientType=clientTypeEnum.kPermanent)
            response = await self._send_single_icdm_command(cmd)
        # Verify DUT command response is successful (no exception)
        except InteractionModelError as e:
            asserts.assert_fail(f"Unexpected error returned when registering client: {e}, command: {cmd}")

        icd_counter_at_registration = response.ICDCounter
        log.info(f"RegisterClient response ICDCounter: {icd_counter_at_registration}")

        # *** STEP 3 ***
        # TH sends the RemoveActiveModeReq test event trigger
        self.step("3")
        await self.check_test_event_triggers_enabled()
        await self.send_test_event_triggers(
            eventTrigger=ICDTestEventTriggerOperations.kRemoveActiveModeReq)

        # Wait for DUT to transition to Idle Mode after active_mode_duration_ms
        wait_time_for_idle_s = (active_mode_duration_ms / 1000.0) + 1.0
        log.info(f"Waiting {wait_time_for_idle_s}s for DUT to transition to Idle Mode...")
        await asyncio.sleep(wait_time_for_idle_s)

        # *** STEP 4 ***
        # Wait for at least one IdleModeDuration cycle.
        # DUT sends a check-in message upon transitioning from Idle Mode to Active Mode.
        self.step("4")
        is_ci = self.check_pics("PICS_SDK_CI_ONLY")
        if is_ci:
            # In CI, the test app internally cycles faster than its reported IdleModeDuration,
            # so we cap the wait at kMaxCIWaitTimeSeconds to avoid long test runs.
            wait_time_for_checkin_s = kMaxCIWaitTimeSeconds
        else:
            # On a real DUT, the reported IdleModeDuration should match the actual timer,
            # so we wait the full duration to ensure the check-in is received.
            wait_time_for_checkin_s = idle_mode_duration_s + 1.0

        log.info(f"Waiting {wait_time_for_checkin_s}s for DUT to complete idle cycle and send check-in "
                 f"({'CI mode' if is_ci else 'real DUT mode'}, IdleModeDuration: {idle_mode_duration_s}s)...")
        await asyncio.sleep(wait_time_for_checkin_s)

        # *** STEP 5 ***
        # TH sends AddActiveModeReq to hold DUT in active mode.
        # TH is able to read attributes from the DUT
        self.step("5")
        await self.send_test_event_triggers(
            eventTrigger=ICDTestEventTriggerOperations.kAddActiveModeReq)

        # *** STEP 6 ***
        # TH reads the current ICDCounter attribute from the DUT
        self.step("6")
        current_icd_counter = await self._read_icdm_attribute_expect_success(attributes.ICDCounter)
        log.info(f"ICDCounter after idle cycle: {current_icd_counter} (was {icd_counter_at_registration} at registration)")

        # Verify current ICDCounter is greater than ICDCounter at registration
        asserts.assert_greater(
            current_icd_counter, icd_counter_at_registration,
            "ICDCounter should have changed after IdleModeDuration cycle. "
            "Registration ICDCounter: {icd_counter_at_registration}, Current: {current_icd_counter}")

        # *** STEP 7 ***
        # TH sends command UnregisterClient to the DUT
        # All clients in RegisteredClients are cleared, if any
        self.step("7")
        await self.unregister_all_clients()


if __name__ == "__main__":
    default_matter_test_main()
