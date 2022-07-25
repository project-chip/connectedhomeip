/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "ReadRequestMessage.h"
#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR ReadRequestMessage::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int tagPresenceMask = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("ReadRequestMessage =");
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
        case to_underlying(Tag::kAttributeRequests):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kAttributeRequests))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kAttributeRequests));
            {
                AttributePathIBs::Parser attributeRequests;
                ReturnErrorOnFailure(attributeRequests.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(attributeRequests.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case to_underlying(Tag::kDataVersionFilters):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kDataVersionFilters))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kDataVersionFilters));
            {
                DataVersionFilterIBs::Parser dataVersionFilters;
                ReturnErrorOnFailure(dataVersionFilters.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(dataVersionFilters.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case to_underlying(Tag::kEventRequests):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kEventRequests))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kEventRequests));
            {
                EventPathIBs::Parser eventRequests;
                ReturnErrorOnFailure(eventRequests.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(eventRequests.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case to_underlying(Tag::kEventFilters):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kEventFilters))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kEventFilters));
            {
                EventFilterIBs::Parser eventFilters;
                ReturnErrorOnFailure(eventFilters.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(eventFilters.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case to_underlying(Tag::kIsFabricFiltered):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kIsFabricFiltered))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kIsFabricFiltered));
#if CHIP_DETAIL_LOGGING
            {
                bool isFabricFiltered;
                ReturnErrorOnFailure(reader.Get(isFabricFiltered));
                PRETTY_PRINT("\tisFabricFiltered = %s, ", isFabricFiltered ? "true" : "false");
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

    PRETTY_PRINT("},");
    PRETTY_PRINT_BLANK_LINE();

    if (CHIP_END_OF_TLV == err)
    {
        const int requiredFields = (1 << to_underlying(Tag::kIsFabricFiltered));
        err = (tagPresenceMask & requiredFields) == requiredFields ? CHIP_NO_ERROR : CHIP_ERROR_IM_MALFORMED_READ_REQUEST_MESSAGE;
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR ReadRequestMessage::Parser::GetAttributeRequests(AttributePathIBs::Parser * const apAttributeRequests) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kAttributeRequests)), reader));
    return apAttributeRequests->Init(reader);
}

CHIP_ERROR ReadRequestMessage::Parser::GetDataVersionFilters(DataVersionFilterIBs::Parser * const apDataVersionFilters) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kDataVersionFilters)), reader));
    return apDataVersionFilters->Init(reader);
}

CHIP_ERROR ReadRequestMessage::Parser::GetEventRequests(EventPathIBs::Parser * const apEventRequests) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kEventRequests)), reader));
    return apEventRequests->Init(reader);
}

CHIP_ERROR ReadRequestMessage::Parser::GetEventFilters(EventFilterIBs::Parser * const apEventFilters) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kEventFilters)), reader));
    return apEventFilters->Init(reader);
}

CHIP_ERROR ReadRequestMessage::Parser::GetIsFabricFiltered(bool * const apIsFabricFiltered) const
{
    return GetSimpleValue(to_underlying(Tag::kIsFabricFiltered), TLV::kTLVType_Boolean, apIsFabricFiltered);
}

AttributePathIBs::Builder & ReadRequestMessage::Builder::CreateAttributeRequests()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributeRequests.Init(mpWriter, to_underlying(Tag::kAttributeRequests));
    }
    return mAttributeRequests;
}

DataVersionFilterIBs::Builder & ReadRequestMessage::Builder::CreateDataVersionFilters()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mDataVersionFilters.Init(mpWriter, to_underlying(Tag::kDataVersionFilters));
    }
    return mDataVersionFilters;
}

EventPathIBs::Builder & ReadRequestMessage::Builder::CreateEventRequests()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mEventRequests.Init(mpWriter, to_underlying(Tag::kEventRequests));
    }
    return mEventRequests;
}

EventFilterIBs::Builder & ReadRequestMessage::Builder::CreateEventFilters()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mEventFilters.Init(mpWriter, to_underlying(Tag::kEventFilters));
    }
    return mEventFilters;
}

ReadRequestMessage::Builder & ReadRequestMessage::Builder::IsFabricFiltered(const bool aIsFabricFiltered)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(to_underlying(Tag::kIsFabricFiltered)), aIsFabricFiltered);
    }
    return *this;
}

ReadRequestMessage::Builder & ReadRequestMessage::Builder::EndOfReadRequestMessage()
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
