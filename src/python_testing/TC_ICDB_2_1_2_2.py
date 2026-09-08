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

import asyncio
import logging
import os

from mobly import asserts
from support_modules.icd_support import ICDBaseTest, ICDTransition

import matter.clusters as Clusters
from matter.testing.commissioning import CommissioningInfo, commission_device, get_setup_payload_info_config
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.Objects.IcdManagement
attributes = cluster.Attributes
commands = cluster.Commands
ClientTypeEnum = cluster.Enums.ClientTypeEnum


class TC_ICDB_2_1_2_2(ICDBaseTest):

    # DUT can take more than one cycle to detect a dropped subscriber and resume check-ins, default_timeout
    # is raised to accommodate that (can be overridden by a --timeout on the command line)
    @property
    def default_timeout(self) -> int:
        return 5 * 60

    @async_test_body
    async def setup_class(self):
        super().setup_class()
        TH = self.default_controller

        setup_payload_info = get_setup_payload_info_config(self.matter_test_config)
        info = setup_payload_info[0]

        commissioning_info = CommissioningInfo(
            commissionee_ip_address_just_for_testing=self.matter_test_config.commissionee_ip_address_just_for_testing,
            commissioning_method=self.matter_test_config.commissioning_method or "on-network",
            thread_operational_dataset=self.matter_test_config.thread_operational_dataset,
            wifi_passphrase=self.matter_test_config.wifi_passphrase,
            wifi_ssid=self.matter_test_config.wifi_ssid,
            tc_version_to_simulate=self.matter_test_config.tc_version_to_simulate,
            tc_user_response_to_simulate=self.matter_test_config.tc_user_response_to_simulate,
            thread_ba_host=self.matter_test_config.thread_ba_host,
            thread_ba_port=self.matter_test_config.thread_ba_port,
        )
        status = await commission_device(TH, self.dut_node_id, info, commissioning_info)
        asserts.assert_true(status, f"Failed to commission DUT to TH's fabric: {status}")

    # ============================================================================
    # TC-ICDB-2.1
    # Verify that after client registration, ICD state machine enters check-in state
    # and periodically sends check-in message if there is no active subscription
    # session presences.
    # ============================================================================

    def desc_TC_ICDB_2_1(self) -> str:
        return "[TC-ICDB-2.1] ICD State Machine - With client registration and no active subscription - Single Fabric [DUT as Server]"

    def steps_TC_ICDB_2_1(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT is commissioned to TH."),
            TestStep(1, "TH sends the RegisterClient command to the DUT with TH's node ID as checkInNodeID and monitoredSubject.",
                     "TH is registered as an ICD client on the DUT."),
            TestStep(2, "TH reads from the DUT the RegisteredClients attribute.", """
                     Verify exactly one RegisteredClients entry is present.
                     Verify that the RegisteredClients entry's checkInNodeID and monitoredSubject match TH's node ID."""),
            TestStep(3, "TH reads from the DUT the IdleModeDuration and ActiveModeDuration attributes.",
                     "Store values for later use."),
            TestStep(4, "Wait for a full active-to-idle-to-active ICD transition cycle (IdleModeDuration + ActiveModeDuration).", """
                     Verify current ICDCounter is greater than the ICDCounter after registration, indicating a check-in message was sent."""),
            TestStep(5, "TH sends the UnregisterClient command to the DUT with the checkInNodeID from Step 2. TH reads the RegisteredClients attribute.", """
                     TH is no longer registered as a client on the DUT.
                     Verify the RegisteredClients attribute no longer contains TH's checkInNodeID."""),
            TestStep(6, "Wait for a full active-to-idle-to-active ICD transition cycle (IdleModeDuration + ActiveModeDuration).", """
                     No check-in message is sent.
                     Verify ICDCounter is unchanged from the value read after Step 4. An unchanged counter confirms no check-in was sent."""),
        ]

    def pics_TC_ICDB_2_1(self) -> list[str]:
        return [
            "ICDB.S",
            "ICDM.S.F00",
        ]

    @async_test_body
    async def test_TC_ICDB_2_1(self):
        TH = self.default_controller

        # DUT is commissioned to TH.
        self.step("precondition")

        # *** STEP 1 ***
        # TH sends the RegisterClient command to the DUT with TH's node ID as checkInNodeID and monitoredSubject
        self.step(1)
        checkin_key = os.urandom(16)
        await self.send_single_icdm_command(commands.RegisterClient(
            checkInNodeID=TH.nodeId,
            monitoredSubject=TH.nodeId,
            key=checkin_key,
            clientType=ClientTypeEnum.kPermanent,
        ))
        log.info("RegisterClient SUCCESS for checkInNodeID=%s", TH.nodeId)

        icd_counter_at_registration = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)
        log.info("ICDCounter at registration: %s", icd_counter_at_registration)

        # *** STEP 2 ***
        # TH reads from the DUT the RegisteredClients attribute
        self.step(2)
        registered_clients = await self.read_icdm_attribute_expect_success(attributes.RegisteredClients)
        log.info("RegisteredClients: %s", registered_clients)

        # Verify exactly one RegisteredClients entry is present
        asserts.assert_equal(len(registered_clients), 1,
                             "Expected exactly 1 registered client. TH should have registered in Step 1")

        # Verify that the RegisteredClients entry's checkInNodeID and monitoredSubject match TH's node ID
        rc_entry = registered_clients[0]
        log.info(
            "Registered ICD Client entry: checkInNodeID=%s, monitoredSubject=%s", rc_entry.checkInNodeID, rc_entry.monitoredSubject)
        asserts.assert_equal(rc_entry.checkInNodeID, TH.nodeId,
                             f"checkInNodeID must match TH node ID. Expected: {TH.nodeId}, Got: {rc_entry.checkInNodeID}")
        asserts.assert_equal(rc_entry.monitoredSubject, TH.nodeId,
                             f"monitoredSubject must match TH node ID. Expected: {TH.nodeId}, Got: {rc_entry.monitoredSubject}")

        # *** STEP 3 ***
        # TH reads from the DUT the IdleModeDuration and ActiveModeDuration attributes
        #   - Store values for later use
        self.step(3)
        idle_mode_duration_s = await self.read_icdm_attribute_expect_success(attributes.IdleModeDuration)
        active_mode_duration_ms = await self.read_icdm_attribute_expect_success(attributes.ActiveModeDuration)
        log.info("IdleModeDuration: %ss", idle_mode_duration_s)
        log.info("ActiveModeDuration: %sms", active_mode_duration_ms)

        # *** STEP 4 ***
        # Wait for a full active-to-idle-to-active ICD transition cycle (IdleModeDuration + ActiveModeDuration)
        self.step(4)
        await self.wait_for_transition(ICDTransition.FullCycle,
                                       active_mode_duration_ms=active_mode_duration_ms,
                                       idle_mode_duration_s=idle_mode_duration_s)

        # Verify current ICDCounter is greater than the ICDCounter after registration, indicating a check-in was sent
        icd_counter_after_idle = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)
        asserts.assert_greater(icd_counter_after_idle, icd_counter_at_registration,
                               f"ICDCounter should have incremented after idle cycle (check-in sent). "
                               f"Previous: {icd_counter_at_registration}, Current: {icd_counter_after_idle}")

        # *** STEP 5 ***
        # TH sends the UnregisterClient command to the DUT with the checkInNodeID from Step 2
        self.step(5)
        await self.send_single_icdm_command(commands.UnregisterClient(checkInNodeID=rc_entry.checkInNodeID))
        log.info("UnregisterClient SUCCESS for checkInNodeID=%s", rc_entry.checkInNodeID)

        # Verify the RegisteredClients attribute no longer contains TH's checkInNodeID
        registered_clients_after = await self.read_icdm_attribute_expect_success(attributes.RegisteredClients)
        remaining_ids = [c.checkInNodeID for c in registered_clients_after]
        asserts.assert_not_in(TH.nodeId, remaining_ids,
                              f"TH's checkInNodeID {TH.nodeId} should have been removed from RegisteredClients")

        # *** STEP 6 ***
        # Wait for a full active-to-idle-to-active ICD transition cycle (IdleModeDuration + ActiveModeDuration)
        #   - No check-in message is sent. Confirmed by unchanged ICDCounter.
        self.step(6)
        await self.wait_for_transition(ICDTransition.FullCycle,
                                       active_mode_duration_ms=active_mode_duration_ms,
                                       idle_mode_duration_s=idle_mode_duration_s)

        # Verify ICDCounter is unchanged from the value read after Step 4.
        icd_counter_no_checkin = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)
        asserts.assert_equal(icd_counter_no_checkin, icd_counter_after_idle,
                             f"ICDCounter must not increment after unregistering. DUT should not send check-in messages. "
                             f"Before: {icd_counter_after_idle}, After: {icd_counter_no_checkin}")

    # ============================================================================
    # TC-ICDB-2.2
    # Verify that after client registration and subscription request, the ICD state
    # machine enters the subscribed state, periodically sends subscription reports,
    # and stops sending check-in messages for the duration of the subscription.
    # After the subscription is torn down, the DUT resumes sending check-in messages.
    # ============================================================================

    def desc_TC_ICDB_2_2(self) -> str:
        return "[TC-ICDB-2.2] ICD State Machine - With client registration and active subscription - Single Fabric [DUT as Server]"

    def steps_TC_ICDB_2_2(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT is commissioned to TH."),
            TestStep(1, "TH sends the RegisterClient command to the DUT with TH's node ID as checkInNodeID and monitoredSubject.",
                     "TH is registered as an ICD client on the DUT."),
            TestStep(2, "TH reads from the DUT the RegisteredClients attribute.", """
                     Verify exactly one RegisteredClients entry is present.
                     Verify that the RegisteredClients entry's checkInNodeID and monitoredSubject match TH's node ID."""),
            TestStep(3, "TH reads from the DUT the IdleModeDuration and ActiveModeDuration attributes.",
                     "Store values for later use."),
            TestStep(4, "TH subscribes to the NodeLabel attribute with MinIntervalFloor=0 and MaxIntervalCeiling=IdleModeDuration. TH reads the current ICDCounter value.", """
                     Subscription is established successfully.
                     Store the ICDCounter value as icd_counter_at_subscription."""),
            TestStep(5, "Wait for a full active-to-idle-to-active ICD transition cycle (IdleModeDuration + ActiveModeDuration).",
                     "No check-in message is sent. Confirmed by unchanged ICDCounter in step 6."),
            TestStep(6, "TH reads the ICDCounter attribute.",
                     "Verify ICDCounter is unchanged from icd_counter_at_subscription. An unchanged counter confirms no check-in was sent."),
            TestStep(7, "Wait for a full active-to-idle-to-active ICD transition cycle (IdleModeDuration + ActiveModeDuration). TH writes a new value to the NodeLabel attribute. Verify the existing subscription reports the change.",
                     "The subscription receives a report for the NodeLabel attribute within MaxInterval."),
            TestStep(8, "TH shuts down the subscription, then waits for the DUT to detect the dropped subscriber and resume check-ins.", """
                     DUT resumes sending check-in messages once it detects the subscription is gone.
                     Verify ICDCounter increments beyond icd_counter_at_subscription, indicating a check-in was sent."""),
        ]

    def pics_TC_ICDB_2_2(self) -> list[str]:
        return [
            "ICDB.S",
            "ICDM.S.F00",
        ]

    @async_test_body
    async def test_TC_ICDB_2_2(self):
        TH = self.default_controller

        # DUT is commissioned to TH.
        self.step("precondition")

        # *** STEP 1 ***
        # TH sends the RegisterClient command to the DUT with TH's node ID as checkInNodeID and monitoredSubject
        self.step(1)
        checkin_key = os.urandom(16)
        await self.send_single_icdm_command(commands.RegisterClient(
            checkInNodeID=TH.nodeId,
            monitoredSubject=TH.nodeId,
            key=checkin_key,
            clientType=ClientTypeEnum.kPermanent,
        ))
        log.info("RegisterClient SUCCESS for checkInNodeID=%s", TH.nodeId)

        # *** STEP 2 ***
        # TH reads from the DUT the RegisteredClients attribute
        self.step(2)
        registered_clients = await self.read_icdm_attribute_expect_success(attributes.RegisteredClients)
        log.info("RegisteredClients: %s", registered_clients)

        # Verify exactly one RegisteredClients entry is present
        asserts.assert_equal(len(registered_clients), 1,
                             "Expected exactly 1 registered client. TH should have registered in Step 1")

        # Verify that the RegisteredClients entry's checkInNodeID and monitoredSubject match TH's node ID
        rc_entry = registered_clients[0]
        log.info(
            "Registered ICD Client entry: checkInNodeID=%s, monitoredSubject=%s", rc_entry.checkInNodeID, rc_entry.monitoredSubject)
        asserts.assert_equal(rc_entry.checkInNodeID, TH.nodeId,
                             f"checkInNodeID must match TH node ID. Expected: {TH.nodeId}, Got: {rc_entry.checkInNodeID}")
        asserts.assert_equal(rc_entry.monitoredSubject, TH.nodeId,
                             f"monitoredSubject must match TH node ID. Expected: {TH.nodeId}, Got: {rc_entry.monitoredSubject}")

        # *** STEP 3 ***
        # TH reads from the DUT the IdleModeDuration and ActiveModeDuration attributes
        #   - Store values for later use
        self.step(3)
        idle_mode_duration_s = await self.read_icdm_attribute_expect_success(attributes.IdleModeDuration)
        active_mode_duration_ms = await self.read_icdm_attribute_expect_success(attributes.ActiveModeDuration)
        log.info("IdleModeDuration: %ss", idle_mode_duration_s)
        log.info("ActiveModeDuration: %sms", active_mode_duration_ms)

        # *** STEP 4 ***
        # TH subscribes to the NodeLabel attribute with MinIntervalFloor=0 and MaxIntervalCeiling=IdleModeDuration.
        # NodeLabel is used so the same subscription can verify report delivery in step 7 without re-subscribing.
        self.step(4)
        subscription = await TH.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[(0, Clusters.BasicInformation.Attributes.NodeLabel)],
            reportInterval=(0, idle_mode_duration_s),
            keepSubscriptions=False,
            autoResubscribe=False
        )
        _, max_interval_s = subscription.GetReportingIntervalsSeconds()
        log.info("Subscription established. subscriptionId=%s, MaxInterval=%ss", subscription.subscriptionId, max_interval_s)

        # TH reads the current ICDCounter value
        #   - Store the ICDCounter value as icd_counter_at_subscription
        icd_counter_at_subscription = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)
        log.info("ICDCounter at subscription: %s", icd_counter_at_subscription)

        # *** STEP 5 ***
        # Wait for a full active-to-idle-to-active ICD transition cycle (IdleModeDuration + ActiveModeDuration)
        #   - No check-in message is sent. Confirmed by unchanged ICDCounter in step 6.
        self.step(5)
        await self.wait_for_transition(ICDTransition.FullCycle,
                                       active_mode_duration_ms=active_mode_duration_ms,
                                       idle_mode_duration_s=idle_mode_duration_s)

        # *** STEP 6 ***
        # TH reads the ICDCounter attribute
        self.step(6)
        icd_counter_while_subscribed = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)
        log.info("ICDCounter while subscribed: %s", icd_counter_while_subscribed)

        # Verify ICDCounter is unchanged from icd_counter_at_subscription. An unchanged counter confirms no check-in was sent.
        asserts.assert_equal(icd_counter_while_subscribed, icd_counter_at_subscription,
                             f"ICDCounter must not increment while the subscription is active. "
                             f"DUT should not send check-in messages while subscribed. "
                             f"Before: {icd_counter_at_subscription}, After: {icd_counter_while_subscribed}")

        # *** STEP 7 ***
        # Wait for a full active-to-idle-to-active ICD transition cycle (IdleModeDuration + ActiveModeDuration).
        # TH writes a new value to the NodeLabel attribute and verifies the existing subscription reports it.
        self.step(7)
        await self.wait_for_transition(ICDTransition.FullCycle,
                                       active_mode_duration_ms=active_mode_duration_ms,
                                       idle_mode_duration_s=idle_mode_duration_s)

        report_received = asyncio.Event()

        def on_report_end(transaction):
            report_received.set()

        subscription.SetReportEndCallback(on_report_end)

        await TH.WriteAttribute(
            self.dut_node_id,
            [(0, Clusters.BasicInformation.Attributes.NodeLabel("TC_ICDB_2_2"))]
        )
        log.info("Wrote NodeLabel to 'TC_ICDB_2_2'")

        # The subscription receives a report for the NodeLabel attribute within MaxInterval
        try:
            await asyncio.wait_for(report_received.wait(), timeout=max_interval_s + 1)
            log.info("Subscription received report for NodeLabel within MaxInterval (%ss)", max_interval_s)
        except TimeoutError:
            asserts.fail(f"Subscription did not receive report for NodeLabel within MaxInterval ({max_interval_s}s)")

        # *** STEP 8 ***
        # TH shuts down the subscription, then waits for the DUT to detect the dropped subscriber and resume check-ins
        #   - DUT resumes sending check-in messages once it detects the subscription is gone.
        self.step(8)
        subscription.Shutdown()
        log.info("Subscription shut down.")

        wait_s = self.checkin_resume_wait_s(max_interval_s=max_interval_s,
                                            active_mode_duration_ms=active_mode_duration_ms,
                                            idle_mode_duration_s=idle_mode_duration_s)
        await asyncio.sleep(wait_s)

        # Verify ICDCounter incremented beyond icd_counter_at_subscription (check-in resumed)
        icd_counter_after_shutdown = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)
        asserts.assert_greater(icd_counter_after_shutdown, icd_counter_at_subscription,
                               f"ICDCounter must increment after subscription shutdown (check-in resumed). "
                               f"Before: {icd_counter_at_subscription}, After: {icd_counter_after_shutdown}")

        await self.unregister_all_clients()


if __name__ == "__main__":
    default_matter_test_main()
