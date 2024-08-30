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

#include "fabric_bridge_service/fabric_bridge_service.pb.h"
#include "fabric_bridge_service/fabric_bridge_service.rpc.pb.h"

/// Attribute subscription to attributes that are important to keep track and send to fabric-bridge
/// via RPC when change has been identified.
///
/// An instance of DeviceSubscription is intended to be used only once. Once a DeviceSubscription is
/// terminal, either from an error or from subscriptions getting shut down, we expect the instance
/// to be deleted. Any new subscription should instantiate another instance of DeviceSubscription.
class DeviceSubscription : public chip::app::ReadClient::Callback
{
public:
    DeviceSubscription();

    /// Usually called after we have added a synchronized device to fabric-bridge to monitor
    /// for any changes that need to be propgated to fabric-bridge.
    void StartSubscription(chip::Controller::DeviceController & controller, chip::NodeId nodeId);

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
    std::unique_ptr<chip::app::ReadClient> mClient;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

    chip_rpc_AdministratorCommissioningChanged mCurrentAdministratorCommissioningAttributes;
    bool mChangeDetected = false;
    // Ensures that DeviceSubscription starts a subscription only once.  If instance of
    // DeviceSubscription  can be reused, the class documentation should be updated accordingly.
    bool mSubscriptionStarted = false;
};
