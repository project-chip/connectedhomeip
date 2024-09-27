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
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto --PICS src/app/tests/suites/certification/ci-pics-values
# === END CI TEST ARGUMENTS ===

import logging
import random
from pathlib import Path

import chip.clusters as Clusters
from chip.tlv import TLVReader
from chip import ChipDeviceCtrl
from chip.ChipDeviceCtrl import CommissioningParameters
from chip.exceptions import ChipStackError
from chip.interaction_model import Status
from chip.native import PyChipError
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, MatterStackState
from mdns_discovery import mdns_discovery
from mobly import asserts
from time import sleep

opcreds = Clusters.OperationalCredentials
nonce = random.randbytes(32)


class TC_CADMIN_1_3(MatterBaseTest):
    async def CommissionAttempt(
            self, setupPinCode: int, thnum: int, th: str, fail: bool):

        logging.info(f"-----------------Commissioning with TH_CR{str(thnum)}-------------------------")
        if fail:
            ctx = asserts.assert_raises(ChipStackError)
            self.print_step(0, ctx)
            with ctx:
                await th.CommissionOnNetwork(
                    nodeId=self.dut_node_id, setupPinCode=setupPinCode,
                    filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)
                errcode = PyChipError.from_code(ctx.exception.err)
            logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(
                errcode.is_success, errcode))
            asserts.assert_false(errcode.is_success, 'Commissioning complete did not error as expected')
            asserts.assert_true(errcode.sdk_code == 0x0000000B,
                                'Unexpected error code returned from CommissioningComplete')

        elif not fail:
            await th.CommissionOnNetwork(
                nodeId=self.dut_node_id, setupPinCode=setupPinCode,
                filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)

    async def get_fabrics(self, th: ChipDeviceCtrl) -> int:
        OC_cluster = Clusters.OperationalCredentials
        fabric_info = await self.read_single_attribute_check_success(dev_ctrl=th, fabric_filtered=True, cluster=OC_cluster, attribute=OC_cluster.Attributes.Fabrics)
        return fabric_info

    async def get_rcac_decoded(self, th: str) -> int:
        csrResponse = await self.send_single_cmd(dev_ctrl=th, node_id=self.dut_node_id, cmd=opcreds.Commands.CSRRequest(CSRNonce=nonce, isForUpdateNOC=False))
        TH_certs_real = await th.IssueNOCChain(csrResponse, self.dut_node_id)
        th_rcac_decoded = TLVReader(TH_certs_real.rcacBytes).get()["Any"]
        return th_rcac_decoded

    async def get_txt_record(self):
        discovery = mdns_discovery.MdnsDiscovery(verbose_logging=True)
        comm_service = await discovery.get_commissionable_service(
            discovery_timeout_sec=240,
            log_output=False,
        )
        return comm_service

    async def get_window_status(self) -> int:
        AC_cluster = Clusters.AdministratorCommissioning
        window_status = await self.read_single_attribute_check_success(dev_ctrl=self.th2, fabric_filtered=True, cluster=AC_cluster, attribute=AC_cluster.Attributes.WindowStatus)
        return window_status

    async def OpenCommissioningWindow(self, th: ChipDeviceCtrl, duration: int):
        self.discriminator = random.randint(0, 4095)
        try:
            params = await th.OpenCommissioningWindow(
                nodeid=self.dut_node_id, timeout=duration, iteration=10000, discriminator=self.discriminator, option=1)
            return params

        except Exception as e:
            logging.exception('Error running OpenCommissioningWindow %s', e)
            if str(Clusters.AdministratorCommissioning.Enums.StatusCode.kBusy) in e.msg:
                asserts.assert_true(False, 'Failed to open commissioning window')
            else:
                asserts.assert_true(False, 'Failed to verify')

    async def write_nl_attr(self, th: str, attr_val: object):
        result = await th.WriteAttribute(self.dut_node_id, [(0, attr_val)])
        asserts.assert_equal(result[0].Status, Status.Success, f"{th} node label write failed")

    async def read_nl_attr(self, th: str, attr_val: object):
        try:
            result = await th.ReadAttribute(nodeid=self.dut_node_id, attributes=[(0, attr_val)])
        except Exception as e:
            asserts.assert_equal(e.err, "Received error message from read attribute attempt")

    def pics_TC_CADMIN_1_3(self) -> list[str]:
        return ["CADMIN.S"]

    def steps_TC_CADMIN_1_3(self) -> list[TestStep]:
        return [
            TestStep(1, "TH_CR1 starts a commissioning process with DUT_CE", is_commissioning=True),
            TestStep(2, "TH_CR1 reads the BasicCommissioningInfo attribute from the General Commissioning cluster and saves the MaxCumulativeFailsafeSeconds field as max_window_duration."),
            TestStep("3a", "TH_CR1 opens a commissioning window on DUT_CE using a commissioning timeout of max_window_duration using ECM",
                     "DUT_CE opens its Commissioning window to allow a second commissioning."),
            TestStep("3b", "DNS-SD records shows DUT_CE advertising", "Verify that the DNS-SD advertisement shows CM=2"),
            TestStep("3c", "TH_CR1 writes and reads the Basic Information Cluster’s NodeLabel mandatory attribute of DUT_CE",
                     "Verify DUT_CE responds to both write/read with a success"),
            TestStep(4, "TH creates a controller (TH_CR2) on a new fabric and commissions DUT_CE using that controller. TH_CR2 should commission the device using a different NodeID than TH_CR1.",
                     "Commissioning is successful"),
            TestStep(5, "TH_CR1 reads the Fabrics attribute from the Node Operational Credentials cluster using a fabric-filtered read",
                     "Verify that the RootPublicKey matches the root public key for TH_CR1 and the NodeID matches the node ID used when TH_CR1 commissioned the device."),
            TestStep(6, "TH_CR2 reads the Fabrics attribute from the Node Operational Credentials cluster using a fabric-filtered read",
                     "Verify that the RootPublicKey matches the root public key for TH_CR2 and the NodeID matches the node ID used when TH_CR2 commissioned the device."),
            TestStep(7, "TH_CR1 writes and reads the Basic Information Cluster’s NodeLabel mandatory attribute of DUT_CE",
                     "Verify DUT_CE responds to both write/read with a success"),
            TestStep(8, "TH_CR2 reads, writes and then reads the Basic Information Cluster’s NodeLabel mandatory attribute of DUT_CE",
                     "Verify the initial read reflect the value written in the above step. Verify DUT_CE responds to both write/read with a success"),
            TestStep(9, "TH_CR2 opens a commissioning window on DUT_CE for 180 seconds using ECM"),
            TestStep(10, "Wait for the commissioning window in step 9 to timeout"),
            TestStep(11, "TH_CR2 reads the window status to verify the DUT_CE window is closed",
                     "DUT_CE windows status shows the window is closed"),
            TestStep(12, "TH_CR2 opens a commissioning window on DUT_CE using ECM",
                     "DUT_CE opens its Commissioning window to allow a new commissioning"),
            TestStep(13, "TH_CR1 starts a commissioning process with DUT_CE before the timeout from step 12",
                     "Since DUT_CE was already commissioned by TH_CR1 in step 1, AddNOC fails with NOCResponse with StatusCode field set to FabricConflict (9)"),
        ]

    @async_test_body
    async def test_TC_CADMIN_1_3(self):
        self.step(1)

        # Establishing TH1
        self.th1 = self.default_controller

        self.step(2)
        GC_cluster = Clusters.GeneralCommissioning
        attribute = GC_cluster.Attributes.BasicCommissioningInfo
        duration = await self.read_single_attribute_check_success(endpoint=0, cluster=GC_cluster, attribute=attribute)
        self.max_window_duration = duration.maxCumulativeFailsafeSeconds

        self.step("3a")
        params = await self.OpenCommissioningWindow(th=self.th1, duration=self.max_window_duration)
        setupPinCode = params.setupPinCode

        self.step("3b")
        services = await self.get_txt_record()
        if services.txt_record['CM'] != "2":
            asserts.fail(f"Expected cm record value not found, instead value found was {str(services.txt_record['CM'])}")

        self.step("3c")
        BI_cluster = Clusters.BasicInformation
        nl_attribute = BI_cluster.Attributes.NodeLabel
        await self.write_nl_attr(th=self.th1, attr_val=nl_attribute)
        await self.read_nl_attr(th=self.th1, attr_val=nl_attribute)

        self.step(4)
        # Establishing TH2
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)
        await self.CommissionAttempt(setupPinCode, th=self.th2, fail=False, thnum=2)

        self.step(5)
        # TH_CR1 reads the Fabrics attribute from the Node Operational Credentials cluster using a fabric-filtered read
        th1_fabric_info = await self.get_fabrics(th=self.th1)

        # Verify that the RootPublicKey matches the root public key for TH_CR1 and the NodeID matches the node ID used when TH_CR1 commissioned the device.
        await self.send_single_cmd(dev_ctrl=self.th1, node_id=self.dut_node_id, cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(10))
        th1_rcac_decoded = await self.get_rcac_decoded(th=self.th1)
        if th1_fabric_info[0].rootPublicKey != th1_rcac_decoded[9]:
            asserts.fail(f"public keys from fabric and certs for TH1 are not the same")
        if th1_fabric_info[0].nodeID != self.dut_node_id:
            asserts.fail(f"DUT node ID from fabric does not equal DUT node ID for TH1 during commissioning")

        # Expiring the failsafe timer in an attempt to clean up before TH2 attempt.
        await self.th1.SendCommand(self.dut_node_id, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0))

        self.step(6)
        # TH_CR2 reads the Fabrics attribute from the Node Operational Credentials cluster using a fabric-filtered read
        th2_fabric_info = await self.get_fabrics(th=self.th2)

        # Verify that the RootPublicKey matches the root public key for TH_CR2 and the NodeID matches the node ID used when TH_CR2 commissioned the device.
        await self.send_single_cmd(dev_ctrl=self.th2, node_id=self.dut_node_id, cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(self.max_window_duration))
        th2_rcac_decoded = await self.get_rcac_decoded(th=self.th2)
        if th2_fabric_info[0].rootPublicKey != th2_rcac_decoded[9]:
            asserts.fail(f"public keys from fabric and certs for TH2 are not the same")
        if th2_fabric_info[0].nodeID != self.dut_node_id:
            asserts.fail(f"DUT node ID from fabric does not match DUT node ID for TH2 during commissioning")

        await self.th2.SendCommand(self.dut_node_id, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0))

        self.step(7)
        # TH_CR1 writes and reads the Basic Information Cluster’s NodeLabel mandatory attribute of DUT_CE
        await self.write_nl_attr(th=self.th1, attr_val=nl_attribute)
        await self.read_nl_attr(th=self.th1, attr_val=nl_attribute)

        self.step(8)
        # TH_CR2 writes and reads the Basic Information Cluster’s NodeLabel mandatory attribute of DUT_CE
        await self.write_nl_attr(th=self.th2, attr_val=nl_attribute)
        await self.read_nl_attr(th=self.th2, attr_val=nl_attribute)
        sleep(1)

        self.step(9)
        # TH_CR2 opens a commissioning window on DUT_CE for 180 seconds using ECM
        await self.OpenCommissioningWindow(th=self.th2, duration=180)

        self.step(10)
        # Wait for the commissioning window in step 9 to timeout
        sleep(180)

        self.step(11)
        # TH_CR2 reads the window status to verify the DUT_CE window is closed
        await self.th2.SendCommand(self.dut_node_id, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0))
        window_status = await self.get_window_status()
        # window_status = await self.th2.ReadAttribute(nodeid=self.dut_node_id, attributes=[(0, Clusters.AdministratorCommissioning.Attributes.WindowStatus)])
        self.print_step(0, dir(AC_cluster.Enums))
        self.print_step(1, window_status)

        self.step(12)
        # TH_CR2 opens a commissioning window on DUT_CE using ECM

        self.step(13)
        # TH_CR1 starts a commissioning process with DUT_CE before the timeout from step 12


if __name__ == "__main__":
    default_matter_test_main()
