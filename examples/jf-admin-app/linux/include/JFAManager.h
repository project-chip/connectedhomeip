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

#pragma once

#include <AppMain.h>
#include <app/server/JointFabricAdministrator.h>
#include <app/server/Server.h>

#include <functional>
#include <stdbool.h>
#include <stdint.h>

#include <lib/core/CHIPError.h>
#include <lib/support/ScopedMemoryBuffer.h>

#include "JFARpc.h"

namespace chip {

class JFAManager : public app::JointFabricAdministrator::Delegate
{
public:
    JFAManager() : mOnConnectedCallback(OnConnected, this), mOnConnectionFailureCallback(OnConnectionFailure, this) {}

    CHIP_ERROR Init(Server & server);
    void HandleCommissioningCompleteEvent();
    CHIP_ERROR FinalizeCommissioning(NodeId nodeId, bool isJCM, chip::Crypto::P256PublicKey & trustedIcacPublicKeyB,
                                     uint16_t peerAdminJFAdminClusterEndpointId);

    void SetJFARpc(JFARpc & aJFARpc);
    JFARpc * GetJFARpc();

    /* app::JointFabricAdministrator::Delegate */
    CHIP_ERROR GetIcacCsr(MutableByteSpan & icacCsr) override;
    CHIP_ERROR StoreCrossSignedICAC(const ByteSpan & crossSignedICAC) override;

    CHIP_ERROR GetJointFabricMode(uint8_t & jointFabricMode);

    bool IsDeviceCommissioned() { return jfFabricIndex != kUndefinedFabricId; }
    bool IsDeviceJFAdmin();
    bool IsDeviceJFAnchor();

private:
    // Various actions to take when OnConnected callback is called
    enum OnConnectedAction
    {
        kStandardCommissioningComplete = 0,
        kJCMCommissioning              = 1,
    };

    // JCM flow stage, used to prevent out-of-order invocations
    enum JCMStage
    {
        kJCMStageIdle               = 0, ///< No JCM flow active
        kJCMStageAnnounce           = 1, ///< Sent AnnounceJointFabricAdministrator
        kJCMStageICACCSRRequest     = 2, ///< Sent ICACCSRRequest, waiting for response
        kJCMStageCrossSign          = 3, ///< Cross-signing ICAC CSR via RPC
        kJCMStageAddICAC            = 4, ///< Sent AddICAC, waiting for response
        kJCMStageCommissionComplete = 5, ///< Sent CommissioningComplete
    };

    friend JFAManager & JFAMgr(void);

    static JFAManager sJFA;
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
    JCMStage mJCMStage                           = kJCMStageIdle;
    FabricId jfFabricIndex                       = kUndefinedFabricId;
    EndpointId peerAdminJFAdminClusterEndpointId = kInvalidEndpointId;
    Crypto::P256PublicKey peerAdminICACPubKey;
    uint8_t mICACBuffer[Credentials::kMaxDERCertLength];
    size_t mICACBufferLen         = 0;
    bool mCommissionerInitialized = false;

    // JCM state: pending CSR bytes received from commissionee, cleared after cross-signing
    uint8_t mPendingICACSRBuf[Crypto::kMIN_CSR_Buffer_Size];
    size_t mPendingICACSRLen = 0;

    // JCM state: cross-signed ICAC to be installed via AddICAC, cleared after installation
    uint8_t mCrossSignedICACBuf[Credentials::kMaxDERCertLength];
    size_t mCrossSignedICACLen = 0;

    void ConnectToNode(ScopedNodeId scopedNodeId, OnConnectedAction onConnectedAction);
    CHIP_ERROR SendCommissioningComplete();
    CHIP_ERROR AnnounceJointFabricAdministrator();
    CHIP_ERROR SendICACSRRequest();

    /**
     * Cross-sign the pending ICAC CSR under the anchor root CA via the JFC RPC,
     * storing the resulting cross-signed ICAC in mCrossSignedICACBuf.
     * The CSR to sign must have been saved in mPendingICACSRBuf beforehand.
     */
    CHIP_ERROR CrossSignICAC();

    /**
     * Invoke the AddICAC cluster command on the commissionee with the cross-signed
     * ICAC stored in mCrossSignedICACBuf.
     */
    CHIP_ERROR SendAddICAC();

    static void OnCommissioningCompleteResponse(
        void * context, const app::Clusters::GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data);
    static void OnCommissioningCompleteFailure(void * context, CHIP_ERROR error);
    static void OnAnnounceJointFabricAdministratorResponse(void * context, const chip::app::DataModel::NullObjectType &);
    static void OnAnnounceJointFabricAdministratorFailure(void * context, CHIP_ERROR error);
    static void
    OnSendICACSRRequestResponse(void * context,
                                const app::Clusters::JointFabricAdministrator::Commands::ICACCSRResponse::DecodableType & icaccsr);
    static void OnSendICACSRRequestFailure(void * context, CHIP_ERROR error);
    static void OnAddICACResponse(void * context,
                                  const app::Clusters::JointFabricAdministrator::Commands::ICACResponse::DecodableType & response);
    static void OnAddICACFailure(void * context, CHIP_ERROR error);

    void ReleaseSession();
};

inline JFAManager & JFAMgr(void)
{
    return JFAManager::sJFA;
}

} // namespace chip
