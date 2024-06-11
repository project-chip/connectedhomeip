
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
import logging
import os

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)

kRootEndpointId = 0

cluster = Clusters.Objects.IcdManagement
commands = cluster.Commands
monitoredRegistration = cluster.Structs.MonitoringRegistrationStruct


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
        steps = [
            TestStep("0a", "Commissioning, already done",
                     is_commissioning=True),
            TestStep(
                "0b", "TH reads from the DUT the RegisteredClients attribute. RegisteredClients is empty."),
            TestStep(
                "1a", "TH reads from the DUT the ClientsSupportedPerFabric attribute."),
            TestStep(
                "1b", "TH reads from the DUT the ICDCounter attribute."),
            TestStep(2, "TH sends RegisterClient command."),
            TestStep(3, "TH reads from the DUT the RegisteredClients attribute."),
            TestStep(
                4, "If len(RegisteredClients) is less than ClientsSupportedPerFabric, TH repeats RegisterClient command with different CheckInNodeID(s) until the number of entries in RegisteredClients equals ClientsSupportedPerFabric."),
            TestStep(5, "TH reads from the DUT the RegisteredClients attribute."),
            TestStep(
                6, "TH sends RegisterClient command with a different CheckInNodeID."),
            TestStep(
                7, "TTH sends UnregisterClient command with the CheckInNodeID from Step 6."),
            TestStep(
                8, "TH sends UnregisterClient command with the CheckInNodeID from Step 2."),
            TestStep(
                9, "TH reads from the DUT the RegisteredClients attribute."),
            TestStep(
                10, "Repeat Step 9 with the rest of CheckInNodeIDs from the list of RegisteredClients from Step 4, if any."),
            TestStep(11, "TH reads from the DUT the RegisteredClients attribute."),
            TestStep(
                12, "TH sends UnregisterClient command with the CheckInNodeID from Step 2."),
        ]
        return steps

    def pics_TC_ICDM_3_1(self) -> list[str]:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        pics = [
            "ICDM.S",
            "ICDM.S.F00"
        ]
        return pics

    #
    # ICDM 3.1 Test Body
    #

    @async_test_body
    async def test_TC_ICDM_3_1(self):

        cluster = Clusters.Objects.IcdManagement
        attributes = cluster.Attributes

        # Pre-Condition: Commissioning
        self.step("0a")

        # Pre-Condition: Empty RegisteredClients attribute for all registrations
        self.step("0b")
        registeredClients = await self._read_icdm_attribute_expect_success(
            attributes.RegisteredClients)

        for client in registeredClients:
            try:
                await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=client.checkInNodeID))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")
                pass

        self.step("1a")
        if self.pics_guard(self.check_pics("ICDM.S.A0005")):
            clientsSupportedPerFabric = await self._read_icdm_attribute_expect_success(
                attributes.ClientsSupportedPerFabric)

        self.step("1b")
        if self.pics_guard(self.check_pics("ICDM.S.A0005")):
            icdCounter = await self._read_icdm_attribute_expect_success(attributes.ICDCounter)

        self.step(2)
        if self.pics_guard(self.check_pics("ICDM.S.C00.Rsp")):
            try:
                response = await self._send_single_icdm_command(commands.RegisterClient(checkInNodeID=kStep2CheckInNodeId, monitoredSubject=kStep2MonitoredSubjectStep2, key=kStep2Key))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")
                pass

            # Validate response contains the ICDCounter
            asserts.assert_greater_equal(response.ICDCounter, icdCounter,
                                         "The ICDCounter in the response does not match the read ICDCounter.")

        self.step(3)
        if self.pics_guard(self.check_pics("ICDM.S.A0003")):
            registeredClients = await self._read_icdm_attribute_expect_success(
                attributes.RegisteredClients)
            # Validate list size
            asserts.assert_equal(len(registeredClients), 1,
                                 "The expected length of RegisteredClients is 1. List has the wrong size.")

            # Validate entry values
            asserts.assert_equal(
                registeredClients[0].checkInNodeID, kStep2CheckInNodeId, "The read attribute does not match the registered value.")
            asserts.assert_equal(
                registeredClients[0].monitoredSubject, kStep2MonitoredSubjectStep2, "The read attribute does not match the registered value.")

        self.step(4)
        if self.pics_guard(self.check_pics("ICDM.S.C00.Rsp")):
            if (len(registeredClients) < clientsSupportedPerFabric):
                newClients = []
                # Generate new clients data
                for i in range(clientsSupportedPerFabric - len(registeredClients)):
                    newClients.append({
                        "checkInNodeID": i + 1,
                        "monitoredSubject": i + 1,
                        "key": os.urandom(16)
                    })

            for client in newClients:
                try:
                    response = await self._send_single_icdm_command(commands.RegisterClient(checkInNodeID=client["checkInNodeID"], monitoredSubject=client["monitoredSubject"], key=client["key"]))
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, "Unexpected error returned")
                    pass

                # Validate response contains the ICDCounter
                asserts.assert_greater_equal(response.ICDCounter, icdCounter,
                                             "The ICDCounter in the response does not match the read ICDCounter.")

        self.step(5)
        if self.pics_guard(self.check_pics("ICDM.S.A0003")):
            registeredClients = await self._read_icdm_attribute_expect_success(
                attributes.RegisteredClients)

            # Validate list size
            asserts.assert_equal(len(registeredClients[1:]), len(newClients),
                                 "The expected length of RegisteredClients is clientsSupportedPerFabric. List has the wrong size.")

            for client, expectedClient in zip(registeredClients[1:], newClients):
                asserts.assert_equal(
                    client.checkInNodeID, expectedClient["checkInNodeID"], "The read attribute does not match the registered value.")
                asserts.assert_equal(
                    client.monitoredSubject, expectedClient["monitoredSubject"], "The read attribute does not match the registered value.")

        self.step(6)
        if self.pics_guard(self.check_pics("ICDM.S.C00.Rsp")):
            try:
                response = await self._send_single_icdm_command(commands.RegisterClient(checkInNodeID=0xFFFF, monitoredSubject=0xFFFF, key=os.urandom(16)))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.ResourceExhausted, "Unexpected error returned")
                pass

        self.step(7)
        if self.pics_guard(self.check_pics("ICDM.S.C02.Rsp")):
            try:
                await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=0xFFFF))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.NotFound, "Unexpected error returned")
                pass

        self.step(8)
        if self.pics_guard(self.check_pics("ICDM.S.C02.Rsp")):
            try:
                await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=kStep2CheckInNodeId))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")
                pass

        self.step(9)
        if self.pics_guard(self.check_pics("ICDM.S.A0003")):
            registeredClients = await self._read_icdm_attribute_expect_success(
                attributes.RegisteredClients)

            for client in registeredClients:
                asserts.assert_not_equal(client.checkInNodeID, kStep2CheckInNodeId,
                                         "CheckInNodeID was unregistered. It should not be present in the attribute list.")

        self.step(10)
        if self.pics_guard(self.check_pics("ICDM.S.C02.Rsp")):
            for client in newClients:
                try:
                    await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=client["checkInNodeID"]))
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, "Unexpected error returned")
                    pass

        self.step(11)
        if self.pics_guard(self.check_pics("ICDM.S.A0003")):
            registeredClients = await self._read_icdm_attribute_expect_success(
                attributes.RegisteredClients)

            asserts.assert_true(
                not registeredClients, "This list should empty. An element did not get deleted.")

        self.step(12)
        if self.pics_guard(self.check_pics("ICDM.S.C02.Rsp")):
            try:
                await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=kStep2CheckInNodeId))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.NotFound, "Unexpected error returned")
                pass


if __name__ == "__main__":
    default_matter_test_main()
