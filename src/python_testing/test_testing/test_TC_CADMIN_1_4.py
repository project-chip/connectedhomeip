#!/usr/bin/env -S python3 -B
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

import asyncio
import os
import random
import sys

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main
from chip.tlv import TLVReader
from mobly import asserts

# isort: off

parent_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(parent_dir)

from mdns_discovery import mdns_discovery

# isort: on

# Reachable attribute is off in the pics file
# MaxPathsPerInvoke is not include in the pics file
# Vendor ID is included on ON in the PICS file

opcreds = Clusters.OperationalCredentials
nonce = random.randbytes(32)


class TC_CADMIN_1_4_noreset(MatterBaseTest):
    async def get_fabrics(self, th: ChipDeviceCtrl) -> int:
        OC_cluster = Clusters.OperationalCredentials
        if th == self.th2:
            th2_fabric_info = await th.ReadAttribute(nodeid=self.dut_node_id, fabricFiltered=True, attributes=[(0, OC_cluster.Attributes.Fabrics)])
            th2_fabric_data = list(th2_fabric_info.values())[0]
            th2_fabric_data = list(th2_fabric_data.values())[0]
            fabric_info = vars(list(th2_fabric_data.values())[1][0])

        else:
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

    async def write_nl_attr(self, th: ChipDeviceCtrl, attr_val: object):
        result = await th.WriteAttribute(nodeid=self.dut_node_id, attributes=[(0, attr_val)])
        asserts.assert_equal(result[0].Status, Status.Success, f"{th} node label write failed")

    async def read_nl_attr(self, th: ChipDeviceCtrl, attr_val: object):
        try:
            await th.ReadAttribute(nodeid=self.dut_node_id, attributes=[(0, attr_val)])
        except Exception as e:
            asserts.assert_equal(e.err, "Received error message from read attribute attempt")
            self.print_step(0, e)

    async def read_currentfabricindex(self, th: ChipDeviceCtrl) -> int:
        cluster = Clusters.OperationalCredentials
        attribute = Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        current_fabric_index = await self.read_single_attribute_check_success(dev_ctrl=th, endpoint=0, cluster=cluster, attribute=attribute)
        return current_fabric_index

    @async_test_body
    async def test_TC_CADMIN_1_4(self):
        setupPayloadInfo = self.get_setup_payload_info()
        if not setupPayloadInfo:
            asserts.assert_true(
                False, 'passcode and discriminator must be provided values in order for this test to work due to using BCM, please rerun test with providing --passcode <value> and --discriminator <value>')

        # Establishing TH1
        self.th1 = self.default_controller

        GC_cluster = Clusters.GeneralCommissioning
        attribute = GC_cluster.Attributes.BasicCommissioningInfo
        duration = await self.read_single_attribute_check_success(endpoint=0, cluster=GC_cluster, attribute=attribute)
        self.max_window_duration = duration.maxCumulativeFailsafeSeconds

        obcCmd = Clusters.AdministratorCommissioning.Commands.OpenBasicCommissioningWindow(180)
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=obcCmd, timedRequestTimeoutMs=6000)

        services = await self.get_txt_record()
        if services.txt_record['CM'] != "1":
            asserts.fail(f"Expected cm record value not found, instead value found was {str(services.txt_record['CM'])}")

        BI_cluster = Clusters.BasicInformation
        nl_attribute = BI_cluster.Attributes.NodeLabel
        await self.write_nl_attr(th=self.th1, attr_val=nl_attribute)
        await self.read_nl_attr(th=self.th1, attr_val=nl_attribute)

        # Establishing TH2
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2)
        await self.th2.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=setupPayloadInfo[0].passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=setupPayloadInfo[0].filter_value)

        # TH_CR1 reads the Fabrics attribute from the Node Operational Credentials cluster using a fabric-filtered read
        th1_fabric_info = await self.get_fabrics(th=self.th1)

        # Verify that the RootPublicKey matches the root public key for TH_CR1 and the NodeID matches the node ID used when TH_CR1 commissioned the device.
        await self.send_single_cmd(dev_ctrl=self.th1, node_id=self.dut_node_id, cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(10))
        th1_rcac_decoded = await self.get_rcac_decoded(th=self.th1)
        if th1_fabric_info[0].rootPublicKey != th1_rcac_decoded[9]:
            asserts.fail("public keys from fabric and certs for TH1 are not the same")
        if th1_fabric_info[0].nodeID != self.dut_node_id:
            asserts.fail("DUT node ID from fabric does not equal DUT node ID for TH1 during commissioning")

        # Expiring the failsafe timer in an attempt to clean up before TH2 attempt.
        await self.th1.SendCommand(self.dut_node_id, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0))

        # TH_CR2 reads the Fabrics attribute from the Node Operational Credentials cluster using a fabric-filtered read
        th2_fabric_info = await self.get_fabrics(th=self.th2)

        # Verify that the RootPublicKey matches the root public key for TH_CR2 and the NodeID matches the node ID used when TH_CR2 commissioned the device.
        await self.send_single_cmd(dev_ctrl=self.th2, node_id=self.dut_node_id, cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(self.max_window_duration))
        th2_rcac_decoded = await self.get_rcac_decoded(th=self.th2)
        if th2_fabric_info['rootPublicKey'] != th2_rcac_decoded[9]:
            asserts.fail("public keys from fabric and certs for TH2 are not the same")
        if th2_fabric_info['nodeID'] != self.dut_node_id:
            asserts.fail("DUT node ID from fabric does not match DUT node ID for TH2 during commissioning")

        await self.th2.SendCommand(self.dut_node_id, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0))

        # TH_CR2 reads the CurrentFabricIndex attribute from the Operational Credentials cluster and saves as th2_idx, TH_CR1 sends the RemoveFabric command to the DUT with the FabricIndex set to th2_idx
        th2_idx = await self.th2.ReadAttribute(nodeid=self.dut_node_id, attributes=[(0, Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)])
        outer_key = list(th2_idx.keys())[0]
        inner_key = list(th2_idx[outer_key].keys())[0]
        attribute_key = list(th2_idx[outer_key][inner_key].keys())[1]
        removeFabricCmd = Clusters.OperationalCredentials.Commands.RemoveFabric(th2_idx[outer_key][inner_key][attribute_key])
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=removeFabricCmd)


if __name__ == "__main__":
    asyncio.run(default_matter_test_main())
    sys.exit(0)
