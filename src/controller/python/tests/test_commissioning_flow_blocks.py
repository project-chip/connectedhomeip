import logging
import unittest
from types import SimpleNamespace
from unittest.mock import AsyncMock, patch

import matter.clusters as Clusters
from matter.commissioning import CommissionFailure
from matter.commissioning.commissioning_flow_blocks import CommissioningFlowBlocks
from matter.exceptions import ChipStackError
from matter.interaction_model import InteractionModelError, Status

_FEATURE_PQC_DEVICE_ATTESTATION = 0x1
_PROFILE_ECDSA_MATTER_LEGACY = 0x1
_PROFILE_ML_DSA_44 = 0x2
_PROFILE_ML_DSA_65 = 0x4


class TestCommissioningFlowBlocks(unittest.TestCase):
    def setUp(self):
        self.flow = CommissioningFlowBlocks(None, None, logging.getLogger(__name__))

    def test_selects_strongest_mutual_chain_from_paa_downward(self):
        opcreds = Clusters.OperationalCredentials
        cases = (
            (7, 7, 1, 7, (2, 2, 0)),
            (7, 3, 1, 7, (2, 1, 0)),
            (7, 1, 1, 7, (2, 0, 0)),
            (3, 7, 1, 7, (1, 1, 0)),
            (3, 5, 1, 7, (1, 0, 0)),
            (7, 7, 1, 3, (1, 1, 0)),
            (7, 3, 1, 5, (2, 0, 0)),
            (7, 7, 1, 1, (0, 0, 0)),
            (9, 3, 1, 7, (0, 0, 0)),
            (7, 7, 7, 7, (2, 2, 0)),
            (7, 7, 2, 7, None),  # Phase 1 requires the legacy DAC profile.
            (1, 1, 1, 7, None),  # No PQC issuer is advertised.
            (7, 7, 1, 0, None),  # No locally supported chain.
        )
        for paa, pai, dac, local, expected in cases:
            with self.subTest(paa=paa, pai=pai, dac=dac, local=local):
                device = opcreds(
                    featureMap=_FEATURE_PQC_DEVICE_ATTESTATION,
                    PQCDeviceAttestationProfile=opcreds.Structs.PQCDeviceAttestationProfileStruct(
                        PAASupportedProfiles=paa, PAISupportedProfiles=pai, DACSupportedProfiles=dac))
                with patch("matter.commissioning.commissioning_flow_blocks.get_supported_attestation_profiles", return_value=local):
                    selected = self.flow._select_attestation_certificate_request_profiles(device)
                if expected is None:
                    self.assertIsNone(selected)
                else:
                    self.assertEqual((selected.PAACrypto, selected.PAICrypto, selected.DACCrypto), expected)

    def test_legacy_device_omits_chain_selection(self):
        opcreds = Clusters.OperationalCredentials
        for device in (opcreds(featureMap=0), opcreds(featureMap=1, PQCDeviceAttestationProfile=None)):
            with self.subTest(device=device):
                self.assertIsNone(self.flow._select_attestation_certificate_request_profiles(device))


class TestAttestationChainFallback(unittest.IsolatedAsyncioTestCase):
    def setUp(self):
        self.opcreds = Clusters.OperationalCredentials
        self.types = self.opcreds.Enums.CertificateChainTypeEnum
        self.device = self.opcreds(
            featureMap=1,
            PQCDeviceAttestationProfile=self.opcreds.Structs.PQCDeviceAttestationProfileStruct(
                PAASupportedProfiles=7, PAISupportedProfiles=3, DACSupportedProfiles=1))
        self.controller = SimpleNamespace(SendCommand=AsyncMock())
        self.flow = CommissioningFlowBlocks(self.controller, None, logging.getLogger(__name__))
        for name, value in (("get_supported_attestation_profiles", 7),
                            ("get_max_certificate_chain_document_size", 8192),
                            ("validate_attestation_certificate_profile", None)):
            patcher = patch(f"matter.commissioning.commissioning_flow_blocks.{name}", return_value=value)
            mock = patcher.start()
            self.addCleanup(patcher.stop)
            if name == "validate_attestation_certificate_profile":
                self.validate = mock

    def requests(self):
        return [call.args[2] for call in self.controller.SendCommand.await_args_list]

    async def test_retries_pai_before_lowering_paa_and_keeps_chain_for_dac(self):
        self.controller.SendCommand.side_effect = [
            InteractionModelError(Status.InvalidCommand),
            self.opcreds.Commands.CertificateChainResponse(certificate=b"pai"),
            self.opcreds.Commands.CertificateChainResponse(certificate=b"dac"),
        ]
        self.assertEqual(await self.flow._request_attestation_certificates(1, self.device), (b"pai", b"dac"))
        requests = self.requests()
        self.assertEqual([request.certificateType for request in requests],
                         [self.types.kPAICertificate, self.types.kPAICertificate, self.types.kDACCertificate])
        self.assertEqual([(request.cryptoProfile.PAACrypto, request.cryptoProfile.PAICrypto,
                           request.cryptoProfile.DACCrypto) for request in requests], [(2, 1, 0), (2, 0, 0), (2, 0, 0)])
        self.validate.assert_any_call(b"pai", 0, 2)
        self.validate.assert_any_call(b"dac", 0, 0)

    async def test_exhausts_candidates_once_in_paa_first_order(self):
        self.device.PQCDeviceAttestationProfile.PAISupportedProfiles = 7
        self.controller.SendCommand.side_effect = InteractionModelError(Status.InvalidCommand)
        with self.assertRaises(InteractionModelError):
            await self.flow._request_attestation_certificates(1, self.device)
        requests = self.requests()
        self.assertEqual([(request.cryptoProfile.PAACrypto, request.cryptoProfile.PAICrypto) for request in requests[:-1]],
                         [(2, 2), (2, 1), (2, 0), (1, 1), (1, 0)])
        self.assertIsNone(requests[-1].cryptoProfile)
        self.assertIsNone(requests[-1].maxSegmentSize)
        self.assertTrue(all(request.certificateType == self.types.kPAICertificate for request in requests))

    async def test_legacy_retry_omits_profiles_for_pai_and_dac(self):
        self.device.PQCDeviceAttestationProfile.PAASupportedProfiles = 5
        self.device.PQCDeviceAttestationProfile.PAISupportedProfiles = 1
        self.controller.SendCommand.side_effect = [
            InteractionModelError(Status.InvalidCommand),
            self.opcreds.Commands.CertificateChainResponse(certificate=b"pai"),
            self.opcreds.Commands.CertificateChainResponse(certificate=b"dac"),
        ]
        self.assertEqual(await self.flow._request_attestation_certificates(1, self.device), (b"pai", b"dac"))
        requests = self.requests()
        self.assertEqual([request.certificateType for request in requests],
                         [self.types.kPAICertificate, self.types.kPAICertificate, self.types.kDACCertificate])
        self.assertEqual((requests[0].cryptoProfile.PAACrypto, requests[0].cryptoProfile.PAICrypto), (2, 0))
        self.assertTrue(all(request.cryptoProfile is None for request in requests[1:]))
        self.validate.assert_not_called()

    async def test_does_not_retry_other_errors(self):
        for error in (InteractionModelError(Status.Failure), TimeoutError(), ChipStackError(1)):
            with self.subTest(error=error):
                self.controller.SendCommand.reset_mock()
                self.controller.SendCommand.side_effect = error
                with self.assertRaises(type(error)):
                    await self.flow._request_attestation_certificates(1, self.device)
                self.controller.SendCommand.assert_awaited_once()

    async def test_does_not_retry_after_pai_transfer_starts_or_dac_is_rejected(self):
        for next_segment in (None, 1):
            with self.subTest(next_segment=next_segment):
                self.controller.SendCommand.reset_mock()
                self.controller.SendCommand.side_effect = [
                    self.opcreds.Commands.CertificateChainResponse(
                        certificate=b"pai", totalDocumentSize=6 if next_segment else 3, nextSegmentID=next_segment),
                    InteractionModelError(Status.InvalidCommand),
                ]
                with self.assertRaises(InteractionModelError):
                    await self.flow._request_attestation_certificates(1, self.device)
                requests = self.requests()
                self.assertEqual(len(requests), 2)
                self.assertEqual(requests[0].cryptoProfile, requests[1].cryptoProfile)
                self.assertEqual(requests[1].certificateType,
                                 self.types.kPAICertificate if next_segment else self.types.kDACCertificate)

    async def test_wrong_certificate_fails_without_fallback(self):
        self.controller.SendCommand.return_value = self.opcreds.Commands.CertificateChainResponse(certificate=b"wrong")
        self.validate.side_effect = ChipStackError(1)
        with self.assertRaisesRegex(CommissionFailure, "does not match the requested attestation profile"):
            await self.flow._request_attestation_certificates(1, self.device)
        self.controller.SendCommand.assert_awaited_once()

    async def test_legacy_requests_omit_profiles_and_do_not_retry(self):
        self.device.featureMap = 0
        self.controller.SendCommand.side_effect = [
            self.opcreds.Commands.CertificateChainResponse(certificate=b"pai"),
            self.opcreds.Commands.CertificateChainResponse(certificate=b"dac"),
        ]
        self.assertEqual(await self.flow._request_attestation_certificates(1, self.device), (b"pai", b"dac"))
        self.assertTrue(all(request.cryptoProfile is None for request in self.requests()))
        self.controller.SendCommand.reset_mock()
        self.controller.SendCommand.side_effect = InteractionModelError(Status.InvalidCommand)
        with self.assertRaises(InteractionModelError):
            await self.flow._request_attestation_certificates(1, self.device)
        self.controller.SendCommand.assert_awaited_once()


class TestCertificateChainDocumentLimit(unittest.IsolatedAsyncioTestCase):
    async def test_serializes_certificate_request_profiles(self):
        opcreds = Clusters.OperationalCredentials
        certificate_types = opcreds.Enums.CertificateChainTypeEnum
        profiles = opcreds.Enums.AttestationCryptoProfileEnum
        chain = opcreds.Structs.CryptoProfileChainStruct(
            PAACrypto=profiles.kMlDsa65, PAICrypto=profiles.kMlDsa44, DACCrypto=profiles.kEcdsaMatterLegacy)
        cases = (
            (certificate_types.kPAICertificate, None),
            (certificate_types.kDACCertificate, None),
            (certificate_types.kPAICertificate, chain),
            (certificate_types.kDACCertificate, chain),
        )
        for certificate_type, expected in cases:
            with self.subTest(certificate_type=certificate_type, chain=expected):
                response = opcreds.Commands.CertificateChainResponse(certificate=b"certificate")
                controller = SimpleNamespace(SendCommand=AsyncMock(return_value=response))
                flow = CommissioningFlowBlocks(controller, None, logging.getLogger(__name__))
                with patch("matter.commissioning.commissioning_flow_blocks.get_max_certificate_chain_document_size",
                           return_value=1024), patch(
                               "matter.commissioning.commissioning_flow_blocks.validate_attestation_certificate_profile") as validate:
                    self.assertEqual(await flow._request_certificate_chain(1, certificate_type, expected), b"certificate")
                if expected is None:
                    validate.assert_not_called()
                else:
                    validate.assert_called_once()
                controller.SendCommand.assert_awaited_once()
                request = controller.SendCommand.await_args.args[2]
                decoded = type(request).FromTLV(request.ToTLV())
                self.assertEqual(decoded, request)
                self.assertEqual(decoded.cryptoProfile, expected)
                self.assertEqual(decoded.certificateType, certificate_type)
                if expected is None:
                    self.assertIsNone(decoded.segmentID)
                    self.assertIsNone(decoded.maxSegmentSize)

    async def test_all_segments_keep_the_selected_chain(self):
        opcreds = Clusters.OperationalCredentials
        chain = opcreds.Structs.CryptoProfileChainStruct(PAACrypto=2, PAICrypto=1, DACCrypto=0)
        for certificate_type, subject, issuer in ((opcreds.Enums.CertificateChainTypeEnum.kPAICertificate, 1, 2),
                                                  (opcreds.Enums.CertificateChainTypeEnum.kDACCertificate, 0, 1)):
            with self.subTest(certificate_type=certificate_type):
                responses = [opcreds.Commands.CertificateChainResponse(
                    certificate=data, totalDocumentSize=6, nextSegmentID=segment)
                    for data, segment in ((b"ab", 1), (b"cd", 2), (b"ef", None))]
                controller = SimpleNamespace(SendCommand=AsyncMock(side_effect=responses))
                flow = CommissioningFlowBlocks(controller, None, logging.getLogger(__name__))
                with patch("matter.commissioning.commissioning_flow_blocks.get_max_certificate_chain_document_size",
                           return_value=8) as native_limit, patch(
                               "matter.commissioning.commissioning_flow_blocks.validate_attestation_certificate_profile") as validate:
                    self.assertEqual(await flow._request_certificate_chain(1, certificate_type, chain), b"abcdef")
                    native_limit.assert_called_once_with(subject, issuer)
                    validate.assert_called_once_with(b"abcdef", subject, issuer)
                for index, call in enumerate(controller.SendCommand.await_args_list):
                    request = call.args[2]
                    decoded = type(request).FromTLV(request.ToTLV())
                    self.assertEqual(decoded.cryptoProfile, chain)
                    self.assertEqual(decoded.certificateType, certificate_type)
                    self.assertEqual(decoded.segmentID, index if index else None)

    async def test_rejects_certificate_profile_mismatch(self):
        opcreds = Clusters.OperationalCredentials
        chain = opcreds.Structs.CryptoProfileChainStruct(PAACrypto=2, PAICrypto=1, DACCrypto=0)
        for certificate_type, subject, issuer in ((opcreds.Enums.CertificateChainTypeEnum.kPAICertificate, 1, 2),
                                                  (opcreds.Enums.CertificateChainTypeEnum.kDACCertificate, 0, 1)):
            with self.subTest(certificate_type=certificate_type):
                controller = SimpleNamespace(SendCommand=AsyncMock(return_value=opcreds.Commands.CertificateChainResponse(
                    certificate=b"wrong certificate", totalDocumentSize=17)))
                flow = CommissioningFlowBlocks(controller, None, logging.getLogger(__name__))
                with patch("matter.commissioning.commissioning_flow_blocks.get_max_certificate_chain_document_size",
                           return_value=1024), patch(
                               "matter.commissioning.commissioning_flow_blocks.validate_attestation_certificate_profile",
                               side_effect=ChipStackError(1)) as validate, self.assertRaisesRegex(
                                   CommissionFailure, "does not match the requested attestation profile"):
                    await flow._request_certificate_chain(1, certificate_type, chain)
                validate.assert_called_once_with(b"wrong certificate", subject, issuer)
                controller.SendCommand.assert_awaited_once()

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
                await flow._request_certificate_chain(1, 1, Clusters.OperationalCredentials.Structs.CryptoProfileChainStruct(
                    PAACrypto=profiles.kMlDsa65, PAICrypto=profiles.kMlDsa44, DACCrypto=profiles.kEcdsaMatterLegacy))
            native_limit.assert_called_once_with(profiles.kEcdsaMatterLegacy, profiles.kMlDsa44)
        controller.SendCommand.assert_awaited_once()


if __name__ == "__main__":
    unittest.main()
