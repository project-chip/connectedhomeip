/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include "ResponseSender.h"

#include "QueryReplyFilter.h"

#include <system/SystemClock.h>

namespace mdns {
namespace Minimal {

namespace {

using namespace mdns::Minimal::Internal;

constexpr uint16_t kMdnsStandardPort = 5353;

// Restriction for UDP packets:  https://tools.ietf.org/html/rfc1035#section-4.2.1
//
//    Messages carried by UDP are restricted to 512 bytes (not counting the IP
//    or UDP headers).  Longer messages are truncated and the TC bit is set in
//    the header.
constexpr uint16_t kPacketSizeBytes = 512;

} // namespace
namespace Internal {

bool ResponseSendingState::SendUnicast() const
{
    return mQuery->RequestedUnicastAnswer() || (mSource->SrcPort != kMdnsStandardPort);
}

bool ResponseSendingState::IncludeQuery() const
{
    return (mSource->SrcPort != kMdnsStandardPort);
}

} // namespace Internal

CHIP_ERROR ResponseSender::AddQueryResponder(QueryResponderBase * queryResponder)
{
    // If already existing or we find a free slot, just use it
    // Note that dynamic memory implementations are never expected to be nullptr
    //
    for (auto & responder : mResponders)
    {
        if (responder == nullptr || responder == queryResponder)
        {
            responder = queryResponder;
            return CHIP_NO_ERROR;
        }
    }

#if CHIP_CONFIG_MINMDNS_DYNAMIC_OPERATIONAL_RESPONDER_LIST
    mResponders.push_back(queryResponder);
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NO_MEMORY;
#endif
}

CHIP_ERROR ResponseSender::RemoveQueryResponder(QueryResponderBase * queryResponder)
{
    for (auto it = mResponders.begin(); it != mResponders.end(); it++)
    {
        if (*it == queryResponder)
        {
            *it = nullptr;
#if CHIP_CONFIG_MINMDNS_DYNAMIC_OPERATIONAL_RESPONDER_LIST
            mResponders.erase(it);
#endif
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_ERROR_NOT_FOUND;
}

bool ResponseSender::HasQueryResponders() const
{
    for (auto responder : mResponders)
    {
        if (responder != nullptr)
        {
            return true;
        }
    }
    return false;
}

CHIP_ERROR ResponseSender::Respond(uint16_t messageId, const QueryData & query, const chip::Inet::IPPacketInfo * querySource,
                                   const ResponseConfiguration & configuration)
{
    mSendState.Reset(messageId, query, querySource);

    if (query.IsAnnounceBroadcast())
    {
        // Deny listing large amount of data
        mSendState.MarkWasSent(ResponseItemsSent::kServiceListingData);
    }

    // Responder has a stateful 'additional replies required' that is used within the response
    // loop. 'no additionals required' is set at the start and additionals are marked as the query
    // reply is built.
    for (auto & responder : mResponders)
    {
        {
            if (responder != nullptr)
            {
                responder->ResetAdditionals();
            }
        }
    }

    // send all 'Answer' replies
    {
        const chip::System::Clock::Timestamp kTimeNow = chip::System::SystemClock().GetMonotonicTimestamp();

        QueryReplyFilter queryReplyFilter(query);
        QueryResponderRecordFilter responseFilter;

        responseFilter.SetReplyFilter(&queryReplyFilter);

        if (!mSendState.SendUnicast())
        {
            // According to https://tools.ietf.org/html/rfc6762#section-6  we should multicast at most 1/sec
            //
            // TODO: the 'last sent' value does NOT track the interface we used to send, so this may cause
            //       broadcasts on one interface to throttle broadcasts on another interface.
            responseFilter.SetIncludeOnlyMulticastBeforeMS(kTimeNow - chip::System::Clock::Seconds32(1));
        }
        for (auto & responder : mResponders)
        {
            if (responder == nullptr)
            {
                continue;
            }
            for (auto it = responder->begin(&responseFilter); it != responder->end(); it++)
            {
                it->responder->AddAllResponses(querySource, this, configuration);
                ReturnErrorOnFailure(mSendState.GetError());

                responder->MarkAdditionalRepliesFor(it);

                if (!mSendState.SendUnicast())
                {
                    it->lastMulticastTime = kTimeNow;
                }
            }
        }
    }

    // send all 'Additional' replies
    {
        if (!query.IsAnnounceBroadcast())
        {
            // Initial service broadcast should keep adding data as 'Answers' rather
            // than addtional data (https://datatracker.ietf.org/doc/html/rfc6762#section-8.3)
            mSendState.SetResourceType(ResourceType::kAdditional);
        }

        QueryReplyFilter queryReplyFilter(query);

        queryReplyFilter.SetIgnoreNameMatch(true).SetSendingAdditionalItems(true);

        QueryResponderRecordFilter responseFilter;
        responseFilter
            .SetReplyFilter(&queryReplyFilter) //
            .SetIncludeAdditionalRepliesOnly(true);
        for (auto & responder : mResponders)
        {
            if (responder == nullptr)
            {
                continue;
            }
            for (auto it = responder->begin(&responseFilter); it != responder->end(); it++)
            {
                it->responder->AddAllResponses(querySource, this, configuration);
                ReturnErrorOnFailure(mSendState.GetError());
            }
        }
    }

    return FlushReply();
}

CHIP_ERROR ResponseSender::FlushReply()
{
    ReturnErrorCodeIf(!mResponseBuilder.HasPacketBuffer(), CHIP_NO_ERROR); // nothing to flush

    if (mResponseBuilder.HasResponseRecords())
    {
        char srcAddressString[chip::Inet::IPAddress::kMaxStringLength];
        VerifyOrDie(mSendState.GetSourceAddress().ToString(srcAddressString) != nullptr);

        if (mSendState.SendUnicast())
        {
#if CHIP_MINMDNS_HIGH_VERBOSITY
            ChipLogDetail(Discovery, "Directly sending mDns reply to peer %s on port %d", srcAddressString,
                          mSendState.GetSourcePort());
#endif
            ReturnErrorOnFailure(mServer->DirectSend(mResponseBuilder.ReleasePacket(), mSendState.GetSourceAddress(),
                                                     mSendState.GetSourcePort(), mSendState.GetSourceInterfaceId()));
        }
        else
        {
#if CHIP_MINMDNS_HIGH_VERBOSITY
            ChipLogDetail(Discovery, "Broadcasting mDns reply for query from %s", srcAddressString);
#endif
            ReturnErrorOnFailure(mServer->BroadcastSend(mResponseBuilder.ReleasePacket(), kMdnsStandardPort,
                                                        mSendState.GetSourceInterfaceId(), mSendState.GetSourceAddress().Type()));
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ResponseSender::PrepareNewReplyPacket()
{
    chip::System::PacketBufferHandle buffer = chip::System::PacketBufferHandle::New(kPacketSizeBytes);
    ReturnErrorCodeIf(buffer.IsNull(), CHIP_ERROR_NO_MEMORY);

    mResponseBuilder.Reset(std::move(buffer));
    mResponseBuilder.Header().SetMessageId(mSendState.GetMessageId());

    if (mSendState.IncludeQuery())
    {
        mResponseBuilder.AddQuery(*mSendState.GetQuery());
    }

    return CHIP_NO_ERROR;
}

bool ResponseSender::ShouldSend(const Responder & responder) const
{
    switch (responder.GetQType())
    {
    case QType::A:
        return !mSendState.GetWasSent(ResponseItemsSent::kIPv4Addresses);
    case QType::AAAA:
        return !mSendState.GetWasSent(ResponseItemsSent::kIPv6Addresses);
    case QType::PTR: {
        static const QNamePart kDnsSdQueryPath[] = { "_services", "_dns-sd", "_udp", "local" };

        if (responder.GetQName() == FullQName(kDnsSdQueryPath))
        {
            return !mSendState.GetWasSent(ResponseItemsSent::kServiceListingData);
        }
        break;
    }
    default:
        break;
    }

    return true;
}

void ResponseSender::ResponsesAdded(const Responder & responder)
{
    switch (responder.GetQType())
    {
    case QType::A:
        mSendState.MarkWasSent(ResponseItemsSent::kIPv4Addresses);
        break;
    case QType::AAAA:
        mSendState.MarkWasSent(ResponseItemsSent::kIPv6Addresses);
        break;
    default:
        break;
    }
}

void ResponseSender::AddResponse(const ResourceRecord & record)
{
    ReturnOnFailure(mSendState.GetError());

    if (!mResponseBuilder.HasPacketBuffer())
    {
        mSendState.SetError(PrepareNewReplyPacket());
        ReturnOnFailure(mSendState.GetError());
    }

    if (!mResponseBuilder.Ok())
    {
        mSendState.SetError(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    mResponseBuilder.AddRecord(mSendState.GetResourceType(), record);

    // ResponseBuilder AddRecord will only fail if insufficient space is available (or at least this is
    // the assumption here). It also guarantees that existing data and header are unchanged on
    // failure, hence we can flush and try again. This allows for split replies.
    if (!mResponseBuilder.Ok())
    {
        mResponseBuilder.Header().SetFlags(mResponseBuilder.Header().GetFlags().SetTruncated(true));

        ReturnOnFailure(mSendState.SetError(FlushReply()));
        ReturnOnFailure(mSendState.SetError(PrepareNewReplyPacket()));

        mResponseBuilder.AddRecord(mSendState.GetResourceType(), record);
        if (!mResponseBuilder.Ok())
        {
            // Very much unexpected: single record addition should fit (our records should not be that big).
            ChipLogError(Discovery, "Failed to add single record to mDNS response.");
            mSendState.SetError(CHIP_ERROR_INTERNAL);
        }
    }
}

} // namespace Minimal
} // namespace mdns
