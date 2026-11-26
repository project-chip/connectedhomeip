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

#include "JFAManagerBase.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/server/Server.h>
#include <credentials/CHIPCert.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/GroupDataProvider.h>


#include "JFAPlatform.h"

#include <controller/CHIPCluster.h>
#include <controller/OperationalCredentialsDelegate.h>
#include <lib/support/logging/CHIPLogging.h>


using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Controller;
using namespace chip::Crypto;
using namespace chip::app::Clusters::JointFabricAdministrator;

constexpr uint8_t kJFAvailableShift = 0;
constexpr uint8_t kJFAdminShift     = 1;
constexpr uint8_t kJFAnchorShift    = 2;
constexpr uint8_t kJFDatastoreShift = 3;

constexpr chip::EndpointId kJointFabricAdminEndpointId = 1;

CHIP_ERROR JFAManagerBase::Init(Server & server)
{
    mServer             = &server;
    mCASESessionManager = server.GetCASESessionManager();

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFAManagerBase::GetJointFabricMode(uint8_t & jointFabricMode)
{
    jointFabricMode = ((IsDeviceCommissioned() ? 0 : 1) << kJFAvailableShift) |
        (IsDeviceCommissioned() ? (IsDeviceJFAdmin() ? (1 << kJFAdminShift) : 0) : 0) |
        (IsDeviceCommissioned() ? (IsDeviceJFAnchor() ? (1 << kJFAnchorShift) : 0) : 0) |
        (IsDeviceCommissioned() ? (1 << kJFDatastoreShift) : 0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR JFAManagerBase::FinalizeCommissioning(NodeId nodeId, bool isJCM, P256PublicKey & trustedIcacPublicKeyB,
                                                 uint16_t endpointId)
{
    if (jfFabricIndex == kUndefinedFabricId)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogProgress(JointFabric, "FinalizeCommissioning for NodeID: 0x" ChipLogFormatX64 ", isJCM = %d, peerEndpointId = %d",
                    ChipLogValueX64(nodeId), isJCM, endpointId);

    peerAdminJFAdminClusterEndpointId = endpointId;
    peerAdminICACPubKey               = trustedIcacPublicKeyB;

    // Select the fabric index on which to reach the just-commissioned device.
    //
    // Before node migration, JFA-B anchors its own fabric and devices are commissioned into it,
    // so the local jfFabricIndex is correct.
    //
    // After JFA-B/JFC-B have been onboarded into the Joint Fabric and node migration has finished,
    // JFC-B commissions new devices into Fabric A (the joint fabric) using the JF materials. The
    // device's operational identity then lives on Fabric A, so JFA-B must finalize over its own
    // Fabric-A fabric index (mPeerJFAFabricIndex, the fabric on which JFA-A invoked
    // AnnounceJointFabricAdministrator), not its locally-anchored jfFabricIndex. Using the wrong
    // fabric index derives the wrong compressed fabric id and mDNS resolution of the device times out.
    FabricIndex finalizeFabricIndex = jfFabricIndex;
    if (mPeerJFAFabricIndex != kUndefinedFabricIndex)
    {
        finalizeFabricIndex = mPeerJFAFabricIndex;
        ChipLogProgress(JointFabric,
                        "FinalizeCommissioning: using Joint Fabric A index %u (JF materials active) instead of local index %u",
                        static_cast<unsigned>(mPeerJFAFabricIndex), static_cast<unsigned>(jfFabricIndex));
    }

    ScopedNodeId scopedNodeId = ScopedNodeId(nodeId, finalizeFabricIndex);

    // Remember the node just commissioned so that, once CommissioningComplete succeeds, it can be
    // registered in the anchor JFA-A's authoritative JointFabricDatastore NodeList when this JFA-B
    // is operating inside the Joint Fabric (post node-migration).
    mNewNodeId = nodeId;

    if (!isJCM)
    {
        // Add the node as pending in the datastore before connecting.
        char friendlyNameBuf[32];

        int written =
            snprintf(friendlyNameBuf, sizeof(friendlyNameBuf), "node-0x" ChipLogFormatX64, ChipLogValueX64(nodeId));
        CharSpan friendlyName(friendlyNameBuf, static_cast<size_t>(written));
        CHIP_ERROR addErr = Server::GetInstance().GetJointFabricDatastore().AddPendingNode(nodeId, friendlyName);
        if (addErr == CHIP_NO_ERROR)
        {
            ChipLogProgress(JointFabric, "FinalizeCommissioning: added pending node 0x" ChipLogFormatX64 " to NodeList",
                            ChipLogValueX64(nodeId));
        }
        else
        {
            ChipLogError(JointFabric, "FinalizeCommissioning: AddPendingNode failed: %" CHIP_ERROR_FORMAT, addErr.Format());
        }
    }

    ConnectToNode(scopedNodeId, isJCM ? kJCMCommissioning : kStandardCommissioningComplete);

    return CHIP_NO_ERROR;
}

void JFAManagerBase::SetJFARpc(JFARpc & aJFARpc)
{
    mJFARpc = &aJFARpc;
}

JFARpc * JFAManagerBase::GetJFARpc()
{
    return mJFARpc;
}

void JFAManagerBase::HandleCommissioningCompleteEvent()
{
    // If this device just completed JCM as a commissionee (JFA-B) and we learned the peer JFA-A
    // NodeId, migrate every Fabric-B device from the local datastore NodeList into Fabric A.
    if (GetPeerJFANodeId() != kUndefinedNodeId)
    {
        ChipLogProgress(JointFabric, "HandleCommissioningCompleteEvent: JCM done, starting device migration to Fabric A");
        StartDeviceMigration();
    }

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

                // Add the GroupKeySet entry (GroupKeySetID=0 is the IPK per spec 11.24.6.5).
                Clusters::JointFabricDatastore::Structs::DatastoreGroupKeySetStruct::Type defaultGroupKeySetEntry{ 0 };

                // Fetch the real Fabric-A IPK from the GroupDataProvider and store it in
                // epochKey0 so the datastore entry carries the actual key material.
                {
                    Credentials::GroupDataProvider * gdp = Server::GetInstance().GetGroupDataProvider();
                    if (gdp != nullptr)
                    {
                        Credentials::GroupDataProvider::KeySet ipkKeyset;
                        CHIP_ERROR ipkErr = gdp->GetIpkKeySet(fabricIndex, ipkKeyset);
                        if (ipkErr == CHIP_NO_ERROR && ipkKeyset.num_keys_used > 0)
                        {
                            static_assert(sizeof(ipkKeyset.epoch_keys[0].key) == CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES,
                                          "IPK epoch key size mismatch");
                            memcpy(mIPKBuf, ipkKeyset.epoch_keys[0].key, CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
                            mIPKLen = CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES;

                            // Populate epochKey0 in the datastore entry.
                            defaultGroupKeySetEntry.epochKey0.SetNonNull(
                                ByteSpan(mIPKBuf, mIPKLen));
                            defaultGroupKeySetEntry.epochStartTime0.SetNonNull(
                                ipkKeyset.epoch_keys[0].start_time);

                            ChipLogProgress(JointFabric, "HandleCommissioningCompleteEvent: fetched IPK (%u bytes) for GroupKeySet 0",
                                            static_cast<unsigned>(mIPKLen));
                        }
                        else
                        {
                            ChipLogError(JointFabric, "HandleCommissioningCompleteEvent: GetIpkKeySet failed: %" CHIP_ERROR_FORMAT,
                                         ipkErr.Format());
                        }
                    }
                }

                LogErrorOnFailure(Server::GetInstance().GetJointFabricDatastore().AddGroupKeySetEntry(defaultGroupKeySetEntry));
                LogErrorOnFailure(Server::GetInstance().GetJointFabricDatastore().ForceAddNodeKeySetEntry(0, nodeId));

                // Add default group entry for the JFA itself
                // Uses internal method that bypasses CAT restrictions since JFA setup needs both Admin and Anchor CATs
                Clusters::JointFabricDatastore::Commands::AddGroup::DecodableType addGroupCommandData;
                addGroupCommandData.groupID       = 0;
                addGroupCommandData.friendlyName  = "Default JFA Group Admin"_span;
                addGroupCommandData.groupKeySetID = 0;
                addGroupCommandData.groupCAT      = kAdminCATIdentifier; // Use Admin CAT for default group
                addGroupCommandData.groupCATVersion.SetNonNull(1);
                addGroupCommandData.groupPermission =
                    Clusters::JointFabricDatastore::DatastoreAccessControlEntryPrivilegeEnum::kAdminister;
                LogErrorOnFailure(Server::GetInstance().GetJointFabricDatastore().ForceAddGroup(addGroupCommandData));
                addGroupCommandData.friendlyName  = "Default JFA Group Anchor"_span;
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
                    CHIP_ERROR err = JFAInitCommissioner(fb.GetFabricId(), fabricIndex);
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

bool JFAManagerBase::IsDeviceJFAdmin()
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

bool JFAManagerBase::IsDeviceJFAnchor()
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

void JFAManagerBase::ReleaseSession()
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

void JFAManagerBase::ConnectToNode(ScopedNodeId scopedNodeId, OnConnectedAction onConnectedAction)
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

void JFAManagerBase::OnConnected(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);

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
        TEMPORARY_RETURN_IGNORED jfaManager->AnnounceJointFabricAdministrator();
        break;
    }
    case kDeviceMigration: {
        // Session established to a Fabric-B device; begin the AddNOC migration flow by arming
        // the fail-safe on the device.
        TEMPORARY_RETURN_IGNORED jfaManager->SendArmFailSafe();
        break;
    }
    case kDatastoreAddPendingNode: {
        // Session established to the peer JFA-A; register the migrating device as pending in
        // JFA-A's datastore before connecting to the device itself.
        TEMPORARY_RETURN_IGNORED jfaManager->SendDatastoreAddPendingNode();
        break;
    }
    case kMigrationCommissioningComplete: {
        // Reconnected to the migrated device on the newly installed Fabric A; send
        // CommissioningComplete over this Fabric-A CASE session to finalize the migration.
        TEMPORARY_RETURN_IGNORED jfaManager->SendMigrationCommissioningComplete();
        break;
    }
    case kReadAdminCAT: {
        // Connected to the peer JFA-A at the start of migration; read the fabric-wide
        // Administrator CAT from JFA-A's GroupList once, before migrating any device.
        CHIP_ERROR err = jfaManager->ReadPeerGroupList();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(JointFabric, "kReadAdminCAT: ReadPeerGroupList failed: %" CHIP_ERROR_FORMAT, err.Format());
            jfaManager->ReleaseSession();
        }
        break;
    }
    case kRegisterNewNodeInJFADatastore: {
        // Session established to the peer JFA-A (the anchor) after this JFA-B commissioned a new
        // device into Fabric A. Register the new node in JFA-A's authoritative datastore NodeList
        // via AddPendingNode, then RefreshNode to commit it.
        TEMPORARY_RETURN_IGNORED jfaManager->SendRegisterNewNodeAddPendingNode();
        break;
    }

    default:
        // Subclass-defined action; let the platform-specific manager handle it.
        jfaManager->DispatchExtendedConnectedAction(jfaManager->mOnConnectedAction);
        break;
    }
}

void JFAManagerBase::OnConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogError(JointFabric, "Failed to establish connection to 0x" ChipLogFormatX64 " on fabric index %d",
                 ChipLogValueX64(peerId.GetNodeId()), peerId.GetFabricIndex());

    jfaManager->ReleaseSession();
}

CHIP_ERROR JFAManagerBase::AnnounceJointFabricAdministrator()
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

void JFAManagerBase::OnAnnounceJointFabricAdministratorResponse(void * context, const chip::app::DataModel::NullObjectType & data)
{
    JFAManagerBase * jfaManagerCore = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);

    ChipLogProgress(JointFabric, "OnAnnounceJointFabricAdministratorResponse");

    /* TODO: https://github.com/project-chip/connectedhomeip/issues/38202 */
    TEMPORARY_RETURN_IGNORED jfaManagerCore->SendICACSRRequest();
}

void JFAManagerBase::OnAnnounceJointFabricAdministratorFailure(void * context, CHIP_ERROR error)
{
    JFAManagerBase * jfaManagerCore = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);
    jfaManagerCore->ReleaseSession();

    ChipLogError(JointFabric, "OnAnnounceJointFabricAdministratorFailure: %s\n", chip::ErrorStr(error));
}

CHIP_ERROR JFAManagerBase::SendICACSRRequest()
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

void JFAManagerBase::OnSendICACSRRequestResponse(void * context, const Commands::ICACCSRResponse::DecodableType & icaccsr)
{
    JFAManagerBase * jfaManagerCore = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);
    P256PublicKey pubKey;

    ChipLogProgress(JointFabric, "OnSendICACSRRequestResponse");

    if (!icaccsr.icaccsr.HasValue())
    {
        ChipLogError(JointFabric, "OnSendICACSRRequestResponse: missing ICAC CSR");
        return;
    }

    ByteSpan csrSpan = icaccsr.icaccsr.Value();

    if (CHIP_NO_ERROR != VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), pubKey))
    {
        ChipLogError(JointFabric, "OnSendICACSRRequestResponse: CSR verification failed");
        return;
    }

    if (!jfaManagerCore->peerAdminICACPubKey.Matches(pubKey))
    {
        ChipLogError(JointFabric, "OnSendICACSRRequestResponse: CSR public key mismatch");
        return;
    }

    // Save the commissionee CSR for cross-signing.
    if (csrSpan.size() > sizeof(jfaManagerCore->mPendingICACSRBuf))
    {
        ChipLogError(JointFabric, "OnSendICACSRRequestResponse: CSR too large (%u bytes)", static_cast<unsigned>(csrSpan.size()));
        return;
    }
    memcpy(jfaManagerCore->mPendingICACSRBuf, csrSpan.data(), csrSpan.size());
    jfaManagerCore->mPendingICACSRLen = csrSpan.size();

    ChipLogProgress(JointFabric, "OnSendICACSRRequestResponse: validated ICAC CSR, requesting cross-signing");
    TEMPORARY_RETURN_IGNORED jfaManagerCore->CrossSignICAC();
}

CHIP_ERROR JFAManagerBase::CrossSignICAC()
{
    JFARpc * jfaRpc = GetJFARpc();
    VerifyOrReturnError(jfaRpc != nullptr, CHIP_ERROR_UNINITIALIZED);
    VerifyOrReturnError(mPendingICACSRLen > 0, CHIP_ERROR_INCORRECT_STATE);

    // Obtain the anchor fabric ID from the fabric table so the JFC can bind
    // the ICAC to the correct anchor fabric.
    FabricId anchorFabricId = kUndefinedFabricId;
    for (const auto & fb : mServer->GetFabricTable())
    {
        if (fb.GetFabricIndex() == jfFabricIndex)
        {
            anchorFabricId = fb.GetFabricId();
            break;
        }
    }
    VerifyOrReturnError(anchorFabricId != kUndefinedFabricId, CHIP_ERROR_INCORRECT_STATE);

    MutableByteSpan icacSpan(mCrossSignedICACBuf, sizeof(mCrossSignedICACBuf));
    ByteSpan csrSpan(mPendingICACSRBuf, mPendingICACSRLen);

    CHIP_ERROR err = jfaRpc->GetCrossSignedICAC(anchorFabricId, csrSpan, icacSpan);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(JointFabric, "CrossSignICAC: GetCrossSignedICAC failed: %s", chip::ErrorStr(err));
        return err;
    }

    mCrossSignedICACLen = icacSpan.size();
    ChipLogProgress(JointFabric, "CrossSignICAC: received cross-signed ICAC (%u bytes)",
                    static_cast<unsigned>(mCrossSignedICACLen));

    return SendAddICAC();
}

CHIP_ERROR JFAManagerBase::SendAddICAC()
{
    Commands::AddICAC::Type request;

    if (!mExchangeMgr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    VerifyOrReturnError(mCrossSignedICACLen > 0, CHIP_ERROR_INCORRECT_STATE);

    request.ICACValue = ByteSpan(mCrossSignedICACBuf, mCrossSignedICACLen);

    ChipLogProgress(JointFabric, "SendAddICAC: invoke cluster command (%u bytes)",
                    static_cast<unsigned>(mCrossSignedICACLen));

    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), peerAdminJFAdminClusterEndpointId);
    return cluster.InvokeCommand(request, this, OnSendAddICACResponse, OnSendAddICACFailure);
}

void JFAManagerBase::OnSendAddICACResponse(void * context,
                                       const Commands::ICACResponse::DecodableType & response)
{
    JFAManagerBase * jfaManagerCore = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);

    ChipLogProgress(JointFabric, "OnSendAddICACResponse: statusCode=%u", to_underlying(response.statusCode));

    if (response.statusCode != chip::app::Clusters::JointFabricAdministrator::ICACResponseStatusEnum::kOk)
    {
        ChipLogError(JointFabric, "OnSendAddICACResponse: commissionee rejected ICAC (statusCode=%u)",
                     to_underlying(response.statusCode));
        jfaManagerCore->ReleaseSession();
        return;
    }

    ChipLogProgress(JointFabric, "OnSendAddICACResponse: ICAC accepted, proceeding to CommissioningComplete");

    TEMPORARY_RETURN_IGNORED jfaManagerCore->SendCommissioningComplete();
}

void JFAManagerBase::OnSendAddICACFailure(void * context, CHIP_ERROR error)
{
    JFAManagerBase * jfaManagerCore = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);
    jfaManagerCore->ReleaseSession();

    ChipLogError(JointFabric, "OnSendAddICACFailure: %s\n", chip::ErrorStr(error));
}

void JFAManagerBase::OnSendICACSRRequestFailure(void * context, CHIP_ERROR error)
{
    JFAManagerBase * jfaManagerCore = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);
    jfaManagerCore->ReleaseSession();

    ChipLogError(JointFabric, "OnSendICACSRRequestFailure: %s\n", chip::ErrorStr(error));
}

CHIP_ERROR JFAManagerBase::SendCommissioningComplete()
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

void JFAManagerBase::OnCommissioningCompleteResponse(
    void * context, const GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogProgress(JointFabric, "OnCommissioningCompleteResponse, Code=%u", to_underlying(data.errorCode));

    if (data.errorCode != GeneralCommissioning::CommissioningErrorEnum::kOk)
    {
        ChipLogProgress(JointFabric, "Commssioning Failed (nodeId=%ld, isJCM = %d), Code=%u", jfaManager->mNodeId,
                        jfaManager->mOnConnectedAction == kJCMCommissioning, to_underlying(data.errorCode));
    }
    else
    {
        switch (jfaManager->mOnConnectedAction)
        {
        case kStandardCommissioningComplete: {
            ChipLogProgress(JointFabric, "Standard Commissioning (nodeId=%ld) success", jfaManager->mNodeId);
            {
                CHIP_ERROR refreshErr =
                    Server::GetInstance().GetJointFabricDatastore().RefreshNode(jfaManager->mNodeId);
                if (refreshErr == CHIP_NO_ERROR)
                {
                    ChipLogProgress(JointFabric, "OnCommissioningCompleteResponse: node 0x" ChipLogFormatX64
                                    " committed in NodeList",
                                    ChipLogValueX64(jfaManager->mNodeId));
                }
                else
                {
                    ChipLogError(JointFabric, "OnCommissioningCompleteResponse: RefreshNode failed: %" CHIP_ERROR_FORMAT,
                                 refreshErr.Format());
                }
            }

            // Let subclasses react to a successfully onboarded commissionee; default implementation is a no-op.
            jfaManager->OnCommissioneeOnboarded(ScopedNodeId(jfaManager->mNodeId, jfaManager->jfFabricIndex));
            // If this JFA-B is operating inside the Joint Fabric (post node-migration), the device
            // was commissioned into Fabric A, so it must also be registered in the anchor JFA-A's
            // authoritative datastore NodeList, not only in this JFA-B's local datastore. Release
            // the device session and connect to the peer JFA-A to run the AddPendingNode/RefreshNode
            // registration flow. Return early so the trailing generic ReleaseSession() below does
            // not tear down the session we are about to establish.
            if (jfaManager->mPeerJFAFabricIndex != kUndefinedFabricIndex &&
                jfaManager->mPeerJFANodeId != kUndefinedNodeId)
            {
                ChipLogProgress(JointFabric,
                                "OnCommissioningCompleteResponse: registering new node 0x" ChipLogFormatX64
                                " in anchor JFA-A datastore (peer 0x" ChipLogFormatX64 ")",
                                ChipLogValueX64(jfaManager->mNewNodeId), ChipLogValueX64(jfaManager->mPeerJFANodeId));
                jfaManager->ReleaseSession();
                ScopedNodeId peerJFA(jfaManager->mPeerJFANodeId, jfaManager->mPeerJFAFabricIndex);
                jfaManager->ConnectToNode(peerJFA, kRegisterNewNodeInJFADatastore);
                return;
            }
            break;
        }

        case kJCMCommissioning: {
            ChipLogProgress(JointFabric, "Joint Commissioning Method (nodeId=%ld) success", jfaManager->mNodeId);

            // Add commissionee to the datastore AdminList now that CommissioningComplete succeeded.
            {
                Clusters::JointFabricDatastore::Structs::DatastoreAdministratorInformationEntryStruct::Type newAdmin;
                newAdmin.nodeID   = jfaManager->mNodeId;
                newAdmin.vendorID = VendorId::NotSpecified;
                newAdmin.icac     = ByteSpan(jfaManager->mCrossSignedICACBuf, jfaManager->mCrossSignedICACLen);

                char friendlyNameBuf[32];
                int written = snprintf(friendlyNameBuf, sizeof(friendlyNameBuf), "JFA-B-0x" ChipLogFormatX64,
                                       ChipLogValueX64(jfaManager->mNodeId));
                newAdmin.friendlyName = CharSpan(friendlyNameBuf, static_cast<size_t>(written));

                CHIP_ERROR addErr = Server::GetInstance().GetJointFabricDatastore().AddAdmin(newAdmin);
                if (addErr == CHIP_NO_ERROR)
                {
                    ChipLogProgress(JointFabric, "OnCommissioningCompleteResponse: added commissionee 0x" ChipLogFormatX64
                                    " to AdminList", ChipLogValueX64(jfaManager->mNodeId));
                }
                else
                {
                    ChipLogError(JointFabric, "OnCommissioningCompleteResponse: AddAdmin failed: %" CHIP_ERROR_FORMAT,
                                 addErr.Format());
                }
            }
            break;
        }
        case kDeviceMigration: {
            // Device migration finalizes through OnMigrationCommissioningCompleteResponse, so this
            // path is not expected here. Handled explicitly to satisfy -Werror=switch.
            break;
        }
        case kDatastoreAddPendingNode: {
            // AddPendingNode to JFA-A's datastore finalizes through OnDatastoreAddPendingNodeResponse,
            // not through CommissioningComplete. Handled explicitly to satisfy -Werror=switch.
            break;
        }
        case kMigrationCommissioningComplete: {
            // Device migration finalizes through OnMigrationCommissioningCompleteResponse, not this
            // standard-commissioning path. Handled explicitly to satisfy -Werror=switch.
            break;
        }
        case kReadAdminCAT: {
            // The Admin CAT read does not send CommissioningComplete, so this path is not reached.
            // Handled explicitly to satisfy -Werror=switch.
            break;
        }
        case kRegisterNewNodeInJFADatastore: {
            // Registering the new node in JFA-A's datastore finalizes through
            // OnRegisterNewNodeAddPendingResponse/OnRegisterNewNodeRefreshResponse, not through
            // CommissioningComplete. Handled explicitly to satisfy -Werror=switch.
            break;
        }
        default:
            break;
        }




    }

    jfaManager->ReleaseSession();
}

void JFAManagerBase::OnCommissioningCompleteFailure(void * context, CHIP_ERROR error)
{
    JFAManagerBase * jfaManagerCore = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManagerCore != nullptr);
    jfaManagerCore->ReleaseSession();

    ChipLogError(JointFabric, "Received failure response %s\n", chip::ErrorStr(error));
}

CHIP_ERROR JFAManagerBase::GetIcacCsr(MutableByteSpan & icacCsr)
{
    JFARpc * jfaRpc = GetJFARpc();

    if (jfaRpc)
    {
        return jfaRpc->GetICACCSRForJF(icacCsr);
    }

    return CHIP_ERROR_UNINITIALIZED;
}

CHIP_ERROR JFAManagerBase::StoreCrossSignedICAC(ByteSpan icac)
{
    // Called by HandleAddICAC after the commissionee accepts the cross-signed ICAC.
    // Nothing extra to persist at this layer; the cluster handler already validated it.
    ChipLogProgress(JointFabric, "StoreCrossSignedICAC: ICAC stored (%u bytes)", static_cast<unsigned>(icac.size()));
    return CHIP_NO_ERROR;
}

CHIP_ERROR JFAManagerBase::ClearJFMaterials()
{
    // Discard the locally cached cross-signed ICAC.
    mCrossSignedICACLen = 0;
    memset(mCrossSignedICACBuf, 0, sizeof(mCrossSignedICACBuf));

    // Tell JFC-B to discard its copy.
    JFARpc * jfaRpc = GetJFARpc();
    if (jfaRpc == nullptr)
    {
        ChipLogError(JointFabric, "ClearJFMaterials: JFARpc not available");
        return CHIP_ERROR_UNINITIALIZED;
    }

    CHIP_ERROR err = jfaRpc->ClearJFMaterials();
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(JointFabric, "ClearJFMaterials: JF materials cleared on JFC-B");
    }
    else
    {
        ChipLogError(JointFabric, "ClearJFMaterials: RPC failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

void JFAManagerBase::SetPeerJFANodeId(NodeId nodeId, FabricIndex fabricIndex)
{
    mPeerJFANodeId      = nodeId;
    mPeerJFAFabricIndex = fabricIndex;
    ChipLogProgress(JointFabric, "SetPeerJFANodeId: peer JFA-A NodeId=0x" ChipLogFormatX64 " on fabric index %u",
                    ChipLogValueX64(nodeId), static_cast<unsigned>(fabricIndex));
}

void JFAManagerBase::ClearPeerJFANodeId()
{
    if (mPeerJFANodeId != kUndefinedNodeId)
    {
        ChipLogProgress(JointFabric, "ClearPeerJFANodeId: clearing peer JFA-A NodeId=0x" ChipLogFormatX64,
                        ChipLogValueX64(mPeerJFANodeId));
    }
    mPeerJFANodeId      = kUndefinedNodeId;
    mPeerJFAFabricIndex = kUndefinedFabricIndex;
}


CHIP_ERROR JFAManagerBase::PushJFMaterials(ByteSpan icac, ByteSpan rcac, ByteSpan ipk)
{
    // Forward the Fabric-A JF materials to the local commissioner (JFC-B) so it
    // can issue Fabric-A NOCs for Fabric-B devices being migrated to the joint fabric.
    JFARpc * jfaRpc = GetJFARpc();
    if (jfaRpc == nullptr)
    {
        ChipLogError(JointFabric, "PushJFMaterials: JFARpc not available");
        return CHIP_ERROR_UNINITIALIZED;
    }

    CHIP_ERROR err = jfaRpc->PushJFMaterials(icac, rcac, ipk);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(JointFabric, "PushJFMaterials: JF materials forwarded to local commissioner (%u ICAC, %u RCAC, %u IPK bytes)",
                        static_cast<unsigned>(icac.size()), static_cast<unsigned>(rcac.size()),
                        static_cast<unsigned>(ipk.size()));
    }
    else
    {
        ChipLogError(JointFabric, "PushJFMaterials: PushJFMaterials RPC failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

// --- Device migration (Fabric B -> Fabric A via AddNOC) ---

void JFAManagerBase::StartDeviceMigration()
{
    // Snapshot every node currently in the local (Fabric-B) datastore NodeList so that we can
    // iterate even as we remove entries from the datastore during migration.
    mMigrationNodeList.clear();
    mMigrationIndex = 0;

    const auto & nodeEntries = Server::GetInstance().GetJointFabricDatastore().GetNodeInformationEntries();
    for (const auto & entry : nodeEntries)
    {
        if (entry.nodeID != kUndefinedNodeId)
        {
            mMigrationNodeList.push_back(entry.nodeID);
        }
    }

    // Resolve the anchor (Fabric A) fabric id so JFC-B can bind issued NOCs to the correct fabric.
    mAnchorFabricId = kUndefinedFabricId;
    for (const auto & fb : mServer->GetFabricTable())
    {
        if (fb.GetFabricIndex() == jfFabricIndex)
        {
            mAnchorFabricId = fb.GetFabricId();
            break;
        }
    }

    ChipLogProgress(JointFabric, "StartDeviceMigration: %u Fabric-B device(s) to migrate into Fabric A (anchorFabricId=0x" ChipLogFormatX64 ")",
                    static_cast<unsigned>(mMigrationNodeList.size()), ChipLogValueX64(mAnchorFabricId));

    if (mAnchorFabricId == kUndefinedFabricId)
    {
        ChipLogError(JointFabric, "StartDeviceMigration: anchor fabric id unknown, aborting migration");
        return;
    }

    // The Administrator CAT is a fabric-wide value held in JFA-A's JointFabricDatastore GroupList;
    // it is identical for every device being migrated. Read it exactly once up front (before any
    // device is added as pending or migrated) by connecting to the peer JFA-A with the kReadAdminCAT
    // action. The migration loop only starts once mAdminCAT has been resolved to a valid value.
    if (mPeerJFANodeId == kUndefinedNodeId)
    {
        ChipLogError(JointFabric, "StartDeviceMigration: peer JFA-A NodeId unknown, cannot read Administrator CAT");
        return;
    }
    if (mPeerJFAFabricIndex == kUndefinedFabricIndex)
    {
        ChipLogError(JointFabric, "StartDeviceMigration: peer JFA-A fabric index unknown, cannot reach JFA-A");
        return;
    }

    mAdminCAT = kUndefinedCAT;
    ScopedNodeId peerJFA(mPeerJFANodeId, mPeerJFAFabricIndex);
    ConnectToNode(peerJFA, kReadAdminCAT);
}


void JFAManagerBase::MigrateNextNode()
{
    if (mMigrationIndex >= mMigrationNodeList.size())
    {
        ChipLogProgress(JointFabric, "MigrateNextNode: all Fabric-B devices migrated into Fabric A");
        mMigratingNodeId = kUndefinedNodeId;
        return;
    }

    mMigratingNodeId = mMigrationNodeList[mMigrationIndex];

    ChipLogProgress(JointFabric, "MigrateNextNode: migrating device %u/%u, nodeId=0x" ChipLogFormatX64,
                    static_cast<unsigned>(mMigrationIndex + 1), static_cast<unsigned>(mMigrationNodeList.size()),
                    ChipLogValueX64(mMigratingNodeId));

    // The device must be registered as pending in the anchor (JFA-A) datastore, not in this
    // local JFA-B datastore where it already exists. Connect to the peer JFA-A first and issue
    // a JointFabricDatastore::AddPendingNode cluster command there; the device migration itself
    // continues once that command succeeds.
    if (mPeerJFANodeId == kUndefinedNodeId)
    {
        ChipLogError(JointFabric, "MigrateNextNode: peer JFA-A NodeId unknown, cannot add pending node");
        FinishCurrentMigration(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    // The session to JFA-A must use the fabric index on which JFA-A invoked
    // AnnounceJointFabricAdministrator (the joint fabric), not this device's own
    // locally-commissioned fabric index (jfFabricIndex). Using the wrong fabric
    // index derives the wrong compressed fabric id and mDNS resolution times out.
    if (mPeerJFAFabricIndex == kUndefinedFabricIndex)
    {
        ChipLogError(JointFabric, "MigrateNextNode: peer JFA-A fabric index unknown, cannot reach JFA-A");
        FinishCurrentMigration(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    ScopedNodeId peerJFA(mPeerJFANodeId, mPeerJFAFabricIndex);
    ConnectToNode(peerJFA, kDatastoreAddPendingNode);

}

CHIP_ERROR JFAManagerBase::SendDatastoreAddPendingNode()
{
    Clusters::JointFabricDatastore::Commands::AddPendingNode::Type request;

    if (!mExchangeMgr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    request.nodeID = mMigratingNodeId;

    char friendlyNameBuf[32];
    int written = snprintf(friendlyNameBuf, sizeof(friendlyNameBuf), "Migrated-0x" ChipLogFormatX64, ChipLogValueX64(mMigratingNodeId));
    request.friendlyName = CharSpan(friendlyNameBuf, static_cast<size_t>(written));

    ChipLogProgress(JointFabric,
                    "SendDatastoreAddPendingNode: add node 0x" ChipLogFormatX64 " as pending in JFA-A datastore (peer 0x" ChipLogFormatX64 ")",
                    ChipLogValueX64(mMigratingNodeId), ChipLogValueX64(mPeerJFANodeId));

    // The JointFabricDatastore cluster lives on the JFA admin endpoint of the peer JFA-A.
    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), kJointFabricAdminEndpointId);
    return cluster.InvokeCommand(request, this, OnDatastoreAddPendingNodeResponse, OnDatastoreAddPendingNodeFailure);
}

void JFAManagerBase::OnDatastoreAddPendingNodeResponse(void * context, const chip::app::DataModel::NullObjectType &)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogProgress(JointFabric, "OnDatastoreAddPendingNodeResponse: node 0x" ChipLogFormatX64 " added as pending in JFA-A datastore",
                    ChipLogValueX64(jfaManager->mMigratingNodeId));

    // The fabric-wide Administrator CAT was already resolved once up front (kReadAdminCAT) before
    // the migration loop started, so mAdminCAT is valid here. Proceed directly to the device
    // migration: release the JFA-A session and connect to the device to run the AddNOC flow.
    jfaManager->ContinueMigrationAfterGroupList();
}



void JFAManagerBase::OnDatastoreAddPendingNodeFailure(void * context, CHIP_ERROR error)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogError(JointFabric, "OnDatastoreAddPendingNodeFailure: %s", chip::ErrorStr(error));
    jfaManager->FinishCurrentMigration(error);
}

// --- Post-migration new-node registration in the anchor JFA-A datastore ---
// Register the newly commissioned Fabric-A node (mNewNodeId) as pending in the peer JFA-A's
// authoritative JointFabricDatastore NodeList.
CHIP_ERROR JFAManagerBase::SendRegisterNewNodeAddPendingNode()
{
    if (!mExchangeMgr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    Clusters::JointFabricDatastore::Commands::AddPendingNode::Type request;
    request.nodeID = mNewNodeId;

    char friendlyNameBuf[32];
    int written =
        snprintf(friendlyNameBuf, sizeof(friendlyNameBuf), "node-0x" ChipLogFormatX64, ChipLogValueX64(mNewNodeId));
    request.friendlyName = CharSpan(friendlyNameBuf, static_cast<size_t>(written));

    ChipLogProgress(JointFabric,
                    "SendRegisterNewNodeAddPendingNode: add node 0x" ChipLogFormatX64
                    " as pending in anchor JFA-A datastore",
                    ChipLogValueX64(mNewNodeId));

    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), kJointFabricAdminEndpointId);
    return cluster.InvokeCommand(request, this, OnRegisterNewNodeAddPendingResponse, OnRegisterNewNodeAddPendingFailure);
}

void JFAManagerBase::OnRegisterNewNodeAddPendingResponse(void * context, const chip::app::DataModel::NullObjectType &)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogProgress(JointFabric, "OnRegisterNewNodeAddPendingResponse: node added as pending, refreshing node");

    CHIP_ERROR err = jfaManager->SendRegisterNewNodeRefreshNode();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(JointFabric, "OnRegisterNewNodeAddPendingResponse: SendRegisterNewNodeRefreshNode failed: %s",
                     chip::ErrorStr(err));
        jfaManager->ReleaseSession();
    }
}

void JFAManagerBase::OnRegisterNewNodeAddPendingFailure(void * context, CHIP_ERROR error)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogError(JointFabric, "OnRegisterNewNodeAddPendingFailure: %s", chip::ErrorStr(error));
    jfaManager->ReleaseSession();
}

// Commit the pending node in the peer JFA-A's datastore via RefreshNode.
CHIP_ERROR JFAManagerBase::SendRegisterNewNodeRefreshNode()
{
    if (!mExchangeMgr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    Clusters::JointFabricDatastore::Commands::RefreshNode::Type request;
    request.nodeID = mNewNodeId;

    ChipLogProgress(JointFabric,
                    "SendRegisterNewNodeRefreshNode: refresh node 0x" ChipLogFormatX64 " in anchor JFA-A datastore",
                    ChipLogValueX64(mNewNodeId));

    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), kJointFabricAdminEndpointId);
    return cluster.InvokeCommand(request, this, OnRegisterNewNodeRefreshResponse, OnRegisterNewNodeRefreshFailure);
}

void JFAManagerBase::OnRegisterNewNodeRefreshResponse(void * context, const chip::app::DataModel::NullObjectType &)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogProgress(JointFabric,
                    "OnRegisterNewNodeRefreshResponse: new node committed in anchor JFA-A datastore NodeList");
    jfaManager->ReleaseSession();
}

void JFAManagerBase::OnRegisterNewNodeRefreshFailure(void * context, CHIP_ERROR error)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogError(JointFabric, "OnRegisterNewNodeRefreshFailure: %s", chip::ErrorStr(error));
    jfaManager->ReleaseSession();
}

CHIP_ERROR JFAManagerBase::ReadPeerGroupList()
{
    if (!mExchangeMgr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    ChipLogProgress(JointFabric, "ReadPeerGroupList: reading JFA-A JointFabricDatastore GroupList");

    // The JointFabricDatastore cluster lives on the JFA admin endpoint of the peer JFA-A.
    // Decode the GroupList with the custom raw-TLV JFAGroupInfoRaw instead of the ZAP-generated
    // DecodableType: the generated struct declares groupCAT as Nullable<uint16_t>, which rejects
    // the Administrator CAT value 0xFFFF (reserved null sentinel) with CONSTRAINT_ERROR.
    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), kJointFabricAdminEndpointId);
    return cluster.ReadAttribute<app::DataModel::DecodableList<JFAGroupInfoRaw>,
                                 const app::DataModel::DecodableList<JFAGroupInfoRaw> &>(
        this, Clusters::JointFabricDatastore::Id, Clusters::JointFabricDatastore::Attributes::GroupList::Id,
        OnReadPeerGroupListSuccess, OnReadPeerGroupListFailure, true);
}

void JFAManagerBase::OnReadPeerGroupListSuccess(void * context, const app::DataModel::DecodableList<JFAGroupInfoRaw> & groupList)
{

    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    // Per spec 11.24.6.6, the GroupList contains (at minimum) one group whose GroupCAT equals the
    // Administrator CAT and one whose GroupCAT equals the Anchor CAT. Find the Administrator-CAT
    // group and build the full CAT from its GroupCAT (identifier) and GroupCATVersion (version).
    // The resolved CAT must exactly match the Administrator CAT embedded in the device NOC or the
    // migrated device denies CommissioningComplete with UNSUPPORTED_ACCESS, so this resolution is
    // strict: a matching, versioned (version > 0) Administrator-CAT group must be present.
    jfaManager->mAdminCAT = kUndefinedCAT;

    auto it = groupList.begin();
    while (it.Next())
    {
        const auto & entry = it.GetValue();

        if (entry.groupCATIsNull || entry.groupCAT != kAdminCATIdentifier)
        {
            continue;
        }

        uint16_t version = entry.groupCATVersionIsNull ? 0 : entry.groupCATVersion;
        jfaManager->mAdminCAT =
            (static_cast<CASEAuthTag>(kAdminCATIdentifier) << 16) | static_cast<CASEAuthTag>(version);

        ChipLogProgress(JointFabric,
                        "OnReadPeerGroupListSuccess: resolved Admin CAT 0x%08" PRIX32 " (version %u) from JFA-A GroupList",
                        static_cast<uint32_t>(jfaManager->mAdminCAT), static_cast<unsigned>(version));
        break;
    }

    // This GroupList read happens exactly once, up front, before any device is added as pending or
    // migrated. On any failure the whole migration is aborted (no device loop is started); on
    // success the JFA-A session is released and the per-device migration loop begins.
    if (it.GetStatus() != CHIP_NO_ERROR)
    {
        ChipLogError(JointFabric, "OnReadPeerGroupListSuccess: GroupList decode error: %" CHIP_ERROR_FORMAT
                     "; aborting migration", it.GetStatus().Format());
        jfaManager->ReleaseSession();
        return;
    }

    // A valid Administrator CAT requires a non-zero version. If no Administrator-CAT group was
    // found, or it carried version 0, we cannot build the caseAdminSubject the devices expect:
    // abort the whole migration rather than sending AddNOC with a mismatched subject.
    if (!IsValidCASEAuthTag(jfaManager->mAdminCAT))
    {
        ChipLogError(JointFabric,
                     "OnReadPeerGroupListSuccess: no valid Administrator CAT in JFA-A GroupList (resolved 0x%08" PRIX32
                     "); aborting migration",
                     static_cast<uint32_t>(jfaManager->mAdminCAT));
        jfaManager->ReleaseSession();
        return;
    }

    // Administrator CAT resolved; release the JFA-A session and start migrating the Fabric-B devices.
    jfaManager->ReleaseSession();
    jfaManager->MigrateNextNode();
}

void JFAManagerBase::OnReadPeerGroupListFailure(void * context, CHIP_ERROR error)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    // The Administrator CAT must be read from the JFA-A datastore once before migration starts; if
    // this up-front read fails we cannot determine the caseAdminSubject the devices expect, so abort
    // the whole migration before any device is touched.
    ChipLogError(JointFabric, "OnReadPeerGroupListFailure: %s; aborting migration", chip::ErrorStr(error));
    jfaManager->ReleaseSession();
}



void JFAManagerBase::ContinueMigrationAfterGroupList()
{
    // Done with the JFA-A session; now connect to the device itself and run the AddNOC flow.
    ReleaseSession();

    ScopedNodeId scopedNodeId(mMigratingNodeId, jfFabricIndex);
    ConnectToNode(scopedNodeId, kDeviceMigration);
}



CHIP_ERROR JFAManagerBase::SendArmFailSafe()
{
    GeneralCommissioning::Commands::ArmFailSafe::Type request;

    if (!mExchangeMgr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    request.expiryLengthSeconds = 60;
    request.breadcrumb          = 0;

    ChipLogProgress(JointFabric, "SendArmFailSafe: invoke cluster command for nodeId=0x" ChipLogFormatX64,
                    ChipLogValueX64(mMigratingNodeId));

    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), kRootEndpointId);
    return cluster.InvokeCommand(request, this, OnArmFailSafeResponse, OnArmFailSafeFailure);
}

void JFAManagerBase::OnArmFailSafeResponse(void * context,
                                       const GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogProgress(JointFabric, "OnArmFailSafeResponse: errorCode=%u", to_underlying(data.errorCode));

    if (data.errorCode != GeneralCommissioning::CommissioningErrorEnum::kOk)
    {
        ChipLogError(JointFabric, "OnArmFailSafeResponse: ArmFailSafe rejected (errorCode=%u)", to_underlying(data.errorCode));
        jfaManager->FinishCurrentMigration(CHIP_ERROR_INTERNAL);
        return;
    }

    TEMPORARY_RETURN_IGNORED jfaManager->SendCSRRequest();
}

void JFAManagerBase::OnArmFailSafeFailure(void * context, CHIP_ERROR error)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogError(JointFabric, "OnArmFailSafeFailure: %s", chip::ErrorStr(error));
    jfaManager->FinishCurrentMigration(error);
}

CHIP_ERROR JFAManagerBase::SendCSRRequest()
{
    OperationalCredentials::Commands::CSRRequest::Type request;

    if (!mExchangeMgr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    // A random CSR nonce is required by the server.
    uint8_t csrNonce[Controller::kCSRNonceLength];
    ReturnErrorOnFailure(Crypto::DRBG_get_bytes(csrNonce, sizeof(csrNonce)));
    request.CSRNonce = ByteSpan(csrNonce, sizeof(csrNonce));

    ChipLogProgress(JointFabric, "SendCSRRequest: invoke cluster command for nodeId=0x" ChipLogFormatX64,
                    ChipLogValueX64(mMigratingNodeId));

    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), kRootEndpointId);
    return cluster.InvokeCommand(request, this, OnCSRResponse, OnCSRFailure);
}

void JFAManagerBase::OnCSRResponse(void * context, const OperationalCredentials::Commands::CSRResponse::DecodableType & data)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogProgress(JointFabric, "OnCSRResponse for nodeId=0x" ChipLogFormatX64, ChipLogValueX64(jfaManager->mMigratingNodeId));

    // NOCSRElements is a TLV structure containing the actual CSR; the CSR itself is what JFC-B needs
    // to sign. The ExampleOperationalCredentialsIssuer::SignDeviceNOC path extracts/validates it.
    ByteSpan nocsrElements = data.NOCSRElements;

    ByteSpan csr;
    ByteSpan csrNonce;
    ByteSpan vendorReserved1;
    ByteSpan vendorReserved2;
    ByteSpan vendorReserved3;
    CHIP_ERROR err = Credentials::DeconstructNOCSRElements(nocsrElements, csr, csrNonce, vendorReserved1, vendorReserved2,
                                                           vendorReserved3);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(JointFabric, "OnCSRResponse: DeconstructNOCSRElements failed: %" CHIP_ERROR_FORMAT, err.Format());
        jfaManager->FinishCurrentMigration(err);
        return;
    }

    JFARpc * jfaRpc = jfaManager->GetJFARpc();
    if (jfaRpc == nullptr)
    {
        ChipLogError(JointFabric, "OnCSRResponse: JFARpc not available");
        jfaManager->FinishCurrentMigration(CHIP_ERROR_UNINITIALIZED);
        return;
    }

    // JFC-B returns the device NOC plus the JF materials it already holds: the cross-signed ICAC,
    // the Fabric-A RCAC, and the Fabric-A IPK. No fabric id is transmitted; JFC-B derives it
    // internally from the cross-signed ICAC. JFA-B uses the returned materials to populate the
    // AddTrustedRootCertificate (RCAC) and AddNOC (ICAC + IPK) commands.
    MutableByteSpan nocSpan(jfaManager->mDeviceNOCBuf, sizeof(jfaManager->mDeviceNOCBuf));
    MutableByteSpan icacSpan(jfaManager->mCrossSignedICACBuf, sizeof(jfaManager->mCrossSignedICACBuf));
    MutableByteSpan rcacSpan(jfaManager->mDeviceRCACBuf, sizeof(jfaManager->mDeviceRCACBuf));
    MutableByteSpan ipkSpan(jfaManager->mIPKBuf, sizeof(jfaManager->mIPKBuf));
    err = jfaRpc->GetNOCForDevice(jfaManager->mMigratingNodeId, csr, nocSpan, icacSpan, rcacSpan, ipkSpan);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(JointFabric, "OnCSRResponse: GetNOCForDevice failed: %" CHIP_ERROR_FORMAT, err.Format());
        jfaManager->FinishCurrentMigration(err);
        return;
    }

    jfaManager->mDeviceNOCLen      = nocSpan.size();
    jfaManager->mCrossSignedICACLen = icacSpan.size();
    jfaManager->mDeviceRCACLen     = rcacSpan.size();
    jfaManager->mIPKLen            = ipkSpan.size();
    ChipLogProgress(JointFabric,
                    "OnCSRResponse: obtained Fabric-A NOC (%u bytes), ICAC (%u bytes), RCAC (%u bytes), IPK (%u bytes), "
                    "proceeding to AddTrustedRootCertificate",
                    static_cast<unsigned>(jfaManager->mDeviceNOCLen), static_cast<unsigned>(jfaManager->mCrossSignedICACLen),
                    static_cast<unsigned>(jfaManager->mDeviceRCACLen), static_cast<unsigned>(jfaManager->mIPKLen));


    TEMPORARY_RETURN_IGNORED jfaManager->SendAddTrustedRootCertificate();
}

void JFAManagerBase::OnCSRFailure(void * context, CHIP_ERROR error)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogError(JointFabric, "OnCSRFailure: %s", chip::ErrorStr(error));
    jfaManager->FinishCurrentMigration(error);
}

CHIP_ERROR JFAManagerBase::SendAddTrustedRootCertificate()
{
    OperationalCredentials::Commands::AddTrustedRootCertificate::Type request;

    if (!mExchangeMgr)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    // Use the Fabric-A RCAC that JFC-B returned alongside the device NOC (via GetNOCForDevice),
    // not the locally cached anchor root; JFC-B is the authority for Fabric-A's trust material.
    ByteSpan anchorRCAC = ByteSpan(mDeviceRCACBuf, mDeviceRCACLen);
    VerifyOrReturnError(anchorRCAC.size() > 0, CHIP_ERROR_INCORRECT_STATE);

    request.rootCACertificate = anchorRCAC;

    ChipLogProgress(JointFabric, "SendAddTrustedRootCertificate: invoke cluster command (%u bytes) for nodeId=0x" ChipLogFormatX64,
                    static_cast<unsigned>(anchorRCAC.size()), ChipLogValueX64(mMigratingNodeId));

    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), kRootEndpointId);
    return cluster.InvokeCommand(request, this, OnAddTrustedRootResponse, OnAddTrustedRootFailure);
}

void JFAManagerBase::OnAddTrustedRootResponse(void * context, const chip::app::DataModel::NullObjectType &)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogProgress(JointFabric, "OnAddTrustedRootResponse: Fabric-A root accepted, proceeding to AddNOC");

    TEMPORARY_RETURN_IGNORED jfaManager->SendAddNOC();
}

void JFAManagerBase::OnAddTrustedRootFailure(void * context, CHIP_ERROR error)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogError(JointFabric, "OnAddTrustedRootFailure: %s", chip::ErrorStr(error));
    jfaManager->FinishCurrentMigration(error);
}

CHIP_ERROR JFAManagerBase::SendAddNOC()
{
    OperationalCredentials::Commands::AddNOC::Type request;

    if (!mExchangeMgr)
    {
        ChipLogError(JointFabric, "SendAddNOC: no active exchange");
        return CHIP_ERROR_UNINITIALIZED;
    }

    // Log the state of each required input up front so a silent early-return (the return value
    // is swallowed by TEMPORARY_RETURN_IGNORED at the call site) is visible in the trace.
    ChipLogProgress(JointFabric, "SendAddNOC: entry (NOC %u, ICAC %u, IPK %u bytes)",
                    static_cast<unsigned>(mDeviceNOCLen), static_cast<unsigned>(mCrossSignedICACLen),
                    static_cast<unsigned>(mIPKLen));

    if (mDeviceNOCLen == 0)
    {
        ChipLogError(JointFabric, "SendAddNOC: device NOC missing, aborting migration");
        FinishCurrentMigration(CHIP_ERROR_INCORRECT_STATE);
        return CHIP_ERROR_INCORRECT_STATE;
    }
    if (mCrossSignedICACLen == 0)
    {
        ChipLogError(JointFabric, "SendAddNOC: cross-signed ICAC missing, aborting migration");
        FinishCurrentMigration(CHIP_ERROR_INCORRECT_STATE);
        return CHIP_ERROR_INCORRECT_STATE;
    }
    if (mIPKLen == 0)
    {
        ChipLogError(JointFabric, "SendAddNOC: IPK missing, aborting migration");
        FinishCurrentMigration(CHIP_ERROR_INCORRECT_STATE);
        return CHIP_ERROR_INCORRECT_STATE;
    }


    request.NOCValue  = ByteSpan(mDeviceNOCBuf, mDeviceNOCLen);
    request.ICACValue.SetValue(ByteSpan(mCrossSignedICACBuf, mCrossSignedICACLen));
    request.IPKValue = ByteSpan(mIPKBuf, mIPKLen);

    // CaseAdminSubject is the Administrator CAT; AdminVendorId comes from the local (anchor) fabric.
    request.caseAdminSubject = NodeIdFromCASEAuthTag(mAdminCAT);

    VendorId adminVendorId = VendorId::NotSpecified;
    for (const auto & fb : mServer->GetFabricTable())
    {
        if (fb.GetFabricIndex() == jfFabricIndex)
        {
            adminVendorId = fb.GetVendorId();
            break;
        }
    }
    request.adminVendorId = adminVendorId;

    ChipLogProgress(JointFabric, "SendAddNOC: invoke cluster command for nodeId=0x" ChipLogFormatX64 " (NOC %u, ICAC %u, IPK %u bytes)",
                    ChipLogValueX64(mMigratingNodeId), static_cast<unsigned>(mDeviceNOCLen),
                    static_cast<unsigned>(mCrossSignedICACLen), static_cast<unsigned>(mIPKLen));

    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), kRootEndpointId);
    return cluster.InvokeCommand(request, this, OnAddNOCResponse, OnAddNOCFailure);
}

void JFAManagerBase::OnAddNOCResponse(void * context, const OperationalCredentials::Commands::NOCResponse::DecodableType & data)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogProgress(JointFabric, "OnAddNOCResponse: statusCode=%u", to_underlying(data.statusCode));

    if (data.statusCode != OperationalCredentials::NodeOperationalCertStatusEnum::kOk)
    {
        ChipLogError(JointFabric, "OnAddNOCResponse: device rejected NOC (statusCode=%u)", to_underlying(data.statusCode));
        jfaManager->FinishCurrentMigration(CHIP_ERROR_INTERNAL);
        return;
    }

    // The device now holds a Fabric-A NOC. Per the Matter spec, CommissioningComplete must be
    // invoked over a CASE session established on the newly added fabric (Fabric A), not over the
    // old Fabric-B session that carried AddNOC. Release the Fabric-B session and open a fresh CASE
    // session to the device on the Fabric-A (joint fabric) index before sending CommissioningComplete.
    if (jfaManager->mPeerJFAFabricIndex == kUndefinedFabricIndex)
    {
        ChipLogError(JointFabric, "OnAddNOCResponse: Fabric-A fabric index unknown, cannot finalize on new fabric");
        jfaManager->FinishCurrentMigration(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    jfaManager->ReleaseSession();

    ScopedNodeId scopedNodeId(jfaManager->mMigratingNodeId, jfaManager->mPeerJFAFabricIndex);
    ChipLogProgress(JointFabric,
                    "OnAddNOCResponse: reconnecting to device 0x" ChipLogFormatX64
                    " on Fabric A (fabric index %u) to send CommissioningComplete",
                    ChipLogValueX64(jfaManager->mMigratingNodeId), jfaManager->mPeerJFAFabricIndex);
    jfaManager->ConnectToNode(scopedNodeId, kMigrationCommissioningComplete);
}

CHIP_ERROR JFAManagerBase::SendMigrationCommissioningComplete()
{
    if (!mExchangeMgr)
    {
        ChipLogError(JointFabric, "SendMigrationCommissioningComplete: no active exchange");
        return CHIP_ERROR_UNINITIALIZED;
    }

    ChipLogProgress(JointFabric,
                    "SendMigrationCommissioningComplete: invoke CommissioningComplete for nodeId=0x" ChipLogFormatX64
                    " on Fabric A",
                    ChipLogValueX64(mMigratingNodeId));

    GeneralCommissioning::Commands::CommissioningComplete::Type request;
    Controller::ClusterBase cluster(*mExchangeMgr, mSessionHolder.Get().Value(), kRootEndpointId);
    return cluster.InvokeCommand(request, this, OnMigrationCommissioningCompleteResponse,
                                 OnMigrationCommissioningCompleteFailure);
}


void JFAManagerBase::OnAddNOCFailure(void * context, CHIP_ERROR error)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogError(JointFabric, "OnAddNOCFailure: %s", chip::ErrorStr(error));
    jfaManager->FinishCurrentMigration(error);
}

void JFAManagerBase::OnMigrationCommissioningCompleteResponse(
    void * context, const GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogProgress(JointFabric, "OnMigrationCommissioningCompleteResponse: errorCode=%u", to_underlying(data.errorCode));

    if (data.errorCode == GeneralCommissioning::CommissioningErrorEnum::kOk)
    {
        // Commit the node in the Fabric-A (JFA-A) datastore.
        CHIP_ERROR refreshErr = Server::GetInstance().GetJointFabricDatastore().RefreshNode(jfaManager->mMigratingNodeId);
        if (refreshErr != CHIP_NO_ERROR)
        {
            ChipLogError(JointFabric, "OnMigrationCommissioningCompleteResponse: RefreshNode failed: %" CHIP_ERROR_FORMAT,
                         refreshErr.Format());
        }
        else
        {
            ChipLogProgress(JointFabric, "OnMigrationCommissioningCompleteResponse: node 0x" ChipLogFormatX64
                            " committed in Fabric-A NodeList", ChipLogValueX64(jfaManager->mMigratingNodeId));
        }
        jfaManager->FinishCurrentMigration(CHIP_NO_ERROR);
    }
    else
    {
        ChipLogError(JointFabric, "OnMigrationCommissioningCompleteResponse: CommissioningComplete rejected (errorCode=%u)",
                     to_underlying(data.errorCode));
        jfaManager->FinishCurrentMigration(CHIP_ERROR_INTERNAL);
    }
}

void JFAManagerBase::OnMigrationCommissioningCompleteFailure(void * context, CHIP_ERROR error)
{
    JFAManagerBase * jfaManager = static_cast<JFAManagerBase *>(context);
    VerifyOrDie(jfaManager != nullptr);

    ChipLogError(JointFabric, "OnMigrationCommissioningCompleteFailure: %s", chip::ErrorStr(error));
    jfaManager->FinishCurrentMigration(error);
}

void JFAManagerBase::FinishCurrentMigration(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(JointFabric, "FinishCurrentMigration: node 0x" ChipLogFormatX64 " migrated successfully",
                        ChipLogValueX64(mMigratingNodeId));
    }
    else
    {
        ChipLogError(JointFabric, "FinishCurrentMigration: migration of node 0x" ChipLogFormatX64 " failed: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(mMigratingNodeId), err.Format());
    }

    // Drop the device from the local (Fabric-B) datastore NodeList now that it has been migrated.
    if (err == CHIP_NO_ERROR && mMigratingNodeId != kUndefinedNodeId)
    {
        CHIP_ERROR removeErr = Server::GetInstance().GetJointFabricDatastore().RemoveNode(mMigratingNodeId);
        if (removeErr != CHIP_NO_ERROR)
        {
            ChipLogError(JointFabric, "FinishCurrentMigration: RemoveNode failed: %" CHIP_ERROR_FORMAT, removeErr.Format());
        }
    }

    // Clear per-device state and tear down the session.
    mDeviceNOCLen = 0;
    memset(mDeviceNOCBuf, 0, sizeof(mDeviceNOCBuf));
    ReleaseSession();

    // Advance to the next device.
    mMigrationIndex++;
    MigrateNextNode();
}
