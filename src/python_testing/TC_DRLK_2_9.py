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
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import random
import string

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from drlk_2_x_common import DRLK_COMMON
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts

logger = logging.getLogger(__name__)

drlkcluster = Clusters.DoorLock


class TC_DRLK_2_9(MatterBaseTest, DRLK_COMMON):

    def steps_TC_DRLK_2_9(self) -> list[TestStep]:
        steps = [

            TestStep("1", "TH reads NumberOfTotalUsersSupported attribute.",
                     "Verify that TH is able to read the attribute successfully."),
            TestStep("2a", "TH sends SetUser Command to DUT.", "Verify that the DUT sends SUCCESS response"),
            TestStep("2b", "TH reads MinPINCodeLength attribute .",
                     "Verify that TH is able to read the attribute successfully and value is within range."),
            TestStep("2c", "TH reads MaxPINCodeLength attribute.",
                     "Verify that TH is able to read the attribute successfully and value is within range."),
            TestStep("2d", "TH reads MinRFIDCodeLength attribute.",
                     "Verify that TH is able to read the attribute successfully."),
            TestStep("2e", "TH reads MaxRFIDCodeLength attribute.",
                     "Verify that TH is able to read the attribute successfully and value is within range."),
            TestStep("2f", "TH sends SetCredential Command to DUT.",
                     "Verify that the DUT responds with SetCredentialResponse command with Status SUCCESS."),
            TestStep("3", "TH sends GetCredentialStatus Command .",
                     "Verify that the DUT responds with  GetCredentialStatusResponse Command ."),
            TestStep("4", "TH sends SetCredential Command to DUT.",
                     "Verify that the DUT responds with SetCredentialResponse command with Status INVALID_COMMAND."),
            TestStep("5", "TH sends SetCredential Command to DUT.",
                     "Verify that the DUT responds with SetCredentialResponse command with Status DUPLICATE."),
            TestStep("6a", "TH sends SetCredential Command to DUT.",
                     "Verify that the DUT responds with SetCredentialResponse command with Status DUPLICATE or OCCUPIED."),
            TestStep("6b", "TH sends SetCredential Command to DUT.",
                     "Verify that the DUT responds with SetCredentialResponse command with Status DUPLICATE or OCCUPIED."),
            TestStep("7", "TH sends SetCredential Command to DUT.",
                     "Verify that the DUT responds with SetCredentialResponse command with Status INVALID_COMMAND."),
            TestStep("8", "TH sends ClearCredential Command to DUT.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("9a", "TH sends GetCredentialStatus command to DUT ",
                     "Verify that the DUT responds with  GetCredentialStatusResponse Command."),
            TestStep("9b", "TH sends ClearUser Command to DUT.",
                     "Verify that the DUT sends SUCCESS response"),
            TestStep("10", "TH sends SetUser Command to DUT.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("11", "TH sends SetCredential Command to DUT.",
                     "Verify that the DUT responds with SetCredentialResponse command with Status SUCCESS."),
            TestStep("12a", "TH sends ClearCredential Command to DUT.",
                     " Verify that the DUT sends SUCCESS response."),
            TestStep("13", " TH sends GetCredentialStatus Command.",
                     "Verify that the DUT responds with  GetCredentialStatusResponse Command with credentialExists=FALSE."),
            TestStep("14a", "TH sends ClearCredential Command to DUT",
                     "if {PICS_SF_ALIRO} verify that the DUT sends a SUCCESS response else DUT sends an INVALID_COMMAND response."),
            TestStep("14b", "TH sends ClearCredential Command to DUT with Invalid Credential Type.",
                     "Verify that the DUT sends an INVALID_COMMAND."),
            TestStep("14c", "TH sends ClearUser Command to DUT to clear all the users.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("14d", "TH reads NumberOfPINUsersSupported attribute.",
                     "Verify that TH is able to read the attribute successfully and value is within range."),
            TestStep("15a", "TH reads NumberOfCredentialsSupportedPerUser attribute from DUT.",
                     "Verify that TH is able to read the attribute successfully and value is within range."),
            TestStep("15b", "TH sends SetUser Command to DUT.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("15c", "TH sends SetCredential Command to DUT.",
                     "Verify that the DUT sends SetCredentialResponse command  with Status SUCCESS."),
            TestStep("15d", " TH sends SetCredential Command  with CredentialIndex as 'max-num-credential-user'.",
                     "Verify that the DUT sends SetCredentialResponse command with Status RESOURCE_EXHAUSTION."),
            TestStep("15e", "TH sends ClearCredential Command to DUT to clear all the credentials of PIN type.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("15f", "TH sends ClearUser Command to DUT with UserIndex as 0xFFFE to clear all the users.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("16", " TH sends SetUser Command to DUT.",
                     "Verify that the DUT sends SUCCESS response"),
            TestStep("17", "TH sends SetCredential Command to DUT with CredentialType.RFID.",
                     "Verify that the DUT sends SetCredentialResponse command  with Status SUCCESS."),
            TestStep("18", "TH sends SetCredential Command to DUT with CredentialType.PIN.",
                     "Verify that the DUT sends SetCredentialResponse command  with Status SUCCESS."),
            TestStep("19", "TH sends GetUser Command to DUT.",
                     "Verify that the DUT responds with GetUserResponse Command."),
            TestStep("20", "TH sends SetCredential Command to DUT to modify the CredentialData.",
                     "Verify that the DUT sends SetCredentialResponse command  with Status SUCCESS."),
            TestStep("21", "TH sends GetUser Command to DUT.",
                     "Verify that the DUT responds with GetUserResponse Command."),
            TestStep("22", "TH sends ClearCredential Command to DUT to clear all the credentials.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("23", "TH sends ClearUser Command to DUT to clear all the users.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("24", "TH sends SetUser Command to DUT.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("25", "TH sends ClearAliroReaderConfig Command to DUT.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("26", ",TH sends SetAliroReaderConfig Command to DUT without GroupResolvingKey.",
                     "Verify that DUT sends success response."),
            TestStep("27", ",TH sends SetAliroReaderConfig Command to DUT with GroupResolvingKey.",
                     "Verify that DUT sends success response."),
            TestStep("28", "TH sends SetCredential Command CredentialType as AliroEvictableEndpointKey.",
                     "Verify that the DUT responds with SetCredentialResponse commad with status success "),
            TestStep("29", "TH sends SetCredential Command to DUT with CredentialType.PIN.",
                     "Verify that the DUT sends SetCredentialResponse command  with Status SUCCESS."),
            TestStep("30", "TH sends GetUser Command to DUT.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("31", "TH sends SetCredential Command to modify PIN CredentialType",
                     "Verify that the DUT responds with SetCredentialResponse command with status success "),
            TestStep("32", "TH sends GetUser Command to DUT.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("33", "TH sends SetCredential Command to modify AliroEvictableEndpointKey CredentialType",
                     "Verify that the DUT responds with SetCredentialResponse command with status success "),
            TestStep("34", "TH sends GetUser Command to DUT.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("35", "TH sends ClearUser Command to DUT.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("36", "TH sends ClearCredential Command to DUT to clear all the credentials.",
                     "Verify that the DUT sends SUCCESS response."),
            TestStep("37", "TH sends ClearAliroReaderConfig Command to DUT.",
                     "Verify that the DUT sends SUCCESS response."), ]

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

    def pics_TC_DRLK_2_9(self) -> list[str]:
        return ["DRLK.S"]

    async def generate_max_pincode_len(self, maxPincodeLength):
        return ''.join(random.choices(string.digits, k=maxPincodeLength))

    async def generate_code(self):
        if (self.maxpincodelength is not None):
            validpincodestr = await self.generate_max_pincode_len(self.maxpincodelength)
            self.pin_code = bytes(validpincodestr, 'ascii')
            validpincodestr = await self.generate_max_pincode_len(self.maxpincodelength)
            self.pin_code1 = bytes(validpincodestr, 'ascii')
            validpincodestr = await self.generate_max_pincode_len(self.maxpincodelength)
            self.pin_code2 = bytes(validpincodestr, 'ascii')
            inavlidpincodestr = await self.generate_max_pincode_len(self.maxpincodelength+1)
            self.inavlid_pincode = bytes(inavlidpincodestr, 'ascii')
        if (self.maxrfidcodelength is not None):
            validpincodestr = await self.generate_max_pincode_len(self.maxrfidcodelength)
            self.rfid_tag = bytes(validpincodestr, 'ascii')

    async def send_clear_user_cmd(self, user_index, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.ClearUser(userIndex=user_index),
                                       endpoint=self.app_cluster_endpoint,
                                       timedRequestTimeoutMs=1000)
            asserts.assert_equal(expected_status, Status.Success)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, f"Unexpected error returned: {e}")

    async def get_user(self, userindex, username, useruniqueid, credentiallist, expected_status: Status = Status.Success):
        try:
            response = await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.GetUser(userIndex=userindex),
                                                  endpoint=self.app_cluster_endpoint,
                                                  timedRequestTimeoutMs=1000)

            asserts.assert_true(type_matches(response, Clusters.DoorLock.Commands.GetUserResponse),
                                "Unexpected return type for GetUserResponse")
            asserts.assert_true(response.userIndex == userindex,
                                "Error when executing GetUserResponse command, userIndex={}".format(
                                    str(response.userIndex)))
            asserts.assert_true(response.userName == username,
                                "Error when executing GetUserResponse command, userName={}".format(
                                    str(response.userName)))
            asserts.assert_true(response.userUniqueID == useruniqueid,
                                "Error when executing GetUserResponse command, userUniqueID={}".format(
                                    str(response.userUniqueID)))
            logging.info("Credentials value is GetUserResponse Command %s" % (str(response.credentials)))

            asserts.assert_equal(len(credentiallist), len(response.credentials),  "Error mismatch in expected credential from GetUserResponse command = {}".format(
                str(credentiallist)))
            # traverse through input credentials and match each value with the resonse credential
            for input_credential_index in range(len(credentiallist)):
                match_found = False
                for response_credential_index in range(len(response.credentials)):
                    if (response.credentials[response_credential_index] == credentiallist[input_credential_index]):
                        match_found = True
                        break
                asserts.assert_equal(match_found, True,  "Error mismatch in expected credential from GetUserResponse command = {}".format(
                    str(credentiallist)))

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, f"Unexpected error returned: {e}")

    async def get_credentials_status(self, credentialIndex: int, credentialType: drlkcluster.Enums.CredentialTypeEnum, credential_exists, next_credential_index):

        try:
            credentials_struct = drlkcluster.Structs.CredentialStruct(credentialIndex=credentialIndex,
                                                                      credentialType=credentialType)
            response = await self.send_single_cmd(endpoint=self.app_cluster_endpoint, timedRequestTimeoutMs=1000,
                                                  cmd=drlkcluster.Commands.GetCredentialStatus(
                                                      credential=credentials_struct))
            asserts.assert_true(type_matches(response, Clusters.DoorLock.Commands.GetCredentialStatusResponse),
                                "Unexpected return type for GetCredentialStatus")
            asserts.assert_true(response.credentialExists == credential_exists,
                                "Error when executing GetCredentialStatus command, credentialExists={}".format(
                                    str(response.credentialExists)))
            if (not credential_exists):
                asserts.assert_true(response.userIndex == NullValue,
                                    "Error when executing GetCredentialStatus command, userIndex={}".format(
                                        str(response.userIndex)))
            asserts.assert_true(response.nextCredentialIndex == next_credential_index,
                                "Error when executing GetCredentialStatus command, nextCredentialIndex={}".format(
                                    str(response.nextCredentialIndex)))
            return response
        except InteractionModelError as e:
            logging.error(e)
            asserts.assert_equal(e.status, Status.Success, f"Unexpected error returned: {e}")

    async def set_credential_cmd(self, credential_enum: drlkcluster.Enums.CredentialTypeEnum, statuscode, credentialIndex,
                                 operationType, userIndex, credentialData, userStatus, userType):
        custom_status_code = 149

        credentials = drlkcluster.Structs.CredentialStruct(
            credentialType=credential_enum,
            credentialIndex=credentialIndex)
        try:

            logging.info("Credential Data is %s" % (credentialData))
            response = await self.send_single_cmd(cmd=drlkcluster.Commands.SetCredential(
                operationType=operationType,
                credential=credentials,
                credentialData=credentialData,
                userStatus=userStatus,
                userType=userType,
                userIndex=userIndex),
                endpoint=self.app_cluster_endpoint,
                timedRequestTimeoutMs=1000)
            asserts.assert_true(type_matches(response, drlkcluster.Commands.SetCredentialResponse),
                                "Unexpected return type for SetCredential")
            asserts.assert_equal(response.userIndex, NullValue)
            if (statuscode != custom_status_code):
                asserts.assert_true(response.status == statuscode,
                                    "Error sending SetCredential command, status={}".format(str(response.status)))
            else:
                asserts.assert_true(response.status == 2 or response.status == 3,
                                    "Error sending SetCredential command, status={}".format(str(response.status)))
            return response.nextCredentialIndex
        except InteractionModelError as e:
            logging.exception(e)
            asserts.assert_equal(e.status, statuscode, f"Unexpected error returned: {e}")
            return -1

    async def clear_credentials_cmd(self, credential, expected_status: Status = Status.Success):
        try:

            await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.ClearCredential(credential=credential),
                                       endpoint=self.app_cluster_endpoint,
                                       timedRequestTimeoutMs=1000)
        except InteractionModelError as e:
            logging.exception(e)
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

            signingKey = bytes.fromhex("89d085fc302ca53e279bfcdecdf3c4adb2f5d9bc9ea6c49e9566d144367df3ff")
            verificationKey = bytes.fromhex(
                "047a4c992d753924cdf3779a3c84fec2debaa6f0b3084450878acc7ddcce7856ae57b1ebbe2561015103dd7474c2a183675378ec55f1e465ac3436bf3dd5ca54d4")
            groupIdentifier = bytes.fromhex("89d085fc302ca53e279bfcdecdf3c4ad")
            groupResolvingKey = bytes.fromhex("89d0859bfcdecdf3c4adfc302ca53e27")

            # Checks Pics condition
            if use_group_resolving_key is False:
                pics_check = self.pics_guard(self.check_pics("DRLK.S.F0d") and not self.check_pics("DRLK.S.F0e") and
                                             self.check_pics("DRLK.S.C28.Rsp"))
            else:
                pics_check = self.pics_guard(self.check_pics("DRLK.S.F0e") and self.check_pics("DRLK.S.C28.Rsp"))

            if not use_group_resolving_key and pics_check:
                await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.SetAliroReaderConfig(
                    signingKey=signingKey,
                    verificationKey=verificationKey,
                    groupIdentifier=groupIdentifier),
                    endpoint=self.app_cluster_endpoint,
                    timedRequestTimeoutMs=1000)
                asserts.assert_equal(expected_status, Status.Success)
            elif use_group_resolving_key and pics_check:
                await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.SetAliroReaderConfig(
                    signingKey=signingKey,
                    verificationKey=verificationKey,
                    groupIdentifier=groupIdentifier,
                    groupResolvingKey=groupResolvingKey),
                    endpoint=self.app_cluster_endpoint,
                    timedRequestTimeoutMs=1000)
                asserts.assert_equal(expected_status, Status.Success)
        except InteractionModelError as e:
            logging.exception(f"Got exception when performing SetAliroReaderConfig {e}")
            asserts.assert_equal(e.status, expected_status, f"Unexpected error returned: {e}")

    @async_test_body
    async def test_TC_DRLK_2_9(self):

        self.Attrib = 0
        numberofcredentialsupportedperuser = None
        self.app_cluster_endpoint = 1
        invalid_credential_type = 9
        user_unique_id = 6459
        user_name = "xxx"
        credentialIndex_1 = 1
        credentialIndex_2 = 2
        credentialIndex_3 = 3
        userIndex_1 = 1
        userIndex_2 = 2
        invalid_user_status = 5
        invalid_user_type = 10

        self.pin_code = b""
        self.pin_code1 = b""
        self.pin_code2 = b""
        self.inavlid_pincode = b""
        self.rfid_tag = b""

        self.minpincodelength = None
        self.maxpincodelength = None
        self.maxrfidcodelength = None
        self.minrfidcodelength = None

        self.endpoint = self.user_params.get("endpoint", 1)
        print("endpoint", self.endpoint)

        # Aliro Keys for setting Aliro configuration and credential

        aliroevictableendpointkey1 = bytes.fromhex(
            "047a4c772d753924cdf3779a3c84fec2debaa6f0b3084450878acc7ddcce7856ae57b1ebbe2561015103dd7474c2a183675378ec55f1e465ac3436bf3dd5ca54d4")

        aliroevictableendpointkey2 = bytes.fromhex(
            "047a4c662d753924cdf3779a3c84fec2debaa6f0b3084450878acc7ddcce7856ae57b1ebbe2561015103dd7474c2a183675378ec55f1e465ac3436bf3dd5ca54d4")

        self.step("1")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.A0011")):
            self.numberoftotaluserssupported = await self.read_attributes_from_dut(endpoint=self.app_cluster_endpoint,
                                                                                   cluster=drlkcluster,
                                                                                   attribute=Clusters.DoorLock.Attributes.NumberOfTotalUsersSupported)
            asserts.assert_in(self.numberoftotaluserssupported, range(
                0, 65534), "NumberOfTotalUsersSupported value is out of range")
        self.step("2a")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C1a.Rsp")):
            try:
                await self.send_single_cmd(cmd=drlkcluster.Commands.SetUser(
                    operationType=Clusters.DoorLock.Enums.DataOperationTypeEnum.kAdd,
                    userIndex=userIndex_1,
                    userName=user_name,
                    userUniqueID=user_unique_id,
                    userStatus=Clusters.DoorLock.Enums.UserStatusEnum.kOccupiedEnabled,
                    userType=Clusters.DoorLock.Enums.UserTypeEnum.kUnrestrictedUser,
                    credentialRule=Clusters.DoorLock.Enums.CredentialRuleEnum.kSingle),
                    endpoint=self.app_cluster_endpoint,
                    timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                logging.exception(e)

        self.step("2b")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.F00")):
            self.minpincodelength = await self.read_attributes_from_dut(endpoint=self.app_cluster_endpoint,
                                                                        cluster=drlkcluster,
                                                                        attribute=Clusters.DoorLock.Attributes.MinPINCodeLength)
            asserts.assert_in(self.minpincodelength, range(
                0, 255), "MinPINCodeLength value is out of range")
        self.step("2c")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.F00")):
            self.maxpincodelength = await self.read_attributes_from_dut(endpoint=self.app_cluster_endpoint,
                                                                        cluster=drlkcluster,
                                                                        attribute=Clusters.DoorLock.Attributes.MaxPINCodeLength)
            asserts.assert_in(self.maxpincodelength, range(
                0, 255), "MaxPINCodeLength value is out of range")
        self.step("2d")
        if self.pics_guard(self.check_pics("DRLK.S.F01")):
            self.minrfidcodelength = await self.read_attributes_from_dut(endpoint=self.app_cluster_endpoint,
                                                                         cluster=drlkcluster,
                                                                         attribute=Clusters.DoorLock.Attributes.MinRFIDCodeLength)
            asserts.assert_in(self.minrfidcodelength, range(
                0, 255), "MinRFIDCodeLength value is out of range")
        self.step("2e")
        if self.pics_guard(self.check_pics("DRLK.S.F01")):
            self.maxrfidcodelength = await self.read_attributes_from_dut(endpoint=self.app_cluster_endpoint,
                                                                         cluster=drlkcluster,
                                                                         attribute=Clusters.DoorLock.Attributes.MaxRFIDCodeLength)
            asserts.assert_in(self.maxrfidcodelength, range(
                0, 255), "MaxRFIDCodeLength value is out of range")
        self.step("2f")
        await self.generate_code()
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F08")
                           and self.check_pics("DRLK.S.C22.Rsp") and self.check_pics("DRLK.S.C23.Tx")):

            await self.set_credential_cmd(credentialData=self.pin_code,
                                          operationType=drlkcluster.Enums.DataOperationTypeEnum.kAdd,
                                          credential_enum=drlkcluster.Enums.CredentialTypeEnum.kPin,
                                          credentialIndex=credentialIndex_1, userIndex=userIndex_1, userStatus=NullValue, userType=NullValue, statuscode=Status.Success)
        self.step("3")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F08")
                           and self.check_pics("DRLK.S.C24.Rsp") and self.check_pics("DRLK.S.C25.Tx")):
            await self.get_credentials_status(credentialIndex=credentialIndex_1,
                                              credentialType=drlkcluster.Enums.CredentialTypeEnum.kPin, credential_exists=True, next_credential_index=NullValue)
        self.step("4")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F08")
                           and self.check_pics("DRLK.S.C22.Rsp") and self.check_pics("DRLK.S.C23.Tx")):

            await self.set_credential_cmd(credentialData=self.inavlid_pincode,
                                          operationType=drlkcluster.Enums.DataOperationTypeEnum.kAdd,
                                          credential_enum=drlkcluster.Enums.CredentialTypeEnum.kPin,
                                          credentialIndex=credentialIndex_2, userIndex=NullValue, userStatus=invalid_user_status, userType=invalid_user_type, statuscode=Status.InvalidCommand)
        self.step("5")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F08")
                           and self.check_pics("DRLK.S.C22.Rsp") and self.check_pics("DRLK.S.C23.Tx")):
            await self.set_credential_cmd(credentialData=self.pin_code,
                                          operationType=drlkcluster.Enums.DataOperationTypeEnum.kAdd,
                                          credential_enum=drlkcluster.Enums.CredentialTypeEnum.kPin,
                                          credentialIndex=credentialIndex_2, userIndex=NullValue, userStatus=NullValue, userType=NullValue, statuscode=2)

        self.step("6a")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F08")
                           and self.check_pics("DRLK.S.C22.Rsp") and self.check_pics("DRLK.S.C23.Tx")):
            await self.set_credential_cmd(credentialData=self.pin_code,
                                          operationType=drlkcluster.Enums.DataOperationTypeEnum.kAdd,
                                          credential_enum=drlkcluster.Enums.CredentialTypeEnum.kPin,
                                          credentialIndex=credentialIndex_1, userIndex=NullValue, userStatus=NullValue, userType=NullValue, statuscode=149)
        self.step("6b")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F08")
                           and self.check_pics("DRLK.S.C22.Rsp") and self.check_pics("DRLK.S.C23.Tx")):
            await self.set_credential_cmd(credentialData=self.pin_code1,
                                          operationType=drlkcluster.Enums.DataOperationTypeEnum.kAdd,
                                          credential_enum=drlkcluster.Enums.CredentialTypeEnum.kPin,
                                          credentialIndex=credentialIndex_1, userIndex=NullValue, userStatus=NullValue, userType=NullValue, statuscode=149)
        self.step("7")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F08")
                           and self.check_pics("DRLK.S.C22.Rsp") and self.check_pics("DRLK.S.C23.Tx")):
            await self.set_credential_cmd(credentialData=self.pin_code2,
                                          operationType=drlkcluster.Enums.DataOperationTypeEnum.kModify,
                                          credential_enum=drlkcluster.Enums.CredentialTypeEnum.kPin,
                                          credentialIndex=credentialIndex_3, userIndex=userIndex_1, userStatus=NullValue, userType=NullValue, statuscode=Status.InvalidCommand)
        self.step("8")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C26.Rsp")):
            credentials = drlkcluster.Structs.CredentialStruct(credentialIndex=1,
                                                               credentialType=drlkcluster.Enums.CredentialTypeEnum.kPin)
            await self.clear_credentials_cmd(credential=credentials)

        self.step("9a")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C24.Rsp")):
            await self.get_credentials_status(credentialIndex=credentialIndex_1,
                                              credentialType=drlkcluster.Enums.CredentialTypeEnum.kPin, credential_exists=False, next_credential_index=NullValue)
        self.step("9b")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C1d.Rsp")):
            await self.send_clear_user_cmd(user_index=1)

        self.step("10")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C1a.Rsp")):
            try:
                await self.send_single_cmd(cmd=drlkcluster.Commands.SetUser(
                    operationType=Clusters.DoorLock.Enums.DataOperationTypeEnum.kAdd,
                    userIndex=userIndex_2,
                    userName=user_name,
                    userUniqueID=user_unique_id,
                    userStatus=Clusters.DoorLock.Enums.UserStatusEnum.kOccupiedEnabled,
                    userType=Clusters.DoorLock.Enums.UserTypeEnum.kUnrestrictedUser,
                    credentialRule=Clusters.DoorLock.Enums.CredentialRuleEnum.kSingle),
                    endpoint=self.app_cluster_endpoint,
                    timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                logging.exception(e)

        self.step("11")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F08")
                           and self.check_pics("DRLK.S.C22.Rsp") and self.check_pics("DRLK.S.C23.Tx")):
            await self.set_credential_cmd(credentialData=self.pin_code,
                                          operationType=drlkcluster.Enums.DataOperationTypeEnum.kAdd,
                                          credential_enum=drlkcluster.Enums.CredentialTypeEnum.kPin,
                                          credentialIndex=credentialIndex_1, userIndex=userIndex_2, userStatus=NullValue, userType=NullValue, statuscode=Status.Success)
        self.step("12a")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C26.Rsp")):
            credentials = drlkcluster.Structs.CredentialStruct(credentialIndex=0xFFFE,
                                                               credentialType=drlkcluster.Enums.CredentialTypeEnum.kPin)
            await self.clear_credentials_cmd(credential=credentials)

        self.step("13")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C24.Rsp") and self.check_pics("DRLK.S.C25.Tx")):
            await self.get_credentials_status(credentialIndex=credentialIndex_1,
                                              credentialType=drlkcluster.Enums.CredentialTypeEnum.kPin, credential_exists=False, next_credential_index=NullValue)
        self.step("14a")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C26.Rsp")):
            feature_map = await self.read_attributes_from_dut(endpoint=self.app_cluster_endpoint,
                                                              cluster=drlkcluster,
                                                              attribute=Clusters.DoorLock.Attributes.FeatureMap)
            aliro_enabled = feature_map & Clusters.DoorLock.Bitmaps.Feature.kAliroProvisioning
            credentials = drlkcluster.Structs.CredentialStruct(credentialIndex=credentialIndex_1,
                                                               credentialType=drlkcluster.Enums.CredentialTypeEnum.kAliroNonEvictableEndpointKey)
            if (aliro_enabled):

                await self.clear_credentials_cmd(credential=credentials)
            else:
                try:
                    await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.ClearCredential(credential=credentials),
                                               endpoint=self.app_cluster_endpoint,
                                               timedRequestTimeoutMs=1000)
                    asserts.fail("Unexpected success in sending ClearCredential Command  with invalid CredentialTpe")

                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.InvalidCommand,
                                         "Unexpected error sending ClearCredential Command  with invalid CredentialTpe")

        self.step("14b")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C26.Rsp")):
            credentials = drlkcluster.Structs.CredentialStruct(credentialIndex=credentialIndex_1,
                                                               credentialType=invalid_credential_type)
            try:
                await self.send_single_cmd(cmd=Clusters.DoorLock.Commands.ClearCredential(credential=credentials),
                                           endpoint=self.app_cluster_endpoint,
                                           timedRequestTimeoutMs=1000)
                asserts.fail("Unexpected success in sending ClearCredential Command  with invalid CredentialTpe")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidCommand,
                                     "Unexpected error sending ClearCredential Command  with invalid CredentialTpe")

        self.step("14c")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C1d.Rsp")):
            await self.send_clear_user_cmd(user_index=int(0xFFFE))

        self.step("14d")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.A0012")):
            num_pin_users_supported = await self.read_attributes_from_dut(endpoint=self.app_cluster_endpoint,
                                                                          cluster=drlkcluster,
                                                                          attribute=Clusters.DoorLock.Attributes.NumberOfPINUsersSupported)
            asserts.assert_in(num_pin_users_supported, range(
                0, 65534), "NumberOfPINUsersSupported value is out of range")
        self.step("15a")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.A001c")):

            numberofcredentialsupportedperuser = await self.read_attributes_from_dut(endpoint=self.app_cluster_endpoint,
                                                                                     cluster=drlkcluster,
                                                                                     attribute=Clusters.DoorLock.Attributes.NumberOfCredentialsSupportedPerUser)
        asserts.assert_in(numberofcredentialsupportedperuser, range(
            0, 255), "NumberOfCredentialsSupportedPerUser value is out of range")
        self.step("15b")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C1a.Rsp")):
            try:
                await self.send_single_cmd(cmd=drlkcluster.Commands.SetUser(
                    operationType=Clusters.DoorLock.Enums.DataOperationTypeEnum.kAdd,
                    userIndex=userIndex_1,
                    userName=user_name,
                    userUniqueID=user_unique_id,
                    userStatus=Clusters.DoorLock.Enums.UserStatusEnum.kOccupiedEnabled,
                    userType=Clusters.DoorLock.Enums.UserTypeEnum.kUnrestrictedUser,
                    credentialRule=Clusters.DoorLock.Enums.CredentialRuleEnum.kSingle),
                    endpoint=self.app_cluster_endpoint,
                    timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                logging.exception(e)

        self.step("15c")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F08")
                           and self.check_pics("DRLK.S.C22.Rsp") and self.check_pics("DRLK.S.C23.Tx")):
            if (numberofcredentialsupportedperuser < num_pin_users_supported):
                logging.info("setting 'start_credential_index' to value 1 ")
                start_credential_index = 1
                nextCredentialIndex = 1
                while 1:
                    uniquePincodeString = await self.generate_max_pincode_len(self.maxpincodelength)
                    uniquePincode = bytes(uniquePincodeString, 'ascii')
                    logging.info("Credential Data value is %s" % (uniquePincode))
                    if start_credential_index <= (numberofcredentialsupportedperuser):
                        nextCredentialIndex = await self.set_credential_cmd(credentialData=uniquePincode,
                                                                            operationType=drlkcluster.Enums.DataOperationTypeEnum.kAdd,
                                                                            credential_enum=drlkcluster.Enums.CredentialTypeEnum.kPin,

                                                                            credentialIndex=start_credential_index, userIndex=userIndex_1, userStatus=NullValue, userType=NullValue, statuscode=Status.Success)
                        logging.info(f"The updated value of nextCredentialIndex is {nextCredentialIndex}")
                        start_credential_index += 1
                        asserts.assert_true(nextCredentialIndex == start_credential_index,
                                            "Error mismatch in expected nextCredentialIndex={}".format(str(nextCredentialIndex)))
                        logging.info(f"The updated value of start_credential_index is {start_credential_index}")
                    else:
                        break
        self.step("15d")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F08")
                           and self.check_pics("DRLK.S.C22.Rsp") and self.check_pics("DRLK.S.C23.Tx")):
            if (numberofcredentialsupportedperuser < num_pin_users_supported):
                await self.set_credential_cmd(credentialData=self.pin_code,
                                              operationType=drlkcluster.Enums.DataOperationTypeEnum.kAdd,
                                              credential_enum=drlkcluster.Enums.CredentialTypeEnum.kPin,
                                              credentialIndex=start_credential_index, userIndex=userIndex_1, userStatus=NullValue, userType=NullValue, statuscode=Status.ResourceExhausted)
        self.step("15e")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C26.Rsp")):
            credentials = drlkcluster.Structs.CredentialStruct(credentialIndex=0xFFFE,
                                                               credentialType=drlkcluster.Enums.CredentialTypeEnum.kPin)
            await self.clear_credentials_cmd(credential=credentials)
        self.step("15f")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C1d.Rsp")):
            await self.send_clear_user_cmd(user_index=int(0xFFFE))

        self.step("16")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C1a.Rsp")):
            try:
                await self.send_single_cmd(cmd=drlkcluster.Commands.SetUser(
                    operationType=Clusters.DoorLock.Enums.DataOperationTypeEnum.kAdd,
                    userIndex=userIndex_1,
                    userName=user_name,
                    userUniqueID=user_unique_id,
                    userStatus=Clusters.DoorLock.Enums.UserStatusEnum.kOccupiedEnabled,
                    userType=Clusters.DoorLock.Enums.UserTypeEnum.kUnrestrictedUser,
                    credentialRule=Clusters.DoorLock.Enums.CredentialRuleEnum.kSingle),
                    endpoint=self.app_cluster_endpoint,
                    timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                logging.exception(e)

        self.step("17")
        if self.pics_guard(self.check_pics("DRLK.S.F01") and self.check_pics("DRLK.S.C22.Rsp")
                           and self.check_pics("DRLK.S.C23.Tx")):
            await self.set_credential_cmd(credentialData=self.rfid_tag,
                                          operationType=drlkcluster.Enums.DataOperationTypeEnum.kAdd,
                                          credential_enum=drlkcluster.Enums.CredentialTypeEnum.kRfid,
                                          credentialIndex=credentialIndex_1, userIndex=userIndex_1, userStatus=NullValue, userType=NullValue, statuscode=Status.Success)
        self.step("18")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.C22.Rsp") and self.check_pics("DRLK.S.C23.Tx")):
            await self.set_credential_cmd(credentialData=self.pin_code,
                                          operationType=drlkcluster.Enums.DataOperationTypeEnum.kAdd,
                                          credential_enum=drlkcluster.Enums.CredentialTypeEnum.kPin,
                                          credentialIndex=credentialIndex_1, userIndex=userIndex_1, userStatus=NullValue, userType=NullValue, statuscode=Status.Success)

        self.step("19")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F01") and self.check_pics("DRLK.S.C1b.Rsp")):

            credentiallist: list[drlkcluster.Structs.CredentialStruct]
            credentiallist = [drlkcluster.Structs.CredentialStruct(credentialIndex=credentialIndex_1,
                                                                   credentialType=drlkcluster.Enums.CredentialTypeEnum.kRfid),
                              drlkcluster.Structs.CredentialStruct(credentialIndex=credentialIndex_1,
                                                                   credentialType=drlkcluster.Enums.CredentialTypeEnum.kPin)]
            await self.get_user(userIndex_1, user_name, user_unique_id, credentiallist, Status.Success)

        self.step("20")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.C22.Rsp") and self.check_pics("DRLK.S.C23.Tx")):
            await self.set_credential_cmd(credentialData=self.pin_code1,
                                          operationType=drlkcluster.Enums.DataOperationTypeEnum.kModify,
                                          credential_enum=drlkcluster.Enums.CredentialTypeEnum.kPin,
                                          credentialIndex=credentialIndex_1, userIndex=userIndex_1, userStatus=NullValue, userType=NullValue, statuscode=Status.Success)

        self.step("21")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F01") and self.check_pics("DRLK.S.C1b.Rsp")):
            await self.get_user(userIndex_1, user_name, user_unique_id, credentiallist, Status.Success)

        self.step("22")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C26.Rsp")):
            await self.clear_credentials_cmd(credential=NullValue)

        self.step("23")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C1d.Rsp")):
            await self.send_clear_user_cmd(userIndex_1)

        self.step("24")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C1a.Rsp")):
            try:
                await self.send_single_cmd(cmd=drlkcluster.Commands.SetUser(
                    operationType=Clusters.DoorLock.Enums.DataOperationTypeEnum.kAdd,
                    userIndex=userIndex_1,
                    userName=user_name,
                    userUniqueID=user_unique_id,
                    userStatus=Clusters.DoorLock.Enums.UserStatusEnum.kOccupiedEnabled,
                    userType=Clusters.DoorLock.Enums.UserTypeEnum.kUnrestrictedUser,
                    credentialRule=Clusters.DoorLock.Enums.CredentialRuleEnum.kSingle),
                    endpoint=self.app_cluster_endpoint,
                    timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                logging.exception(e)

        self.step("25")
        if self.check_pics("DRLK.S.C29.Rsp"):
            await self.send_clear_aliro_reader_config_cmd()

        self.step("26")
        await self.send_set_aliro_reader_config_cmd(use_group_resolving_key=False, expected_status=Status.Success)
        self.step("27")
        await self.send_set_aliro_reader_config_cmd(use_group_resolving_key=True, expected_status=Status.Success)

        self.step("28")
        if self.pics_guard(self.check_pics("DRLK.S.F0d")
                           and self.check_pics("DRLK.S.C22.Rsp") and self.check_pics("DRLK.S.C23.Tx")):
            await self.set_credential_cmd(credentialData=aliroevictableendpointkey1,
                                          operationType=drlkcluster.Enums.DataOperationTypeEnum.kAdd,
                                          credential_enum=drlkcluster.Enums.CredentialTypeEnum.kAliroEvictableEndpointKey,
                                          credentialIndex=credentialIndex_1, userIndex=userIndex_1, userStatus=NullValue, userType=NullValue, statuscode=Status.Success)

        self.step("29")
        if self.pics_guard(self.check_pics("DRLK.S.F00")
                           and self.check_pics("DRLK.S.C22.Rsp") and self.check_pics("DRLK.S.C23.Tx")):
            await self.set_credential_cmd(credentialData=self.pin_code,
                                          operationType=drlkcluster.Enums.DataOperationTypeEnum.kAdd,
                                          credential_enum=drlkcluster.Enums.CredentialTypeEnum.kPin,
                                          credentialIndex=credentialIndex_1, userIndex=userIndex_1, userStatus=NullValue, userType=NullValue, statuscode=Status.Success)
        self.step("30")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F0d") and self.check_pics("DRLK.S.C1b.Rsp")):

            credentiallist: list[drlkcluster.Structs.CredentialStruct]
            credentiallist = [drlkcluster.Structs.CredentialStruct(credentialIndex=credentialIndex_1,
                                                                   credentialType=drlkcluster.Enums.CredentialTypeEnum.kAliroEvictableEndpointKey),
                              drlkcluster.Structs.CredentialStruct(credentialIndex=credentialIndex_1,
                                                                   credentialType=drlkcluster.Enums.CredentialTypeEnum.kPin)]
            await self.get_user(userIndex_1, user_name, user_unique_id, credentiallist, Status.Success)
        self.step("31")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.C22.Rsp") and self.check_pics("DRLK.S.C23.Tx")):
            await self.set_credential_cmd(credentialData=self.pin_code2,
                                          operationType=drlkcluster.Enums.DataOperationTypeEnum.kModify,
                                          credential_enum=drlkcluster.Enums.CredentialTypeEnum.kPin,
                                          credentialIndex=credentialIndex_1, userIndex=userIndex_1, userStatus=NullValue, userType=NullValue, statuscode=Status.Success)

        self.step("32")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F0d") and self.check_pics("DRLK.S.C1b.Rsp")):

            credentiallist: list[drlkcluster.Structs.CredentialStruct]
            credentiallist = [drlkcluster.Structs.CredentialStruct(credentialIndex=credentialIndex_1,
                                                                   credentialType=drlkcluster.Enums.CredentialTypeEnum.kAliroEvictableEndpointKey),
                              drlkcluster.Structs.CredentialStruct(credentialIndex=credentialIndex_1,
                                                                   credentialType=drlkcluster.Enums.CredentialTypeEnum.kPin)]
            await self.get_user(userIndex_1, user_name, user_unique_id, credentiallist, Status.Success)
        self.step("33")
        if self.pics_guard(self.check_pics("DRLK.S.F0d") and self.check_pics("DRLK.S.C22.Rsp") and self.check_pics("DRLK.S.C23.Tx")):
            await self.set_credential_cmd(credentialData=aliroevictableendpointkey2,
                                          operationType=drlkcluster.Enums.DataOperationTypeEnum.kModify,
                                          credential_enum=drlkcluster.Enums.CredentialTypeEnum.kAliroEvictableEndpointKey,
                                          credentialIndex=credentialIndex_1, userIndex=userIndex_1, userStatus=NullValue, userType=NullValue, statuscode=Status.Success)
        self.step("34")
        if self.pics_guard(self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F0d") and self.check_pics("DRLK.S.C1b.Rsp")):

            credentiallist: list[drlkcluster.Structs.CredentialStruct]
            credentiallist = [drlkcluster.Structs.CredentialStruct(credentialIndex=credentialIndex_1,
                                                                   credentialType=drlkcluster.Enums.CredentialTypeEnum.kAliroEvictableEndpointKey),
                              drlkcluster.Structs.CredentialStruct(credentialIndex=credentialIndex_1,
                                                                   credentialType=drlkcluster.Enums.CredentialTypeEnum.kPin)]
            await self.get_user(userIndex_1, user_name, user_unique_id, credentiallist, Status.Success)

        self.step("35")
        if self.pics_guard(self.check_pics("DRLK.S.F08") and self.check_pics("DRLK.S.C1d.Rsp")):
            await self.send_clear_user_cmd(userIndex_1)

        self.step("36")
        if self.pics_guard(self.check_pics("DRLK.S.C26.Rsp")):
            await self.clear_credentials_cmd(credential=NullValue)

        self.step("37")
        if self.pics_guard(self.check_pics("DRLK.S.C29.Rsp")):
            await self.send_clear_aliro_reader_config_cmd()


if __name__ == '__main__':
    default_matter_test_main()
