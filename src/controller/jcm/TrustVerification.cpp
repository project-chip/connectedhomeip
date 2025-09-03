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

#include "TrustVerification.h"

#include <controller/InvokeInteraction.h>
#include <credentials/CHIPCert.h>
#include <credentials/FabricTable.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::Crypto;
using namespace chip::app::Clusters;

namespace chip {
namespace Controller {
namespace JCM {

CHIP_ERROR VendorIdVerificationClient::VerifyNOCCertificateChain(
    const ByteSpan & nocSpan, 
    const ByteSpan  & icacSpan, 
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
    DeviceProxy * deviceProxy, 
    const FabricIndex & fabricIndex, 
    const VendorId & vendorID, 
    const ByteSpan & rcacSpan,
    const ByteSpan & icacSpan,
    const ByteSpan & nocSpan,
    const ByteSpan clientChallengeSpan,
    const SignVIDVerificationResponse::DecodableType responseData
)
{
    // Steps 1-9 have already been completed prior to the response callback

    // Extract the root public key
    P256PublicKeySpan rcacPublicKeySpan;
    ReturnLogErrorOnFailure(ExtractPublicKeyFromChipCert(rcacSpan, rcacPublicKeySpan));
    // Extract the root public key
    P256PublicKey rcacPublicKey(rcacPublicKeySpan);

    // Locally generate the vendor_fabric_binding_message
    uint8_t vendorFabricBindingMessageBuffer[kVendorFabricBindingMessageV1Size];
    MutableByteSpan vendorFabricBindingMessageSpan{ vendorFabricBindingMessageBuffer };
    ReturnLogErrorOnFailure(Crypto::GenerateVendorFabricBindingMessage(FabricBindingVersion::kVersion1, rcacPublicKey,
                  fabricIndex, vendorID, vendorFabricBindingMessageSpan));

    // Locally generate the vendor_id_verification_tbs message
    ByteSpan vidVerificationStatementSpan;
    uint8_t vidVerificationStatementBuffer[kVendorIdVerificationTbsV1MaxSize];
    MutableByteSpan vidVerificationTbsSpan{ vidVerificationStatementBuffer };

    // Retrieve attestation challenge
    ByteSpan attestationChallengeSpan = deviceProxy->GetSecureSession().Value()->AsSecureSession()->GetCryptoContext().GetAttestationChallenge();
    ReturnLogErrorOnFailure(GenerateVendorIdVerificationToBeSigned(fabricIndex, clientChallengeSpan, attestationChallengeSpan,
                                                            vendorFabricBindingMessageSpan, vidVerificationStatementSpan,
                                                            vidVerificationTbsSpan));

    // 10. Given the subject public key associated with the fabric being verified, validate that Crypto_Verify(noc_public_key,
    // vendor_id_verification_tbs, signature) succeeds, otherwise the procedure terminates as failed.
    ByteSpan signatureSpan = responseData.signature;
    VerifyOrReturnError(signatureSpan.size() >= P256ECDSASignature::Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    P256PublicKeySpan nocPublicKeySpan;
    ReturnLogErrorOnFailure(ExtractPublicKeyFromChipCert(nocSpan, nocPublicKeySpan));

    P256PublicKey nocPublicKey(nocPublicKeySpan);
    P256ECDSASignature signature;
    memcpy(signature.Bytes(), signatureSpan.data(), signature.Capacity());
    signature.SetLength(signature.Capacity());
    ReturnLogErrorOnFailure(nocPublicKey.ECDSA_validate_msg_signature(vidVerificationTbsSpan.data(), vidVerificationTbsSpan.size(), signature));

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

    // 13. Given that all prior steps succeeded, the candidate fabric’s VendorID SHALL be deemed authentic and err should be CHIP_NO_ERROR.
    return CHIP_NO_ERROR;
}

CHIP_ERROR VendorIdVerificationClient::VerifyVendorId(
    DeviceProxy * deviceProxy,
    const FabricIndex & fabricIndex,
    const VendorId & vendorID,
    const ByteSpan & rcacSpan,
    const ByteSpan & icacSpan,
    const ByteSpan & nocSpan)
{
    ChipLogProgress(Controller, "Performing vendor ID verification for vendor ID: %u", vendorID);

    // Generate a 32-octet random challenge
    uint8_t kClientChallenge[32];
    DRBG_get_bytes(kClientChallenge, sizeof(kClientChallenge));
    ByteSpan clientChallengeSpan{ kClientChallenge };
    chip::app::Clusters::OperationalCredentials::Commands::SignVIDVerificationRequest::Type request;

    // The selected fabric information that matches the Fabrics attribute whose VendorID field to be verified 
    // is in the JCMTrustVerificationInfo info
    request.fabricIndex = fabricIndex;
    request.clientChallenge = clientChallengeSpan;

    auto onSuccessCb = [this, deviceProxy, fabricIndex, vendorID, rcacSpan, icacSpan, nocSpan, kClientChallenge](const app::ConcreteCommandPath & aPath, const app::StatusIB & aStatus,
                                     const decltype(request)::ResponseType & responseData) {
        ChipLogProgress(Controller, "Successfully received SignVIDVerificationResponse");
        ByteSpan clientChallenge{ kClientChallenge };
        CHIP_ERROR err = this->Verify(deviceProxy, fabricIndex, vendorID, rcacSpan, icacSpan, nocSpan, clientChallenge, responseData);
        ChipLogProgress(Controller, "Vendor ID verification completed with result: %s", ErrorStr(err));
        this->OnVendorIdVerficationComplete(err);
    };

    auto onFailureCb = [this](CHIP_ERROR err) {
        ChipLogError(Controller, "Failed to receive SignVIDVerificationResponse: %s", ErrorStr(err));
        this->OnVendorIdVerficationComplete(err);
    };

    CHIP_ERROR err = InvokeCommandRequest(deviceProxy->GetExchangeManager(), deviceProxy->GetSecureSession().Value(), 
                                            kRootEndpointId, request,
                                            onSuccessCb, onFailureCb);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to send SignVIDVerificationRequest: %s", ErrorStr(err));
        this->OnVendorIdVerficationComplete(err);
    }

    return CHIP_NO_ERROR;
}

} // namespace JCM
} // namespace Controller
} // namespace chip