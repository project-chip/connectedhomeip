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
Verify that after client registration and subscription request, the ICD state machine enters the
subscribed state and stops sending check-in messages for the duration of the subscription.
After the subscription is torn down, the DUT resumes sending check-in messages.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/icdbehavior.adoc#322-tc-icdb-22-icd-state-machine---with-client-registration-and-active-subscription---single-fabric-dut_server

Notes/Considerations
In CI:
    --icdIdleModeDuration is set to 10s so the DUT naturally cycles quickly.
    --icdActiveModeDurationMs is set to 10000ms to extend the Active Mode window, giving the
      test enough time to establish a subscription before the DUT returns to Idle.
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


class TC_ICDB_2_2(ICDBaseTest):

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

    def desc_TC_ICDB_2_2(self) -> str:
        return "[TC-ICDB-2.2] ICD State Machine - With client registration and active subscription - Single Fabric [DUT as Server]"

    def steps_TC_ICDB_2_2(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning with ICD client registration."),
            TestStep(1, "TH reads from the DUT the RegisteredClients attribute.", """
                     Verify exactly one RegisteredClients entry is present.
                     Verify that the RegisteredClients entry's checkInNodeID and monitoredSubject match TH's node ID."""),
            TestStep(2, "TH reads from the DUT the IdleModeDuration and ActiveModeDuration attributes.",
                     "Store values for later use."),
            TestStep(3, "TH subscribes to the ICDCounter attribute with MinIntervalFloor=0 and MaxIntervalCeiling=IdleModeDuration. TH reads the current ICDCounter value.", """
                     Subscription is established successfully.
                     Store the ICDCounter value as icd_counter_at_subscription."""),
            TestStep(4, "Wait for a full active-to-idle-to-active ICD transition cycle.",
                     "No check-in message is sent."),
            TestStep(5, "TH reads the ICDCounter attribute.",
                     "Verify ICDCounter is unchanged from icd_counter_at_subscription."),
            TestStep(6, "TH shuts down the subscription. Wait for a full active-to-idle-to-active ICD transition cycle.", """
                     DUT resumed sending check-in messages after the subscription was torn down.
                     Verify ICDCounter is greater than icd_counter_at_subscription."""),
        ]

    def pics_TC_ICDB_2_2(self) -> list[str]:
        return [
            "ICDB.S",
            "ICDM.S.F00",
        ]

    @async_test_body
    async def test_TC_ICDB_2_2(self):
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
        # TH subscribes to the ICDCounter attribute with MinIntervalFloor=0 and MaxIntervalCeiling=IdleModeDuration
        self.step(3)
        subscription = await TH.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[(self.ROOT_NODE_ENDPOINT_ID, attributes.ICDCounter)],
            reportInterval=(0, idle_mode_duration_s),
            keepSubscriptions=False,
            autoResubscribe=False
        )
        log.info(f"Subscription established. subscriptionId={subscription.subscriptionId}")

        # TH reads the current ICDCounter value
        #   - Store the ICDCounter value as icd_counter_at_subscription
        icd_counter_at_subscription = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)
        log.info(f"ICDCounter at subscription: {icd_counter_at_subscription}")

        # *** STEP 4 ***
        # Wait for a full active-to-idle-to-active ICD transition cycle
        #   - No check-in message is sent.
        self.step(4)
        await self.wait_for_transition(ICDTransition.FullCycle,
                                       active_mode_duration_ms=active_mode_duration_ms,
                                       idle_mode_duration_s=idle_mode_duration_s)

        # *** STEP 5 ***
        # TH reads the ICDCounter attribute
        self.step(5)
        icd_counter_while_subscribed = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)
        log.info(f"ICDCounter while subscribed: {icd_counter_while_subscribed}")

        # Verify ICDCounter is unchanged from icd_counter_at_subscription
        asserts.assert_equal(icd_counter_while_subscribed, icd_counter_at_subscription,
                             f"ICDCounter must not increment while the subscription is active. "
                             f"DUT should not send check-in messages while subscribed. "
                             f"Before: {icd_counter_at_subscription}, After: {icd_counter_while_subscribed}")

        # *** STEP 6 ***
        # TH shuts down the subscription. Wait for a full active-to-idle-to-active ICD transition cycle.
        #   - DUT resumed sending check-in messages after the subscription was torn down.
        self.step(6)
        subscription.Shutdown()
        log.info("Subscription shut down.")
        await self.wait_for_transition(ICDTransition.FullCycle,
                                       active_mode_duration_ms=active_mode_duration_ms,
                                       idle_mode_duration_s=idle_mode_duration_s)

        # Verify ICDCounter is greater than icd_counter_at_subscription
        icd_counter_after_shutdown = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)
        asserts.assert_greater(icd_counter_after_shutdown, icd_counter_at_subscription,
                               f"ICDCounter must increment after subscription shutdown (check-in resumed). "
                               f"Before: {icd_counter_at_subscription}, After: {icd_counter_after_shutdown}")


if __name__ == "__main__":
    default_matter_test_main()
