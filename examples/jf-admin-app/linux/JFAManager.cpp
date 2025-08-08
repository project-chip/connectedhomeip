/*
 *
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

#include "JFAManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>

#include <controller/CHIPCluster.h>
#include <lib/support/logging/CHIPLogging.h>

#include <platform/KeyValueStoreManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Controller;
using namespace chip::Crypto;
using namespace chip::Credentials;
using namespace chip::DeviceLayer::PersistedStorage;

constexpr uint8_t kJFAvailableShift = 0;
constexpr uint8_t kJFAdminShift     = 1;
constexpr uint8_t kJFAnchorShift    = 2;
constexpr uint8_t kJFDatastoreShift = 3;

JFAManager JFAManager::sJFA;
static constexpr const char kJCMICAC[] = "kJCMICAC";

CHIP_ERROR JFAManager::Init(Server & server)
{
    mServer             = &server;
    mCASESessionManager = server.GetCASESessionManager();

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFAManager::GetJointFabricMode(uint8_t & jointFabricMode)
{
    jointFabricMode = ((IsDeviceCommissioned() ? 0 : 1) << kJFAvailableShift) |
        (IsDeviceCommissioned() ? (IsDeviceJFAdmin() ? (1 << kJFAdminShift) : 0) : 0) |
        (IsDeviceCommissioned() ? (IsDeviceJFAnchor() ? (1 << kJFAnchorShift) : 0) : 0) |
        (IsDeviceCommissioned() ? (1 << kJFDatastoreShift) : 0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR JFAManager::FinalizeCommissioning(NodeId nodeId, bool isJCM, P256PublicKey & trustedIcacPublicKeyB, uint16_t endpointId)
{
    if (jfFabricIndex == kUndefinedFabricIndex)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogProgress(JointFabric, "FinalizeCommissioning for NodeID: 0x" ChipLogFormatX64 ", isJCM = %d, peerEndpointId = %d",
                    ChipLogValueX64(nodeId), isJCM, endpointId);

    for (size_t i = 0; i < Crypto::kP256_PublicKey_Length; ++i)
    {
        ChipLogProgress(JointFabric, "trustedIcacPublicKeyB[%li] = %02X", i, trustedIcacPublicKeyB.Bytes()[i]);
    }

    peerAdminJFAdminClusterEndpointId = endpointId;
    peerAdminICACPubKey               = trustedIcacPublicKeyB;
    ScopedNodeId scopedNodeId         = ScopedNodeId(nodeId, jfFabricIndex);
    ConnectToNode(scopedNodeId, isJCM ? kJCMCommissioning : kStandardCommissioningComplete);

    return CHIP_NO_ERROR;
}

void JFAManager::SetJFARpc(JFARpc & aJFARpc)
{
    mJFARpc = &aJFARpc;
}

JFARpc * JFAManager::GetJFARpc()
{
    return mJFARpc;
}

void JFAManager::HandleCommissioningCompleteEvent(FabricIndex fabricIndex)
{
    auto & fabricTable            = Server::GetInstance().GetFabricTable();
    const FabricInfo * fabricInfo = fabricTable.FindFabricWithIndex(fabricIndex);
    CATValues cats;

    if (jfFabricIndex == kUndefinedFabricIndex && fabricInfo)
    {
        if (CHIP_NO_ERROR == fabricTable.FetchCATs(fabricIndex, cats) && cats.ContainsIdentifier(kAdminCATIdentifier) &&
            cats.ContainsIdentifier(kAnchorCATIdentifier))
        {
            (void) app::Clusters::JointFabricAdministrator::Attributes::AdministratorFabricIndex::Set(1, fabricIndex);
            jfFabricIndex = fabricIndex;
            jfFabricId    = fabricInfo->GetFabricId();
        }
    }
    else if (pendingCrossSignedICAC)
    {
        KeyValueStoreMgr().Put(kJCMICAC, crossSignedICACChipCert.data(), crossSignedICACChipCert.size());
        pendingCrossSignedICAC = false;

        (void) app::Clusters::JointFabricAdministrator::Attributes::AdministratorFabricIndex::Set(1, fabricIndex);
        jfFabricIndex = fabricIndex;
        jfFabricId    = fabricInfo->GetFabricId();
    }
}

void JFAManager::HandleFailsafeTimerExpired()
{
    pendingCrossSignedICAC = false;
}

bool JFAManager::IsDeviceJFAdmin()
{
    if (jfFabricIndex == kUndefinedFabricId)
    {
        return false;
    }

    CATValues cats;

    if (mServer->GetFabricTable().FetchCATs(jfFabricIndex, cats) == CHIP_NO_ERROR)
    {
        if (cats.ContainsIdentifier(kAdminCATIdentifier))
        {
            return true;
        }
    }

    return false;
}

bool JFAManager::IsDeviceJFAnchor()
{
    if (jfFabricIndex == kUndefinedFabricId)
    {
        return false;
    }

    CATValues cats;

    if (mServer->GetFabricTable().FetchCATs(jfFabricIndex, cats) == CHIP_NO_ERROR)
    {
        if (cats.ContainsIdentifier(kAnchorCATIdentifier))
        {
            return true;
        }
    }

    return false;
}

void JFAManager::ReleaseSession()
{
    auto optionalSessionHandle = mSessionHolder.Get();

    if (optionalSessionHandle.HasValue())
    {
        if (optionalSessionHandle.Value()->IsActiveSession())
        {
            optionalSessionHandle.Value()->AsSecureSession()->MarkAsDefunct();
        }
    }
    mSessionHolder.Release();
}

void JFAManager::ConnectToNode(ScopedNodeId scopedNodeId, OnConnectedAction onConnectedAction)
{
    VerifyOrDie(mServer != nullptr);

    if ((scopedNodeId.GetFabricIndex() == kUndefinedFabricIndex) || (scopedNodeId.GetNodeId() == kUndefinedNodeId))
    {
        ChipLogError(JointFabric, "Invalid node location!");
        return;
    }

    // Set the action to take once connection is successfully established
    mOnConnectedAction = onConnectedAction;

    ChipLogDetail(JointFabric, "Establishing session to node ID 0x" ChipLogFormatX64 " on fabric index %d",
                  ChipLogValueX64(scopedNodeId.GetNodeId()), scopedNodeId.GetFabricIndex());

    mCASESessionManager->FindOrEstablishSession(scopedNodeId, &mOnConnectedCallback, &mOnConnectionFailureCallback);
}

void JFAManager::OnConnected(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    JFAManager * jfaManager = static_cast<JFAManager *>(context);

    VerifyOrDie(jfaManager != nullptr);
    jfaManager->mSessionHolder.Grab(sessionHandle);
    jfaManager->mExchangeMgr = &exchangeMgr;

    ChipLogProgress(JointFabric, "Established CASE");

    switch (jfaManager->mOnConnectedAction)
    {
    case kStandardCommissioningComplete: {
        jfaManager->SendCommissioningComplete();
        break;
    }
    case kJCMCommissioning: {
        jfaManager->AnnounceJointFabricAdministrator();
        break;
    }

    default:
        break;
    }
}

void JFAManager::OnConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    JFAManager * jfaManager = static_cast<JFAManager *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogError(JointFabric, "Failed to establish connection to 0x" ChipLogFormatX64 " on fabric index %d",
                 ChipLogValueX64(peerId.GetNodeId()), peerId.GetFabricIndex());

    jfaManager->ReleaseSession();
}

CHIP_ERROR JFAManager::AnnounceJointFabricAdministrator()
{
    JointFabricAdministrator::Commands::AnnounceJointFabricAdministrator::Type request;

    if (!mExchangeMgr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    ChipLogProgress(JointFabric, "AnnounceJointFabricAdministrator: invoke cluster command.");
    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), peerAdminJFAdminClusterEndpointId);
    return cluster.InvokeCommand(request, this, OnAnnounceJointFabricAdministratorResponse,
                                 OnAnnounceJointFabricAdministratorFailure);
}

void JFAManager::OnAnnounceJointFabricAdministratorResponse(void * context, const chip::app::DataModel::NullObjectType & data)
{
    JFAManager * jfaManagerCore = static_cast<JFAManager *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);

    ChipLogProgress(JointFabric, "OnAnnounceJointFabricAdministratorResponse");

    /* TODO: https://github.com/project-chip/connectedhomeip/issues/38202 */
    jfaManagerCore->SendICACSRRequest();
}

void JFAManager::OnAnnounceJointFabricAdministratorFailure(void * context, CHIP_ERROR error)
{
    JFAManager * jfaManagerCore = static_cast<JFAManager *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);
    jfaManagerCore->ReleaseSession();

    ChipLogError(JointFabric, "OnAnnounceJointFabricAdministratorFailure: %s\n", chip::ErrorStr(error));
}

CHIP_ERROR JFAManager::SendICACSRRequest()
{
    JointFabricAdministrator::Commands::ICACCSRRequest::Type request;

    if (!mExchangeMgr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    ChipLogProgress(JointFabric, "SendICACSRRequest: invoke cluster command.");

    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), peerAdminJFAdminClusterEndpointId);
    return cluster.InvokeCommand(request, this, OnSendICACSRRequestResponse, OnSendICACSRRequestFailure);
}

void JFAManager::OnSendICACSRRequestResponse(void * context,
                                             const JointFabricAdministrator::Commands::ICACCSRResponse::DecodableType & icaccsr)
{
    JFAManager * jfaManagerCore = static_cast<JFAManager *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);
    P256PublicKey pubKey;

    ChipLogProgress(JointFabric, "OnSendICACSRRequestResponse");

    if ((CHIP_NO_ERROR == VerifyCertificateSigningRequest(icaccsr.icaccsr.data(), icaccsr.icaccsr.size(), pubKey)) &&
        jfaManagerCore->peerAdminICACPubKey.Matches(pubKey))
    {
        ChipLogProgress(JointFabric, "OnSendICACSRRequestResponse: validated ICAC CSR");

        ByteSpan icacCSR{ icaccsr.icaccsr.data(), icaccsr.icaccsr.size() };
        uint8_t mRequestBytes[kMaxDERCertLength] = { 0 };
        MutableByteSpan crossSignedICAC{ mRequestBytes, kMaxDERCertLength };

        if (CHIP_NO_ERROR == jfaManagerCore->GetCrossSignedIcac(jfaManagerCore->jfFabricId, icacCSR, crossSignedICAC))
        {
            ChipLogProgress(JointFabric, "OnSendICACSRRequestResponse, GetCrossSignedIcac, no error");
            uint8_t chipCertBuf[kMaxCHIPCertLength];
            MutableByteSpan chipCert{ chipCertBuf };

            if (CHIP_NO_ERROR == ConvertX509CertToChipCert(ByteSpan{ crossSignedICAC.data(), crossSignedICAC.size() }, chipCert))
            {
                ByteSpan icac = ByteSpan{ chipCert.data(), chipCert.size() };
                jfaManagerCore->SendAddICAC(icac);
            }
            else
            {
                ChipLogProgress(JointFabric, "OnSendICACSRRequestResponse, ConvertX509CertToChipCert, error");
            }
        }
        else
        {
            ChipLogProgress(JointFabric, "OnSendICACSRRequestResponse, GetCrossSignedIcac, error");
        }
    }
}

void JFAManager::OnSendICACSRRequestFailure(void * context, CHIP_ERROR error)
{
    JFAManager * jfaManagerCore = static_cast<JFAManager *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);
    jfaManagerCore->ReleaseSession();

    ChipLogError(JointFabric, "OnSendICACSRRequestFailure: %s\n", chip::ErrorStr(error));
}

CHIP_ERROR JFAManager::SendAddICAC(ByteSpan & crossSignedICAC)
{

    JointFabricAdministrator::Commands::AddICAC::Type request;

    request.ICACValue = crossSignedICAC;

    if (!mExchangeMgr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), peerAdminJFAdminClusterEndpointId);
    return cluster.InvokeCommand(request, this, OnSendAddICACResponse, OnSendAddICACFailure);
}

void JFAManager::OnSendAddICACResponse(
    void * context, const chip::app::Clusters::JointFabricAdministrator::Commands::ICACResponse::DecodableType & response)
{
    JFAManager * jfaManagerCore = static_cast<JFAManager *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);

    ChipLogProgress(JointFabric, "OnSendAddICACResponse");

    if (response.statusCode == chip::app::Clusters::JointFabricAdministrator::ICACResponseStatusEnum::kOk)
    {
        jfaManagerCore->SendCommissioningComplete();
    }
}

void JFAManager::OnSendAddICACFailure(void * context, CHIP_ERROR error)
{
    JFAManager * jfaManagerCore = static_cast<JFAManager *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);
    jfaManagerCore->ReleaseSession();

    ChipLogError(JointFabric, "OnSendAddICACFailure: %s\n", chip::ErrorStr(error));
}

CHIP_ERROR JFAManager::SendCommissioningComplete()
{
    GeneralCommissioning::Commands::CommissioningComplete::Type request;

    if (!mExchangeMgr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    ChipLogProgress(JointFabric, "SendCommissioningComplete: invoke cluster command.");
    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), kRootEndpointId);
    return cluster.InvokeCommand(request, this, OnCommissioningCompleteResponse, OnCommissioningCompleteFailure);
}

void JFAManager::OnCommissioningCompleteResponse(
    void * context, const GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data)
{
    JFAManager * jfaManagerCore = static_cast<JFAManager *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);
    jfaManagerCore->ReleaseSession();

    ChipLogProgress(JointFabric, "OnCommissioningCompleteResponse, Code=%u", to_underlying(data.errorCode));

    if (data.errorCode != GeneralCommissioning::CommissioningErrorEnum::kOk)
    {
        // TODO
    }
    else
    {
        // TODO
    }

    jfaManagerCore->ReleaseSession();
}

void JFAManager::OnCommissioningCompleteFailure(void * context, CHIP_ERROR error)
{
    JFAManager * jfaManagerCore = static_cast<JFAManager *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);
    jfaManagerCore->ReleaseSession();

    ChipLogError(JointFabric, "Received failure response %s\n", chip::ErrorStr(error));
}

CHIP_ERROR JFAManager::GetIcacCsr(MutableByteSpan & icacCsr)
{
    JFARpc * jfaRpc = GetJFARpc();

    if (jfaRpc)
    {
        return jfaRpc->GetICACCSRForJF(icacCsr);
    }

    return CHIP_ERROR_UNINITIALIZED;
}

void JFAManager::OnAddICAC(MutableByteSpan & receivedCrossSignedICAC)
{
    ChipLogProgress(JointFabric, "OnAddICAC");

    /* just the first iteration of the code where a single cross-signed ICAC is received then used for issuing new NOCs
     * In the future iterations we need to add support for replacing a cross-signed ICAC:
     * see https://github.com/project-chip/connectedhomeip/issues/40393
     */
    crossSignedICACChipCert = MutableByteSpan{ crossSignedICACChipCertBuf };
    CopySpanToMutableSpan(ByteSpan{ receivedCrossSignedICAC.data(), receivedCrossSignedICAC.size() }, crossSignedICACChipCert);

    pendingCrossSignedICAC = true;
}

CHIP_ERROR JFAManager::GetCrossSignedIcac(uint64_t anchorFabricId, ByteSpan & icacCSR, MutableByteSpan & crossSignedICAC)
{
    JFARpc * jfaRpc = GetJFARpc();

    if (jfaRpc)
    {
        return jfaRpc->GetCrossSignedIcacForJF(anchorFabricId, icacCSR, crossSignedICAC);
    }

    return CHIP_ERROR_UNINITIALIZED;
}
