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

import logging
import random
from typing import Optional

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.ChipDeviceCtrl import CommissioningParameters
from chip.exceptions import ChipStackError
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mdns_discovery.mdns_discovery import DNSRecordType, MdnsDiscovery, MdnsServiceType
from mobly import asserts


class TC_CADMIN_1_15(MatterBaseTest):
    async def OpenCommissioningWindow(self, th: ChipDeviceCtrl, expectedErrCode: Optional[Clusters.AdministratorCommissioning.Enums.StatusCode] = None) -> CommissioningParameters:
        if expectedErrCode == 0x00:
            params = await th.OpenCommissioningWindow(
                nodeid=self.dut_node_id, timeout=self.max_window_duration, iteration=10000, discriminator=self.discriminator, option=1)
            return params

        else:
            ctx = asserts.assert_raises(ChipStackError)
            with ctx:
                await th.OpenCommissioningWindow(
                    nodeid=self.dut_node_id, timeout=self.max_window_duration, iteration=10000, discriminator=self.discriminator, option=1)
            errcode = ctx.exception.chip_error
            logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(errcode.is_success, errcode))
            asserts.assert_false(errcode.is_success, 'Commissioning complete did not error as expected')
            asserts.assert_true(errcode.sdk_code == expectedErrCode,
                                'Unexpected error code returned from CommissioningComplete')

    async def read_currentfabricindex(self, th: ChipDeviceCtrl) -> int:
        cluster = Clusters.Objects.OperationalCredentials
        attribute = Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        current_fabric_index = await self.read_single_attribute_check_success(dev_ctrl=th, endpoint=0, cluster=cluster, attribute=attribute)
        return current_fabric_index

    async def get_fabrics(self, th: ChipDeviceCtrl) -> int:
        OC_cluster = Clusters.OperationalCredentials
        fabrics = await self.read_single_attribute_check_success(dev_ctrl=th, fabric_filtered=False, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.Fabrics)
        return fabrics

    async def CommissionAttempt(
            self, setupPinCode: int, thnum: int, th):

        logging.info(f"-----------------Commissioning with TH_CR{str(thnum)}-------------------------")
        await th.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)

    def steps_TC_CADMIN_1_15(self) -> list[TestStep]:
        return [
            TestStep(
                1, "TH_CR1 gets the MaxCumulativeFailsafeSeconds value from BasicCommissioningInfo attribute in GeneralCommissioning Cluster",
                "Should set the MaxCumulativeFailsafeSeconds value from BasicCommissioningInfo attribute to timeout"),
            TestStep(
                2, "TH_CR1 reads the Fabrics attribute from the Node Operational Credentials cluster using a non-fabric-filtered read. Save the number of fabrics in the list as initial_number_of_fabrics"),
            TestStep(
                3, "TH_CR1 opens commissioning window on DUT with duration set to value for MaxCumulativeFailsafeSeconds",
                "Commissioning window should open with timeout set to MaxCumulativeFailsafeSeconds"),
            TestStep(4, "TH_CR2 fully commissions DUT_CE", "DUT should fully commission"),
            TestStep(
                5, "TH_CR1 opens commissioning window on DUT with duration set to value from BasicCommissioningInfo", "New commissioning window should open and be set to timeout"),
            TestStep(6, "TH_CR3 fully commissions DUT_CE", "DUT should fully commission to TH_CR3"),
            TestStep(7, "TH_CR2 reads the Fabrics attribute from the Node Operational Credentials cluster using a non-fabric-filtered read",
                     "Verify the list shows initial_number_of_fabrics + 2 fabrics"),
            TestStep(8, "Verify DUT_CE is now discoverable over DNS-SD with 3 Operational service records (_matter._tcp SRV records)."),
            TestStep(9, "TH_CR2 reads the CurrentFabricIndex from the Node Operational Credentials cluster and saves as fabric_idx_cr2"),
            TestStep(10, "TH_CR2 sends RemoveFabric with FabricIndex = fabric_idx_cr2 command to DUT_CE",
                     "Verify DUT_CE responses with NOCResponse with a StatusCode OK"),
            TestStep(11, "TH_CR2 reads the Basic Information Cluster’s NodeLabel attribute of DUT_CE",
                     "Verify read/write commands fail as expected since the DUT_CE is no longer on the network"),
            TestStep(12, "TH_CR1 reads the list of Fabrics on DUT_CE",
                     "Verify the list shows initial_number_of_fabrics + 1 fabrics and fabric_idx_cr2 is not included."),
            TestStep(13, "TH_CR1 sends a OpenCommissioningWindow command to DUT_CE using a commissioning timeout of max_window_duration"),
            TestStep(14, "TH_CR2 commissions DUT_CE", "Commissioning is successful"),
            TestStep(15, "TH_CR2 reads the Fabrics attribute from the Node Operational Credentials cluster using a non-fabric-filtered read",
                     "Verify the list shows initial_number_of_fabrics + 2 fabrics and fabric_idx_cr2 is not included, since a new fabric index should have been allocated."),
            TestStep(16, "TH_CR2 reads the CurrentFabricIndex from the Node Operational Credentials cluster and saves as fabric_idx_cr2_2"),
            TestStep(17, "TH_CR3 reads the CurrentFabricIndex from the Node Operational Credentials cluster and saves as fabric_idx_cr3"),
            TestStep(18, "TH_CR1 sends RemoveFabric with FabricIndex = fabric_idx_cr2_2 command to DUT_CE",
                     "Verify DUT_CE responses with NOCResponse with a StatusCode OK"),
            TestStep(19, "TH_CR1 sends RemoveFabric with FabricIndex = fabric_idx_cr3 command to DUT_CE",
                     "Verify DUT_CE responses with NOCResponse with a StatusCode OK"),
            TestStep(20, "TH_CR1 reads the list of Fabrics on DUT_CE",
                     "Verify the list shows initial_number_of_fabrics fabrics."),
        ]

    def pics_TC_CADMIN_1_15(self) -> list[str]:
        return ["CADMIN.S"]

    @async_test_body
    async def test_TC_CADMIN_1_15(self):
        # Establishing TH1 controller
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)

        # Establishing TH2 controller
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        # Establishing TH3 controller
        th3_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th3_fabric_admin = th3_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th2.fabricId + 1)
        self.th3 = th3_fabric_admin.NewController(nodeId=3, useTestCommissioner=True)

        self.step(1)
        GC_cluster = Clusters.GeneralCommissioning
        attribute = GC_cluster.Attributes.BasicCommissioningInfo
        duration = await self.read_single_attribute_check_success(endpoint=0, cluster=GC_cluster, attribute=attribute)
        self.max_window_duration = duration.maxCumulativeFailsafeSeconds

        self.step(2)
        fabrics = await self.get_fabrics(th=self.th1)
        initial_number_of_fabrics = len(fabrics)

        self.step(3)
        params = await self.OpenCommissioningWindow(th=self.th1, expectedErrCode=0x00)
        setupPinCode = params.setupPinCode

        self.step(4)
        await self.CommissionAttempt(setupPinCode, thnum=2, th=self.th2)

        self.step(5)
        params2 = await self.OpenCommissioningWindow(th=self.th1, expectedErrCode=0x00)
        setupPinCode2 = params2.setupPinCode

        self.step(6)
        await self.CommissionAttempt(setupPinCode2, thnum=3, th=self.th3)

        self.step(7)
        fabrics = await self.get_fabrics(th=self.th2)
        if len(fabrics) != initial_number_of_fabrics + 2:
            # len of fabrics is expected to be 3, if 3 not found then we assert failure here
            asserts.fail("Expected number of fabrics not correct")

        self.step(8)
        # Gathering instance names and servers associated in order to verify there are 3 service records for DUT.
        mdns = MdnsDiscovery()
        services = await MdnsDiscovery.get_all_services(mdns, log_output=True)
        compressed_fabric_id = self.default_controller.GetCompressedFabricId()
        instance_name = f'{compressed_fabric_id:016X}-{self.dut_node_id:016X}'
        instance_qname = f"{instance_name}.{MdnsServiceType.OPERATIONAL.value}"
        self.print_step("instance qname", instance_qname)

        target_server = None
        instance_names = []
        for service in services['_matter._tcp.local.']:
            if service.instance_name not in instance_names:
                if service.instance_name in instance_qname:
                    target_server = service.server

            if target_server != None:
                instance_names.append(service.instance_name)

        asserts.assert_equal(3, len(instance_names),
                             f"Did not get back the expected number of instances as we expected 3, but got len(instance_names)")

        self.step(9)
        fabric_idx_cr2 = await self.read_currentfabricindex(th=self.th2)

        self.step(10)
        removeFabricCmd = Clusters.OperationalCredentials.Commands.RemoveFabric(fabric_idx_cr2)
        await self.th2.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=removeFabricCmd)

        self.step(11)
        # Verifies TH_CR2 is unable to read the Basic Information Cluster’s NodeLabel attribute of DUT_CE as no longer on network
        try:
            await self.read_single_attribute_check_success(
                dev_ctrl=self.th2,
                endpoint=0,
                cluster=Clusters.BasicInformation,
                attribute=Clusters.BasicInformation.Attributes.NodeLabel
            )
            asserts.fail("Expected exception not thrown")
        except ChipStackError as e:
            # Verify that the DUT returns an "Timeout" status response
            asserts.assert_equal(e.err, 0x00000032,
                                 "Expected to timeout as DUT_CE is no longer on network")

        self.step(12)
        fabrics2 = await self.get_fabrics(th=self.th1)
        fabric_indexes = [fabric.fabricIndex for fabric in fabrics2]
        if len(fabrics2) != initial_number_of_fabrics + 1:
            # len of fabrics is expected to be 2, if 2 not found then we assert failure
            asserts.fail(f"Expected number of fabrics not correct, should show 2, but instead shows {str(len(fabrics2))}")

        if fabric_idx_cr2 in fabric_indexes:
            asserts.fail("fabricIndexes should consist of indexes 1 and 3 at this point")

        self.step(13)
        params3 = await self.OpenCommissioningWindow(self.th1, expectedErrCode=0x00)
        setupPinCode3 = params3.setupPinCode

        self.step(14)
        await self.CommissionAttempt(setupPinCode3, thnum=2, th=self.th2)

        self.step(15)
        fabrics3 = await self.get_fabrics(th=self.th2)
        fabric_indexes2 = [fabric.fabricIndex for fabric in fabrics3]
        if len(fabrics3) != initial_number_of_fabrics + 2:
            # len of fabrics is expected to be 3, if 3 not found then we assert failure
            asserts.fail("Expected number of fabrics not correct")

        if fabric_idx_cr2 in fabric_indexes2:
            asserts.fail("fabricIndexes should not consist of fabric_idx_cr2, but it appears it was")

        self.step(16)
        fabric_idx_cr2_2 = await self.read_currentfabricindex(th=self.th2)

        self.step(17)
        fabric_idx_cr3 = await self.read_currentfabricindex(th=self.th3)

        self.step(18)
        removeFabricCmd2 = Clusters.OperationalCredentials.Commands.RemoveFabric(fabric_idx_cr2_2)
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=removeFabricCmd2)

        self.step(19)
        removeFabricCmd3 = Clusters.OperationalCredentials.Commands.RemoveFabric(fabric_idx_cr3)
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=removeFabricCmd3)

        self.step(20)
        fabrics4 = await self.get_fabrics(th=self.th1)
        if len(fabrics4) > initial_number_of_fabrics:
            asserts.fail(
                f"Expected number of fabrics not correct, should be {str(initial_number_of_fabrics)}, but instead found {str(len(fabrics4))}")


if __name__ == "__main__":
    default_matter_test_main()
