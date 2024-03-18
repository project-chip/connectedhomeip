/**
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file defines message helper functions in CHIP interaction model
 *
 */

#include "MessageDefHelper.h"
#include <algorithm>
#include <app/AppConfig.h>
#include <app/SpecificationDefinedRevisions.h>
#include <app/util/basic-types.h>
#include <inttypes.h>
#include <lib/support/logging/CHIPLogging.h>
#include <stdarg.h>
#include <stdio.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT && CHIP_DETAIL_LOGGING
// this is used to run in signle thread for IM message debug purpose
namespace {
uint32_t gPrettyPrintingDepthLevel = 0;
char gLineBuffer[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
size_t gCurLineBufferSize = 0;
} // namespace

void PrettyPrintIMBlankLine()
{
    if (gCurLineBufferSize)
    {
        // Don't need to explicitly NULL-terminate the string because
        // snprintf takes care of that.
        ChipLogDetail(DataManagement, "%s", gLineBuffer);
        gCurLineBufferSize = 0;
    }

    for (uint32_t i = 0; i < gPrettyPrintingDepthLevel; i++)
    {
        if (sizeof(gLineBuffer) > gCurLineBufferSize)
        {
            size_t sizeLeft = sizeof(gLineBuffer) - gCurLineBufferSize;
            size_t ret      = (size_t) (snprintf(gLineBuffer + gCurLineBufferSize, sizeLeft, "\t"));
            if (ret > 0)
            {
                gCurLineBufferSize += std::min(ret, sizeLeft);
            }
        }
    }
}

void PrettyPrintIM(bool aIsNewLine, const char * aFmt, ...)
{
    va_list args;
    va_start(args, aFmt);

    if (aIsNewLine)
    {
        PrettyPrintIMBlankLine();
    }

    if (sizeof(gLineBuffer) > gCurLineBufferSize)
    {
        size_t sizeLeft = sizeof(gLineBuffer) - gCurLineBufferSize;
        size_t ret      = (size_t) (vsnprintf(gLineBuffer + gCurLineBufferSize, sizeLeft, aFmt, args));
        if (ret > 0)
        {
            gCurLineBufferSize += std::min(ret, sizeLeft);
        }
    }

    va_end(args);
}
void IncreaseDepth()
{
    gPrettyPrintingDepthLevel++;
}

void DecreaseDepth()
{
    gPrettyPrintingDepthLevel--;
}
#endif

#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR CheckIMPayload(TLV::TLVReader & aReader, int aDepth, const char * aLabel)
{
    if (aDepth == 0)
    {
        PRETTY_PRINT("%s = ", aLabel);
    }
    else
    {
        if (TLV::IsContextTag(aReader.GetTag()))
        {
            PRETTY_PRINT("0x%" PRIx32 " = ", TLV::TagNumFromTag(aReader.GetTag()));
        }
        else if (TLV::IsProfileTag(aReader.GetTag()))
        {
            PRETTY_PRINT("0x%" PRIx32 "::0x%" PRIx32 " = ", TLV::ProfileIdFromTag(aReader.GetTag()),
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
        PRETTY_PRINT("{");
        break;

    case TLV::kTLVType_Array:
        PRETTY_PRINT_SAMELINE("[");
        PRETTY_PRINT("\t\t");
        break;

    case TLV::kTLVType_SignedInteger: {
        int64_t value_s64;

        ReturnErrorOnFailure(aReader.Get(value_s64));

        // TODO: Figure out how to not use PRId64 here, since it's not supported
        // on all libcs.
        PRETTY_PRINT_SAMELINE("%" PRId64 " (signed), ", value_s64);
        break;
    }

    case TLV::kTLVType_UnsignedInteger: {
        uint64_t value_u64;

        ReturnErrorOnFailure(aReader.Get(value_u64));

        // TODO: Figure out how to not use PRIu64 here, since it's not supported
        // on all libcs.
        PRETTY_PRINT_SAMELINE("%" PRIu64 " (unsigned), ", value_u64);
        break;
    }

    case TLV::kTLVType_FloatingPointNumber: {
        double value_fp;

        ReturnErrorOnFailure(aReader.Get(value_fp));

        PRETTY_PRINT_SAMELINE("%f, ", value_fp);
        break;
    }
    case TLV::kTLVType_Boolean: {
        bool value_b;

        ReturnErrorOnFailure(aReader.Get(value_b));

        PRETTY_PRINT_SAMELINE("%s, ", value_b ? "true" : "false");
        break;
    }

    case TLV::kTLVType_UTF8String: {
        char value_s[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];

#if CHIP_DETAIL_LOGGING
        uint32_t readerLen = aReader.GetLength();
#endif // CHIP_DETAIL_LOGGING
        CHIP_ERROR err = aReader.GetString(value_s, sizeof(value_s));
        VerifyOrReturnError(err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL, err);

        if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            PRETTY_PRINT_SAMELINE("... (char string too long: %" PRIu32 " chars) ...", readerLen);
        }
        else
        {
            PRETTY_PRINT_SAMELINE("\"%s\" (%" PRIu32 " chars), ", value_s, readerLen);
        }
        break;
    }

    case TLV::kTLVType_ByteString: {
        uint8_t value_b[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
        uint32_t len, readerLen;

        readerLen = aReader.GetLength();

        CHIP_ERROR err = aReader.GetBytes(value_b, sizeof(value_b));
        VerifyOrReturnError(err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL, err);

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
                PRETTY_PRINT_SAMELINE("0x%02x, ", value_b[i]);
            }
        }

        PRETTY_PRINT("] (%" PRIu32 " bytes)", readerLen);
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

        ReturnErrorOnFailure(aReader.EnterContainer(type));

        CHIP_ERROR err;
        while ((err = aReader.Next()) == CHIP_NO_ERROR)
        {
            PRETTY_PRINT_INCDEPTH();

            ReturnErrorOnFailure(CheckIMPayload(aReader, aDepth + 1, aLabel));

            PRETTY_PRINT_DECDEPTH();
        }

        PRETTY_PRINT("%c,", terminating_char);

        ReturnErrorOnFailure(aReader.ExitContainer(type));
    }

    return CHIP_NO_ERROR;
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

}; // namespace app
}; // namespace chip
