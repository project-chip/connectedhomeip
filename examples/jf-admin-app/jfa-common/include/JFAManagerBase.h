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

#include "JFARpc.h"

namespace chip {

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

    void ConnectToNode(ScopedNodeId scopedNodeId, OnConnectedAction onConnectedAction);
    CHIP_ERROR SendCommissioningComplete();
    CHIP_ERROR AnnounceJointFabricAdministrator();
    CHIP_ERROR SendICACSRRequest();

    static void OnCommissioningCompleteResponse(
        void * context, const app::Clusters::GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data);
    static void OnCommissioningCompleteFailure(void * context, CHIP_ERROR error);
    static void OnAnnounceJointFabricAdministratorResponse(void * context, const chip::app::DataModel::NullObjectType &);
    static void OnAnnounceJointFabricAdministratorFailure(void * context, CHIP_ERROR error);
    static void
    OnSendICACSRRequestResponse(void * context,
                                const app::Clusters::JointFabricAdministrator::Commands::ICACCSRResponse::DecodableType & icaccsr);
    static void OnSendICACSRRequestFailure(void * context, CHIP_ERROR error);

    void ReleaseSession();
};

// Returns the active JFAManager singleton. Defined per platform (see
// JFAManagerLinux.cpp for the Linux example). Shared code and the pigweed
// RPC server both go through this accessor, so neither has to know the
// concrete subclass type.
JFAManagerBase & JFAMgr();

} // namespace chip
