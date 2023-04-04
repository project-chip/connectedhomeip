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

import logging
from glob import glob
from pathlib import Path
from typing import Optional

import chip.clusters as Clusters
from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat
from cryptography.x509 import AuthorityKeyIdentifier, Certificate, SubjectKeyIdentifier, load_der_x509_certificate
from matter_testing_support import MatterBaseTest, async_test_body, bytes_from_hex, default_matter_test_main, hex_from_bytes
from mobly import asserts

FORBIDDEN_AKID = [
    bytes_from_hex("78:5C:E7:05:B8:6B:8F:4E:6F:C7:93:AA:60:CB:43:EA:69:68:82:D5"),
    bytes_from_hex("6A:FD:22:77:1F:51:1F:EC:BF:16:41:97:67:10:DC:DC:31:A1:71:7E")
]


def load_all_paa(paa_path: Path) -> dict:
    logging.info("Loading all PAAs in %s" % paa_path)

    paa_by_skid = {}
    for filename in glob(str(paa_path.joinpath("*.der"))):
        with open(filename, "rb") as derfile:
            # Load cert
            paa_der = derfile.read()
            paa_cert = load_der_x509_certificate(paa_der)

            # Find the subject key identifier (if present), and record it
            for extension in paa_cert.extensions:
                if extension.oid == SubjectKeyIdentifier.oid:
                    skid = extension.value.key_identifier
                    paa_by_skid[skid] = (Path(filename).name, paa_cert)

    return paa_by_skid


def extract_akid(cert: Certificate) -> Optional[bytes]:
    # Find the authority key identifier (if present)
    for extension in cert.extensions:
        if extension.oid == AuthorityKeyIdentifier.oid:
            return extension.value.key_identifier
    else:
        return None


class TC_DA_1_7(MatterBaseTest):
    @async_test_body
    async def test_TC_DA_1_7(self):
        # For real tests, we require more than one DUT
        # On the CI, this doesn't make sense to do since all the examples use the same DAC
        # To specify more than 1 DUT, use a list of discriminators and passcodes
        allow_sdk_dac = self.user_params.get("allow_sdk_dac", False)
        if allow_sdk_dac:
            asserts.assert_equal(len(self.matter_test_config.discriminator), 1, "Only one device can be tested with SDK DAC")
        if not allow_sdk_dac:
            asserts.assert_equal(len(self.matter_test_config.discriminator), 2, "This test requires 2 DUTs")
        pk = []
        for i in range(len(self.matter_test_config.dut_node_id)):
            pk.append(await self.single_DUT(i, self.matter_test_config.dut_node_id[i]))

        asserts.assert_equal(len(pk), len(set(pk)), "Found matching public keys in different DUTs")

    async def single_DUT(self, dut_index: int, dut_node_id: int) -> bytes:
        # Option to allow SDK roots (skip step 4 check 2)
        allow_sdk_dac = self.user_params.get("allow_sdk_dac", False)

        logging.info("Pre-condition: load all PAAs SKIDs")
        conf = self.matter_test_config
        paa_by_skid = load_all_paa(conf.paa_trust_store_path)
        logging.info("Found %d PAAs" % len(paa_by_skid))

        logging.info("DUT {} Step 1: Commissioning, already done".format(dut_index))
        dev_ctrl = self.default_controller

        logging.info("DUT {} Step 2: Get PAI of DUT1 with certificate chain request".format(dut_index))
        result = await dev_ctrl.SendCommand(dut_node_id, 0,
                                            Clusters.OperationalCredentials.Commands.CertificateChainRequest(2))
        pai = result.certificate
        asserts.assert_less_equal(len(pai), 600, "PAI cert must be at most 600 bytes")
        key = 'pai_{}'.format(dut_index)
        self.record_data({key: hex_from_bytes(pai)})

        logging.info("DUT {} Step 3: Get DAC of DUT1 with certificate chain request".format(dut_index))
        result = await dev_ctrl.SendCommand(dut_node_id, 0,
                                            Clusters.OperationalCredentials.Commands.CertificateChainRequest(1))
        dac = result.certificate
        asserts.assert_less_equal(len(dac), 600, "DAC cert must be at most 600 bytes")
        key = 'dac_{}'.format(dut_index)
        self.record_data({key: hex_from_bytes(dac)})

        logging.info("DUT {} Step 4 check 1: Ensure PAI's AKID matches a PAA and signature is valid".format(dut_index))
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

        logging.info("DUT {} Step 4 check 2: Verify PAI AKID not in denylist of SDK PAIs".format(dut_index))
        if allow_sdk_dac:
            logging.warn("===> TEST STEP SKIPPED: Allowing SDK DACs!")
        else:
            for candidate in FORBIDDEN_AKID:
                asserts.assert_not_equal(hex_from_bytes(pai_akid), hex_from_bytes(candidate), "PAI AKID must not be in denylist")

        logging.info("DUT {} Step 5: Extract subject public key of DAC and save".format(dut_index))
        dac_cert = load_der_x509_certificate(dac)
        pk = dac_cert.public_key().public_bytes(encoding=Encoding.X962, format=PublicFormat.UncompressedPoint)
        logging.info("Subject public key pk: %s" % hex_from_bytes(pk))
        key = 'pk_{}'.format(dut_index)
        self.record_data({key: hex_from_bytes(pk)})
        return pk


if __name__ == "__main__":
    default_matter_test_main()
