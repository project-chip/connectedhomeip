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

namespace Internal {

/// Represents the internal state for sending a currently active request
class ResponseSendingState
{
public:
    ResponseSendingState() {}

    void Reset(uint32_t messageId, const QueryData & query, const chip::Inet::IPPacketInfo * packet)
    {
        mMessageId    = messageId;
        mQuery        = &query;
        mSource       = packet;
        mSendError    = CHIP_NO_ERROR;
        mResourceType = ResourceType::kAnswer;
    }

    void SetResourceType(ResourceType resourceType) { mResourceType = resourceType; }
    ResourceType GetResourceType() const { return mResourceType; }

    CHIP_ERROR SetError(CHIP_ERROR chipError)
    {
        mSendError = chipError;
        return mSendError;
    }
    CHIP_ERROR GetError() const { return mSendError; }

    uint32_t GetMessageId() const { return mMessageId; }

    const QueryData * GetQuery() const { return mQuery; }

    /// Check if the reply should be sent as a unicast reply
    bool SendUnicast() const;

    /// Check if the original query should be included in the reply
    bool IncludeQuery() const;

    const chip::Inet::IPPacketInfo * GetSource() const { return mSource; }

    uint16_t GetSourcePort() const { return mSource->SrcPort; }
    const chip::Inet::IPAddress & GetSourceAddress() const { return mSource->SrcAddress; }
    chip::Inet::InterfaceId GetSourceInterfaceId() const { return mSource->Interface; }

private:
    const QueryData * mQuery                 = nullptr;               // query being replied to
    const chip::Inet::IPPacketInfo * mSource = nullptr;               // Where to send the reply (if unicast)
    uint32_t mMessageId                      = 0;                     // message id for the reply
    ResourceType mResourceType               = ResourceType::kAnswer; // what is being sent right now
    CHIP_ERROR mSendError                    = CHIP_NO_ERROR;
};

} // namespace Internal

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
    ResponseBuilder mResponseBuilder;          // packet being built
    Internal::ResponseSendingState mSendState; // sending state
};

} // namespace Minimal
} // namespace mdns
