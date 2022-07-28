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
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR InvokeResponseMessage::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int tagPresenceMask = 0;
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
        case to_underlying(Tag::kInvokeResponses):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kInvokeResponses))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kInvokeResponses));
            {
                InvokeResponseIBs::Parser invokeResponses;
                ReturnErrorOnFailure(invokeResponses.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(invokeResponses.CheckSchemaValidity());
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
        const int requiredFields = (1 << to_underlying(Tag::kSuppressResponse)) | (1 << to_underlying(Tag::kInvokeResponses));
        err =
            (tagPresenceMask & requiredFields) == requiredFields ? CHIP_NO_ERROR : CHIP_ERROR_IM_MALFORMED_INVOKE_RESPONSE_MESSAGE;
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR InvokeResponseMessage::Parser::GetSuppressResponse(bool * const apSuppressResponse) const
{
    return GetSimpleValue(to_underlying(Tag::kSuppressResponse), TLV::kTLVType_Boolean, apSuppressResponse);
}

CHIP_ERROR InvokeResponseMessage::Parser::GetInvokeResponses(InvokeResponseIBs::Parser * const apStatus) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kInvokeResponses)), reader));
    return apStatus->Init(reader);
}

InvokeResponseMessage::Builder & InvokeResponseMessage::Builder::SuppressResponse(const bool aSuppressResponse)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(to_underlying(Tag::kSuppressResponse)), aSuppressResponse);
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

InvokeResponseMessage::Builder & InvokeResponseMessage::Builder::EndOfInvokeResponseMessage()
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
