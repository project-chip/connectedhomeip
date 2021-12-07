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

#include <app/AppBuildConfig.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR ReportDataMessage::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t tagPresenceMask = 0;
    chip::TLV::TLVReader reader;
    AttributeReportIBs::Parser attributeReportIBs;
    EventReportIBs::Parser eventReportIBs;

    PRETTY_PRINT("ReportDataMessage =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);

        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_SuppressResponse:
            VerifyOrExit(!(tagPresenceMask & (1 << kCsTag_SuppressResponse)), err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << kCsTag_SuppressResponse);
            VerifyOrExit(chip::TLV::kTLVType_Boolean == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool SuppressResponse;
                err = reader.Get(SuppressResponse);
                SuccessOrExit(err);
                PRETTY_PRINT("\tSuppressResponse = %s, ", SuppressResponse ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_SubscriptionId:
            VerifyOrExit(!(tagPresenceMask & (1 << kCsTag_SubscriptionId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << kCsTag_SubscriptionId);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint64_t subscriptionId;
                err = reader.Get(subscriptionId);
                SuccessOrExit(err);
                PRETTY_PRINT("\tSubscriptionId = 0x%" PRIx64 ",", subscriptionId);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_AttributeReportIBs:
            // check if this tag has appeared before
            VerifyOrExit(!(tagPresenceMask & (1 << kCsTag_AttributeReportIBs)), err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << kCsTag_AttributeReportIBs);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                attributeReportIBs.Init(reader);

                PRETTY_PRINT_INCDEPTH();
                err = attributeReportIBs.CheckSchemaValidity();
                SuccessOrExit(err);
                PRETTY_PRINT_DECDEPTH();
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_EventReports:
            // check if this tag has appeared before
            VerifyOrExit(!(tagPresenceMask & (1 << kCsTag_EventReports)), err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << kCsTag_EventReports);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                eventReportIBs.Init(reader);

                PRETTY_PRINT_INCDEPTH();
                err = eventReportIBs.CheckSchemaValidity();
                SuccessOrExit(err);
                PRETTY_PRINT_DECDEPTH();
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_MoreChunkedMessages:
            VerifyOrExit(!(tagPresenceMask & (1 << kCsTag_MoreChunkedMessages)), err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << kCsTag_MoreChunkedMessages);
            VerifyOrExit(chip::TLV::kTLVType_Boolean == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool moreChunkedMessages;
                err = reader.Get(moreChunkedMessages);
                SuccessOrExit(err);
                PRETTY_PRINT("\tMoreChunkedMessages = %s, ", moreChunkedMessages ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            ExitNow(err = CHIP_ERROR_INVALID_TLV_TAG);
        }
    }

    PRETTY_PRINT("}");
    PRETTY_PRINT("");
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);
    err = reader.ExitContainer(mOuterContainerType);

exit:

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR ReportDataMessage::Parser::GetSuppressResponse(bool * const apSuppressResponse) const
{
    return GetSimpleValue(kCsTag_SuppressResponse, chip::TLV::kTLVType_Boolean, apSuppressResponse);
}

CHIP_ERROR ReportDataMessage::Parser::GetSubscriptionId(uint64_t * const apSubscriptionId) const
{
    return GetUnsignedInteger(kCsTag_SubscriptionId, apSubscriptionId);
}

CHIP_ERROR ReportDataMessage::Parser::GetAttributeReportIBs(AttributeReportIBs::Parser * const apAttributeReportIBs) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_AttributeReportIBs), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apAttributeReportIBs->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR ReportDataMessage::Parser::GetEventReports(EventReportIBs::Parser * const apEventReports) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_EventReports), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apEventReports->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR ReportDataMessage::Parser::GetMoreChunkedMessages(bool * const apMoreChunkedMessages) const
{
    return GetSimpleValue(kCsTag_MoreChunkedMessages, chip::TLV::kTLVType_Boolean, apMoreChunkedMessages);
}

ReportDataMessage::Builder & ReportDataMessage::Builder::SuppressResponse(const bool aSuppressResponse)
{
    // skip if error has already been set
    SuccessOrExit(mError);
    mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_SuppressResponse), aSuppressResponse);

exit:
    return *this;
}

ReportDataMessage::Builder & ReportDataMessage::Builder::SubscriptionId(const uint64_t aSubscriptionId)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_SubscriptionId), aSubscriptionId);
    }
    return *this;
}

AttributeReportIBs::Builder & ReportDataMessage::Builder::CreateAttributeReportIBs()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributeReportIBsBuilder.Init(mpWriter, kCsTag_AttributeReportIBs);
    }
    else
    {
        mAttributeReportIBsBuilder.ResetError(mError);
    }
    return mAttributeReportIBsBuilder;
}

EventReportIBs::Builder & ReportDataMessage::Builder::CreateEventReports()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mEventReportsBuilder.Init(mpWriter, kCsTag_EventReports);
    }
    else
    {
        mAttributeReportIBsBuilder.ResetError(mError);
    }
    return mEventReportsBuilder;
}

ReportDataMessage::Builder & ReportDataMessage::Builder::MoreChunkedMessages(const bool aMoreChunkedMessages)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_MoreChunkedMessages), aMoreChunkedMessages);
    }
    return *this;
}

ReportDataMessage::Builder & ReportDataMessage::Builder::EndOfReportDataMessage()
{
    EndOfContainer();
    return *this;
}
}; // namespace app
}; // namespace chip
