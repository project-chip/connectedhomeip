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
from dataclasses import dataclass

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from chip.interaction_model import InteractionModelError, Status
from mdns_discovery import mdns_discovery

cluster = Clusters.Objects.IcdManagement
commands = cluster.Commands
attributes = cluster.Attributes
modes = cluster.Enums.OperatingModeEnum
ClientTypeEnum = cluster.Enums.ClientTypeEnum


@dataclass
class Client:
    checkInNodeID: int
    subjectId: int
    key: bytes
    clientType: ClientTypeEnum


logger = logging.getLogger(__name__)
kRootEndpointId = 0

client1 = Client(
    checkInNodeID=1,
    subjectId=1,
    key=b"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f",
    clientType=ClientTypeEnum.kEphemeral
)


class TC_ICDM_5_1(MatterBaseTest):

    #
    # Class Helper functions
    #

    async def _read_icdm_attribute_expect_success(self, attribute):
        return await self.read_single_attribute_check_success(endpoint=kRootEndpointId, cluster=cluster, attribute=attribute)

    async def _send_single_icdm_command(self, command):
        return await self.send_single_cmd(command, endpoint=kRootEndpointId)

    async def _get_icd_txt_record(self):
        discovery = mdns_discovery.MdnsDiscovery()
        localCompressedNodeId = hex(self.default_controller.GetCompressedFabricId())
        dutNodeId = hex(self.dut_node_id)

        services = await discovery.get_all_services(log_output=False)
        icdTxtRecord = modes.kUnknownEnumValue.value

        for service in services[mdns_discovery.MdnsServiceType.OPERATIONAL.value]:

            # Generate hex for the compressedFabric and NodeId
            serviceName = service.instance_name.split("-")
            compressedFabric = hex(int(serviceName[0], 16))
            nodeId = hex(int(serviceName[1], 16))

            # Find the correct MdnsServiceEntry
            if localCompressedNodeId == compressedFabric and dutNodeId == nodeId:
                icdTxtRecord = service.txt_record['ICD']

        return icdTxtRecord

    #
    # Test Harness Helpers
    #

    def desc_TC_ICDM_5_1(self) -> str:
        """Returns a description of this test"""
        return "[TC-ICDM-5.1] Operating Mode with DUT as Server"

    def steps_TC_ICDM_5_1(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads from the DUT the RegisteredClients attribute"),
            TestStep("2a", "TH reads from the DUT the OperatingMode attribute."),
            TestStep("2b", "Verify that the ICD DNS-SD TXT key is present."),
            TestStep("3a", "TH sends RegisterClient command."),
            TestStep("3b", "TH reads from the DUT the OperatingMode attribute."),
            TestStep("3c", "Verify that mDNS is advertising ICD key."),
            TestStep(4, "TH sends UnregisterClient command with CheckInNodeID1."),
            TestStep("5a", "TH reads from the DUT the OperatingMode attribute."),
            TestStep("5b", "Verify that the ICD DNS-SD TXT key is present."),
        ]
        return steps

    def pics_TC_ICDM_5_1(self) -> list[str]:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        pics = [
            "ICDM.S",
            "ICDM.S.F02",
        ]
        return pics

    #
    # ICDM 5.1 Test Body
    #

    @async_test_body
    async def test_TC_ICDM_5_1(self):

        # Commissioning
        self.step(0)

        try:
            self.step(1)
            registeredClients = await self._read_icdm_attribute_expect_success(
                attributes.RegisteredClients)

            for client in registeredClients:
                try:
                    await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=client.checkInNodeID))
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, "Unexpected error returned")

            self.step("2a")
            operatingMode = await self._read_icdm_attribute_expect_success(attributes.OperatingMode)
            asserts.assert_equal(operatingMode, modes.kSit.value)

            self.step("2b")
            icdTxtRecord = await self._get_icd_txt_record()
            asserts.assert_equal(int(icdTxtRecord), modes.kSit.value, "OperatingMode Is not in SIT mode.")

            self.step("3a")
            try:
                await self._send_single_icdm_command(commands.RegisterClient(checkInNodeID=client1.checkInNodeID, monitoredSubject=client1.subjectId, key=client1.key, clientType=client1.clientType))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")

            self.step("3b")
            operatingMode = await self._read_icdm_attribute_expect_success(attributes.OperatingMode)
            asserts.assert_equal(operatingMode, modes.kLit.value)

            self.step("3c")
            icdTxtRecord = await self._get_icd_txt_record()
            asserts.assert_equal(int(icdTxtRecord), modes.kLit.value, "OperatingMode Is not in Lit mode.")

            self.step(4)
            try:
                await self._send_single_icdm_command(commands.UnregisterClient(checkInNodeID=client1.checkInNodeID))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")

            self.step("5a")
            operatingMode = await self._read_icdm_attribute_expect_success(attributes.OperatingMode)
            asserts.assert_equal(operatingMode, modes.kSit.value)

            self.step("5b")
            icdTxtRecord = await self._get_icd_txt_record()
            asserts.assert_equal(int(icdTxtRecord), modes.kSit.value, "OperatingMode Is not in SIT mode.")

        finally:
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
