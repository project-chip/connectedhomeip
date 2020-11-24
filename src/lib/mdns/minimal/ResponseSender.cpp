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
constexpr uint16_t kPacketSizeBytes  = 512;

} // namespace

CHIP_ERROR ResponseSender::Respond(const QueryData & query, const chip::Inet::IPPacketInfo * querySource)
{
    mSendError = CHIP_NO_ERROR;

    mCurrentSource = querySource;
    mSendUnicast   = query.GetUnicastAnswer();

    mResponder->ResetAdditionals();

    mCurrentResourceType = ResourceType::kAnswer; // direct answer
    QueryReplyFilter filter(query);
    for (auto it = mResponder->begin(); it != mResponder->end(); it++)
    {
        Responder * responder = it->responder;

        if (!filter.SendAnswer(responder->GetQType(), responder->GetQClass(), responder->GetQName()))
        {
            continue;
        }

        responder->AddAllResponses(querySource, this);
        ReturnErrorOnFailure(mSendError);

        mResponder->MarkAdditionalReplyesFor(it);
    }

    mCurrentResourceType = ResourceType::kAdditional; // Additional parts
    filter.SetIgnoreNameMatch(true);
    for (auto it = mResponder->additional_begin(); it != mResponder->additional_end(); it++)
    {
        Responder * responder = it->responder;

        if (!filter.SendAnswer(responder->GetQType(), responder->GetQClass(), responder->GetQName()))
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

        if (!mSendUnicast && (mCurrentSource->SrcPort == kMdnsStandardPort))
        {
            ChipLogProgress(Discovery, "Broadcasting mDns reply");
            ReturnErrorOnFailure(
                mServer->BroadcastSend(mCurrentPacket.Release_ForNow(), kMdnsStandardPort, mCurrentSource->Interface));
        }
        else
        {
            ChipLogProgress(Discovery, "Directly sending mDns reply to peer on port %d", mCurrentSource->SrcPort);
            ReturnErrorOnFailure(mServer->DirectSend(mCurrentPacket.Release_ForNow(), mCurrentSource->SrcAddress,
                                                     mCurrentSource->SrcPort, mCurrentSource->Interface));
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
            mSendError = CHIP_ERROR_INTERNAL;
        }
    }
}

} // namespace Minimal
} // namespace mdns
