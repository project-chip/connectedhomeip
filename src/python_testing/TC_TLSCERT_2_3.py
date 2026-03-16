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

import logging

import test_plan_support
from mobly import asserts
from TC_TLSCERT_Base import TC_TLSCERT_Base

import matter.clusters as Clusters
from matter.ChipDeviceCtrl import TransportPayloadCapability
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_TLSCERT_2_3(TC_TLSCERT_Base):
    def pics_TC_TLSCERT_2_3(self):
        return ["TLSCERT.S"]

    def desc_TC_TLSCERT_2_3(self) -> str:
        return "[TC-TLSCERT-2.3] ProvisionRootCertificate command verification with several entries"

    def steps_TC_TLSCERT_2_3(self) -> list[TestStep]:
        return [
            *self.get_two_fabric_substeps(),
            TestStep(2, "CR2 reads MaxRootCertificates attribute into myMaxRootCerts."),
            TestStep(3, "Populate myRootCert[] with myMaxRootCerts + 3 distinct, valid, self-signed, DER-encoded x509 certificates."),
            TestStep(4, "CR1 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[i], for each i in [0..1].",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid[i]."),
            TestStep(5, "CR2 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[i], for each i in [2..myMaxRootCerts+1].",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid[i]."),
            TestStep(6, "CR1 reads ProvisionedRootCertificates attribute using a fabric-filtered read on Large Message-capable transport.",
                     "DUT replies with a list of TLSCertStruct with 2 entries. The entries should correspond to the first two entries in myRootCert and myCaid."),
            TestStep(7, "CR2 reads ProvisionedRootCertificates attribute using a fabric-filtered read on Large Message-capable transport.",
                     "DUT replies with a list of TLSCertStruct with myMaxRootCerts entries. The entries should correspond to the entries 2 through myMaxRootCerts+1 of myRootCert and myCaid."),
            TestStep(8, "CR1 sends FindRootCertificate command with null CAID.",
                     "DUT replies with a list of TLSCertStruct with 2 entries. The entries should correspond to the first two entries in myRootCert and myCaid."),
            TestStep(9, "CR2 sends FindRootCertificate command with null CAID.",
                     "DUT replies with a list of TLSCertStruct with myMaxRootCerts entries. The entries should correspond to the entries 2 through myMaxRootCerts + 1 of myRootCert and myCaid."),
            TestStep(10, "CR2 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[myMaxRootCerts+2].",
                     test_plan_support.verify_status(Status.ResourceExhausted)),
            TestStep(11, "CR2 sends RemoveRootCertificate command with CAID set to myCaid[2].",
                     test_plan_support.verify_success()),
            TestStep(12, "CR2 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[myMaxRootCerts+2].",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid[myMaxRootCerts+2]."),
            TestStep(13, "CR2 reads ProvisionedRootCertificates attribute using a fabric-filtered read on Large Message-capable transport.",
                     "DUT replies with a list of TLSCertStruct with myMaxRootCerts entries."),
            TestStep(14, "CR2 sends FindRootCertificate command with null CAID.",
                     "DUT replies with a list of TLSCertStruct with myMaxRootCerts entries."),
            TestStep(15, "CR2 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[2].",
                     test_plan_support.verify_status(Status.ResourceExhausted)),
            TestStep(16, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
            TestStep(17, "CR1 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[2].",
                     "DUT replies with a TLSCAID value."),
            TestStep(18, "CR1 sends RemoveRootCertificate command with CAID set to myCaid[i], for each i in [0..1].",
                     test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_3(self):
        attributes = Clusters.TlsCertificateManagement.Attributes

        setup_data = await self.common_two_fabric_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(2)
        my_max_root_certs = await cr2_cmd.read_tls_cert_attribute(attributes.MaxRootCertificates)

        self.step(3)
        my_root_cert = [cr1_cmd.gen_cert() for _ in range(my_max_root_certs + 3)]
        my_caid = [None] * (my_max_root_certs + 3)

        self.step(4)
        for i in range(2):
            response = await cr1_cmd.send_provision_root_command(certificate=my_root_cert[i])
            cr1_cmd.assert_valid_caid(response.caid)
            my_caid[i] = response.caid

        self.step(5)
        log.warning(f"maximum cert {my_max_root_certs}")
        for i in range(2, my_max_root_certs + 2):
            log.warning(f"Root cert {i}")
            response = await cr2_cmd.send_provision_root_command(certificate=my_root_cert[i])
            cr2_cmd.assert_valid_caid(response.caid)
            my_caid[i] = response.caid

        self.step(6)
        found_certs = await cr1_cmd.read_root_certs_attribute_as_map(TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(len(found_certs), 2, "Expected 2 certificates for CR1")
        for i in range(2):
            asserts.assert_in(my_caid[i], found_certs)
            asserts.assert_equal(found_certs[my_caid[i]].certificate, my_root_cert[i])

        self.step(7)
        found_certs = await cr2_cmd.read_root_certs_attribute_as_map(TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(len(found_certs), my_max_root_certs, "Expected myMaxRootCerts certificates for CR2")
        for i in range(2, my_max_root_certs + 2):
            asserts.assert_in(my_caid[i], found_certs)
            asserts.assert_equal(found_certs[my_caid[i]].certificate, my_root_cert[i])

        self.step(8)
        find_response = await cr1_cmd.send_find_root_command()
        asserts.assert_equal(len(find_response.certificateDetails), 2, "Expected 2 certificates for CR1")

        self.step(9)
        find_response = await cr2_cmd.send_find_root_command()
        asserts.assert_equal(len(find_response.certificateDetails), my_max_root_certs,
                             "Expected myMaxRootCerts certificates for CR2")

        self.step(10)
        await cr2_cmd.send_provision_root_command(certificate=my_root_cert[my_max_root_certs + 2], expected_status=Status.ResourceExhausted)

        self.step(11)
        await cr2_cmd.send_remove_root_command(caid=my_caid[2])

        self.step(12)
        response = await cr2_cmd.send_provision_root_command(certificate=my_root_cert[my_max_root_certs + 2])
        cr2_cmd.assert_valid_caid(response.caid)
        my_caid[my_max_root_certs + 2] = response.caid

        self.step(13)
        found_certs = await cr2_cmd.read_root_certs_attribute_as_map(TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(len(found_certs), my_max_root_certs, "Expected myMaxRootCerts certificates for CR2")

        self.step(14)
        find_response = await cr2_cmd.send_find_root_command()
        asserts.assert_equal(len(find_response.certificateDetails), my_max_root_certs,
                             "Expected myMaxRootCerts certificates for CR2")

        self.step(15)
        await cr2_cmd.send_provision_root_command(certificate=my_root_cert[2], expected_status=Status.ResourceExhausted)

        self.step(16)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)

        self.step(17)
        response = await cr1_cmd.send_provision_root_command(certificate=my_root_cert[2])
        cr1_cmd.assert_valid_caid(response.caid)

        self.step(18)
        for i in range(2):
            await cr1_cmd.send_remove_root_command(caid=my_caid[i])


if __name__ == "__main__":
    default_matter_test_main()
