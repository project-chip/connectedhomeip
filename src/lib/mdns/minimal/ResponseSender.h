/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#pragma once

#include "Parser.h"
#include "ResponseBuilder.h"
#include "Server.h"

#include <mdns/minimal/responders/QueryResponder.h>

#include <inet/InetLayer.h>
#include <system/SystemPacketBuffer.h>

namespace mdns {
namespace Minimal {

/// Sends responses to mDNS queries.
///
/// Handles processing the query via a QueryResponderBase and then sending back the reply
/// using appropriate paths (unicast or multicast) via the given Server.
class ResponseSender : public ResponderDelegate
{
public:
    ResponseSender(ServerBase * server, QueryResponderBase * responder) : mServer(server), mResponder(responder) {}

    /// Send back the response to a particular query
    CHIP_ERROR Respond(uint32_t messageId, const QueryData & query, const chip::Inet::IPPacketInfo * querySource);

    // Implementation of ResponderDelegate
    void AddResponse(const ResourceRecord & record) override;

private:
    CHIP_ERROR FlushReply();
    CHIP_ERROR PrepareNewReplyPacket();

    ServerBase * mServer;
    QueryResponderBase * mResponder;

    /// Current send state
    chip::System::PacketBufferHandle mCurrentPacket; // packet currently being built
    ResponseBuilder mResponseBuilder;
    const chip::Inet::IPPacketInfo * mCurrentSource = nullptr;
    uint32_t mCurrentMessageId                      = 0;
    bool mSendUnicast                               = false;
    ResourceType mCurrentResourceType               = ResourceType::kAnswer;
    CHIP_ERROR mSendError                           = CHIP_NO_ERROR;
};

} // namespace Minimal
} // namespace mdns
