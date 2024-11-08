/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
 *      This file defines the interface for downcalls from WiFiPAFLayer
 *      to a WiFiPAF transport.
 */

#pragma once

#include <lib/core/NodeId.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace WiFiPAF {

struct WiFiPAFSession
{
    enum PAFRole
    {
        publisher,
        subscriber
    };
    PAFRole role;
    uint32_t id;
    uint32_t peer_id;
    uint8_t peer_addr[6];
    NodeId nodeId;
    uint16_t discriminator;
};

class WiFiPAFLayerDelegate
{
public:
    virtual ~WiFiPAFLayerDelegate()                                                                   = default;
    virtual void OnWiFiPAFMessageReceived(WiFiPAFSession & RxInfo, System::PacketBufferHandle && msg) = 0;
};

} // namespace WiFiPAF
} // namespace chip
