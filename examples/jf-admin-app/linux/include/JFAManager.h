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
#include <app/server/Server.h>

#include <functional>
#include <stdbool.h>
#include <stdint.h>

#include <lib/core/CHIPError.h>

#include "JFARpc.h"

namespace chip {

class JFAManager
{
public:
    JFAManager() : mOnConnectedCallback(OnConnected, this), mOnConnectionFailureCallback(OnConnectionFailure, this) {}

    CHIP_ERROR Init(Server & server);
    void HandleCommissioningCompleteEvent();
    CHIP_ERROR FinalizeCommissioning(NodeId nodeId, bool isJCM, chip::Crypto::P256PublicKey & trustedIcacPublicKeyB);

    void SetJFARpc(JFARpc & aJFARpc);

private:
    // Various actions to take when OnConnected callback is called
    enum OnConnectedAction
    {
        kStandardCommissioningComplete = 0,
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
    OnConnectedAction mOnConnectedAction = kStandardCommissioningComplete;
    FabricId jfFabricIndex               = kUndefinedFabricId;

    void ConnectToNode(ScopedNodeId scopedNodeId, OnConnectedAction onConnectedAction);
    CHIP_ERROR SendCommissioningComplete();
    static void OnCommissioningCompleteResponse(
        void * context, const app::Clusters::GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data);
    static void OnCommissioningCompleteFailure(void * context, CHIP_ERROR error);
    void ReleaseSession();
};

inline JFAManager & JFAMgr(void)
{
    return JFAManager::sJFA;
}

} // namespace chip
