#
#    Copyright (c) 2025 Project CHIP Authors
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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import random
from datetime import timedelta
from time import sleep

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.ChipDeviceCtrl import ScopedNodeId, WaitForCheckIn
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_ICDB_1_1(MatterBaseTest):

    def desc_TC_ICDB_1_1(self) -> str:
        """Returns the description of the test case."""
        return "[TC-ICDB-1.1] ICD Check-In Protocol - Register client - idle mode duration"

    def pics_TC_ICDB_1_1(self) -> str:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        pics = [
            "ICDB.S",
            "ICDM.S.F00"
        ]
        return pics

    def steps_TC_ICDB_1_1(self) -> list[TestStep]:
        """Returns the steps of the test case."""
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("1a", "TH reads from the DUT the RegisteredClients attribute and Unregisters all clients if any."),
            TestStep("1b", "TH reads from the DUT the IdleModeDuration, ActiveModeDuration, and ActiveModeThreshold attributes."),
            TestStep(2, "TH sends RegisterClient command."),
            TestStep(3, "Wait for DUT transition to Idle Mode."),
            TestStep(4, "Wait for 1 or more cycle of IdleModeDuration. Verify Check-In message.")
        ]

        return steps

    @async_test_body
    async def test_TC_ICDB_1_1(self):

        cluster = Clusters.Objects.IcdManagement
        commands = cluster.Commands
        attributes = cluster.Attributes

        # Initialize variables
        require_unregister = False
        registration_info = None
        th2_certificate_authority = None
        th2_fabric_admin = None

        # Commissioning Step
        self.step(1)
        self.th1 = self.default_controller

        try:
            self.step("1a")
            try:
                registered_clients = await self.read_single_attribute_check_success(endpoint=0, cluster=Clusters.Objects.IcdManagement, attribute=attributes.RegisteredClients)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Failed to read RegisteredClients attribute")

            for client in registered_clients:
                try:
                    await self.send_single_cmd(cmd=commands.UnregisterClient(checkInNodeID=client.checkInNodeID), endpoint=0)
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.Success, f"Failed to unregister client {client.checkInNodeID}: {e}")

            self.step("1b")
            try:
                icd_configs = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, cluster, attributes.IdleModeDuration),
                                                                                             (0, cluster, attributes.ActiveModeDuration),
                                                                                             (0, cluster, attributes.ActiveModeThreshold)])

                idle_mode_duration_s = icd_configs[0][cluster][attributes.IdleModeDuration]
                active_mode_duration_ms = icd_configs[0][cluster][attributes.ActiveModeDuration]
                active_mode_threshold_ms = icd_configs[0][cluster][attributes.ActiveModeThreshold]

                logger.info(f"ICD Config - IdleModeDuration: {idle_mode_duration_s}s, "
                            f"ActiveModeDuration: {active_mode_duration_ms}ms, "
                            f"ActiveModeThreshold: {active_mode_threshold_ms}ms")

            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Failed to read ICD configuration attributes")

            self.step(2)
            try:
                discriminator = random.randint(0, 4095)
                th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
                th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
                self.th2 = th2_fabric_admin.NewController(nodeId=self.th1.nodeId + 1, useTestCommissioner=True)

                registration_info = self.th2.GenerateICDRegistrationParameters()
                registration_info.stayActiveMs = 0  # We do not need to keep the client active
                self.th2.EnableICDRegistration(registration_info)

                params = await self.th1.OpenCommissioningWindow(nodeid=self.dut_node_id, timeout=900, iteration=10000,
                                                                discriminator=discriminator, option=self.th1.CommissioningWindowPasscode.kTokenWithRandomPin)

                logger.info("Commissioning TH2 on DUT...")
                await self.th2.CommissionOnNetwork(
                    nodeId=self.dut_node_id, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
                    filter=discriminator, setupPinCode=params.setupPinCode)

                require_unregister = True
                logger.info("TH2 successfully commissioned")

            except Exception as e:
                asserts.fail(f"Failed to commission TH2 on the DUT: {e}")

            # Wait for DUT transition to Idle Mode
            self.step(3)
            transition_wait_time = timedelta(milliseconds=(active_mode_threshold_ms + active_mode_duration_ms)).total_seconds()
            logger.info(f"Waiting {transition_wait_time:.2f}s for DUT transition to Idle Mode")
            sleep(transition_wait_time)

            self.step(4)
            # Wait for 1 or more cycle of IdleModeDuration to receive a Check-In message
            timeout_seconds = idle_mode_duration_s  # Increased margin for reliability
            logger.info(f"Waiting up to {timeout_seconds}s for Check-In message")
            try:
                logger.info(
                    f"Waiting for Check-In message with ScopedNodeId: {ScopedNodeId(self.dut_node_id, self.th2.GetFabricIndexInternal())}")

                await WaitForCheckIn(ScopedNodeId(self.dut_node_id, self.th2.GetFabricIndexInternal()),
                                     timeoutSeconds=timeout_seconds)
                logger.info("Check-In message received successfully")
            except TimeoutError:
                asserts.fail(f"Check-In message not received within {timeout_seconds}s (IdleModeDuration: {idle_mode_duration_s}s)")

        finally:
            # Post Condition processing
            if require_unregister:
                try:
                    if registration_info:
                        await self.th2.UnpairDevice(self.dut_node_id)
                except Exception as e:
                    asserts.fail(f"Failed to unregister client: {e}")
                    pass


if __name__ == "__main__":
    default_matter_test_main()
