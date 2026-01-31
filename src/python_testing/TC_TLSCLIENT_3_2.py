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

import random

import test_plan_support
from mobly import asserts
from TC_TLSCLIENT_Base import TC_TLSCLIENT_Base

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main


class TC_TLSCLIENT_3_2(TC_TLSCLIENT_Base):
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
        my_csr[0] = cr1_cmd.assert_valid_csr(response, my_nonce[0])

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


if __name__ == "__main__":
    default_matter_test_main()
