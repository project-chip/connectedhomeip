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
#include <lib/support/logging/BinaryLogging.h>

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
        ChipLogError(JointFabric, "JCM: Invalid stage: %d", static_cast<int>(currentStage));
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
    case TrustVerificationStage::kComplete:
        error = TrustVerificationError::kSuccess;
        break;
    default:
        ChipLogError(JointFabric, "JCM: Invalid stage: %d", static_cast<int>(nextStage));
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
        ChipLogProgress(JointFabric, "JCM: Administrator Device passed JCM Trust Verification");

        mOnCompletion(CHIP_NO_ERROR);
    }
    else
    {
        ChipLogError(JointFabric, "JCM: Failed in verifying JCM Trust Verification: err %s", EnumToString(error).c_str());

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

    mLocalAdminFabricIndex = kUndefinedFabricIndex;

    auto onSuccess = [this](const ConcreteAttributePath & path, const Attr::DecodableType & val) {
        if (val.IsNull())
        {
            ChipLogError(JointFabric, "JCM: Failed to read commissioner's AdministratorFabricIndex: received null");
            this->TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex,
                                                 TrustVerificationError::kReadAdminFabricIndexFailed);
        }
        else
        {
            FabricIndex remoteFabricIndex = static_cast<FabricIndex>(val.Value());
            if (!IsValidFabricIndex(remoteFabricIndex))
            {
                ChipLogError(JointFabric, "JCM: AdministratorFabricIndex %u is out of range",
                             static_cast<unsigned>(remoteFabricIndex));
                this->TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex,
                                                     TrustVerificationError::kInvalidAdministratorFabricIndex);
                return;
            }
            mInfo.adminFabricIndex = remoteFabricIndex;
            ChipLogProgress(JointFabric, "JCM: Commissioner reports AdministratorFabricIndex=%u; resolving local fabric entry",
                            static_cast<unsigned>(remoteFabricIndex));
            this->FetchAdministratorFabricDescriptor(remoteFabricIndex);
        }
    };

    auto onError = [this](const ConcreteAttributePath *, CHIP_ERROR err) {
        ChipLogError(JointFabric, "JCM: Failed to read commissioner's AdministratorFabricIndex: %" CHIP_ERROR_FORMAT, err.Format());
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

void JCMCommissionee::FetchAdministratorFabricDescriptor(FabricIndex remoteFabricIndex)
{
    using FabricsAttr                            = chip::app::Clusters::OperationalCredentials::Attributes::Fabrics::TypeInfo;
    Messaging::ExchangeContext * exchangeContext = mCommandHandle.Get()->GetExchangeContext();
    if (exchangeContext == nullptr)
    {
        ChipLogError(JointFabric, "JCM: Exchange context is no longer valid while resolving AdministratorFabricIndex");
        TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex, TrustVerificationError::kReadAdminFabricIndexFailed);
        return;
    }
    SessionHandle session = exchangeContext->GetSessionHandle();
    auto onSuccess        = [this, remoteFabricIndex](const ConcreteAttributePath &, const FabricsAttr::DecodableType & fabrics) {
        TrustVerificationError error = TrustVerificationError::kInvalidAdministratorFabricIndex;
        auto iter                    = fabrics.begin();
        while (iter.Next())
        {
            const auto & fabricDescriptor = iter.GetValue();
            if (fabricDescriptor.fabricIndex == remoteFabricIndex)
            {
                error = PopulateLocalAdminFabricInfo(remoteFabricIndex, fabricDescriptor);
                break;
            }
        }
        if ((error == TrustVerificationError::kInvalidAdministratorFabricIndex) && (iter.GetStatus() != CHIP_NO_ERROR))
        {
            ChipLogError(JointFabric, "JCM: Failed to decode Fabrics list while resolving AdministratorFabricIndex %u",
                                static_cast<unsigned>(remoteFabricIndex));
            error = TrustVerificationError::kInternalError;
        }
        if (error == TrustVerificationError::kSuccess)
        {
            CHIP_ERROR credsErr = FetchAdministratorOperationalCredentials(remoteFabricIndex);
            if (credsErr == CHIP_NO_ERROR)
            {
                return;
            }
            ChipLogError(JointFabric, "JCM: Failed to initiate administrator credential read: %" CHIP_ERROR_FORMAT,
                                credsErr.Format());
            error = TrustVerificationError::kInternalError;
        }
        if (error != TrustVerificationError::kSuccess)
        {
            ChipLogError(JointFabric, "JCM: Unable to resolve AdministratorFabricIndex %u to a local fabric entry",
                                static_cast<unsigned>(remoteFabricIndex));
        }
        TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex, error);
    };
    auto onError = [this](const ConcreteAttributePath *, CHIP_ERROR err) {
        ChipLogError(JointFabric, "JCM: Failed to read commissioner's Fabrics list: %" CHIP_ERROR_FORMAT, err.Format());
        TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex, TrustVerificationError::kReadAdminFabricIndexFailed);
    };
    CHIP_ERROR err = chip::Controller::ReadAttribute<FabricsAttr>(&chip::Server::GetInstance().GetExchangeManager(), session,
                                                                  kRootEndpointId, onSuccess, onError,
                                                                  /*fabricFiltered=*/true);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(JointFabric, "JCM: Failed to initiate Fabrics read: %" CHIP_ERROR_FORMAT, err.Format());
        TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex, TrustVerificationError::kReadAdminFabricIndexFailed);
    }
}
Credentials::JCM::TrustVerificationError JCMCommissionee::PopulateLocalAdminFabricInfo(
    FabricIndex remoteFabricIndex,
    const chip::app::Clusters::OperationalCredentials::Structs::FabricDescriptorStruct::DecodableType & fabricDescriptor)
{
    if (fabricDescriptor.rootPublicKey.size() != Crypto::kP256_PublicKey_Length)
    {
        ChipLogError(JointFabric, "JCM: Root public key size mismatch while resolving AdministratorFabricIndex %u",
                     static_cast<unsigned>(remoteFabricIndex));
        return TrustVerificationError::kInternalError;
    }
    Crypto::P256PublicKey remoteRootPublicKey;
    memcpy(remoteRootPublicKey.Bytes(), fabricDescriptor.rootPublicKey.data(), fabricDescriptor.rootPublicKey.size());
    const FabricTable & fabricTable = Server::GetInstance().GetFabricTable();
    const FabricInfo * localFabric  = fabricTable.FindFabric(remoteRootPublicKey, fabricDescriptor.fabricID);
    if (localFabric == nullptr)
    {
        for (const auto & candidate : fabricTable)
        {
            if (candidate.GetFabricId() != fabricDescriptor.fabricID)
            {
                continue;
            }
            Crypto::P256PublicKey candidateRoot;
            if ((candidate.FetchRootPubkey(candidateRoot) == CHIP_NO_ERROR) &&
                (candidateRoot.Length() == fabricDescriptor.rootPublicKey.size()) &&
                (memcmp(candidateRoot.ConstBytes(), fabricDescriptor.rootPublicKey.data(), candidateRoot.Length()) == 0))
            {
                localFabric = &candidate;
                break;
            }
        }
    }
    if (localFabric == nullptr)
    {
        return TrustVerificationError::kInvalidAdministratorFabricIndex;
    }
    mLocalAdminFabricIndex = localFabric->GetFabricIndex();
    ChipLogProgress(JointFabric, "JCM: Administrator fabric resolved: remote=%u local=%u", static_cast<unsigned>(remoteFabricIndex),
                    static_cast<unsigned>(mLocalAdminFabricIndex));
    return TrustVerificationError::kSuccess;
}

CHIP_ERROR JCMCommissionee::FetchAdministratorOperationalCredentials(FabricIndex remoteFabricIndex)
{
    using TrustedRootAttr = chip::app::Clusters::OperationalCredentials::Attributes::TrustedRootCertificates::TypeInfo;

    Messaging::ExchangeContext * exchangeContext = mCommandHandle.Get()->GetExchangeContext();
    VerifyOrReturnError(exchangeContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    SessionHandle session = exchangeContext->GetSessionHandle();

    auto onSuccess = [this, remoteFabricIndex](const ConcreteAttributePath &, const TrustedRootAttr::DecodableType & roots) {
        TrustVerificationError result = TrustVerificationError::kInternalError;

        auto iter = roots.begin();
        if (iter.Next())
        {
            ByteSpan rootSpan = iter.GetValue();
            mInfo.adminRCAC.CopyFromSpan(rootSpan);
            if ((rootSpan.size() == 0) || (mInfo.adminRCAC.AllocatedSize() != rootSpan.size()))
            {
                ChipLogError(JointFabric, "JCM: Failed to store administrator root cert");
                result = TrustVerificationError::kInternalError;
            }
            else
            {
                ChipLogProgress(JointFabric, "JCM: Admin RCAC contents");
                ChipLogByteSpan(JointFabric, rootSpan);
                result = TrustVerificationError::kSuccess;
            }
        }
        else if (iter.GetStatus() == CHIP_NO_ERROR)
        {
            ChipLogError(JointFabric, "JCM: Trusted root list empty for AdministratorFabricIndex %u",
                         static_cast<unsigned>(remoteFabricIndex));
            result = TrustVerificationError::kInvalidAdministratorFabricIndex;
        }
        else
        {
            ChipLogError(JointFabric, "JCM: Failed to decode trusted roots while resolving AdministratorFabricIndex %u",
                         static_cast<unsigned>(remoteFabricIndex));
            result = TrustVerificationError::kInternalError;
        }

        if (result == TrustVerificationError::kSuccess)
        {
            CHIP_ERROR err = FetchAdministratorNOCs(remoteFabricIndex);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(JointFabric, "JCM: Failed to initiate commissioner's NOCs read: %" CHIP_ERROR_FORMAT, err.Format());
                TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex, TrustVerificationError::kInternalError);
            }
            return;
        }

        TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex, result);
    };

    auto onError = [this](const ConcreteAttributePath *, CHIP_ERROR err) {
        ChipLogError(JointFabric, "JCM: Failed to read commissioner's trusted roots: %" CHIP_ERROR_FORMAT, err.Format());
        TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex, TrustVerificationError::kInternalError);
    };

    return chip::Controller::ReadAttribute<TrustedRootAttr>(&chip::Server::GetInstance().GetExchangeManager(), session,
                                                            kRootEndpointId, onSuccess, onError, /*fabricFiltered=*/true);
}

CHIP_ERROR JCMCommissionee::FetchAdministratorNOCs(FabricIndex remoteFabricIndex)
{
    using NOCsAttr = chip::app::Clusters::OperationalCredentials::Attributes::NOCs::TypeInfo;

    Messaging::ExchangeContext * exchangeContext = mCommandHandle.Get()->GetExchangeContext();
    VerifyOrReturnError(exchangeContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    SessionHandle session = exchangeContext->GetSessionHandle();

    auto onSuccess = [this, remoteFabricIndex](const ConcreteAttributePath &, const NOCsAttr::DecodableType & nocs) {
        TrustVerificationError result = TrustVerificationError::kInvalidAdministratorFabricIndex;

        auto iter = nocs.begin();
        while (iter.Next())
        {
            const auto & nocStruct = iter.GetValue();

            if (nocStruct.fabricIndex == remoteFabricIndex)
            {
                ByteSpan nocSpan = nocStruct.noc;
                mInfo.adminNOC.CopyFromSpan(nocSpan);
                if ((nocSpan.size() == 0) || (mInfo.adminNOC.AllocatedSize() != nocSpan.size()))
                {
                    ChipLogError(JointFabric, "JCM: Failed to store administrator NOC");
                    result = TrustVerificationError::kInternalError;
                    break;
                }

                if (!nocStruct.icac.IsNull())
                {
                    ByteSpan icacSpan = nocStruct.icac.Value();
                    mInfo.adminICAC.CopyFromSpan(icacSpan);
                    if ((icacSpan.size() != 0) && (mInfo.adminICAC.AllocatedSize() != icacSpan.size()))
                    {
                        ChipLogError(JointFabric, "JCM: Failed to store administrator ICAC");
                        result = TrustVerificationError::kInternalError;
                        break;
                    }
                    if (icacSpan.size() != 0)
                    {
                        ChipLogProgress(JointFabric, "JCM: Admin ICAC contents");
                        ChipLogByteSpan(JointFabric, icacSpan);
                    }
                }
                else
                {
                    mInfo.adminICAC.Free();
                }

                ChipLogProgress(JointFabric, "JCM: Admin NOC contents");
                ChipLogByteSpan(JointFabric, nocSpan);

                result = TrustVerificationError::kSuccess;
                break;
            }
        }

        if (iter.GetStatus() != CHIP_NO_ERROR)
        {
            ChipLogError(JointFabric, "JCM: Failed to decode NOCs list while resolving AdministratorFabricIndex %u",
                         static_cast<unsigned>(remoteFabricIndex));
            result = TrustVerificationError::kInternalError;
        }
        else if (result != TrustVerificationError::kSuccess)
        {
            ChipLogError(JointFabric, "JCM: Unable to locate NOC entry for AdministratorFabricIndex %u",
                         static_cast<unsigned>(remoteFabricIndex));
        }

        TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex, result);
    };

    auto onError = [this](const ConcreteAttributePath *, CHIP_ERROR err) {
        ChipLogError(JointFabric, "JCM: Failed to read commissioner's NOCs list: %" CHIP_ERROR_FORMAT, err.Format());
        TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex, TrustVerificationError::kReadAdminFabricIndexFailed);
    };

    return chip::Controller::ReadAttribute<NOCsAttr>(&chip::Server::GetInstance().GetExchangeManager(), session, kRootEndpointId,
                                                     onSuccess, onError, /*fabricFiltered=*/true);
}

TrustVerificationError JCMCommissionee::PerformVendorIdVerification()
{
    TrustVerificationError parseError = this->ParseCommissionerAdminInfo();
    if (parseError != TrustVerificationError::kSuccess)
    {
        ChipLogError(JointFabric, "JCM: Failed to parse commissioner administrator info");
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
        ChipLogError(JointFabric, "JCM: Failed to start VendorId verification: %s", ErrorStr(err));
        OnVendorIdVerficationComplete(err);
        return TrustVerificationError::kVendorIdVerificationFailed;
    }

    return TrustVerificationError::kAsync;
}

TrustVerificationError JCMCommissionee::CrossCheckAdministratorIds()
{
    const FabricTable & fabricTable = Server::GetInstance().GetFabricTable();

    // Get accessing fabric from the current command
    const FabricInfo * accessingFabricInfo = fabricTable.FindFabricWithIndex(mAccessingFabricIndex);
    VerifyOrReturnError(accessingFabricInfo != nullptr, TrustVerificationError::kInternalError);

    const FabricId accessingFabricId = accessingFabricInfo->GetFabricId();
    if (accessingFabricId != mInfo.adminFabricId)
    {
        ChipLogError(JointFabric, "JCM: Accessing FabricID does not match AdministratorFabricID");
        return TrustVerificationError::kAdministratorIdMismatched;
    }

    Crypto::P256PublicKey accessingRootPubKey;
    CHIP_ERROR err = accessingFabricInfo->FetchRootPubkey(accessingRootPubKey);
    if (err != CHIP_NO_ERROR || accessingRootPubKey.Length() != Crypto::kP256_PublicKey_Length ||
        mInfo.rootPublicKey.AllocatedSize() != Crypto::kP256_PublicKey_Length)
    {
        ChipLogError(JointFabric, "JCM: Unable to fetch or size-mismatch accessing RootPublicKey");
        return TrustVerificationError::kInternalError;
    }

    if (memcmp(mInfo.rootPublicKey.Get(), accessingRootPubKey.ConstBytes(), Crypto::kP256_PublicKey_Length) != 0)
    {
        ChipLogError(JointFabric, "JCM: Accessing RootPublicKey does not match Administrator RootPublicKey");
        return TrustVerificationError::kAdministratorIdMismatched;
    }

    return TrustVerificationError::kSuccess;
}

TrustVerificationError JCMCommissionee::ParseCommissionerAdminInfo()
{
    const FabricIndex fabricIndex       = mLocalAdminFabricIndex;
    const FabricIndex remoteFabricIndex = mInfo.adminFabricIndex;
    if (!IsValidFabricIndex(fabricIndex))
    {
        ChipLogError(JointFabric, "JCM: Local administrator fabric index not resolved");
        return TrustVerificationError::kInvalidAdministratorFabricIndex;
    }

    // Populate mInfo from local FabricTable for the target fabric
    const FabricTable & fabricTable = Server::GetInstance().GetFabricTable();
    const FabricInfo * fabricInfo   = fabricTable.FindFabricWithIndex(fabricIndex);
    VerifyOrReturnError(fabricInfo != nullptr, TrustVerificationError::kInvalidAdministratorFabricIndex);

    mInfo.adminVendorId = fabricInfo->GetVendorId();
    mInfo.adminFabricId = fabricInfo->GetFabricId();

    ChipLogProgress(
        JointFabric, "JCM: Administrator fabric mapped remote index %u to local index %u (FabricID=0x" ChipLogFormatX64 ")",
        static_cast<unsigned>(remoteFabricIndex), static_cast<unsigned>(fabricIndex), ChipLogValueX64(mInfo.adminFabricId));

    // Fetch root public key
    Crypto::P256PublicKey rootPubKey;
    if (fabricInfo->FetchRootPubkey(rootPubKey) != CHIP_NO_ERROR)
    {
        ChipLogError(JointFabric, "JCM: Failed to fetch fabric root key");
        return TrustVerificationError::kInternalError;
    }
    if (rootPubKey.Length() != Crypto::kP256_PublicKey_Length)
    {
        ChipLogError(JointFabric, "JCM: Fabric root key size mismatch");
        return TrustVerificationError::kInternalError;
    }
    const chip::ByteSpan keySpan(rootPubKey.ConstBytes(), rootPubKey.Length());
    mInfo.rootPublicKey.CopyFromSpan(keySpan);
    ChipLogProgress(JointFabric, "JCM: Admin RootPublicKey contents");
    ChipLogByteSpan(JointFabric, keySpan);

    if (mInfo.adminRCAC.AllocatedSize() == 0 || mInfo.adminNOC.AllocatedSize() == 0)
    {
        ChipLogError(JointFabric, "JCM: Administrator credentials missing after remote fetch");
        return TrustVerificationError::kInternalError;
    }

    ChipLogProgress(JointFabric, "JCM: Admin RCAC contents");
    ChipLogByteSpan(JointFabric, ByteSpan(mInfo.adminRCAC.Get(), mInfo.adminRCAC.AllocatedSize()));

    if (mInfo.adminICAC.AllocatedSize() > 0)
    {
        ChipLogProgress(JointFabric, "JCM: Admin ICAC contents");
        ChipLogByteSpan(JointFabric, ByteSpan(mInfo.adminICAC.Get(), mInfo.adminICAC.AllocatedSize()));
    }
    else
    {
        ChipLogProgress(JointFabric, "JCM: Admin ICAC not present");
    }

    ChipLogProgress(JointFabric, "JCM: Admin NOC contents");
    ChipLogByteSpan(JointFabric, ByteSpan(mInfo.adminNOC.Get(), mInfo.adminNOC.AllocatedSize()));

    return TrustVerificationError::kSuccess;
}

} // namespace JointFabricAdministrator
} // namespace Clusters
} // namespace app
} // namespace chip
