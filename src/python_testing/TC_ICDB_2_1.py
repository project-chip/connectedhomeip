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
#       --icdIdleModeDuration 10
#       --icdActiveModeDurationMs 10000
#     script-args: >
#       --storage-path admin_storage.json
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

'''
Purpose
Verify that after client registration, the ICD state machine enters the check-in state and
periodically sends check-in messages if there are no active subscriptions. After the client
unregisters, the DUT stops sending check-in messages.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/icdbehavior.adoc#321-tc-icdb-21-icd-state-machine---with-client-registration-and-no-active-subscription---single-fabric-dut_server

Notes/Considerations
In CI:
    --icdIdleModeDuration is set to 10s so the DUT naturally cycles quickly.
    --icdActiveModeDurationMs is set to 10000ms to extend the Active Mode window, giving the
      test enough time to send commands before the DUT returns to Idle.
On a real DUT:
    The full IdleModeDuration and ActiveModeDuration values are used.
    Use --timeout <seconds> in the script-args if wait times are expected to be large.
'''

import logging

from mobly import asserts
from support_modules.icd_support import ICDBaseTest, ICDTransition

import matter.clusters as Clusters
from matter.testing.commissioning import get_setup_payload_info_config
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.Objects.IcdManagement
attributes = cluster.Attributes
commands = cluster.Commands


class TC_ICDB_2_1(ICDBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()
        TH = self.default_controller

        # Get setup payload info
        setup_payload_info = get_setup_payload_info_config(self.matter_test_config)
        info = setup_payload_info[0]

        # Enable ICD registration on the TH
        TH.EnableICDRegistration(TH.GenerateICDRegistrationParameters())

        # Commission the DUT
        await TH.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=info.passcode,
            filterType=info.filter_type,
            filter=info.filter_value
        )

        # Save ICDCounter value for later use
        self.icd_counter_at_commissioning = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)
        log.info(f"ICDCounter at commissioning: {self.icd_counter_at_commissioning}")

    def desc_TC_ICDB_2_1(self) -> str:
        return "[TC-ICDB-2.1] ICD State Machine - With client registration and no active subscription - Single Fabric [DUT as Server]"

    def steps_TC_ICDB_2_1(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning with ICD client registration."),
            TestStep(1, "TH reads from the DUT the RegisteredClients attribute.", """
                     Verify exactly one RegisteredClients entry is present.
                     Verify that the RegisteredClients entry's checkInNodeID and monitoredSubject match TH's node ID."""),
            TestStep(2, "TH reads from the DUT the IdleModeDuration and ActiveModeDuration attributes.",
                     "Store values for later use."),
            TestStep(3, "Wait for a full active-to-idle-to-active ICD transition cycle.", """
                     Verify current ICDCounter is greater than the ICDCounter after commissioning."""),
            TestStep(4, "TH sends the UnregisterClient command to the DUT with the checkInNodeID from Step 1.",
                     "TH is no longer registered as a client on the DUT."),
            TestStep(5, "Wait for a full active-to-idle-to-active ICD transition cycle.", """
                     No check-in message is sent.
                     Verify ICDCounter is unchanged from the value read after Step 3."""),
        ]

    def pics_TC_ICDB_2_1(self) -> list[str]:
        return [
            "ICDB.S",
            "ICDM.S.F00",
        ]

    @async_test_body
    async def test_TC_ICDB_2_1(self):
        TH = self.default_controller

        # Precondition: Commissioning with ICD client registration.
        # setup_class handles this — DUT's RegisteredClients already contains TH's entry.
        self.step("precondition")

        # *** STEP 1 ***
        # TH reads from the DUT the RegisteredClients attribute
        self.step(1)
        registered_clients = await self.read_icdm_attribute_expect_success(attributes.RegisteredClients)
        log.info(f"RegisteredClients: {registered_clients}")

        # Verify exactly one RegisteredClients entry is present
        asserts.assert_equal(len(registered_clients), 1,
                             "Expected exactly 1 registered client. TH should have registered during commissioning")

        # Verify that the RegisteredClients entry's checkInNodeID and monitoredSubject match TH's node ID
        rc_entry = registered_clients[0]
        log.info(
            f"Registered ICD Client entry: checkInNodeID={rc_entry.checkInNodeID}, monitoredSubject={rc_entry.monitoredSubject}")
        asserts.assert_equal(rc_entry.checkInNodeID, TH.nodeId,
                             f"checkInNodeID must match TH node ID. Expected: {TH.nodeId}, Got: {rc_entry.checkInNodeID}")
        asserts.assert_equal(rc_entry.monitoredSubject, TH.nodeId,
                             f"monitoredSubject must match TH node ID. Expected: {TH.nodeId}, Got: {rc_entry.monitoredSubject}")

        # *** STEP 2 ***
        # TH reads from the DUT the IdleModeDuration and ActiveModeDuration attributes
        #   - Store values for later use
        self.step(2)
        idle_mode_duration_s = await self.read_icdm_attribute_expect_success(attributes.IdleModeDuration)
        active_mode_duration_ms = await self.read_icdm_attribute_expect_success(attributes.ActiveModeDuration)
        log.info(f"IdleModeDuration: {idle_mode_duration_s}s")
        log.info(f"ActiveModeDuration: {active_mode_duration_ms}ms")

        # *** STEP 3 ***
        # Wait for a full active-to-idle-to-active ICD transition cycle
        self.step(3)
        await self.wait_for_transition(ICDTransition.FullCycle,
                                       active_mode_duration_ms=active_mode_duration_ms,
                                       idle_mode_duration_s=idle_mode_duration_s)

        # Verify current ICDCounter is greater than the ICDCounter after commissioning
        icd_counter_after_idle = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)
        asserts.assert_greater(icd_counter_after_idle, self.icd_counter_at_commissioning,
                               f"ICDCounter should have incremented after idle cycle (check-in sent). "
                               f"Previous: {self.icd_counter_at_commissioning}, Current: {icd_counter_after_idle}")

        # *** STEP 4 ***
        # TH sends the UnregisterClient command to the DUT with the checkInNodeID from Step 1
        self.step(4)
        await self.send_single_icdm_command(commands.UnregisterClient(checkInNodeID=rc_entry.checkInNodeID))
        log.info(f"UnregisterClient SUCCESS for checkInNodeID={rc_entry.checkInNodeID}")

        # *** STEP 5 ***
        # Wait for a full active-to-idle-to-active ICD transition cycle
        #   - No check-in message is sent.
        self.step(5)
        await self.wait_for_transition(ICDTransition.FullCycle,
                                       active_mode_duration_ms=active_mode_duration_ms,
                                       idle_mode_duration_s=idle_mode_duration_s)

        # Verify ICDCounter is unchanged from the value read after Step 3.
        icd_counter_no_checkin = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)
        asserts.assert_equal(icd_counter_no_checkin, icd_counter_after_idle,
                             f"ICDCounter must not increment after unregistering. DUT should not send check-in messages. "
                             f"Before: {icd_counter_after_idle}, After: {icd_counter_no_checkin}")


if __name__ == "__main__":
    default_matter_test_main()
