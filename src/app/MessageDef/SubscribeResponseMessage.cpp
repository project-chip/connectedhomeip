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
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR SubscribeResponseMessage::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int tagPresenceMask = 0;
    TLV::TLVReader reader;
    PRETTY_PRINT("SubscribeResponseMessage =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::IsContextTag(reader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
        uint32_t tagNum = TLV::TagNumFromTag(reader.GetTag());
        switch (tagNum)
        {
        case to_underlying(Tag::kSubscriptionId):
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kSubscriptionId))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kSubscriptionId));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint64_t subscriptionId;
                ReturnErrorOnFailure(reader.Get(subscriptionId));
                PRETTY_PRINT("\tSubscriptionId = 0x%" PRIx64 ",", subscriptionId);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kMinIntervalFloorSeconds):
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kMinIntervalFloorSeconds))),
                                CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kMinIntervalFloorSeconds));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t minIntervalFloorSeconds;
                ReturnErrorOnFailure(reader.Get(minIntervalFloorSeconds));
                PRETTY_PRINT("\tMinIntervalFloorSeconds = 0x%" PRIx16 ",", minIntervalFloorSeconds);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kMaxIntervalCeilingSeconds):
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kMaxIntervalCeilingSeconds))),
                                CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kMaxIntervalCeilingSeconds));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t maxIntervalCeilingSeconds;
                ReturnErrorOnFailure(reader.Get(maxIntervalCeilingSeconds));
                PRETTY_PRINT("\tMaxIntervalCeilingSeconds = 0x%" PRIx16 ",", maxIntervalCeilingSeconds);
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
    PRETTY_PRINT("");

    if (CHIP_END_OF_TLV == err)
    {
        const uint16_t RequiredFields = (1 << to_underlying(Tag::kSubscriptionId)) |
            (1 << to_underlying(Tag::kMinIntervalFloorSeconds)) | (1 << to_underlying(Tag::kMaxIntervalCeilingSeconds));

        if ((tagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_SUBSCRIBE_RESPONSE_MESSAGE;
        }
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR SubscribeResponseMessage::Parser::GetSubscriptionId(uint64_t * const apSubscribeId) const
{
    return GetUnsignedInteger(to_underlying(Tag::kSubscriptionId), apSubscribeId);
}

CHIP_ERROR SubscribeResponseMessage::Parser::GetMinIntervalFloorSeconds(uint16_t * const apMinIntervalFloorSeconds) const
{
    return GetUnsignedInteger(to_underlying(Tag::kMinIntervalFloorSeconds), apMinIntervalFloorSeconds);
}

CHIP_ERROR SubscribeResponseMessage::Parser::GetMaxIntervalCeilingSeconds(uint16_t * const apMaxIntervalCeilingSeconds) const
{
    return GetUnsignedInteger(to_underlying(Tag::kMaxIntervalCeilingSeconds), apMaxIntervalCeilingSeconds);
}

SubscribeResponseMessage::Builder & SubscribeResponseMessage::Builder::SubscriptionId(const uint64_t aSubscribeId)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kSubscriptionId)), aSubscribeId);
    }
    return *this;
}

SubscribeResponseMessage::Builder &
SubscribeResponseMessage::Builder::MinIntervalFloorSeconds(const uint16_t aMinIntervalFloorSeconds)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kMinIntervalFloorSeconds)), aMinIntervalFloorSeconds);
    }
    return *this;
}

SubscribeResponseMessage::Builder &
SubscribeResponseMessage::Builder::MaxIntervalCeilingSeconds(const uint16_t aMaxIntervalCeilingSeconds)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kMaxIntervalCeilingSeconds)), aMaxIntervalCeilingSeconds);
    }
    return *this;
}

SubscribeResponseMessage::Builder & SubscribeResponseMessage::Builder::EndOfSubscribeResponseMessage()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = MessageBuilder::EncodeInteractionModelRevision();
    }
    if (mError == CHIP_NO_ERROR)
    {
        EndOfContainer();
    }
    return *this;
}
} // namespace app
} // namespace chip
