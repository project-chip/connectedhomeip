/*
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "VendorIdVerificationClient.h"

#include <controller/InvokeInteraction.h>
#include <credentials/CHIPCert.h>
#include <credentials/FabricTable.h>

namespace chip {
namespace Credentials {
namespace JCM {

CHIP_ERROR VendorIdVerificationClient::VerifyNOCCertificateChain(const ByteSpan & nocSpan, const ByteSpan & icacSpan,
                                                                 const ByteSpan & rcacSpan)
{
    ValidationContext validContext;

    validContext.Reset();
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kClientAuth);

    ChipCertificateSet certificates;
    constexpr uint8_t kMaxNumCertsInOpCreds = 3;
    ReturnLogErrorOnFailure(certificates.Init(kMaxNumCertsInOpCreds));
    ReturnLogErrorOnFailure(certificates.LoadCert(rcacSpan, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));
    if (!icacSpan.empty())
    {
        ReturnLogErrorOnFailure(certificates.LoadCert(icacSpan, BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash)));
    }
    ReturnLogErrorOnFailure(certificates.LoadCert(nocSpan, BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash)));
    ReturnLogErrorOnFailure(certificates.ValidateCert(certificates.GetLastCert(), validContext));

    return CHIP_NO_ERROR;
}

CHIP_ERROR VendorIdVerificationClient::Verify(
    TrustVerificationInfo * info, const ByteSpan clientChallengeSpan, const ByteSpan attestationChallengeSpan,
    const app::Clusters::OperationalCredentials::Commands::SignVIDVerificationResponse::DecodableType responseData)
{
    // Steps 1-9 have already been completed prior to the response callback
    const FabricIndex fabricIndex = info->adminFabricIndex;
    const VendorId vendorID       = info->adminVendorId;
    const FabricId fabricId       = info->adminFabricId;
    auto rcacSpan                 = info->adminRCAC.Span();
    auto icacSpan                 = info->adminICAC.Span();
    auto nocSpan                  = info->adminNOC.Span();

    // Extract the root public key
    P256PublicKeySpan rcacPublicKeySpan;
    ReturnLogErrorOnFailure(ExtractPublicKeyFromChipCert(rcacSpan, rcacPublicKeySpan));
    Crypto::P256PublicKey rcacPublicKey(rcacPublicKeySpan);

    // Locally generate the vendor_fabric_binding_message
    uint8_t vendorFabricBindingMessageBuffer[Crypto::kVendorFabricBindingMessageV1Size];
    MutableByteSpan vendorFabricBindingMessageSpan{ vendorFabricBindingMessageBuffer };
    ReturnLogErrorOnFailure(Crypto::GenerateVendorFabricBindingMessage(Crypto::FabricBindingVersion::kVersion1, rcacPublicKey,
                                                                       fabricId, vendorID, vendorFabricBindingMessageSpan));

    // Locally generate the vendor_id_verification_tbs message
    ByteSpan vidVerificationStatementSpan;
    uint8_t vidVerificationStatementBuffer[Crypto::kVendorIdVerificationTbsV1MaxSize];
    MutableByteSpan vidVerificationTbsSpan{ vidVerificationStatementBuffer };

    // Generate the ToBeSigned portion of the VID verification
    ReturnLogErrorOnFailure(Crypto::GenerateVendorIdVerificationToBeSigned(fabricIndex, clientChallengeSpan,
                                                                           attestationChallengeSpan, vendorFabricBindingMessageSpan,
                                                                           vidVerificationStatementSpan, vidVerificationTbsSpan));

    // Extract the operational public key
    P256PublicKeySpan nocPublicKeySpan;
    ReturnLogErrorOnFailure(ExtractPublicKeyFromChipCert(nocSpan, nocPublicKeySpan));
    Crypto::P256PublicKey nocPublicKey(nocPublicKeySpan);

    // 10. Given the subject public key associated with the fabric being verified, validate that Crypto_Verify(noc_public_key,
    // vendor_id_verification_tbs, signature) succeeds, otherwise the procedure terminates as failed.
    Crypto::P256ECDSASignature signature;
    ReturnValueOnFailure(signature.SetLength(responseData.signature.size()), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(signature.Bytes(), responseData.signature.data(), responseData.signature.size());

    ReturnLogErrorOnFailure(
        nocPublicKey.ECDSA_validate_msg_signature(vidVerificationTbsSpan.data(), vidVerificationTbsSpan.size(), signature));

    // 11. Verify that the NOC chain is valid using Crypto_VerifyChain() against the entire chain reconstructed from both the
    // NOCs and TrustedRootCertificates attribute entries whose FabricIndex field matches the fabric being verified.
    // If the chain is not valid, the procedure terminates as failed.
    ReturnLogErrorOnFailure(VerifyNOCCertificateChain(nocSpan, icacSpan, rcacSpan));

    // 12a If the VIDVerificationStatement field was present in the entry in Fabrics whose VendorID is being verified
    // (Not currently supported)
    // 12b. Otherwise (i.e VIDVerificationStatement not used):
    // 12bi. Look-up the entry in the Operational Trust Anchors Schema under the expected VendorID (VID field)
    // being verified whose IsRoot field is true and whose SubjectKeyID matches the SubjectKeyID field value of
    // the TrustedRootCertificates attribute entry whose FabricIndex field matches the fabric being verified
    // (i.e. the RCAC of the candidate fabric)
    ChipCertificateData certData;
    // Decode root cert with default (null) options
    ReturnLogErrorOnFailure(DecodeChipCert(rcacSpan, certData));
    ByteSpan globallyTrustedRootSpan;
    ReturnLogErrorOnFailure(OnLookupOperationalTrustAnchor(vendorID, certData.mSubjectKeyId, globallyTrustedRootSpan));

    // 12bii. Verify that the NOC chain is valid using Crypto_VerifyChain() against the entire chain reconstructed
    // from the NOCs attribute entry whose FabricIndex field matches the fabric being verified, but populating
    // the trusted root with the GloballyTrustedRoot certificate rather than the value in TrustedRootCertificates
    // associated with the candidate fabric. If the chain is not valid, the procedure terminates as failed.
    ReturnLogErrorOnFailure(VerifyNOCCertificateChain(nocSpan, icacSpan, globallyTrustedRootSpan));

    // 13. Given that all prior steps succeeded, the candidate fabric’s VendorID SHALL be deemed authentic and err should be
    // CHIP_NO_ERROR.
    return CHIP_NO_ERROR;
}

CHIP_ERROR VendorIdVerificationClient::VerifyVendorId(Messaging::ExchangeManager * exchangeMgr, const SessionGetterFunc getSession,
                                                      TrustVerificationInfo * info)
{
    ChipLogProgress(Controller, "Performing vendor ID verification for vendor ID: %u", info->adminVendorId);

    // Generate a 32-octet random challenge
    uint8_t kClientChallenge[32];
    TEMPORARY_RETURN_IGNORED Crypto::DRBG_get_bytes(kClientChallenge, sizeof(kClientChallenge));
    ByteSpan clientChallengeSpan{ kClientChallenge };
    chip::app::Clusters::OperationalCredentials::Commands::SignVIDVerificationRequest::Type request;

    // The selected fabric information that matches the Fabrics attribute whose VendorID field to be verified
    // is in the JCMTrustVerificationInfo info
    request.fabricIndex     = info->adminFabricIndex;
    request.clientChallenge = clientChallengeSpan;

    auto onSuccessCb = [&, getSession, info, kClientChallenge](const app::ConcreteCommandPath & aPath,
                                                               const app::StatusIB & aStatus,
                                                               const decltype(request)::ResponseType & responseData) {
        ChipLogProgress(Controller, "Successfully received SignVIDVerificationResponse");
        ByteSpan clientChallenge{ kClientChallenge };
        ByteSpan attestationChallenge = getSession().Value()->AsSecureSession()->GetCryptoContext().GetAttestationChallenge();
        CHIP_ERROR err                = Verify(info, clientChallenge, attestationChallenge, responseData);
        ChipLogProgress(Controller, "Vendor ID verification completed with result: %s", ErrorStr(err));
        OnVendorIdVerificationComplete(err);
    };

    auto onFailureCb = [&](CHIP_ERROR err) {
        ChipLogError(Controller, "Failed to receive SignVIDVerificationResponse: %s", ErrorStr(err));
        OnVendorIdVerificationComplete(err);
    };

    CHIP_ERROR err =
        Controller::InvokeCommandRequest(exchangeMgr, getSession().Value(), kRootEndpointId, request, onSuccessCb, onFailureCb);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to send SignVIDVerificationRequest: %s", ErrorStr(err));
        this->OnVendorIdVerificationComplete(err);
    }

    return CHIP_NO_ERROR;
}

} // namespace JCM
} // namespace Credentials
} // namespace chip
