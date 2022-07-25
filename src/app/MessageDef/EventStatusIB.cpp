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

#include "EventStatusIB.h"
#include "MessageDefHelper.h"
#include "StructBuilder.h"
#include "StructParser.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR EventStatusIB::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int tagPresenceMask = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("EventStatusIB =");
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
        case to_underlying(Tag::kPath):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kPath))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kPath));
            {
                EventPathIB::Parser path;
                ReturnErrorOnFailure(path.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(path.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case to_underlying(Tag::kErrorStatus):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kErrorStatus))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kErrorStatus));
            {
                StatusIB::Parser errorStatus;
                ReturnErrorOnFailure(errorStatus.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(errorStatus.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
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
        const int requiredFields = (1 << to_underlying(Tag::kPath)) | (1 << to_underlying(Tag::kErrorStatus));
        err = (tagPresenceMask & requiredFields) == requiredFields ? CHIP_NO_ERROR : CHIP_ERROR_IM_MALFORMED_EVENT_STATUS_IB;
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR EventStatusIB::Parser::GetPath(EventPathIB::Parser * const apPath) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kPath)), reader));
    return apPath->Init(reader);
}

CHIP_ERROR EventStatusIB::Parser::GetErrorStatus(StatusIB::Parser * const apErrorStatus) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kErrorStatus)), reader));
    return apErrorStatus->Init(reader);
}

EventPathIB::Builder & EventStatusIB::Builder::CreatePath()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mPath.Init(mpWriter, to_underlying(Tag::kPath));
    }
    return mPath;
}

StatusIB::Builder & EventStatusIB::Builder::CreateErrorStatus()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mErrorStatus.Init(mpWriter, to_underlying(Tag::kErrorStatus));
    }
    return mErrorStatus;
}

EventStatusIB::Builder & EventStatusIB::Builder::EndOfEventStatusIB()
{
    EndOfContainer();
    return *this;
}
} // namespace app
} // namespace chip
