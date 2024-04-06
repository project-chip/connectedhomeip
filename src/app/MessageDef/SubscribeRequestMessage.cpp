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

#include "SubscribeRequestMessage.h"
#include "MessageDefHelper.h"

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR SubscribeRequestMessage::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;

    PRETTY_PRINT("SubscribeRequestMessage =");
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
        case to_underlying(Tag::kKeepSubscriptions):
            VerifyOrReturnError(TLV::kTLVType_Boolean == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool keepSubscriptions;
                ReturnErrorOnFailure(reader.Get(keepSubscriptions));
                PRETTY_PRINT("\tKeepSubscriptions = %s, ", keepSubscriptions ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kMinIntervalFloorSeconds):
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t MinIntervalFloorSeconds;
                ReturnErrorOnFailure(reader.Get(MinIntervalFloorSeconds));
                PRETTY_PRINT("\tMinIntervalFloorSeconds = 0x%x,", MinIntervalFloorSeconds);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kMaxIntervalCeilingSeconds):
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t MaxIntervalCeilingSeconds;
                ReturnErrorOnFailure(reader.Get(MaxIntervalCeilingSeconds));
                PRETTY_PRINT("\tMaxIntervalCeilingSeconds = 0x%x,", MaxIntervalCeilingSeconds);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kAttributeRequests): {
            AttributePathIBs::Parser attributeRequests;
            ReturnErrorOnFailure(attributeRequests.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(attributeRequests.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }
        break;
        case to_underlying(Tag::kDataVersionFilters): {
            DataVersionFilterIBs::Parser dataVersionFilters;
            ReturnErrorOnFailure(dataVersionFilters.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(dataVersionFilters.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }
        break;
        case to_underlying(Tag::kEventRequests): {
            EventPathIBs::Parser eventRequests;
            ReturnErrorOnFailure(eventRequests.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(eventRequests.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }
        break;
        case to_underlying(Tag::kEventFilters): {
            EventFilterIBs::Parser eventFilters;
            ReturnErrorOnFailure(eventFilters.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(eventFilters.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }
        break;
        case to_underlying(Tag::kIsFabricFiltered):
#if CHIP_DETAIL_LOGGING
        {
            bool isFabricFiltered;
            ReturnErrorOnFailure(reader.Get(isFabricFiltered));
            PRETTY_PRINT("\tisFabricFiltered = %s, ", isFabricFiltered ? "true" : "false");
        }
#endif // CHIP_DETAIL_LOGGING
        break;
        case Revision::kInteractionModelRevisionTag:
            ReturnErrorOnFailure(MessageParser::CheckInteractionModelRevision(reader));
            break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }

    PRETTY_PRINT("},");
    PRETTY_PRINT_BLANK_LINE();

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

CHIP_ERROR SubscribeRequestMessage::Parser::GetKeepSubscriptions(bool * const apKeepExistingSubscription) const
{
    return GetSimpleValue(to_underlying(Tag::kKeepSubscriptions), TLV::kTLVType_Boolean, apKeepExistingSubscription);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetMinIntervalFloorSeconds(uint16_t * const apMinIntervalFloorSeconds) const
{
    return GetUnsignedInteger(to_underlying(Tag::kMinIntervalFloorSeconds), apMinIntervalFloorSeconds);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetMaxIntervalCeilingSeconds(uint16_t * const apMaxIntervalCeilingSeconds) const
{
    return GetUnsignedInteger(to_underlying(Tag::kMaxIntervalCeilingSeconds), apMaxIntervalCeilingSeconds);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetAttributeRequests(AttributePathIBs::Parser * const apAttributeRequests) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kAttributeRequests), reader));
    return apAttributeRequests->Init(reader);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetDataVersionFilters(DataVersionFilterIBs::Parser * const apDataVersionFilters) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kDataVersionFilters), reader));
    return apDataVersionFilters->Init(reader);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetEventRequests(EventPathIBs::Parser * const apEventRequests) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kEventRequests), reader));
    return apEventRequests->Init(reader);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetEventFilters(EventFilterIBs::Parser * const apEventFilters) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kEventFilters), reader));
    return apEventFilters->Init(reader);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetIsFabricFiltered(bool * const apIsFabricFiltered) const
{
    return GetSimpleValue(to_underlying(Tag::kIsFabricFiltered), TLV::kTLVType_Boolean, apIsFabricFiltered);
}

SubscribeRequestMessage::Builder & SubscribeRequestMessage::Builder::KeepSubscriptions(const bool aKeepSubscriptions)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(Tag::kKeepSubscriptions), aKeepSubscriptions);
    }
    return *this;
}

SubscribeRequestMessage::Builder &
SubscribeRequestMessage::Builder::MinIntervalFloorSeconds(const uint16_t aMinIntervalFloorSeconds)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(Tag::kMinIntervalFloorSeconds), aMinIntervalFloorSeconds);
    }
    return *this;
}

SubscribeRequestMessage::Builder &
SubscribeRequestMessage::Builder::MaxIntervalCeilingSeconds(const uint16_t aMaxIntervalCeilingSeconds)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(Tag::kMaxIntervalCeilingSeconds), aMaxIntervalCeilingSeconds);
    }
    return *this;
}

AttributePathIBs::Builder & SubscribeRequestMessage::Builder::CreateAttributeRequests()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributeRequests.Init(mpWriter, to_underlying(Tag::kAttributeRequests));
    }
    return mAttributeRequests;
}

DataVersionFilterIBs::Builder & SubscribeRequestMessage::Builder::CreateDataVersionFilters()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mDataVersionFilters.Init(mpWriter, to_underlying(Tag::kDataVersionFilters));
    }
    return mDataVersionFilters;
}

EventPathIBs::Builder & SubscribeRequestMessage::Builder::CreateEventRequests()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mEventRequests.Init(mpWriter, to_underlying(Tag::kEventRequests));
    }
    return mEventRequests;
}

EventFilterIBs::Builder & SubscribeRequestMessage::Builder::CreateEventFilters()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mEventFilters.Init(mpWriter, to_underlying(Tag::kEventFilters));
    }
    return mEventFilters;
}

SubscribeRequestMessage::Builder & SubscribeRequestMessage::Builder::IsFabricFiltered(const bool aIsFabricFiltered)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(Tag::kIsFabricFiltered), aIsFabricFiltered);
    }
    return *this;
}

CHIP_ERROR SubscribeRequestMessage::Builder::EndOfSubscribeRequestMessage()
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
