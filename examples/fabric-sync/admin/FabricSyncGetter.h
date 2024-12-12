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

#include <memory>
#include <optional>

namespace admin {

/**
 * @brief Class used to get FabricSynchronization from SupportedDeviceCategories attribute of Commissioner Control Cluster.
 *
 * Functionality:
 *  - Establishes a CASE session to communicate with the remote bridge.
 *  - Retrieves the attribute data from the endpoint which host Aggregator.
 *  - Provides callbacks for success, error, and completion when retrieving data.
 */
class FabricSyncGetter : public chip::app::ReadClient::Callback
{
public:
    using OnDoneCallback = std::function<void(chip::TLV::TLVReader & data)>;

    FabricSyncGetter();

    /**
     * @brief Initiates the process of retrieving fabric synchronization data from the target device.
     *
     * @param onDoneCallback A callback function to be invoked when the data retrieval is complete.
     * @param controller The device controller used to establish a session with the target device.
     * @param nodeId The Node ID of the target device.
     * @param endpointId The Endpoint ID from which to retrieve the fabric synchronization data.
     * @return CHIP_ERROR Returns an error if the process fails, CHIP_NO_ERROR on success.
     */
    CHIP_ERROR GetFabricSynchronizationData(OnDoneCallback onDoneCallback, chip::Controller::DeviceController & controller,
                                            chip::NodeId nodeId, chip::EndpointId endpointId);

    ///////////////////////////////////////////////////////////////
    // ReadClient::Callback implementation
    ///////////////////////////////////////////////////////////////
    void OnAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data,
                         const chip::app::StatusIB & status) override;
    void OnError(CHIP_ERROR error) override;
    void OnDone(chip::app::ReadClient * apReadClient) override;

    ///////////////////////////////////////////////////////////////
    // callbacks for CASE session establishment
    ///////////////////////////////////////////////////////////////
    void OnDeviceConnected(chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle);
    void OnDeviceConnectionFailure(const chip::ScopedNodeId & peerId, CHIP_ERROR error);

private:
    std::unique_ptr<chip::app::ReadClient> mClient;

    OnDoneCallback mOnDoneCallback;
    chip::EndpointId mEndpointId;
    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
};

} // namespace admin
