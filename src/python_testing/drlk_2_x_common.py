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
import random
import string
import time

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import type_matches
from mobly import asserts


class DRLK_COMMON:
    async def read_drlk_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.DoorLock
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    async def write_drlk_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.DoorLock
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(self.endpoint, attribute)])
        err_msg = "Received error status {} when writing {}:{}".format(str(result[0].Status), str(cluster), str(attribute))
        asserts.assert_equal(result[0].Status, Status.Success, err_msg)

    async def write_drlk_attribute_expect_error(self, attribute, error):
        cluster = Clusters.Objects.DoorLock
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(self.endpoint, attribute)])
        err_msg = "Did not see expected error {} when writing {}:{}".format(str(error), str(cluster), str(attribute))
        asserts.assert_equal(result[0].Status, error, err_msg)

    async def send_drlk_cmd_expect_success(self, command) -> None:
        await self.send_single_cmd(cmd=command, endpoint=self.endpoint, timedRequestTimeoutMs=1000)

    async def send_drlk_cmd_expect_error(self, command, error: Status) -> None:
        try:
            await self.send_single_cmd(cmd=command, endpoint=self.endpoint, timedRequestTimeoutMs=1000)
            asserts.assert_true(False, "Unexpected command success, command=%s", command)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, error, "Unexpected error returned")
            pass

    async def send_set_credential_cmd(self, operationType, credential, credentialData, userIndex, userStatus, userType) -> Clusters.Objects.DoorLock.Commands.SetCredentialResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.DoorLock.Commands.SetCredential(operationType=operationType,
                                                                                              credential=credential,
                                                                                              credentialData=credentialData,
                                                                                              userIndex=userIndex,
                                                                                              userStatus=userStatus,
                                                                                              userType=userType),
                                         endpoint=self.endpoint,
                                         timedRequestTimeoutMs=1000)
        asserts.assert_true(type_matches(ret, Clusters.Objects.DoorLock.Commands.SetCredentialResponse),
                            "Unexpected return type for SetCredential")
        asserts.assert_true(ret.status == Status.Success, "Error sending SetCredential command, status={}".format(str(ret.status)))
        return ret

    async def send_clear_credential_cmd(self, credential) -> None:
        await self.send_single_cmd(cmd=Clusters.Objects.DoorLock.Commands.ClearCredential(credential=credential),
                                   endpoint=self.endpoint,
                                   timedRequestTimeoutMs=1000)
        ret = await self.send_single_cmd(cmd=Clusters.Objects.DoorLock.Commands.GetCredentialStatus(credential=self.createdCredential),
                                         endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.DoorLock.Commands.GetCredentialStatusResponse),
                            "Unexpected return type for GetCredentialStatus")
        asserts.assert_false(ret.credentialExists, "Error clearing Credential (credentialExists==True)")

    async def cleanup_users_and_credentials(self):
        self.print_step("Cleanup", "Clear created User and Credential on the DUT")
        if self.createdCredential:
            await self.send_clear_credential_cmd(self.createdCredential)
            logging.info("Credential cleared at CredentialIndex %d" % (self.createdCredential.credentialIndex))
        self.createdCredential = None

    async def generate_pincode(self, maxPincodeLength):
        return ''.join(random.choices(string.digits, k=maxPincodeLength))

    async def teardown(self):
        await self.cleanup_users_and_credentials()

    async def run_drlk_test_common(self, lockUnlockCommand, lockUnlockCmdRspPICS, lockUnlockText, doAutoRelockTest):
        is_ci = self.check_pics('PICS_SDK_CI_ONLY')

        self.createdCredential = None

        self.endpoint = self.user_params.get("endpoint", 1)

        # Allow for user overrides of these values
        credentialIndex = self.user_params.get("credential_index", 1)
        userCodeTemporaryDisableTime = self.user_params.get("user_code_temporary_disable_time", 15)
        wrongCodeEntryLimit = self.user_params.get("wrong_code_entry_limit", 3)
        autoRelockTime = self.user_params.get("auto_relock_time", 60)
        if is_ci:
            autoRelockTime = 10

        cluster = Clusters.Objects.DoorLock
        attributes = Clusters.DoorLock.Attributes

        validPincode = None
        invalidPincode = None

        self.print_step(0, "Commissioning, already done")

        if self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F07"):
            self.print_step("Preconditions.1a",
                            "TH reads MaxPINCodeLength attribute from DUT and generates a valid PINCode")
            maxPincodeLength_dut = await self.read_drlk_attribute_expect_success(attribute=attributes.MaxPINCodeLength)
            logging.info("MaxPINCodeLength value is %s" % (maxPincodeLength_dut))

            validPincodeString = await self.generate_pincode(maxPincodeLength_dut)
            while True:
                invalidPincodeString = await self.generate_pincode(maxPincodeLength_dut)
                if invalidPincodeString != validPincodeString:
                    break
            logging.info("Valid PinCode=%s, Invalid PinCode=%s" % (validPincodeString, invalidPincodeString))

            validPincode = bytes(validPincodeString, 'ascii')
            invalidPincode = bytes(invalidPincodeString, 'ascii')

            self.print_step("Preconditions.1b",
                            "TH sends SetCredential command to DUT to set up User and Credential at CredentialIndex {}".format(str(credentialIndex)))
            credential = cluster.Structs.CredentialStruct(credentialIndex=credentialIndex,
                                                          credentialType=Clusters.DoorLock.Enums.CredentialTypeEnum.kPin)
            ret = await self.send_set_credential_cmd(Clusters.DoorLock.Enums.DataOperationTypeEnum.kAdd,
                                                     credential,
                                                     validPincode,
                                                     NullValue,
                                                     NullValue,
                                                     NullValue)
            logging.info("Credential created at CredentialIndex %d, UserIndex %d." % (credentialIndex, ret.userIndex))
            self.createdCredential = credential

        requirePinForRemoteOperation_dut = False
        if self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F07"):
            self.print_step("1", "TH writes the RequirePINforRemoteOperation attribute value as false on the DUT")
            attribute = attributes.RequirePINforRemoteOperation(False)
            if self.check_pics("DRLK.S.M.RequirePINForRemoteOperationAttributeWritable"):
                print("---------------------- PICS is true")
                await self.write_drlk_attribute_expect_success(attribute=attribute)
            else:
                print("---------------------- PICS is false")
                await self.write_drlk_attribute_expect_error(attribute=attribute, error=Status.UnsupportedWrite)

            if self.check_pics("DRLK.S.A0033"):
                self.print_step("2", "TH reads and saves the value of the RequirePINforRemoteOperation attribute from the DUT")
                requirePinForRemoteOperation_dut = await self.read_drlk_attribute_expect_success(attribute=attributes.RequirePINforRemoteOperation)
                logging.info("Current RequirePINforRemoteOperation value is %s" % (requirePinForRemoteOperation_dut))

                if self.check_pics("DRLK.S.M.RequirePINForRemoteOperationAttributeWritable"):
                    self.print_step("2a", "TH verifies that RequirePINforRemoteOperation is FALSE")
                    asserts.assert_false(requirePinForRemoteOperation_dut, "RequirePINforRemoteOperation is expected to be FALSE")
            else:
                asserts.assert_true(
                    False, "RequirePINforRemoteOperation is a mandatory attribute if DRLK.S.F07(COTA) & DRLK.S.F00(PIN)")

        if self.check_pics(lockUnlockCmdRspPICS):
            self.print_step("3", "TH sends %s Command to the DUT without PINCode" % lockUnlockText)
            command = lockUnlockCommand(PINCode=None)
            if requirePinForRemoteOperation_dut:
                await self.send_drlk_cmd_expect_error(command=command, error=Status.Failure)
            else:
                await self.send_drlk_cmd_expect_success(command=command)

            self.print_step("4", "TH sends %s Command to the DUT with valid PINCode" % lockUnlockText)
            command = lockUnlockCommand(PINCode=validPincode)
            await self.send_drlk_cmd_expect_success(command=command)
        else:
            asserts.assert_true(False, "%sResponse is a mandatory command response and must be supported in PICS" % lockUnlockText)

        if self.check_pics("DRLK.S.F00") and self.check_pics("DRLK.S.F07"):
            self.print_step("5", "TH writes the RequirePINforRemoteOperation attribute value as true on the DUT")
            attribute = attributes.RequirePINforRemoteOperation(True)
            if self.check_pics("DRLK.S.M.RequirePINForRemoteOperationAttributeWritable"):
                await self.write_drlk_attribute_expect_success(attribute=attribute)
            else:
                await self.write_drlk_attribute_expect_error(attribute=attribute, error=Status.UnsupportedWrite)

            if self.check_pics("DRLK.S.A0033"):
                self.print_step("6", "TH reads and saves the value of the RequirePINforRemoteOperation attribute from the DUT")
                requirePinForRemoteOperation_dut = await self.read_drlk_attribute_expect_success(attribute=attributes.RequirePINforRemoteOperation)
                logging.info("Current RequirePINforRemoteOperation value is %s" % (requirePinForRemoteOperation_dut))

                if self.check_pics("DRLK.S.M.RequirePINForRemoteOperationAttributeWritable"):
                    self.print_step("6a", "TH verifies that RequirePINforRemoteOperation is TRUE")
                    asserts.assert_true(requirePinForRemoteOperation_dut, "RequirePINforRemoteOperation is expected to be TRUE")

        if self.check_pics("DRLK.S.F00") and self.check_pics(lockUnlockCmdRspPICS) and self.check_pics("DRLK.S.A0033"):
            self.print_step("7", "TH sends %s Command to the DUT with an invalid PINCode" % lockUnlockText)
            command = lockUnlockCommand(PINCode=invalidPincode)
            await self.send_drlk_cmd_expect_error(command=command, error=Status.Failure)

            self.print_step("8", "TH sends %s Command to the DUT without PINCode" % lockUnlockText)
            command = lockUnlockCommand(PINCode=None)
            if requirePinForRemoteOperation_dut:
                await self.send_drlk_cmd_expect_error(command=command, error=Status.Failure)
            else:
                await self.send_drlk_cmd_expect_success(command=command)

        if self.check_pics(lockUnlockCmdRspPICS) and self.check_pics("DRLK.S.A0033"):
            self.print_step("9", "TH sends %s Command to the DUT with valid PINCode" % lockUnlockText)
            command = lockUnlockCommand(PINCode=validPincode)
            await self.send_drlk_cmd_expect_success(command=command)

        if self.check_pics("DRLK.S.F00") or self.check_pics("DRLK.S.F01"):
            self.print_step("10a", "TH writes the WrongCodeEntryLimit to any value between 1 and 255")
            attribute = attributes.WrongCodeEntryLimit(wrongCodeEntryLimit)
            if self.check_pics("DRLK.S.M.WrongCodeEntryLimitAttributeWritable"):
                await self.write_drlk_attribute_expect_success(attribute=attribute)
            else:
                await self.write_drlk_attribute_expect_error(attribute=attribute, error=Status.UnsupportedWrite)

            self.print_step("10b", "TH reads the value of WrongCodeEntryLimit attribute. Verify a range of 1-255")
            wrongCodeEntryLimit_dut = await self.read_drlk_attribute_expect_success(attribute=attributes.WrongCodeEntryLimit)
            logging.info("WrongCodeEntryLimit value is %s" % (wrongCodeEntryLimit_dut))
            asserts.assert_in(wrongCodeEntryLimit_dut, range(1, 255), "WrongCodeEntryLimit value is out of range")

            self.print_step("11a", "TH writes the UserCodeTemporaryDisableTime to any value between 1 and 255")
            attribute = attributes.UserCodeTemporaryDisableTime(userCodeTemporaryDisableTime)
            if self.check_pics("DRLK.S.M.UserCodedTemporaryDisableTimeAttributeWritable"):
                await self.write_drlk_attribute_expect_success(attribute=attribute)
            else:
                await self.write_drlk_attribute_expect_error(attribute=attribute, error=Status.UnsupportedWrite)

            self.print_step("11b", "TH reads the value of UserCodeTemporaryDisableTime attribute. Verify a range of 1-255")
            userCodeTemporaryDisableTime_dut = await self.read_drlk_attribute_expect_success(attribute=attributes.UserCodeTemporaryDisableTime)
            logging.info("UserCodeTemporaryDisableTime value is %s" % (userCodeTemporaryDisableTime_dut))
            asserts.assert_in(userCodeTemporaryDisableTime_dut, range(1, 255), "UserCodeTemporaryDisableTime value is out of range")

        if self.check_pics(lockUnlockCmdRspPICS) and self.check_pics("DRLK.S.F00"):
            self.print_step("12", "TH sends {} Command to the DUT with an invalid PINCode, repeated {} times".format(
                lockUnlockText, wrongCodeEntryLimit_dut))
            for i in range(wrongCodeEntryLimit_dut):
                command = lockUnlockCommand(PINCode=invalidPincode)
                await self.send_drlk_cmd_expect_error(command=command, error=Status.Failure)

            self.print_step("13", "TH sends %s Command to the DUT with valid PINCode. Verify failure or no response" % lockUnlockText)
            command = lockUnlockCommand(PINCode=validPincode)
            await self.send_drlk_cmd_expect_error(command=command, error=Status.Failure)

            if self.check_pics("DRLK.S.A0031"):
                self.print_step("14", "Wait for UserCodeTemporaryDisableTime seconds")
                time.sleep(userCodeTemporaryDisableTime_dut)

            if not doAutoRelockTest:
                self.print_step("15", "Send %s with valid Pincode and verify success" % lockUnlockText)
                command = lockUnlockCommand(PINCode=validPincode)
                await self.send_drlk_cmd_expect_success(command=command)

        if doAutoRelockTest:
            if self.check_pics("DRLK.S.A0023"):
                self.print_step("15", "TH writes the AutoRelockTime attribute value on the DUT")
                attribute = attributes.AutoRelockTime(autoRelockTime)
                if self.check_pics("DRLK.S.M.AutoRelockTimeAttributeWritable"):
                    await self.write_drlk_attribute_expect_success(attribute=attribute)
                else:
                    await self.write_drlk_attribute_expect_error(attribute=attribute, error=Status.UnsupportedWrite)

                self.print_step("16", "TH reads the value of AutoRelockTime attribute.")
                autoRelockTime_dut = await self.read_drlk_attribute_expect_success(attribute=attributes.AutoRelockTime)
                logging.info("AutoRelockTime value is %s" % (autoRelockTime_dut))

            if self.check_pics(lockUnlockCmdRspPICS):
                self.print_step("17", "Send %s with valid Pincode and verify success" % lockUnlockText)
                command = lockUnlockCommand(PINCode=validPincode)
                await self.send_drlk_cmd_expect_success(command=command)

            if self.check_pics("DRLK.S.A0023"):
                self.print_step("18a", "Wait for AutoRelockTime seconds")
                # Add additional wait time buffer for motor movement, etc.
                time.sleep(autoRelockTime_dut + 5)

                if self.check_pics("DRLK.S.A0000"):
                    self.print_step("18b", "TH reads LockState attribute after AutoRelockTime Expires")
                    lockstate_dut = await self.read_drlk_attribute_expect_success(attribute=attributes.LockState)
                    logging.info("Current LockState is %s" % (lockstate_dut))
                    asserts.assert_equal(lockstate_dut, Clusters.DoorLock.Enums.DlLockState.kLocked,
                                         "LockState expected to be value==Locked")

        await self.cleanup_users_and_credentials()

    async def run_drlk_test_2_2(self):
        await self.run_drlk_test_common(lockUnlockCommand=Clusters.Objects.DoorLock.Commands.LockDoor,
                                        lockUnlockCmdRspPICS="DRLK.S.C00.Rsp",
                                        lockUnlockText="LockDoor",
                                        doAutoRelockTest=False)

    async def run_drlk_test_2_3(self):
        await self.run_drlk_test_common(lockUnlockCommand=Clusters.Objects.DoorLock.Commands.UnlockDoor,
                                        lockUnlockCmdRspPICS="DRLK.S.C01.Rsp",
                                        lockUnlockText="UnlockDoor",
                                        doAutoRelockTest=True)

    async def run_drlk_test_2_12(self):
        await self.run_drlk_test_common(lockUnlockCommand=Clusters.Objects.DoorLock.Commands.UnboltDoor,
                                        lockUnlockCmdRspPICS="DRLK.S.C27.Rsp",
                                        lockUnlockText="UnboltDoor",
                                        doAutoRelockTest=True)
