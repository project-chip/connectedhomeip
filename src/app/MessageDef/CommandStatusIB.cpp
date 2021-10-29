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
 *      This file defines CommandStatusIB parser and builder in CHIP interaction model
 *
 */

#include "CommandStatusIB.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
CHIP_ERROR CommandStatusIB::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    mReader.Init(aReader);
    VerifyOrReturnError(chip::TLV::kTLVType_Structure == mReader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    ReturnErrorOnFailure(mReader.EnterContainer(mOuterContainerType));
    return CHIP_NO_ERROR;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR CommandStatusIB::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int TagPresenceMask = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("CommandStatusIB =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::IsContextTag(reader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
        uint32_t tagNum = TLV::TagNumFromTag(reader.GetTag());
        switch (tagNum)
        {
        case to_underlying(Tag::kPath):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kPath))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kPath));
            {
                CommandPathIB::Parser path;
                ReturnErrorOnFailure(path.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(path.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case to_underlying(Tag::kErrorStatus):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kErrorStatus))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kErrorStatus));
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
    PRETTY_PRINT("");

    if (CHIP_END_OF_TLV == err)
    {
        const int RequiredFields = (1 << to_underlying(Tag::kPath)) | (1 << to_underlying(Tag::kErrorStatus));

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_COMMAND_STATUS_IB;
        }
    }

    ReturnErrorOnFailure(err);
    ReturnErrorOnFailure(reader.ExitContainer(mOuterContainerType));
    return CHIP_NO_ERROR;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR CommandStatusIB::Parser::GetPath(CommandPathIB::Parser * const apPath) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kPath)), reader));
    ReturnErrorOnFailure(apPath->Init(reader));
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandStatusIB::Parser::GetErrorStatus(StatusIB::Parser * const apErrorStatus) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kErrorStatus)), reader));
    ReturnErrorOnFailure(apErrorStatus->Init(reader));
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandStatusIB::Builder::_Init(TLV::TLVWriter * const apWriter, const TLV::Tag aTag)
{
    mpWriter = apWriter;
    mError   = mpWriter->StartContainer(aTag, TLV::kTLVType_Structure, mOuterContainerType);
    ReturnErrorOnFailure(mError);
    return mError;
}

CHIP_ERROR CommandStatusIB::Builder::Init(TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse)
{
    return _Init(apWriter, TLV::ContextTag(aContextTagToUse));
}

CHIP_ERROR CommandStatusIB::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return _Init(apWriter, TLV::AnonymousTag);
}

CommandPathIB::Builder & CommandStatusIB::Builder::CreatePath()
{
    mError = mPath.Init(mpWriter, to_underlying(Tag::kPath));
    return mPath;
}

StatusIB::Builder & CommandStatusIB::Builder::CreateErrorStatus()
{
    mError = mErrorStatus.Init(mpWriter, to_underlying(Tag::kErrorStatus));
    return mErrorStatus;
}

CommandStatusIB::Builder & CommandStatusIB::Builder::EndOfCommandStatusIB()
{
    EndOfContainer();
    return *this;
}
}; // namespace app
}; // namespace chip
