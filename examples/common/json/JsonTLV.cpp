/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
 *
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
 *      This file implements interfaces for debugging and logging
 *      CHIP TLV.
 *
 */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <ctype.h>
#include <inttypes.h>
#include <string.h>

#include <lib/core/TLV.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include "JsonTLV.h"

namespace chip {

namespace TLV {

namespace Json {

void ENFORCE_FORMAT(1, 2) TLVDumpWriter(const char * aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    vprintf(aFormat, args);

    va_end(args);
}

static void ENFORCE_FORMAT(2, 3) FormattedWrite(Encoding::BufferWriter & writer, const char * aFormat, ...)
{
    char out_buf[100];

    va_list args;

    va_start(args, aFormat);

    vsnprintf(out_buf, sizeof(out_buf), aFormat, args);

    va_end(args);

    writer.Put(out_buf);
}

/**
 *  Write the TLV element referenced by @a aReader in human-readable form using
 *  @a aWriter.
 *
 *  @param[in]     aWriter   The writer to log the TLV data.
 *  @param[in]     aIndent   The indentation for logging the current depth into
 *                           the TLV data.
 *  @param[in]     aReader   A read-only reference to the TLV reader containing
 *                           the TLV data to log.
 *  @param[in]     aDepth    The current depth into the TLV data.
 *
 */
static void WriteHandlerJSON(Encoding::BufferWriter & aWriter, const char * aIndent, const TLVReader & aReader, size_t aDepth)
{
    const TLVType type     = aReader.GetType();
    const Tag tag          = aReader.GetTag();
    const uint32_t len     = aReader.GetLength();
    const uint8_t * strbuf = nullptr;
    CHIP_ERROR err         = CHIP_NO_ERROR;
    TLVReader temp;
    TLVTagControl tagControl;

    temp.Init(aReader);
    tagControl = static_cast<TLVTagControl>(temp.GetControlByte() & kTLVTagControlMask);

    for (size_t i = 0; i < aDepth; i++)
        FormattedWrite(aWriter, "%s", aIndent);

    if (TLVTypeIsContainer(type))
    {
        if (tagControl == TLVTagControl::ContextSpecific)
        {
            FormattedWrite(aWriter, "\"0x%x\" : ", TagNumFromTag(tag));
        }

        // add any prefix
        switch (type)
        {
        case kTLVType_Structure:
            FormattedWrite(aWriter, "{\n");
            break;

        case kTLVType_Array:
            FormattedWrite(aWriter, "[\n");
            break;

        case kTLVType_List:
            FormattedWrite(aWriter, "[\n");
            break;

        default:
            break;
        }
    }
    else
    {
        if (tagControl == TLVTagControl::ContextSpecific)
        {
            FormattedWrite(aWriter, "\"0x%x\" : { \"tlvType\":\"0x%02x\", \"value\": ", TagNumFromTag(tag), type);
        }
        else
        {
            FormattedWrite(aWriter, "{ \"tlvType\":\"0x%02x\", \"value\": ", type);
        }

        switch (type)
        {

        case kTLVType_SignedInteger:
            int64_t sVal;
            err = temp.Get(sVal);
            VerifyOrExit(err == CHIP_NO_ERROR, FormattedWrite(aWriter, "Error in kTLVType_SignedInteger"));
            FormattedWrite(aWriter, "%" PRIi64, sVal);
            break;

        case kTLVType_UnsignedInteger:
            uint64_t uVal;
            err = temp.Get(uVal);
            VerifyOrExit(err == CHIP_NO_ERROR, FormattedWrite(aWriter, "Error in kTLVType_UnsignedInteger"));
            FormattedWrite(aWriter, "%" PRIu64, uVal);
            break;

        case kTLVType_Boolean:
            bool bVal;
            err = temp.Get(bVal);
            VerifyOrExit(err == CHIP_NO_ERROR, FormattedWrite(aWriter, "Error in kTLVType_Boolean"));
            FormattedWrite(aWriter, "%s", bVal ? "true" : "false");
            break;

        case kTLVType_FloatingPointNumber:
            double fpVal;
            err = temp.Get(fpVal);
            VerifyOrExit(err == CHIP_NO_ERROR, FormattedWrite(aWriter, "Error in kTLVType_FloatingPointNumber"));
            FormattedWrite(aWriter, "%lf", fpVal);
            break;

        case kTLVType_UTF8String:
            err = temp.GetDataPtr(strbuf);
            VerifyOrExit(err == CHIP_NO_ERROR, FormattedWrite(aWriter, "Error in kTLVType_UTF8String"));
            FormattedWrite(aWriter, "\"%-.*s\"", static_cast<int>(len), strbuf);
            break;

        case kTLVType_ByteString:
            err = temp.GetDataPtr(strbuf);
            VerifyOrExit(err == CHIP_NO_ERROR, FormattedWrite(aWriter, "Error in kTLVType_ByteString"));
            FormattedWrite(aWriter, "hex:");
            for (uint32_t i = 0; i < len; i++)
            {
                FormattedWrite(aWriter, "%02X", strbuf[i]);
            }
            break;

        case kTLVType_Null:
            FormattedWrite(aWriter, "null");
            break;

        case kTLVType_NotSpecified:
            FormattedWrite(aWriter, "ns");
            break;

        default:
            FormattedWrite(aWriter, "Error: Type is not primitive.");
            break;
        }

        FormattedWrite(aWriter, " }");
    }

exit:
    return;
}

static void WriteHandlerJSONAfter(Encoding::BufferWriter & aWriter, const char * aIndent, const TLVType type, size_t aDepth,
                                  bool endOfList)
{
    if (TLVTypeIsContainer(type))
    {
        for (size_t i = 0; i < aDepth; i++)
            FormattedWrite(aWriter, "%s", aIndent);

        // add any postfix
        switch (type)
        {
        case kTLVType_Structure:
            FormattedWrite(aWriter, "}");
            break;

        case kTLVType_Array:
            FormattedWrite(aWriter, "]");
            break;

        case kTLVType_List:
            FormattedWrite(aWriter, "]");
            break;

        default:
            break;
        }
    }

    if (!endOfList)
    {
        FormattedWrite(aWriter, ",");
    }

    FormattedWrite(aWriter, "\n");
}

/**
 *  Log the TLV data within the specified reader in human-readable form.
 *
 *  @param[in]     aReader   A read-only reference to the TLV reader containing
 *                           the TLV data to log.
 *  @param[in]     aDepth    The current depth into the TLV data.
 *  @param[in,out] aContext  A pointer to the handler-specific context.
 *
 *  @retval  #CHIP_NO_ERROR                On success.
 *
 *  @retval  #CHIP_ERROR_INVALID_ARGUMENT  If aContext is NULL or if
 *                                          aContext->mWriter is NULL.
 *
 */
CHIP_ERROR WriteHandlerJSON(const TLVReader & aReader, size_t aDepth, void * aContext)
{
    static const char indent[] = "    ";
    JsonContext * context;

    VerifyOrReturnError(aContext != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    context = static_cast<JsonContext *>(aContext);

    WriteHandlerJSON(context->mWriter, indent, aReader, aDepth);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WriteHandlerJSONAfter(const TLVType theType, size_t aDepth, bool endOfList, void * aContext)
{
    static const char indent[] = "    ";
    JsonContext * context;

    VerifyOrReturnError(aContext != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    context = static_cast<JsonContext *>(aContext);

    WriteHandlerJSONAfter(context->mWriter, indent, theType, aDepth, endOfList);

    return CHIP_NO_ERROR;
}
/**
 *  Write the TLV data within the specified reader in human-readable form with
 *  the specified writer.
 *
 *  @param[in]     aReader          A read-only reference to the TLV reader containing
 *                                  the TLV data to log.
 *
 *  @param[in]     aWriter          A dump writer to log the TLV data of the TLV reader.
 *
 *  @retval  #CHIP_NO_ERROR        On success.
 *
 */
CHIP_ERROR WriteJSON(const TLVReader & aReader, Encoding::BufferWriter & aWriter)
{
    void * context          = nullptr;
    JsonContext dumpContext = { aWriter, context };
    CHIP_ERROR retval;

    retval = Utilities::Iterate(aReader, WriteHandlerJSON, &dumpContext, true /* recurse */, WriteHandlerJSONAfter);

    aWriter.Put('\0');

    return retval;
}

} // namespace Json

} // namespace TLV

} // namespace chip
