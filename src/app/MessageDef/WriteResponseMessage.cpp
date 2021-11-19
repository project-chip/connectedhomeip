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

#include "WriteResponseMessage.h"
#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR WriteResponseMessage::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int TagPresenceMask = 0;
    TLV::TLVReader reader;
    AttributeStatuses::Parser writeResponses;
    PRETTY_PRINT("WriteResponseMessage =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);
        uint32_t tagNum = TLV::TagNumFromTag(reader.GetTag());
        switch (tagNum)
        {
        case to_underlying(Tag::kWriteResponses):
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kWriteResponses))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kWriteResponses));
            VerifyOrReturnError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
            ReturnErrorOnFailure(writeResponses.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(writeResponses.CheckSchemaValidity());
            PRETTY_PRINT_DECDEPTH();
            break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }

    PRETTY_PRINT("}");
    PRETTY_PRINT("");

    if (CHIP_END_OF_TLV == err)
    {
        const int RequiredFields = (1 << to_underlying(Tag::kWriteResponses));

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_WRITE_RESPONSE_MESSAGE;
        }
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR WriteResponseMessage::Parser::GetWriteResponses(AttributeStatuses::Parser * const apWriteResponses) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kWriteResponses)), reader));
    return apWriteResponses->Init(reader);
}

AttributeStatuses::Builder & WriteResponseMessage::Builder::CreateWriteResponses()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mWriteResponses.Init(mpWriter, to_underlying(Tag::kWriteResponses));
    }
    return mWriteResponses;
}

AttributeStatuses::Builder & WriteResponseMessage::Builder::GetWriteResponses()
{
    return mWriteResponses;
}

WriteResponseMessage::Builder & WriteResponseMessage::Builder::EndOfWriteResponseMessage()
{
    EndOfContainer();
    return *this;
}
} // namespace app
} // namespace chip
