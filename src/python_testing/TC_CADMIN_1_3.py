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

import ipaddress
import random
import time
from datetime import timedelta

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts
from zeroconf import ServiceBrowser, ServiceListener, Zeroconf


class CommissionableListener(ServiceListener):
    def __init__(self, long_discriminator: int):
        self.services: dict[str, str] = {}
        self.long_discriminator = long_discriminator

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        print(f"-------------------------------------------------Service {name} updated")

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        print(f"-------------------------------------------------Service {name} removed")
        # TODO: Delete from dir

    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        print(f"-------------------------------------------------Service {name} added, service info: {info}")

        info = zc.get_service_info(type_, name)
        # TODO: check for the long discriminator
        print(f"-------------------------------------------------Service {name} added, service info: {info}")
        self.services[name] = type_


class TC_CADMIN_1_3(MatterBaseTest):
    @async_test_body
    async def test_TC_CADMIN_1_3(self):
        self.print_step(1, "Commissioning - already done")
        TH_CR1 = self.default_controller

        # setup the DNS-SD browser first so we don't miss any records when the commissioning window opens
        discriminator = 1234  # random.randint(1, 4095)
        zeroconf = Zeroconf()
        listener = CommissionableListener(discriminator)

        self.print_step("2.a", f"TH_CR1 opens a commissioning window using ECM with discriminator {discriminator}")
        params = TH_CR1.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=600, iteration=10000, discriminator=discriminator, option=1)

        self.print_step("2.b", "Verify DUT is advertising with TXT record key-value pair CM=2")
        # TODO: Do this wih a semaphore
        browser = ServiceBrowser(zeroconf, "_matter._tcp.local.", listener)
        # TODO actually check

        # time.sleep(15)
        # zeroconf.close()

        self.print_step("2.c", "TH_CR1 writes and reads basic information cluster NodeLabel")
        # TODO write
        ret = self.read_single_attribute_check_success(
            dev_ctrl=TH_CR1, cluster=Clusters.BasicInformation, attribute=Clusters.BasicInformation.Attributes.NodeLabel)

        self.print_step(3, "TH_CR2 commissions DUT")
        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=2)
        TH_CR2 = new_fabric_admin.NewController(nodeId=112233)
        errcode = TH_CR2.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=1234)
        asserts.assert_true(errcode.is_success, 'Commissioning did not complete successfully')

        self.print_step(4, "Verify DUT is advertising operational records for both fabrics")

        self.print_step(5, "TH_CR1 reads the Fabrics attribute and verify there are two entires matching the NOCs previously commissioned")

        self.print_step(6, "TH_CR2 reads the Fabrics attribute and verify there are two entires matching the NOCs previously commissioned")

        self.print_step(7, "TH_CR1 write and reads the NodeLabel attribute of the basic information cluster")

        self.print_step(8, "TH_CR2 reads, writes, then reads the NodeLabel attribute of the basic information cluster and verifies the first read corresponds to the value set in the previous step")

        self.print_step(9, "TH_CR2 opens a commissioning window on the DUT using ECM")

        self.print_step(10, "Wait for the commissioning window in step 9 to timeout")

        self.print_step(11, "TH_CR2 reads the window status to verify that the DUT window is closed")

        self.print_step(12, "TH_CR2 opens a commissioning window on the DUT using ECM")

        self.print_step(13, "TH_CH2 starts a commissioning process. Verify this fails with FabricConflict error")


if __name__ == "__main__":
    default_matter_test_main()
