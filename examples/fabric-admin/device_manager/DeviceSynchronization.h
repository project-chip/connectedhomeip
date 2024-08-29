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

/// Ensures that device data is synchronized to the remote fabric bridge.
///
/// Includes a state machine that:
///   - initiates a "read basic information data" command to fetch basic information
///   - upon receiving such information, ensures that synchronized device data is sent
///     to the remote end.
class DeviceSynchronizer : public chip::app::ReadClient::Callback
{
public:
    DeviceSynchronizer();

    /// Usually called after commissioning is complete, initiates a
    /// read of required data from the remote node ID and then will synchronize
    /// the device towards the fabric bridge
    void StartDeviceSynchronization(chip::Controller::DeviceController & controller, chip::NodeId nodeId, bool deviceIsIcd);

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

    static DeviceSynchronizer & Instance();

private:
    std::unique_ptr<chip::app::ReadClient> mClient;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

    bool mDeviceSyncInProcess                      = false;
    chip_rpc_SynchronizedDevice mCurrentDeviceData = chip_rpc_SynchronizedDevice_init_default;
};
