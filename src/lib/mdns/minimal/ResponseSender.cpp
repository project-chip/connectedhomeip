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

#include "ResponseSender.h"

#include "QueryReplyFilter.h"

#include <support/ReturnMacros.h>

#define RETURN_IF_ERROR(err)                                                                                                       \
    do                                                                                                                             \
    {                                                                                                                              \
        if (err)                                                                                                                   \
        {                                                                                                                          \
            return;                                                                                                                \
        }                                                                                                                          \
    } while (false)

namespace mdns {
namespace Minimal {

namespace {

constexpr uint16_t kMdnsStandardPort = 5353;

// Restriction for UDP packets:  https://tools.ietf.org/html/rfc1035#section-4.2.1
//
//    Messages carried by UDP are restricted to 512 bytes (not counting the IP
//    or UDP headers).  Longer messages are truncated and the TC bit is set in
//    the header.
constexpr uint16_t kPacketSizeBytes = 512;

} // namespace

CHIP_ERROR ResponseSender::Respond(uint32_t messageId, const QueryData & query, const chip::Inet::IPPacketInfo * querySource)
{
    mSendError = CHIP_NO_ERROR;

    mCurrentSource    = querySource;
    mCurrentMessageId = messageId;
    mSendUnicast      = query.RequestedUnicastAnswer() || (querySource->SrcPort != kMdnsStandardPort);
    // TODO: at this point we may want to ensure we protect against excessive multicast packet flooding.
    // According to https://tools.ietf.org/html/rfc6762#section-6  we should multicast at most 1/sec
    // TBD: do we filter out frequent multicasts or should we switch to unicast in those cases

    // Responder has a stateful 'additional replies required' that is used within the response
    // loop. 'no additionals required' is set at the start and additionals are marked as the query
    // reply is built.
    mResponder->ResetAdditionals();

    mCurrentResourceType = ResourceType::kAnswer; // direct answer
    QueryReplyFilter filter(query);
    for (auto it = mResponder->begin(); it != mResponder->end(); it++)
    {
        Responder * responder = it->responder;

        if (!filter.Accept(responder->GetQType(), responder->GetQClass(), responder->GetQName()))
        {
            continue;
        }

        responder->AddAllResponses(querySource, this);
        ReturnErrorOnFailure(mSendError);

        mResponder->MarkAdditionalRepliesFor(it);
    }

    mCurrentResourceType = ResourceType::kAdditional; // Additional parts
    filter.SetIgnoreNameMatch(true);
    for (auto it = mResponder->additional_begin(); it != mResponder->additional_end(); it++)
    {
        Responder * responder = it->responder;

        if (!filter.Accept(responder->GetQType(), responder->GetQClass(), responder->GetQName()))
        {
            continue;
        }

        it->responder->AddAllResponses(querySource, this);
        ReturnErrorOnFailure(mSendError);
    }

    return FlushReply();
}

CHIP_ERROR ResponseSender::FlushReply()
{
    ReturnErrorCodeIf(mCurrentPacket.IsNull(), CHIP_NO_ERROR); // nothing to flush

    if (mResponseBuilder.HasResponseRecords())
    {

        if (mSendUnicast)
        {
            ChipLogProgress(Discovery, "Directly sending mDns reply to peer on port %d", mCurrentSource->SrcPort);
            ReturnErrorOnFailure(mServer->DirectSend(mCurrentPacket.Release_ForNow(), mCurrentSource->SrcAddress,
                                                     mCurrentSource->SrcPort, mCurrentSource->Interface));
        }
        else
        {
            ChipLogProgress(Discovery, "Broadcasting mDns reply");
            ReturnErrorOnFailure(
                mServer->BroadcastSend(mCurrentPacket.Release_ForNow(), kMdnsStandardPort, mCurrentSource->Interface));
        }
        mResponseBuilder.Invalidate();
        mCurrentPacket.Adopt(nullptr);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ResponseSender::PrepareNewReplyPacket()
{
    mCurrentPacket = chip::System::PacketBuffer::NewWithAvailableSize(kPacketSizeBytes);
    ReturnErrorCodeIf(mCurrentPacket.IsNull(), CHIP_ERROR_NO_MEMORY);

    mResponseBuilder.Reset(mCurrentPacket.Get_ForNow());

    mResponseBuilder.Header().SetMessageId(mCurrentMessageId);

    return CHIP_NO_ERROR;
}

void ResponseSender::AddResponse(const ResourceRecord & record)
{
    RETURN_IF_ERROR(mSendError);

    if (mCurrentPacket.IsNull())
    {
        mSendError = PrepareNewReplyPacket();
        RETURN_IF_ERROR(mSendError);
    }

    if (!mResponseBuilder.Ok())
    {
        mSendError = CHIP_ERROR_INCORRECT_STATE;
        return;
    }

    mResponseBuilder.AddRecord(mCurrentResourceType, record);

    // respons build AddRecord will only fail if insufficient space is available (or at least this is
    // the assumption here). It also guarantees that existing data and header are unchanged on
    // failure, hence we can flush and try again. This allows for split replies.
    if (!mResponseBuilder.Ok())
    {
        mResponseBuilder.Header().SetFlags(mResponseBuilder.Header().GetFlags().SetTruncated(true));

        mSendError = FlushReply();
        RETURN_IF_ERROR(mSendError);

        mSendError = PrepareNewReplyPacket();
        RETURN_IF_ERROR(mSendError);

        mResponseBuilder.AddRecord(mCurrentResourceType, record);
        if (!mResponseBuilder.Ok())
        {
            // Very much unexpected: single record addtion should fit (our records should not be that big).
            ChipLogError(Discovery, "Failed to add single record to mDNS response.");
            mSendError = CHIP_ERROR_INTERNAL;
        }
    }
}

} // namespace Minimal
} // namespace mdns
