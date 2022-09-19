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

#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {

namespace TLV {

namespace Debug {

/**
 *  Dump the TLV element referenced by @a aReader in human-readable form using
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
static void DumpHandler(DumpWriter aWriter, const char * aIndent, const TLVReader & aReader, size_t aDepth)
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

    aWriter("0x%02X, ", temp.GetLengthRead());

    for (size_t i = 0; i < aDepth; i++)
        aWriter("%s", aIndent);

    if (IsProfileTag(tag))
    {
        aWriter("tag[%s]: 0x%x::0x%x::0x%x, ", DecodeTagControl(tagControl), VendorIdFromTag(tag), ProfileNumFromTag(tag),
                TagNumFromTag(tag));
    }
    else if (IsContextTag(tag))
    {
        aWriter("tag[%s]: 0x%x, ", DecodeTagControl(tagControl), TagNumFromTag(tag));
    }
    else if (IsSpecialTag(tag))
    {

        aWriter("tag[%s]: 0x%x, ", DecodeTagControl(tagControl), tag);
    }
    else
    {
        aWriter("tag[unknown]: 0x%x, ", tag);
    }

    aWriter("type: %s (0x%02x), ", DecodeType(type), type);

    if (TLVTypeIsContainer(type))
    {
        aWriter("container: ");
    }
    else
    {
        if (type == kTLVType_UTF8String || type == kTLVType_ByteString)
            aWriter("length: %" PRIu32 ", ", len);

        aWriter("value: ");

        switch (type)
        {

        case kTLVType_SignedInteger:
            int64_t sVal;
            err = temp.Get(sVal);
            VerifyOrExit(err == CHIP_NO_ERROR, aWriter("Error in kTLVType_SignedInteger"));
            aWriter("%" PRIi64, sVal);
            break;

        case kTLVType_UnsignedInteger:
            uint64_t uVal;
            err = temp.Get(uVal);
            VerifyOrExit(err == CHIP_NO_ERROR, aWriter("Error in kTLVType_UnsignedInteger"));
            aWriter("%" PRIu64, uVal);
            break;

        case kTLVType_Boolean:
            bool bVal;
            err = temp.Get(bVal);
            VerifyOrExit(err == CHIP_NO_ERROR, aWriter("Error in kTLVType_Boolean"));
            aWriter("%s", bVal ? "true" : "false");
            break;

        case kTLVType_FloatingPointNumber:
            double fpVal;
            err = temp.Get(fpVal);
            VerifyOrExit(err == CHIP_NO_ERROR, aWriter("Error in kTLVType_FloatingPointNumber"));
            aWriter("%lf", fpVal);
            break;

        case kTLVType_UTF8String:
            err = temp.GetDataPtr(strbuf);
            VerifyOrExit(err == CHIP_NO_ERROR, aWriter("Error in kTLVType_UTF8String"));
            aWriter("\"%-.*s\"", static_cast<int>(len), strbuf);
            break;

        case kTLVType_ByteString:
            err = temp.GetDataPtr(strbuf);
            VerifyOrExit(err == CHIP_NO_ERROR, aWriter("Error in kTLVType_ByteString"));
            aWriter("hex:");
            for (uint32_t i = 0; i < len; i++)
            {
                aWriter("%02X", strbuf[i]);
            }
            break;

        case kTLVType_Null:
            aWriter("NULL");
            break;

        case kTLVType_NotSpecified:
            aWriter("Not Specified");
            break;

        default:
            aWriter("Error: Type is not primitive.");
            break;
        }
    }

exit:
    aWriter("\n");
}

/**
 *  Decode a TLV tag control with a descriptive string.
 *
 *  @param[in]     aTagControl  The TLV tag control to decode and for which to return
 *                              a descriptive string.
 *
 *  @return  A pointer to a NULL-terminated string describing the specified
 *           tag control on success; otherwise, NULL.
 *
 */
const char * DecodeTagControl(const TLVTagControl aTagControl)
{
    const char * retval;

    switch (aTagControl)
    {

    case TLVTagControl::Anonymous:
        retval = "Anonymous";
        break;

    case TLVTagControl::ContextSpecific:
        retval = "Context Specific";
        break;

    case TLVTagControl::CommonProfile_2Bytes:
        retval = "Common Profile (2 Bytes)";
        break;

    case TLVTagControl::CommonProfile_4Bytes:
        retval = "Common Profile (4 Bytes)";
        break;

    case TLVTagControl::ImplicitProfile_2Bytes:
        retval = "Implicit Profile (2 Bytes)";
        break;

    case TLVTagControl::ImplicitProfile_4Bytes:
        retval = "Implicit Profile (4 Bytes)";
        break;

    case TLVTagControl::FullyQualified_6Bytes:
        retval = "Fully Qualified (6 Bytes)";
        break;

    case TLVTagControl::FullyQualified_8Bytes:
        retval = "Fully Qualified (8 Bytes)";
        break;

    default:
        retval = nullptr;
        break;
    }

    return retval;
}

/**
 *  Decode a TLV type with a descriptive string.
 *
 *  @param[in]     aType     The TLV type to decode and for which to return
 *                           a descriptive string.
 *
 *  @return  A pointer to a NULL-terminated string describing the specified
 *           type on success; otherwise, NULL.
 *
 */
const char * DecodeType(const TLVType aType)
{
    const char * retval;

    switch (aType)
    {

    case kTLVType_NotSpecified:
        retval = "Not Specified";
        break;

    case kTLVType_SignedInteger:
        retval = "Signed Fixed Point";
        break;

    case kTLVType_UnsignedInteger:
        retval = "Unsigned Fixed Point";
        break;

    case kTLVType_Boolean:
        retval = "Boolean";
        break;

    case kTLVType_FloatingPointNumber:
        retval = "Floating Point";
        break;

    case kTLVType_UTF8String:
        retval = "UTF-8 String";
        break;

    case kTLVType_ByteString:
        retval = "Octet String";
        break;

    case kTLVType_Null:
        retval = "Null";
        break;

    case kTLVType_Structure:
        retval = "Structure";
        break;

    case kTLVType_Array:
        retval = "Array";
        break;

    case kTLVType_List:
        retval = "List";
        break;

    default:
        retval = nullptr;
        break;
    }

    return retval;
}

/**
 *  Log the TLV data within the specified reader in human-readable form to
 *  the specified writer.
 *
 *  @param[in]     aWriter   The writer to log the TLV data.
 *  @param[in]     aReader   A read-only reference to the TLV reader containing
 *                           the TLV data to log.
 *
 *  @retval  #CHIP_NO_ERROR  Unconditionally.
 *
 */
CHIP_ERROR DumpIterator(DumpWriter aWriter, const TLVReader & aReader)
{
    const char * tabs  = "";
    const size_t depth = 0;
    CHIP_ERROR retval  = CHIP_NO_ERROR;

    DumpHandler(aWriter, tabs, aReader, depth);

    return retval;
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
CHIP_ERROR DumpHandler(const TLVReader & aReader, size_t aDepth, void * aContext)
{
    static const char indent[] = "    ";
    DumpContext * context;

    VerifyOrReturnError(aContext != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    context = static_cast<DumpContext *>(aContext);

    VerifyOrReturnError(context->mWriter != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    DumpHandler(context->mWriter, indent, aReader, aDepth);

    return CHIP_NO_ERROR;
}

/**
 *  Dump the TLV data within the specified reader in human-readable form with
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
CHIP_ERROR Dump(const TLVReader & aReader, DumpWriter aWriter)
{
    void * context          = nullptr;
    DumpContext dumpContext = { aWriter, context };
    CHIP_ERROR retval;

    retval = Utilities::Iterate(aReader, DumpHandler, &dumpContext);

    return retval;
}

} // namespace Debug

} // namespace TLV

} // namespace chip
