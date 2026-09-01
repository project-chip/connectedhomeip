import logging
import unittest

import matter.clusters as Clusters
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


if __name__ == "__main__":
    unittest.main()
