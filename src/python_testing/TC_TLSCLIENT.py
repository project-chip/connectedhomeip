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

import random
import string
from datetime import timedelta

import test_plan_support
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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
from mobly import asserts
from TC_TLS_Utils import TLSUtils

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main
from matter.utils import CommissioningBuildingBlocks


class TC_TLSCLIENT(MatterBaseTest):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    class TwoFabricData:
        def __init__(self, cr1_cmd: TLSUtils, cr2_cmd: TLSUtils):
            self.cr1_cmd = cr1_cmd
            self.cr2_cmd = cr2_cmd

    # This test creates the maximum number of endpoints, which can take some time
    @property
    def default_timeout(self) -> int:
        return timedelta(minutes=5).total_seconds()

    async def common_setup(self):
        self.step(1)
        endpoint = self.get_endpoint()
        cr1_cmd = TLSUtils(self, endpoint=endpoint)
        cr1 = self.default_controller

        self.step(2)
        # Establishing CR2 controller
        cr2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        cr2_fabric_admin = cr2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=cr1.fabricId + 1)
        cr2 = cr2_fabric_admin.NewController(nodeId=cr1.nodeId + 1)
        cr2_dut_node_id = self.dut_node_id+1

        self.step(3)
        _, noc_resp, _ = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
            commissionerDevCtrl=cr1, newFabricDevCtrl=cr2,
            existingNodeId=self.dut_node_id, newNodeId=cr2_dut_node_id
        )
        fabric_index_cr2 = noc_resp.fabricIndex

        cr2_cmd = TLSUtils(self, endpoint=endpoint, dev_ctrl=cr2, node_id=cr2_dut_node_id, fabric_index=fabric_index_cr2)
        return self.TwoFabricData(cr1_cmd=cr1_cmd, cr2_cmd=cr2_cmd)

    def get_common_steps(self) -> list[TestStep]:
        return [
            TestStep(1, test_plan_support.commission_if_required('CR1'), is_commissioning=True),
            TestStep(2, test_plan_support.open_commissioning_window()),
            TestStep(3, test_plan_support.commission_from_existing('CR1', 'CR2')),
        ]

    def pics_TC_TLSCLIENT_2_1(self):
        return ["TLSCLIENT.S"]

    def desc_TC_TLSCLIENT_2_1(self) -> str:
        return "[TC-TLSCLIENT-2.1] Attributes with Server as DUT"

    def steps_TC_TLSCLIENT_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, test_plan_support.commission_if_required('CR1'), is_commissioning=True),
            TestStep(2, "TH reads MaxProvisioned attribute", "DUT replies with an uint8 value between 5 and 254."),
            TestStep(3, "TH reads ProvisionedEndpoints attribute", "DUT replies with an empty list of TLSEndpointStruct."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsClientManagement))
    async def test_TC_TLSCLIENT_2_1(self):
        self.step(1)
        attributes = Clusters.TlsClientManagement.Attributes
        cr1_cmd = TLSUtils(self, endpoint=self.get_endpoint())

        self.step(2)
        max_provisioned = await cr1_cmd.read_tls_client_attribute(attributes.MaxProvisioned)
        asserts.assert_greater_equal(max_provisioned, 5, "MaxProvisioned should be >= 5")
        asserts.assert_less_equal(max_provisioned, 254, "MaxProvisioned should be <= 254")

        self.step(3)
        provisioned_endpoints = await cr1_cmd.read_tls_client_attribute(attributes.ProvisionedEndpoints)
        asserts.assert_equal(len(provisioned_endpoints), 0, "ProvisionedEndpoints should be empty")

    def pics_TC_TLSCLIENT_3_1(self):
        return ["TLSCLIENT.S", "TLSCERT.S"]

    def desc_TC_TLSCLIENT_3_1(self) -> str:
        return "[TC-TLSCLIENT-3.1] ProvisionEndpoint command basic insertion and modification"

    def steps_TC_TLSCLIENT_3_1(self) -> list[TestStep]:
        return [
            *self.get_common_steps(),
            TestStep(4, "Set myHostname to a valid value."),
            TestStep(5, "Populate myPort[] 3 distinct valid values."),
            TestStep(6, "Populate myRootCert[] with 2 distinct valid values."),
            TestStep(7, "CR1 sends ProvisionRootCertificate command to the TLSCertificateManagementCluster with null CAID and Certificate set to myRootCert[0].",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid[0]."),
            TestStep(8, "CR2 sends ProvisionRootCertificate command to the TLSCertificateManagementCluster with null CAID and Certificate set to myRootCert[1].",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid[1]."),
            TestStep(9, "Populate myNonce[] with 2 distinct, random 32-octet values"),
            TestStep(10, "CR1 sends ClientCSR command with Nonce set to myNonce[0]",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid[0] and CSR in myCsr[0]."),
            TestStep(11, "CR2 sends ClientCSR command with Nonce set to myNonce[1].",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid[1] and CSR in myCsr[1]."),
            TestStep(12,
                     "Populate myClientCert[] with 2 distinct, valid, self-signed, DER-encoded x509 certificates using each respective public key from myCsr[i]."),
            TestStep(
                13, "CR1 sends ProvisionClientCertificate command to the TLSCertificateManagementCluster with CCDID set to myCcdid[0] and ClientCertificateDetails set to myClientCert[0].", test_plan_support.verify_success()),
            TestStep(
                14, "CR2 sends ProvisionClientCertificate command to the TLSCertificateManagementCluster with CCDID set to myCcdid[1] and ClientCertificateDetails set to myClientCert[1].", test_plan_support.verify_success()),
            TestStep(15, "CR1 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[0], CAID myCaid[0], CCDID myCcdid[0] and null EndpointID.",
                     "DUT replies with a TLSEndpointID value. Store the returned value as myEndpoint[0]."),
            TestStep(16, "CR1 reads ProvisionedEndpoints attribute.",
                     "DUT replies with a list of TLSEndpointStruct with one entry. The entry should correspond to (myEndpoint[0], myHostname, myPort[0], myCaid[0], myCcdid[0], myReferenceCount) where myReferenceCount is 0"),
            TestStep(17, "CR2 reads ProvisionedEndpoints attribute.", "DUT replies with an empty list of TLSEndpointStruct."),
            TestStep(18, "CR2 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[1], CAID myCaid[1], CCDID myCcdid[1] and null EndpointID.",
                     "DUT replies with a TLSEndpointID value equal to myEndpoint[1]."),
            TestStep(19, "CR1 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[2], CAID myCaid[0], CCDID myCcdid[0] and EndpointID myEndpoint[0].",
                     "DUT replies with a TLSEndpointID value equal to myEndpoint[0]."),
            TestStep(20, "CR1 reads ProvisionedEndpoints attribute.",
                     "DUT replies with a list of TLSEndpointStruct with one entry. The entry should correspond to (myEndpoint[0], myHostname, myPort[2], myCaid[0], myCcdid[0], myReferenceCount) where myReferenceCount is 0"),
            TestStep(21, "CR2 reads ProvisionedEndpoints attribute.",
                     "DUT replies with a list of TLSEndpointStruct with one entry. The entry should correspond to (myEndpoint[1], myHostname, myPort[1], myCaid[1], myCcdid[1], myReferenceCount) where myReferenceCount is 0"),
            TestStep(
                22, "CR1 sends RemoveEndpoint command with EndpointID set to myEndpoint[0].", test_plan_support.verify_success()),
            TestStep(23, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsClientManagement))
    async def test_TC_TLSCLIENT_3_1(self):
        setup_data = await self.common_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd
        attributes = Clusters.TlsClientManagement.Attributes

        self.step(4)
        my_hostname = b"myhostname.matter.com"

        self.step(5)
        my_port = [1234, 5678, 9012]

        self.step(6)
        my_root_cert = [cr1_cmd.gen_cert(), cr1_cmd.gen_cert()]
        my_caid = [None, None]

        self.step(7)
        res = await cr1_cmd.send_provision_root_command(certificate=my_root_cert[0])
        my_caid[0] = res.caid

        self.step(8)
        res = await cr2_cmd.send_provision_root_command(certificate=my_root_cert[1])
        my_caid[1] = res.caid

        self.step(9)
        my_nonce = [random.randbytes(32), random.randbytes(32)]
        my_ccdid = [None, None]
        my_csr = [None, None]

        self.step(10)
        response = await cr1_cmd.send_csr_command(nonce=my_nonce[0])
        cr1_cmd.assert_valid_ccdid(response.ccdid)
        my_ccdid[0] = response.ccdid
        my_csr[0] = cr2_cmd.assert_valid_csr(response, my_nonce[0])

        self.step(11)
        response = await cr2_cmd.send_csr_command(nonce=my_nonce[1])
        cr2_cmd.assert_valid_ccdid(response.ccdid)
        my_ccdid[1] = response.ccdid
        my_csr[1] = cr2_cmd.assert_valid_csr(response, my_nonce[1])

        self.step(12)
        my_client_cert = [None, None]
        root = cr1_cmd.get_key()
        for i in range(2):
            my_client_cert[i] = cr1_cmd.gen_cert_with_key(root, public_key=my_csr[i].public_key(), subject=my_csr[i].subject)

        self.step(13)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid[0], certificate=my_client_cert[0])

        self.step(14)
        await cr2_cmd.send_provision_client_command(ccdid=my_ccdid[1], certificate=my_client_cert[1])

        self.step(15)
        my_endpoint = [None, None]
        res = await cr1_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[0], caid=my_caid[0], ccdid=my_ccdid[0])
        asserts.assert_is_not_none(res.endpointID)
        my_endpoint[0] = res.endpointID

        self.step(16)
        endpoints = await cr1_cmd.read_tls_client_attribute(attributes.ProvisionedEndpoints)
        asserts.assert_equal(len(endpoints), 1)
        asserts.assert_equal(endpoints[0].endpointID, my_endpoint[0])
        asserts.assert_equal(endpoints[0].hostname, my_hostname)
        asserts.assert_equal(endpoints[0].port, my_port[0])
        asserts.assert_equal(endpoints[0].caid, my_caid[0])
        asserts.assert_equal(endpoints[0].ccdid, my_ccdid[0])
        asserts.assert_equal(endpoints[0].referenceCount, 0)

        self.step(17)
        endpoints = await cr2_cmd.read_tls_client_attribute(attributes.ProvisionedEndpoints)
        asserts.assert_equal(len(endpoints), 0)

        self.step(18)
        res = await cr2_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[1], caid=my_caid[1], ccdid=my_ccdid[1])
        asserts.assert_is_not_none(res.endpointID)
        my_endpoint[1] = res.endpointID

        self.step(19)
        res = await cr1_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[2], caid=my_caid[0], ccdid=my_ccdid[0], endpoint_id=my_endpoint[0])
        asserts.assert_equal(res.endpointID, my_endpoint[0])

        self.step(20)
        endpoints = await cr1_cmd.read_tls_client_attribute(attributes.ProvisionedEndpoints)
        asserts.assert_equal(len(endpoints), 1)
        asserts.assert_equal(endpoints[0].endpointID, my_endpoint[0])
        asserts.assert_equal(endpoints[0].hostname, my_hostname)
        asserts.assert_equal(endpoints[0].port, my_port[2])
        asserts.assert_equal(endpoints[0].caid, my_caid[0])
        asserts.assert_equal(endpoints[0].ccdid, my_ccdid[0])
        asserts.assert_equal(endpoints[0].referenceCount, 0)

        self.step(21)
        endpoints = await cr2_cmd.read_tls_client_attribute(attributes.ProvisionedEndpoints)
        asserts.assert_equal(len(endpoints), 1)
        asserts.assert_equal(endpoints[0].endpointID, my_endpoint[1])
        asserts.assert_equal(endpoints[0].hostname, my_hostname)
        asserts.assert_equal(endpoints[0].port, my_port[1])
        asserts.assert_equal(endpoints[0].caid, my_caid[1])
        asserts.assert_equal(endpoints[0].ccdid, my_ccdid[1])
        asserts.assert_equal(endpoints[0].referenceCount, 0)

        self.step(22)
        await cr1_cmd.send_remove_tls_endpoint_command(endpoint_id=my_endpoint[0])

        self.step(23)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)

    def pics_TC_TLSCLIENT_3_2(self):
        return ["TLSCLIENT.S", "TLSCERT.S"]

    def desc_TC_TLSCLIENT_3_2(self) -> str:
        return "[TC-TLSCLIENT-3.2] ProvisionEndpoint command verification with several entries"

    def steps_TC_TLSCLIENT_3_2(self) -> list[TestStep]:
        return [
            *self.get_common_steps(),
            TestStep(4, "Set myHostname to a valid value."),
            TestStep(5, "Populate myPort[] with myMaxProvisioned + 3 distinct valid values."),
            TestStep(6, "Populate myRootCert[] with 2 distinct, valid, self-signed, DER-encoded x509 certificates"),
            TestStep(
                7, "CR1 sends ProvisionRootCertificate command to the TLSCertificateManagementCluster with null CAID and Certificate set to myRootCert[0].", "DUT replies with a TLSCAID value. Store the returned value as myCaid[0]."),
            TestStep(
                8, "CR2 sends ProvisionRootCertificate command to the TLSCertificateManagementCluster with null CAID and Certificate set to myRootCert[1].", "DUT replies with a TLSCAID value. Store the returned value as myCaid[1]."),
            TestStep(9, "Populate myNonce[] with 2 distinct, random 32-octet values"),
            TestStep(10, "CR1 sends ClientCSR command with Nonce set to myNonce[0]",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid[0] and CSR in myCsr[0]."),
            TestStep(11, "CR2 sends ClientCSR command with Nonce set to myNonce[1].",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid[1] and CSR in myCsr[1]."),
            TestStep(12,
                     "Populate myClientCert[] with 2 distinct, valid, self-signed, DER-encoded x509 certificates using each respective public key from myCsr[i]."),
            TestStep(
                13, "CR1 sends ProvisionClientCertificate command to the TLSCertificateManagementCluster with CCDID set to myCcdid[0] and ClientCertificateDetails set to myClientCert[0].", test_plan_support.verify_success()),
            TestStep(
                14, "CR2 sends ProvisionClientCertificate command to the TLSCertificateManagementCluster with CCDID set to myCcdid[1] and ClientCertificateDetails set to myClientCert[1].", test_plan_support.verify_success()),
            TestStep(15, "CR1 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[0], CAID myCaid[0], CCDID myCcdid[0] and null EndpointID.",
                     "DUT replies with a TLSEndpointID value. Store the returned value as myEndpoint[0]."),
            TestStep(16, "CR2 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[i], CAID myCaid[1], CCDID myCcdid[1] and null EndpointID, for i in (1..myMaxProvisioned+1).",
                     "DUT replies with a TLSEndpointID value. Store the returned value as myEndpoint[i]."),
            TestStep(17, "CR1 reads ProvisionedEndpoints attribute.",
                     "DUT replies with a list of TLSEndpointStruct with 1 entry. The entries should correspond to (myEndpoint[0], myHostname, myPort[i], myCaid[0], myCcdid[0], myReferenceCount) where myReferenceCount is 0"),
            TestStep(18, "CR2 reads ProvisionedEndpoints attribute.",
                     "DUT replies with a list of TLSEndpointStruct with myMaxProvisioned entries. The entries should correspond to (myEndpoint[i], myHostname, myPort[i], myCaid[1], myCcdid[1], myReferenceCount) where myReferenceCount is 0, and i is [2..myMaxProvisioned-1]."),
            TestStep(19, "CR2 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[myMaxProvisioned], CAID myCaid[1], CCDID myCcdid[1] and null EndpointID.", test_plan_support.verify_status(
                Status.ResourceExhausted)),
            TestStep(20, "CR2 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[myMaxProvisioned+1], CAID myCaid[1], CCDID myCcdid[1] and EndpointID set to myEndpoint[1]",
                     "DUT replies with a TLSEndpointID value equal to myEndpoint[1]"),
            TestStep(
                21, "CR2 sends RemoveEndpoint command with EndpointID set to myEndpoint[2]", test_plan_support.verify_success()),
            TestStep(22, "CR2 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[myMaxProvisioned+2], CAID myCaid[1], CCDID myCcdid[1] and null EndpointID, for i in (1..myMaxProvisioned+1).",
                     "DUT replies with a TLSEndpointID value. Store the returned value as myEndpoint[myMaxProvisioned+1]."),
            TestStep(23, "CR2 reads ProvisionedEndpoints attribute.",
                     "DUT replies with a list of TLSEndpointStruct with myMaxProvisioned entries. The entries should correspond to (myEndpoint[i], myHostname, myPort[i], myCaid[1], myCcdid[1], myReferenceCount) where myReferenceCount is 0, and i is [2..myMaxProvisioned+2]."),
            TestStep(
                24, "CR1 sends RemoveEndpoint command with EndpointID set to myEndpoint[0]", test_plan_support.verify_success()),
            TestStep(25, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsClientManagement))
    async def test_TC_TLSCLIENT_3_2(self):
        setup_data = await self.common_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd
        attributes = Clusters.TlsClientManagement.Attributes

        max_provisioned = await cr1_cmd.read_tls_client_attribute(attributes.MaxProvisioned)

        self.step(4)
        my_hostname = b"myhostname.matter.com"

        self.step(5)
        my_port = [1000 + i for i in range(max_provisioned + 3)]

        self.step(6)
        my_root_cert = [cr1_cmd.gen_cert(), cr1_cmd.gen_cert()]
        my_caid = [None, None]

        self.step(7)
        res = await cr1_cmd.send_provision_root_command(certificate=my_root_cert[0])
        my_caid[0] = res.caid

        self.step(8)
        res = await cr2_cmd.send_provision_root_command(certificate=my_root_cert[1])
        my_caid[1] = res.caid

        self.step(9)
        my_nonce = [random.randbytes(32), random.randbytes(32)]
        my_ccdid = [None, None]
        my_csr = [None, None]

        self.step(10)
        response = await cr1_cmd.send_csr_command(nonce=my_nonce[0])
        cr1_cmd.assert_valid_ccdid(response.ccdid)
        my_ccdid[0] = response.ccdid
        my_csr[0] = cr2_cmd.assert_valid_csr(response, my_nonce[0])

        self.step(11)
        response = await cr2_cmd.send_csr_command(nonce=my_nonce[1])
        cr2_cmd.assert_valid_ccdid(response.ccdid)
        my_ccdid[1] = response.ccdid
        my_csr[1] = cr2_cmd.assert_valid_csr(response, my_nonce[1])

        self.step(12)
        my_client_cert = [None, None]
        root = cr1_cmd.get_key()
        for i in range(2):
            my_client_cert[i] = cr1_cmd.gen_cert_with_key(root, public_key=my_csr[i].public_key(), subject=my_csr[i].subject)

        self.step(13)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid[0], certificate=my_client_cert[0])

        self.step(14)
        await cr2_cmd.send_provision_client_command(ccdid=my_ccdid[1], certificate=my_client_cert[1])
        my_endpoint = [None] * (max_provisioned + 2)

        self.step(15)
        res = await cr1_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[0], caid=my_caid[0], ccdid=my_ccdid[0])
        asserts.assert_is_not_none(res.endpointID)
        my_endpoint[0] = res.endpointID

        self.step(16)
        for i in range(1, max_provisioned+1):
            res = await cr2_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[i], caid=my_caid[1], ccdid=my_ccdid[1])
            asserts.assert_is_not_none(res.endpointID)
            my_endpoint[i] = res.endpointID

        self.step(17)
        endpoints1 = await cr1_cmd.read_tls_client_attribute(attributes.ProvisionedEndpoints)
        asserts.assert_equal(len(endpoints1), 1)

        self.step(18)
        endpoints2 = await cr2_cmd.read_tls_client_attribute(attributes.ProvisionedEndpoints)
        asserts.assert_equal(len(endpoints2), max_provisioned)
        found_endpoints = {}
        for ep in endpoints2:
            found_endpoints[ep.endpointID] = ep
        for i in range(1, max_provisioned+1):
            ep = found_endpoints[my_endpoint[i]]
            asserts.assert_equal(ep.hostname, my_hostname)
            asserts.assert_equal(ep.port, my_port[i])
            asserts.assert_equal(ep.caid, my_caid[1])
            asserts.assert_equal(ep.ccdid, my_ccdid[1])
            asserts.assert_equal(ep.referenceCount, 0)

        self.step(19)
        await cr2_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[max_provisioned+1], caid=my_caid[1], ccdid=my_ccdid[1], expected_status=Status.ResourceExhausted)

        self.step(20)
        res = await cr2_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[max_provisioned+1], caid=my_caid[1], ccdid=my_ccdid[1], endpoint_id=my_endpoint[1])
        asserts.assert_equal(res.endpointID, my_endpoint[1])

        self.step(21)
        await cr2_cmd.send_remove_tls_endpoint_command(endpoint_id=my_endpoint[1])

        self.step(22)
        res = await cr2_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[max_provisioned+2], caid=my_caid[1], ccdid=my_ccdid[1])
        asserts.assert_is_not_none(res.endpointID)
        my_endpoint[max_provisioned+1] = res.endpointID

        self.step(23)
        endpoints2 = await cr2_cmd.read_tls_client_attribute(attributes.ProvisionedEndpoints)
        asserts.assert_equal(len(endpoints2), max_provisioned)
        found_endpoints = {}
        for ep in endpoints2:
            found_endpoints[ep.endpointID] = ep
        for i in range(2, max_provisioned+2):
            ep = found_endpoints[my_endpoint[i]]
            asserts.assert_equal(ep.hostname, my_hostname)
            if i == max_provisioned+1:
                port = my_port[max_provisioned+2]
            else:
                port = my_port[i]
            asserts.assert_equal(ep.port, port)
            asserts.assert_equal(ep.caid, my_caid[1])
            asserts.assert_equal(ep.ccdid, my_ccdid[1])
            asserts.assert_equal(ep.referenceCount, 0)

        self.step(24)
        await cr1_cmd.send_remove_tls_endpoint_command(endpoint_id=my_endpoint[0])

        self.step(25)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)

    def pics_TC_TLSCLIENT_3_3(self):
        return ["TLSCLIENT.S", "TLSCERT.S"]

    def desc_TC_TLSCLIENT_3_3(self) -> str:
        return "[TC-TLSCLIENT-3.3] ProvisionEndpoint command verification with bad arguments"

    def steps_TC_TLSCLIENT_3_3(self) -> list[TestStep]:
        return [
            *self.get_common_steps(),
            TestStep(4, "Set myShortHostname to a value shorter than 4."),
            TestStep(5, "Set myLongHostname to a value longer than 253."),
            TestStep(6, "Set myHostname to a valid value."),
            TestStep(7, "Populate myPort[] with 2 distinct valid values."),
            TestStep(8, "Populate myRootCert[] with 2 distinct valid values."),
            TestStep(9, "CR1 sends ProvisionRootCertificate command to the TLSCertificateManagementCluster with null CAID and Certificate set to myRootCert[0].",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid[0]."),
            TestStep(10, "CR2 sends ProvisionRootCertificate command to the TLSCertificateManagementCluster with null CAID and Certificate set to myRootCert[1].",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid[1]."),
            TestStep(11, "Populate myNonce[] with 2 distinct, random 32-octet values"),
            TestStep(12, "CR1 sends ClientCSR command with Nonce set to myNonce[0]",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid[0] and CSR in myCsr[0]."),
            TestStep(13, "CR2 sends ClientCSR command with Nonce set to myNonce[1].",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid[1] and CSR in myCsr[1]."),
            TestStep(14,
                     "Populate myClientCert[] with 2 distinct, valid, self-signed, DER-encoded x509 certificates using each respective public key from myCsr[i]."),
            TestStep(15, "CR1 sends ProvisionClientCertificate command to the TLSCertificateManagementCluster with CCDID set to myCcdid[0] and ClientCertificateDetails set to myClientCert[0].",
                     test_plan_support.verify_success()),
            TestStep(16, "CR2 sends ProvisionClientCertificate command to the TLSCertificateManagementCluster with CCDID set to myCcdid[1] and ClientCertificateDetails set to myClientCert[1].",
                     test_plan_support.verify_success()),
            TestStep(17, "CR1 sends ProvisionEndpoint command with valid Hostname myShortHostname, Port myPort[0], CAID myCaid[0], CCDID myCcdid[0] and null EndpointID.",
                     test_plan_support.verify_status(Status.ConstraintError)),
            TestStep(18, "CR1 sends ProvisionEndpoint command with valid Hostname myLongHostname, Port myPort[0], CAID myCaid[0], CCDID myCcdid[0] and null EndpointID.",
                     test_plan_support.verify_status(Status.ConstraintError)),
            TestStep(19, "CR2 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[0], CAID myCaid[0], CCDID myCcdid[1] and null EndpointID.",
                     "DUT replies with the cluster-specific error RootCertificateNotFound."),
            TestStep(20, "CR2 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[0], CAID myCaid[1], CCDID myCcdid[0] and null EndpointID.",
                     "DUT replies with the cluster-specific error ClientCertificateNotFound."),
            TestStep(21, "CR1 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[0], CAID myCaid[0], CCDID myCcdid[0] and null EndpointID.",
                     "DUT replies with a TLSEndpointID value. Store the returned value as myEndpoint[0]."),
            TestStep(22, "CR1 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[0], CAID myCaid[0], CCDID myCcdid[0] and null EndpointID.",
                     "DUT replies with the cluster-specific error EndpointAlreadyInstalled."),
            TestStep(23, "CR2 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[0], CAID myCaid[1], CCDID myCcdid[1] and null EndpointID.",
                     "DUT replies with a TLSEndpointID value"),
            TestStep(24, "CR1 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[1], CAID myCaid[0], CCDID myCcdid[0] and EndpointID set to myEndpoint + 1.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(25, "CR2 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[1], CAID myCaid[1], CCDID myCcdid[1] and EndpointID set to myEndpoint.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(26, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsClientManagement))
    async def test_TC_TLSCLIENT_3_3(self):
        setup_data = await self.common_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(4)
        my_short_hostname = b"abc"

        self.step(5)
        my_long_hostname = ''.join(random.choices(
            string.ascii_letters + string.digits, k=254)).encode('utf-8')

        self.step(6)
        my_hostname = b"myhostname.matter.com"

        self.step(7)
        my_port = [1234, 5678]

        self.step(8)
        my_root_cert = [cr1_cmd.gen_cert(), cr2_cmd.gen_cert()]
        my_caid = [None, None]

        self.step(9)
        res = await cr1_cmd.send_provision_root_command(certificate=my_root_cert[0])
        my_caid[0] = res.caid

        self.step(10)
        res = await cr2_cmd.send_provision_root_command(certificate=my_root_cert[1])
        my_caid[1] = res.caid

        self.step(11)
        my_nonce = [random.randbytes(32), random.randbytes(32)]
        my_ccdid = [None, None]
        my_csr = [None, None]

        self.step(12)
        response = await cr1_cmd.send_csr_command(nonce=my_nonce[0])
        my_ccdid[0] = response.ccdid
        my_csr[0] = cr1_cmd.assert_valid_csr(response, my_nonce[0])

        self.step(13)
        response = await cr2_cmd.send_csr_command(nonce=my_nonce[1])
        my_ccdid[1] = response.ccdid
        my_csr[1] = cr2_cmd.assert_valid_csr(response, my_nonce[1])

        self.step(14)
        my_client_cert = [None, None]
        root = cr1_cmd.get_key()
        for i in range(2):
            my_client_cert[i] = cr1_cmd.gen_cert_with_key(root, public_key=my_csr[i].public_key(), subject=my_csr[i].subject)

        self.step(15)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid[0], certificate=my_client_cert[0])

        self.step(16)
        await cr2_cmd.send_provision_client_command(ccdid=my_ccdid[1], certificate=my_client_cert[1])

        self.step(17)
        await cr1_cmd.send_provision_tls_endpoint_command(hostname=my_short_hostname, port=my_port[0], caid=my_caid[0], ccdid=my_ccdid[0], expected_status=Status.ConstraintError)

        self.step(18)
        await cr1_cmd.send_provision_tls_endpoint_command(hostname=my_long_hostname, port=my_port[0], caid=my_caid[0], ccdid=my_ccdid[0], expected_status=Status.ConstraintError)

        self.step(19)
        await cr2_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[0], caid=my_caid[0], ccdid=my_ccdid[1], expected_status=Clusters.TlsClientManagement.Enums.StatusCodeEnum.kRootCertificateNotFound)

        self.step(20)
        await cr2_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[0], caid=my_caid[1], ccdid=my_ccdid[0], expected_status=Clusters.TlsClientManagement.Enums.StatusCodeEnum.kClientCertificateNotFound)

        self.step(21)
        res = await cr1_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[0], caid=my_caid[0], ccdid=my_ccdid[0])
        asserts.assert_is_not_none(res.endpointID)
        my_endpoint_id = res.endpointID

        self.step(22)
        await cr1_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[0], caid=my_caid[0], ccdid=my_ccdid[0], expected_status=Clusters.TlsClientManagement.Enums.StatusCodeEnum.kEndpointAlreadyInstalled)

        self.step(23)
        await cr2_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[0], caid=my_caid[1], ccdid=my_ccdid[1])

        self.step(24)
        await cr1_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[1], caid=my_caid[0], ccdid=my_ccdid[0], endpoint_id=my_endpoint_id + 1, expected_status=Status.NotFound)

        self.step(25)
        await cr2_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[1], caid=my_caid[1], ccdid=my_ccdid[1], endpoint_id=my_endpoint_id, expected_status=Status.NotFound)

        self.step(26)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)

    def pics_TC_TLSCLIENT_3_10(self):
        return ["TLSCLIENT.S", "TLSCERT.S"]

    def desc_TC_TLSCLIENT_3_10(self) -> str:
        return "[TC-TLSCLIENT-3.10] FindEndpoint command verification"

    def steps_TC_TLSCLIENT_3_10(self) -> list[TestStep]:
        return [
            *self.get_common_steps(),
            TestStep(4, "Set myHostname to a valid value."),
            TestStep(5, "Set myPort to a valid port value."),
            TestStep(6, "Set myRootCert to a valid value."),
            TestStep(7, "CR1 sends ProvisionRootCertificate command to the TLSCertificateManagementCluster with null CAID and Certificate set to myRootCert.",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid."),
            TestStep(8, "Populate myNonce with a distinct, random 32-octet values"),
            TestStep(9, "CR1 sends ClientCSR command with Nonce set to myNonce",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid and CSR in myCsr."),
            TestStep(10,
                     "Populate myClientCert with a distinct, valid, self-signed, DER-encoded x509 certificates using each respective public key from myCsr."),
            TestStep(11, "CR1 sends ProvisionClientCertificate command to the TLSCertificateManagementCluster with CCDID set to myCcdid and ClientCertificateDetails set to myClientCert.",
                     test_plan_support.verify_success()),
            TestStep(12, "CR1 sends FindEndpoint command with EndpointID 0.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(13, "CR1 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort, CAID myCaid, CCDID myCcdid and null EndpointID.",
                     "DUT replies with a TLSEndpointID value. Store the returned value as myEndpoint."),
            TestStep(14, "CR1 sends FindEndpoint command with EndpointID set to myEndpoint.",
                     "DUT replies with a TLSEndpointStruct with value (myEndpoint, myHostname, myPort, myCaid, myCcdid, myReferenceCount), where myReferenceCount is 0."),
            TestStep(15, "CR1 sends FindEndpoint command with EndpointID set to myEndpoint + 1.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(16, "CR2 sends FindEndpoint command with EndpointID set to myEndpoint.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(
                17, "CR1 sends RemoveEndpoint command with EndpointID set to myEndpoint[0].", test_plan_support.verify_success()),
            TestStep(18, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsClientManagement))
    async def test_TC_TLSCLIENT_3_10(self):
        setup_data = await self.common_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(4)
        my_hostname = b"myhostname.matter.com"

        self.step(5)
        my_port = 1234

        self.step(6)
        my_root_cert = cr1_cmd.gen_cert()

        self.step(7)
        res = await cr1_cmd.send_provision_root_command(certificate=my_root_cert)
        my_caid = res.caid

        self.step(8)
        my_nonce = random.randbytes(32)

        self.step(9)
        res = await cr1_cmd.send_csr_command(nonce=my_nonce)
        my_ccdid = res.ccdid
        csr = cr1_cmd.assert_valid_csr(res, my_nonce)

        self.step(10)
        my_client_cert = cr1_cmd.gen_cert_with_key(
            cr1_cmd.get_key(), public_key=csr.public_key(), subject=csr.subject)

        self.step(11)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid, certificate=my_client_cert)

        self.step(12)
        await cr1_cmd.send_find_tls_endpoint_command(endpoint_id=0, expected_status=Status.NotFound)

        self.step(13)
        res = await cr1_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port, caid=my_caid, ccdid=my_ccdid)
        asserts.assert_is_not_none(res.endpointID)
        my_endpoint = res.endpointID

        self.step(14)
        res = await cr1_cmd.send_find_tls_endpoint_command(endpoint_id=my_endpoint)
        asserts.assert_equal(res.endpoint.endpointID, my_endpoint)
        asserts.assert_equal(res.endpoint.hostname, my_hostname)
        asserts.assert_equal(res.endpoint.port, my_port)
        asserts.assert_equal(res.endpoint.caid, my_caid)
        asserts.assert_equal(res.endpoint.ccdid, my_ccdid)
        asserts.assert_equal(res.endpoint.referenceCount, 0)

        self.step(15)
        await cr1_cmd.send_find_tls_endpoint_command(endpoint_id=my_endpoint + 1, expected_status=Status.NotFound)

        self.step(16)
        await cr2_cmd.send_find_tls_endpoint_command(endpoint_id=my_endpoint, expected_status=Status.NotFound)

        self.step(17)
        await cr1_cmd.send_remove_tls_endpoint_command(endpoint_id=my_endpoint)

        self.step(18)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)

    def pics_TC_TLSCLIENT_3_20(self):
        return ["TLSCLIENT.S", "TLSCERT.S"]

    def desc_TC_TLSCLIENT_3_20(self) -> str:
        return "[TC-TLSCLIENT-3.20] RemoveEndpoint command verification"

    def steps_TC_TLSCLIENT_3_20(self) -> list[TestStep]:
        return [
            *self.get_common_steps(),
            TestStep(4, "Set myHostname to a valid value."),
            TestStep(5, "Populate myPort[] with 3 distinct valid port values."),
            TestStep(6, "Populate myRootCert[] with 2 distinct valid values."),
            TestStep(7, "CR1 sends ProvisionRootCertificate command to the TLSCertificateManagementCluster with null CAID and Certificate set to myRootCert[0].",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid[0]."),
            TestStep(8, "CR2 sends ProvisionRootCertificate command to the TLSCertificateManagementCluster with null CAID and Certificate set to myRootCert[1].",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid[1]."),
            TestStep(9, "Populate myNonce[] with 2 distinct, random 32-octet values"),
            TestStep(10, "CR1 sends ClientCSR command with Nonce set to myNonce[0]",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid[0] and CSR in myCsr[0]."),
            TestStep(11, "CR2 sends ClientCSR command with Nonce set to myNonce[1].",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid[1] and CSR in myCsr[1]."),
            TestStep(12,
                     "Populate myClientCert[] with 2 distinct, valid, self-signed, DER-encoded x509 certificates using each respective public key from myCsr[i]."),
            TestStep(13, "CR1 sends ProvisionClientCertificate command to the TLSCertificateManagementCluster with CCDID set to myCcdid[0] and ClientCertificateDetails set to myClientCert[0].",
                     test_plan_support.verify_success()),
            TestStep(14, "CR2 sends ProvisionClientCertificate command to the TLSCertificateManagementCluster with CCDID set to myCcdid[1] and ClientCertificateDetails set to myClientCert[1].",
                     test_plan_support.verify_success()),
            TestStep(15, "CR1 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[0], CAID myCaid[0], CCDID myCcdid[0] and null EndpointID.",
                     "DUT replies with a TLSEndpointID value. Store the returned value as myEndpoint[0]."),
            TestStep(16, "CR1 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[1], CAID myCaid[0], CCDID myCcdid[0] and null EndpointID.",
                     "DUT replies with a TLSEndpointID value. Store the returned value as myEndpoint[1]."),
            TestStep(17, "CR2 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort[2], CAID myCaid[1], CCDID myCcdid[1] and null EndpointID.",
                     "DUT replies with a TLSEndpointID value. Store the returned value as myEndpoint[2]."),
            TestStep(18, "CR1 sends RemoveEndpoint command with EndpointID set to myEndpoint[0].",
                     test_plan_support.verify_success()),
            TestStep(19, "CR1 sends FindEndpoint command with EndpointID set to myEndpoint[0].",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(20, "CR1 sends RemoveEndpoint command with EndpointID set to myEndpoint[0].",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(21, "CR2 sends RemoveEndpoint command with EndpointID set to myEndpoint[1].",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(22, "CR1 sends RemoveEndpoint command with EndpointID set to myEndpoint[1].",
                     test_plan_support.verify_success()),
            TestStep(23, "CR2 sends RemoveRootCertificate command to the TLSCertificateManagementCluster with CAID myCaid[1].",
                     test_plan_support.verify_status(Status.InvalidInState)),
            TestStep(24, "CR2 sends RemoveClientCertificate command to the TLSCertificateManagementCluster with CAID myCcdid[1].",
                     test_plan_support.verify_status(Status.InvalidInState)),
            TestStep(25, "CR2 sends RemoveEndpoint command with EndpointID set to myEndpoint[2].",
                     test_plan_support.verify_success()),
            TestStep(26, "CR2 sends RemoveRootCertificate command to the TLSCertificateManagementCluster with CAID myCaid[1].",
                     test_plan_support.verify_success()),
            TestStep(27, "CR2 sends RemoveClientCertificate command to the TLSCertificateManagementCluster with CAID myCcdid[1].",
                     test_plan_support.verify_success()),
            TestStep(28, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsClientManagement))
    async def test_TC_TLSCLIENT_3_20(self):
        setup_data = await self.common_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(4)
        my_hostname = b"myhostname.matter.com"

        self.step(5)
        my_port = [1000 + i for i in range(3)]

        self.step(6)
        my_root_cert = [cr1_cmd.gen_cert(), cr2_cmd.gen_cert()]
        my_caid = [None, None]

        self.step(7)
        res = await cr1_cmd.send_provision_root_command(certificate=my_root_cert[0])
        my_caid[0] = res.caid

        self.step(8)
        res = await cr2_cmd.send_provision_root_command(certificate=my_root_cert[1])
        my_caid[1] = res.caid

        self.step(9)
        my_nonce = [random.randbytes(32), random.randbytes(32)]
        my_ccdid = [None, None]
        my_csr = [None, None]

        self.step(10)
        res = await cr1_cmd.send_csr_command(nonce=my_nonce[0])
        my_ccdid[0] = res.ccdid
        my_csr[0] = cr1_cmd.assert_valid_csr(res, my_nonce[0])

        self.step(11)
        res = await cr2_cmd.send_csr_command(nonce=my_nonce[1])
        my_ccdid[1] = res.ccdid
        my_csr[1] = cr2_cmd.assert_valid_csr(res, my_nonce[1])

        self.step(12)
        my_client_cert = [None, None]
        root = cr1_cmd.get_key()
        for i in range(2):
            my_client_cert[i] = cr1_cmd.gen_cert_with_key(root, public_key=my_csr[i].public_key(), subject=my_csr[i].subject)

        self.step(13)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid[0], certificate=my_client_cert[0])

        self.step(14)
        await cr2_cmd.send_provision_client_command(ccdid=my_ccdid[1], certificate=my_client_cert[1])

        my_endpoint = [None] * 3

        self.step(15)
        res = await cr1_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[0], caid=my_caid[0], ccdid=my_ccdid[0])
        asserts.assert_is_not_none(res.endpointID)
        my_endpoint[0] = res.endpointID

        self.step(16)
        res = await cr1_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[1], caid=my_caid[0], ccdid=my_ccdid[0])
        asserts.assert_is_not_none(res.endpointID)
        my_endpoint[1] = res.endpointID

        self.step(17)
        res = await cr2_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port[2], caid=my_caid[1], ccdid=my_ccdid[1])
        asserts.assert_is_not_none(res.endpointID)
        my_endpoint[2] = res.endpointID

        self.step(18)
        await cr1_cmd.send_remove_tls_endpoint_command(endpoint_id=my_endpoint[0])

        self.step(19)
        await cr1_cmd.send_find_tls_endpoint_command(endpoint_id=my_endpoint[0], expected_status=Status.NotFound)

        self.step(20)
        await cr1_cmd.send_remove_tls_endpoint_command(endpoint_id=my_endpoint[0], expected_status=Status.NotFound)

        self.step(21)
        await cr2_cmd.send_remove_tls_endpoint_command(endpoint_id=my_endpoint[1], expected_status=Status.NotFound)

        self.step(22)
        await cr1_cmd.send_remove_tls_endpoint_command(endpoint_id=my_endpoint[1])

        self.step(23)
        await cr2_cmd.send_remove_root_command(caid=my_caid[1], expected_status=Status.InvalidInState)

        self.step(24)
        await cr2_cmd.send_remove_client_command(ccdid=my_ccdid[1], expected_status=Status.InvalidInState)

        self.step(25)
        await cr2_cmd.send_remove_tls_endpoint_command(endpoint_id=my_endpoint[2])

        self.step(26)
        await cr2_cmd.send_remove_root_command(caid=my_caid[1])

        self.step(27)
        await cr2_cmd.send_remove_client_command(ccdid=my_ccdid[1])

        self.step(28)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)


if __name__ == "__main__":
    default_matter_test_main()
