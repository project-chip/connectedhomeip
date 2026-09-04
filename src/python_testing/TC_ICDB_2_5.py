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
from support_modules.icd_support import ICDBaseTest, assert_subscription_heartbeat_received

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.testing.commissioning import get_setup_payload_info_config
from matter.testing.decorators import async_test_body
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

'''
Purpose
Verify that with multiple fabrics, one registered client with an active subscription and one
unregistered client with an active subscription, the ICD state machine enters subscribed state,
periodically sends subscription reports to both clients, and does not send check-in messages.

Roles
- DUT as ICD server (commissionee)
- TH1 as commissioner on Fabric 1, with ICD client registration
- TH2 as commissioner on Fabric 2, without ICD client registration

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/icdbehavior.adoc#tc-icdb-2-5-icd-state-machine-with-1-client-registration-with-subscription-and-1-unregistered-client-with-subscription-multiple-fabrics-dut_server
'''

'''
Notes/Considerations
In CI, --icdIdleModeDuration is set to 10s for fast cycling.
On a real DUT, use the --timeout <seconds> script argument if IdleModeDuration is large to prevent the test from timing out.
'''

cluster = Clusters.Objects.IcdManagement
attributes = cluster.Attributes
commands = cluster.Commands
ClientTypeEnum = cluster.Enums.ClientTypeEnum

MIN_INTERVAL_FLOOR_S = 0
MAX_INTERVAL_CEILING_S = 60
ONE_HOUR_S = 3600


class TC_ICDB_2_5(ICDBaseTest):

    @async_test_body
    async def setup_class(self):
        # *** PRECONDITION ***
        # Commission DUT to TH1
        # Commission DUT to TH2 on a separate fabric
        super().setup_class()

        # TH1 commissions DUT (ICD registration done explicitly via RegisterClient in the test body)
        self.th1 = self.default_controller
        setup_payload_info = get_setup_payload_info_config(self.matter_test_config)
        info = setup_payload_info[0]
        await self.th1.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=info.passcode,
            filterType=info.filter_type,
            filter=info.filter_value
        )

        # TH2 commissions DUT on a separate fabric (intentionally no ICD registration)
        self.th2 = self.create_new_controller()

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

    def desc_TC_ICDB_2_5(self) -> str:
        return "[TC-ICDB-2.5] ICD State Machine - With 1 client registration with subscription and 1 unregistered client with subscription - Multiple Fabrics [DUT as Server]"

    def pics_TC_ICDB_2_5(self) -> list[str]:
        return [
            "ICDB.S",
            "ICDM.S.F00",
        ]

    @async_test_body
    async def test_TC_ICDB_2_5(self):

        # *** PRECONDITION ***
        self.step("precondition", "Commission DUT to TH1 and TH2.")

        # *** STEP 1 ***
        self.step(1, """TH1 sends the RegisterClient command to the DUT with TH1's node ID as checkInNodeID and monitoredSubject.
                        TH1 reads the RegisteredClients attribute.""", expectation="""
                            TH1 is registered as an ICD client on the DUT.
                            Verify exactly one RegisteredClients entry is present.
                            Verify that the RegisteredClients entry's checkInNodeID and monitoredSubject match TH1's node ID.""")
        th1_checkin_key = os.urandom(16)
        th1_check_in_node_id = self.th1.nodeId
        await self.send_single_icdm_command(commands.RegisterClient(
            checkInNodeID=th1_check_in_node_id,
            monitoredSubject=th1_check_in_node_id,
            key=th1_checkin_key,
            clientType=ClientTypeEnum.kPermanent,
        ))
        log.info("TH1 RegisterClient SUCCESS for checkInNodeID=%s", th1_check_in_node_id)

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
        self.step(2, """TH2 reads from the DUT the RegisteredClients attribute.""", expectation="""
                            Verify RegisteredClients is empty (TH2 intentionally did not register as an ICD client).""")
        th2_registered_clients = await self.read_icdm_attribute_expect_success(
            attributes.RegisteredClients, controller=self.th2, node_id=self.th2_dut_node_id
        )

        # Verify RegisteredClients is empty for TH2
        asserts.assert_equal(len(th2_registered_clients), 0,
                             f"Expected 0 RegisteredClients entries for TH2 (intentionally unregistered), got {len(th2_registered_clients)}")

        # *** STEP 3 ***
        self.step(3, """TH1 reads from the DUT the IdleModeDuration attribute.""", expectation="""
                            Save value as idle_mode_duration_s, with it calculate: SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT = MAX(idle_mode_duration_s, 3600s).
                            Store value for later use.""")
        idle_mode_duration_s = await self.read_icdm_attribute_expect_success(attributes.IdleModeDuration)
        log.info("IdleModeDuration: %ss", idle_mode_duration_s)
        subscription_max_interval_publisher_limit_s = max(idle_mode_duration_s, ONE_HOUR_S)

        # *** STEP 4 ***
        self.step(4, """TH1 and TH2 each subscribe to the ICDCounter attribute, with MinIntervalFloor and MaxIntervalCeiling.""", expectation="""
                            Verify MinIntervalFloor <= MaxInterval <= MAX(SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT, MaxIntervalCeiling) for both TH1 and TH2.""")
        th1_subscription = await self.default_controller.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[(self.ROOT_NODE_ENDPOINT_ID, attributes.ICDCounter)],
            reportInterval=(MIN_INTERVAL_FLOOR_S, MAX_INTERVAL_CEILING_S),
            autoResubscribe=False
        )

        th2_subscription = await self.th2.ReadAttribute(
            nodeId=self.th2_dut_node_id,
            attributes=[(self.ROOT_NODE_ENDPOINT_ID, attributes.ICDCounter)],
            reportInterval=(MIN_INTERVAL_FLOOR_S, MAX_INTERVAL_CEILING_S),
            autoResubscribe=False
        )

        _, th1_max_interval_s = th1_subscription.GetReportingIntervalsSeconds()
        _, th2_max_interval_s = th2_subscription.GetReportingIntervalsSeconds()
        log.info("TH1 MaxInterval: %ss", th1_max_interval_s)
        log.info("TH2 MaxInterval: %ss", th2_max_interval_s)

        # Verify MinIntervalFloor <= MaxInterval <= MAX(SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT, MaxIntervalCeiling) for TH1
        asserts.assert_less_equal(th1_max_interval_s,
                                  max(subscription_max_interval_publisher_limit_s, MAX_INTERVAL_CEILING_S),
                                  f"TH1 MaxInterval {th1_max_interval_s}s exceeds MAX(SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT, MaxIntervalCeiling)")

        # Verify MinIntervalFloor <= MaxInterval <= MAX(SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT, MaxIntervalCeiling) for TH2
        asserts.assert_less_equal(th2_max_interval_s,
                                  max(subscription_max_interval_publisher_limit_s, MAX_INTERVAL_CEILING_S),
                                  f"TH2 MaxInterval {th2_max_interval_s}s exceeds MAX(SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT, MaxIntervalCeiling)")

        # *** STEP 5 ***
        self.step(5, """Wait for 1 or more MaxInterval.""", expectation="""
                            Verify DUT sent a subscription report to both TH1 and TH2 within MaxInterval.
                            Verify ICDCounter is unchanged.""")
        max_interval_s = max(th1_max_interval_s, th2_max_interval_s)

        # Read ICDCounter before the subscription wait to detect any check-in activity
        icd_counter_before = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)

        # Verify DUT sent a subscription report to both TH1 and TH2 within MaxInterval
        await asyncio.gather(
            assert_subscription_heartbeat_received(th1_subscription, max_interval_s),
            assert_subscription_heartbeat_received(th2_subscription, max_interval_s)
        )

        # Verify ICDCounter is unchanged
        icd_counter_after = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)
        asserts.assert_equal(icd_counter_after, icd_counter_before,
                             f"ICDCounter should not have changed while subscriptions are active. "
                             f"Before: {icd_counter_before}, After: {icd_counter_after}")


if __name__ == "__main__":
    default_matter_test_main()
