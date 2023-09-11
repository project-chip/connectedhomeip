/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
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

InvokeResponseMessage::Builder & InvokeResponseMessage::Builder::SuppressResponse(const bool aSuppressResponse)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(Tag::kSuppressResponse), aSuppressResponse);
    }
    return *this;
}

InvokeResponseIBs::Builder & InvokeResponseMessage::Builder::CreateInvokeResponses()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mInvokeResponses.Init(mpWriter, to_underlying(Tag::kInvokeResponses));
    }
    return mInvokeResponses;
}

CHIP_ERROR InvokeResponseMessage::Builder::EndOfInvokeResponseMessage()
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
