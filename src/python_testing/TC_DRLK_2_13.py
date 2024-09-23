#
#    Copyright (c) 2024 Project CHIP Authors
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
# test-runner-runs: run1
# test-runner-run/run1/app: ${CHIP_LOCK_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import random
from dataclasses import dataclass

import chip.clusters as Clusters
from chip.clusters.Attribute import EventPriority
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts

logger = logging.getLogger(__name__)

cluster = Clusters.DoorLock


@dataclass
class AliroAttributeVerify:
    th_step: str
    attribute: Clusters.DoorLock.Attributes
    attribute_value: bytes


class TC_DRLK_2_13(MatterBaseTest):

    def steps_TC_DRLK_2_13(self) -> list[TestStep]:
        steps = [
            TestStep("0", "Commissoning with DUT is done", is_commissioning=True),
            TestStep("1a", "TH reads OperationalCredentials cluster's CurrentFabricIndex and save the attribute",
                     "TH Reads Attribute Successfully"),
            TestStep("1b", "TH sends ClearUser Command to DUT with the UserIndex as 0xFFFE to clear all the users",
                     "DUT responds with Success response"),
            TestStep("1c", "TH sends ClearCredential Command to DUT to clear all the credentials",
                     "Verify that the DUT sends SUCCESS response"),
            TestStep("2a",
                     "TH sends ClearAliroReaderConfig Command to DUT for clearing any existing Aliro Configuration",
                     "Verify that the DUT sends SUCCESS response"),
            TestStep("2b", ",TH sends SetAliroReaderConfig Command to DUT without GroupResolvingKey",
                     "DUT sends success response"),
            TestStep("2c", "TH sends SetAliroReaderConfig Command to DUT with GroupResolvingKey",
                     "DUT sends success response"),
            TestStep("3", "TH reads AliroReaderVerificationKey attribute from DUT",
                     "Verify that AliroReaderVerificationKey value is same as 'verificationkey'"),
            TestStep("4", "TH reads AliroReaderGroupIdentifier attribute from DUT",
                     "Verify that AliroReaderGroupIdentifier value is same as 'groupidentifier'"),
            TestStep("5", "TH reads AliroGroupResolvingKey attribute from DUT",
                     "Verify that AliroGroupResolvingKey value is same 'groupresolvingkey'"),
            TestStep("6a", ",TH sends SetAliroReaderConfig Command to DUT without GroupResolvingKey",
                     "Verify that the DUT sends INVALID_IN_STATE response"),
            TestStep("6b", "TH sends SetAliroReaderConfig Command to DUT with GroupResolvingKey",
                     "Verify that the DUT sends INVALID_IN_STATE response"),
            TestStep("7", "TH sends ClearAliroReaderConfig Command to DUT",
                     "Verify that the DUT sends SUCCESS response"),
            TestStep("8", "TH reads AliroReaderVerificationKey attribute from DUT",
                     "Verify that AliroReaderVerificationKey value is null"),
            TestStep("9", "TH reads AliroReaderGroupIdentifier attribute from DUT",
                     "Verify that AliroReaderGroupIdentifier value is null"),
            TestStep("10", "TH reads AliroGroupResolvingKey attribute from DUT",
                     "Verify that AliroGroupResolvingKey value is null"),
            TestStep("11a", ",TH sends SetAliroReaderConfig Command to DUT without GroupResolvingKey",
                     "DUT sends success response"),
            TestStep("11b", "TH sends SetAliroReaderConfig Command to DUT with GroupResolvingKey",
                     "DUT sends success response"),
            TestStep("12a",
                     "TH reads NumberOfAliroEndpointKeysSupported store as 'max_aliro_keys_supported' if max_aliro_keys_supported>= 2 continue with the next steps,"
                     "Verify that Read operation is successful"),
            TestStep("12b",
                     "TH sends SetUser Command to DUT to create an Aliro user using username as 'AliroUser' and unique id 111",
                     "Verify that the DUT sends SUCCESS response"""),
            TestStep("13a", "TH sends SetCredential Command Credential as (6, 2)",
                     "Verify that the DUT responds with SetCredentialResponse command, DUT responds with status success in SetCredentialResponse"),
            TestStep("13b", "TH sends SetCredential Command with Credential as (6, 1)",
                     "Verify that the DUT responds with SetCredentialResponse command, DUT responds with status success in SetCredentialResponse"),
            TestStep("14", "TH reads the LockUserChange event list from DUT",
                     "Verify list has an event LockDataType: 11 as latest event with DataOperationType: 0(DataOperationTypeEnum.Add) along with other information"),
            TestStep("15", "TH sends GetCredentialStatus Command with Credential as (6,1)",
                     "Verify DUT responds with GetCredentialStatusResponse having CredentialExists is true and UserIndex as 1"),
            TestStep("16",
                     "TH sends ClearCredential Command to DUT to clear the CredentialType of AliroCredentialIssuerKey",
                     "Verify that the DUT sends SUCCESS response"),
            TestStep("17", "TH reads the LockUserChange event list from DUT",
                     "Verify list has an event LockDataType: 11 as latest event with DataOperationType: 1(DataOperationTypeEnum.Clear) along with other information"),
            TestStep("18", "TH sends SetCredential Command to DUT with CredentialType as AliroEvictableEndpointKey",
                     "Verify that the DUT responds with SetCredentialResponse command, DUT responds with status success in SetCredentialResponse"),
            TestStep("19", "TH reads the LockUserChange event list from DUT",
                     "Verify list has an event LockDataType: 11 as latest event with DataOperationType: 0(DataOperationTypeEnum.Add) along with other information"),
            TestStep("20", "TH sends GetCredentialStatus Command with Credential as 7 1",
                     "Verify DUT responds with GetCredentialStatusResponse having CredentialExists is true and UserIndex as 1"),
            TestStep("21",
                     "TH sends ClearCredential Command to DUT to clear the CredentialType of AliroEvictableEndpointKey",
                     "Verify that the DUT sends SUCCESS response"),
            TestStep("22", "TH reads the LockUserChange event list from DUT",
                     "Verify list has an event LockDataType: 12 as latest event with DataOperationType: 1(DataOperationTypeEnum.Clear) along with other information"),
            TestStep("23", "TH sends SetCredential Command to DUT with CredentialType as AliroNonEvictableEndpointKey ",
                     "Verify that the DUT responds with SetCredentialResponse command and Status success."),
            TestStep("24", "TH reads the LockUserChange event list from DUT",
                     "Verify list has an event LockDataType: 12 as latest event with DataOperationType: 0(DataOperationTypeEnum.Add) along with other information"),
            TestStep("25", "TH sends GetCredentialStatus Command with Credential as 8 1",
                     "Verify DUT responds with GetCredentialStatusResponse having CredentialExists is true and UserIndex as 1"),
            TestStep("26",
                     "TH sends ClearCredential Command to DUT to clear the CredentialType of AliroNonEvictableEndpointKey",
                     "DUT sends SUCCESS response"),
            TestStep("27", "TH reads the LockUserChange event list from DUT",
                     "Verify list has an event LockDataType: 13 as latest event with DataOperationType: 1(DataOperationTypeEnum.Clear) along with other information"),
            TestStep("28a", "Th Reads NumberOfCredentialsSupportedPerUser saves as numberofcredentialsupportedperuser,"
                            "Read operation is successful"),
            TestStep("28b", "TH sends ClearCredential Command to DUT to clear all the credentials of Aliro type "
                            "TH sends ClearUser Command with UserIndex as 1 to DUT to clear alirouser"
                            "Executing steps 29 to 35 only when 'max_aliro_keys_supported <= numberofcredentialsupportedperuser' else exit script",
                     "Verify that Read operation is successful"),
            TestStep("29a", "TH sends SetUser Command to DUT to create an Aliro user",
                     "DUT sends SUCCESS response"),
            TestStep("29b",
                     "TH performs repeated number of SetCredential commands with credentials as 8 'startcredentialindex' until 'max_aliro_keys_supported - 1', startcredentialindex initially has value 1 .",
                     "Verify that the DUT responds with SetCredentialResponse command and Status success."),
            TestStep("30",
                     "TH sends SetCredential Command to DUT with CredentialType as AliroEvictableEndpointKey for the 'alirouser' ",
                     "Verify that the DUT responds with SetCredentialResponse command and Status success. This step will fill the last slot with credentialType as AliroEvictableEndpointKey"),
            TestStep("31",
                     "TH sends SetCredential Command to DUT with CredentialType as AliroNonEvictableEndpointKey and number of credentials for 'alirouser' exceeds the max_aliro_keys_supported",
                     "Verify that the DUT responds with SetCredentialResponse command and Status ResourceExhausted"),
            TestStep("32", "TH sends GetCredentialStatus Command with Credential as 7 1",
                     "DUT responds with GetCredentialStatusResponse Command and CredentialExists is True"),
            TestStep("33", "TH sends GetCredentialStatus Command with Credential as 8 max_aliro_keys_supported",
                     "Verify DUT responds with GetCredentialStatusResponse having CredentialExists is false and UserIndex as 1"),
            TestStep("34", "TH sends ClearCredential Command to DUT to clear the ALIRO CredentialType",
                     "Verify that the DUT sends SUCCESS response"),
            TestStep("35", "TH sends ClearUser Command to DUT with the UserIndex as 1",
                     "Verify that the DUT sends SUCCESS response"),
        ]

        return steps

    async def read_attributes_from_dut(self, endpoint, cluster, attribute, expected_status: Status = Status.Success):
        try:
            attribute_value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                             attribute=attribute)
            asserts.assert_equal(expected_status, Status.Success)
            return attribute_value
        except Exception as e:
            logging.error(e)
            asserts.assert_equal(expected_status, Status.Success,
                                 f"Error reading attributes, response={attribute_value}")

    async def aliro_attribute_verifiers(self, aliro_attribute_verify_steps: list[AliroAttributeVerify]):

        for aliro_attribute_step in aliro_attribute_verify_steps:
            th_step = aliro_attribute_step.th_step
            self.step(th_step)
            if aliro_attribute_step.attribute == Clusters.DoorLock.Attributes.AliroReaderVerificationKey:
                pics_condition = self.pics_guard(self.check_pics("DRLK.S.F0d") and self.check_pics("DRLK.S.A0080"))
            elif aliro_attribute_step.attribute == Clusters.DoorLock.Attributes.AliroReaderGroupIdentifier:
                pics_condition = self.pics_guard(self.check_pics("DRLK.S.F0d") and self.check_pics("DRLK.S.A0081"))
            elif aliro_attribute_step.attribute == Clusters.DoorLock.Attributes.AliroGroupResolvingKey:
                pics_condition = self.pics_guard(self.check_pics("DRLK.S.F0d") and self.check_pics("DRLK.S.A0084"))
            else:
                pics_condition = False

            if pics_condition:
                dut_aliro_key = await self.read_attributes_from_dut(endpoint=self.app_cluster_endpoint,
                                                                    cluster=Clusters.Objects.DoorLock,
                                                                    attribute=aliro_attribute_step.attribute)
                asserts.assert_equal(dut_aliro_key, aliro_attribute_step.attribute_value,
                                     f"Aliro Attribute key verification Failed, readAttributeResponse{dut_aliro_key}")

    def pics_TC_DRLK_2_13(self) -> list[str]:
        return ["DRLK.S.F0d"]

    async def send_clear_user_cmd(self, user_index, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.ClearUser(userIndex=user_index),
                                       endpoint=self.app_cluster_endpoint,
                                       timedRequestTimeoutMs=1000)
            asserts.assert_equal(expected_status, Status.Success)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, f"Unexpected error returned: {e}")

    async def send_clear_aliro_reader_config_cmd(self, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.ClearAliroReaderConfig(),
                                       endpoint=self.app_cluster_endpoint,
                                       timedRequestTimeoutMs=1000)
            asserts.assert_equal(expected_status, Status.Success)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, f"Unexpected error returned: {e}")

    async def send_set_aliro_reader_config_cmd(self, use_group_resolving_key: bool,
                                               expected_status: Status = Status.Success):
        try:
            # Checks Pics condition
            if use_group_resolving_key is False:
                pics_check = self.pics_guard(self.check_pics("DRLK.S.F0d") and not self.check_pics("DRLK.S.F0e") and
                                             self.check_pics("DRLK.S.C28.Rsp"))

            else:
                pics_check = self.pics_guard(self.check_pics("DRLK.S.F0e") and self.check_pics("DRLK.S.C28.Rsp"))

            if not use_group_resolving_key and pics_check:
                await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.SetAliroReaderConfig(
                    signingKey=self.signingKey,
                    verificationKey=self.verificationKey,
                    groupIdentifier=self.groupIdentifier),
                    endpoint=self.app_cluster_endpoint,
                    timedRequestTimeoutMs=1000)
                asserts.assert_equal(expected_status, Status.Success)
            elif use_group_resolving_key and pics_check:
                await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.SetAliroReaderConfig(
                    signingKey=self.signingKey,
                    verificationKey=self.verificationKey,
                    groupIdentifier=self.groupIdentifier,
                    groupResolvingKey=self.groupResolvingKey),
                    endpoint=self.app_cluster_endpoint,
                    timedRequestTimeoutMs=1000)
                asserts.assert_equal(expected_status, Status.Success)
        except InteractionModelError as e:
            logging.exception(f"Got exception when performing SetAliroReaderConfig {e}")
            asserts.assert_equal(e.status, expected_status, f"Unexpected error returned: {e}")

    async def get_credentials_status(self, credentialIndex: int, credentialType: cluster.Enums.CredentialTypeEnum,
                                     step, userIndex, credential_exists=True):
        if step:
            self.step(step)
        try:
            flags = ["DRLK.S.F0d", "DRLK.S.C24.Rsp", "DRLK.S.C25.Tx"]
            if self.pics_guard(all([self.check_pics(p) for p in flags])):
                credentials_struct = cluster.Structs.CredentialStruct(credentialIndex=credentialIndex,
                                                                      credentialType=credentialType)
                response = await self.send_single_cmd(endpoint=self.app_cluster_endpoint, timedRequestTimeoutMs=1000,
                                                      cmd=cluster.Commands.GetCredentialStatus(
                                                          credential=credentials_struct))
                asserts.assert_true(type_matches(response, Clusters.DoorLock.Commands.GetCredentialStatusResponse),
                                    "Unexpected return type for GetCredentialStatus")
                asserts.assert_true(response.credentialExists == credential_exists,
                                    "Error when executing GetCredentialStatus command, credentialExists={}".format(
                                        str(response.credentialExists)))
                asserts.assert_equal(userIndex, response.userIndex,
                                     f"User Index is not matching, UserIndex={response.userIndex}")
                return response
        except InteractionModelError as e:
            logging.error(e)
            asserts.assert_equal(e.status, Status.Success, f"Unexpected error returned: {e}")

    async def set_credential_cmd(self, credential_enum: Clusters.DoorLock.Enums.CredentialTypeEnum, credentialIndex,
                                 operationType, userIndex, credentialData, userStatus, userType, step=None,
                                 expected_status: Status = Status.Success):
        if step:
            self.step(step)
        credentials = cluster.Structs.CredentialStruct(
            credentialType=credential_enum,
            credentialIndex=credentialIndex)
        if self.pics_guard(self.check_pics("DRLK.S.F0d") and self.check_pics("DRLK.S.C22.Rsp")):
            try:
                response = await self.send_single_cmd(cmd=Clusters.Objects.DoorLock.Commands.SetCredential(
                    operationType=operationType,
                    credential=credentials,
                    credentialData=credentialData,
                    userStatus=userStatus,
                    userType=userType,
                    userIndex=userIndex),
                    endpoint=self.app_cluster_endpoint,
                    timedRequestTimeoutMs=1000)
                asserts.assert_true(type_matches(response, Clusters.Objects.DoorLock.Commands.SetCredentialResponse),
                                    "Unexpected return type for SetCredential")
                asserts.assert_true(response.status == expected_status,
                                    "Error sending SetCredential command, status={}".format(str(response.status)))
            except InteractionModelError as e:
                logging.exception(e)
                asserts.assert_equal(e.status, Status.Success, f"Unexpected error returned: {e}")

    async def read_and_validate_lock_event_change(self, verify_lock_data_type, priority, operation_source,
                                                  data_type_operation, fabric_index, node_id, data_index, user_index):
        response_event_list = await self.default_controller.ReadEvent(self.dut_node_id,
                                                                      [1, cluster.Events.LockUserChange])
        for event_read_result in reversed(response_event_list):
            if not hasattr(event_read_result.Data, "lockDataType"):
                continue
            if event_read_result.Data.lockDataType == verify_lock_data_type:
                asserts.assert_equal(event_read_result.Header.Priority, priority,
                                     f"Priority mismatch, Priority={event_read_result.Header.Priority}")
                asserts.assert_equal(event_read_result.Data.operationSource, operation_source,
                                     f"OperationSource not matching, operationSource={operation_source}")
                asserts.assert_equal(event_read_result.Data.fabricIndex, fabric_index,
                                     f"FabricIndex not matching, fabricIndex={fabric_index}")
                asserts.assert_equal(event_read_result.Data.dataIndex, data_index,
                                     f"DataIndex not matching, dataIndex={data_index}")
                asserts.assert_equal(event_read_result.Data.sourceNode, node_id,
                                     f"SourceNode not matching, sourceNode={node_id}")
                asserts.assert_equal(event_read_result.Data.userIndex, user_index,
                                     f"UserIndex not matching, userIndex={user_index}")
                asserts.assert_equal(event_read_result.Data.dataOperationType, data_type_operation,
                                     f"DataOperationType not matching, dataOperationType={data_type_operation}")
                break

    async def clear_credentials_cmd(self, credential, step=None, expected_status: Status = Status.Success):
        try:
            if step is not None:
                self.step(step)
            if self.pics_guard(self.check_pics("DRLK.S.F0d") and self.check_pics("DRLK.S.C26.Rsp")):
                await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.ClearCredential(credential=credential),
                                           endpoint=self.app_cluster_endpoint,
                                           timedRequestTimeoutMs=1000)
                asserts.assert_equal(expected_status, Status.Success)
        except InteractionModelError as e:
            logging.exception(e)
            asserts.assert_equal(e.status, expected_status, f"Unexpected error returned: {e}")

    async def clear_all_aliro_credential(self):
        all_aliro_cred_list = []
        aliro_cred_issuer_credentials = cluster.Structs.CredentialStruct(credentialIndex=1,
                                                                         credentialType=Clusters.DoorLock.Enums.CredentialTypeEnum.kAliroCredentialIssuerKey)
        all_aliro_cred_list.append(aliro_cred_issuer_credentials)
        aliro_evict_endpoint_credentials = cluster.Structs.CredentialStruct(credentialIndex=1,
                                                                            credentialType=Clusters.DoorLock.Enums.CredentialTypeEnum.kAliroEvictableEndpointKey)
        all_aliro_cred_list.append(aliro_evict_endpoint_credentials)
        aliro_non_evict_endpoint_credentials = cluster.Structs.CredentialStruct(credentialIndex=1,
                                                                                credentialType=Clusters.DoorLock.Enums.CredentialTypeEnum.kAliroNonEvictableEndpointKey)
        all_aliro_cred_list.append(aliro_non_evict_endpoint_credentials)
        for creds in all_aliro_cred_list:
            await self.clear_credentials_cmd(credential=creds)

    def generate_unique_octbytes(self, length=65) -> bytes:
        return ''.join(random.choices('01234567', k=length)).encode()

    @async_test_body
    async def test_TC_DRLK_2_13(self):
        self.max_aliro_keys_supported = None
        self.numberofcredentialsupportedperuser = None
        self.signingKey = bytes.fromhex("89d085fc302ca53e279bfcdecdf3c4adb2f5d9bc9ea6c49e9566d144367df3ff")
        self.verificationKey = bytes.fromhex(
            "047a4c992d753924cdf3779a3c84fec2debaa6f0b3084450878acc7ddcce7856ae57b1ebbe2561015103dd7474c2a183675378ec55f1e465ac3436bf3dd5ca54d4")
        self.groupIdentifier = bytes.fromhex("89d085fc302ca53e279bfcdecdf3c4ad")
        self.groupResolvingKey = bytes.fromhex("89d0859bfcdecdf3c4adfc302ca53e27")
        self.common_cluster_endpoint = 0
        self.app_cluster_endpoint = self.matter_test_config.endpoint
        self.alirouser = "AliroUser"
        self.alirocredentialissuerkey = bytes.fromhex(
            "047a4c882d753924cdf3779a3c84fec2debaa6f0b3084450878acc7ddcce7856ae57b1ebbe2561015103dd7474c2a183675378ec55f1e465ac3436bf3dd5ca54d4")
        self.alirocredentialissuerkey_secondary = bytes.fromhex(
            "047a4c112d753924cdf3779a3c84fec2debaa6f0b3084450878acc7ddcce7856ae57b1ebbe2561015103dd7474c2a183675378ec55f1e465ac3436bf3dd5ca54d4")
        self.aliroevictableendpointkey = bytes.fromhex(
            "047a4c772d753924cdf3779a3c84fec2debaa6f0b3084450878acc7ddcce7856ae57b1ebbe2561015103dd7474c2a183675378ec55f1e465ac3436bf3dd5ca54d4")
        self.alirononevictableendpointkey = bytes.fromhex(
            "047a4c662d753924cdf3779a3c84fec2debaa6f0b3084450878acc7ddcce7856ae57b1ebbe2561015103dd7474c2a183675378ec55f1e465ac3436bf3dd5ca54d4")
        self.alirononevictableendpointkey1 = bytes.fromhex(
            "047a4c552d753924cdf3779a3c84fec2debaa6f0b3084450878acc7ddcce7856ae57b1ebbe2561015103dd7474c2a183675378ec55f1e465ac3436bf3dd5ca54d4")
        #  step 1 TH reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute
        self.step("0")
        self.step("1a")
        self.fabric_idx1 = await self.read_attributes_from_dut(endpoint=self.common_cluster_endpoint,
                                                               cluster=Clusters.Objects.OperationalCredentials,
                                                               attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
                                                               )
        self.step("1b")
        if self.pics_guard(self.check_pics("DRLK.S.C1d.Rsp")):
            await self.send_clear_user_cmd(user_index=int(0xFFFE))

        self.step("1c")
        if self.pics_guard(self.check_pics("DRLK.S.C26.Rsp")):
            await self.clear_credentials_cmd(credential=NullValue)

        # step 2
        self.step("2a")
        if self.pics_guard(self.check_pics("DRLK.S.C26.Rsp")):
            await self.send_clear_aliro_reader_config_cmd()
        self.step("2b")
        await self.send_set_aliro_reader_config_cmd(use_group_resolving_key=False, expected_status=Status.Success)
        self.step("2c")
        await self.send_set_aliro_reader_config_cmd(use_group_resolving_key=True, expected_status=Status.Success)
        # step 3,4,5
        aliro_attribute_verify_steps = [
            AliroAttributeVerify(th_step="3", attribute=Clusters.Objects.DoorLock.Attributes.AliroReaderVerificationKey,
                                 attribute_value=self.verificationKey),
            AliroAttributeVerify(th_step="4", attribute=Clusters.Objects.DoorLock.Attributes.AliroReaderGroupIdentifier,
                                 attribute_value=self.groupIdentifier),
            AliroAttributeVerify(th_step="5", attribute=Clusters.Objects.DoorLock.Attributes.AliroGroupResolvingKey,
                                 attribute_value=self.groupResolvingKey)
        ]
        await self.aliro_attribute_verifiers(aliro_attribute_verify_steps=aliro_attribute_verify_steps)

        #  step 6
        self.step("6a")
        await self.send_set_aliro_reader_config_cmd(use_group_resolving_key=False,
                                                    expected_status=Status.InvalidInState)
        self.step("6b")
        await self.send_set_aliro_reader_config_cmd(use_group_resolving_key=True, expected_status=Status.InvalidInState)
        #  step 7
        self.step("7")
        if self.check_pics("DRLK.S.C26.Rsp"):
            await self.send_clear_aliro_reader_config_cmd()

        # steps 8,9,10
        aliro_attribute_verify_steps = [
            AliroAttributeVerify(th_step="8", attribute=Clusters.Objects.DoorLock.Attributes.AliroReaderVerificationKey,
                                 attribute_value=NullValue),
            AliroAttributeVerify(th_step="9", attribute=Clusters.Objects.DoorLock.Attributes.AliroReaderGroupIdentifier,
                                 attribute_value=NullValue),
            AliroAttributeVerify(th_step="10", attribute=Clusters.Objects.DoorLock.Attributes.AliroGroupResolvingKey,
                                 attribute_value=NullValue)
        ]
        await self.aliro_attribute_verifiers(aliro_attribute_verify_steps=aliro_attribute_verify_steps)

        # step 11
        self.step("11a")
        await self.send_set_aliro_reader_config_cmd(use_group_resolving_key=False, expected_status=Status.Success)
        self.step("11b")
        await self.send_set_aliro_reader_config_cmd(use_group_resolving_key=True, expected_status=Status.Success)

        #  step 12 Setting User
        self.step("12a")
        if self.pics_guard(self.check_pics("DRLK.S.A0088")):
            self.max_aliro_keys_supported = await self.read_attributes_from_dut(endpoint=self.app_cluster_endpoint,
                                                                                cluster=Clusters.Objects.DoorLock,
                                                                                attribute=Clusters.DoorLock.Attributes.NumberOfAliroEndpointKeysSupported)
            if self.max_aliro_keys_supported < 2:
                self.skip_all_remaining_steps("13")
                return
        self.step("12b")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C1a.Rsp")):
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.DoorLock.Commands.SetUser(
                    operationType=Clusters.DoorLock.Enums.DataOperationTypeEnum.kAdd,
                    userIndex=1,
                    userName=self.alirouser,
                    userUniqueID=111,
                    userStatus=Clusters.DoorLock.Enums.UserStatusEnum.kOccupiedEnabled,
                    userType=Clusters.DoorLock.Enums.UserTypeEnum.kUnrestrictedUser,
                    credentialRule=Clusters.DoorLock.Enums.CredentialRuleEnum.kSingle),
                    endpoint=self.app_cluster_endpoint,
                    timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                logging.exception(e)
                asserts.assert_equal(e.status, Status.Success, f"Unexpected error returned: {e}")

        # step 13
        self.step("13a")
        # The first SetCredentialsCommand is done as a placeholder for the user slot as ClearCredentials removes user info as well
        await self.set_credential_cmd(credentialData=self.alirononevictableendpointkey1,
                                      operationType=cluster.Enums.DataOperationTypeEnum.kAdd,
                                      credential_enum=cluster.Enums.CredentialTypeEnum.kAliroNonEvictableEndpointKey,
                                      credentialIndex=2, userIndex=1, userStatus=NullValue, userType=NullValue)
        self.step("13b")
        await self.set_credential_cmd(credentialData=self.alirocredentialissuerkey,
                                      operationType=cluster.Enums.DataOperationTypeEnum.kAdd,
                                      credential_enum=cluster.Enums.CredentialTypeEnum.kAliroCredentialIssuerKey,
                                      credentialIndex=1, userIndex=1, userStatus=NullValue, userType=NullValue)

        # step 14
        self.step("14")
        await self.read_and_validate_lock_event_change(
            verify_lock_data_type=Clusters.DoorLock.Enums.LockDataTypeEnum.kAliroCredentialIssuerKey,
            operation_source=Clusters.DoorLock.Enums.OperationSourceEnum.kRemote, user_index=1,
            data_index=1, fabric_index=self.fabric_idx1, node_id=self.matter_test_config.controller_node_id,
            data_type_operation=Clusters.DoorLock.Enums.DataOperationTypeEnum.kAdd, priority=EventPriority.INFO
        )

        # step 15
        self.step("15")
        await self.get_credentials_status(credentialIndex=1,
                                          credentialType=cluster.Enums.CredentialTypeEnum.kAliroCredentialIssuerKey,
                                          step=None, userIndex=1)

        # step 16
        credentials = cluster.Structs.CredentialStruct(credentialIndex=1,
                                                       credentialType=cluster.Enums.CredentialTypeEnum.kAliroCredentialIssuerKey)
        await self.clear_credentials_cmd(step="16", credential=credentials)

        # step 17
        self.step("17")
        await self.read_and_validate_lock_event_change(
            verify_lock_data_type=Clusters.DoorLock.Enums.LockDataTypeEnum.kAliroCredentialIssuerKey,
            operation_source=Clusters.DoorLock.Enums.OperationSourceEnum.kRemote, user_index=1,
            data_index=1, fabric_index=self.fabric_idx1, node_id=self.matter_test_config.controller_node_id,
            data_type_operation=Clusters.DoorLock.Enums.DataOperationTypeEnum.kClear, priority=EventPriority.INFO
        )

        # step 18
        await self.set_credential_cmd(step="18", credentialData=self.aliroevictableendpointkey,
                                      operationType=cluster.Enums.DataOperationTypeEnum.kAdd,
                                      credential_enum=cluster.Enums.CredentialTypeEnum.kAliroEvictableEndpointKey,
                                      credentialIndex=1, userIndex=1, userStatus=NullValue, userType=NullValue)
        # step 19
        self.step("19")
        await self.read_and_validate_lock_event_change(
            verify_lock_data_type=Clusters.DoorLock.Enums.LockDataTypeEnum.kAliroEvictableEndpointKey,
            operation_source=Clusters.DoorLock.Enums.OperationSourceEnum.kRemote, user_index=1,
            data_index=1, fabric_index=self.fabric_idx1, node_id=self.matter_test_config.controller_node_id,
            data_type_operation=Clusters.DoorLock.Enums.DataOperationTypeEnum.kAdd, priority=EventPriority.INFO
        )
        # step 20
        await self.get_credentials_status(credentialIndex=1,
                                          credentialType=cluster.Enums.CredentialTypeEnum.kAliroEvictableEndpointKey,
                                          step="20", userIndex=1)
        # step 21
        evictable_credentials = cluster.Structs.CredentialStruct(credentialIndex=1,
                                                                 credentialType=cluster.Enums.CredentialTypeEnum.kAliroEvictableEndpointKey)
        await self.clear_credentials_cmd(step="21", credential=evictable_credentials)

        # step 22
        self.step("22")
        await self.read_and_validate_lock_event_change(
            verify_lock_data_type=Clusters.DoorLock.Enums.LockDataTypeEnum.kAliroEvictableEndpointKey,
            operation_source=Clusters.DoorLock.Enums.OperationSourceEnum.kRemote, user_index=1,
            data_index=1, fabric_index=self.fabric_idx1, node_id=self.matter_test_config.controller_node_id,
            data_type_operation=Clusters.DoorLock.Enums.DataOperationTypeEnum.kClear, priority=EventPriority.INFO
        )

        # step 23
        await self.set_credential_cmd(step="23", credentialData=self.alirononevictableendpointkey,
                                      operationType=cluster.Enums.DataOperationTypeEnum.kAdd,
                                      credential_enum=cluster.Enums.CredentialTypeEnum.kAliroNonEvictableEndpointKey,
                                      credentialIndex=1, userIndex=1, userStatus=NullValue, userType=NullValue)
        # step 24
        self.step("24")
        await self.read_and_validate_lock_event_change(
            verify_lock_data_type=Clusters.DoorLock.Enums.LockDataTypeEnum.kAliroNonEvictableEndpointKey,
            operation_source=Clusters.DoorLock.Enums.OperationSourceEnum.kRemote, user_index=1,
            data_index=1, fabric_index=self.fabric_idx1, node_id=self.matter_test_config.controller_node_id,
            data_type_operation=Clusters.DoorLock.Enums.DataOperationTypeEnum.kAdd, priority=EventPriority.INFO
        )

        # step 25
        await self.get_credentials_status(step="25", credentialIndex=1,
                                          credentialType=cluster.Enums.CredentialTypeEnum.kAliroNonEvictableEndpointKey,
                                          userIndex=1)

        # step 26
        non_evictable_credentials = cluster.Structs.CredentialStruct(credentialIndex=1,
                                                                     credentialType=cluster.Enums.CredentialTypeEnum.kAliroNonEvictableEndpointKey)
        await self.clear_credentials_cmd(step="26", credential=non_evictable_credentials)

        # step 27
        self.step("27")
        await self.read_and_validate_lock_event_change(
            verify_lock_data_type=Clusters.DoorLock.Enums.LockDataTypeEnum.kAliroNonEvictableEndpointKey,
            operation_source=Clusters.DoorLock.Enums.OperationSourceEnum.kRemote, user_index=1,
            data_index=1, fabric_index=self.fabric_idx1, node_id=self.matter_test_config.controller_node_id,
            data_type_operation=Clusters.DoorLock.Enums.DataOperationTypeEnum.kClear, priority=EventPriority.INFO
        )

        # step 28
        self.step("28a")
        if self.check_pics("DRLK.S.A001c"):
            self.numberofcredentialsupportedperuser = await self.read_attributes_from_dut(
                endpoint=self.app_cluster_endpoint,
                cluster=Clusters.Objects.DoorLock,
                attribute=Clusters.DoorLock.Attributes.NumberOfCredentialsSupportedPerUser)
        logging.info(f"After reading  attribute NumberOfCredentialsSupportedPerUser we get"
                     f" value {self.numberofcredentialsupportedperuser} ")
        self.step("28b")
        await self.clear_all_aliro_credential()
        await self.send_clear_user_cmd(user_index=1)
        if self.pics_guard(self.check_pics("DRLK.S.A001c") and self.check_pics("DRLK.S.A0088")):
            if self.max_aliro_keys_supported > self.numberofcredentialsupportedperuser:
                logging.info(
                    "Skipping execution from Step 29a to step 35 since 'max_aliro_keys_supported > numberofcredentialsupportedperuser' as per test plan spec")
                self.skip_all_remaining_steps("29a")
            else:
                # Perform setUser as we have removed user in previous step and SetCredentials always expects a user with index to be present
                self.step("29a")
                if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C1a.Rsp")):
                    try:
                        await self.send_single_cmd(cmd=Clusters.Objects.DoorLock.Commands.SetUser(
                            operationType=Clusters.DoorLock.Enums.DataOperationTypeEnum.kAdd,
                            userIndex=1,
                            userName="alirouser",
                            userUniqueID=111,
                            userStatus=Clusters.DoorLock.Enums.UserStatusEnum.kOccupiedEnabled,
                            userType=Clusters.DoorLock.Enums.UserTypeEnum.kUnrestrictedUser,
                            credentialRule=Clusters.DoorLock.Enums.CredentialRuleEnum.kSingle),
                            endpoint=self.app_cluster_endpoint,
                            timedRequestTimeoutMs=1000)
                    except InteractionModelError as e:
                        logging.exception(e)
                        asserts.assert_equal(e.status, Status.Success, f"Unexpected error returned: {e}")
                self.step("29b")
                logging.info("setting 'start_credential_index' to value 1 ")
                start_credential_index = 1
                credentials_data = self.alirononevictableendpointkey
                while 1:
                    if start_credential_index <= (self.max_aliro_keys_supported - 1):
                        if start_credential_index != 1:
                            credentials_data = self.generate_unique_octbytes()

                        await self.set_credential_cmd(credentialData=credentials_data,
                                                      operationType=cluster.Enums.DataOperationTypeEnum.kAdd,
                                                      credential_enum=cluster.Enums.CredentialTypeEnum.kAliroNonEvictableEndpointKey,
                                                      credentialIndex=start_credential_index, userIndex=1,
                                                      userStatus=NullValue,
                                                      userType=NullValue)
                        start_credential_index += 1
                        logging.info(f"The updated value of start_credential_index is {start_credential_index}")
                    else:
                        break
                self.step("30")
                await self.set_credential_cmd(credentialData=self.alirononevictableendpointkey,
                                              operationType=cluster.Enums.DataOperationTypeEnum.kAdd,
                                              credential_enum=cluster.Enums.CredentialTypeEnum.kAliroEvictableEndpointKey,
                                              credentialIndex=1, userIndex=1, userStatus=NullValue,
                                              userType=NullValue)

                # step 31
                self.step("31")
                await self.set_credential_cmd(credentialData=self.alirononevictableendpointkey1,
                                              operationType=cluster.Enums.DataOperationTypeEnum.kAdd,
                                              credential_enum=cluster.Enums.CredentialTypeEnum.kAliroNonEvictableEndpointKey,
                                              credentialIndex=self.max_aliro_keys_supported, userIndex=1, userStatus=NullValue,
                                              userType=NullValue,
                                              expected_status=Status.ResourceExhausted)
                # step 32
                await self.get_credentials_status(step="32", credentialIndex=1,
                                                  credentialType=cluster.Enums.CredentialTypeEnum.kAliroEvictableEndpointKey,
                                                  credential_exists=True, userIndex=1)

                # step 33
                await self.get_credentials_status(step="33", credentialIndex=self.max_aliro_keys_supported,
                                                  credentialType=cluster.Enums.CredentialTypeEnum.kAliroNonEvictableEndpointKey,
                                                  credential_exists=False, userIndex=NullValue)
                # step 34
                self.step("34")
                if self.pics_guard(self.check_pics("DRLK.S.C26.Rsp")):
                    await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.ClearCredential(credential=NullValue),
                                               endpoint=self.app_cluster_endpoint,
                                               timedRequestTimeoutMs=1000)

                # step 35
                self.step("35")
                await self.send_clear_user_cmd(user_index=1)


if __name__ == '__main__':
    default_matter_test_main()
