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
using namespace Credentials::JCM;

namespace chip {
namespace app {
namespace Clusters {
namespace JointFabricAdministrator {

/*
 * DeviceCommissioner public interface and override implementation
 */
CHIP_ERROR JCMCommissionee::VerifyTrustAgainstCommissionerAdmin()
{
    StartTrustVerification();
    return CHIP_NO_ERROR;
}

CHIP_ERROR JCMCommissionee::OnLookupOperationalTrustAnchor(VendorId vendorID, Credentials::CertificateKeyId & subjectKeyId,
                                                           ByteSpan & globallyTrustedRootSpan)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Perform DCL Lookup https://zigbee-alliance.github.io/distributed-compliance-ledger/#/Query/NocCertificatesByVidAndSkid
    // temporarily return the already known remote admin trusted root certificate
    globallyTrustedRootSpan = mInfo.adminRCAC.Span();

    return err;
}

void JCMCommissionee::OnVendorIdVerficationComplete(const CHIP_ERROR & err)
{
    TrustVerificationError result =
        (err == CHIP_NO_ERROR) ? TrustVerificationError::kSuccess : TrustVerificationError::kVendorIdVerificationFailed;
    TrustVerificationStageFinished(kPerformingVendorIDVerification, result);
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
#if CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
    if (mInfo.adminEndpointId == kInvalidEndpointId)
    {
        return TrustVerificationError::kInvalidAdministratorEndpointId;
    }
    Server::GetInstance().GetJointFabricAdministrator().SetPeerJFAdminClusterEndpointId(mInfo.adminEndpointId);
    return TrustVerificationError::kSuccess;
#else
    return TrustVerificationError::kInternalError;
#endif // CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
}

TrustVerificationError JCMCommissionee::ReadCommissionerAdminFabricIndex()
{
    using Attr = chip::app::Clusters::JointFabricAdministrator::Attributes::AdministratorFabricIndex::TypeInfo;

    auto onSuccess = [this](const ConcreteAttributePath & path, const Attr::DecodableType & val) {
        if (val.IsNull())
        {
            ChipLogError(Controller, "JCM: Failed to read commissioner's AdministratorFabricIndex: received null");
            this->TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex,
                                                 TrustVerificationError::kReadAdminFabricIndexFailed);
        }
        else
        {
            mInfo.adminFabricIndex = val.Value();
            ChipLogProgress(Controller, "JCM: Successfully read commissioner's AdministratorFabricIndex");
            this->TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex, TrustVerificationError::kSuccess);
        }
    };

    auto onError = [this](const ConcreteAttributePath *, CHIP_ERROR err) {
        ChipLogError(Controller, "JCM: Failed to read commissioner's AdministratorFabricIndex: %" CHIP_ERROR_FORMAT, err.Format());
        this->TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex,
                                             TrustVerificationError::kReadAdminFabricIndexFailed);
    };

    Messaging::ExchangeContext * exchangeContext = mCommandHandle.Get()->GetExchangeContext();
    SessionHandle session                        = exchangeContext->GetSessionHandle();

    chip::Messaging::ExchangeManager * exchangeMgr = &chip::Server::GetInstance().GetExchangeManager();
    CHIP_ERROR err = chip::Controller::ReadAttribute<Attr>(exchangeMgr, session, mInfo.adminEndpointId, onSuccess, onError,
                                                           /*fabricFiltered=*/true);

    if (err == CHIP_NO_ERROR)
    {
        return TrustVerificationError::kAsync;
    }

    return TrustVerificationError::kReadAdminFabricIndexFailed;
}

TrustVerificationError JCMCommissionee::PerformVendorIdVerification()
{
    TrustVerificationError parseError = this->ParseCommissionerAdminInfo();
    if (parseError != TrustVerificationError::kSuccess)
    {
        ChipLogError(Controller, "JCM: Failed to parse commissioner administrator info");
        return parseError;
    }

    chip::Messaging::ExchangeManager * exchangeMgr = &chip::Server::GetInstance().GetExchangeManager();

    auto sessionHandleGetter = [this]() -> Optional<SessionHandle> {
        Messaging::ExchangeContext * ec = this->mCommandHandle.Get()->GetExchangeContext();
        if (ec == nullptr)
        {
            return Optional<SessionHandle>::Missing();
        }
        return MakeOptional(ec->GetSessionHandle());
    };

    // Kick off async vendor ID verification
    CHIP_ERROR err = VerifyVendorId(exchangeMgr, sessionHandleGetter, &mInfo);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JCM: Failed to start VendorId verification: %s", ErrorStr(err));
        OnVendorIdVerficationComplete(err);
        return TrustVerificationError::kVendorIdVerificationFailed;
    }

    return TrustVerificationError::kAsync;
}

TrustVerificationError JCMCommissionee::CrossCheckAdministratorIds()
{
    const FabricTable & fabricTable = Server::GetInstance().GetFabricTable();

    // Get accessing fabric from the current command
    const FabricIndex accessingFabricIndex = mCommandHandle.Get()->GetAccessingFabricIndex();
    const FabricInfo * accessingFabricInfo = fabricTable.FindFabricWithIndex(accessingFabricIndex);
    VerifyOrReturnError(accessingFabricInfo != nullptr, TrustVerificationError::kInternalError);

    const FabricId accessingFabricId = accessingFabricInfo->GetFabricId();
    if (accessingFabricId != mInfo.adminFabricId)
    {
        ChipLogError(Controller, "JCM: Accessing FabricID does not match AdministratorFabricID");
        return TrustVerificationError::kAdministratorIdMismatched;
    }

    Crypto::P256PublicKey accessingRootPubKey;
    CHIP_ERROR err = accessingFabricInfo->FetchRootPubkey(accessingRootPubKey);
    if (err != CHIP_NO_ERROR || accessingRootPubKey.Length() != Crypto::kP256_PublicKey_Length ||
        mInfo.rootPublicKey.AllocatedSize() != Crypto::kP256_PublicKey_Length)
    {
        ChipLogError(Controller, "JCM: Unable to fetch or size-mismatch accessing RootPublicKey");
        return TrustVerificationError::kInternalError;
    }

    if (memcmp(mInfo.rootPublicKey.Get(), accessingRootPubKey.ConstBytes(), Crypto::kP256_PublicKey_Length) != 0)
    {
        ChipLogError(Controller, "JCM: Accessing RootPublicKey does not match Administrator RootPublicKey");
        return TrustVerificationError::kAdministratorIdMismatched;
    }

    return TrustVerificationError::kSuccess;
}

TrustVerificationError JCMCommissionee::ParseCommissionerAdminInfo()
{
    const FabricIndex fabricIndex = mInfo.adminFabricIndex;

    // Populate mInfo from local FabricTable for the target fabric
    const FabricTable & fabricTable = Server::GetInstance().GetFabricTable();
    const FabricInfo * fabricInfo   = fabricTable.FindFabricWithIndex(fabricIndex);
    VerifyOrReturnError(fabricInfo != nullptr, TrustVerificationError::kInvalidAdministratorFabricIndex);

    mInfo.adminFabricIndex = fabricIndex;
    mInfo.adminVendorId    = fabricInfo->GetVendorId();
    mInfo.adminFabricId    = fabricInfo->GetFabricId();

    // Fetch root public key
    Crypto::P256PublicKey rootPubKey;
    if (fabricInfo->FetchRootPubkey(rootPubKey) != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JCM: Failed to fetch fabric root key");
        return TrustVerificationError::kInternalError;
    }
    if (rootPubKey.Length() != Crypto::kP256_PublicKey_Length)
    {
        ChipLogError(Controller, "JCM: Fabric root key size mismatch");
        return TrustVerificationError::kInternalError;
    }
    const chip::ByteSpan keySpan(rootPubKey.ConstBytes(), rootPubKey.Length());
    mInfo.rootPublicKey.CopyFromSpan(keySpan);

    // Fetch RCAC/ICAC/NOC from the fabric table for this fabric
    uint8_t rcacBuf[Credentials::kMaxCHIPCertLength];
    MutableByteSpan rcacSpan{ rcacBuf };
    uint8_t icacBuf[Credentials::kMaxCHIPCertLength];
    MutableByteSpan icacSpan{ icacBuf };
    uint8_t nocBuf[Credentials::kMaxCHIPCertLength];
    MutableByteSpan nocSpan{ nocBuf };

    if (fabricTable.FetchRootCert(fabricIndex, rcacSpan) != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JCM: Failed to fetch commissioner root cert");
        return TrustVerificationError::kVendorIdVerificationFailed;
    }
    if (fabricTable.FetchICACert(fabricIndex, icacSpan) != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JCM: Failed to fetch commissioner ICAC");
        return TrustVerificationError::kVendorIdVerificationFailed;
    }
    if (fabricTable.FetchNOCCert(fabricIndex, nocSpan) != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JCM: Failed to fetch commissioner NOC");
        return TrustVerificationError::kVendorIdVerificationFailed;
    }

    // Copy certs into mInfo buffers
    if (!mInfo.adminRCAC.Alloc(rcacSpan.size()) || !mInfo.adminICAC.Alloc(icacSpan.size()) || !mInfo.adminNOC.Alloc(nocSpan.size()))
    {
        return TrustVerificationError::kInternalError;
    }
    memcpy(mInfo.adminRCAC.Get(), rcacSpan.data(), rcacSpan.size());
    memcpy(mInfo.adminICAC.Get(), icacSpan.data(), icacSpan.size());
    memcpy(mInfo.adminNOC.Get(), nocSpan.data(), nocSpan.size());
    return TrustVerificationError::kSuccess;
}

} // namespace JointFabricAdministrator
} // namespace Clusters
} // namespace app
} // namespace chip
