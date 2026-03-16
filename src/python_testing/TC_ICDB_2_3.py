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
#       --icdIdleModeDuration 10
#       --icdActiveModeDurationMs 10000
#     script-args: >
#       --storage-path admin_storage.json
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""
TC-ICDB-2.3: ICD State Machine - With both client registrations and no active subscription - 
Multiple Fabrics.

Purpose:
    Verify that after client registrations on multiple fabrics, the ICD state machine
    enters check-in state and periodically sends check-in messages to both clients when
    there are no active subscription sessions on any fabric.

Required Devices:
  - DUT as commissionee on two separate fabrics
  - TH1 as commissioner on its own fabric, with ICD registration
  - TH2 as commissioner on its own fabric, with ICD registration

Notes/Considerations:
    - CI uses short idle/active durations (--icdIdleModeDuration 10, --icdActiveModeDurationMs 10000)
      to complete check-in cycles within CI timeout.

Test Plan:
    https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/icdbehavior.adoc

"""

import logging

from mobly import asserts
from support_modules.icd_support import ICDBaseTest, ICDTransition

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import InteractionModelError
from matter.testing.commissioning import get_setup_payload_info_config
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.Objects.IcdManagement
attributes = cluster.Attributes
commands = cluster.Commands


class TC_ICDB_2_3(ICDBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()
        # *** PRECONDITION ***
        # Commission DUT to TH1 with ICD registration
        # Commission DUT to TH2 with ICD registration

        # TH1 setup with ICD registration on its own fabric
        self.th1 = self.default_controller
        th1_icd_params = self.th1.GenerateICDRegistrationParameters()
        self.th1.EnableICDRegistration(th1_icd_params)

        # TH1 commissions DUT
        setup_payload_infos = get_setup_payload_info_config(self.matter_test_config)
        setup_payload_info = setup_payload_infos[0]
        self.th1_dut_node_id = self.dut_node_id
        await self.th1.CommissionOnNetwork(
            nodeId=self.th1_dut_node_id,
            setupPinCode=setup_payload_info.passcode,
            filterType=setup_payload_info.filter_type,
            filter=setup_payload_info.filter_value
        )

        # TH2 setup with ICD registration on its own fabric
        th2_ca = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_ca.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)
        th2_icd_params = self.th2.GenerateICDRegistrationParameters()
        self.th2_expected_check_in_node_id = th2_icd_params.checkInNodeId
        self.th2.EnableICDRegistration(th2_icd_params)

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

    def desc_TC_ICDB_2_3(self) -> str:
        return "[TC-ICDB-2.3] ICD State Machine - Multiple Fabrics - DUT as Server"

    def steps_TC_ICDB_2_3(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning DUT to TH1 and TH2 with ICD client registration."),
            TestStep(1, "TH1 reads from the DUT the RegisteredClients attribute.", """
                     Verify exactly one RegisteredClients entry is present.
                     Verify that the RegisteredClients entry's checkInNodeID and monitoredSubject match TH1's node ID."""),
            TestStep(2, "TH2 reads from the DUT the RegisteredClients attribute.", """
                     Verify exactly one RegisteredClients entry is present.
                     Verify that the RegisteredClients entry's checkInNodeID and monitoredSubject match TH2's node ID."""),
            TestStep(3, "TH1 reads from the DUT the IdleModeDuration, ActiveModeDuration, and ICDCounter attributes.",
                     "Store values for later use."),
            TestStep(4, "Wait for a full active-to-idle-to-active ICD transition cycle. TH1 reads the ICDCounter attribute.",
                     "Verify ICDCounter is greater than the initial value."),
            TestStep(5, "TH2 sends UnregisterClient command with TH2's checkInNodeID.",
                     "Command success."),
            TestStep(6, "Wait for a full active-to-idle-to-active ICD transition cycle. TH1 reads the ICDCounter attribute.",
                     "Verify ICDCounter is greater than the value from step 4."),
            TestStep(7, "TH1 sends UnregisterClient command with TH1's checkInNodeID.",
                     "Command success."),
            TestStep(8, "Wait for a full active-to-idle-to-active ICD transition cycle. TH1 reads the ICDCounter attribute.",
                     "Verify ICDCounter is unchanged from the value after TH2 unregister."),
        ]

    def pics_TC_ICDB_2_3(self) -> list[str]:
        return [
            "ICDB.S",
            "ICDM.S.F00",
        ]

    async def read_icdm_attribute_expect_success(self, attribute):
        return await self.read_single_attribute_check_success(
            endpoint=self.ROOT_NODE_ENDPOINT_ID, cluster=cluster, attribute=attribute)

    async def send_single_icdm_command(self, command):
        return await self.send_single_cmd(command, endpoint=self.ROOT_NODE_ENDPOINT_ID)

    async def th2_read_icdm_attribute_expect_success(self, attribute):
        return await self.read_single_attribute_check_success(
            endpoint=self.ROOT_NODE_ENDPOINT_ID, cluster=cluster, attribute=attribute,
            dev_ctrl=self.th2, node_id=self.th2_dut_node_id)

    async def th2_send_single_icdm_command(self, command):
        return await self.send_single_cmd(
            command, endpoint=self.ROOT_NODE_ENDPOINT_ID,
            dev_ctrl=self.th2, node_id=self.th2_dut_node_id)

    @async_test_body
    async def test_TC_ICDB_2_3(self):

        # *** PRECONDITION ***
        # Commissioning DUT to TH1 and TH2 with ICD client registration.
        self.step("precondition")

        # *** STEP 1a ***
        # TH1 reads from the DUT the RegisteredClients attribute.
        self.step(1)
        th1_registered_clients = await self.read_icdm_attribute_expect_success(attributes.RegisteredClients)
        th1_check_in_node_id = th1_registered_clients[0].checkInNodeID
        th1_monitored_subject = th1_registered_clients[0].monitoredSubject

        # Verify exactly one RegisteredClients entry is present.
        asserts.assert_equal(len(th1_registered_clients), 1,
                             f"Exactly one RegisteredClients entry must be present, got {len(th1_registered_clients)}, {th1_registered_clients}")

        # Verify that the RegisteredClients entry's checkInNodeID and monitoredSubject match TH1's node ID.
        asserts.assert_equal(th1_check_in_node_id, self.matter_test_config.controller_node_id,
                             f"The RegisteredClients entry's checkInNodeID ({th1_check_in_node_id}) must match TH1's node ID ({self.matter_test_config.controller_node_id}).")

        # Verify that the RegisteredClients entry's monitoredSubject matches TH1's node ID.
        asserts.assert_equal(th1_monitored_subject, self.matter_test_config.controller_node_id,
                             f"The RegisteredClients entry's monitoredSubject ({th1_monitored_subject}) must match TH1's node ID ({self.matter_test_config.controller_node_id}).")

        # *** STEP 2 ***
        # TH2 reads from the DUT the RegisteredClients attribute.
        self.step(2)
        th2_registered_clients = await self.th2_read_icdm_attribute_expect_success(attributes.RegisteredClients)
        th2_check_in_node_id = th2_registered_clients[0].checkInNodeID
        th2_monitored_subject = th2_registered_clients[0].monitoredSubject

        # Verify exactly one RegisteredClients entry is present.
        asserts.assert_equal(len(th2_registered_clients), 1,
                             f"Exactly one RegisteredClients entry must be present, got {len(th2_registered_clients)}, {th2_registered_clients}")

        # Verify that the RegisteredClients entry's checkInNodeID and monitoredSubject match TH2's node ID.
        asserts.assert_equal(th2_check_in_node_id, self.th2_expected_check_in_node_id,
                             f"The RegisteredClients entry's checkInNodeID ({th2_check_in_node_id}) must match TH2's node ID ({self.th2_expected_check_in_node_id})")

        # Verify that the RegisteredClients entry's monitoredSubject matches TH2's node ID.
        asserts.assert_equal(th2_monitored_subject, self.th2_expected_check_in_node_id,
                             f"The RegisteredClients entry's monitoredSubject ({th2_monitored_subject}) must match TH2's node ID ({self.th2_expected_check_in_node_id})")

        # *** STEP 3 ***
        # TH1 reads from the DUT the IdleModeDuration, ActiveModeDuration, and ICDCounter attributes.
        self.step(3)
        idle_mode_duration_s = await self.read_icdm_attribute_expect_success(attributes.IdleModeDuration)
        active_mode_duration_ms = await self.read_icdm_attribute_expect_success(attributes.ActiveModeDuration)
        icd_counter_initial = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)
        log.info(f"IdleModeDuration: {idle_mode_duration_s}s")
        log.info(f"ActiveModeDuration: {active_mode_duration_ms}ms")
        log.info(f"ICDCounter initial: {icd_counter_initial}")

        # *** STEP 4 ***
        # Wait for a full active-to-idle-to-active ICD transition cycle. TH1 reads the ICDCounter attribute.
        self.step(4)
        await self.wait_for_transition(ICDTransition.FullCycle,
                                       active_mode_duration_ms=active_mode_duration_ms,
                                       idle_mode_duration_s=idle_mode_duration_s)
        current_icd_counter_after_full_cycle = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)

        # Verify ICDCounter is greater than the initial value.
        asserts.assert_greater(current_icd_counter_after_full_cycle, icd_counter_initial,
                               f"Current ICDCounter ({current_icd_counter_after_full_cycle}) must be greater than the initial value ({icd_counter_initial}).")

        # *** STEP 5 ***
        # TH2 sends UnregisterClient command with the TH2's checkInNodeID.
        self.step(5)
        try:
            await self.th2_send_single_icdm_command(commands.UnregisterClient(checkInNodeID=th2_check_in_node_id))
        except InteractionModelError as e:
            asserts.assert_fail(f"UnregisterClient command with checkInNodeID {th2_check_in_node_id} failed with error: {e}")

        # *** STEP 6 ***
        # Wait for a full active-to-idle-to-active ICD transition cycle. TH1 reads the ICDCounter attribute.
        self.step(6)
        await self.wait_for_transition(ICDTransition.FullCycle,
                                       active_mode_duration_ms=active_mode_duration_ms,
                                       idle_mode_duration_s=idle_mode_duration_s)
        current_icd_counter_after_th2_unregister = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)

        # Verify ICDCounter is greater than the value from step 4.
        asserts.assert_greater(current_icd_counter_after_th2_unregister, current_icd_counter_after_full_cycle,
                               f"Current ICDCounter ({current_icd_counter_after_th2_unregister}) must be greater than the value from step 4 ({current_icd_counter_after_full_cycle}).")

        # *** STEP 7 ***
        # TH1 sends UnregisterClient command with the TH1's checkInNodeID.
        self.step(7)
        try:
            await self.send_single_icdm_command(commands.UnregisterClient(checkInNodeID=th1_check_in_node_id))
        except InteractionModelError as e:
            asserts.assert_fail(f"UnregisterClient command with checkInNodeID {th1_check_in_node_id} failed with error: {e}")

        # *** STEP 8 ***
        # Wait for a full active-to-idle-to-active ICD transition cycle. TH1 reads the ICDCounter attribute.
        self.step(8)
        await self.wait_for_transition(ICDTransition.FullCycle,
                                       active_mode_duration_ms=active_mode_duration_ms,
                                       idle_mode_duration_s=idle_mode_duration_s)
        current_icd_counter_after_th1_unregister = await self.read_icdm_attribute_expect_success(attributes.ICDCounter)

        # Verify ICDCounter is unchanged from the value after TH2 unregister.
        asserts.assert_equal(current_icd_counter_after_th1_unregister, current_icd_counter_after_th2_unregister,
                             f"Current ICDCounter ({current_icd_counter_after_th1_unregister}) must be unchanged from the value after TH2 unregister ({current_icd_counter_after_th2_unregister}).")


if __name__ == "__main__":

    default_matter_test_main()
