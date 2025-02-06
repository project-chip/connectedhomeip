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
 *      This file defines interface for objects interested in MRP events for analytics
 */

#pragma once

#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>

namespace chip {
namespace Messaging {

class ReliableMessageAnalyticsDelegate
{
public:
    virtual ~ReliableMessageAnalyticsDelegate() = default;
    enum class EventType
    {
        kInitialSend,
        kRetransmission,
        kAcknowledged,
        kFailed,
        kUndefined, /* Should be last element in enum */
    };

    struct TransmitEvent
    {
        NodeId nodeId           = kUndefinedNodeId;
        FabricIndex fabricIndex = kUndefinedFabricIndex;
        EventType eventType     = EventType::kUndefined;
        uint32_t messageCounter = 0;
    };

    virtual void OnTransmitEvent(const TransmitEvent & event) = 0;
};

} // namespace Messaging
} // namespace chip
