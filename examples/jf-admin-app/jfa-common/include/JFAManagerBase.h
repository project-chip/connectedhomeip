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

// OS-agnostic Joint Fabric Administrator manager base class.
//
// JFAManagerBase holds the shared logic, and each platform derives
// a concrete subclass (for example JFAManagerLinux) that provides
// its own singleton and overrides the virtual hooks when needed.
//
// Platform-specific commissioner-mode bring-up is reached through
// JFAPlatform.h hooks, not through CommissionerMain.h directly, so this file
// has no Linux dependency.

#pragma once

#include <app/server/JointFabricAdministrator.h>
#include <app/server/Server.h>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>

#include <functional>
#include <stdbool.h>
#include <stdint.h>

#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <app/data-model/DecodableList.h>

#include "JFARpc.h"

namespace chip {

// Raw-TLV decodable mirror of JointFabricDatastore::DatastoreGroupInformationEntryStruct used only
// to read the peer JFA-A GroupList attribute. The ZAP-generated struct declares groupCAT and
// groupCATVersion as Nullable<uint16_t>, whose maximum encodable value is 0xFFFE because 0xFFFF is
// the reserved null sentinel. The Administrator CAT identifier is exactly 0xFFFF, so decoding a
// group whose GroupCAT is the Administrator CAT with the generated type fails with CONSTRAINT_ERROR.
// This struct reads those two fields as plain uint16_t (and ignores all other fields) so the full
// 0xFFFF Administrator CAT identifier can be recovered.
struct JFAGroupInfoRaw
{
    uint16_t groupCAT             = 0;
    bool groupCATIsNull           = true;
    uint16_t groupCATVersion      = 0;
    bool groupCATVersionIsNull    = true;

    static constexpr bool kIsFabricScoped = false;

    CHIP_ERROR Decode(TLV::TLVReader & reader)
    {
        // Field tags per DatastoreGroupInformationEntryStruct: 3 = GroupCAT, 4 = GroupCATVersion.
        constexpr uint8_t kGroupCATTag        = 3;
        constexpr uint8_t kGroupCATVersionTag = 4;

        TLV::TLVType outer;
        ReturnErrorOnFailure(reader.EnterContainer(outer));

        CHIP_ERROR err;
        while ((err = reader.Next()) == CHIP_NO_ERROR)
        {
            if (!TLV::IsContextTag(reader.GetTag()))
            {
                continue;
            }

            const uint8_t tagNum = static_cast<uint8_t>(TLV::TagNumFromTag(reader.GetTag()));
            if (tagNum == kGroupCATTag)
            {
                if (reader.GetType() == TLV::kTLVType_Null)
                {
                    groupCATIsNull = true;
                }
                else
                {
                    ReturnErrorOnFailure(reader.Get(groupCAT));
                    groupCATIsNull = false;
                }
            }
            else if (tagNum == kGroupCATVersionTag)
            {
                if (reader.GetType() == TLV::kTLVType_Null)
                {
                    groupCATVersionIsNull = true;
                }
                else
                {
                    ReturnErrorOnFailure(reader.Get(groupCATVersion));
                    groupCATVersionIsNull = false;
                }
            }
        }

        VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
        return reader.ExitContainer(outer);
    }
};


class JFAManagerBase : public app::JointFabricAdministrator::Delegate
{
public:
    JFAManagerBase() : mOnConnectedCallback(OnConnected, this), mOnConnectionFailureCallback(OnConnectionFailure, this) {}
    ~JFAManagerBase() override = default;

    virtual CHIP_ERROR Init(Server & server);
    virtual void HandleCommissioningCompleteEvent();

    // Called when the system transitions to "network ready" (DNS-SD up).
    // Default: no-op. Platforms that need to restore state after a reboot
    // override this hook. The reference Linux example never arms it.
    virtual void OnNetworkReady() {}

    CHIP_ERROR FinalizeCommissioning(NodeId nodeId, bool isJCM, chip::Crypto::P256PublicKey & trustedIcacPublicKeyB,
                                     uint16_t peerAdminJFAdminClusterEndpointId);

    void SetJFARpc(JFARpc & aJFARpc);
    JFARpc * GetJFARpc();

    /* app::JointFabricAdministrator::Delegate */
    CHIP_ERROR GetIcacCsr(MutableByteSpan & icacCsr) override;
    CHIP_ERROR StoreCrossSignedICAC(ByteSpan icac) override;
    CHIP_ERROR PushJFMaterials(ByteSpan icac, ByteSpan rcac, ByteSpan ipk) override;
    // Discard locally cached cross-signed ICAC and tell JFC-B to clear its copy.
    CHIP_ERROR ClearJFMaterials() override;
    // Record / clear the NodeId of the peer JFA (JFA-A) that initiated JCM on this device.
    void SetPeerJFANodeId(NodeId nodeId, FabricIndex fabricIndex) override;
    void ClearPeerJFANodeId() override;


    // Returns the NodeId of the peer JFA-A that is currently running JCM against
    // this device, or kUndefinedNodeId when no JCM is in progress.
    NodeId GetPeerJFANodeId() const { return mPeerJFANodeId; }

    CHIP_ERROR GetJointFabricMode(uint8_t & jointFabricMode);

    bool IsDeviceCommissioned() { return jfFabricIndex != kUndefinedFabricId; }
    bool IsDeviceJFAdmin();
    bool IsDeviceJFAnchor();

protected:
    // Actions dispatched by OnConnected once CASE is established. The base
    // handles the two values it knows about. Subclasses that need extra
    // actions assign them numeric values starting at kFirstPlatformAction and
    // override DispatchExtendedConnectedAction() to handle them.
    enum OnConnectedAction : uint8_t
    {
        kStandardCommissioningComplete = 0,
        kJCMCommissioning              = 1,
        kDeviceMigration               = 2,
        // Connected to the peer JFA-A to register the device as pending in JFA-A's datastore
        // (JointFabricDatastore::AddPendingNode) before migrating it.
        kDatastoreAddPendingNode       = 3,
        // Reconnected to the migrated device over a CASE session on the newly installed Fabric A
        // (after AddNOC) to send GeneralCommissioning::CommissioningComplete on that fabric.
        kMigrationCommissioningComplete = 4,
        // Connected to the peer JFA-A once at the start of migration to read the fabric-wide
        // Administrator CAT from JFA-A's JointFabricDatastore GroupList, before any device is
        // added as pending or migrated.
        kReadAdminCAT                   = 5,
        // Post-migration standard commissioning: after a new device has been commissioned into
        // Fabric A (the joint fabric) by this JFA-B and CommissioningComplete succeeded, connect to
        // the peer JFA-A (the anchor) and register the new node in JFA-A's authoritative
        // JointFabricDatastore NodeList (AddPendingNode followed by RefreshNode).
        kRegisterNewNodeInJFADatastore  = 6,

        // First numeric value available for subclass-defined actions. Values
        // below this are reserved by the base.
        kFirstPlatformAction = 32,
    };

    // Called immediately after a commissionee has been fully onboarded onto
    // the joint fabric (CommissioningComplete succeeded). Default: no-op.
    virtual void OnCommissioneeOnboarded(ScopedNodeId commissionee) { (void) commissionee; }

    // Escape hatch for subclass-defined OnConnectedAction values. Default
    // does nothing; subclasses override to handle their own extended actions.
    virtual void DispatchExtendedConnectedAction(OnConnectedAction action) { (void) action; }

    // (No default Administrator CAT: the full, versioned Administrator CAT must be resolved from the
    // peer JFA-A JointFabricDatastore GroupList before AddNOC. If it cannot be resolved to a valid
    // CAT the migration is aborted rather than falling back to a guessed value, because the AddNOC
    // caseAdminSubject must exactly match the Administrator CAT embedded in the device NOC or the
    // device will deny CommissioningComplete with UNSUPPORTED_ACCESS.)

    static void OnConnected(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    static void OnConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);

    Server * mServer                          = nullptr;
    CASESessionManager * mCASESessionManager  = nullptr;
    Messaging::ExchangeManager * mExchangeMgr = nullptr;
    JFARpc * mJFARpc                          = nullptr;
    SessionHolder mSessionHolder;
    Callback::Callback<OnDeviceConnected> mOnConnectedCallback;
    Callback::Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback;
    NodeId mNodeId                               = kUndefinedNodeId;
    OnConnectedAction mOnConnectedAction         = kStandardCommissioningComplete;
    FabricId jfFabricIndex                       = kUndefinedFabricId;
    EndpointId peerAdminJFAdminClusterEndpointId = kInvalidEndpointId;
    Crypto::P256PublicKey peerAdminICACPubKey;
    uint8_t mICACBuffer[Credentials::kMaxDERCertLength];
    size_t mICACBufferLen         = 0;
    bool mCommissionerInitialized = false;

    // Fabric-A IPK (epoch key 0) fetched after commissioning; pushed to JFC-B.
    uint8_t mIPKBuf[chip::Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
    size_t mIPKLen = 0;

    // Pending commissionee CSR received during ICACCSRResponse; used to request cross-signing.
    uint8_t mPendingICACSRBuf[Crypto::kMIN_CSR_Buffer_Size];
    size_t mPendingICACSRLen = 0;

    // Cross-signed ICAC received from the JFC via RPC; sent to the commissionee via AddICAC.
    uint8_t mCrossSignedICACBuf[Credentials::kMaxDERCertLength];
    size_t mCrossSignedICACLen = 0;

    // NodeId of the peer JFA (JFA-A) that initiated JCM against this device.
    // Set when AnnounceJointFabricAdministrator succeeds; cleared on fail-safe expiry.
    NodeId mPeerJFANodeId = kUndefinedNodeId;

    // Fabric index on which the peer JFA-A invoked AnnounceJointFabricAdministrator
    // (the joint fabric). The session to JFA-A must be established on this fabric
    // index, not on this device's own locally-commissioned fabric index.
    FabricIndex mPeerJFAFabricIndex = kUndefinedFabricIndex;


    // --- Device migration (Fabric B -> Fabric A via AddNOC) state ---
    // After JCM succeeds, this JFA (acting as JFA-B commissioner) iterates all the Fabric-B
    // devices from the local datastore NodeList and onboards each of them into Fabric A.
    std::vector<NodeId> mMigrationNodeList;
    size_t mMigrationIndex           = 0;
    FabricId mAnchorFabricId         = kUndefinedFabricId;
    NodeId mMigratingNodeId          = kUndefinedNodeId;

    // Fabric-A NOC obtained for the current device from JFC-B (DER); sent via AddNOC.
    uint8_t mDeviceNOCBuf[Credentials::kMaxDERCertLength];
    size_t mDeviceNOCLen = 0;

    // Fabric-A root CA (RCAC) returned by JFC-B alongside the device NOC; sent via
    // AddTrustedRootCertificate. CHIP TLV format.
    uint8_t mDeviceRCACBuf[Credentials::kMaxDERCertLength];
    size_t mDeviceRCACLen = 0;

    // Administrator CAT (caseAdminSubject) resolved from the peer JFA-A JointFabricDatastore
    // GroupList before the device migration. Starts as kUndefinedCAT and is only set to a valid,
    // versioned Administrator CAT once the GroupList read succeeds. If it cannot be resolved the
    // migration is aborted, so AddNOC is never sent with an undefined/invalid caseAdminSubject.
    CASEAuthTag mAdminCAT = kUndefinedCAT;

    // --- Post-migration new-node registration state ---
    // NodeId of the device that was just commissioned into Fabric A (the joint fabric) by this
    // JFA-B, which must be registered in the anchor JFA-A's authoritative JointFabricDatastore
    // NodeList. Set in FinalizeCommissioning and used by the register-in-JFA-A flow.
    NodeId mNewNodeId = kUndefinedNodeId;




    void ConnectToNode(ScopedNodeId scopedNodeId, OnConnectedAction onConnectedAction);
    CHIP_ERROR SendCommissioningComplete();
    CHIP_ERROR AnnounceJointFabricAdministrator();
    CHIP_ERROR SendICACSRRequest();
    CHIP_ERROR CrossSignICAC();
    CHIP_ERROR SendAddICAC();

    // Device migration helpers.
    void StartDeviceMigration();
    void MigrateNextNode();
    // Register the current device as pending in the peer JFA-A datastore via a
    // JointFabricDatastore::AddPendingNode cluster command, then connect to the device.
    CHIP_ERROR SendDatastoreAddPendingNode();
    // Read the peer JFA-A JointFabricDatastore GroupList and extract the Administrator CAT
    // (the group whose GroupCAT equals the Administrator CAT identifier, combined with its
    // GroupCATVersion) into mAdminCAT. If no valid, versioned Administrator CAT group is found
    // the migration is aborted (no fallback value is used).
    CHIP_ERROR ReadPeerGroupList();

    CHIP_ERROR SendArmFailSafe();

    CHIP_ERROR SendCSRRequest();
    CHIP_ERROR SendAddTrustedRootCertificate();
    CHIP_ERROR SendAddNOC();
    // Invoke GeneralCommissioning::CommissioningComplete over the active CASE session, which (after
    // the kMigrationCommissioningComplete reconnect) is established on the newly installed Fabric A.
    CHIP_ERROR SendMigrationCommissioningComplete();
    void FinishCurrentMigration(CHIP_ERROR err);


    static void OnDatastoreAddPendingNodeResponse(void * context, const chip::app::DataModel::NullObjectType &);
    static void OnDatastoreAddPendingNodeFailure(void * context, CHIP_ERROR error);

    // --- Post-migration new-node registration in the anchor JFA-A datastore ---
    // Invoke JointFabricDatastore::AddPendingNode on the peer JFA-A to register the newly
    // commissioned Fabric-A node (mNewNodeId) in JFA-A's authoritative NodeList.
    CHIP_ERROR SendRegisterNewNodeAddPendingNode();
    static void OnRegisterNewNodeAddPendingResponse(void * context, const chip::app::DataModel::NullObjectType &);
    static void OnRegisterNewNodeAddPendingFailure(void * context, CHIP_ERROR error);
    // Invoke JointFabricDatastore::RefreshNode on the peer JFA-A to commit the pending node.
    CHIP_ERROR SendRegisterNewNodeRefreshNode();
    static void OnRegisterNewNodeRefreshResponse(void * context, const chip::app::DataModel::NullObjectType &);
    static void OnRegisterNewNodeRefreshFailure(void * context, CHIP_ERROR error);

    static void OnReadPeerGroupListSuccess(void * context, const app::DataModel::DecodableList<JFAGroupInfoRaw> & groupList);
    static void OnReadPeerGroupListFailure(void * context, CHIP_ERROR error);
    // Continue the migration once the Admin CAT lookup finished (success or fallback): release the
    // JFA-A session and connect to the migrating device to run the AddNOC flow.
    void ContinueMigrationAfterGroupList();
    static void OnArmFailSafeResponse(
        void * context,
        const app::Clusters::GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data);

    static void OnArmFailSafeFailure(void * context, CHIP_ERROR error);
    static void OnCSRResponse(void * context,
                              const app::Clusters::OperationalCredentials::Commands::CSRResponse::DecodableType & data);
    static void OnCSRFailure(void * context, CHIP_ERROR error);
    static void OnAddTrustedRootResponse(void * context, const chip::app::DataModel::NullObjectType &);
    static void OnAddTrustedRootFailure(void * context, CHIP_ERROR error);
    static void OnAddNOCResponse(void * context,
                                 const app::Clusters::OperationalCredentials::Commands::NOCResponse::DecodableType & data);
    static void OnAddNOCFailure(void * context, CHIP_ERROR error);
    static void OnMigrationCommissioningCompleteResponse(
        void * context,
        const app::Clusters::GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data);
    static void OnMigrationCommissioningCompleteFailure(void * context, CHIP_ERROR error);


    static void OnCommissioningCompleteResponse(
        void * context, const app::Clusters::GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data);
    static void OnCommissioningCompleteFailure(void * context, CHIP_ERROR error);
    static void OnAnnounceJointFabricAdministratorResponse(void * context, const chip::app::DataModel::NullObjectType &);
    static void OnAnnounceJointFabricAdministratorFailure(void * context, CHIP_ERROR error);
    static void
    OnSendICACSRRequestResponse(void * context,
                                const app::Clusters::JointFabricAdministrator::Commands::ICACCSRResponse::DecodableType & icaccsr);
    static void OnSendICACSRRequestFailure(void * context, CHIP_ERROR error);
    static void
    OnSendAddICACResponse(void * context,
                          const app::Clusters::JointFabricAdministrator::Commands::ICACResponse::DecodableType & response);
    static void OnSendAddICACFailure(void * context, CHIP_ERROR error);

    void ReleaseSession();
};

// Returns the active JFAManager singleton. Defined per platform (see
// JFAManagerLinux.cpp for the Linux example). Shared code and the pigweed
// RPC server both go through this accessor, so neither has to know the
// concrete subclass type.
JFAManagerBase & JFAMgr();

} // namespace chip
