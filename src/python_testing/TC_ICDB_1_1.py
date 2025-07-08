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

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from time import sleep
from chip.ChipDeviceCtrl import ScopedNodeId


class TC_ICDB_1_1(MatterBaseTest):

    #
    # Class Helper functions
    #
    async def _read_icdm_attribute_expect_success(self, attribute):
        return await self.read_single_attribute_check_success(endpoint=0, cluster=Clusters.Objects.IcdManagement, attribute=attribute)

    async def _send_single_icdm_command(self, command):
        return await self.send_single_cmd(command, endpoint=0)

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

        # Commissioning Step
        self.step(1)

        try:
            self.step("1a")
            try:
                registered_clients = await self._read_icdm_attribute_expect_success(attribute=attributes.RegisteredClients)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            for client in registered_clients:
                try:
                    await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=client.checkInNodeID))
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            self.step("1b")
            try:
                icd_configs = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, cluster, attributes.IdleModeDuration),
                                                                                             (0, cluster, attributes.ActiveModeDuration),
                                                                                             (0, cluster, attributes.ActiveModeThreshold)])
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            self.step(2)
            registration_info = self.default_controller.GenerateICDRegistrationParameters()
            try:
                await self._send_single_icdm_command(commands.RegisterClient(checkInNodeID=registration_info.checkInNodeID,
                                                                             monitoredSubject=registration_info.monitoredSubject,
                                                                             key=registration_info.key,
                                                                             clientType=registration_info.clientType))
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            self.step(3)
            # Wait for DUT transition to Idle Mode - ActiveModeThreshold + ActiveModeDuration
            sleep(icd_configs.ActiveModeThreshold + icd_configs.ActiveModeDuration)

            self.step(4)
            # Wait for 1 or more cycle of IdleModeDuration to receinve a Check-In message
            try:
                self.default_controller.WaitForCheckIn(ScopedNodeId(self.dut_node_id, self.default_controller.GetFabricIndexInternal()),
                                                       timeout=icd_configs.IdleModeDuration)
            except TimeoutError:
                asserts.fail("Check-In message not received within IdleModeDuration")

        finally:
            # Post Condition processing
            try:
                await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=registration_info.checkInNodeID))
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
