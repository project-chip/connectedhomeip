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
/**
 *    @file
 *      This file defines WriteRequestMessage parser and builder in CHIP interaction model
 *
 */

#include "WriteRequestMessage.h"
#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR WriteRequestMessage::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int tagPresenceMask = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("WriteRequestMessage =");
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
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kSuppressResponse))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kSuppressResponse));
#if CHIP_DETAIL_LOGGING
            {
                bool suppressResponse;
                ReturnErrorOnFailure(reader.Get(suppressResponse));
                PRETTY_PRINT("\tsuppressResponse = %s, ", suppressResponse ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;

        case to_underlying(Tag::kTimedRequest):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kTimedRequest))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kTimedRequest));
#if CHIP_DETAIL_LOGGING
            {
                bool timedRequest;
                ReturnErrorOnFailure(reader.Get(timedRequest));
                PRETTY_PRINT("\ttimedRequest = %s, ", timedRequest ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kWriteRequests):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kWriteRequests))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kWriteRequests));
            {
                AttributeDataIBs::Parser writeRequests;
                ReturnErrorOnFailure(writeRequests.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(writeRequests.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case to_underlying(Tag::kMoreChunkedMessages):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kMoreChunkedMessages))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kMoreChunkedMessages));
#if CHIP_DETAIL_LOGGING
            {
                bool moreChunkedMessages;
                ReturnErrorOnFailure(reader.Get(moreChunkedMessages));
                PRETTY_PRINT("\tmoreChunkedMessages = %s, ", moreChunkedMessages ? "true" : "false");
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
        const int requiredFields = ((1 << to_underlying(Tag::kTimedRequest)) | (1 << to_underlying(Tag::kWriteRequests)));
        err = (tagPresenceMask & requiredFields) == requiredFields ? CHIP_NO_ERROR : CHIP_ERROR_IM_MALFORMED_WRITE_REQUEST_MESSAGE;
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR WriteRequestMessage::Parser::GetSuppressResponse(bool * const apSuppressResponse) const
{
    return GetSimpleValue(to_underlying(Tag::kSuppressResponse), TLV::kTLVType_Boolean, apSuppressResponse);
}

CHIP_ERROR WriteRequestMessage::Parser::GetTimedRequest(bool * const apTimedRequest) const
{
    return GetSimpleValue(to_underlying(Tag::kTimedRequest), TLV::kTLVType_Boolean, apTimedRequest);
}

CHIP_ERROR WriteRequestMessage::Parser::GetWriteRequests(AttributeDataIBs::Parser * const apAttributeDataIBs) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kWriteRequests)), reader));
    return apAttributeDataIBs->Init(reader);
}

CHIP_ERROR WriteRequestMessage::Parser::GetMoreChunkedMessages(bool * const apMoreChunkedMessages) const
{
    return GetSimpleValue(to_underlying(Tag::kMoreChunkedMessages), TLV::kTLVType_Boolean, apMoreChunkedMessages);
}

WriteRequestMessage::Builder & WriteRequestMessage::Builder::SuppressResponse(const bool aSuppressResponse)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(to_underlying(Tag::kSuppressResponse)), aSuppressResponse);
    }
    return *this;
}

WriteRequestMessage::Builder & WriteRequestMessage::Builder::TimedRequest(const bool aTimedRequest)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(to_underlying(Tag::kTimedRequest)), aTimedRequest);
    }
    return *this;
}

AttributeDataIBs::Builder & WriteRequestMessage::Builder::CreateWriteRequests()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mWriteRequests.Init(mpWriter, to_underlying(Tag::kWriteRequests));
    }
    return mWriteRequests;
}

WriteRequestMessage::Builder & WriteRequestMessage::Builder::MoreChunkedMessages(const bool aMoreChunkedMessages)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(to_underlying(Tag::kMoreChunkedMessages)), aMoreChunkedMessages);
    }
    return *this;
}

WriteRequestMessage::Builder & WriteRequestMessage::Builder::EndOfWriteRequestMessage()
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
