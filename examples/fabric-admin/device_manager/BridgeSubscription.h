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
 * @brief Class used to subscribe to attributes and events from the remote bridged device.
 *
 * The Descriptor Cluster contains attributes such as the Parts List, which provides a list
 * of endpoints or devices that are part of a composite device or bridge. The CommissionerControl
 * Cluster generates events related to commissioning requests, which can be monitored to track
 * device commissioning status.
 *
 * When subscribing to attributes and events of a bridged device from another fabric, the class:
 *    - Establishes a secure session with the device (if needed) via CASE (Chip over
 *      Authenticated Session Establishment) session.
 *    - Subscribes to the specified attributes in the Descriptor Cluster (e.g., Parts List) and
 *      events in the CommissionerControl Cluster (e.g., CommissioningRequestResult) of the remote
 *      device on the specified node and endpoint.
 *    - Invokes the provided callback upon successful or unsuccessful subscription, allowing
 *      further handling of data or errors.
 *
 * This class also implements the necessary callbacks to handle attribute data reports, event data,
 * errors, and session establishment procedures.
 */
class BridgeSubscription : public chip::app::ReadClient::Callback
{
public:
    BridgeSubscription();

    CHIP_ERROR StartSubscription(chip::Controller::DeviceController & controller, chip::NodeId nodeId, chip::EndpointId endpointId);

    ///////////////////////////////////////////////////////////////
    // ReadClient::Callback implementation
    ///////////////////////////////////////////////////////////////
    void OnAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data,
                         const chip::app::StatusIB & status) override;
    void OnEventData(const chip::app::EventHeader & eventHeader, chip::TLV::TLVReader * data,
                     const chip::app::StatusIB * status) override;
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
    chip::EndpointId mEndpointId;
    bool subscriptionStarted = false;
};

} // namespace admin
