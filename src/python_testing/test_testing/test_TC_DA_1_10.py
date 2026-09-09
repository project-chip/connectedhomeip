#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2026 Project CHIP Authors
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

"""Exercise DA-1.10 against certificate vectors, including invalid DUT responses."""

import json
import sys
import tempfile
import unittest
from pathlib import Path
from types import MethodType, SimpleNamespace

from mobly import signals

import matter.clusters as Clusters

_CHIP_ROOT = Path(__file__).resolve().parents[3]
sys.path.append(str(_CHIP_ROOT / "src/python_testing"))

from support_modules.pqc_support import (AttestationCryptoProfile, OperationalCredentialsFeature, is_ml_dsa_supported,  # noqa: E402
                                         kCertificateSegmentSize, profile_mask)
from TC_DA_1_10 import TC_DA_1_10  # noqa: E402
from test_pqc_support import _load_pem_fixtures  # noqa: E402


@unittest.skipUnless(is_ml_dsa_supported(), "cryptography ML-DSA support is required")
class TestTCDA110(unittest.IsolatedAsyncioTestCase):
    def setUp(self):
        self.fixtures = _load_pem_fixtures()
        vector = json.loads((_CHIP_ROOT / 'credentials/development/commissioner_dut/'
                             'struct_dac_cert_version_v3/test_case_vector.json').read_text())
        self.legacy_pai = bytes.fromhex(vector['pai_cert'])
        self.legacy_dac = bytes.fromhex(vector['dac_cert'])
        self.paa_profile = AttestationCryptoProfile.kMlDsa65
        self.pai_profile = AttestationCryptoProfile.kMlDsa65
        self.paa = self.fixtures['kMlDsa65PaaPem']
        self.pai = self.fixtures['kMlDsa65PaiPem']
        self.dac = self.fixtures['kMlDsa65PaiDacPem']
        self.feature_map = OperationalCredentialsFeature.kPQCDeviceAttestation
        self.dac_profiles = profile_mask(AttestationCryptoProfile.kEcdsaMatterLegacy)
        self.requests = []
        self.steps = []

    async def _run_test(self, alter_response=None):
        opcreds = Clusters.OperationalCredentials
        cert_type = opcreds.Enums.CertificateChainTypeEnum

        async def read_attribute(*, cluster, attribute, endpoint):
            self.assertEqual((cluster, endpoint), (opcreds, 0))
            if attribute == opcreds.Attributes.FeatureMap:
                return self.feature_map
            self.assertEqual(attribute, opcreds.Attributes.PQCDeviceAttestationProfile)
            return opcreds.Structs.PQCDeviceAttestationProfileStruct(
                PAASupportedProfiles=profile_mask(self.paa_profile) | profile_mask(AttestationCryptoProfile.kEcdsaMatterLegacy),
                PAISupportedProfiles=profile_mask(self.pai_profile) | profile_mask(AttestationCryptoProfile.kEcdsaMatterLegacy),
                DACSupportedProfiles=self.dac_profiles)

        async def send_command(*, cmd, endpoint):
            self.assertEqual(endpoint, 0)
            self.assertIsInstance(cmd, opcreds.Commands.CertificateChainRequest)
            self.requests.append(cmd)
            self.assertIn(cmd.certificateType, (cert_type.kPAICertificate, cert_type.kDACCertificate))
            is_pai = cmd.certificateType == cert_type.kPAICertificate
            if cmd.cryptoProfile is None:
                # Legacy requests must omit all three optional selection/segmentation fields.
                self.assertIsNone(cmd.segmentID)
                self.assertIsNone(cmd.maxSegmentSize)
                response = opcreds.Commands.CertificateChainResponse(
                    certificate=self.legacy_pai if is_pai else self.legacy_dac)
            else:
                expected = self.pai_profile if is_pai else AttestationCryptoProfile.kEcdsaMatterLegacy
                self.assertEqual(cmd.cryptoProfile, expected)
                self.assertEqual(cmd.maxSegmentSize, kCertificateSegmentSize)
                document = self.pai if is_pai else self.dac
                offset = cmd.segmentID * kCertificateSegmentSize
                end = offset + kCertificateSegmentSize
                response = opcreds.Commands.CertificateChainResponse(
                    certificate=document[offset:end], totalDocumentSize=len(document),
                    nextSegmentID=cmd.segmentID + 1 if end < len(document) else None)
            return alter_response(cmd, response) if alter_response else response

        with tempfile.TemporaryDirectory() as directory:
            Path(directory, 'pqc.der').write_bytes(self.paa)
            Path(directory, 'legacy.der').write_bytes(
                (_CHIP_ROOT / 'credentials/development/paa-root-certs/Chip-Test-PAA-FFF1-Cert.der').read_bytes())
            test_case = SimpleNamespace(
                matter_test_config=SimpleNamespace(paa_trust_store_path=Path(directory)),
                step=self.steps.append, read_single_attribute_check_success=read_attribute, send_single_cmd=send_command)
            for name in ('_send_certificate_chain_request', '_retrieve_legacy_certificate'):
                setattr(test_case, name, MethodType(getattr(TC_DA_1_10, name), test_case))
            await TC_DA_1_10.test_TC_DA_1_10.__wrapped__(test_case)
        self.assertEqual(self.steps, list(range(13)))
        self.assertEqual([cmd.certificateType for cmd in self.requests if cmd.cryptoProfile is None],
                         [cert_type.kPAICertificate, cert_type.kDACCertificate])

    async def test_validates_pqc_and_legacy_chains(self):
        await self._run_test()

    async def test_selects_ml_dsa_44_when_65_is_absent(self):
        self.paa_profile = self.pai_profile = AttestationCryptoProfile.kMlDsa44
        self.paa = self.fixtures['kMlDsa44PaaPem']
        self.pai = self.fixtures['kMlDsa44PaiPem']
        self.dac = self.fixtures['kMlDsa44PaiDacPem']
        await self._run_test()

    async def test_independent_paa_and_pai_profiles(self):
        self.pai_profile = AttestationCryptoProfile.kMlDsa44
        self.pai = self.fixtures['kMlDsa44PaiUnderMlDsa65PaaPem']
        self.dac = self.fixtures['kMlDsa44PaiUnderMlDsa65PaaDacPem']
        await self._run_test()

    async def test_ecdsa_pai_under_pqc_paa(self):
        self.pai_profile = AttestationCryptoProfile.kEcdsaMatterLegacy
        self.pai = self.fixtures['kP256PaiUnderMlDsa65PaaPem']
        self.dac = self.fixtures['kP256PaiUnderMlDsa65PaaDacPem']
        await self._run_test()

    async def test_rejects_missing_pqc_feature(self):
        # DA-1.10 requires the DUT to advertise PQCDA before requesting PQC credentials.
        self.feature_map = 0
        with self.assertRaisesRegex(signals.TestFailure, 'does not have PQCDA set'):
            await self._run_test()
        self.assertEqual(self.requests, [])

    async def test_rejects_pai_key_mismatching_selected_profile(self):
        # Advertising ML-DSA-44 while returning an ML-DSA-65 subject key is invalid.
        self.pai_profile = AttestationCryptoProfile.kMlDsa44
        with self.assertRaisesRegex(signals.TestFailure, 'public key algorithm must match selectedPAIProfile'):
            await self._run_test()

    async def test_rejects_missing_legacy_dac_profile(self):
        # Phase 1 requires the DAC bitmap to advertise its P-256 subject key.
        self.dac_profiles = profile_mask(AttestationCryptoProfile.kMlDsa65)
        with self.assertRaisesRegex(signals.TestFailure, 'DAC'):
            await self._run_test()
        self.assertEqual(self.steps[-1], 6)

    async def test_rejects_dac_signature_mismatching_pai(self):
        # A DAC signed by ML-DSA-44 cannot belong to the selected ML-DSA-65 PAI chain.
        self.dac = self.fixtures['kMlDsa44PaiDacPem']
        with self.assertRaisesRegex(signals.TestFailure, 'signature algorithm must match selectedPAIProfile'):
            await self._run_test()

    async def test_rejects_invalid_response_type(self):
        # A successful command must return CertificateChainResponse, not another payload type.
        with self.assertRaisesRegex(signals.TestFailure, 'invalid response to CertificateChainRequest'):
            await self._run_test(lambda cmd, response: None)

    async def test_rejects_segmented_legacy_certificate(self):
        # Legacy requests must return the entire certificate with no NextSegmentID.
        def segment_legacy(cmd, response):
            if cmd.cryptoProfile is None:
                response.nextSegmentID = 1
            return response

        with self.assertRaisesRegex(signals.TestFailure, 'NextSegmentID for the legacy'):
            await self._run_test(segment_legacy)

    async def test_rejects_empty_legacy_certificate(self):
        # An empty legacy certificate cannot satisfy the required fallback chain retrieval.
        def empty_legacy(cmd, response):
            if cmd.cryptoProfile is None:
                response.certificate = b''
            return response

        with self.assertRaisesRegex(signals.TestFailure, 'empty legacy'):
            await self._run_test(empty_legacy)

    async def test_rejects_corrupted_dac_signature(self):
        # Preserve valid DER and algorithm identifiers but damage the signature bytes.
        self.dac = self.dac[:-1] + bytes([self.dac[-1] ^ 1])
        with self.assertRaisesRegex(signals.TestFailure, 'signature'):
            await self._run_test()
        self.assertEqual(self.steps[-1], 11)

    async def test_rejects_corrupted_legacy_dac_signature(self):
        # Step 12 must cryptographically validate the fallback chain as well as the PQC chain.
        self.legacy_dac = self.legacy_dac[:-1] + bytes([self.legacy_dac[-1] ^ 1])
        with self.assertRaisesRegex(signals.TestFailure, 'signature'):
            await self._run_test()
        self.assertEqual(self.steps[-1], 12)


if __name__ == '__main__':
    unittest.main()
