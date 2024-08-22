
#
#    Copyright (c) 2023 Project CHIP Authors
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
# test-runner-runs: run1
# test-runner-run/run1/app: ${LIT_ICD_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import time
from dataclasses import dataclass

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)

# ==========================
# Constants
# ==========================

kRootEndpointId = 0

cluster = Clusters.Objects.IcdManagement
commands = cluster.Commands
ClientTypeEnum = cluster.Enums.ClientTypeEnum
attributes = cluster.Attributes


@dataclass
class Client:
    checkInNodeID: int
    subjectId: int
    key: bytes
    clientType: ClientTypeEnum


#
# Test Input Data
#
kIncorrectKey = b"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1a"
kInvalidKey = b"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e1g"

client1 = Client(
    checkInNodeID=1,
    subjectId=1,
    key=b"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f",
    clientType=ClientTypeEnum.kEphemeral
)

client2 = Client(
    checkInNodeID=1,
    subjectId=2,
    key=b"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1e",
    clientType=ClientTypeEnum.kEphemeral
)

client3 = Client(
    checkInNodeID=1,
    subjectId=3,
    key=b"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1d",
    clientType=ClientTypeEnum.kEphemeral
)

client4 = Client(
    checkInNodeID=1,
    subjectId=4,
    key=b"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1c",
    clientType=ClientTypeEnum.kEphemeral
)

client5 = Client(
    checkInNodeID=5,
    subjectId=5,
    key=b"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1b",
    clientType=ClientTypeEnum.kEphemeral
)

client6 = Client(
    checkInNodeID=5,
    subjectId=6,
    key=b"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1a",
    clientType=ClientTypeEnum.kEphemeral
)

client7 = Client(
    checkInNodeID=5,
    subjectId=7,
    key=b"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x0f",
    clientType=ClientTypeEnum.kEphemeral
)

client8 = Client(
    checkInNodeID=5,
    subjectId=8,
    key=b"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x0e",
    clientType=ClientTypeEnum.kEphemeral
)


class TC_ICDM_3_2(MatterBaseTest):

    #
    # Class Helper functions
    #
    async def _read_icdm_attribute_expect_success(self, attribute):
        return await self.read_single_attribute_check_success(endpoint=kRootEndpointId, cluster=cluster, attribute=attribute)

    async def _send_single_icdm_command(self, command):
        return await self.send_single_cmd(command, endpoint=kRootEndpointId)
    #
    # Test Harness Helpers
    #

    def desc_TC_ICDM_3_2(self) -> str:
        """Returns a description of this test"""
        return "[TC-ICDM-3.2] Verify RegisterClient Command with DUT as Server"

    def steps_TC_ICDM_3_2(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads from the DUT the RegisteredClients attribute. RegisteredClients is empty."),
            TestStep("2a", "TH sends RegisterClient command."),
            TestStep("2b", "TH reads from the DUT the RegisteredClients attribute."),
            TestStep("2c", "Power cycle DUT."),
            TestStep("2d", "TH waits for {PIXIT.WAITTIME.REBOOT}"),
            TestStep("2e", "TH reads from the DUT the RegisteredClients attribute."),
            TestStep("3a", "TH sends RegisterClient command with same CheckInNodeID1 as in Step 1a and different MonitorSubID2 and Key2."),
            TestStep("3b", "TH reads from the DUT the RegisteredClients attribute."),
            TestStep("4a", "TH sends RegisterClient command with same CheckInNodeID1 as in Step 1a and different MonitorSubID3 and Key3, and an invalid VerificationKey3."),
            TestStep("4b", "TH reads from the DUT the RegisteredClients attribute."),
            TestStep("5a", "TH sends RegisterClient command with same CheckInNodeID1 as in Step 1a and different MonitorSubID4 and Key4, and a valid wrong VerificationKey4."),
            TestStep("5b", "TH reads from the DUT the RegisteredClients attribute."),
            TestStep("6a", "TH sends UnregisterClient command with CheckInNodeID1."),
            TestStep("6b", "TH reads from the DUT the RegisteredClients attribute."),
            TestStep(7, "Set the TH to Manage privilege for ICDM cluster."),
            TestStep("8a", "TH sends RegisterClient command."),
            TestStep("8b", "TH sends RegisterClient command with same CheckInNodeID5 as in Step 6a and different MonitorSubID6 and Key6, and an invalid VerificationKey6."),
            TestStep("8c", "TH sends RegisterClient command with same CheckInNodeID5 as in Step 6a and different MonitorSubID7 and Key7, and an valid wrong VerificationKey7."),
            TestStep("8d", "TH sends RegisterClient command with same CheckInNodeID5 and VerificationKey5 as in Step 6a and different MonitorSubID9 and Key9."),
            TestStep(9, "TH sends UnregisterClient command with the CheckInNodeID5 and VerificationKey5."),
        ]
        return steps

    def pics_TC_ICDM_3_2(self) -> list[str]:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        pics = [
            "ICDM.S",
            "ICDM.S.F00"
        ]
        return pics

    #
    # ICDM 3.2 Test Body
    #

    @ async_test_body
    async def test_TC_ICDM_3_2(self):
        is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        if not is_ci:
            asserts.assert_true('PIXIT.WAITTIME.REBOOT' in self.matter_test_config.global_test_params,
                                "PIXIT.WAITTIME.REBOOT must be included on the command line in "
                                "the --int-arg flag as PIXIT.WAITTIME.REBOOT:<wait time>")

            wait_time_reboot = self.matter_test_config.global_test_params['PIXIT.WAITTIME.REBOOT']
            if wait_time_reboot == 0:
                asserts.fail("PIXIT.WAITTIME.REBOOT shall be higher than 0.")

        # Pre-Condition: Commissioning
        self.step(0)

        # Empty RegisteredClients attribute for all registrations
        self.step(1)
        registeredClients = await self._read_icdm_attribute_expect_success(
            attributes.RegisteredClients)

        for client in registeredClients:
            try:
                await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=client.checkInNodeID))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")

        try:
            self.step("2a")
            try:
                await self._send_single_icdm_command(commands.RegisterClient(checkInNodeID=client1.checkInNodeID, monitoredSubject=client1.subjectId, key=client1.key, clientType=client1.clientType))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")

            self.step("2b")
            registeredClients = await self._read_icdm_attribute_expect_success(
                attributes.RegisteredClients)
            # Validate list size
            asserts.assert_equal(len(registeredClients), 1,
                                 "The expected length of RegisteredClients is 1. List has the wrong size.")

            # Validate entry values
            asserts.assert_equal(
                registeredClients[0].checkInNodeID, client1.checkInNodeID, "The read attribute does not match the registered value.")
            asserts.assert_equal(
                registeredClients[0].monitoredSubject, client1.subjectId, "The read attribute does not match the registered value.")
            asserts.assert_equal(
                registeredClients[0].clientType, client1.clientType, "The read attribute does not match the registered value.")

            # Reboot
            self.step("2c")
            if not is_ci:
                self.wait_for_user_input(prompt_msg="Restart DUT. Press Enter when restart has been initiated.")

            self.step("2d")
            if not is_ci:
                time.sleep(wait_time_reboot)

            self.step("2e")
            registeredClients = await self._read_icdm_attribute_expect_success(
                attributes.RegisteredClients)
            # Validate list size
            asserts.assert_equal(len(registeredClients), 1,
                                 "The expected length of RegisteredClients is 1. List has the wrong size.")

            # Validate entry values
            asserts.assert_equal(
                registeredClients[0].checkInNodeID, client1.checkInNodeID, "The read attribute does not match the registered value.")
            asserts.assert_equal(
                registeredClients[0].monitoredSubject, client1.subjectId, "The read attribute does not match the registered value.")
            asserts.assert_equal(
                registeredClients[0].clientType, client1.clientType, "The read attribute does not match the registered value.")

            self.step("3a")
            try:
                await self._send_single_icdm_command(commands.RegisterClient(checkInNodeID=client2.checkInNodeID, monitoredSubject=client2.subjectId, key=client2.key, clientType=client2.clientType))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")

            self.step("3b")
            registeredClients = await self._read_icdm_attribute_expect_success(
                attributes.RegisteredClients)
            # Validate list size
            asserts.assert_equal(len(registeredClients), 1,
                                 "The expected length of RegisteredClients is 1. List has the wrong size.")

            # Validate entry values
            asserts.assert_equal(
                registeredClients[0].checkInNodeID, client2.checkInNodeID, "The read attribute does not match the registered value.")
            asserts.assert_equal(
                registeredClients[0].monitoredSubject, client2.subjectId, "The read attribute does not match the registered value.")
            asserts.assert_equal(
                registeredClients[0].clientType, client2.clientType, "The read attribute does not match the registered value.")

            self.step("4a")
            try:
                await self._send_single_icdm_command(commands.RegisterClient(checkInNodeID=client3.checkInNodeID, monitoredSubject=client3.subjectId, key=client3.key, clientType=client3.clientType, verificationKey=kInvalidKey))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")

            self.step("4b")
            registeredClients = await self._read_icdm_attribute_expect_success(
                attributes.RegisteredClients)
            # Validate list size
            asserts.assert_equal(len(registeredClients), 1,
                                 "The expected length of RegisteredClients is 1. List has the wrong size.")

            # Validate entry values
            asserts.assert_equal(
                registeredClients[0].checkInNodeID, client3.checkInNodeID, "The read attribute does not match the registered value.")
            asserts.assert_equal(
                registeredClients[0].monitoredSubject, client3.subjectId, "The read attribute does not match the registered value.")
            asserts.assert_equal(
                registeredClients[0].clientType, client3.clientType, "The read attribute does not match the registered value.")

            self.step("5a")
            try:
                await self._send_single_icdm_command(commands.RegisterClient(checkInNodeID=client4.checkInNodeID, monitoredSubject=client4.subjectId, key=client4.key, clientType=client4.clientType, verificationKey=kIncorrectKey))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")

            self.step("5b")
            registeredClients = await self._read_icdm_attribute_expect_success(
                attributes.RegisteredClients)
            # Validate list size
            asserts.assert_equal(len(registeredClients), 1,
                                 "The expected length of RegisteredClients is 1. List has the wrong size.")

            # Validate entry values
            asserts.assert_equal(
                registeredClients[0].checkInNodeID, client4.checkInNodeID, "The read attribute does not match the registered value.")
            asserts.assert_equal(
                registeredClients[0].monitoredSubject, client4.subjectId, "The read attribute does not match the registered value.")
            asserts.assert_equal(
                registeredClients[0].clientType, client4.clientType, "The read attribute does not match the registered value.")

            self.step("6a")
            try:
                await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=client4.checkInNodeID))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")

            self.step("6b")
            registeredClients = await self._read_icdm_attribute_expect_success(
                attributes.RegisteredClients)
            asserts.assert_equal(len(registeredClients), 0,
                                 "The RegisteredClients list must be empty. List has the wrong size.")

            self.step(7)
            ac = Clusters.AccessControl
            previousAcl = await self.read_single_attribute_check_success(cluster=ac, attribute=ac.Attributes.Acl)
            newAcls = []

            # Set Admin permissions on Access Control cluster
            newAclEntry = ac.Structs.AccessControlEntryStruct(privilege=ac.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                                                              authMode=ac.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                              subjects=previousAcl[0].subjects, targets=[ac.Structs.AccessControlTargetStruct(
                                                                  cluster=Clusters.AccessControl.id)], fabricIndex=previousAcl[0].fabricIndex
                                                              )
            newAcls.append(newAclEntry)

            # Set Manage permissions on ICD Management cluster
            newAclEntry = ac.Structs.AccessControlEntryStruct(privilege=ac.Enums.AccessControlEntryPrivilegeEnum.kManage,
                                                              authMode=ac.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                              subjects=previousAcl[0].subjects, targets=[ac.Structs.AccessControlTargetStruct(
                                                                  cluster=Clusters.IcdManagement.id)], fabricIndex=previousAcl[0].fabricIndex
                                                              )
            newAcls.append(newAclEntry)

            try:
                await self.default_controller.WriteAttribute(nodeid=self.dut_node_id, attributes=[(0, ac.Attributes.Acl(newAcls))])
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")

            self.step("8a")
            try:
                await self._send_single_icdm_command(commands.RegisterClient(checkInNodeID=client5.checkInNodeID, monitoredSubject=client5.subjectId, key=client5.key, clientType=client5.clientType))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")

            self.step("8b")
            try:
                await self._send_single_icdm_command(commands.RegisterClient(checkInNodeID=client6.checkInNodeID, monitoredSubject=client6.subjectId, key=client6.key, clientType=client6.clientType, verificationKey=kInvalidKey))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Failure, "Unexpected error returned")

            self.step("8c")
            try:
                await self._send_single_icdm_command(commands.RegisterClient(checkInNodeID=client7.checkInNodeID, monitoredSubject=client7.subjectId, key=client7.key, clientType=client7.clientType, verificationKey=kIncorrectKey))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Failure, "Unexpected error returned")

            self.step("8d")
            try:
                await self._send_single_icdm_command(commands.RegisterClient(checkInNodeID=client8.checkInNodeID, monitoredSubject=client8.subjectId, key=client8.key, clientType=client8.clientType, verificationKey=client5.key))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")

            self.step(9)
            try:
                await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=client8.checkInNodeID, verificationKey=client8.key))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")

        # Post-Condition steps
        finally:
            # Reset ACLs
            try:
                await self.default_controller.WriteAttribute(nodeid=self.dut_node_id, attributes=[(0, ac.Attributes.Acl(previousAcl))])
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")

            # Clear all RegisteredClients
            registeredClients = await self._read_icdm_attribute_expect_success(
                attributes.RegisteredClients)

            for client in registeredClients:
                try:
                    await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=client.checkInNodeID))
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, "Unexpected error returned")


if __name__ == "__main__":
    default_matter_test_main()
