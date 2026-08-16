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
#include <app/server/Server.h>

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
#include <CommissionerMain.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

#include <controller/CHIPCluster.h>
#include <credentials/CHIPCert.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Controller;
using namespace chip::Credentials;
using namespace chip::Crypto;
using namespace chip::app::Clusters::JointFabricAdministrator;

constexpr uint8_t kJFAvailableShift = 0;
constexpr uint8_t kJFAdminShift     = 1;
constexpr uint8_t kJFAnchorShift    = 2;
constexpr uint8_t kJFDatastoreShift = 3;

constexpr chip::EndpointId kJointFabricAdminEndpointId = 1;

JFAManager JFAManager::sJFA;

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
    if (jfFabricIndex == kUndefinedFabricId)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogProgress(JointFabric, "FinalizeCommissioning for NodeID: 0x" ChipLogFormatX64 ", isJCM = %d, peerEndpointId = %d",
                    ChipLogValueX64(nodeId), isJCM, endpointId);

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

void JFAManager::HandleCommissioningCompleteEvent()
{
    for (const auto & fb : mServer->GetFabricTable())
    {
        FabricIndex fabricIndex = fb.GetFabricIndex();
        CATValues cats;

        if ((jfFabricIndex == kUndefinedFabricIndex) && mServer->GetFabricTable().FetchCATs(fabricIndex, cats) == CHIP_NO_ERROR)
        {
            /* When JFA is commissioned, it has to be issued a NOC with Anchor CAT and Administrator CAT */
            if (cats.ContainsIdentifier(kAdminCATIdentifier) && cats.ContainsIdentifier(kAnchorCATIdentifier))
            {
                (void) app::Clusters::JointFabricAdministrator::Attributes::AdministratorFabricIndex::Set(
                    kJointFabricAdminEndpointId, fabricIndex);

                jfFabricIndex = fabricIndex;

                // Record the joint fabric index so the datastore can be purged if this fabric is ever removed.
                Server::GetInstance().GetJointFabricDatastore().SetAnchorFabricIndex(fabricIndex);

                // Set AnchorRootCA
                uint8_t mAnchorRootCABuffer[Credentials::kMaxDERCertLength];
                MutableByteSpan rootCertSpan(mAnchorRootCABuffer);
                if (mServer->GetFabricTable().FetchRootCert(fabricIndex, rootCertSpan) == CHIP_NO_ERROR)
                {
                    if (Server::GetInstance().GetJointFabricDatastore().SetAnchorRootCA(rootCertSpan) == CHIP_NO_ERROR)
                    {
                        ChipLogProgress(JointFabric, "Set AnchorRootCA (%u bytes)", static_cast<unsigned>(rootCertSpan.size()));
                    }
                    else
                    {
                        ChipLogError(JointFabric, "Failed to set AnchorRootCA");
                    }
                }

                // obtain nodeid and use it to set anchornodeid
                NodeId nodeId = fb.GetNodeId();
                if (nodeId != kUndefinedNodeId)
                {
                    if (Server::GetInstance().GetJointFabricDatastore().SetAnchorNodeId(nodeId) == CHIP_NO_ERROR)
                    {
                        ChipLogProgress(JointFabric, "Set AnchorNodeId to 0x" ChipLogFormatX64, ChipLogValueX64(nodeId));
                    }
                    else
                    {
                        ChipLogError(JointFabric, "Failed to set AnchorNodeId to 0x" ChipLogFormatX64, ChipLogValueX64(nodeId));
                    }
                }

                VendorId vendorId = fb.GetVendorId();
                if (vendorId != VendorId::NotSpecified)
                {
                    if (Server::GetInstance().GetJointFabricDatastore().SetAnchorVendorId(vendorId) == CHIP_NO_ERROR)
                    {
                        ChipLogProgress(JointFabric, "Set AnchorVendorId to %d", vendorId);
                    }
                    else
                    {
                        ChipLogError(JointFabric, "Failed to set AnchorVendorId to %d", vendorId);
                    }
                }

                if (vendorId != VendorId::NotSpecified && nodeId != kUndefinedNodeId)
                {
                    char friendlyNameBuffer[32];
                    int written = snprintf(friendlyNameBuffer, sizeof(friendlyNameBuffer), "jfa-0x%04X-0x" ChipLogFormatX64,
                                           to_underlying(vendorId), ChipLogValueX64(nodeId));
                    CharSpan friendlyName = CharSpan(friendlyNameBuffer, static_cast<size_t>(written));

                    if (Server::GetInstance().GetJointFabricDatastore().SetFriendlyName(friendlyName) == CHIP_NO_ERROR)
                    {
                        ChipLogProgress(JointFabric, "Set FriendlyName to %.*s", static_cast<int>(friendlyName.size()),
                                        friendlyName.data());
                    }
                    else
                    {
                        ChipLogError(JointFabric, "Failed to set FriendlyName to %.*s", static_cast<int>(friendlyName.size()),
                                     friendlyName.data());
                    }
                }

                Server::GetInstance().GetJointFabricDatastore().SetStatus(
                    Clusters::JointFabricDatastore::DatastoreStateEnum::kPending,
                    static_cast<uint32_t>(System::SystemClock().GetMonotonicTimestamp().count()), 0);

                Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type defaultGroupKeySetEntry{ 0 };
                LogErrorOnFailure(Server::GetInstance().GetJointFabricDatastore().AddGroupKeySetEntry(defaultGroupKeySetEntry));
                LogErrorOnFailure(Server::GetInstance().GetJointFabricDatastore().ForceAddNodeKeySetEntry(0, nodeId));

                // Add default group entry for the JFA itself
                // Uses internal method that bypasses CAT restrictions since JFA setup needs both Admin and Anchor CATs
                Clusters::JointFabricDatastore::Commands::AddGroup::DecodableType addGroupCommandData;
                addGroupCommandData.groupID       = 0;
                addGroupCommandData.friendlyName  = "Default JFA Group"_span;
                addGroupCommandData.groupKeySetID = 0;
                addGroupCommandData.groupCAT      = kAdminCATIdentifier; // Use Admin CAT for default group
                addGroupCommandData.groupCATVersion.SetNonNull(1);
                addGroupCommandData.groupPermission =
                    Clusters::JointFabricDatastore::DatastoreAccessControlEntryPrivilegeEnum::kAdminister;
                LogErrorOnFailure(Server::GetInstance().GetJointFabricDatastore().ForceAddGroup(addGroupCommandData));
                addGroupCommandData.groupID  = 1;
                addGroupCommandData.groupCAT = kAnchorCATIdentifier; // Use Anchor CAT for second default group
                LogErrorOnFailure(Server::GetInstance().GetJointFabricDatastore().ForceAddGroup(addGroupCommandData));

                Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type newAdmin;
                newAdmin.nodeID       = nodeId;
                newAdmin.friendlyName = "Default JFA Admin"_span;
                newAdmin.vendorID     = vendorId;
                newAdmin.icac         = ByteSpan(mICACBuffer, mICACBufferLen);

                LogErrorOnFailure(Server::GetInstance().GetJointFabricDatastore().AddAdmin(newAdmin));

                if (!mCommissionerInitialized)
                {
                    CHIP_ERROR err = InitCommissioner(LinuxDeviceOptions::GetInstance().securedCommissionerPort,
                                                      LinuxDeviceOptions::GetInstance().unsecuredCommissionerPort, fb.GetFabricId(),
                                                      fabricIndex);
                    if (err == CHIP_NO_ERROR)
                    {
                        mCommissionerInitialized = true;
                        ChipLogProgress(JointFabric, "Commissioner mode initialized on commissioned fabric index %u",
                                        static_cast<unsigned>(fabricIndex));
                    }
                    else
                    {
                        ChipLogError(JointFabric, "Failed to initialize commissioner mode: %" CHIP_ERROR_FORMAT, err.Format());
                    }
                }

                ChipLogProgress(JointFabric, "Joint Fabric Administrator commissioned on fabric index %d", fabricIndex);
            }
        }
    }
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
    mNodeId            = scopedNodeId.GetNodeId();
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
        TEMPORARY_RETURN_IGNORED jfaManager->SendCommissioningComplete();
        break;
    }
    case kJCMCommissioning: {
        jfaManager->mJCMStage = kJCMStageAnnounce;
        TEMPORARY_RETURN_IGNORED jfaManager->AnnounceJointFabricAdministrator();
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

    jfaManager->mJCMStage = kJCMStageIdle;
    jfaManager->ReleaseSession();
}

CHIP_ERROR JFAManager::AnnounceJointFabricAdministrator()
{
    Commands::AnnounceJointFabricAdministrator::Type request;

    if (!mExchangeMgr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    ChipLogProgress(JointFabric, "AnnounceJointFabricAdministrator: invoke cluster command.");
    request.endpointID = kJointFabricAdminEndpointId;

    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), peerAdminJFAdminClusterEndpointId);
    return cluster.InvokeCommand(request, this, OnAnnounceJointFabricAdministratorResponse,
                                 OnAnnounceJointFabricAdministratorFailure);
}

void JFAManager::OnAnnounceJointFabricAdministratorResponse(void * context, const chip::app::DataModel::NullObjectType & data)
{
    JFAManager * jfaManagerCore = static_cast<JFAManager *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);

    ChipLogProgress(JointFabric, "OnAnnounceJointFabricAdministratorResponse");

    jfaManagerCore->mJCMStage = kJCMStageICACCSRRequest;
    /* TODO: https://github.com/project-chip/connectedhomeip/issues/38202 */
    TEMPORARY_RETURN_IGNORED jfaManagerCore->SendICACSRRequest();
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
    Commands::ICACCSRRequest::Type request;

    if (!mExchangeMgr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    ChipLogProgress(JointFabric, "SendICACSRRequest: invoke cluster command.");

    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), peerAdminJFAdminClusterEndpointId);
    return cluster.InvokeCommand(request, this, OnSendICACSRRequestResponse, OnSendICACSRRequestFailure);
}

void JFAManager::OnSendICACSRRequestResponse(void * context, const Commands::ICACCSRResponse::DecodableType & icaccsr)
{
    JFAManager * jfaManagerCore = static_cast<JFAManager *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);
    P256PublicKey pubKey;

    ChipLogProgress(JointFabric, "OnSendICACSRRequestResponse");

    if (!icaccsr.icaccsr.HasValue())
    {
        ChipLogError(JointFabric, "OnSendICACSRRequestResponse: missing ICAC CSR in response");
        jfaManagerCore->ReleaseSession();
        return;
    }

    ByteSpan csrSpan = icaccsr.icaccsr.Value();

    if (CHIP_NO_ERROR != VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), pubKey))
    {
        ChipLogError(JointFabric, "OnSendICACSRRequestResponse: ICAC CSR signature verification failed");
        jfaManagerCore->ReleaseSession();
        return;
    }

    if (!jfaManagerCore->peerAdminICACPubKey.Matches(pubKey))
    {
        ChipLogError(JointFabric, "OnSendICACSRRequestResponse: ICAC CSR public key does not match TrustedIcacPublicKeyB");
        jfaManagerCore->ReleaseSession();
        return;
    }

    ChipLogProgress(JointFabric, "OnSendICACSRRequestResponse: validated ICAC CSR");

    // Save the CSR for cross-signing
    VerifyOrDie(csrSpan.size() <= sizeof(jfaManagerCore->mPendingICACSRBuf));
    memcpy(jfaManagerCore->mPendingICACSRBuf, csrSpan.data(), csrSpan.size());
    jfaManagerCore->mPendingICACSRLen = csrSpan.size();

    // Cross-sign the CSR under the anchor root CA
    CHIP_ERROR err = jfaManagerCore->CrossSignICAC();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(JointFabric, "OnSendICACSRRequestResponse: CrossSignICAC failed: %" CHIP_ERROR_FORMAT, err.Format());
        jfaManagerCore->mPendingICACSRLen = 0;
        jfaManagerCore->ReleaseSession();
        return;
    }

    // Validate that the cross-signed ICAC chains to the anchor root before installing
    {
        constexpr uint8_t kMaxCertsInChain = 2;
        uint8_t rcacBuf[Credentials::kMaxCHIPCertLength];
        MutableByteSpan rcacSpan{ rcacBuf };
        Credentials::ChipCertificateSet certificates;
        Credentials::ValidationContext validContext;

        if (mServer->GetFabricTable().FetchRootCert(jfaManagerCore->jfFabricIndex, rcacSpan) != CHIP_NO_ERROR)
        {
            ChipLogError(JointFabric, "OnSendICACSRRequestResponse: failed to fetch anchor root cert");
            jfaManagerCore->mCrossSignedICACLen = 0;
            jfaManagerCore->mPendingICACSRLen   = 0;
            jfaManagerCore->ReleaseSession();
            return;
        }

        if (certificates.Init(kMaxCertsInChain) != CHIP_NO_ERROR ||
            certificates.LoadCert(rcacSpan, BitFlags<Credentials::CertDecodeFlags>(Credentials::CertDecodeFlags::kIsTrustAnchor)) !=
                CHIP_NO_ERROR ||
            certificates.LoadCert(ByteSpan(jfaManagerCore->mCrossSignedICACBuf, jfaManagerCore->mCrossSignedICACLen),
                                  BitFlags<Credentials::CertDecodeFlags>(Credentials::CertDecodeFlags::kGenerateTBSHash)) !=
                CHIP_NO_ERROR)
        {
            ChipLogError(JointFabric, "OnSendICACSRRequestResponse: failed to load certificates for chain validation");
            jfaManagerCore->mCrossSignedICACLen = 0;
            jfaManagerCore->mPendingICACSRLen   = 0;
            jfaManagerCore->ReleaseSession();
            return;
        }

        validContext.Reset();
        validContext.mRequiredKeyUsages.Set(Credentials::KeyUsageFlags::kKeyCertSign);
        validContext.mRequiredCertType = Credentials::CertType::kICA;

        if (certificates.ValidateCert(certificates.GetLastCert(), validContext) != CHIP_NO_ERROR)
        {
            ChipLogError(JointFabric, "OnSendICACSRRequestResponse: cross-signed ICAC chain validation failed");
            jfaManagerCore->mCrossSignedICACLen = 0;
            jfaManagerCore->mPendingICACSRLen   = 0;
            jfaManagerCore->ReleaseSession();
            return;
        }
    }

    ChipLogProgress(JointFabric, "OnSendICACSRRequestResponse: cross-signed ICAC validated, invoking AddICAC");

    jfaManagerCore->mJCMStage = kJCMStageAddICAC;

    // Proceed with installing the cross-signed ICAC on the commissionee
    err = jfaManagerCore->SendAddICAC();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(JointFabric, "OnSendICACSRRequestResponse: SendAddICAC failed: %" CHIP_ERROR_FORMAT, err.Format());
        jfaManagerCore->mCrossSignedICACLen = 0;
        jfaManagerCore->mPendingICACSRLen   = 0;
        jfaManagerCore->ReleaseSession();
    }
}

void JFAManager::OnSendICACSRRequestFailure(void * context, CHIP_ERROR error)
{
    JFAManager * jfaManagerCore = static_cast<JFAManager *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);
    jfaManagerCore->ReleaseSession();

    ChipLogError(JointFabric, "OnSendICACSRRequestFailure: %s\n", chip::ErrorStr(error));
}

CHIP_ERROR JFAManager::CrossSignICAC()
{
    JFARpc * jfaRpc = GetJFARpc();
    VerifyOrReturnError(jfaRpc != nullptr, CHIP_ERROR_UNINITIALIZED);
    VerifyOrReturnError(mPendingICACSRLen > 0, CHIP_ERROR_INCORRECT_STATE);

    // Retrieve the anchor fabric ID from the fabric table
    const FabricInfo * fabricInfo = mServer->GetFabricTable().FindFabricWithIndex(jfFabricIndex);
    VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INCORRECT_STATE);
    FabricId anchorFabricId = fabricInfo->GetFabricId();

    ByteSpan csrSpan(mPendingICACSRBuf, mPendingICACSRLen);
    MutableByteSpan crossSignedICAC(mCrossSignedICACBuf, sizeof(mCrossSignedICACBuf));

    mJCMStage = kJCMStageCrossSign;
    ChipLogProgress(JointFabric, "CrossSignICAC: requesting cross-signed ICAC for anchor fabric 0x" ChipLogFormatX64,
                    ChipLogValueX64(anchorFabricId));

    CHIP_ERROR err = jfaRpc->GetCrossSignedICAC(anchorFabricId, csrSpan, crossSignedICAC);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(JointFabric, "CrossSignICAC: GetCrossSignedICAC failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    mCrossSignedICACLen = crossSignedICAC.size();
    ChipLogProgress(JointFabric, "CrossSignICAC: received cross-signed ICAC (%u bytes)",
                    static_cast<unsigned>(mCrossSignedICACLen));
    return CHIP_NO_ERROR;
}

CHIP_ERROR JFAManager::SendAddICAC()
{
    Commands::AddICAC::Type request;

    if (!mExchangeMgr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    VerifyOrReturnError(mCrossSignedICACLen > 0, CHIP_ERROR_INCORRECT_STATE);

    request.ICACValue = ByteSpan(mCrossSignedICACBuf, mCrossSignedICACLen);

    ChipLogProgress(JointFabric, "SendAddICAC: invoke cluster command with %u byte ICAC",
                    static_cast<unsigned>(mCrossSignedICACLen));
    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), peerAdminJFAdminClusterEndpointId);
    return cluster.InvokeCommand(request, this, OnAddICACResponse, OnAddICACFailure);
}

void JFAManager::OnAddICACResponse(void * context,
                                   const Commands::ICACResponse::DecodableType & response)
{
    JFAManager * jfaManagerCore = static_cast<JFAManager *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);

    ChipLogProgress(JointFabric, "OnAddICACResponse: statusCode=%u", to_underlying(response.statusCode));

    jfaManagerCore->mCrossSignedICACLen = 0;
    jfaManagerCore->mPendingICACSRLen   = 0;

    if (response.statusCode != app::Clusters::JointFabricAdministrator::ICACResponseStatusEnum::kOk)
    {
        ChipLogError(JointFabric, "OnAddICACResponse: AddICAC command failed with status %u",
                     to_underlying(response.statusCode));
        jfaManagerCore->ReleaseSession();
        return;
    }

    ChipLogProgress(JointFabric, "OnAddICACResponse: ICAC installed successfully, proceeding to CommissioningComplete");
    jfaManagerCore->mJCMStage = kJCMStageCommissionComplete;
    TEMPORARY_RETURN_IGNORED jfaManagerCore->SendCommissioningComplete();
}

void JFAManager::OnAddICACFailure(void * context, CHIP_ERROR error)
{
    JFAManager * jfaManagerCore = static_cast<JFAManager *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);

    jfaManagerCore->mCrossSignedICACLen = 0;
    jfaManagerCore->mPendingICACSRLen   = 0;
    jfaManagerCore->ReleaseSession();

    ChipLogError(JointFabric, "OnAddICACFailure: %s\n", chip::ErrorStr(error));
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
    JFAManager * jfaManager = static_cast<JFAManager *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogProgress(JointFabric, "OnCommissioningCompleteResponse, Code=%u", to_underlying(data.errorCode));

    if (data.errorCode != GeneralCommissioning::CommissioningErrorEnum::kOk)
    {
        ChipLogError(JointFabric, "Commissioning Failed (nodeId=%ld, isJCM = %d), Code=%u", jfaManager->mNodeId,
                     jfaManager->mOnConnectedAction == kJCMCommissioning, to_underlying(data.errorCode));
    }
    else
    {
        switch (jfaManager->mOnConnectedAction)
        {
        case kStandardCommissioningComplete: {
            ChipLogProgress(JointFabric, "Standard Commissioning (nodeId=%ld) success", jfaManager->mNodeId);
            break;
        }
        case kJCMCommissioning: {
            ChipLogProgress(JointFabric, "Joint Commissioning Method (nodeId=%ld) success", jfaManager->mNodeId);
            break;
        }
        }
    }

    jfaManager->mJCMStage = kJCMStageIdle;
    jfaManager->ReleaseSession();
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

CHIP_ERROR JFAManager::StoreCrossSignedICAC(const ByteSpan & crossSignedICAC)
{
    VerifyOrReturnError(crossSignedICAC.size() <= sizeof(mICACBuffer), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(mICACBuffer, crossSignedICAC.data(), crossSignedICAC.size());
    mICACBufferLen = crossSignedICAC.size();

    ChipLogProgress(JointFabric, "StoreCrossSignedICAC: stored cross-signed ICAC (%u bytes)",
                    static_cast<unsigned>(mICACBufferLen));
    return CHIP_NO_ERROR;
}
