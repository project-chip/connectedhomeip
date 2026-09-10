import logging
import unittest
from types import SimpleNamespace
from unittest.mock import AsyncMock, patch

import matter.clusters as Clusters
from matter.commissioning import CommissionFailure
from matter.commissioning.commissioning_flow_blocks import CommissioningFlowBlocks

_FEATURE_PQC_DEVICE_ATTESTATION = 0x1
_PROFILE_ECDSA_MATTER_LEGACY = 0x1
_PROFILE_ML_DSA_44 = 0x2
_PROFILE_ML_DSA_65 = 0x4


class TestCommissioningFlowBlocks(unittest.TestCase):
    def setUp(self):
        self.flow = CommissioningFlowBlocks(None, None, logging.getLogger(__name__))

    def test_selects_pai_and_dac_profiles_independently(self):
        profile_support = Clusters.OperationalCredentials.Structs.PQCDeviceAttestationProfileStruct(
            PAASupportedProfiles=_PROFILE_ECDSA_MATTER_LEGACY | _PROFILE_ML_DSA_65,
            PAISupportedProfiles=_PROFILE_ECDSA_MATTER_LEGACY | _PROFILE_ML_DSA_65,
            DACSupportedProfiles=_PROFILE_ECDSA_MATTER_LEGACY | _PROFILE_ML_DSA_44,
        )
        opcreds = Clusters.OperationalCredentials(
            featureMap=_FEATURE_PQC_DEVICE_ATTESTATION,
            PQCDeviceAttestationProfile=profile_support,
        )

        selected = self.flow._select_attestation_certificate_request_profiles(opcreds)

        self.assertEqual(
            selected.pai,
            Clusters.OperationalCredentials.Enums.AttestationCryptoProfileEnum.kMlDsa65,
        )
        self.assertEqual(
            selected.dac,
            Clusters.OperationalCredentials.Enums.AttestationCryptoProfileEnum.kMlDsa44,
        )

    def test_falls_back_to_parameterless_legacy_when_dac_does_not_advertise_legacy(self):
        profile_support = Clusters.OperationalCredentials.Structs.PQCDeviceAttestationProfileStruct(
            PAASupportedProfiles=_PROFILE_ECDSA_MATTER_LEGACY | _PROFILE_ML_DSA_44,
            PAISupportedProfiles=_PROFILE_ECDSA_MATTER_LEGACY | _PROFILE_ML_DSA_44,
            DACSupportedProfiles=_PROFILE_ML_DSA_44,
        )
        opcreds = Clusters.OperationalCredentials(
            featureMap=_FEATURE_PQC_DEVICE_ATTESTATION,
            PQCDeviceAttestationProfile=profile_support,
        )

        selected = self.flow._select_attestation_certificate_request_profiles(opcreds)

        self.assertIsNone(selected.pai)
        self.assertIsNone(selected.dac)

    def test_falls_back_to_parameterless_legacy_when_no_pqc_issuer_is_advertised(self):
        profile_support = Clusters.OperationalCredentials.Structs.PQCDeviceAttestationProfileStruct(
            PAASupportedProfiles=_PROFILE_ECDSA_MATTER_LEGACY,
            PAISupportedProfiles=_PROFILE_ECDSA_MATTER_LEGACY,
            DACSupportedProfiles=_PROFILE_ECDSA_MATTER_LEGACY,
        )
        opcreds = Clusters.OperationalCredentials(
            featureMap=_FEATURE_PQC_DEVICE_ATTESTATION,
            PQCDeviceAttestationProfile=profile_support,
        )

        selected = self.flow._select_attestation_certificate_request_profiles(opcreds)

        self.assertIsNone(selected.pai)
        self.assertIsNone(selected.dac)


class TestCertificateChainDocumentLimit(unittest.IsolatedAsyncioTestCase):
    async def test_uses_native_limit_for_single_and_segmented_responses(self):
        # A small substituted native limit makes boundary coverage independent of its current value.
        for segmented in (False, True):
            for size in (8, 9):
                with self.subTest(segmented=segmented, size=size):
                    certificate = b"x" * size
                    response = SimpleNamespace(certificate=certificate,
                                               totalDocumentSize=size if segmented else None, nextSegmentID=None)
                    controller = SimpleNamespace(SendCommand=AsyncMock(return_value=response))
                    flow = CommissioningFlowBlocks(controller, None, logging.getLogger(__name__))
                    with patch("matter.commissioning.commissioning_flow_blocks.get_max_certificate_chain_document_size",
                               return_value=8) as native_limit:
                        if size == 8:
                            self.assertEqual(await flow._request_certificate_chain(1, 1, None), certificate)
                        else:
                            # Both response forms must reject a document larger than the native SDK limit.
                            with self.assertRaisesRegex(CommissionFailure, "document size"):
                                await flow._request_certificate_chain(1, 1, None)
                        native_limit.assert_called_once_with()
                    controller.SendCommand.assert_awaited_once()


if __name__ == "__main__":
    unittest.main()
