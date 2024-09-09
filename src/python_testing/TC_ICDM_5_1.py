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
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mdns_discovery import mdns_discovery
from mobly import asserts

Cluster = Clusters.Objects.IcdManagement
Commands = Cluster.Commands
Attributes = Cluster.Attributes
OperatingModeEnum = Cluster.Enums.OperatingModeEnum
ClientTypeEnum = Cluster.Enums.ClientTypeEnum


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
    key=bytes([x for x in range(0x10, 0x20)]),
    clientType=ClientTypeEnum.kEphemeral
)


class TC_ICDM_5_1(MatterBaseTest):

    #
    # Class Helper functions
    #

    async def _read_icdm_attribute_expect_success(self, attribute) -> OperatingModeEnum:
        return await self.read_single_attribute_check_success(endpoint=kRootEndpointId, cluster=Cluster, attribute=attribute)

    async def _send_single_icdm_command(self, command):
        return await self.send_single_cmd(command, endpoint=kRootEndpointId)

    async def _get_icd_txt_record(self) -> OperatingModeEnum:
        discovery = mdns_discovery.MdnsDiscovery(verbose_logging=True)
        service = await discovery.get_operational_service(
            node_id=self.dut_node_id,
            compressed_fabric_id=self.default_controller.GetCompressedFabricId(),
            log_output=True, discovery_timeout_sec=240)

        asserts.assert_is_not_none(
            service, f"Failed to get operational node service information for {self.dut_node_id} on {self.default_controller.GetCompressedFabricId()}")

        icdTxtRecord = OperatingModeEnum(int(service.txt_record['ICD']))
        if icdTxtRecord.value != int(service.txt_record['ICD']):
            raise AttributeError(f'Not a known ICD type: {service.txt_record["ICD"]}')

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
                Attributes.RegisteredClients)

            for client in registeredClients:
                try:
                    await self._send_single_icdm_command(Commands.UnregisterClient(checkInNodeID=client.checkInNodeID))
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, "Unexpected error returned")

            self.step("2a")
            operatingMode = await self._read_icdm_attribute_expect_success(Attributes.OperatingMode)
            asserts.assert_equal(operatingMode, OperatingModeEnum.kSit)

            self.step("2b")
            icdTxtRecord = await self._get_icd_txt_record()
            asserts.assert_equal(icdTxtRecord, OperatingModeEnum.kSit, "OperatingMode Is not in SIT mode.")

            self.step("3a")
            try:
                await self._send_single_icdm_command(Commands.RegisterClient(checkInNodeID=client1.checkInNodeID, monitoredSubject=client1.subjectId, key=client1.key, clientType=client1.clientType))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")

            self.step("3b")
            operatingMode = await self._read_icdm_attribute_expect_success(Attributes.OperatingMode)
            asserts.assert_equal(operatingMode, OperatingModeEnum.kLit)

            self.step("3c")
            icdTxtRecord = await self._get_icd_txt_record()
            asserts.assert_equal(icdTxtRecord, OperatingModeEnum.kLit, "OperatingMode Is not in Lit mode.")

            self.step(4)
            try:
                await self._send_single_icdm_command(Commands.UnregisterClient(checkInNodeID=client1.checkInNodeID))
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, "Unexpected error returned")

            self.step("5a")
            operatingMode = await self._read_icdm_attribute_expect_success(Attributes.OperatingMode)
            asserts.assert_equal(operatingMode, OperatingModeEnum.kSit)

            self.step("5b")
            icdTxtRecord = await self._get_icd_txt_record()
            asserts.assert_equal(icdTxtRecord, OperatingModeEnum.kSit, "OperatingMode Is not in SIT mode.")

        finally:
            registeredClients = await self._read_icdm_attribute_expect_success(
                Attributes.RegisteredClients)

            for client in registeredClients:
                try:
                    await self._send_single_icdm_command(Commands.UnregisterClient(checkInNodeID=client.checkInNodeID))
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, "Unexpected error returned")


if __name__ == "__main__":
    default_matter_test_main()
