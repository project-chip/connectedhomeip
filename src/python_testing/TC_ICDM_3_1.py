
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

import logging
import os
from enum import IntEnum

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)

kRootEndpointId = 0

cluster = Clusters.Objects.IcdManagement
commands = cluster.Commands
monitoredRegistration = cluster.Structs.MonitoringRegistrationStruct
clientTypeEnum = cluster.Enums.ClientTypeEnum
features = cluster.Bitmaps.Feature


class ICDTestEventTriggerOperations(IntEnum):
    """
    Copy of ICDTestEventTriggerEvent from ICDManager.cpp.
    Both enum classes must use the same values to maintain compatibility.

    TODO: Figure out how to use the class from ICDManager instead of maintaining a copy.
    """
    kAddActiveModeReq = 0x0046000000000001
    kRemoveActiveModeReq = 0x0046000000000002
    kInvalidateHalfCounterValues = 0x0046000000000003
    kInvalidateAllCounterValues = 0x0046000000000004
    kForceMaximumCheckInBackOffState = 0x0046000000000005


# Step 2 Registration entry
kStep2CheckInNodeId = 101
kStep2MonitoredSubjectStep2 = 1001
kStep2Key = b"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"


class TC_ICDM_3_1(MatterBaseTest):

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

    def desc_TC_ICDM_3_1(self) -> str:
        """Returns a description of this test"""
        return "[TC-ICDM-3.1] Register/Unregister Clients with DUT as Server"

    def steps_TC_ICDM_3_1(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads from the DUT the FeatureMap. If the CIP feature is not supported on the cluster, skip all remaining steps"),
            TestStep("2a", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster."),
            TestStep("2b", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.ICDM.S.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.ICDM.S.TEST_EVENT_TRIGGER for the Active Mode requirement."),
            TestStep(3, "TH reads from the DUT the RegisteredClients attribute. If not empty, TH sends command UnregisterClient to clear all clients in RegisteredClients."),
            TestStep(4, "TH reads from the DUT the ClientsSupportedPerFabric attribute."),
            TestStep(5, "TH reads from the DUT the ICDCounter attribute."),
            TestStep(6, "TH sends RegisterClient command. - CheckInNodeID: registering client’s node ID - MonitoredSubject: MonitoredSubID - Key: shared secret between the client and the ICD - ClientType : Ephemeral(1)."),
            TestStep(7, "TH reads from the DUT the RegisteredClients attribute."),
            TestStep(8, "If len(RegisteredClients) is less than ClientsSupportedPerFabric, TH repeats RegisterClient command with different CheckInNodeID(s) and the Permanent(0) ClientType until the number of entries in RegisteredClients equals ClientsSupportedPerFabric."),
            TestStep(9, "TH reads from the DUT the RegisteredClients attribute."),
            TestStep(10, "TH sends RegisterClient command with a different CheckInNodeID."),
            TestStep(11, "TH sends UnregisterClient command with the CheckInNodeID from Step 10."),
            TestStep(12, "TH sends UnregisterClient command with the CheckInNodeID from Step 6."),
            TestStep(13, "TH reads from the DUT the RegisteredClients attribute."),
            TestStep(14, "Repeat Step 12-13 with the rest of CheckInNodeIDs from the list of RegisteredClients from Step 8, if any."),
            TestStep(15, "TH reads from the DUT the RegisteredClients attribute."),
            TestStep(16, "TH sends UnregisterClient command with the CheckInNodeID from Step 6."),
        ]

    def pics_TC_ICDM_3_1(self) -> list[str]:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return [
            "ICDM.S",
        ]

    #
    # ICDM 3.1 Test Body
    #

    @async_test_body
    async def test_TC_ICDM_3_1(self):

        cluster = Clusters.Objects.IcdManagement
        attributes = cluster.Attributes

        # Step 0: Commissioning (already done)
        self.step(0)

        try:
            # Step 1: FeatureMap check (skip if not supported)
            self.step(1)
            featureMap = await self._read_icdm_attribute_expect_success(attributes.FeatureMap)
            if featureMap & features.kCheckInProtocolSupport == 0:
                logger.info('CheckInProtocolSupport feature (ICDM.S.F00) is not supported, skipping test.')
                self.mark_all_remaining_steps_skipped("2a")
                return

            # Step 2a: Read TestEventTriggersEnabled from General Diagnostics Cluster
            self.step("2a")
            self.check_test_event_triggers_enabled()

            # Step 2b: Send TestEventTrigger command to General Diagnostics Cluster
            self.step("2b")
            self.send_test_event_triggers(eventTrigger=ICDTestEventTriggerOperations.kAddActiveModeReq)

            # Step 3: Read RegisteredClients, clear if not empty
            self.step(3)
            registeredClients = await self._read_icdm_attribute_expect_success(attributes.RegisteredClients)
            for client in registeredClients:
                try:
                    await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=client.checkInNodeID))
                except InteractionModelError as e:
                    asserts.assert_fail(f"Unexpected error returned : {e}")

            # Step 4: Read ClientsSupportedPerFabric
            self.step(4)
            clientsSupportedPerFabric = await self._read_icdm_attribute_expect_success(attributes.ClientsSupportedPerFabric)

            # Step 5: Read ICDCounter
            self.step(5)
            icdCounter = await self._read_icdm_attribute_expect_success(attributes.ICDCounter)

            # Step 6: RegisterClient (Ephemeral)
            self.step(6)
            try:
                response = await self._send_single_icdm_command(commands.RegisterClient(
                    checkInNodeID=kStep2CheckInNodeId,
                    monitoredSubject=kStep2MonitoredSubjectStep2,
                    key=kStep2Key,
                    clientType=clientTypeEnum.kEphemeral))
            except InteractionModelError as e:
                asserts.assert_fail(f"Unexpected error returned : {e}")

            # Validate response contains the ICDCounter
            asserts.assert_greater_equal(response.ICDCounter, icdCounter,
                                         "The ICDCounter in the response does not match the read ICDCounter.")

            # Step 7: Read RegisteredClients, verify entry
            self.step(7)
            registeredClients = await self._read_icdm_attribute_expect_success(attributes.RegisteredClients)

            # Validate list size
            asserts.assert_equal(len(registeredClients), 1,
                                 "The expected length of RegisteredClients is 1. List has the wrong size.")

            # Validate entry values
            asserts.assert_equal(
                registeredClients[0].checkInNodeID, kStep2CheckInNodeId, "The read attribute does not match the registered value.")
            asserts.assert_equal(
                registeredClients[0].monitoredSubject, kStep2MonitoredSubjectStep2, "The read attribute does not match the registered value.")
            asserts.assert_equal(
                registeredClients[0].clientType, clientTypeEnum.kEphemeral, "The read attribute does not match the registered value.")

            # Step 8: Fill RegisteredClients to ClientsSupportedPerFabric with Permanent clients
            self.step(8)
            if len(registeredClients) < clientsSupportedPerFabric:
                newClients = []
                # Generate new clients data
                for i in range(clientsSupportedPerFabric - len(registeredClients)):
                    # Generate ICD registration parameters using the controller's method
                    icd_params = self.default_controller.GenerateICDRegistrationParameters()
                    # Increment the checkInNodeID and monitoredSubject by i to ensure uniqueness
                    icd_params.checkInNodeId += i
                    newClients.append(icd_params)

                for client in newClients:
                    try:
                        response = await self._send_single_icdm_command(commands.RegisterClient(
                            checkInNodeID=client.checkInNodeId,
                            monitoredSubject=client.monitoredSubject,
                            key=client.symmetricKey,
                            clientType=client.clientType))
                    except InteractionModelError as e:
                        asserts.assert_fail(f"Unexpected error returned : {e}")

                    # Validate response contains the ICDCounter
                    asserts.assert_greater_equal(response.ICDCounter, icdCounter,
                                                 "The ICDCounter in the response does not match the read ICDCounter.")

            # Step 9: Read RegisteredClients
            self.step(9)
            registeredClients = await self._read_icdm_attribute_expect_success(attributes.RegisteredClients)

            # Validate list
            asserts.assert_equal(len(registeredClients[1:]), len(newClients),
                                 "The expected length of RegisteredClients is clientsSupportedPerFabric. List has the wrong size.")

            for client, expectedClient in zip(registeredClients[1:], newClients):
                asserts.assert_equal(
                    client.checkInNodeID, expectedClient.checkInNodeId, "The read attribute does not match the registered value.")
                asserts.assert_equal(
                    client.monitoredSubject, expectedClient.monitoredSubject, "The read attribute does not match the registered value.")
                asserts.assert_equal(
                    client.clientType, expectedClient.clientType, "The read attribute does not match the registered value.")

            # Step 10: RegisterClient with a different CheckInNodeID (should fail RESOURCE_EXHAUSTED)
            self.step(10)
            with asserts.assert_raises(InteractionModelError) as e:
                await self._send_single_icdm_command(commands.RegisterClient(
                    checkInNodeID=0xFFFF,
                    monitoredSubject=0xFFFF,
                    key=os.urandom(16),
                    clientType=clientTypeEnum.kPermanent))
            asserts.assert_equal(e.exception.status, Status.ResourceExhausted, "Unexpected error returned")

            # Step 11: UnregisterClient with CheckInNodeID from Step 10 (should fail NOT_FOUND)
            self.step(11)
            with asserts.assert_raises(InteractionModelError) as e:
                await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=0xFFFF))
            asserts.assert_equal(e.exception.status, Status.NotFound, "Unexpected error returned")

            # Step 12: UnregisterClient with CheckInNodeID from Step 6
            self.step(12)
            try:
                await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=kStep2CheckInNodeId))
            except InteractionModelError as e:
                asserts.assert_fail(f"Unexpected error returned : {e}")

            # Step 13: Read RegisteredClients, verify Step 6 client is not present
            self.step(13)
            registeredClients = await self._read_icdm_attribute_expect_success(attributes.RegisteredClients)
            for client in registeredClients:
                asserts.assert_not_equal(client.checkInNodeID, kStep2CheckInNodeId,
                                         "CheckInNodeID was unregistered. It should not be present in the attribute list.")

            # Step 14: Repeat Step 12-13 for remaining clients from Step 8
            self.step(14)
            for client in newClients:
                try:
                    await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=client.checkInNodeId))
                except InteractionModelError as e:
                    asserts.assert_fail(f"Unexpected error returned : {e}")

                registeredClients = await self._read_icdm_attribute_expect_success(attributes.RegisteredClients)
                for remainingClient in registeredClients:
                    asserts.assert_not_equal(
                        remainingClient.checkInNodeID, client.checkInNodeId, "CheckInNodeID was unregistered. It should not be present in the attribute list.")

            # Step 15: Read RegisteredClients, should be empty
            self.step(15)
            registeredClients = await self._read_icdm_attribute_expect_success(attributes.RegisteredClients)
            asserts.assert_true(not registeredClients, "This list should be empty. An element did not get deleted.")

            # Step 16: UnregisterClient with CheckInNodeID from Step 6 (should fail NOT_FOUND)
            self.step(16)
            with asserts.assert_raises(InteractionModelError) as e:
                await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=kStep2CheckInNodeId))
            asserts.assert_equal(e.exception.status, Status.NotFound, "Unexpected error returned")

        finally:
            # Post-Condition Steps

            registeredClients = await self._read_icdm_attribute_expect_success(attributes.RegisteredClients)
            try:
                for client in registeredClients:
                    await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=client.checkInNodeID))
            except InteractionModelError as e:
                asserts.assert_fail(f"Unexpected error returned : {e}")
            finally:
                await self.send_test_event_triggers(eventTrigger=ICDTestEventTriggerOperations.kRemoveActiveModeReq)


if __name__ == "__main__":
    default_matter_test_main()
