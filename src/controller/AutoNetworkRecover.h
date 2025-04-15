/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

using namespace chip::app::Clusters;

namespace chip {
namespace Controller {

typedef void (*OnNetworkRecover)(void * context, NodeId remoteNodeId, CHIP_ERROR status);

/**
 * A helper class to remove fabric given some parameters.
 */
class NetworkRecoverBase
{
public:
    NetworkRecoverBase(DeviceController * controller) :
        mController(controller),
        mOnDeviceConnectedCallback(&OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(&OnDeviceConnectionFailureFn, this)
    {}
    ~NetworkRecoverBase()
    {
        ChipLogDetail(Controller, "~NetworkRecoverBase");
    }

    enum class NetworkType : uint8_t
    {
        kWiFi,
        kThread,
    };

    enum class Step : uint8_t
    {
        // Ready to start recovering the network.
        kSendArmFailSafe = 0,
        // Need to get Current Fabric Index.
        kSendRemoveNetwork,
        // Need to send Remove Fabric Command.
        kSendAddOrUpdateNetwork,
        //
        kSendConnectNetwork,
        //
        kClearCASESessions,
        //
        kSendCommissioningComplete,
    };

    /*
     * @brief
     *   Try to look up the device attached to our controller with the given
     *   remote node id and ask it to remove Fabric.
     *   If function returns an error, callback will never be be executed. Otherwise, callback will always be executed.
     *
     * @param[in] remoteNodeId The remote device Id
     * @param[in] callback The callback to call once the remote fabric is completed or not.
     */
    CHIP_ERROR RecoverNetwork(NodeId remoteNodeId,
        RendezvousParameters params,  
        const WiFiCredentials & wiFiCredentials,
        uint64_t breadcrumb,
        OnNetworkRecover callback);
    CHIP_ERROR RecoverNetwork(NodeId remoteNodeId, chip::ByteSpan threadOperationalDataset, uint64_t breadcrumb, Callback::Callback<OnNetworkRecover> * callback);

private:
    DeviceController * mController;

    chip::Callback::Callback<OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
    OnNetworkRecover mOnSuccessNetworkRecoverCallback;

    NodeId mRemoteNodeId;
    NetworkType mNetworkType;
    Optional<WiFiCredentials> mWiFiCredentials;
    chip::ByteSpan mThreadOperationalDataset;
    uint64_t mBreadcrumb;

    Step mNextStep = Step::kSendArmFailSafe;

    CHIP_ERROR SendArmFailSafe(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    CHIP_ERROR SendRemoveNetwork(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    CHIP_ERROR SendAddOrUpdateNetwork(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    CHIP_ERROR SendConnectNetwork(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    CHIP_ERROR ClearCASESessions(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    CHIP_ERROR SendCommissioningComplete(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);

    static void OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);

    static void OnArmFailSafeResponse(void * context, const GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data);
    static void OnNetworkConfigResponse(void * context, const NetworkCommissioning::Commands::NetworkConfigResponse::DecodableType & data);
    static void OnConnectNetworkResponse(void * context, const NetworkCommissioning::Commands::ConnectNetworkResponse::DecodableType & data);
    static void OnCommissioningCompleteResponse(void * context, const GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data);

    static void OnCommandFailure(void * context, CHIP_ERROR error);

    static void FinishRecoverNetwork(void * context, CHIP_ERROR err);
};

/**
 * A helper class that can be used by consumers that don't care about the callback from the
 * remove fabric process and just want automatic cleanup of the NetworkRecover when done
 * with it.
 */
class AutoNetworkRecover : private NetworkRecoverBase
{
public:
    // Takes the same arguments as NetworkRecover::RecoverNetwork except without the callback.
    static CHIP_ERROR RecoverNetwork(DeviceController * controller, NodeId remoteNodeId, RendezvousParameters params, const WiFiCredentials & wiFiCredentials, uint64_t breadcrumb = 0, OnNetworkRecover callback = nullptr);
    // static CHIP_ERROR RecoverNetwork(DeviceController * controller, NodeId remoteNodeId, chip::ByteSpan threadOperationalDataset, uint64_t breadcrumb, Callback::Callback<OnNetworkRecover> * callback);

private:
    AutoNetworkRecover(DeviceController * controller);
    static void OnNetworkRecoverHandler(void * context, NodeId remoteNodeId, CHIP_ERROR status);
    chip::Callback::Callback<OnNetworkRecover> mOnNetworkRecoverCallback;
};

} // namespace Controller
} // namespace chip
