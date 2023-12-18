#
#    Copyright (c) 2022 Project CHIP Authors
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
import time

import chip.CertificateAuthority
import chip.clusters as Clusters
import chip.clusters.enum
import chip.FabricAdmin
from chip import ChipDeviceCtrl
from chip.ChipDeviceCtrl import CommissioningParameters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

# Commissioning stage numbers - we should find a better way to match these to the C++ code
kArmFailsafe = 4
kConfigRegulatory = 5
kSendPAICertificateRequest = 10
kSendDACCertificateRequest = 11
kSendAttestationRequest = 12
kSendOpCertSigningRequest = 14
kSendTrustedRootCert = 17
kSendNOC = 18


class TC_CGEN_2_4(MatterBaseTest):

    def OpenCommissioningWindow(self) -> CommissioningParameters:
        try:
            params = self.th1.OpenCommissioningWindow(
                nodeid=self.dut_node_id, timeout=600, iteration=10000, discriminator=self.discriminator, option=1)
            return params

        except Exception as e:
            logging.exception('Error running OpenCommissioningWindow %s', e)
            asserts.assert_true(False, 'Failed to open commissioning window')

    async def CommissionToStageSendCompleteAndCleanup(
            self, stage: int, expectedErrorPart: chip.native.ErrorSDKPart, expectedErrCode: int):

        logging.info("-----------------Fail on step {}-------------------------".format(stage))
        params = self.OpenCommissioningWindow()
        self.th2.ResetTestCommissioner()
        # This will run the commissioning up to the point where stage x is run and the
        # response is sent before the test commissioner simulates a failure
        self.th2.SetTestCommissionerPrematureCompleteAfter(stage)
        errcode = self.th2.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)
        logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(errcode.is_success, errcode))
        asserts.assert_false(errcode.is_success, 'Commissioning complete did not error as expected')
        asserts.assert_true(errcode.sdk_part == expectedErrorPart, 'Unexpected error type returned from CommissioningComplete')
        asserts.assert_true(errcode.sdk_code == expectedErrCode, 'Unexpected error code returned from CommissioningComplete')
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)
        # The failsafe cleanup is scheduled after the command completes, so give it a bit of time to do that
        time.sleep(1)

    @async_test_body
    async def test_TC_CGEN_2_4(self):
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)
        # kArmFailsafe, expect General error 0x7e (UNSUPPORTED_ACCESS)
        await self.CommissionToStageSendCompleteAndCleanup(kArmFailsafe, chip.native.ErrorSDKPart.IM_GLOBAL_STATUS, 0x7e)
        # kConfigRegulatory, expect General error 0x7e (UNSUPPORTED_ACCESS)
        await self.CommissionToStageSendCompleteAndCleanup(kConfigRegulatory, chip.native.ErrorSDKPart.IM_GLOBAL_STATUS, 0x7e)
        # kSendPAICertificateRequest, expect General error 0x7e (UNSUPPORTED_ACCESS)
        await self.CommissionToStageSendCompleteAndCleanup(kSendPAICertificateRequest, chip.native.ErrorSDKPart.IM_GLOBAL_STATUS, 0x7e)
        # kSendDACCertificateRequest, expect General error 0x7e (UNSUPPORTED_ACCESS)
        await self.CommissionToStageSendCompleteAndCleanup(kSendDACCertificateRequest, chip.native.ErrorSDKPart.IM_GLOBAL_STATUS, 0x7e)
        # kSendAttestationRequest, expect General error 0x7e (UNSUPPORTED_ACCESS)
        await self.CommissionToStageSendCompleteAndCleanup(kSendAttestationRequest, chip.native.ErrorSDKPart.IM_GLOBAL_STATUS, 0x7e)
        # kSendOpCertSigningRequest, expect General error 0x7e (UNSUPPORTED_ACCESS)
        await self.CommissionToStageSendCompleteAndCleanup(kSendOpCertSigningRequest, chip.native.ErrorSDKPart.IM_GLOBAL_STATUS, 0x7e)
        # kSendTrustedRootCert, expect General error 0x7e (UNSUPPORTED_ACCESS)
        await self.CommissionToStageSendCompleteAndCleanup(kSendTrustedRootCert, chip.native.ErrorSDKPart.IM_GLOBAL_STATUS, 0x7e)
        # kSendNOC, expect cluster error InvalidAuthentication
        await self.CommissionToStageSendCompleteAndCleanup(kSendNOC, chip.native.ErrorSDKPart.IM_CLUSTER_STATUS, 0x02)

        logging.info('Step 15 - TH1 opens a commissioning window')
        params = self.OpenCommissioningWindow()

        logging.info('Step 16 - TH2 fully commissions the DUT')
        self.th2.ResetTestCommissioner()
        errcode = self.th2.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)
        logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(errcode.is_success, errcode))

        logging.info('Step 17 - TH1 sends an arm failsafe')
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
        # This will throw if not successful
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd)

        logging.info('Step 18 - TH1 reads the location capability')
        attr = Clusters.GeneralCommissioning.Attributes.LocationCapability
        cap = await self.read_single_attribute(dev_ctrl=self.th1, node_id=self.dut_node_id, endpoint=0, attribute=attr)
        if cap == Clusters.GeneralCommissioning.Enums.RegulatoryLocationTypeEnum.kIndoor:
            newloc = Clusters.GeneralCommissioning.Enums.RegulatoryLocationTypeEnum.kOutdoor
        elif cap == Clusters.GeneralCommissioning.Enums.RegulatoryLocationTypeEnum.kOutdoor:
            newloc = Clusters.GeneralCommissioning.Enums.RegulatoryLocationTypeEnum.kIndoor
        else:
            newloc = Clusters.GeneralCommissioning.Enums.RegulatoryLocationTypeEnum.extend_enum_if_value_doesnt_exist(3)

        logging.info('Step 19 Send SetRgulatoryConfig with incorrect location newloc = {}'.format(newloc))
        cmd = Clusters.GeneralCommissioning.Commands.SetRegulatoryConfig(
            newRegulatoryConfig=newloc, countryCode="XX", breadcrumb=0)
        ret = await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd)
        asserts.assert_true(ret.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kValueOutsideRange)

        logging.info('Step 20 - TH2 sends CommissioningComplete')
        cmd = Clusters.GeneralCommissioning.Commands.CommissioningComplete()
        resp = await self.th2.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd)
        asserts.assert_true(isinstance(resp, Clusters.GeneralCommissioning.Commands.CommissioningCompleteResponse),
                            'Incorrect response type from command')
        asserts.assert_true(
            resp.errorCode == Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kInvalidAuthentication, 'Incorrect error code')

        logging.info('Step 21 - TH1 sends an arm failsafe with timeout==0')
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        # This will throw if not successful
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd)

        logging.info('Step 22 - TH1 sends CommissioningComplete')
        cmd = Clusters.GeneralCommissioning.Commands.CommissioningComplete()
        resp = await self.th2.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd)
        asserts.assert_true(isinstance(resp, Clusters.GeneralCommissioning.Commands.CommissioningCompleteResponse),
                            'Incorrect response type from command')
        asserts.assert_true(resp.errorCode == Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kNoFailSafe,
                            'Incorrect error code')

        logging.info('Step 23 - TH2 reads fabric index')
        attr = Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        th2FabricIndex = await self.read_single_attribute(dev_ctrl=self.th2, node_id=self.dut_node_id, endpoint=0, attribute=attr)

        logging.info('Step 24 - TH1 removes TH2')
        cmd = Clusters.OperationalCredentials.Commands.RemoveFabric(fabricIndex=th2FabricIndex)
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd)


if __name__ == "__main__":
    default_matter_test_main()
