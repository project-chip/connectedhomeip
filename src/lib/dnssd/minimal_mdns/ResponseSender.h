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

#include <lib/dnssd/minimal_mdns/responders/QueryResponder.h>

#include <system/SystemPacketBuffer.h>

#if CHIP_CONFIG_MINMDNS_DYNAMIC_OPERATIONAL_RESPONDER_LIST

#include <list>
using QueryResponderPtrPool = std::list<mdns::Minimal::QueryResponderBase *>;
#else

#include <array>

// Note: ptr storage is 2 + number of operational networks required, based on
// the current implementation of Advertiser_ImplMinimalMdns.cpp:
//    - 1 for commissionable advertising
//    - 1 for commissioner responder
//    - extra for every operational advertisement
using QueryResponderPtrPool = std::array<mdns::Minimal::QueryResponderBase *, CHIP_CONFIG_MAX_FABRICS + 2>;

#endif

namespace mdns {
namespace Minimal {

namespace Internal {

// Flags for keeping track of items having been sent as DNSSD responses
//
// We rely on knowing Matter DNSSD only sends the same set of data
// for some instances like A/AAAA always being the same.
//
enum class ResponseItemsSent : uint8_t
{
    // DNSSD may have different servers referenced by IP addresses,
    // however we know the matter dnssd server name is fixed and
    // the same even across SRV records.
    kIPv4Addresses = 0x01,
    kIPv6Addresses = 0x02,

    // Boot time advertisement filters. We allow multiple of these
    // however we also allow filtering them out at response start
    kServiceListingData = 0x04,
};

/// Represents the internal state for sending a currently active request
class ResponseSendingState
{
public:
    ResponseSendingState() {}

    void Reset(uint16_t messageId, const QueryData & query, const chip::Inet::IPPacketInfo * packet)
    {
        mMessageId    = messageId;
        mQuery        = &query;
        mSource       = packet;
        mSendError    = CHIP_NO_ERROR;
        mResourceType = ResourceType::kAnswer;
        mSentItems.ClearAll();
    }

    void SetResourceType(ResourceType resourceType) { mResourceType = resourceType; }
    ResourceType GetResourceType() const { return mResourceType; }

    CHIP_ERROR SetError(CHIP_ERROR chipError)
    {
        mSendError = chipError;
        return mSendError;
    }
    CHIP_ERROR GetError() const { return mSendError; }

    uint16_t GetMessageId() const { return mMessageId; }

    const QueryData * GetQuery() const { return mQuery; }

    /// Check if the reply should be sent as a unicast reply
    bool SendUnicast() const;

    /// Check if the original query should be included in the reply
    bool IncludeQuery() const;

    const chip::Inet::IPPacketInfo * GetSource() const { return mSource; }

    uint16_t GetSourcePort() const { return mSource->SrcPort; }
    const chip::Inet::IPAddress & GetSourceAddress() const { return mSource->SrcAddress; }
    chip::Inet::InterfaceId GetSourceInterfaceId() const { return mSource->Interface; }

    bool GetWasSent(ResponseItemsSent item) const { return mSentItems.Has(item); }
    void MarkWasSent(ResponseItemsSent item) { mSentItems.Set(item); }

private:
    const QueryData * mQuery                 = nullptr;               // query being replied to
    const chip::Inet::IPPacketInfo * mSource = nullptr;               // Where to send the reply (if unicast)
    uint16_t mMessageId                      = 0;                     // message id for the reply
    ResourceType mResourceType               = ResourceType::kAnswer; // what is being sent right now
    CHIP_ERROR mSendError                    = CHIP_NO_ERROR;
    chip::BitFlags<ResponseItemsSent> mSentItems;
};

} // namespace Internal

/// Sends responses to mDNS queries.
///
/// Handles processing the query via a QueryResponderBase and then sending back the reply
/// using appropriate paths (unicast or multicast) via the given Server.
class ResponseSender : public ResponderDelegate
{
public:
    ResponseSender(ServerBase * server) : mServer(server) {}

    CHIP_ERROR AddQueryResponder(QueryResponderBase * queryResponder);
    CHIP_ERROR RemoveQueryResponder(QueryResponderBase * queryResponder);
    bool HasQueryResponders() const;

    /// Send back the response to a particular query
    CHIP_ERROR Respond(uint16_t messageId, const QueryData & query, const chip::Inet::IPPacketInfo * querySource,
                       const ResponseConfiguration & configuration);

    // Implementation of ResponderDelegate
    void AddResponse(const ResourceRecord & record) override;
    bool ShouldSend(const Responder &) const override;
    void ResponsesAdded(const Responder &) override;

    void SetServer(ServerBase * server) { mServer = server; }

private:
    CHIP_ERROR FlushReply();
    CHIP_ERROR PrepareNewReplyPacket();

    ServerBase * mServer;
    QueryResponderPtrPool mResponders = {};

    /// Current send state
    ResponseBuilder mResponseBuilder;          // packet being built
    Internal::ResponseSendingState mSendState; // sending state
};

} // namespace Minimal
} // namespace mdns
