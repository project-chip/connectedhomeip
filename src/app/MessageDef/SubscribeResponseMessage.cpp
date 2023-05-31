/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "SubscribeResponseMessage.h"
#include "MessageDefHelper.h"

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR SubscribeResponseMessage::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;
    PRETTY_PRINT("SubscribeResponseMessage =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        if (!TLV::IsContextTag(reader.GetTag()))
        {
            continue;
        }
        uint32_t tagNum = TLV::TagNumFromTag(reader.GetTag());
        switch (tagNum)
        {
        case to_underlying(Tag::kSubscriptionId):
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                SubscriptionId subscriptionId;
                ReturnErrorOnFailure(reader.Get(subscriptionId));
                PRETTY_PRINT("\tSubscriptionId = 0x%" PRIx32 ",", subscriptionId);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kMaxInterval):
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t maxInterval;
                ReturnErrorOnFailure(reader.Get(maxInterval));
                PRETTY_PRINT("\tMaxInterval = 0x%x,", maxInterval);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kInteractionModelRevisionTag:
            ReturnErrorOnFailure(MessageParser::CheckInteractionModelRevision(reader));
            break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }
    PRETTY_PRINT("}");
    PRETTY_PRINT_BLANK_LINE();

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

CHIP_ERROR SubscribeResponseMessage::Parser::GetSubscriptionId(SubscriptionId * const apSubscribeId) const
{
    return GetUnsignedInteger(to_underlying(Tag::kSubscriptionId), apSubscribeId);
}

CHIP_ERROR SubscribeResponseMessage::Parser::GetMaxInterval(uint16_t * const apMaxInterval) const
{
    return GetUnsignedInteger(to_underlying(Tag::kMaxInterval), apMaxInterval);
}

SubscribeResponseMessage::Builder & SubscribeResponseMessage::Builder::SubscriptionId(const chip::SubscriptionId aSubscribeId)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(Tag::kSubscriptionId), aSubscribeId);
    }
    return *this;
}

SubscribeResponseMessage::Builder & SubscribeResponseMessage::Builder::MaxInterval(const uint16_t aMaxInterval)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(Tag::kMaxInterval), aMaxInterval);
    }
    return *this;
}

CHIP_ERROR SubscribeResponseMessage::Builder::EndOfSubscribeResponseMessage()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = MessageBuilder::EncodeInteractionModelRevision();
    }
    if (mError == CHIP_NO_ERROR)
    {
        EndOfContainer();
    }
    return GetError();
}
} // namespace app
} // namespace chip
