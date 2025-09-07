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

#include "JCMCommissionee.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributePathParams.h>
#include <app/CommandHandlerInterface.h>
#include <app/InteractionModelEngine.h>
#include <app/ReadClient.h>
#include <app/ReadPrepareParams.h>
#include <app/server/Server.h>
#include <controller/ReadInteraction.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>

using namespace ::chip;
using namespace ::chip::app;

namespace chip {
namespace Controller {
namespace JCM {

/*
 * DeviceCommissioner public interface and override implementation
 */
CHIP_ERROR JCMCommissionee::VerifyTrustAgainstCommissionerAdmin()
{
    // FabricTable & fabricTable                      = Server::GetInstance().GetFabricTable();
    // chip::Messaging::ExchangeManager * exchangeMgr = &chip:::GetInstance().GetExchangeManager();
    // app::CommandHandler::Handle commandHandle(&ctx.mCommandHandler);
    // auto * ec = ctx.mCommandHandler.GetExchangeContext();
    // VerifyOrReturnError(ec != nullptr, CHIP_ERROR_INCORRECT_STATE);
    // SessionHandle sessionHandle = ec->GetSessionHandle();

    // // Get Accessing Fabric IDs
    // FabricIndex accessingFabricIndex       = ctx.mCommandHandler.GetAccessingFabricIndex();
    // const FabricInfo * accessingFabricInfo = fabricTable.FindFabricWithIndex(accessingFabricIndex);
    // Crypto::P256PublicKey accessingRootPubKey;
    // CHIP_ERROR err             = accessingFabricInfo->FetchRootPubkey(accessingRootPubKey);
    // FabricId accessingFabricId = accessingFabricInfo->GetFabricId();

    // using Attr     = chip::app::Clusters::JointFabricAdministrator::Attributes::AdministratorFabricIndex::TypeInfo;
    // auto onSuccess = [](const ConcreteAttributePath &, const Attr::DecodableType & val) { /* TODO use val */ };
    // auto onError   = [](const ConcreteAttributePath *, CHIP_ERROR err) { /* TODO handle err */ };

    // // ReturnErrorOnFailure(chip::Controller::ReadAttribute<Attr::DecodableType>(
    // //     exchangeMgr, sessionHandle, endpointId, Clusters::JointFabricAdministrator::Id, Attr::Id, onSuccess, onError,
    // //     /*fabricFiltered=*/true));
    // ReturnErrorOnFailure(chip::Controller::ReadAttribute<Attr>(exchangeMgr, sessionHandle, endpointId, onSuccess, onError,
    //                                                            /*fabricFiltered=*/true));

    // TODO: Check that the RootPublicKey and FabricID of the accessing fabric (found in the FabricDescriptorStruct) match the
    // RootPublicKey and FabricID of the Fabric indicated by AdministratorFabricIndex.

    // TODO: Figure out where I get the vendor ID, rcacSpan, icacSpan, and nocSpan
    mOnCompletion(CHIP_NO_ERROR);

    // CHIP_ERROR err = VerifyVendorId(exchangeMgr, sessionHandleGetter, accessingFabricIndex, vendorId, rcacSpan, icacSpan,
    // nocSpan);

    return CHIP_NO_ERROR;
}

CHIP_ERROR JCMCommissionee::OnLookupOperationalTrustAnchor(VendorId vendorID, CertificateKeyId & subjectKeyId,
                                                           ByteSpan & globallyTrustedRootSpan)
{
    // TODO I don't think any action needs to be taken here

    return CHIP_NO_ERROR;
}

void JCMCommissionee::OnVendorIdVerficationComplete(const CHIP_ERROR & err)
{
    // TODO I don't think any action needs to be taken here
}

TrustVerificationStage JCMCommissionee::GetNextTrustVerificationStage(const TrustVerificationStage & currentStage)
{
    TrustVerificationStage nextStage = TrustVerificationStage::kIdle;

    switch (currentStage)
    {
    case TrustVerificationStage::kIdle:
        nextStage = TrustVerificationStage::kStoringEndpointID;
        break;
    case TrustVerificationStage::kStoringEndpointID:
        nextStage = TrustVerificationStage::kReadingCommissionerAdminFabricIndex;
        break;
    case TrustVerificationStage::kReadingCommissionerAdminFabricIndex:
        nextStage = TrustVerificationStage::kPerformingVendorIDVerification;
        break;
    case TrustVerificationStage::kPerformingVendorIDVerification:
        nextStage = TrustVerificationStage::kCrossCheckingAdministratorIds;
        break;
    case TrustVerificationStage::kCrossCheckingAdministratorIds:
        nextStage = TrustVerificationStage::kComplete;
        break;
    default:
        ChipLogError(Controller, "JCM: Invalid stage: %d", static_cast<int>(currentStage));
        nextStage = TrustVerificationStage::kError;
        break;
    }

    return nextStage;
}

void JCMCommissionee::PerformTrustVerificationStage(const TrustVerificationStage & nextStage)
{
    TrustVerificationError error = TrustVerificationError::kSuccess;

    switch (nextStage)
    {
    case TrustVerificationStage::kStoringEndpointID:
        error = StoreEndpointId();
        break;
    case TrustVerificationStage::kReadingCommissionerAdminFabricIndex:
        error = ReadCommissionerAdminFabricIndex();
        break;
    case TrustVerificationStage::kPerformingVendorIDVerification:
        error = PerformVendorIdVerification();
        break;
    case TrustVerificationStage::kCrossCheckingAdministratorIds:
        error = CrossCheckAdministratorIds();
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

void JCMCommissionee::OnTrustVerificationComplete(TrustVerificationError error)
{
    if (error == TrustVerificationError::kSuccess)
    {
        ChipLogProgress(Controller, "JCM: Administrator Device passed JCM Trust Verification");

        mOnCompletion(CHIP_NO_ERROR);
    }
    else
    {
        ChipLogError(Controller, "JCM: Failed in verifying JCM Trust Verification: err %s", EnumToString(error).c_str());

        mOnCompletion(CHIP_ERROR_INTERNAL);
    }
}

TrustVerificationError JCMCommissionee::StoreEndpointId()
{
    if (mInfo.adminEndpointId == kInvalidEndpointId)
    {
        return TrustVerificationError::kInvalidAdministratorEndpointId;
    }
    Server::GetInstance().GetJointFabricAdministrator().SetPeerJFAdminClusterEndpointId(mInfo.adminEndpointId);
    return TrustVerificationError::kSuccess;
}

TrustVerificationError JCMCommissionee::ReadCommissionerAdminFabricIndex()
{
    using Attr     = chip::app::Clusters::JointFabricAdministrator::Attributes::AdministratorFabricIndex::TypeInfo;
    auto onSuccess = [this](const ConcreteAttributePath &, const Attr::DecodableType & val) {
        this->TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex, TrustVerificationError::kSuccess);
    };
    auto onError = [this](const ConcreteAttributePath *, CHIP_ERROR err) {
        this->TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex,
                                             TrustVerificationError::kReadAdminFabricIndexFailed);
    };

    ExchangeContext * exchangeContext = mCommandHandle.Get()->GetExchangeContext();
    SessionHandle session             = exchangeContext->GetSessionHandle();

    chip::Messaging::ExchangeManager * exchangeMgr = &chip::Server::GetInstance().GetExchangeManager();
    CHIP_ERROR err = chip::Controller::ReadAttribute<Attr>(exchangeMgr, session, mInfo.adminEndpointId, onSuccess, onError,
                                                           /*fabricFiltered=*/true);

    if (err == CHIP_NO_ERROR)
    {
        return TrustVerificationError::kAsync;
    }
    else
    {
        return TrustVerificationError::kReadAdminFabricIndexFailed;
    }
}

TrustVerificationError JCMCommissionee::PerformVendorIdVerification()
{
    // TODO
    return TrustVerificationError::kSuccess;
}

TrustVerificationError JCMCommissionee::CrossCheckAdministratorIds()
{
    // TODO
    // FabricTable & fabricTable              = Server::GetInstance().GetFabricTable();
    // FabricIndex accessingFabricIndex       = mCommandHandle.Get()->GetAccessingFabricIndex();
    // const FabricInfo * accessingFabricInfo = fabricTable.FindFabricWithIndex(accessingFabricIndex);
    // Crypto::P256PublicKey accessingRootPubKey;
    // CHIP_ERROR err             = accessingFabricInfo->FetchRootPubkey(accessingRootPubKey);
    // FabricId accessingFabricId = accessingFabricInfo->GetFabricId();
    return TrustVerificationError::kSuccess;
}

} // namespace JCM
} // namespace Controller
} // namespace chip
