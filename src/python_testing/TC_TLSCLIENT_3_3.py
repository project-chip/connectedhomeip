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
import string

import test_plan_support
from mobly import asserts
from TC_TLSCLIENT_Base import TC_TLSCLIENT_Base

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main


class TC_TLSCLIENT_3_3(TC_TLSCLIENT_Base):
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


if __name__ == "__main__":
    default_matter_test_main()
