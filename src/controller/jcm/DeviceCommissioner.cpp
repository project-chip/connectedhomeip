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

#include "DeviceCommissioner.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/InteractionModelEngine.h>
#include <controller/CommissioningDelegate.h>
#include <credentials/CHIPCert.h>
#include <credentials/FabricTable.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::Crypto;
using namespace chip::app::Clusters;

namespace chip {
namespace Controller {
namespace JCM {

/*
 * DeviceCommissioner public interface and override implementation
 */
CHIP_ERROR DeviceCommissioner::StartJCMTrustVerification(DeviceProxy *proxy)
{
    TrustVerificationError error = TrustVerificationError::kSuccess;

    mDeviceProxy = proxy;

    ChipLogProgress(Controller, "JCM: Starting Trust Verification");

    TrustVerificationStageFinished(TrustVerificationStage::kIdle, error);
    if (error != TrustVerificationError::kSuccess)
    {
        ChipLogError(Controller, "JCM: Failed to start Trust Verification: %s", EnumToString(error).c_str());
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

void DeviceCommissioner::ContinueAfterUserConsent(bool consent)
{
    TrustVerificationError error = TrustVerificationError::kSuccess;

    if (!consent)
    {
        error = TrustVerificationError::kUserDeniedConsent;
    }

    TrustVerificationStageFinished(TrustVerificationStage::kAskingUserForConsent, error);
}


void DeviceCommissioner::ContinueAfterVendorIDVerification(CHIP_ERROR err)
{
    TrustVerificationError error = TrustVerificationError::kSuccess;

    if (err != CHIP_NO_ERROR)
    {
        error = TrustVerificationError::kVendorIdVerificationFailed;
    }

    TrustVerificationStageFinished(TrustVerificationStage::kPerformingVendorIDVerification, error);
}

CHIP_ERROR DeviceCommissioner::ParseAdminFabricIndexAndEndpointId(ReadCommissioningInfo & info)
{
    auto attributeCache = info.attributes;

    CHIP_ERROR err = attributeCache->ForEachAttribute(
        Clusters::JointFabricAdministrator::Id, [this, &attributeCache](const ConcreteAttributePath & path) {
            using namespace Clusters::JointFabricAdministrator::Attributes;
            AdministratorFabricIndex::TypeInfo::DecodableType administratorFabricIndex;

            VerifyOrReturnError(path.mAttributeId == AdministratorFabricIndex::Id, CHIP_NO_ERROR);
            ReturnErrorOnFailure(attributeCache->Get<AdministratorFabricIndex::TypeInfo>(path, administratorFabricIndex));

            if (!administratorFabricIndex.IsNull() && administratorFabricIndex.Value() != kUndefinedFabricIndex)
            {
                ChipLogProgress(Controller, "JCM: AdministratorFabricIndex: %d", administratorFabricIndex.Value());
                mInfo.adminFabricIndex = administratorFabricIndex.Value();
                mInfo.adminEndpointId  = path.mEndpointId;
            }
            else
            {
                ChipLogError(Controller, "JCM: JF Administrator Cluster not found!");
                return CHIP_ERROR_NOT_FOUND;
            }

            // This is not an error; error checking is after iterating through the attributes
            return CHIP_NO_ERROR;
        });

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JCM: Failed to parse Administrator Fabric Index: %s", err.AsString());
        return err;
    }

    if (mInfo.adminFabricIndex == kUndefinedFabricIndex)
    {
        ChipLogError(Controller, "JCM: Invalid Fabric Index");
        return CHIP_ERROR_NOT_FOUND;
    }

    if (mInfo.adminEndpointId == kInvalidEndpointId)
    {
        ChipLogError(Controller, "JCM: Invalid Endpoint ID");
        return CHIP_ERROR_NOT_FOUND;
    }

    ChipLogProgress(Controller, "JCM: Successfully parsed the Administrator Fabric Index and Endpoint ID");

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::ParseOperationalCredentials(ReadCommissioningInfo & info)
{
    auto attributeCache = info.attributes;

    CHIP_ERROR err =
        attributeCache->ForEachAttribute(OperationalCredentials::Id, [this, &attributeCache](const ConcreteAttributePath & path) {
            using namespace chip::app::Clusters::OperationalCredentials::Attributes;

            switch (path.mAttributeId)
            {
            case Fabrics::Id: {
                Fabrics::TypeInfo::DecodableType fabrics;
                ReturnErrorOnFailure(attributeCache->Get<Fabrics::TypeInfo>(path, fabrics));
                bool foundMatchingFabricIndex = false;

                auto iter = fabrics.begin();
                while (iter.Next())
                {
                    auto & fabricDescriptor = iter.GetValue();

                    if (fabricDescriptor.VIDVerificationStatement.HasValue())
                    {
                        ChipLogError(
                            Controller,
                            "JCM: A VID Verification Statement is not supported, Joint Fabric requires an ICA on the fabric!");
                        return CHIP_ERROR_CANCELLED;
                    }

                    if (fabricDescriptor.fabricIndex == mInfo.adminFabricIndex)
                    {
                        if (fabricDescriptor.rootPublicKey.size() != kP256_PublicKey_Length)
                        {
                            ChipLogError(Controller, "JCM: Fabric root key size mismatch");
                            return CHIP_ERROR_KEY_NOT_FOUND;
                        }

                        mInfo.rootPublicKey.CopyFromSpan(fabricDescriptor.rootPublicKey);
                        mInfo.adminVendorId      = fabricDescriptor.vendorID;
                        mInfo.adminFabricId      = fabricDescriptor.fabricID;
                        foundMatchingFabricIndex = true;

                        ChipLogProgress(Controller, "JCM: Successfully parsed the Administrator Fabric Table");
                        break;
                    }
                }
                if (!foundMatchingFabricIndex)
                {
                    return CHIP_ERROR_NOT_FOUND;
                }

                // Successfully parsed the Fabric Descriptor
                return CHIP_NO_ERROR;
            }
            case NOCs::Id: {
                NOCs::TypeInfo::DecodableType nocs;
                ReturnErrorOnFailure(attributeCache->Get<NOCs::TypeInfo>(path, nocs));

                auto iter = nocs.begin();
                while (iter.Next())
                {
                    auto & nocStruct = iter.GetValue();

                    if (nocStruct.fabricIndex == mInfo.adminFabricIndex)
                    {
                        mInfo.adminNOC.CopyFromSpan(nocStruct.noc);

                        if (!nocStruct.icac.IsNull())
                        {
                            auto icac = nocStruct.icac.Value();
                            mInfo.adminICAC.CopyFromSpan(icac);
                        }
                        else
                        {
                            ChipLogError(Controller, "JCM: ICAC not present!");
                            return CHIP_ERROR_CERT_NOT_FOUND;
                        }
                        ChipLogProgress(Controller, "JCM: Successfully parsed the Administrator NOC and ICAC");
                        break;
                    }
                }
                return CHIP_NO_ERROR;
            }
            default:
                // Ignore other attributes; this is not an error condition
                return CHIP_NO_ERROR;
            }

            // This is not an error; error checking is after iterating through the attributes
            return CHIP_NO_ERROR;
        });

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JCM: Failed to parse Operational Credentials: %s", err.AsString());
        return err;
    }

    if (mInfo.rootPublicKey.AllocatedSize() == 0)
    {
        ChipLogError(Controller, "JCM: Root public key is empty!");
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    if (mInfo.adminNOC.AllocatedSize() == 0)
    {
        ChipLogError(Controller, "JCM: Administrator NOC is empty!");
        return CHIP_ERROR_CERT_NOT_FOUND;
    }

    if (mInfo.adminICAC.AllocatedSize() == 0)
    {
        ChipLogError(Controller, "JCM: Administrator ICAC is empty!");
        return CHIP_ERROR_CERT_NOT_FOUND;
    }

    ChipLogProgress(Controller, "JCM: Successfully parsed the Operational Credentials");

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceCommissioner::ParseTrustedRoot(ReadCommissioningInfo & info)
{
    auto attributeCache = info.attributes;

    CHIP_ERROR err =
        attributeCache->ForEachAttribute(OperationalCredentials::Id, [this, &attributeCache](const ConcreteAttributePath & path) {
            using namespace chip::app::Clusters::OperationalCredentials::Attributes;

            switch (path.mAttributeId)
            {
            case TrustedRootCertificates::Id: {
                bool foundMatchingRcac = false;
                TrustedRootCertificates::TypeInfo::DecodableType trustedCAs;
                ReturnErrorOnFailure(attributeCache->Get<TrustedRootCertificates::TypeInfo>(path, trustedCAs));

                auto iter = trustedCAs.begin();
                while (iter.Next())
                {
                    auto & trustedCA = iter.GetValue();
                    P256PublicKeySpan trustedCAPublicKeySpan;

                    ReturnErrorOnFailure(ExtractPublicKeyFromChipCert(trustedCA, trustedCAPublicKeySpan));
                    P256PublicKey trustedCAPublicKey{ trustedCAPublicKeySpan };

                    if (mInfo.rootPublicKey.AllocatedSize() != kP256_PublicKey_Length)
                    {
                        ChipLogError(Controller, "JCM: Fabric root key size mismatch");
                        return CHIP_ERROR_KEY_NOT_FOUND;
                    }

                    P256PublicKeySpan rootPubKeySpan(mInfo.rootPublicKey.Get());
                    P256PublicKey fabricTableRootPublicKey{ rootPubKeySpan };

                    if (trustedCAPublicKey.Matches(fabricTableRootPublicKey) && trustedCA.size())
                    {
                        mInfo.adminRCAC.CopyFromSpan(trustedCA);
                        // Successfully found the matching RCAC
                        foundMatchingRcac = true;
                        break;
                    }
                }
                if (!foundMatchingRcac)
                {
                    // Since a matching RCAC was not found, we cannot proceed
                    return CHIP_ERROR_CERT_NOT_FOUND;
                }

                // Successfully parsed the Trusted Root Certificates
                return CHIP_NO_ERROR;
            }
            default:
                // Ignore other attributes; this is not an error condition
                return CHIP_NO_ERROR;
            }
            return CHIP_NO_ERROR;
        });

    if (mInfo.adminRCAC.AllocatedSize() == 0)
    {
        ChipLogError(Controller, "JCM: Did not find a matching RCAC!");
        mInfo.adminFabricIndex = kUndefinedFabricIndex;
        return CHIP_ERROR_CERT_NOT_FOUND;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JCM: Failed to parse Trusted Root Certificates: %s", err.AsString());
    }

    return err;
}

CHIP_ERROR DeviceCommissioner::ParseExtraCommissioningInfo(ReadCommissioningInfo & info, const CommissioningParameters & params)
{
    using namespace OperationalCredentials::Attributes;

    CHIP_ERROR err = CHIP_NO_ERROR;

#if CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
    if (!params.GetUseJCM().ValueOr(false))
    {
        return chip::Controller::DeviceCommissioner::ParseExtraCommissioningInfo(info, params);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC

    err = ParseAdminFabricIndexAndEndpointId(info);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JCM: Failed to find Administrator Fabric Index and Endpoint ID");
        return err;
    }

    err = ParseOperationalCredentials(info);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JCM: Failed to find Fabric Descriptor Information");
        return err;
    }

    err = ParseTrustedRoot(info);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JCM: Failed to find Trusted Root Certificate");
        return err;
    }

    return chip::Controller::DeviceCommissioner::ParseExtraCommissioningInfo(info, params);
}

TrustVerificationError DeviceCommissioner::VerifyAdministratorInformation()
{
    ChipLogProgress(Controller, "JCM: Verify joint fabric administrator endpoint and fabric index");

    if (mInfo.adminEndpointId == kInvalidEndpointId)
    {
        ChipLogError(Controller, "JCM: Administrator endpoint ID not found!");
        return TrustVerificationError::kInvalidAdministratorEndpointId;
    }

    if (mInfo.adminFabricIndex == kUndefinedFabricIndex)
    {
        ChipLogError(Controller, "JCM: Administrator fabric index not found!");
        return TrustVerificationError::kInvalidAdministratorFabricIndex;
    }

    CATValues cats;
    auto nocSpan = mInfo.adminNOC.Span();
    ExtractCATsFromOpCert(nocSpan, cats);

    if (!cats.ContainsIdentifier(kAdminCATIdentifier))
    {
        return TrustVerificationError::kInvalidAdministratorCAT;
    }

    ChipLogProgress(Controller, "JCM: Administrator endpoint ID: %d", mInfo.adminEndpointId);
    ChipLogProgress(Controller, "JCM: Administrator fabric index: %d", mInfo.adminFabricIndex);
    ChipLogProgress(Controller, "JCM: Administrator vendor ID: %d", mInfo.adminVendorId);
    ChipLogProgress(Controller, "JCM: Administrator fabric ID: %llu", static_cast<unsigned long long>(mInfo.adminFabricId));

    return TrustVerificationError::kSuccess;
}

static CHIP_ERROR VerifyNOCCertificateChain(ByteSpan nocSpan, ByteSpan icacSpan, ByteSpan rcacSpan)
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

void DeviceCommissioner::ContinueAfterLookupOperationalTrustAnchor(CHIP_ERROR err, ByteSpan globallyTrustedRootSpan)
{
    if (err != CHIP_NO_ERROR)
    {
        ContinueAfterVendorIDVerification(err);
        return;
    }

    // 12bii. Verify that the NOC chain is valid using Crypto_VerifyChain() against the entire chain reconstructed 
    // from the NOCs attribute entry whose FabricIndex field matches the fabric being verified, but populating 
    // the trusted root with the GloballyTrustedRoot certificate rather than the value in TrustedRootCertificates 
    // associated with the candidate fabric. If the chain is not valid, the procedure terminates as failed.
    err = VerifyNOCCertificateChain(mInfo.adminNOC.Span(), mInfo.adminICAC.Span(), globallyTrustedRootSpan);

    // 13. If all prior step succeeded err == CHIP_NO_ERROR and the candidate fabric's VendorID SHALL be deemed authentic.
    ContinueAfterVendorIDVerification(err);
}

CHIP_ERROR DeviceCommissioner::OnSignVIDVerificationSuccessCb(
    ByteSpan signatureSpan,
    ByteSpan clientChallengeSpan)
{
    ChipLogProgress(Controller, "Vendor ID verification succeeded for vendor ID: %u", mInfo.adminVendorId);

    // Extract the root public key
    P256PublicKeySpan rootPublicKeySpan(mInfo.rootPublicKey.Get());
    P256PublicKey rootPublicKey(rootPublicKeySpan);

    // Locally generate the vendor_fabric_binding_message
    uint8_t vendorFabricBindingMessageBuffer[kVendorFabricBindingMessageV1Size];
    MutableByteSpan vendorFabricBindingMessageSpan{ vendorFabricBindingMessageBuffer };
    ReturnLogErrorOnFailure(Crypto::GenerateVendorFabricBindingMessage(FabricBindingVersion::kVersion1, rootPublicKey,
                  mInfo.adminFabricIndex, mInfo.adminVendorId, vendorFabricBindingMessageSpan));

    // Locally generate the vendor_id_verification_tbs message
    ByteSpan vidVerificationStatementSpan;
    uint8_t vidVerificationStatementBuffer[kVendorIdVerificationTbsV1MaxSize];
    MutableByteSpan vidVerificationTbsSpan{ vidVerificationStatementBuffer };

    // Retrieve attestation challenge
    ByteSpan attestationChallengeSpan = mDeviceProxy->GetSecureSession().Value()->AsSecureSession()->GetCryptoContext().GetAttestationChallenge();
    ReturnLogErrorOnFailure(GenerateVendorIdVerificationToBeSigned(mInfo.adminFabricIndex, clientChallengeSpan, attestationChallengeSpan,
                                                            vendorFabricBindingMessageSpan, vidVerificationStatementSpan,
                                                            vidVerificationTbsSpan));

    // 10. Given the subject public key associated with the fabric being verified, validate that Crypto_Verify(noc_public_key,
    // vendor_id_verification_tbs, signature) succeeds, otherwise the procedure terminates as failed.
    VerifyOrReturnError(signatureSpan.size() >= P256ECDSASignature::Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    P256PublicKeySpan nocPublicKeySpan;
    ReturnLogErrorOnFailure(ExtractPublicKeyFromChipCert(mInfo.adminNOC.Span(), nocPublicKeySpan));

    P256PublicKey nocPublicKey(nocPublicKeySpan);
    P256ECDSASignature signature;
    memcpy(signature.Bytes(), signatureSpan.data(), signature.Capacity());
    signature.SetLength(signature.Capacity());
    ReturnLogErrorOnFailure(nocPublicKey.ECDSA_validate_msg_signature(vidVerificationTbsSpan.data(), vidVerificationTbsSpan.size(), signature));

    // 11. Verify that the NOC chain is valid using Crypto_VerifyChain() against the entire chain reconstructed from both the
    // NOCs and TrustedRootCertificates attribute entries whose FabricIndex field matches the fabric being verified.
    // If the chain is not valid, the procedure terminates as failed.
    ReturnLogErrorOnFailure(VerifyNOCCertificateChain(mInfo.adminNOC.Span(), mInfo.adminICAC.Span(), mInfo.adminRCAC.Span()));

    // 12a If the VIDVerificationStatement field was present in the entry in Fabrics whose VendorID is being verified 
    // (Not currently supported)
    // 12b. Otherwise (i.e VIDVerificationStatement not used):
    // 12bi. Look-up the entry in the Operational Trust Anchors Schema under the expected VendorID (VID field) 
    // being verified whose IsRoot field is true and whose SubjectKeyID matches the SubjectKeyID field value of 
    // the TrustedRootCertificates attribute entry whose FabricIndex field matches the fabric being verified 
    // (i.e. the RCAC of the candidate fabric)
    mTrustVerificationDelegate->OnLookupOperationalTrustAnchor(*this, mInfo);
    
    // 13. Given that all prior steps succeeded, the candidate fabric’s VendorID SHALL be deemed authentic and err should be CHIP_NO_ERROR.
    return CHIP_NO_ERROR;
}

TrustVerificationError DeviceCommissioner::PerformVendorIDVerificationProcedure()
{
    ChipLogProgress(Controller, "Performing vendor ID verification for vendor ID: %u", mInfo.adminVendorId);

    // Generate a 32-octet random challenge
    uint8_t kClientChallenge[32];
    DRBG_get_bytes(kClientChallenge, sizeof(kClientChallenge));
    ByteSpan clientChallengeSpan{ kClientChallenge };
    chip::app::Clusters::OperationalCredentials::Commands::SignVIDVerificationRequest::Type request;

    // The selected fabric information that matches the Fabrics attribute whose VendorID field to be verified 
    // is in the JCMTrustVerificationInfo info
    request.fabricIndex = mInfo.adminFabricIndex;
    request.clientChallenge = clientChallengeSpan;

    auto onSuccessCb = [this, &clientChallengeSpan](const app::ConcreteCommandPath & aPath, const app::StatusIB & aStatus,
                                     const decltype(request)::ResponseType & responseData) {
        CHIP_ERROR err = this->OnSignVIDVerificationSuccessCb(responseData.signature, clientChallengeSpan); 
        if (err != CHIP_NO_ERROR)
        {
            this->ContinueAfterVendorIDVerification(err);
        }
    };

    auto onFailureCb = [this](CHIP_ERROR err) {
        this->ContinueAfterVendorIDVerification(err);
    };

    CHIP_ERROR err = InvokeCommandRequest(mDeviceProxy->GetExchangeManager(), mDeviceProxy->GetSecureSession().Value(), 
                                            kRootEndpointId, request,
                                            onSuccessCb, onFailureCb);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to send SignVIDVerificationRequest: %s", ErrorStr(err));
        ContinueAfterVendorIDVerification(err);
    }

    if (mTrustVerificationDelegate == nullptr)
    {
        ChipLogError(Controller, "JCM: TrustVerificationDelegate is not set");
        return TrustVerificationError::kTrustVerificationDelegateNotSet; // Indicate that the delegate is not set
    }


    return TrustVerificationError::kAsync; // Indicate that this is an async operation
}

TrustVerificationError DeviceCommissioner::AskUserForConsent()
{
    ChipLogProgress(Controller, "JCM: Asking user for consent");
    if (mTrustVerificationDelegate == nullptr)
    {
        ChipLogError(Controller, "JCM: TrustVerificationDelegate is not set");
        return TrustVerificationError::kTrustVerificationDelegateNotSet; // Indicate that the delegate is not set
    }

    mTrustVerificationDelegate->OnAskUserForConsent(*this, mInfo);
    return TrustVerificationError::kAsync; // Indicate that this is an async operation
}

void DeviceCommissioner::PerformTrustVerificationStage(TrustVerificationStage nextStage)
{
    TrustVerificationError error = TrustVerificationError::kSuccess;

    switch (nextStage)
    {
    case TrustVerificationStage::kVerifyingAdministratorInformation:
        error = VerifyAdministratorInformation();
        break;
    case TrustVerificationStage::kPerformingVendorIDVerification:
        error = PerformVendorIDVerificationProcedure();
        break;
    case TrustVerificationStage::kAskingUserForConsent:
        error = AskUserForConsent();
        break;
    case TrustVerificationStage::kComplete:
        error = TrustVerificationError::kSuccess;
        break;
    case TrustVerificationStage::kError:
        error = TrustVerificationError::kInternalError;
        break;
    default:
        ChipLogError(Controller, "JCM: Invalid stage: %d", static_cast<int>(nextStage));
        error = TrustVerificationError::kInternalError;
        break;
    }

    if (error != TrustVerificationError::kAsync)
    {
        TrustVerificationStageFinished(nextStage, error);
    }
}

void DeviceCommissioner::TrustVerificationStageFinished(TrustVerificationStage completedStage, TrustVerificationError error)
{
    ChipLogProgress(Controller, "JCM: Trust Verification Stage Finished: %s", EnumToString(completedStage).c_str());

    if (mTrustVerificationDelegate != nullptr)
    {
        mTrustVerificationDelegate->OnProgressUpdate(*this, completedStage, mInfo, error);
    }

    if (error != TrustVerificationError::kSuccess)
    {
        OnTrustVerificationComplete(error);
        return;
    }

    if (completedStage == TrustVerificationStage::kComplete || completedStage == TrustVerificationStage::kError)
    {
        ChipLogProgress(Controller, "JCM: Trust Verification already complete or error");
        OnTrustVerificationComplete(error);
        return;
    }

    auto nextStage = GetNextTrustVerificationStage(completedStage);
    if (nextStage == TrustVerificationStage::kError)
    {
        OnTrustVerificationComplete(TrustVerificationError::kInternalError);
        return;
    }

    PerformTrustVerificationStage(nextStage);
}

TrustVerificationStage DeviceCommissioner::GetNextTrustVerificationStage(TrustVerificationStage currentStage)
{
    TrustVerificationStage nextStage = TrustVerificationStage::kIdle;

    switch (currentStage)
    {
    case TrustVerificationStage::kIdle:
        nextStage = TrustVerificationStage::kVerifyingAdministratorInformation;
        break;
    case TrustVerificationStage::kVerifyingAdministratorInformation:
        nextStage = TrustVerificationStage::kPerformingVendorIDVerification;
        break;
    case TrustVerificationStage::kPerformingVendorIDVerification:
        nextStage = TrustVerificationStage::kAskingUserForConsent;
        break;
    case TrustVerificationStage::kAskingUserForConsent:
        nextStage = TrustVerificationStage::kComplete;
        break;
    default:
        ChipLogError(Controller, "JCM: Invalid stage: %d", static_cast<int>(currentStage));
        nextStage = TrustVerificationStage::kError;
        break;
    }

    return nextStage;
}

void DeviceCommissioner::OnTrustVerificationComplete(TrustVerificationError error)
{
    if (error == TrustVerificationError::kSuccess)
    {
        ChipLogProgress(Controller, "JCM: Administrator Device passed JCM Trust Verification");

        CommissioningStageComplete(CHIP_NO_ERROR);
    }
    else
    {
        ChipLogError(Controller, "JCM: Failed in verifying JCM Trust Verification: err %s", EnumToString(error).c_str());

        CommissioningDelegate::CommissioningReport report;
        report.Set<TrustVerificationError>(error);

        CommissioningStageComplete(CHIP_ERROR_INTERNAL, report);
    }
}

void DeviceCommissioner::CleanupCommissioning(DeviceProxy * proxy, NodeId nodeId, const CompletionStatus & completionStatus)
{
    chip::Controller::DeviceCommissioner::CleanupCommissioning(proxy, nodeId, completionStatus);

    mInfo.Cleanup();
}

} // namespace JCM
} // namespace Controller
} // namespace chip
