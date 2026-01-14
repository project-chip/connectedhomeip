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

void JCMCommissionee::VerifyTrustAgainstCommissionerAdmin()
{
    StartTrustVerification();
}

CHIP_ERROR JCMCommissionee::ReadAdminFabricIndexAttribute(
    std::function<void(const ConcreteAttributePath &, const FabricIndexAttr::DecodableType &)> onSuccess, ReadErrorHandler onError)
{
    return ReadAttribute<FabricIndexAttr>(mInfo.adminEndpointId, std::move(onSuccess), std::move(onError), false);
}

CHIP_ERROR JCMCommissionee::ReadAdminFabricsAttribute(
    std::function<void(const ConcreteAttributePath &, const FabricsAttr::DecodableType &)> onSuccess, ReadErrorHandler onError)
{
    return ReadAttribute<FabricsAttr>(kRootEndpointId, std::move(onSuccess), std::move(onError), false);
}

CHIP_ERROR JCMCommissionee::ReadAdminCertsAttribute(
    std::function<void(const ConcreteAttributePath &, const CertsAttr::TypeInfo::DecodableType &)> onSuccess,
    ReadErrorHandler onError)
{
    return ReadAttribute<CertsAttr>(kRootEndpointId, std::move(onSuccess), std::move(onError), false);
}

CHIP_ERROR JCMCommissionee::ReadAdminNOCsAttribute(
    std::function<void(const ConcreteAttributePath &, const NOCsAttr::DecodableType &)> onSuccess, ReadErrorHandler onError)
{
    return ReadAttribute<NOCsAttr>(kRootEndpointId, std::move(onSuccess), std::move(onError), false);
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

void JCMCommissionee::OnVendorIdVerificationComplete(const CHIP_ERROR & err)
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
    auto onSuccess = [this](const ConcreteAttributePath & path, const FabricIndexAttr::DecodableType & val) {
        if (val.IsNull())
        {
            ChipLogError(JointFabric, "JCM: Failed to read commissioner's AdministratorFabricIndex: received null");
            this->TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex,
                                                 TrustVerificationError::kReadAdminAttributeFailed);
        }
        else
        {
            FabricIndex remoteFabricIndex = static_cast<FabricIndex>(val.Value());
            if (!IsValidFabricIndex(remoteFabricIndex))
            {
                ChipLogError(JointFabric, "JCM: AdministratorFabricIndex %u is invalid", static_cast<unsigned>(remoteFabricIndex));
                this->TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex,
                                                     TrustVerificationError::kInvalidAdministratorFabricIndex);
                return;
            }
            mInfo.adminFabricIndex = remoteFabricIndex;
            this->TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex, TrustVerificationError::kSuccess);
        }
    };

    auto onError = [this](const ConcreteAttributePath *, CHIP_ERROR err) {
        ChipLogError(JointFabric, "JCM: Failed to read commissioner's AdministratorFabricIndex: %" CHIP_ERROR_FORMAT, err.Format());
        this->TrustVerificationStageFinished(kReadingCommissionerAdminFabricIndex,
                                             TrustVerificationError::kReadAdminAttributeFailed);
    };

    CHIP_ERROR err = ReadAdminFabricIndexAttribute(onSuccess, onError);

    if (err == CHIP_NO_ERROR)
    {
        return TrustVerificationError::kAsync;
    }

    return TrustVerificationError::kReadAdminAttributeFailed;
}

CHIP_ERROR JCMCommissionee::ReadAdminFabrics(OnCompletionFunc onComplete)
{
    auto onReadSuccess = [this, onComplete](const ConcreteAttributePath &, const FabricsAttr::DecodableType & fabrics) {
        // Get the root public key from the fabric corresponding to the Administrator Fabric Index (mInfo.adminFabricIndex)
        auto iter  = fabrics.begin();
        bool found = false;
        while (iter.Next())
        {
            const auto & fabricDescriptor = iter.GetValue();
            if (fabricDescriptor.fabricIndex == mInfo.adminFabricIndex)
            {
                mInfo.rootPublicKey.CopyFromSpan(fabricDescriptor.rootPublicKey);
                mInfo.adminFabricId = fabricDescriptor.fabricID;
                mInfo.adminVendorId = fabricDescriptor.vendorID;
                ChipLogProgress(
                    JointFabric,
                    "JCM: Copied rootPublicKey, fabricID, and vendorID from fabric indicated by Administrator Fabric Index");
                found = true;
                break;
            }
        }

        CHIP_ERROR err = iter.GetStatus();
        if (err == CHIP_NO_ERROR && !found)
        {
            ChipLogError(JointFabric, "JCM: Administrator fabric not found in Fabrics list");
            err = CHIP_ERROR_NOT_FOUND;
        }
        onComplete(err);
    };
    auto onError = [onComplete](const ConcreteAttributePath *, CHIP_ERROR err) {
        ChipLogError(JointFabric, "JCM: Failed to read commissioner's Fabrics list: %" CHIP_ERROR_FORMAT, err.Format());
        onComplete(err);
    };
    return ReadAdminFabricsAttribute(onReadSuccess, onError);
}

void JCMCommissionee::FetchCommissionerInfo(OnCompletionFunc onComplete)
{
    CHIP_ERROR fabricReadErr = ReadAdminFabrics([this, onComplete](CHIP_ERROR fabricsResultErr) {
        if (fabricsResultErr != CHIP_NO_ERROR)
        {
            onComplete(fabricsResultErr);
            return;
        }

        CHIP_ERROR certReadErr = ReadAdminCerts([this, onComplete](CHIP_ERROR certsResultErr) {
            if (certsResultErr != CHIP_NO_ERROR)
            {
                onComplete(certsResultErr);
                return;
            }

            CHIP_ERROR nocReadErr = ReadAdminNOCs([onComplete](CHIP_ERROR nocResultErr) { onComplete(nocResultErr); });
            if (nocReadErr != CHIP_NO_ERROR)
            {
                onComplete(nocReadErr);
            }
        });
        if (certReadErr != CHIP_NO_ERROR)
        {
            onComplete(certReadErr);
        }
    });

    if (fabricReadErr != CHIP_NO_ERROR)
    {
        onComplete(fabricReadErr);
    }
}

TrustVerificationError JCMCommissionee::PerformVendorIdVerification()
{
    FetchCommissionerInfo([this](CHIP_ERROR err) {
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(JointFabric, "Failed to read commissioner info. Error: %" CHIP_ERROR_FORMAT, err.Format());
            OnVendorIdVerificationComplete(err);
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

        CHIP_ERROR verifyErr = VerifyVendorId(exchangeMgr, sessionHandleGetter, &mInfo);
        if (verifyErr != CHIP_NO_ERROR)
        {
            ChipLogError(JointFabric, "JCM: Failed to start VendorId verification: %s", ErrorStr(verifyErr));
            OnVendorIdVerificationComplete(verifyErr);
        }
    });

    return TrustVerificationError::kAsync;
}

TrustVerificationError JCMCommissionee::CrossCheckAdministratorIds()
{
    const FabricTable & fabricTable = Server::GetInstance().GetFabricTable();

    // Get accessing fabric from the current command
    const FabricInfo * accessingFabricInfo = fabricTable.FindFabricWithIndex(mAccessingFabricIndex);
    VerifyOrReturnError(accessingFabricInfo != nullptr, TrustVerificationError::kInternalError);

    Crypto::P256PublicKey accessingRootPubKey;
    CHIP_ERROR err = accessingFabricInfo->FetchRootPubkey(accessingRootPubKey);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(JointFabric, "JCM: Unable to fetch accessing RootPublicKey");
    }
    return ValidateAdministratorIdsMatch(accessingFabricInfo->GetFabricId(), accessingRootPubKey);
}

CHIP_ERROR JCMCommissionee::ReadAdminCerts(OnCompletionFunc onComplete)
{
    auto onSuccess = [this, onComplete](const ConcreteAttributePath &, const CertsAttr::DecodableType & roots) {
        // Find the RCAC
        auto iter = roots.begin();
        if (!iter.Next())
        {
            onComplete(CHIP_ERROR_INTERNAL);
            return;
        }
        ByteSpan rootSpan = iter.GetValue();

        // Copy the RCAC to mInfo
        mInfo.adminRCAC.CopyFromSpan(rootSpan);
        if ((rootSpan.size() == 0) || (mInfo.adminRCAC.AllocatedSize() != rootSpan.size()))
        {
            ChipLogError(JointFabric, "JCM: Failed to store administrator root cert");
            onComplete(CHIP_ERROR_INTERNAL);
            return;
        }
        ChipLogProgress(JointFabric, "JCM: Successfully read admin RCAC");

        onComplete(CHIP_NO_ERROR);
    };

    auto onError = [onComplete](const ConcreteAttributePath *, CHIP_ERROR err) {
        ChipLogError(JointFabric, "JCM: Failed to read commissioner RCAC: %" CHIP_ERROR_FORMAT, err.Format());
        onComplete(err);
    };

    return ReadAdminCertsAttribute(onSuccess, onError);
}

CHIP_ERROR JCMCommissionee::ReadAdminNOCs(OnCompletionFunc onComplete)
{
    auto onSuccess = [this, onComplete](const ConcreteAttributePath &, const NOCsAttr::DecodableType & nocs) {
        auto iter  = nocs.begin();
        bool found = false;

        while (iter.Next())
        {
            auto & nocStruct = iter.GetValue();

            // Search for the NOC struct that matches the Administrator Fabric Index
            if (nocStruct.fabricIndex != mInfo.adminFabricIndex)
            {
                continue;
            }

            // Get the NOC and ICAC from the noc struct
            mInfo.adminNOC.CopyFromSpan(nocStruct.noc);
            size_t nocSize = nocStruct.noc.size();
            if ((nocSize == 0) || (mInfo.adminNOC.AllocatedSize() != nocSize))
            {
                ChipLogError(JointFabric, "JCM: Failed to store administrator NOC");
                onComplete(CHIP_ERROR_INTERNAL);
                return;
            }

            if (nocStruct.icac.IsNull())
            {
                ChipLogError(JointFabric, "JCM: ICAC not found");
                onComplete(CHIP_ERROR_INTERNAL);
                return;
            }
            ByteSpan icacSpan = nocStruct.icac.Value();
            mInfo.adminICAC.CopyFromSpan(icacSpan);
            size_t icacSize = icacSpan.size();
            if ((icacSize == 0) || (mInfo.adminICAC.AllocatedSize() != icacSize))
            {
                ChipLogError(JointFabric, "JCM: Failed to store administrator ICAC");
                onComplete(CHIP_ERROR_INTERNAL);
                return;
            }
            found = true;
            break;
        }
        CHIP_ERROR err = iter.GetStatus();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(JointFabric, "JCM: Error decoding NOCs. iter status: %" CHIP_ERROR_FORMAT, err.Format());
            onComplete(CHIP_ERROR_INTERNAL);
            return;
        }

        if (!found)
        {
            ChipLogError(JointFabric, "JCM: Administrator NOC not found in NOCs list");
            onComplete(CHIP_ERROR_NOT_FOUND);
            return;
        }

        ChipLogProgress(JointFabric, "JCM: Successfully read NOC and ICAC");
        onComplete(CHIP_NO_ERROR);
    };

    auto onError = [onComplete](const ConcreteAttributePath *, CHIP_ERROR err) {
        ChipLogError(JointFabric, "JCM: Failed to read commissioner NOCs: %" CHIP_ERROR_FORMAT, err.Format());
        onComplete(err);
    };

    return ReadAdminNOCsAttribute(onSuccess, onError);
}

TrustVerificationError JCMCommissionee::ValidateAdministratorIdsMatch(FabricId accessingFabricId,
                                                                      const Crypto::P256PublicKey & accessingRootPubKey) const
{
    if (accessingFabricId != mInfo.adminFabricId)
    {
        ChipLogError(JointFabric, "JCM: Accessing FabricID does not match AdministratorFabricID");
        return TrustVerificationError::kAdministratorIdMismatched;
    }

    const size_t accessingKeyLen    = accessingRootPubKey.Length();
    const size_t commissionerKeyLen = mInfo.rootPublicKey.AllocatedSize();
    if (accessingKeyLen != Crypto::kP256_PublicKey_Length || commissionerKeyLen != Crypto::kP256_PublicKey_Length)
    {
        ChipLogError(JointFabric, "JCM: RootPublicKey length mismatch");
        return TrustVerificationError::kInternalError;
    }

    if (memcmp(mInfo.rootPublicKey.Get(), accessingRootPubKey.ConstBytes(), Crypto::kP256_PublicKey_Length) != 0)
    {
        ChipLogError(JointFabric, "JCM: Accessing RootPublicKey does not match Administrator RootPublicKey");
        return TrustVerificationError::kAdministratorIdMismatched;
    }

    return TrustVerificationError::kSuccess;
}

} // namespace JointFabricAdministrator
} // namespace Clusters
} // namespace app
} // namespace chip
