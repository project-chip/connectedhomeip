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

/**
 * @brief Class used to get UniqueID from Bridged Device Basic Information Cluster
 *
 * When syncing a device from another fabric that does not have a UniqueID, spec
 * dictates:
 *    When a Fabric Synchronizing Administrator commissions a Synchronized Device,
 *    it SHALL persist and maintain an association with the UniqueID in the Bridged
 *    Device Basic Information Cluster exposed by another Fabric Synchronizing
 *    Administrator.
 *
 * This class assists in retrieving the UniqueId in the above situation.
 */
class UniqueIdGetter : public chip::app::ReadClient::Callback
{
public:
    using OnDoneCallback = std::function<void(std::optional<chip::CharSpan>)>;

    UniqueIdGetter();

    CHIP_ERROR GetUniqueId(OnDoneCallback onDoneCallback, chip::Controller::DeviceController & controller, chip::NodeId nodeId,
                           chip::EndpointId endpointId);

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

    OnDoneCallback mOnDoneCallback;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

    bool mCurrentlyGettingUid = false;
    bool mUniqueIdHasValue    = false;
    char mUniqueId[33];
    chip::EndpointId mEndpointId;
};
