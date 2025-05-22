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

#include <app/OperationalSessionSetup.h>
#include <controller/CHIPDeviceController.h>
#include <lib/core/CHIPCallback.h>

namespace chip {
namespace Controller {

typedef void (*OnNetworkRecover)(void * context, NodeId remoteNodeId, CHIP_ERROR status);

/**
 * A helper class to recover network given some parameters.
 */
class NetworkRecoverBase
{
public:
    NetworkRecoverBase(DeviceController * controller) :
        mController(controller), mOnDeviceConnectedCallback(&OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(&OnDeviceConnectionFailureFn, this)
    {}
    ~NetworkRecoverBase() {}

    enum class NetworkType : uint8_t
    {
        kWiFi,
        kThread,
    };

    enum class Step : uint8_t
    {
        // Ready to start recovering the network.
        kSendArmFailSafe = 0,
        // Need to get Last Newtork ID
        kReadLastNetworkID,
        // Need to send Remove Network Command.
        kSendRemoveNetwork,
        // Need to send Add or Update WiFi/Thread Network Command.
        kSendAddOrUpdateNetwork,
        // Need to send Connect Network Command.
        kSendConnectNetwork,
        // Need to release CASE Sessions.
        kReleaseSessions,
        // Need to send Commissioning Complete Command.
        kSendCommissioningComplete,
        // Need to send DisArmFailSafe Command.
        kSendDisArmFailSafe,
    };

    CHIP_ERROR RecoverNetwork(NodeId remoteNodeId, Transport::PeerAddress & addr, const Optional<WiFiCredentials> & wiFiCredentials,
                              const Optional<ByteSpan> & operationalDataset, uint64_t breadcrumb,
                              chip::Callback::Callback<OnNetworkRecover> * callback);

private:
    DeviceController * mController;

    chip::Callback::Callback<OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
    chip::Callback::Callback<OnNetworkRecover> * mNetworkRecoverCallback;

    NodeId mRemoteNodeId;
    NetworkType mNetworkType;
    Optional<WiFiCredentials> mWiFiCredentials;
    Optional<chip::ByteSpan> mOperationalDataset;
    uint64_t mBreadcrumb;
    chip::ByteSpan mLastNetworkID;

    Step mNextStep = Step::kSendArmFailSafe;

    CHIP_ERROR SendArmFailSafe(uint16_t timeout, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    CHIP_ERROR ReadLastNetworkID(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    CHIP_ERROR SendRemoveNetwork(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    CHIP_ERROR SendAddOrUpdateNetwork(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    CHIP_ERROR SendConnectNetwork(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    CHIP_ERROR ReleaseSessions(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    CHIP_ERROR SendCommissioningComplete(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);

    static void OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);

    static void
    OnArmFailSafeResponse(void * context,
                          const chip::app::Clusters::GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data);
    static void OnSuccessReadLastNetworkID(void * context, const chip::app::DataModel::Nullable<chip::ByteSpan> & networkID);
    static void
    OnNetworkConfigResponse(void * context,
                            const chip::app::Clusters::NetworkCommissioning::Commands::NetworkConfigResponse::DecodableType & data);
    static void OnConnectNetworkResponse(
        void * context, const chip::app::Clusters::NetworkCommissioning::Commands::ConnectNetworkResponse::DecodableType & data);
    static void OnCommissioningCompleteResponse(
        void * context,
        const chip::app::Clusters::GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data);

    static void OnReadAttributeFailure(void * context, CHIP_ERROR error);
    static void OnCommandFailure(void * context, CHIP_ERROR error);

    static void FinishRecoverNetwork(void * context, CHIP_ERROR err);
};

/**
 * A helper class that can be used by consumers that don't care about the callback from the
 * network recovery process and just want automatic cleanup of the AutoNetworkRecover when done
 * with it.
 */
class AutoNetworkRecover : private NetworkRecoverBase
{
public:
    static CHIP_ERROR RecoverNetwork(NetworkRecover * recover, NodeId remoteNodeId, Transport::PeerAddress & addr,
                                     const Optional<WiFiCredentials> & wiFiCredentials,
                                     const Optional<ByteSpan> & operationalDataset, uint64_t breadcrumb = 0);

private:
    NetworkRecover * mNetworkRecover = nullptr;
    AutoNetworkRecover(DeviceController * controller);
    static void OnNetworkRecoverComplete(void * context, NodeId remoteNodeId, CHIP_ERROR status);
    chip::Callback::Callback<OnNetworkRecover> mOnNetworkRecoverCompleteCallback;
};

} // namespace Controller
} // namespace chip
