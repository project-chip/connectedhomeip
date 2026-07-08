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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Controller;
using namespace chip::Crypto;

JFAManager JFAManager::sJFA;

CHIP_ERROR JFAManager::Init(Server & server)
{
    mServer             = &server;
    mCASESessionManager = server.GetCASESessionManager();

    return CHIP_NO_ERROR;
}

CHIP_ERROR JFAManager::FinalizeCommissioning(NodeId nodeId, bool isJCM, P256PublicKey & trustedIcacPublicKeyB)
{
    if (jfFabricIndex == kUndefinedFabricId)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogProgress(JointFabric, "FinalizeCommissioning for NodeID: 0x" ChipLogFormatX64 ", isJCM = %d", ChipLogValueX64(nodeId),
                    isJCM);

    ScopedNodeId scopedNodeId = ScopedNodeId(nodeId, jfFabricIndex);

    ConnectToNode(scopedNodeId, kStandardCommissioningComplete);

    return CHIP_NO_ERROR;
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
                (void) app::Clusters::JointFabricAdministrator::Attributes::AdministratorFabricIndex::Set(1, fabricIndex);

                jfFabricIndex = fabricIndex;
<<<<<<< HEAD
=======

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
>>>>>>> 512611bc67 (Implement AI-requested changes for Joint Fabric (#72456))
            }
        }
    }
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
