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
#include "JCMTrustVerification.h"
#include <controller/JCMCommissioner.h>
#include <controller/CommissioningDelegate.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/InteractionModelEngine.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>
#include <controller/JCMCommissioner.h>
#include <credentials/CHIPCert.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace chip::app::Clusters;

namespace chip {
namespace Controller {

/*
 * JCMDeviceCommissioner public interface and override implementation    
 */
CHIP_ERROR JCMDeviceCommissioner::StartJCMTrustVerification(DeviceProxy * proxy)
{
    ChipLogProgress(Controller, "JCM: Starting Trust Verification");

    VerifyOrReturnError(proxy != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mNextStage = JCMTrustVerificationStage::kStarted;
    mDeviceProxy = proxy;

    AdvanceTrustVerificationStage(JCMTrustVerificationResult::kSuccess);
    
    return CHIP_NO_ERROR;  
}

void JCMDeviceCommissioner::OnTrustVerificationComplete(JCMTrustVerificationResult result)
{
    if (result == JCMTrustVerificationResult::kSuccess)
    {
        ChipLogProgress(Controller, "JCM: Administrator Device passed JCM Trust Verification");

        CommissioningStageComplete(CHIP_NO_ERROR);
    }
    else
    {
        ChipLogError(Controller, "JCM: Failed in verifying 'JCM Trust Verification': err %hu",
                     static_cast<uint16_t>(result));

        CommissioningDelegate::CommissioningReport report;
        report.Set<JCMTrustVerificationError>(result);

        CommissioningStageComplete(CHIP_ERROR_INTERNAL, report);
    }
}

void JCMDeviceCommissioner::ContinueAfterUserConsent(bool consent)
{
    if (consent)
    {
        ChipLogProgress(Controller, "JCM: User consent granted");
        AdvanceTrustVerificationStage(JCMTrustVerificationResult::kSuccess);
    }
    else
    {
        ChipLogError(Controller, "JCM: User denied consent");
        AdvanceTrustVerificationStage(JCMTrustVerificationResult::KUserDeniedConsent);
    }
}

CHIP_ERROR JCMDeviceCommissioner::ParseAdminFabricIndexAndEndpointId(ReadCommissioningInfo & info)
{
    auto attributeCache = info.attributes;
    
    CHIP_ERROR err = attributeCache->ForEachAttribute(Clusters::JointFabricAdministrator::Id, [this, &attributeCache](const ConcreteAttributePath & path) {
        using namespace Clusters::JointFabricAdministrator::Attributes;
        AdministratorFabricIndex::TypeInfo::DecodableType administratorFabricIndex;

        VerifyOrReturnError(path.mAttributeId == AdministratorFabricIndex::Id, CHIP_NO_ERROR);
        ReturnErrorOnFailure(attributeCache->Get<AdministratorFabricIndex::TypeInfo>(path, administratorFabricIndex));

        if (!administratorFabricIndex.IsNull() && administratorFabricIndex.Value() != kUndefinedFabricIndex)
        {
            ChipLogProgress(Controller, "JCM: AdministratorFabricIndex: %d", administratorFabricIndex.Value());
            mInfo.adminFabricIndex = administratorFabricIndex.Value();
            mInfo.adminEndpointId = path.mEndpointId;
        }
        else
        {
            ChipLogError(Controller, "JCM: JF Administrator Cluster not found!");
            return CHIP_ERROR_NOT_FOUND;
        }

        return CHIP_NO_ERROR;
    });

    return err;
}

CHIP_ERROR JCMDeviceCommissioner::ParseOperationalCredentials(ReadCommissioningInfo & info)
{
    auto attributeCache = info.attributes;

    CHIP_ERROR err = attributeCache->ForEachAttribute(OperationalCredentials::Id, [this, &attributeCache](const ConcreteAttributePath & path) {
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
                    if (fabricDescriptor.fabricIndex != kUndefinedFabricIndex)
                    {
                        if (fabricDescriptor.rootPublicKey.size() != Crypto::kP256_PublicKey_Length)
                        {
                            ChipLogError(Controller, "JCM: DeviceCommissioner::ParseJFAdministratorInfo - fabric root key size mismatch");
                            return CHIP_ERROR_KEY_NOT_FOUND;
                        }

                        mInfo.rootKeySpan = fabricDescriptor.rootPublicKey;
                        mInfo.adminVendorId = fabricDescriptor.vendorID;
                        mInfo.adminFabricId = fabricDescriptor.fabricID;

                        if (fabricDescriptor.VIDVerificationStatement.HasValue())
                        {
                            ChipLogError(Controller, "JCM: Per-home RCAC are not supported by JF for now!");
                            return CHIP_ERROR_CANCELLED;
                        }
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
                        if (!mInfo.adminNOC.Alloc(nocStruct.noc.size()))
                        {
                            ChipLogError(Controller, "JCM: DeviceCommissioner::ParseJFAdministratorInfo - cannot allocate memory for admin noc");
                            return CHIP_ERROR_NO_MEMORY;
                        }

                        memcpy(mInfo.adminNOC.Get(), nocStruct.noc.data(), nocStruct.noc.size());

                        if (!nocStruct.icac.IsNull())
                        {
                            auto icac = nocStruct.icac.Value();
                            if (!mInfo.adminICAC.Alloc(icac.size()))
                            {
                                ChipLogError(Controller, "JCM: DeviceCommissioner::ParseJFAdministratorInfo - cannot allocate memory for admin icac");
                                return CHIP_ERROR_NO_MEMORY;
                            }
    
                            memcpy(mInfo.adminICAC.Get(), icac.data(), icac.size());
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

    return err;
}

CHIP_ERROR JCMDeviceCommissioner::ParseTrustedRoot(ReadCommissioningInfo & info)
{
    auto attributeCache = info.attributes;

    CHIP_ERROR err = attributeCache->ForEachAttribute(OperationalCredentials::Id, [this, &attributeCache](const ConcreteAttributePath & path) {
        using namespace chip::app::Clusters::OperationalCredentials::Attributes;
        bool foundMatchingRcac = false;

        switch (path.mAttributeId)
        {
            case TrustedRootCertificates::Id: {
                TrustedRootCertificates::TypeInfo::DecodableType trustedCAs;
                ReturnErrorOnFailure(attributeCache->Get<TrustedRootCertificates::TypeInfo>(path, trustedCAs));

                   auto iter = trustedCAs.begin();
                   while (iter.Next())
                   {
                       auto & trustedCA = iter.GetValue();
                       Credentials::P256PublicKeySpan trustedCAPublicKeySpan;

                       ReturnErrorOnFailure(Credentials::ExtractPublicKeyFromChipCert(trustedCA, trustedCAPublicKeySpan));
                       Crypto::P256PublicKey trustedCAPublicKey{ trustedCAPublicKeySpan };

                       if (mInfo.rootKeySpan.size() != Crypto::kP256_PublicKey_Length)
                       {
                           ChipLogError(Controller, "JCM: DeviceCommissioner::ParseJFAdministratorInfo - fabric root key size mismatch");
                           return CHIP_ERROR_KEY_NOT_FOUND;
                       }

                       Credentials::P256PublicKeySpan rootPubKeySpan(mInfo.rootKeySpan.data());
                       Crypto::P256PublicKey fabricTableRootPublicKey{ rootPubKeySpan };

                       if (trustedCAPublicKey.Matches(fabricTableRootPublicKey) && trustedCA.size())
                       {
                            if (!mInfo.adminRCAC.Alloc(trustedCA.size()))
                            {
                                ChipLogError(Controller, "JCM: DeviceCommissioner::ParseJFAdministratorInfo - cannot allocate memory for admin rcac");
                                return CHIP_ERROR_NO_MEMORY;
                            }

                            memcpy(mInfo.adminRCAC.Get(), trustedCA.data(), trustedCA.size());
                            ChipLogProgress(Controller, "JCM: Successfully parsed the Administrator RCAC");
                            foundMatchingRcac = true;
                            break;
                       }
                   }
                   if (!foundMatchingRcac)
                   {
                       ChipLogError(Controller, "JCM: Cannot find a matching RCAC!");
                       return CHIP_ERROR_CERT_NOT_FOUND;
                   }
                   return CHIP_NO_ERROR;
               }
               default:
                   return CHIP_NO_ERROR;
           }
           return CHIP_NO_ERROR;
    });

    return err;
}

CHIP_ERROR JCMDeviceCommissioner::ParseExtraCommissioningInfo(ReadCommissioningInfo & info)
{
    using namespace OperationalCredentials::Attributes;

    CHIP_ERROR err = CHIP_NO_ERROR;

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
        ChipLogError(Controller, "JCM: Failed to find Operational Credentials");
        return err;
    }

    return err;
}

void JCMDeviceCommissioner::VerifyAdministratorEndpointAndFabricIndex()
{
    ChipLogProgress(Controller, "JCM: Verify joint fabric administrator endpoint and fabric index");

    if (mInfo.adminEndpointId == kInvalidEndpointId)
    {
        ChipLogError(Controller, "JCM: Administrator endpoint ID not found!");
        AdvanceTrustVerificationStage(JCMTrustVerificationResult::kJoineeNotAnAdministrator);
        return;
    }
    if (mInfo.adminFabricIndex == kUndefinedFabricIndex)
    {
        ChipLogError(Controller, "JCM: Administrator fabric index not found!");
        AdvanceTrustVerificationStage(JCMTrustVerificationResult::kJoineeNotAnAdministrator);
        return;
    }

    ChipLogProgress(Controller, "JCM: Administrator endpoint ID: %d", mInfo.adminEndpointId);
    ChipLogProgress(Controller, "JCM: Administrator fabric index: %d", mInfo.adminFabricIndex);
    ChipLogProgress(Controller, "JCM: Administrator vendor ID: %d", mInfo.adminVendorId);
    ChipLogProgress(Controller, "JCM: Administrator fabric ID: %ld", mInfo.adminFabricId);

    AdvanceTrustVerificationStage(JCMTrustVerificationResult::kSuccess);
}

void JCMDeviceCommissioner::PerformVendorIDVerificationProcedure()
{
    ChipLogProgress(Controller, "Performing Vendor ID Verification Procedure");

    // TODO: Implement the Vendor ID verification procedure

    AdvanceTrustVerificationStage(JCMTrustVerificationResult::kSuccess);
}

void JCMDeviceCommissioner::VerifyNOCContainsAdministratorCAT()
{
    ChipLogProgress(Controller, "JCM: Verifying NOC contains Administrator CAT");

    // TODO: Implement the verification of NOC containing Administrator CAT

    AdvanceTrustVerificationStage(JCMTrustVerificationResult::kSuccess);
}

void JCMDeviceCommissioner::AskUserForConsent()
{
    ChipLogProgress(Controller, "JCM: Asking user for consent");
    if (mTrustVerificationDelegate != nullptr)
    {
        VendorId vendorId = static_cast<VendorId>(mInfo.adminVendorId);
        mTrustVerificationDelegate->OnAskUserForConsent(*this, vendorId);
    } else {
        ChipLogError(Controller, "JCM: TrustVerificationDelegate is not set");
        AdvanceTrustVerificationStage(JCMTrustVerificationResult::kTrustVerificationDelegateNotSet);
    }
}

void JCMDeviceCommissioner::AdvanceTrustVerificationStage(JCMTrustVerificationResult result)
{
    if (mTrustVerificationDelegate != nullptr)
    {
        mTrustVerificationDelegate->OnProgressUpdate(*this, mNextStage, result);
    }
    
    if (result != JCMTrustVerificationResult::kSuccess)
    {
        // Handle error
        ChipLogError(Controller, "JCM: Error in Trust Verification: %d", static_cast<int>(result));
        OnTrustVerificationComplete(result);
        return;
    }

    switch (mNextStage)
    {
        case chip::Controller::JCMTrustVerificationStage::kStarted:
            mNextStage = JCMTrustVerificationStage::kVerifyingAdministratorEndpointAndFabricIndex;
            VerifyAdministratorEndpointAndFabricIndex();
            break;
        case JCMTrustVerificationStage::kVerifyingAdministratorEndpointAndFabricIndex:
            mNextStage = JCMTrustVerificationStage::kPerformingVendorIDVerificationProcedure;
            PerformVendorIDVerificationProcedure();
            break;
        case JCMTrustVerificationStage::kPerformingVendorIDVerificationProcedure:
            mNextStage = JCMTrustVerificationStage::kVerifyingNOCContainsAdministratorCAT;
            VerifyNOCContainsAdministratorCAT();
            break;
        case JCMTrustVerificationStage::kVerifyingNOCContainsAdministratorCAT:
            mNextStage = JCMTrustVerificationStage::kAskingUserForConsent;
            AskUserForConsent();
            break;
        case JCMTrustVerificationStage::kAskingUserForConsent:
            mNextStage = JCMTrustVerificationStage::kIdle;
            OnTrustVerificationComplete(result);
            break;
        default:
            ChipLogError(Controller, "JCM: Invalid stage: %d", static_cast<int>(mNextStage));
            OnTrustVerificationComplete(JCMTrustVerificationResult::kInternalError);
            break;
    }
}

void JCMDeviceCommissioner::CleanupCommissioning(DeviceProxy * proxy, NodeId nodeId, const CompletionStatus & completionStatus)
{
    mNextStage = JCMTrustVerificationStage::kIdle;
    mInfo.clear();

    DeviceCommissioner::CleanupCommissioning(proxy, nodeId, completionStatus);
}

/*
 * JCMAutoCommissioner override implementation
*/
CHIP_ERROR JCMAutoCommissioner::SetCommissioningParameters(const CommissioningParameters & params)
{
    ReturnErrorOnFailure(AutoCommissioner::SetCommissioningParameters(params));

    if (params.UseJCM().ValueOr(false)) {
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

} // namespace Controller
} // namespace chip
