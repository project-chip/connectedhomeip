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

#include "InvokeResponseMessage.h"
#include "MessageDefHelper.h"

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR InvokeResponseMessage::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;

    PRETTY_PRINT("InvokeResponseMessage =");
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
        case to_underlying(Tag::kInvokeResponses): {
            InvokeResponseIBs::Parser invokeResponses;
            ReturnErrorOnFailure(invokeResponses.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(invokeResponses.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }
        break;
        case to_underlying(Tag::kMoreChunkedMessages):
#if CHIP_DETAIL_LOGGING
        {
            bool moreChunkedMessages;
            ReturnErrorOnFailure(reader.Get(moreChunkedMessages));
            PRETTY_PRINT("\tmoreChunkedMessages = %s, ", moreChunkedMessages ? "true" : "false");
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

CHIP_ERROR InvokeResponseMessage::Parser::GetSuppressResponse(bool * const apSuppressResponse) const
{
    return GetSimpleValue(to_underlying(Tag::kSuppressResponse), TLV::kTLVType_Boolean, apSuppressResponse);
}

CHIP_ERROR InvokeResponseMessage::Parser::GetInvokeResponses(InvokeResponseIBs::Parser * const apStatus) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kInvokeResponses), reader));
    return apStatus->Init(reader);
}

CHIP_ERROR InvokeResponseMessage::Parser::GetMoreChunkedMessages(bool * const apMoreChunkedMessages) const
{
    return GetSimpleValue(to_underlying(Tag::kMoreChunkedMessages), TLV::kTLVType_Boolean, apMoreChunkedMessages);
}

CHIP_ERROR InvokeResponseMessage::Builder::InitWithEndBufferReserved(TLV::TLVWriter * const apWriter)
{
    ReturnErrorOnFailure(Init(apWriter));
    ReturnErrorOnFailure(GetWriter()->ReserveBuffer(GetSizeToEndInvokeResponseMessage()));
    mIsEndBufferReserved = true;
    return CHIP_NO_ERROR;
}

InvokeResponseMessage::Builder & InvokeResponseMessage::Builder::SuppressResponse(const bool aSuppressResponse)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(Tag::kSuppressResponse), aSuppressResponse);
    }
    return *this;
}

InvokeResponseIBs::Builder & InvokeResponseMessage::Builder::CreateInvokeResponses(const bool aReserveEndBuffer)
{
    if (mError == CHIP_NO_ERROR)
    {
        if (aReserveEndBuffer)
        {
            mError = mInvokeResponses.InitWithEndBufferReserved(mpWriter, to_underlying(Tag::kInvokeResponses));
        }
        else
        {
            mError = mInvokeResponses.Init(mpWriter, to_underlying(Tag::kInvokeResponses));
        }
    }
    return mInvokeResponses;
}

InvokeResponseMessage::Builder & InvokeResponseMessage::Builder::MoreChunkedMessages(const bool aMoreChunkedMessages)
{
    // If any changes are made to how we encoded MoreChunkedMessage that involves how many
    // bytes are needed, a corresponding change to GetSizeForMoreChunkResponses indicating
    // the new size that will be required.

    // skip if error has already been set
    SuccessOrExit(mError);

    if (mIsMoreChunkMessageBufferReserved)
    {
        mError = GetWriter()->UnreserveBuffer(GetSizeForMoreChunkResponses());
        SuccessOrExit(mError);
        mIsMoreChunkMessageBufferReserved = false;
    }

    mError = mpWriter->PutBoolean(TLV::ContextTag(Tag::kMoreChunkedMessages), aMoreChunkedMessages);
exit:
    return *this;
}

CHIP_ERROR InvokeResponseMessage::Builder::ReserveSpaceForMoreChunkedMessages()
{
    ReturnErrorOnFailure(GetWriter()->ReserveBuffer(GetSizeForMoreChunkResponses()));
    mIsMoreChunkMessageBufferReserved = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR InvokeResponseMessage::Builder::EndOfInvokeResponseMessage()
{
    // If any changes are made to how we end the invoke response message that involves how many
    // bytes are needed, a corresponding change to GetSizeToEndInvokeResponseMessage indicating
    // the new size that will be required.
    ReturnErrorOnFailure(mError);
    if (mIsEndBufferReserved)
    {
        ReturnErrorOnFailure(GetWriter()->UnreserveBuffer(GetSizeToEndInvokeResponseMessage()));
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

uint32_t InvokeResponseMessage::Builder::GetSizeForMoreChunkResponses()
{
    // MoreChunkedMessages() encodes a uint8_t with context tag 0x02. This means 1 control byte,
    // 1 byte for the tag. For booleans the value is encoded in control byte.
    uint32_t kEncodeMoreChunkedMessages = 1 + 1;

    return kEncodeMoreChunkedMessages;
}

uint32_t InvokeResponseMessage::Builder::GetSizeToEndInvokeResponseMessage()
{
    // EncodeInteractionModelRevision() encodes a uint8_t with context tag 0xFF. This means 1 control byte,
    // 1 byte for the tag, 1 byte for the value.
    uint32_t kEncodeInteractionModelSize = 1 + 1 + 1;
    uint32_t kEndOfContainerSize         = 1;

    return kEncodeInteractionModelSize + kEndOfContainerSize;
}
} // namespace app
} // namespace chip
