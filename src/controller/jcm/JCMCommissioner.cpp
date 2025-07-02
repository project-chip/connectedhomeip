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

#include "JCMCommissioner.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/InteractionModelEngine.h>
#include <controller/CommissioningDelegate.h>
#include <credentials/CHIPCert.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace chip::app::Clusters;

namespace chip {
namespace Controller {
namespace JCM {

/*
 * JCMDeviceCommissioner public interface and override implementation
 */
CHIP_ERROR JCMDeviceCommissioner::StartJCMTrustVerification()
{
    JCMTrustVerificationError error = JCMTrustVerificationError::kSuccess;

    ChipLogProgress(Controller, "JCM: Starting Trust Verification");

    TrustVerificationStageFinished(JCMTrustVerificationStage::kIdle, error);
    if (error != JCMTrustVerificationError::kSuccess)
    {
        ChipLogError(Controller, "JCM: Failed to start Trust Verification: %s", EnumToString(error).c_str());
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

void JCMDeviceCommissioner::ContinueAfterUserConsent(bool consent)
{
    JCMTrustVerificationError error = JCMTrustVerificationError::kSuccess;

    if (!consent)
    {
        error = JCMTrustVerificationError::kUserDeniedConsent;
    }

    TrustVerificationStageFinished(JCMTrustVerificationStage::kAskingUserForConsent, error);
}

void JCMDeviceCommissioner::ContinueAfterVendorIDVerification(bool verified)
{
    JCMTrustVerificationError error = JCMTrustVerificationError::kSuccess;

    if (!verified)
    {
        error = JCMTrustVerificationError::kVendorIdVerificationFailed;
    }

    TrustVerificationStageFinished(JCMTrustVerificationStage::kPerformingVendorIDVerification, error);
}

CHIP_ERROR JCMDeviceCommissioner::ParseAdminFabricIndexAndEndpointId(ReadCommissioningInfo & info)
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

CHIP_ERROR JCMDeviceCommissioner::ParseOperationalCredentials(ReadCommissioningInfo & info)
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

                auto iter = fabrics.begin();
                while (iter.Next())
                {
                    auto & fabricDescriptor = iter.GetValue();

                    if (fabricDescriptor.VIDVerificationStatement.HasValue())
                    {
                        ChipLogError(Controller, "JCM: Per-home RCAC are not supported by JF for now!");
                        return CHIP_ERROR_CANCELLED;
                    }

                    if (fabricDescriptor.fabricIndex != kUndefinedFabricIndex)
                    {
                        if (fabricDescriptor.rootPublicKey.size() != Crypto::kP256_PublicKey_Length)
                        {
                            ChipLogError(Controller,
                                         "JCM: DeviceCommissioner::ParseJFAdministratorInfo - fabric root key size mismatch");
                            return CHIP_ERROR_KEY_NOT_FOUND;
                        }

                        mInfo.rootPublicKey.CopyFromSpan(fabricDescriptor.rootPublicKey);
                        mInfo.adminVendorId = fabricDescriptor.vendorID;
                        mInfo.adminFabricId = fabricDescriptor.fabricID;

                        ChipLogProgress(Controller, "JCM: Successfully parsed the Administrator Fabric Table");
                        break;
                    }
                }

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
                return CHIP_NO_ERROR;
            }

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

CHIP_ERROR JCMDeviceCommissioner::ParseTrustedRoot(ReadCommissioningInfo & info)
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
                    Credentials::P256PublicKeySpan trustedCAPublicKeySpan;

                    ReturnErrorOnFailure(Credentials::ExtractPublicKeyFromChipCert(trustedCA, trustedCAPublicKeySpan));
                    Crypto::P256PublicKey trustedCAPublicKey{ trustedCAPublicKeySpan };

                    if (mInfo.rootPublicKey.AllocatedSize() != Crypto::kP256_PublicKey_Length)
                    {
                        ChipLogError(Controller,
                                     "JCM: DeviceCommissioner::ParseJFAdministratorInfo - fabric root key size mismatch");
                        return CHIP_ERROR_KEY_NOT_FOUND;
                    }

                    Credentials::P256PublicKeySpan rootPubKeySpan(mInfo.rootPublicKey.Get());
                    Crypto::P256PublicKey fabricTableRootPublicKey{ rootPubKeySpan };

                    if (trustedCAPublicKey.Matches(fabricTableRootPublicKey) && trustedCA.size())
                    {
                        mInfo.adminRCAC.CopyFromSpan(trustedCA);

                        foundMatchingRcac = true;
                        break;
                    }
                }
                if (!foundMatchingRcac)
                {
                    return CHIP_ERROR_CERT_NOT_FOUND;
                }
                return CHIP_NO_ERROR;
            }
            default:
                return CHIP_NO_ERROR;
            }
            return CHIP_NO_ERROR;
        });

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JCM: Failed to parse Trusted Root Certificates: %s", err.AsString());
        return err;
    }

    if (mInfo.adminRCAC.AllocatedSize() == 0)
    {
        ChipLogError(Controller, "JCM: Did not find a matching RCAC!");
        return CHIP_ERROR_CERT_NOT_FOUND;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR JCMDeviceCommissioner::ParseExtraCommissioningInfo(ReadCommissioningInfo & info, const CommissioningParameters & params)
{
    using namespace OperationalCredentials::Attributes;

    CHIP_ERROR err = CHIP_NO_ERROR;

    if (!params.GetUseJCM().ValueOr(false))
    {
        return DeviceCommissioner::ParseExtraCommissioningInfo(info, params);
    }

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

    return err;
}

JCMTrustVerificationError JCMDeviceCommissioner::VerifyAdministratorInformation()
{
    ChipLogProgress(Controller, "JCM: Verify joint fabric administrator endpoint and fabric index");

    if (mInfo.adminEndpointId == kInvalidEndpointId)
    {
        ChipLogError(Controller, "JCM: Administrator endpoint ID not found!");
        return JCMTrustVerificationError::kInvalidAdministratorEndpointId;
    }

    if (mInfo.adminFabricIndex == kUndefinedFabricIndex)
    {
        ChipLogError(Controller, "JCM: Administrator fabric index not found!");
        return JCMTrustVerificationError::kInvalidAdministratorFabricIndex;
    }

    CATValues cats;
    auto nocSpan = mInfo.adminNOC.GetSpan();
    Credentials::ExtractCATsFromOpCert(nocSpan, cats);

    if (!cats.ContainsIdentifier(kAdminCATIdentifier))
    {
        return JCMTrustVerificationError::kInvalidAdministratorCAT;
    }

    ChipLogProgress(Controller, "JCM: Administrator endpoint ID: %d", mInfo.adminEndpointId);
    ChipLogProgress(Controller, "JCM: Administrator fabric index: %d", mInfo.adminFabricIndex);
    ChipLogProgress(Controller, "JCM: Administrator vendor ID: %d", mInfo.adminVendorId);
    ChipLogProgress(Controller, "JCM: Administrator fabric ID: %llu", (unsigned long long) mInfo.adminFabricId);

    return JCMTrustVerificationError::kSuccess;
}

JCMTrustVerificationError JCMDeviceCommissioner::PerformVendorIDVerificationProcedure()
{
    ChipLogProgress(Controller, "Performing Vendor ID Verification Procedure");

    if (mTrustVerificationDelegate == nullptr)
    {
        ChipLogError(Controller, "JCM: TrustVerificationDelegate is not set");
        return JCMTrustVerificationError::kTrustVerificationDelegateNotSet; // Indicate that the delegate is not set
    }

    mTrustVerificationDelegate->OnVerifyVendorId(*this, mInfo);
    return JCMTrustVerificationError::kAsync; // Indicate that this is an async operation
}

JCMTrustVerificationError JCMDeviceCommissioner::AskUserForConsent()
{
    ChipLogProgress(Controller, "JCM: Asking user for consent");
    if (mTrustVerificationDelegate == nullptr)
    {
        ChipLogError(Controller, "JCM: TrustVerificationDelegate is not set");
        return JCMTrustVerificationError::kTrustVerificationDelegateNotSet; // Indicate that the delegate is not set
    }

    mTrustVerificationDelegate->OnAskUserForConsent(*this, mInfo);
    return JCMTrustVerificationError::kAsync; // Indicate that this is an async operation
}

void JCMDeviceCommissioner::PerformTrustVerificationStage(JCMTrustVerificationStage nextStage)
{
    JCMTrustVerificationError error = JCMTrustVerificationError::kSuccess;

    switch (nextStage)
    {
    case JCMTrustVerificationStage::kVerifyingAdministratorInformation:
        error = VerifyAdministratorInformation();
        break;
    case JCMTrustVerificationStage::kPerformingVendorIDVerification:
        error = PerformVendorIDVerificationProcedure();
        break;
    case JCMTrustVerificationStage::kAskingUserForConsent:
        error = AskUserForConsent();
        break;
    case JCMTrustVerificationStage::kComplete:
        error = JCMTrustVerificationError::kSuccess;
        break;
    case JCMTrustVerificationStage::kError:
        error = JCMTrustVerificationError::kInternalError;
        break;
    default:
        ChipLogError(Controller, "JCM: Invalid stage: %d", static_cast<int>(nextStage));
        error = JCMTrustVerificationError::kInternalError;
        break;
    }

    if (error != JCMTrustVerificationError::kAsync)
    {
        TrustVerificationStageFinished(nextStage, error);
    }
}

void JCMDeviceCommissioner::TrustVerificationStageFinished(JCMTrustVerificationStage completedStage,
                                                           JCMTrustVerificationError error)
{
    ChipLogProgress(Controller, "JCM: Trust Verification Stage Finished: %s", EnumToString(completedStage).c_str());

    if (mTrustVerificationDelegate != nullptr)
    {
        mTrustVerificationDelegate->OnProgressUpdate(*this, completedStage, mInfo, error);
    }

    if (error != JCMTrustVerificationError::kSuccess)
    {
        OnTrustVerificationComplete(error);
        return;
    }

    if (completedStage == JCMTrustVerificationStage::kComplete || completedStage == JCMTrustVerificationStage::kError)
    {
        ChipLogProgress(Controller, "JCM: Trust Verification already complete or error");
        OnTrustVerificationComplete(error);
        return;
    }

    auto nextStage = GetNextTrustVerificationStage(completedStage);
    if (nextStage == JCMTrustVerificationStage::kError)
    {
        OnTrustVerificationComplete(JCMTrustVerificationError::kInternalError);
        return;
    }

    PerformTrustVerificationStage(nextStage);
}

JCMTrustVerificationStage JCMDeviceCommissioner::GetNextTrustVerificationStage(JCMTrustVerificationStage currentStage)
{
    JCMTrustVerificationStage nextStage = JCMTrustVerificationStage::kIdle;

    switch (currentStage)
    {
    case JCMTrustVerificationStage::kIdle:
        nextStage = JCMTrustVerificationStage::kVerifyingAdministratorInformation;
        break;
    case JCMTrustVerificationStage::kVerifyingAdministratorInformation:
        nextStage = JCMTrustVerificationStage::kPerformingVendorIDVerification;
        break;
    case JCMTrustVerificationStage::kPerformingVendorIDVerification:
        nextStage = JCMTrustVerificationStage::kAskingUserForConsent;
        break;
    case JCMTrustVerificationStage::kAskingUserForConsent:
        nextStage = JCMTrustVerificationStage::kComplete;
        break;
    default:
        ChipLogError(Controller, "JCM: Invalid stage: %d", static_cast<int>(currentStage));
        nextStage = JCMTrustVerificationStage::kError;
        break;
    }

    return nextStage;
}

void JCMDeviceCommissioner::OnTrustVerificationComplete(JCMTrustVerificationError error)
{
    if (error == JCMTrustVerificationError::kSuccess)
    {
        ChipLogProgress(Controller, "JCM: Administrator Device passed JCM Trust Verification");

        CommissioningStageComplete(CHIP_NO_ERROR);
    }
    else
    {
        ChipLogError(Controller, "JCM: Failed in verifying JCM Trust Verification: err %s", EnumToString(error).c_str());

        CommissioningDelegate::CommissioningReport report;
        report.Set<JCMTrustVerificationError>(error);

        CommissioningStageComplete(CHIP_ERROR_INTERNAL, report);
    }
}

void JCMDeviceCommissioner::CleanupCommissioning(DeviceProxy * proxy, NodeId nodeId, const CompletionStatus & completionStatus)
{
    DeviceCommissioner::CleanupCommissioning(proxy, nodeId, completionStatus);

    mInfo.Cleanup();
}

/*
 * JCMAutoCommissioner override implementation
 */
CHIP_ERROR JCMAutoCommissioner::SetCommissioningParameters(const CommissioningParameters & params)
{
    ReturnErrorOnFailure(AutoCommissioner::SetCommissioningParameters(params));

    if (params.GetUseJCM().ValueOr(false))
    {
        auto extraReadPaths = params.GetExtraReadPaths();

        mTempReadPaths.clear();
        mTempReadPaths.reserve(extraReadPaths.size() + mExtraReadPaths.size());
        mTempReadPaths.insert(mTempReadPaths.end(), extraReadPaths.begin(), extraReadPaths.end());
        mTempReadPaths.insert(mTempReadPaths.end(), mExtraReadPaths.begin(), mExtraReadPaths.end());

        // Set the extra read paths for JCM
        mParams.SetExtraReadPaths(Span<app::AttributePathParams>(mTempReadPaths.data(), mTempReadPaths.size()));
    }

    return CHIP_NO_ERROR;
}

void JCMAutoCommissioner::CleanupCommissioning()
{
    mTempReadPaths.clear();

    AutoCommissioner::CleanupCommissioning();
}

} // namespace JCM
} // namespace Controller
} // namespace chip
