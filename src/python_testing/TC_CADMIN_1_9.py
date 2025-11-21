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
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
from copy import deepcopy

from mobly import asserts
from support_modules.cadmin_support import CADMINBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.exceptions import ChipStackError
from matter.native import PyChipError
from matter.testing.matter_testing import CustomCommissioningParameters, TestStep, async_test_body, default_matter_test_main


class TC_CADMIN_1_9(CADMINBaseTest):
    async def OpenCommissioningWindowForMaxTime(self) -> CustomCommissioningParameters:
        cluster = Clusters.GeneralCommissioning
        attribute = cluster.Attributes.BasicCommissioningInfo
        duration = await self.read_single_attribute_check_success(endpoint=0, cluster=cluster, attribute=attribute)
        return await self.open_commissioning_window(dev_ctrl=self.th1, node_id=self.dut_node_id, timeout=duration.maxCumulativeFailsafeSeconds)

    async def CommissionOnNetwork(self, params: CustomCommissioningParameters):
        ctx = asserts.assert_raises(ChipStackError)
        with ctx:
            await self.th2.CommissionOnNetwork(
                nodeId=self.dut_node_id,
                setupPinCode=params.commissioningParameters.setupPinCode,
                filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
                filter=params.randomDiscriminator
            )
        errcode = PyChipError.from_code(ctx.exception.err)
        return errcode

    async def CommissionAttempt(self, params: CustomCommissioningParameters, expectedErrCode: int):
        if expectedErrCode == 3:
            for cycle in range(20):
                logging.info("-----------------Current Iteration {}-------------------------".format(cycle+1))
                new_params = deepcopy(params)
                new_params.commissioningParameters.setupPinCode = self.generate_unique_random_value(
                    params.commissioningParameters.setupPinCode)
                errcode = await self.CommissionOnNetwork(new_params)
                logging.info('Commissioning complete done. Successful? {}, errorcode = {}, cycle={}'.format(
                    errcode.is_success, errcode, (cycle+1)))
                asserts.assert_false(errcode.is_success, 'Commissioning complete did not error as expected')
                asserts.assert_true(errcode.sdk_code == expectedErrCode,
                                    'Unexpected error code returned from CommissioningComplete')

        elif expectedErrCode == 50:
            logging.info("-----------------Attempting connection expecting timeout-------------------------")
            errcode = await self.CommissionOnNetwork(params)
            logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(errcode.is_success, errcode))
            asserts.assert_false(errcode.is_success, 'Commissioning complete did not error as expected')
            # TODO: Adding try or except clause here as the errcode code be either 50 for timeout or 3 for incorrect state at this time
            # until issue mentioned in https://github.com/project-chip/connectedhomeip/issues/34383 can be resolved
            asserts.assert_in(errcode.sdk_code, [expectedErrCode, 3], 'Unexpected error code returned from CommissioningComplete')

    def steps_TC_CADMIN_1_9(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(
                2, "TH1 opens commissioning window on DUT with duration set to value for maxCumulativeFailsafeSeconds"),
            TestStep(3, "TH2 attempts to connect 20 times to endpoint with incorrect passcode"),
            TestStep(4, "TH2 attempts to connect to endpoint with correct passcode"),
            TestStep(5, "TH1 opening Commissioning Window one more time to validate ability to do so"),
            TestStep(6, "TH1 revoking Commissioning Window"),
        ]

    def pics_TC_CADMIN_1_9(self) -> list[str]:
        return ["CADMIN.S"]

    @async_test_body
    async def test_TC_CADMIN_1_9(self):
        self.step(1)

        # Establishing TH1 and TH2
        self.th1 = self.default_controller
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        self.step(2)
        params = await self.OpenCommissioningWindowForMaxTime()

        self.step(3)
        await self.CommissionAttempt(params, expectedErrCode=0x03)

        self.step(4)
        await self.CommissionAttempt(params, expectedErrCode=0x32)

        self.step(5)
        params = await self.OpenCommissioningWindowForMaxTime()

        self.step(6)
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.th1.SendCommand(nodeId=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)
        # The failsafe cleanup is scheduled after the command completes, so give it a bit of time to do that
        await asyncio.sleep(1)


if __name__ == "__main__":
    default_matter_test_main()
