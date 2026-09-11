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

"""Exercise profile negotiation and certificate retrieval with real certificate vectors."""

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
from TC_DA_1_12 import TC_DA_1_12  # noqa: E402
from test_pqc_support import _load_pem_fixtures  # noqa: E402


class _ProfileRetrievalComplete(Exception):
    """Stop before legacy retrieval and attestation, which integration tests exercise."""


@unittest.skipUnless(is_ml_dsa_supported(), "cryptography ML-DSA support is required")
class TestTCDA112Profiles(unittest.IsolatedAsyncioTestCase):
    async def _retrieve(self, pai_profile, pai_name, dac_name):
        fixtures = _load_pem_fixtures()
        opcreds = Clusters.OperationalCredentials
        cert_type = opcreds.Enums.CertificateChainTypeEnum
        requests = []

        async def read_attribute(*, cluster, attribute, endpoint):
            self.assertEqual(endpoint, 0)
            self.assertEqual(cluster, opcreds)
            if attribute == opcreds.Attributes.FeatureMap:
                return OperationalCredentialsFeature.kPQCDeviceAttestation
            self.assertEqual(attribute, opcreds.Attributes.PQCDeviceAttestationProfile)
            return opcreds.Structs.PQCDeviceAttestationProfileStruct(
                PAASupportedProfiles=profile_mask(AttestationCryptoProfile.kMlDsa65),
                PAISupportedProfiles=profile_mask(pai_profile),
                DACSupportedProfiles=profile_mask(AttestationCryptoProfile.kEcdsaMatterLegacy))

        async def send_command(*, cmd, endpoint):
            self.assertEqual(endpoint, 0)
            requests.append(cmd)
            expected = (pai_profile if cmd.certificateType == cert_type.kPAICertificate
                        else AttestationCryptoProfile.kEcdsaMatterLegacy)
            self.assertEqual(cmd.cryptoProfile, expected)
            document = fixtures[pai_name if cmd.certificateType == cert_type.kPAICertificate else dac_name]
            offset = cmd.segmentID * kCertificateSegmentSize
            end = offset + kCertificateSegmentSize
            return opcreds.Commands.CertificateChainResponse(
                certificate=document[offset:end], totalDocumentSize=len(document),
                nextSegmentID=cmd.segmentID + 1 if end < len(document) else None)

        def step(number):
            if number == 7:
                raise _ProfileRetrievalComplete

        with tempfile.TemporaryDirectory() as directory:
            Path(directory, 'paa.der').write_bytes(fixtures['kMlDsa65PaaPem'])
            test_case = SimpleNamespace(
                matter_test_config=SimpleNamespace(paa_trust_store_path=Path(directory)),
                user_params={}, step=step, read_single_attribute_check_success=read_attribute,
                send_single_cmd=send_command)
            test_case._send_certificate_chain_request = MethodType(TC_DA_1_12._send_certificate_chain_request, test_case)
            with self.assertRaises(_ProfileRetrievalComplete):
                await TC_DA_1_12.test_TC_DA_1_12.__wrapped__(test_case)
        self.assertEqual({request.certificateType for request in requests},
                         {cert_type.kPAICertificate, cert_type.kDACCertificate})

    async def test_ecdsa_dac_under_ml_dsa_65_pai(self):
        await self._retrieve(AttestationCryptoProfile.kMlDsa65, 'kMlDsa65PaiPem', 'kMlDsa65PaiDacPem')

    async def test_independent_paa_and_pai_profiles(self):
        await self._retrieve(AttestationCryptoProfile.kMlDsa44,
                             'kMlDsa44PaiUnderMlDsa65PaaPem', 'kMlDsa44PaiUnderMlDsa65PaaDacPem')

    async def test_ecdsa_pai_under_pqc_paa(self):
        await self._retrieve(AttestationCryptoProfile.kEcdsaMatterLegacy,
                             'kP256PaiUnderMlDsa65PaaPem', 'kP256PaiUnderMlDsa65PaaDacPem')

    async def test_rejects_pai_key_mismatching_selected_profile(self):
        # Advertising ML-DSA-44 while serving an ML-DSA-65 PAI violates the requested key profile.
        with self.assertRaisesRegex(signals.TestFailure, 'public key algorithm must match selectedPAIProfile'):
            await self._retrieve(AttestationCryptoProfile.kMlDsa44, 'kMlDsa65PaiPem', 'kMlDsa65PaiDacPem')


if __name__ == '__main__':
    unittest.main()
