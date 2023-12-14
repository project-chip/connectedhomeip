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

#include "CommandStatusIB.h"
#include "MessageDefHelper.h"
#include "StructBuilder.h"
#include "StructParser.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR CommandStatusIB::Parser::PrettyPrint() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int tagPresenceMask = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("CommandStatusIB =");
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
                CommandPathIB::Parser path;
                ReturnErrorOnFailure(path.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(path.PrettyPrint());
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
                ReturnErrorOnFailure(errorStatus.PrettyPrint());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case to_underlying(Tag::kRef):
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
            {
                uint16_t reference;
                ReturnErrorOnFailure(reader.Get(reference));
                PRETTY_PRINT("\tRef = 0x%x,", reference);
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
        err = CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

CHIP_ERROR CommandStatusIB::Parser::GetPath(CommandPathIB::Parser * const apPath) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kPath), reader));
    return apPath->Init(reader);
}

CHIP_ERROR CommandStatusIB::Parser::GetErrorStatus(StatusIB::Parser * const apErrorStatus) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kErrorStatus), reader));
    return apErrorStatus->Init(reader);
}

CHIP_ERROR CommandStatusIB::Parser::GetRef(uint16_t * const apRef) const
{
    return GetUnsignedInteger(to_underlying(Tag::kRef), apRef);
}

CommandPathIB::Builder & CommandStatusIB::Builder::CreatePath()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mPath.Init(mpWriter, to_underlying(Tag::kPath));
    }
    return mPath;
}

StatusIB::Builder & CommandStatusIB::Builder::CreateErrorStatus()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mErrorStatus.Init(mpWriter, to_underlying(Tag::kErrorStatus));
    }
    return mErrorStatus;
}

CHIP_ERROR CommandStatusIB::Builder::Ref(const uint16_t aRef)
{
    return mpWriter->Put(TLV::ContextTag(Tag::kRef), aRef);
}

CHIP_ERROR CommandStatusIB::Builder::EndOfCommandStatusIB()
{
    EndOfContainer();
    return GetError();
}
} // namespace app
} // namespace chip
