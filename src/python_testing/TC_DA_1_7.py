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

from matter_testing_support import MatterBaseTest, default_matter_test_main, async_test_body
from matter_testing_support import hex_from_bytes, bytes_from_hex
from chip.interaction_model import Status
import chip.clusters as Clusters
import logging
from mobly import asserts
from pathlib import Path
from glob import glob
from cryptography.x509 import load_der_x509_certificate, SubjectKeyIdentifier, AuthorityKeyIdentifier, Certificate
from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.serialization import PublicFormat, Encoding
from typing import Optional

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
        # Option to allow SDK roots (skip step 4 check 2)
        allow_sdk_dac = self.user_params.get("allow_sdk_dac", False)

        logging.info("Pre-condition: load all PAAs SKIDs")
        conf = self.matter_test_config
        paa_by_skid = load_all_paa(conf.paa_trust_store_path)
        logging.info("Found %d PAAs" % len(paa_by_skid))

        logging.info("Step 1: Commissioning, already done")
        dev_ctrl = self.default_controller

        logging.info("Step 2: Get PAI of DUT1 with certificate chain request")
        result = await dev_ctrl.SendCommand(self.dut_node_id, 0, Clusters.OperationalCredentials.Commands.CertificateChainRequest(2))
        pai_1 = result.certificate
        asserts.assert_less_equal(len(pai_1), 600, "PAI cert must be at most 600 bytes")
        self.record_data({"pai_1": hex_from_bytes(pai_1)})

        logging.info("Step 3: Get DAC of DUT1 with certificate chain request")
        result = await dev_ctrl.SendCommand(self.dut_node_id, 0, Clusters.OperationalCredentials.Commands.CertificateChainRequest(1))
        dac_1 = result.certificate
        asserts.assert_less_equal(len(dac_1), 600, "DAC cert must be at most 600 bytes")
        self.record_data({"dac_1": hex_from_bytes(dac_1)})

        logging.info("Step 4 check 1: Ensure PAI's AKID matches a PAA and signature is valid")
        pai1_cert = load_der_x509_certificate(pai_1)
        pai1_akid = extract_akid(pai1_cert)
        if pai1_akid not in paa_by_skid:
            asserts.fail("DUT1's PAI (%s) not matched in PAA trust store" % hex_from_bytes(pai1_akid))

        filename, paa_cert = paa_by_skid[pai1_akid]
        logging.info("Matched PAA file %s, subject: %s" % (filename, paa_cert.subject))
        public_key = paa_cert.public_key()

        try:
            public_key.verify(signature=pai1_cert.signature, data=pai1_cert.tbs_certificate_bytes,
                              signature_algorithm=ec.ECDSA(hashes.SHA256()))
        except InvalidSignature as e:
            asserts.fail("Failed to verify PAI signature against PAA public key: %s" % str(e))
        logging.info("Validated PAI signature against PAA")

        logging.info("Step 4 check 2: Verify PAI AKID not in denylist of SDK PAIs")
        if allow_sdk_dac:
            logging.warn("===> TEST STEP SKIPPED: Allowing SDK DACs!")
        else:
            for candidate in FORBIDDEN_AKID:
                asserts.assert_not_equal(hex_from_bytes(pai1_akid), hex_from_bytes(candidate), "PAI AKID must not be in denylist")

        logging.info("Step 5: Extract subject public key of DAC and save")
        dac1_cert = load_der_x509_certificate(dac_1)
        pk_1 = dac1_cert.public_key().public_bytes(encoding=Encoding.X962, format=PublicFormat.UncompressedPoint)
        logging.info("Subject public key pk_1: %s" % hex_from_bytes(pk_1))
        self.record_data({"pk_1": hex_from_bytes(pk_1)})


if __name__ == "__main__":
    default_matter_test_main()
