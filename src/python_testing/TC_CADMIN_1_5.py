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
#     quiet: false
# === END CI TEST ARGUMENTS ===

import asyncio
import asyncio.exceptions as ae
import logging
import random
from time import sleep

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.ChipDeviceCtrl import CommissioningParameters
from chip.exceptions import ChipStackError
import chip.interaction_model
from chip.native import PyChipError
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mdns_discovery import mdns_discovery
from mobly import asserts


class TC_CADMIN_1_5(MatterBaseTest):
    async def get_txt_record(self):
        discovery = mdns_discovery.MdnsDiscovery(verbose_logging=True)
        comm_service = await discovery.get_commissionable_service(
            discovery_timeout_sec=240,
            log_output=False,
        )
        return comm_service

    async def OpenCommissioningWindow(self, timeout: int, iteration: int = 10000, discriminator: int = 1234) -> CommissioningParameters:
        try:
            params = await self.th1.OpenCommissioningWindow(
                nodeid=self.dut_node_id, timeout=timeout, iteration=iteration, discriminator=self.discriminator, option=1)
            return params

        except Exception as e:
            logging.exception('Error running OpenCommissioningWindow %s', e)
            asserts.assert_true(False, 'Failed to open commissioning window')

    async def CommissionOnNetwork(self, setup_code: int):
        ctx = asserts.assert_raises(ChipStackError)
        try:
            with ctx:
                await self.th2.CommissionOnNetwork(
                    nodeId=self.dut_node_id, setupPinCode=setup_code,
                    filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)
            errcode = PyChipError.from_code(ctx.exception.err)

        except ae.CancelledError as e:
            errcode = e.__cause__
        return errcode

    def steps_TC_CADMIN_1_5(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH_CR1 opens a commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM"),
            TestStep(3, "TH_CR1 finds DUT_CE advertising as a commissionable node on DNS-SD",
                     "Verify that the DNS-SD advertisement TXT record shows CM=2"),
            TestStep(4, "TH_CR2 attempts to start a commissioning process with DUT_CE after 190 seconds",
                     "TH_CR2 should fail to commission the DUT since the window should be closed. This may be a failure to find the commissionable node or a failure to establish a PASE connection."),
            TestStep(
                5, "TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM", "{resDutSuccess}"),
            TestStep(6, "TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command", "{resDutSuccess}"),
            TestStep(7, "TH_CR2 attempts to start a commissioning process with DUT_CE",
                     "TH_CR2 should fail to commission the DUT since the window should be closed. This may be a failure to find the commissionable node or a failure to establish a PASE connection."),
            TestStep(8, "TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command.",
                     "Verify this command fails with the cluster specific status code of WindowNotOpen"),
            TestStep(9, "TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM with a discriminator of 4096",
                     "Verify DUT_CE fails to open Commissioning window with status code 3 (PakeParameterError)"),
            TestStep(10, "TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM with the iterations set to 999",
                     "Verify DUT_CE fails to open Commissioning window with status code 3 (PakeParameterError)"),
            TestStep(11, "TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM with the iterations set to 100001",
                     "Verify DUT_CE fails to open Commissioning window with status code 3 (PakeParameterError)"),
            TestStep(12, "TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM with the salt set to 'too_short'",
                     "Verify DUT_CE fails to open Commissioning window with status code 3 (PakeParameterError)"),
            TestStep(13, "TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM with the salt set to 'this pake salt very very very long'",
                     "Verify DUT_CE fails to open Commissioning window with status code 3 (PakeParameterError)"),
            TestStep(14, "TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of {PIXIT_CWDURATION} seconds using ECM",
                     "Verify DUT_CE opens its Commissioning window to allow a second commissioning"),
            TestStep(15, "TH_CR1 opens another commissioning window on DUT_CE using a commissioning timeout of {PIXIT_CWDURATION} seconds using ECM",
                     "Verify DUT_CE fails to open Commissioning window with status code 2 (Busy)"),
            TestStep(16, "TH_CR2 starts a commissioning process with DUT_CE",
                     "Commissioning is successful"),
            TestStep(17, "TH_CR1 tries to revoke the commissioning window on DUT_CE using RevokeCommissioning command",
                     "Verify DUT_CE fails to revoke giving status code 4 (WindowNotOpen) as there was no window open"),
        ]

    def pics_TC_CADMIN_1_5(self) -> list[str]:
        return ["CADMIN.S"]

    @async_test_body
    async def test_TC_CADMIN_1_5(self):
        self.step(1)

        # Establishing TH1 and TH2
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        self.step(2)
        # TH_CR1 opens a commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM
        params = await self.OpenCommissioningWindow(timeout=180)

        self.step(3)
        # TH_CR1 finds DUT_CE advertising as a commissionable node on DNS-SD
        services = await self.get_txt_record()
        if services.txt_record['CM'] != "2":
            asserts.fail(f"Expected cm record value not found, instead value found was {str(services.txt_record['CM'])}")

        self.step(4)
        # TH_CR2 attempts to start a commissioning process with DUT_CE after 190 seconds
        sleep(190)
        # await self.CommissionAttempt(setupPinCode, expectedErrCode=0x03)
        await self.CommissionOnNetwork(params.setupPinCode)

        self.step(5)
        # TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM
        params2 = await self.OpenCommissioningWindow(timeout=180)

        self.step(6)
        # TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command
        try:
            revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
            await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)
        except chip.interaction_model.InteractionModelError as e:
            self.print_step("interaction model error", dir(e))

        self.step(7)
        # TH_CR2 attempts to start a commissioning process with DUT_CE
        await self.CommissionOnNetwork(params2.setupPinCode)

        self.step(8)
        # TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command.
        try:
            revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
            await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)
        except chip.interaction_model.InteractionModelError as e:
            asserts.assert_true(e.clusterStatus, 4, "Cluster status must be 4 to pass this step")

        self.step(9)
        # TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM with a discriminator of 4096
        await self.OpenCommissioningWindow(timeout=180, discriminator=4096)

        self.step(10)
        # TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM with the iterations set to 999
        await self.OpenCommissioningWindow(timeout=180, iteration=999)

        self.step(11)
        # TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM with the iterations set to 100001
        await self.OpenCommissioningWindow(timeout=180, iteration=100001)

        self.step(12)
        # TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM with the salt set to 'too_short'
        await self.OpenCommissioningWindow(timeout=180, salt="too_short")

        self.step(13)
        # TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM with the salt set to 'this pake salt very very very long'
        await self.OpenCommissioningWindow(timeout=180, salt="'this pake salt very very very long'")

        self.step(14)
        # TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of {PIXIT_CWDURATION} seconds using ECM
        cluster = Clusters.GeneralCommissioning
        attribute = cluster.Attributes.BasicCommissioningInfo
        duration = await self.read_single_attribute_check_success(endpoint=0, cluster=cluster, attribute=attribute)
        params2 = await self.OpenCommissioningWindow(timeout=duration.maxCumulativeFailsafeSeconds)

        self.step(15)
        # TH_CR1 opens another commissioning window on DUT_CE using a commissioning timeout of {PIXIT_CWDURATION} seconds using ECM
        params3 = await self.OpenCommissioningWindow(timeout=duration.maxCumulativeFailsafeSeconds)

        self.step(16)
        # TH_CR2 starts a commissioning process with DUT_CE
        await self.CommissionOnNetwork(params3.setupPinCode)

        self.step(17)
        # TH_CR1 tries to revoke the commissioning window on DUT_CE using RevokeCommissioning command
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)


if __name__ == "__main__":
    default_matter_test_main()
