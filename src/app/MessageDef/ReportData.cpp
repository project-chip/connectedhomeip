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
 *      This file defines ReportData parser and builder in CHIP interaction model
 *
 */

#include "ReportData.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
CHIP_ERROR ReportData::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the reader here
    mReader.Init(aReader);

    VerifyOrExit(chip::TLV::kTLVType_Structure == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = mReader.EnterContainer(mOuterContainerType);

exit:
    ChipLogFunctError(err);

    return err;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR ReportData::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;
    AttributeDataList::Parser attributeDataList;
    EventList::Parser eventList;

    PRETTY_PRINT("ReportData =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);

        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_SuppressResponse:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_SuppressResponse)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_SuppressResponse);
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
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_SubscriptionId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_SubscriptionId);
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
        case kCsTag_AttributeDataList:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_AttributeDataList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributeDataList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                attributeDataList.Init(reader);

                PRETTY_PRINT_INCDEPTH();
                err = attributeDataList.CheckSchemaValidity();
                SuccessOrExit(err);
                PRETTY_PRINT_DECDEPTH();
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_EventDataList:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_EventDataList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EventDataList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                eventList.Init(reader);

                PRETTY_PRINT_INCDEPTH();
                err = eventList.CheckSchemaValidity();
                SuccessOrExit(err);
                PRETTY_PRINT_DECDEPTH();
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_MoreChunkedMessages:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_MoreChunkedMessages)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_MoreChunkedMessages);
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
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR ReportData::Parser::GetSuppressResponse(bool * const apSuppressResponse) const
{
    return GetSimpleValue(kCsTag_SuppressResponse, chip::TLV::kTLVType_Boolean, apSuppressResponse);
}

CHIP_ERROR ReportData::Parser::GetSubscriptionId(uint64_t * const apSubscriptionId) const
{
    return GetUnsignedInteger(kCsTag_SubscriptionId, apSubscriptionId);
}

CHIP_ERROR ReportData::Parser::GetAttributeDataList(AttributeDataList::Parser * const apAttributeDataList) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_AttributeDataList), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apAttributeDataList->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR ReportData::Parser::GetEventDataList(EventList::Parser * const apEventDataList) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_EventDataList), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apEventDataList->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR ReportData::Parser::GetMoreChunkedMessages(bool * const apMoreChunkedMessages) const
{
    return GetSimpleValue(kCsTag_MoreChunkedMessages, chip::TLV::kTLVType_Boolean, apMoreChunkedMessages);
}

CHIP_ERROR ReportData::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

ReportData::Builder & ReportData::Builder::SuppressResponse(const bool aSuppressResponse)
{
    // skip if error has already been set
    SuccessOrExit(mError);
    mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_SuppressResponse), aSuppressResponse);

exit:
    ChipLogFunctError(mError);
    return *this;
}

ReportData::Builder & ReportData::Builder::SubscriptionId(const uint64_t aSubscriptionId)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_SubscriptionId), aSubscriptionId);
        ChipLogFunctError(mError);
    }
    return *this;
}

AttributeDataList::Builder & ReportData::Builder::CreateAttributeDataListBuilder()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributeDataListBuilder.Init(mpWriter, kCsTag_AttributeDataList);
        ChipLogFunctError(mError);
    }
    else
    {
        mAttributeDataListBuilder.ResetError(mError);
    }
    return mAttributeDataListBuilder;
}

EventList::Builder & ReportData::Builder::CreateEventDataListBuilder()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mEventDataListBuilder.Init(mpWriter, kCsTag_EventDataList);
        ChipLogFunctError(mError);
    }
    else
    {
        mAttributeDataListBuilder.ResetError(mError);
    }
    return mEventDataListBuilder;
}

ReportData::Builder & ReportData::Builder::MoreChunkedMessages(const bool aMoreChunkedMessages)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_MoreChunkedMessages), aMoreChunkedMessages);
        ChipLogFunctError(mError);
    }
    return *this;
}

ReportData::Builder & ReportData::Builder::EndOfReportData()
{
    EndOfContainer();
    return *this;
}
}; // namespace app
}; // namespace chip
