/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#pragma once

#include <app/ReadClient.h>
#include <controller/CHIPDeviceController.h>
#include <lib/core/DataModelTypes.h>

#include <memory>

#if defined(PW_RPC_ENABLED)
#include "fabric_bridge_service/fabric_bridge_service.pb.h"
#include "fabric_bridge_service/fabric_bridge_service.rpc.pb.h"
#endif

class DeviceSubscriptionManager;

/// Attribute subscription to attributes that are important to keep track and send to fabric-bridge
/// via RPC when change has been identified.
///
/// An instance of DeviceSubscription is intended to be used only once. Once a DeviceSubscription is
/// terminated, either from an error or from subscriptions getting shut down, we expect the instance
/// to be deleted. Any new subscription should instantiate another instance of DeviceSubscription.
class DeviceSubscription : public chip::app::ReadClient::Callback
{
public:
    using OnDoneCallback = std::function<void(chip::NodeId)>;

    DeviceSubscription();

    CHIP_ERROR StartSubscription(OnDoneCallback onDoneCallback, chip::Controller::DeviceController & controller,
                                 chip::NodeId nodeId);

    /// This will trigger stopping the subscription. Once subscription is stopped the OnDoneCallback
    /// provided in StartSubscription will be called to indicate that subscription have been terminated.
    ///
    /// Must only be called after StartSubscription was successfully called.
    void StopSubscription();

    ///////////////////////////////////////////////////////////////
    // ReadClient::Callback implementation
    ///////////////////////////////////////////////////////////////
    void OnAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data,
                         const chip::app::StatusIB & status) override;
    void OnReportEnd() override;
    void OnError(CHIP_ERROR error) override;
    void OnDone(chip::app::ReadClient * apReadClient) override;

    ///////////////////////////////////////////////////////////////
    // callbacks for CASE session establishment
    ///////////////////////////////////////////////////////////////
    void OnDeviceConnected(chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle);
    void OnDeviceConnectionFailure(const chip::ScopedNodeId & peerId, CHIP_ERROR error);

private:
    enum class State : uint8_t
    {
        Idle,                ///< Default state that the object starts out in, where no work has commenced
        Connecting,          ///< We are waiting for OnDeviceConnected or OnDeviceConnectionFailure callbacks to be called
        Stopping,            ///< We are waiting for OnDeviceConnected or OnDeviceConnectionFailure callbacks so we can terminate
        SubscriptionStarted, ///< We have started a subscription.
        AwaitingDestruction, ///< The object has completed its work and is awaiting destruction.
    };

    void MoveToState(const State aTargetState);
    const char * GetStateStr() const;

    chip::NodeId mNodeId = chip::kUndefinedNodeId;

    OnDoneCallback mOnDoneCallback;
    std::unique_ptr<chip::app::ReadClient> mClient;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

#if defined(PW_RPC_ENABLED)
    chip_rpc_AdministratorCommissioningChanged mCurrentAdministratorCommissioningAttributes;
#endif

    bool mChangeDetected = false;
    State mState         = State::Idle;
};
