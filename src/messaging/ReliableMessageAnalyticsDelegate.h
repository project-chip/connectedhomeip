/*
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

/**
 *    @file
 *      This file defines an interface for objects interested in MRP events for analytics
 */

#pragma once

#include <optional>

#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>

namespace chip {
namespace Messaging {

class ReliableMessageAnalyticsDelegate
{
public:
    virtual ~ReliableMessageAnalyticsDelegate() = default;

    enum class SessionType
    {
        kEstablishedCase,
        // Initially, we are starting with only one session type, but we are considering the future when we expand to allow
        // other session types, such as establishing a CASE session.
    };

    enum class EventType
    {
        // Event associated with first time this specific message is sent.
        kInitialSend,
        // Event associated with re-transmitting a message that was previously sent but not acknowledged.
        kRetransmission,
        // Event associated with receiving an acknowledgement of a previously sent message.
        kAcknowledged,
        // Event associated with transmission of a message that failed to be acknowledged.
        kFailed,
    };

    struct TransmitEvent
    {
        // When the session has a peer node ID, this will be a value other than kUndefinedNodeId.
        NodeId nodeId = kUndefinedNodeId;
        // When the session has a fabric index, this will be a value other than kUndefinedFabricIndex.
        FabricIndex fabricIndex = kUndefinedFabricIndex;
        // Session type of session the message involved is being sent on.
        SessionType sessionType = SessionType::kEstablishedCase;
        // The transmit event type.
        EventType eventType = EventType::kInitialSend;
        // The outgoing message counter associated with the event. If there is no outgoing message counter
        // this value will be 0.
        uint32_t messageCounter = 0;
        // If the eventType is kRetransmission, this value will be populated with the number of the
        // retransmission attempt. A value of 1 indicates the first retransmission (i.e. the second
        // transmission of the message). This value should never be 0.
        std::optional<uint8_t> retransmissionCount;
    };

    virtual void OnTransmitEvent(const TransmitEvent & event) = 0;
};

} // namespace Messaging
} // namespace chip
