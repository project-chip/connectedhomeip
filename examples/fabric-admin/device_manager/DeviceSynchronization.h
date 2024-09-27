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

#include "UniqueIdGetter.h"

#include <app/ReadClient.h>
#include <controller/CHIPDeviceController.h>
#include <lib/core/DataModelTypes.h>

#include <memory>

#if defined(PW_RPC_ENABLED)
#include "fabric_bridge_service/fabric_bridge_service.pb.h"
#include "fabric_bridge_service/fabric_bridge_service.rpc.pb.h"
#endif

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
    ///
    /// @param controller Must be a non-null pointer. The DeviceController instance
    ///        pointed to must out live the entire device synchronization process.
    /// @param nodeId Node ID of the device we need to syncronize data from.
    /// @param deviceIsIcd If the device is an ICD device.
    void StartDeviceSynchronization(chip::Controller::DeviceController * controller, chip::NodeId nodeId, bool deviceIsIcd);

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
    enum class State : uint8_t
    {
        Idle,             ///< Default state that the object starts out in, where no work has commenced
        Connecting,       ///< We are waiting for OnDeviceConnected or OnDeviceConnectionFailure callbacks to be called
        AwaitingResponse, ///< We have started reading BasicInformation cluster attributes
        ReceivedResponse, ///< We have received a ReportEnd from reading BasicInformation cluster attributes
        ReceivedError,    ///< We recieved an error while reading of BasicInformation cluster attributes
        GettingUid,       ///< We are getting UniqueId from the remote fabric sync bridge.
    };

    void GetUniqueId();
    void SynchronizationCompleteAddDevice();

    void MoveToState(const State targetState);
    const char * GetStateStr() const;

    std::unique_ptr<chip::app::ReadClient> mClient;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

    State mState = State::Idle;
    // mController is expected to remain valid throughout the entire device synchronization process (i.e. when
    // mState != Idle).
    chip::Controller::DeviceController * mController = nullptr;
    chip::NodeId mNodeId                             = chip::kUndefinedNodeId;
#if defined(PW_RPC_ENABLED)
    chip_rpc_SynchronizedDevice mCurrentDeviceData = chip_rpc_SynchronizedDevice_init_default;
#endif
    UniqueIdGetter mUniqueIdGetter;
};
