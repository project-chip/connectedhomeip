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

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include "InvokeRequestMessage.h"
#include "MessageDefHelper.h"

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR InvokeRequestMessage::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;

    PRETTY_PRINT("InvokeRequestMessage =");
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
#if CHIP_DETAIL_LOGGING
        {
            bool suppressResponse;
            ReturnErrorOnFailure(reader.Get(suppressResponse));
            PRETTY_PRINT("\tsuppressResponse = %s, ", suppressResponse ? "true" : "false");
        }
#endif // CHIP_DETAIL_LOGGING
        break;

        case to_underlying(Tag::kTimedRequest):
#if CHIP_DETAIL_LOGGING
        {
            bool timedRequest;
            ReturnErrorOnFailure(reader.Get(timedRequest));
            PRETTY_PRINT("\ttimedRequest = %s, ", timedRequest ? "true" : "false");
        }
#endif // CHIP_DETAIL_LOGGING
        break;
        case to_underlying(Tag::kInvokeRequests): {
            InvokeRequests::Parser invokeRequests;
            ReturnErrorOnFailure(invokeRequests.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(invokeRequests.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }
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

CHIP_ERROR InvokeRequestMessage::Parser::GetSuppressResponse(bool * const apSuppressResponse) const
{
    return GetSimpleValue(to_underlying(Tag::kSuppressResponse), TLV::kTLVType_Boolean, apSuppressResponse);
}

CHIP_ERROR InvokeRequestMessage::Parser::GetTimedRequest(bool * const apTimedRequest) const
{
    return GetSimpleValue(to_underlying(Tag::kTimedRequest), TLV::kTLVType_Boolean, apTimedRequest);
}

CHIP_ERROR InvokeRequestMessage::Parser::GetInvokeRequests(InvokeRequests::Parser * const apInvokeRequests) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kInvokeRequests), reader));
    return apInvokeRequests->Init(reader);
}

CHIP_ERROR InvokeRequestMessage::Builder::InitWithEndBufferReserved(TLV::TLVWriter * const apWriter)
{
    ReturnErrorOnFailure(Init(apWriter));
    ReturnErrorOnFailure(GetWriter()->ReserveBuffer(GetSizeToEndInvokeRequestMessage()));
    mIsEndBufferReserved = true;
    return CHIP_NO_ERROR;
}

InvokeRequestMessage::Builder & InvokeRequestMessage::Builder::SuppressResponse(const bool aSuppressResponse)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(Tag::kSuppressResponse), aSuppressResponse);
    }
    return *this;
}

InvokeRequestMessage::Builder & InvokeRequestMessage::Builder::TimedRequest(const bool aTimedRequest)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(Tag::kTimedRequest), aTimedRequest);
    }
    return *this;
}

InvokeRequests::Builder & InvokeRequestMessage::Builder::CreateInvokeRequests(const bool aReserveEndBuffer)
{
    if (mError == CHIP_NO_ERROR)
    {
        if (aReserveEndBuffer)
        {
            mError = mInvokeRequests.InitWithEndBufferReserved(mpWriter, to_underlying(Tag::kInvokeRequests));
        }
        else
        {
            mError = mInvokeRequests.Init(mpWriter, to_underlying(Tag::kInvokeRequests));
        }
    }
    return mInvokeRequests;
}

CHIP_ERROR InvokeRequestMessage::Builder::EndOfInvokeRequestMessage()
{
    // If any changes are made to how we end the invoke request message that involves how many
    // bytes are needed, a corresponding change to GetSizeToEndInvokeRequestMessage indicating
    // the new size that will be required.
    ReturnErrorOnFailure(mError);
    if (mIsEndBufferReserved)
    {
        ReturnErrorOnFailure(GetWriter()->UnreserveBuffer(GetSizeToEndInvokeRequestMessage()));
        mIsEndBufferReserved = false;
    }
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

uint32_t InvokeRequestMessage::Builder::GetSizeToEndInvokeRequestMessage()
{
    // EncodeInteractionModelRevision() encodes a uint8_t with context tag 0xFF. This means 1 control byte,
    // 1 byte for the tag, 1 byte for the value.
    uint32_t kEncodeInteractionModelSize = 1 + 1 + 1;
    uint32_t kEndOfContainerSize         = 1;

    return kEncodeInteractionModelSize + kEndOfContainerSize;
}
}; // namespace app
}; // namespace chip
