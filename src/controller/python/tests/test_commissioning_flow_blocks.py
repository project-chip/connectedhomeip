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

    def test_paa_bound_uses_largest_advertised_profile(self):
        profiles = Clusters.OperationalCredentials.Enums.AttestationCryptoProfileEnum
        for bitmap, expected in ((1, profiles.kEcdsaMatterLegacy), (3, profiles.kMlDsa44),
                                 (5, profiles.kMlDsa65), (7, profiles.kMlDsa65), (9, None)):
            with self.subTest(bitmap=bitmap):
                opcreds = Clusters.OperationalCredentials(
                    featureMap=1,
                    PQCDeviceAttestationProfile=Clusters.OperationalCredentials.Structs.PQCDeviceAttestationProfileStruct(
                        PAASupportedProfiles=bitmap, PAISupportedProfiles=3, DACSupportedProfiles=1))
                selected = self.flow._select_attestation_certificate_request_profiles(opcreds)
                self.assertEqual(selected.paa, expected)


class TestCertificateChainDocumentLimit(unittest.IsolatedAsyncioTestCase):
    async def test_rejects_invalid_continuation_ids_before_another_request(self):
        # Require 1 first, then consecutive IDs: reject repeats, skips, and backwards IDs.
        for segment_ids in ((0,), (2,), (1, 1), (1, 3), (1, 2, 1)):
            with self.subTest(segment_ids=segment_ids):
                responses = [SimpleNamespace(certificate=b"x", totalDocumentSize=8, nextSegmentID=segment_id)
                             for segment_id in segment_ids]
                controller = SimpleNamespace(SendCommand=AsyncMock(side_effect=responses))
                flow = CommissioningFlowBlocks(controller, None, logging.getLogger(__name__))
                with patch("matter.commissioning.commissioning_flow_blocks.get_max_certificate_chain_document_size",
                           return_value=8), self.assertRaisesRegex(CommissionFailure, "invalid nextSegmentID progression"):
                    await flow._request_certificate_chain(1, 1, None)
                self.assertEqual(controller.SendCommand.await_count, len(responses))

    async def test_assembles_consecutive_segments(self):
        responses = [SimpleNamespace(certificate=certificate, totalDocumentSize=6, nextSegmentID=segment_id)
                     for certificate, segment_id in ((b"ab", 1), (b"cd", 2), (b"ef", None))]
        controller = SimpleNamespace(SendCommand=AsyncMock(side_effect=responses))
        flow = CommissioningFlowBlocks(controller, None, logging.getLogger(__name__))
        with patch("matter.commissioning.commissioning_flow_blocks.get_max_certificate_chain_document_size", return_value=8):
            self.assertEqual(await flow._request_certificate_chain(1, 1, None), b"abcdef")
        self.assertEqual([call.args[2].segmentID for call in controller.SendCommand.await_args_list], [None, 1, 2])

    async def test_rejects_empty_segments_without_requesting_another_segment(self):
        for continuation in (False, True):
            for next_segment_id in (None, 2):
                with self.subTest(continuation=continuation, next_segment_id=next_segment_id):
                    responses = []
                    if continuation:
                        responses.append(SimpleNamespace(certificate=b"x", totalDocumentSize=2, nextSegmentID=1))
                    responses.append(SimpleNamespace(certificate=b"", totalDocumentSize=2, nextSegmentID=next_segment_id))
                    # A finite response list also prevents a regression from hanging the test.
                    controller = SimpleNamespace(SendCommand=AsyncMock(side_effect=responses))
                    flow = CommissioningFlowBlocks(controller, None, logging.getLogger(__name__))
                    # Empty segments cannot advance reassembly, including an empty final segment.
                    with patch("matter.commissioning.commissioning_flow_blocks.get_max_certificate_chain_document_size",
                               return_value=8), self.assertRaisesRegex(CommissionFailure, "empty certificate segment"):
                        await flow._request_certificate_chain(1, 1, None)
                    self.assertEqual(controller.SendCommand.await_count, len(responses))

    async def test_rejects_empty_single_response(self):
        controller = SimpleNamespace(SendCommand=AsyncMock(return_value=SimpleNamespace(
            certificate=b"", totalDocumentSize=None, nextSegmentID=None)))
        flow = CommissioningFlowBlocks(controller, None, logging.getLogger(__name__))
        with patch("matter.commissioning.commissioning_flow_blocks.get_max_certificate_chain_document_size",
                   return_value=8), self.assertRaisesRegex(CommissionFailure, "empty certificate segment"):
            await flow._request_certificate_chain(1, 1, None)
        controller.SendCommand.assert_awaited_once()

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
                        native_limit.assert_called_once_with(None, None)
                    controller.SendCommand.assert_awaited_once()

    async def test_rejects_advertised_total_before_requesting_another_segment(self):
        profiles = Clusters.OperationalCredentials.Enums.AttestationCryptoProfileEnum
        controller = SimpleNamespace(SendCommand=AsyncMock(return_value=SimpleNamespace(
            certificate=b"x", totalDocumentSize=4733, nextSegmentID=1)))
        flow = CommissioningFlowBlocks(controller, None, logging.getLogger(__name__))
        with patch("matter.commissioning.commissioning_flow_blocks.get_max_certificate_chain_document_size",
                   return_value=4732) as native_limit:
            with self.assertRaisesRegex(CommissionFailure, "document size"):
                await flow._request_certificate_chain(1, 1, profiles.kEcdsaMatterLegacy, profiles.kMlDsa44)
            native_limit.assert_called_once_with(profiles.kEcdsaMatterLegacy, profiles.kMlDsa44)
        controller.SendCommand.assert_awaited_once()


if __name__ == "__main__":
    unittest.main()
