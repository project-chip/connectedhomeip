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

from mobly import asserts
from support_modules.icd_support import ICDBaseTest, assert_subscription_heartbeat_received

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.testing.commissioning import CommissioningInfo, commission_device, get_setup_payload_info_config
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

'''
Purpose
Verify that with multiple fabrics, as long as there is 1 or more client registration and 1 or more
active subscriptions, the ICD state machine enters subscribed state, periodically sends subscription
reports, and stops sending check-in messages.

Verify that the ICD state machine returns to check-in state and resumes periodic check-in messages
when there are no more active subscriptions on any fabric.

Roles
- DUT as ICD server (commissionee)
- TH1 as commissioner on Fabric 1
- TH2 as commissioner on Fabric 2

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/icdbehavior.adoc#tc-icdb-2-4-icd-state-machine-multiple-fabrics-dut_server
'''

'''
Notes/Considerations
In CI, --icdIdleModeDuration is set to 10s for fast cycling.
On a real DUT, use the --timeout <seconds> script argument if IdleModeDuration is large to prevent the test from timing out.
'''

cluster = Clusters.Objects.IcdManagement
attributes = cluster.Attributes
commands = cluster.Commands

ONE_HOUR_S = 3600


class TC_ICDB_2_4(ICDBaseTest):

    # DUT can take more than one cycle to detect a dropped subscriber and resume check-ins, default_timeout
    # is raised to accommodate that (can be overridden by a --timeout on the command line)
    @property
    def default_timeout(self) -> int:
        return 5 * 60

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        # TH1 commissions DUT with ICD registration (registers as a kPermanent ICD client during
        # commissioning, which also stores the symmetric key client-side so received check-ins decrypt).
        self.th1 = self.default_controller
        self.th1.EnableICDRegistration(self.th1.GenerateICDRegistrationParameters())
        setup_payload_info = get_setup_payload_info_config(self.matter_test_config)
        info = setup_payload_info[0]
        commissioning_info = CommissioningInfo(
            commissionee_ip_address_just_for_testing=self.matter_test_config.commissionee_ip_address_just_for_testing,
            commissioning_method=self.matter_test_config.commissioning_method or "on-network",
            thread_operational_dataset=self.matter_test_config.thread_operational_dataset,
            wifi_passphrase=self.matter_test_config.wifi_passphrase,
            wifi_ssid=self.matter_test_config.wifi_ssid,
            thread_ba_host=self.matter_test_config.thread_ba_host,
            thread_ba_port=self.matter_test_config.thread_ba_port,
        )
        status = await commission_device(self.th1, self.dut_node_id, info, commissioning_info)
        asserts.assert_true(status, f"Failed to commission DUT to TH1's fabric: {status}")

        # TH2 commissions DUT on a separate fabric with ICD registration (same client receive-stack arming as TH1)
        self.th2 = self.create_new_controller(enable_icd_registration=True)

        # TH2 commissions DUT
        ecw = await self.open_commissioning_window(dev_ctrl=self.th1, node_id=self.dut_node_id, timeout=600)
        self.th2_dut_node_id = self.dut_node_id + 1
        await self.th2.CommissionOnNetwork(
            nodeId=self.th2_dut_node_id,
            setupPinCode=ecw.commissioningParameters.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=ecw.randomDiscriminator
        )

    def teardown_class(self):
        if hasattr(self, 'th2') and self.th2 is not None:
            self.th2.Shutdown()
        super().teardown_class()

    def desc_TC_ICDB_2_4(self) -> str:
        return "[TC-ICDB-2.4] ICD State Machine - Multiple Fabrics [DUT as Server]"

    def steps_TC_ICDB_2_4(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commission DUT to TH1 and TH2."),
            TestStep(1, "TH1 reads the RegisteredClients attribute (TH1 was registered as an ICD client during commissioning).", """
                     TH1 is registered as an ICD client on the DUT.
                     Verify exactly one RegisteredClients entry is present.
                     Verify that the RegisteredClients entry's checkInNodeID and monitoredSubject match TH1's node ID."""),
            TestStep(2, "TH2 reads the RegisteredClients attribute (TH2 was registered as an ICD client during commissioning).", """
                     TH2 is registered as an ICD client on the DUT.
                     Verify exactly one RegisteredClients entry is present.
                     Verify that the RegisteredClients entry's checkInNodeID and monitoredSubject match TH2's node ID."""),
            TestStep(3, "TH1 reads from the DUT the IdleModeDuration, ActiveModeDuration, ActiveModeThreshold, and MaximumCheckInBackoff attributes.",
                     "Store values for later use."),
            TestStep(4, "TH1 and TH2 each subscribe to the ICDCounter attribute, with MinIntervalFloor=0 and MaxIntervalCeiling=IdleModeDuration.", """
                     Verify MinIntervalFloor <= MaxInterval <= MAX(SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT, MaxIntervalCeiling) for both TH1 and TH2."""),
            TestStep(5, "Wait for 1 or more MaxInterval.", """
                     No check-in message is sent to TH1 or TH2 while subscriptions are active.
                     Verify TH1 and TH2 each receive a subscription report within MaxInterval.
                     Verify ICDCounter is unchanged, confirming no check-in messages were sent."""),
            TestStep(6, "Deactivate the subscription between DUT and TH1, then wait for the DUT to detect the dropped subscriber and resume check-ins to TH1.", """
                     DUT starts sending check-in messages to TH1.
                     Verify TH1 receives a check-in from the DUT within the detection-aware timeout.
                     Verify TH2 receives a subscription report within MaxInterval."""),
            TestStep(7, "Deactivate the subscription between DUT and TH2, then wait for the DUT to resume check-ins on both fabrics.", """
                     DUT sends check-in messages to both TH1 and TH2 once no subscription covers them.
                     Verify TH1 receives a check-in from the DUT within the detection-aware timeout.
                     Verify TH2 receives a check-in from the DUT within the detection-aware timeout."""),
        ]

    def pics_TC_ICDB_2_4(self) -> list[str]:
        return [
            "ICDB.S",
            "ICDM.S.F00",
        ]

    @async_test_body
    async def test_TC_ICDB_2_4(self):

        # *** PRECONDITION ***
        # Commission DUT to TH1 and TH2.
        self.step("precondition")

        # *** STEP 1 ***
        # TH1 reads the RegisteredClients attribute (TH1 was registered as an ICD client during commissioning)
        self.step(1)
        th1_check_in_node_id = self.th1.nodeId

        # TH1 reads RegisteredClients to verify registration
        th1_registered_clients = await self.read_icdm_attribute_expect_success(attributes.RegisteredClients)

        # Verify exactly one RegisteredClients entry is present
        asserts.assert_equal(len(th1_registered_clients), 1,
                             f"Exactly one RegisteredClients entry must be present, got {len(th1_registered_clients)}")

        # Verify checkInNodeID and monitoredSubject match TH1's node ID
        rc_th1 = th1_registered_clients[0]
        asserts.assert_equal(rc_th1.checkInNodeID, th1_check_in_node_id,
                             f"checkInNodeID ({rc_th1.checkInNodeID}) must match TH1's node ID ({th1_check_in_node_id})")
        asserts.assert_equal(rc_th1.monitoredSubject, th1_check_in_node_id,
                             f"monitoredSubject ({rc_th1.monitoredSubject}) must match TH1's node ID ({th1_check_in_node_id})")

        # *** STEP 2 ***
        # TH2 reads the RegisteredClients attribute (TH2 was registered as an ICD client during commissioning)
        self.step(2)
        th2_check_in_node_id = self.th2.nodeId

        # TH2 reads RegisteredClients to verify registration
        th2_registered_clients = await self.read_icdm_attribute_expect_success(
            attributes.RegisteredClients, controller=self.th2, node_id=self.th2_dut_node_id
        )

        # Verify exactly one RegisteredClients entry is present
        asserts.assert_equal(len(th2_registered_clients), 1,
                             f"Exactly one RegisteredClients entry must be present, got {len(th2_registered_clients)}")

        # Verify checkInNodeID and monitoredSubject match TH2's node ID
        rc_th2 = th2_registered_clients[0]
        asserts.assert_equal(rc_th2.checkInNodeID, th2_check_in_node_id,
                             f"checkInNodeID ({rc_th2.checkInNodeID}) must match TH2's node ID ({th2_check_in_node_id})")
        asserts.assert_equal(rc_th2.monitoredSubject, th2_check_in_node_id,
                             f"monitoredSubject ({rc_th2.monitoredSubject}) must match TH2's node ID ({th2_check_in_node_id})")

        # *** STEP 3 ***
        # TH1 reads from the DUT the IdleModeDuration, ActiveModeDuration, ActiveModeThreshold, and MaximumCheckInBackoff attributes
        self.step(3)
        idle_mode_duration_s = await self.read_icdm_attribute_expect_success(attributes.IdleModeDuration)
        active_mode_duration_ms = await self.read_icdm_attribute_expect_success(attributes.ActiveModeDuration)
        active_mode_threshold_ms = await self.read_icdm_attribute_expect_success(attributes.ActiveModeThreshold)
        maximum_check_in_backoff_s = await self.read_icdm_attribute_expect_success(attributes.MaximumCheckInBackOff)
        log.info("IdleModeDuration: %ss", idle_mode_duration_s)
        log.info("ActiveModeDuration: %sms", active_mode_duration_ms)
        log.info("ActiveModeThreshold: %sms", active_mode_threshold_ms)
        log.info("MaximumCheckInBackoff: %ss", maximum_check_in_backoff_s)

        # *** STEP 4 ***
        # TH1 and TH2 each subscribe to the ICDCounter attribute with MinIntervalFloor and MaxIntervalCeiling
        self.step(4)
        subscription_max_interval_publisher_limit_s = max(idle_mode_duration_s, ONE_HOUR_S)

        th1_subscription = await self.default_controller.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[(self.ROOT_NODE_ENDPOINT_ID, attributes.ICDCounter)],
            reportInterval=(0, idle_mode_duration_s),
            autoResubscribe=False
        )

        th2_subscription = await self.th2.ReadAttribute(
            nodeId=self.th2_dut_node_id,
            attributes=[(self.ROOT_NODE_ENDPOINT_ID, attributes.ICDCounter)],
            reportInterval=(0, idle_mode_duration_s),
            autoResubscribe=False
        )

        th1_min_interval_s, th1_max_interval_s = th1_subscription.GetReportingIntervalsSeconds()
        th2_min_interval_s, th2_max_interval_s = th2_subscription.GetReportingIntervalsSeconds()
        log.info("TH1 MinInterval: %ss, MaxInterval: %ss", th1_min_interval_s, th1_max_interval_s)
        log.info("TH2 MinInterval: %ss, MaxInterval: %ss", th2_min_interval_s, th2_max_interval_s)

        # Verify MinIntervalFloor <= MaxInterval for both subscriptions
        asserts.assert_less_equal(th1_min_interval_s, th1_max_interval_s,
                                  f"TH1 MinIntervalFloor {th1_min_interval_s}s must be <= MaxInterval {th1_max_interval_s}s")
        asserts.assert_less_equal(th2_min_interval_s, th2_max_interval_s,
                                  f"TH2 MinIntervalFloor {th2_min_interval_s}s must be <= MaxInterval {th2_max_interval_s}s")

        # Verify MaxInterval <= MAX(SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT, MaxIntervalCeiling) for TH1
        asserts.assert_less_equal(th1_max_interval_s,
                                  max(subscription_max_interval_publisher_limit_s, idle_mode_duration_s),
                                  f"TH1 MaxInterval {th1_max_interval_s}s exceeds MAX(SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT, MaxIntervalCeiling)")

        # Verify MaxInterval <= MAX(SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT, MaxIntervalCeiling) for TH2
        asserts.assert_less_equal(th2_max_interval_s,
                                  max(subscription_max_interval_publisher_limit_s, idle_mode_duration_s),
                                  f"TH2 MaxInterval {th2_max_interval_s}s exceeds MAX(SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT, MaxIntervalCeiling)")

        # *** STEP 5 ***
        # Wait for 1 or more MaxInterval.
        # While subscriptions are active, the DUT should be in subscribed state and not send check-ins.
        self.step(5)
        max_interval_s = max(th1_max_interval_s, th2_max_interval_s)

        icd_counter_before_wait = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)

        # Verify DUT sent a subscription report to both TH1 and TH2 within MaxInterval
        await asyncio.gather(
            assert_subscription_heartbeat_received(th1_subscription, max_interval_s),
            assert_subscription_heartbeat_received(th2_subscription, max_interval_s)
        )

        # Verify ICDCounter is unchanged, confirming no check-in messages were sent
        icd_counter_after_wait = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)
        asserts.assert_equal(
            icd_counter_after_wait, icd_counter_before_wait,
            f"ICDCounter must not change while subscriptions are active. "
            f"Before: {icd_counter_before_wait}, After: {icd_counter_after_wait}"
        )

        # *** STEP 6 ***
        # Deactivate the subscription between DUT and TH1, then wait for the DUT to detect the dropped
        # subscriber and resume check-ins to TH1
        #   - DUT starts sending check-in messages to TH1
        self.step(6)
        th1_subscription.Shutdown()
        th1_checkin_timeout_s = self.checkin_resume_wait_s(
            max_interval_s=th1_max_interval_s,
            active_mode_duration_ms=active_mode_duration_ms, idle_mode_duration_s=idle_mode_duration_s,
            maximum_check_in_backoff_s=maximum_check_in_backoff_s,
            dev_ctrl=self.th1, node_id=self.dut_node_id)

        # Verify TH1 receives a check-in from the DUT once its subscription is dropped
        await self.assert_checkin_received(self.th1, self.dut_node_id, th1_checkin_timeout_s)

        # Verify TH2 subscription is still active
        await assert_subscription_heartbeat_received(th2_subscription, th2_max_interval_s)

        # *** STEP 7 ***
        # Deactivate the subscription between DUT and TH2, then wait for the DUT to resume check-ins on both fabrics
        #   - DUT sends check-in messages to both TH1 and TH2 once no subscription covers them
        self.step(7)
        th2_subscription.Shutdown()
        th2_checkin_timeout_s = self.checkin_resume_wait_s(
            max_interval_s=th2_max_interval_s,
            active_mode_duration_ms=active_mode_duration_ms, idle_mode_duration_s=idle_mode_duration_s,
            dev_ctrl=self.th2, node_id=self.th2_dut_node_id)

        # Verify TH1 and TH2 each receive a check-in from the DUT once no subscription covers them
        await asyncio.gather(
            self.assert_checkin_received(self.th1, self.dut_node_id, th1_checkin_timeout_s),
            self.assert_checkin_received(self.th2, self.th2_dut_node_id, th2_checkin_timeout_s),
        )

        log.info("TH1 and TH2 each received a check-in; DUT has resumed check-in state on both fabrics.")


if __name__ == "__main__":
    default_matter_test_main()
