/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
/**
 *    @file
 *      This file defines ReportDataMessage parser and builder in CHIP interaction model
 *
 */

#include "ReportDataMessage.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

using namespace chip;

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR ReportDataMessage::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int tagPresenceMask = 0;
    TLV::TLVReader reader;
    AttributeReportIBs::Parser attributeReportIBs;
    EventReportIBs::Parser eventReportIBs;

    PRETTY_PRINT("ReportDataMessage =");
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
        case to_underlying(Tag::kSuppressResponse):
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kSuppressResponse))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kSuppressResponse));
            VerifyOrReturnError(TLV::kTLVType_Boolean == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool SuppressResponse;
                ReturnErrorOnFailure(reader.Get(SuppressResponse));
                PRETTY_PRINT("\tSuppressResponse = %s, ", SuppressResponse ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kSubscriptionId):
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kSubscriptionId))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kSubscriptionId));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                SubscriptionId subscriptionId;
                ReturnErrorOnFailure(reader.Get(subscriptionId));
                PRETTY_PRINT("\tSubscriptionId = 0x%" PRIx32 ",", subscriptionId);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kAttributeReportIBs):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kAttributeReportIBs))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kAttributeReportIBs));
            VerifyOrReturnError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                attributeReportIBs.Init(reader);

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(attributeReportIBs.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kEventReports):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kEventReports))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kEventReports));
            VerifyOrReturnError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                eventReportIBs.Init(reader);

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(eventReportIBs.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kMoreChunkedMessages):
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kMoreChunkedMessages))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kMoreChunkedMessages));
            VerifyOrReturnError(TLV::kTLVType_Boolean == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool moreChunkedMessages;
                ReturnErrorOnFailure(reader.Get(moreChunkedMessages));
                PRETTY_PRINT("\tMoreChunkedMessages = %s, ", moreChunkedMessages ? "true" : "false");
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
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR ReportDataMessage::Parser::GetSuppressResponse(bool * const apSuppressResponse) const
{
    return GetSimpleValue(to_underlying(Tag::kSuppressResponse), TLV::kTLVType_Boolean, apSuppressResponse);
}

CHIP_ERROR ReportDataMessage::Parser::GetSubscriptionId(SubscriptionId * const apSubscriptionId) const
{
    return GetUnsignedInteger(to_underlying(Tag::kSubscriptionId), apSubscriptionId);
}

CHIP_ERROR ReportDataMessage::Parser::GetAttributeReportIBs(AttributeReportIBs::Parser * const apAttributeReportIBs) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kAttributeReportIBs)), reader));
    return apAttributeReportIBs->Init(reader);
}

CHIP_ERROR ReportDataMessage::Parser::GetEventReports(EventReportIBs::Parser * const apEventReports) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kEventReports)), reader));
    return apEventReports->Init(reader);
}

CHIP_ERROR ReportDataMessage::Parser::GetMoreChunkedMessages(bool * const apMoreChunkedMessages) const
{
    return GetSimpleValue(to_underlying(Tag::kMoreChunkedMessages), TLV::kTLVType_Boolean, apMoreChunkedMessages);
}

ReportDataMessage::Builder & ReportDataMessage::Builder::SuppressResponse(const bool aSuppressResponse)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(to_underlying(Tag::kSuppressResponse)), aSuppressResponse);
    }
    return *this;
}

ReportDataMessage::Builder & ReportDataMessage::Builder::SubscriptionId(const chip::SubscriptionId aSubscriptionId)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kSubscriptionId)), aSubscriptionId);
    }
    return *this;
}

AttributeReportIBs::Builder & ReportDataMessage::Builder::CreateAttributeReportIBs()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributeReportIBsBuilder.Init(mpWriter, to_underlying(Tag::kAttributeReportIBs));
    }
    return mAttributeReportIBsBuilder;
}

EventReportIBs::Builder & ReportDataMessage::Builder::CreateEventReports()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mEventReportsBuilder.Init(mpWriter, to_underlying(Tag::kEventReports));
    }
    return mEventReportsBuilder;
}

ReportDataMessage::Builder & ReportDataMessage::Builder::MoreChunkedMessages(const bool aMoreChunkedMessages)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(to_underlying(Tag::kMoreChunkedMessages)), aMoreChunkedMessages);
    }
    return *this;
}

ReportDataMessage::Builder & ReportDataMessage::Builder::EndOfReportDataMessage()
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
