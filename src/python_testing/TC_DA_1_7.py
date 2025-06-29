#
#    Copyright (c) 2022 Project CHIP Authors
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --bool-arg allow_sdk_dac:true
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
from glob import glob
from pathlib import Path
from typing import List, Optional

import chip.clusters as Clusters
from chip.testing.conversions import bytes_from_hex, hex_from_bytes
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat
from cryptography.x509 import AuthorityKeyIdentifier, Certificate, SubjectKeyIdentifier, load_der_x509_certificate
from mobly import asserts

# Those are SDK samples that are known to be non-production.
FORBIDDEN_AKID = [
    bytes_from_hex("78:5C:E7:05:B8:6B:8F:4E:6F:C7:93:AA:60:CB:43:EA:69:68:82:D5"),
    bytes_from_hex("6A:FD:22:77:1F:51:1F:EC:BF:16:41:97:67:10:DC:DC:31:A1:71:7E")
]

# List of certificate names that are known to have some issues, but not yet
# updated in DCL. They will fail the test at runtime if seen, but not in CI.
ALLOWED_SKIPPED_FILENAMES = [
    "dcld_mirror_SERIALNUMBER_63709380400001_CN_NXP_Matter_Test_PAA_O_NXP_Semiconductors_NV_C_NL.der",
    "dcld_mirror_SERIALNUMBER_63709330400001_CN_NXP_Matter_PAA_O_NXP_Semiconductors_NV_C_NL.der"
]


def load_all_paa(paa_path: Path) -> dict:
    logging.info("Loading all PAAs in %s" % paa_path)

    paa_by_skid = {}
    for filename in glob(str(paa_path.joinpath("*.der"))):
        with open(filename, "rb") as derfile:
            logging.info(f"Loading PAA: {filename}")
            try:
                # Load cert
                paa_der = derfile.read()
                paa_cert = load_der_x509_certificate(paa_der)

                # Find the subject key identifier (if present), and record it
                for extension in paa_cert.extensions:
                    if extension.oid == SubjectKeyIdentifier.oid:
                        skid = extension.value.key_identifier
                        paa_by_skid[skid] = (Path(filename).name, paa_cert)
            except (ValueError, IOError) as e:
                logging.error(f"Failed to load {filename}: {str(e)}")
                if Path(filename).name not in ALLOWED_SKIPPED_FILENAMES:
                    logging.error(f"Re-raising error and failing: found new invalid PAA: {filename}")
                    raise

    return paa_by_skid


def extract_akid(cert: Certificate) -> Optional[bytes]:
    # Find the authority key identifier (if present)
    for extension in cert.extensions:
        if extension.oid == AuthorityKeyIdentifier.oid:
            return extension.value.key_identifier
    else:
        return None


class TC_DA_1_7(MatterBaseTest):
    ''' TC-DA-1.7

        This test requires two instances of the DUT with the same PID/VID to confirm that the individual
        devices are provisioned with different device attestation keys even in the same product line.

        In order to fully test this locally, it requires two separate instances of the example app
        with different keys, different discrimimators, different ports, and different KVSs.
        The example apps use the SDK authority key, so the script needs the allow_sdk_dac arg set.

        One suggested method of testing:

        Terminal 1: (first DUT - default port 5540)
        rm kvs1
        ${PATH_TO_ALL_CLUSTERS}/chip-all-clusters-app --dac_provider \
            credentials/development/commissioner_dut/struct_cd_authorized_paa_list_count1_valid/test_case_vector.json \
            --product-id 32768 --KVS kvs1 --discriminator 12

        Terminal 2: (second DUT - port 5541)
        rm kvs2
        ${PATH_TO_ALL_CLUSTERS}/chip-all-clusters-app --dac_provider \
            credentials/development/commissioner_dut/struct_cd_authorized_paa_list_count2_valid/test_case_vector.json \
            --product-id 32768 --KVS kvs2 --discriminator 34 --secured-device-port 5541

        Terminal 3: (test application)
        ./scripts/tests/run_python_test.py --script "src/python_testing/TC_DA_1_7.py" \
            --script-args "--storage-path admin_storage.json --commissioning-method on-network \
                --discriminator 12 34 --passcode 20202021 20202021 --bool-arg allow_sdk_dac:true"
    '''

    def setup_class(self):
        self.allow_sdk_dac = self.user_params.get("allow_sdk_dac", False)
        self.post_cert_test = self.user_params.get("post_cert_test", False)

    def expected_number_of_DUTs(self) -> int:
        return 1 if (self.allow_sdk_dac or self.post_cert_test) else 2

    def steps_one_dut(self, DUT: int) -> List[TestStep]:
        return [TestStep(f'{DUT}', f'Test DUT{DUT} DAC chain as follows:'),
                TestStep(f'{DUT}.1', f'TH sends CertificateChainRequest Command to DUT{DUT} with the CertificateType set to PAICertificate',
                         'Verify that the DUT returns a CertificateChainResponse. Save the returned Certificate as `pai_cert`.'),
                TestStep(f'{DUT}.2', 'TH sends CertificateChainRequest Command to DUT1 with the CertificateType set to DACCertificate',
                         'Verify that the DUT returns a CertificateChainResponse. Save the returned Certificate as `dac_cert`.'),
                TestStep(f'{DUT}.3', 'TH extracts the Authority Key Identifier from the PAI certificate',
                         ('* Verify that the Authority Key Identifier is signed by a PAA in the DCL. (Ensure that it is not SDK’s test PAAs)\n'
                          '* Verify that PAI authority key ID must not be one of:\n'
                          '  - 78: 5C: E7: 05: B8: 6B: 8F: 4E: 6F: C7: 93: AA: 60: CB: 43: EA: 69: 68:82: D5\n'
                          '  - 6A: FD: 22: 77: 1F: 51: 1F: EC: BF: 16: 41: 97: 67: 10: DC: DC: 31: A1: 71: 7E\n'
                          '* Save the selected PAA certificate as `paa_cert`\n')),
                TestStep(f'{DUT}.4', 'TH extracts ASN.1 DER bytes for the entire issuer field of `dac_cert` and subject field of `pai_cert`.',
                         'Verify that the `dac_cert` `issuer field is byte-forbyte equivalent to the `pai_cert`subject field.'),
                TestStep(f'{DUT}.5', 'TH extracts ASN.1 DER bytes for the entire issuer field of `pai_cert` and subject field of `paa_cert`.',
                         'Verify that the `pai_cert` issuer field is byte-forbyte equivalent to the `paa_cert` subject field.'),
                TestStep(f'{DUT}.6', f'TH extracts the public key from the DAC and saves as `pk_{DUT}`.')
                ]

    def steps_TC_DA_1_7(self):
        steps = [TestStep(0, "Commission DUT if not already done", is_commissioning=True)]
        steps += self.steps_one_dut(1)
        if self.expected_number_of_DUTs() == 2:
            steps += self.steps_one_dut(2)
        steps += [TestStep(3, "Verify that `pk_1` is not equal to `pk_2",
                           "Public keys do not match")]
        return steps

    @async_test_body
    async def test_TC_DA_1_7(self):
        num = 0
        if self.matter_test_config.discriminators:
            num += len(self.matter_test_config.discriminators)
        if self.matter_test_config.qr_code_content:
            num += len(self.matter_test_config.qr_code_content)
        if self.matter_test_config.manual_code:
            num += len(self.matter_test_config.manual_code)

        if num != self.expected_number_of_DUTs():
            if self.allow_sdk_dac:
                msg = "The allow_sdk_dac flag is only for use in CI. When using this test in CI, please specify a single discriminator, manual-code or qr-code-content"
            elif self.post_cert_test:
                msg = "The post_cert_test flag is only for use post-certification. When using this flag, please specify a single discriminator, manual-code or qr-code-content"
            else:
                msg = "This test requires two devices for use at certification. Please specify two device discriminators ex. --discriminator 1234 5678"
            asserts.fail(msg)

        pk = []
        # Commissioning - already done.
        self.step(0)

        for i in range(len(self.matter_test_config.dut_node_ids)):
            pk.append(await self.single_DUT(i+1, self.matter_test_config.dut_node_ids[i]))

        self.step(3)
        asserts.assert_equal(len(pk), len(set(pk)), "Found matching public keys in different DUTs")

    async def single_DUT(self, dut_index: int, dut_node_id: int) -> bytes:
        logging.info("Pre-condition: load all PAAs SKIDs")
        conf = self.matter_test_config
        paa_by_skid = load_all_paa(conf.paa_trust_store_path)
        logging.info("Found %d PAAs" % len(paa_by_skid))

        # Test plan step introducing test for each DUT
        self.step(f'{dut_index}')
        dev_ctrl = self.default_controller

        self.step(f'{dut_index}.1')
        result = await dev_ctrl.SendCommand(dut_node_id, 0,
                                            Clusters.OperationalCredentials.Commands.CertificateChainRequest(2))
        pai = result.certificate
        asserts.assert_less_equal(len(pai), 600, "PAI cert must be at most 600 bytes")
        key = 'pai_{}'.format(dut_index)
        self.record_data({key: hex_from_bytes(pai)})

        self.step(f'{dut_index}.2')
        result = await dev_ctrl.SendCommand(dut_node_id, 0,
                                            Clusters.OperationalCredentials.Commands.CertificateChainRequest(1))
        dac = result.certificate
        asserts.assert_less_equal(len(dac), 600, "DAC cert must be at most 600 bytes")
        key = 'dac_{}'.format(dut_index)
        self.record_data({key: hex_from_bytes(dac)})

        self.step(f'{dut_index}.3')
        logging.info("DUT {} Step 3 check 1: Ensure PAI's AKID matches a PAA and signature is valid".format(dut_index))
        pai_cert = load_der_x509_certificate(pai)
        pai_akid = extract_akid(pai_cert)
        if pai_akid not in paa_by_skid:
            asserts.fail("DUT %d PAI (%s) not matched in PAA trust store" % (dut_index, hex_from_bytes(pai_akid)))

        filename, paa_cert = paa_by_skid[pai_akid]
        logging.info("Matched PAA file %s, subject: %s" % (filename, paa_cert.subject))
        public_key = paa_cert.public_key()

        try:
            public_key.verify(signature=pai_cert.signature, data=pai_cert.tbs_certificate_bytes,
                              signature_algorithm=ec.ECDSA(hashes.SHA256()))
        except InvalidSignature as e:
            asserts.fail("DUT %d: Failed to verify PAI signature against PAA public key: %s" % (dut_index, str(e)))
        logging.info("Validated PAI signature against PAA")

        logging.info("DUT {} Step 3 check 2: Verify PAI AKID not in denylist of SDK PAIs".format(dut_index))
        if self.allow_sdk_dac:
            logging.warning("===> TEST STEP SKIPPED: Allowing SDK DACs!")
        else:
            for candidate in FORBIDDEN_AKID:
                asserts.assert_not_equal(hex_from_bytes(pai_akid), hex_from_bytes(candidate), "PAI AKID must not be in denylist")

        self.step(f'{dut_index}.4')
        # dac issuer == pai subject
        dac_cert = load_der_x509_certificate(dac)
        asserts.assert_equal(dac_cert.issuer, pai_cert.subject, "DAC issuer does not match PAI subject")

        self.step(f'{dut_index}.5')
        # pai issues == paa subject
        asserts.assert_equal(pai_cert.issuer, paa_cert.subject, "PAI issuer does not match PAA subject")

        self.step(f'{dut_index}.6')
        pk = dac_cert.public_key().public_bytes(encoding=Encoding.X962, format=PublicFormat.UncompressedPoint)
        logging.info("Subject public key pk: %s" % hex_from_bytes(pk))
        key = 'pk_{}'.format(dut_index)
        self.record_data({key: hex_from_bytes(pk)})
        return pk


if __name__ == "__main__":
    default_matter_test_main()
