#
#    Copyright (c) 2025 Project CHIP Authors
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

import string

import test_plan_support
from cryptography.x509 import CertificateBuilder, UnrecognizedExtension
from cryptography.x509.oid import ObjectIdentifier
from TC_TLS_Utils import TLSUtils

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep
from matter.utils import CommissioningBuildingBlocks


class TC_TLSCERT_Base(MatterBaseTest):
    """Base class for TC_TLSCERT test cases with common setup methods."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    class TwoFabricData:
        def __init__(self, cr1_cmd: TLSUtils, cr2_cmd: TLSUtils):
            self.cr1_cmd = cr1_cmd
            self.cr2_cmd = cr2_cmd

    def get_common_substeps(self, step_prefix: string = "1") -> list[TestStep]:
        return [
            TestStep(f'{step_prefix}.1', test_plan_support.commission_if_required('CR1'), is_commissioning=True),
            TestStep(f'{step_prefix}.2', "CR1 reads ProvisionedRootCertificates attribute using a fabric-filtered read",
                     "A list of TLSCertStruct. Store all resulting CAID in caidToClean."),
            TestStep(
                f'{step_prefix}.3', "CR1 sends RemoveRootCertificate command with CAID set to caidToClean[i] for all entries returned", test_plan_support.verify_success()),
            TestStep(f'{step_prefix}.4', "CR1 reads ProvisionedClientCertificates attribute using a fabric-filtered read",
                     "A list of TLSClientCertificateDetailStruct. Store all resulting CCDID in ccdidToClean."),
            TestStep(
                f'{step_prefix}.5', "CR1 sends RemoveClientCertificate command with CCDID set to ccdidToClean[i] for all entries returned.", test_plan_support.verify_success()),
        ]

    async def common_setup(self, step_prefix: string = "1") -> TLSUtils:
        self.step(f'{step_prefix}.1')
        attributes = Clusters.TlsCertificateManagement.Attributes
        endpoint = self.get_endpoint()

        # Establishing CR1 controller
        cr1_cmd = TLSUtils(self, endpoint=endpoint)

        self.step(f'{step_prefix}.2')
        root_certs = await cr1_cmd.read_tls_cert_attribute(attributes.ProvisionedRootCertificates)

        self.step(f'{step_prefix}.3')
        for cert in root_certs:
            await cr1_cmd.send_remove_root_command(caid=cert.caid)

        self.step(f'{step_prefix}.4')
        client_certs = await cr1_cmd.read_tls_cert_attribute(attributes.ProvisionedClientCertificates)

        self.step(f'{step_prefix}.5')
        for cert in client_certs:
            await cr1_cmd.send_remove_client_command(ccdid=cert.ccdid)

        return cr1_cmd

    def get_two_fabric_substeps(self, step_prefix: string = "1") -> list[TestStep]:
        return [
            *self.get_common_substeps(f'{step_prefix}.1'),
            TestStep(f'{step_prefix}.2', test_plan_support.open_commissioning_window()),
            TestStep(f'{step_prefix}.3', test_plan_support.commission_from_existing('CR1', 'CR2')),
        ]

    async def common_two_fabric_setup(self, step_prefix: string = "1") -> TwoFabricData:
        cr1_cmd = await self.common_setup(f'{step_prefix}.1')
        cr1 = self.default_controller
        endpoint = self.get_endpoint()

        self.step(f'{step_prefix}.2')
        # Establishing CR2 controller
        cr2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        cr2_fabric_admin = cr2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=cr1.fabricId + 1)
        cr2 = cr2_fabric_admin.NewController(nodeId=cr1.nodeId + 1)
        cr2_dut_node_id = self.dut_node_id + 1

        self.step(f'{step_prefix}.3')
        _, noc_resp, _ = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
            commissionerDevCtrl=cr1, newFabricDevCtrl=cr2,
            existingNodeId=self.dut_node_id, newNodeId=cr2_dut_node_id
        )
        fabric_index_cr2 = noc_resp.fabricIndex

        cr2_cmd = TLSUtils(self, endpoint=endpoint, dev_ctrl=cr2, node_id=cr2_dut_node_id, fabric_index=fabric_index_cr2)
        return self.TwoFabricData(cr1_cmd=cr1_cmd, cr2_cmd=cr2_cmd)

    def pad_out_certificate(self) -> CertificateBuilder:
        builder = CertificateBuilder()
        for i in range(100):
            builder = builder.add_extension(
                UnrecognizedExtension(
                    oid=ObjectIdentifier(f"1.1.{i}"),
                    value=b'\x23' * 20
                ),
                critical=False
            )
        return builder
