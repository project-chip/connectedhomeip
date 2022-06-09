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

#include "CommandDataIB.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>

namespace chip {
namespace app {
CHIP_ERROR
CommandDataIB::Parser::ParseFields(TLV::TLVReader & aReader, int aDepth) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aDepth == 0)
    {
        PRETTY_PRINT("\tCommandFields = ");
    }
    else
    {
        if (TLV::IsContextTag(aReader.GetTag()))
        {
            PRETTY_PRINT("\t0x%" PRIx32 " = ", TLV::TagNumFromTag(aReader.GetTag()));
        }
        else if (TLV::IsProfileTag(aReader.GetTag()))
        {
            PRETTY_PRINT("\t0x%" PRIx32 "::0x%" PRIx32 " = ", TLV::ProfileIdFromTag(aReader.GetTag()),
                         TLV::TagNumFromTag(aReader.GetTag()));
        }
        else
        {
            // Anonymous tag, don't print anything
        }
    }

    switch (aReader.GetType())
    {
    case TLV::kTLVType_Structure:
        PRETTY_PRINT("\t{");
        break;

    case TLV::kTLVType_Array:
        PRETTY_PRINT_SAMELINE("[");
        PRETTY_PRINT("\t\t");
        break;

    case TLV::kTLVType_SignedInteger: {
        int64_t value_s64;

        err = aReader.Get(value_s64);
        SuccessOrExit(err);

        PRETTY_PRINT_SAMELINE("%" PRId64 ", ", value_s64);
        break;
    }

    case TLV::kTLVType_UnsignedInteger: {
        uint64_t value_u64;

        err = aReader.Get(value_u64);
        SuccessOrExit(err);

        PRETTY_PRINT_SAMELINE("%" PRIu64 ", ", value_u64);
        break;
    }

    case TLV::kTLVType_FloatingPointNumber: {
        double value_fp;

        err = aReader.Get(value_fp);
        SuccessOrExit(err);

        PRETTY_PRINT_SAMELINE("%lf, ", value_fp);
        break;
    }
    case TLV::kTLVType_Boolean: {
        bool value_b;

        err = aReader.Get(value_b);
        SuccessOrExit(err);

        PRETTY_PRINT_SAMELINE("%s, ", value_b ? "true" : "false");
        break;
    }

    case TLV::kTLVType_UTF8String: {
        char value_s[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];

        err = aReader.GetString(value_s, sizeof(value_s));
        VerifyOrExit(err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL, );

        if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            PRETTY_PRINT_SAMELINE("... (byte string too long) ...");
            err = CHIP_NO_ERROR;
        }
        else
        {
            PRETTY_PRINT_SAMELINE("\"%s\", ", value_s);
        }
        break;
    }

    case TLV::kTLVType_ByteString: {
        uint8_t value_b[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
        uint32_t len, readerLen;

        readerLen = aReader.GetLength();

        err = aReader.GetBytes(value_b, sizeof(value_b));
        VerifyOrExit(err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL, );

        PRETTY_PRINT_SAMELINE("[");
        PRETTY_PRINT("\t\t");

        if (readerLen < sizeof(value_b))
        {
            len = readerLen;
        }
        else
        {
            len = sizeof(value_b);
        }

        if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            PRETTY_PRINT_SAMELINE("... (byte string too long) ...");
        }
        else
        {
            for (size_t i = 0; i < len; i++)
            {
                PRETTY_PRINT_SAMELINE("0x%x, ", value_b[i]);
            }
        }

        err = CHIP_NO_ERROR;
        PRETTY_PRINT("]");
        break;
    }

    case TLV::kTLVType_Null:
        PRETTY_PRINT_SAMELINE("NULL");
        break;

    default:
        PRETTY_PRINT_SAMELINE("--");
        break;
    }

    if (aReader.GetType() == TLV::kTLVType_Structure || aReader.GetType() == TLV::kTLVType_Array)
    {
        const char terminating_char = (aReader.GetType() == TLV::kTLVType_Structure) ? '}' : ']';
        TLV::TLVType type;

        IgnoreUnusedVariable(terminating_char);

        err = aReader.EnterContainer(type);
        SuccessOrExit(err);

        while ((err = aReader.Next()) == CHIP_NO_ERROR)
        {
            PRETTY_PRINT_INCDEPTH();

            err = ParseFields(aReader, aDepth + 1);
            SuccessOrExit(err);

            PRETTY_PRINT_DECDEPTH();
        }

        PRETTY_PRINT("\t%c,", terminating_char);

        err = aReader.ExitContainer(type);
        SuccessOrExit(err);
    }

exit:
    return err;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR CommandDataIB::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int tagPresenceMask = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("CommandDataIB =");
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
                ReturnErrorOnFailure(path.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }

            break;
        case to_underlying(Tag::kFields):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kFields))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kFields));
            ReturnErrorOnFailure(ParseFields(reader, 0));
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
        const int requiredFields = 1 << to_underlying(Tag::kPath);
        err = (tagPresenceMask & requiredFields) == requiredFields ? CHIP_NO_ERROR : CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB;
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR CommandDataIB::Parser::GetPath(CommandPathIB::Parser * const apPath) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kPath)), reader));
    return apPath->Init(reader);
}

CHIP_ERROR CommandDataIB::Parser::GetFields(TLV::TLVReader * const apReader) const
{
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kFields)), *apReader));
    return CHIP_NO_ERROR;
}

CommandPathIB::Builder & CommandDataIB::Builder::CreatePath()
{
    mError = mPath.Init(mpWriter, to_underlying(Tag::kPath));
    return mPath;
}

CommandDataIB::Builder & CommandDataIB::Builder::EndOfCommandDataIB()
{
    EndOfContainer();
    return *this;
}
} // namespace app
} // namespace chip
