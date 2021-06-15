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
 *      This file defines AttributeDataElement parser and builder in CHIP interaction model
 *
 */

#include "AttributeDataElement.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
CHIP_ERROR AttributeDataElement::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the reader here
    mReader.Init(aReader);

    VerifyOrExit(chip::TLV::kTLVType_Structure == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = mReader.EnterContainer(mOuterContainerType);

exit:
    ChipLogFunctError(err);

    return err;
}

CHIP_ERROR
AttributeDataElement::Parser::ParseData(chip::TLV::TLVReader & aReader, int aDepth) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aDepth == 0)
    {
        PRETTY_PRINT("\tData = ");
    }
    else
    {
        if (chip::TLV::IsContextTag(aReader.GetTag()))
        {
            PRETTY_PRINT("\t0x%" PRIx32 " = ", chip::TLV::TagNumFromTag(aReader.GetTag()));
        }
        else if (chip::TLV::IsProfileTag(aReader.GetTag()))
        {
            PRETTY_PRINT("\t0x%" PRIx32 "::0x%" PRIx32 " = ", chip::TLV::ProfileIdFromTag(aReader.GetTag()),
                         chip::TLV::TagNumFromTag(aReader.GetTag()));
        }
        else
        {
            // Anonymous tag, don't print anything
        }
    }

    switch (aReader.GetType())
    {
    case chip::TLV::kTLVType_Structure:
        PRETTY_PRINT("\t{");
        break;

    case chip::TLV::kTLVType_Array:
        PRETTY_PRINT_SAMELINE("[");
        PRETTY_PRINT("\t\t");
        break;

    case chip::TLV::kTLVType_SignedInteger: {
        int64_t value_s64;

        err = aReader.Get(value_s64);
        SuccessOrExit(err);

        PRETTY_PRINT_SAMELINE("%" PRId64 ", ", value_s64);
        break;
    }

    case chip::TLV::kTLVType_UnsignedInteger: {
        uint64_t value_u64;

        err = aReader.Get(value_u64);
        SuccessOrExit(err);

        PRETTY_PRINT_SAMELINE("%" PRIu64 ", ", value_u64);
        break;
    }

    case chip::TLV::kTLVType_Boolean: {
        bool value_b;

        err = aReader.Get(value_b);
        SuccessOrExit(err);

        PRETTY_PRINT_SAMELINE("%s, ", value_b ? "true" : "false");
        break;
    }

    case chip::TLV::kTLVType_UTF8String: {
        char value_s[256];

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

    case chip::TLV::kTLVType_ByteString: {
        uint8_t value_b[256];
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
                PRETTY_PRINT_SAMELINE("0x%" PRIx8 ", ", value_b[i]);
            }
        }

        err = CHIP_NO_ERROR;
        PRETTY_PRINT("\t]");
        break;
    }

    case chip::TLV::kTLVType_Null:
        PRETTY_PRINT_SAMELINE("NULL");
        break;

    default:
        PRETTY_PRINT_SAMELINE("--");
        break;
    }

    if (aReader.GetType() == chip::TLV::kTLVType_Structure || aReader.GetType() == chip::TLV::kTLVType_Array)
    {
        const char terminating_char = (aReader.GetType() == chip::TLV::kTLVType_Structure) ? '}' : ']';
        chip::TLV::TLVType type;

        IgnoreUnusedVariable(terminating_char);

        err = aReader.EnterContainer(type);
        SuccessOrExit(err);

        while ((err = aReader.Next()) == CHIP_NO_ERROR)
        {
            PRETTY_PRINT_INCDEPTH();

            err = ParseData(aReader, aDepth + 1);
            SuccessOrExit(err);

            PRETTY_PRINT_DECDEPTH();
        }

        PRETTY_PRINT("\t%c,", terminating_char);

        err = aReader.ExitContainer(type);
        SuccessOrExit(err);
    }

exit:
    ChipLogFunctError(err);
    return err;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributeDataElement::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;
    uint32_t tagNum = 0;

    PRETTY_PRINT("AttributeDataElement =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);

        tagNum = chip::TLV::TagNumFromTag(reader.GetTag());

        switch (tagNum)
        {
        case kCsTag_AttributePath:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_AttributePath)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributePath);
            VerifyOrExit(chip::TLV::kTLVType_List == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            {
                AttributePath::Parser path;
                err = path.Init(reader);
                SuccessOrExit(err);

                PRETTY_PRINT_INCDEPTH();
                err = path.CheckSchemaValidity();
                SuccessOrExit(err);
                PRETTY_PRINT_DECDEPTH();
            }

            break;
        case kCsTag_DataVersion:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_DataVersion)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_DataVersion);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                chip::DataVersion version;
                err = reader.Get(version);
                SuccessOrExit(err);

                PRETTY_PRINT("\tDataElementVersion = 0x%" PRIx64 ",", version);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_Data:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_Data)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_Data);

            err = ParseData(reader, 0);
            SuccessOrExit(err);
            break;
        case kCsTag_MoreClusterDataFlag:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_MoreClusterDataFlag)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_MoreClusterDataFlag);
            VerifyOrExit(chip::TLV::kTLVType_Boolean == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                bool flag;
                err = reader.Get(flag);
                SuccessOrExit(err);

                PRETTY_PRINT("\tMoreClusterDataFlag = %s,", flag ? "true" : "false");
            }

#endif // CHIP_DETAIL_LOGGING
            break;

        default:
            PRETTY_PRINT("\tUnknown tag num %" PRIu32, tagNum);
            break;
        }
    }

    PRETTY_PRINT("},");
    PRETTY_PRINT("");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        // Either the data or deleted keys should be present.
        const uint16_t RequiredFields = (1 << kCsTag_AttributePath) | (1 << kCsTag_DataVersion) | (1 << kCsTag_Data);
        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_DATA_ELEMENT;
        }
    }
    SuccessOrExit(err);
    err = reader.ExitContainer(mOuterContainerType);

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR AttributeDataElement::Parser::GetAttributePath(AttributePath::Parser * const apAttributePath) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_AttributePath), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_List == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apAttributePath->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR AttributeDataElement::Parser::GetDataVersion(chip::DataVersion * const apVersion) const
{
    return GetUnsignedInteger(kCsTag_DataVersion, apVersion);
}

CHIP_ERROR AttributeDataElement::Parser::GetData(chip::TLV::TLVReader * const apReader) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_Data), *apReader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR AttributeDataElement::Parser::GetMoreClusterDataFlag(bool * const apGetMoreClusterDataFlag) const
{
    return GetSimpleValue(kCsTag_MoreClusterDataFlag, chip::TLV::kTLVType_Boolean, apGetMoreClusterDataFlag);
}

CHIP_ERROR AttributeDataElement::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

AttributePath::Builder & AttributeDataElement::Builder::CreateAttributePathBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributePathBuilder.ResetError(mError));

    mError = mAttributePathBuilder.Init(mpWriter, kCsTag_AttributePath);
    ChipLogFunctError(mError);

exit:
    // on error, mAttributePathBuilder would be un-/partial initialized and cannot be used to write anything
    return mAttributePathBuilder;
}

AttributeDataElement::Builder & AttributeDataElement::Builder::DataVersion(const chip::DataVersion aDataVersion)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_DataVersion), aDataVersion);
    ChipLogFunctError(mError);

exit:
    return *this;
}

AttributeDataElement::Builder & AttributeDataElement::Builder::MoreClusterData(const bool aMoreClusterData)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    if (aMoreClusterData)
    {
        mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_MoreClusterDataFlag), true);
        ChipLogFunctError(mError);
    }

exit:
    return *this;
}

AttributeDataElement::Builder & AttributeDataElement::Builder::EndOfAttributeDataElement()
{
    EndOfContainer();
    return *this;
}
}; // namespace app
}; // namespace chip
