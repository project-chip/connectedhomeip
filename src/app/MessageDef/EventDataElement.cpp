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
 *      This file defines EventDataElement parser and builder in CHIP interaction model
 *
 */

#include "EventDataElement.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
CHIP_ERROR EventDataElement::Parser::Init(const chip::TLV::TLVReader & aReader)
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
EventDataElement::Parser::ParseData(chip::TLV::TLVReader & aReader, int aDepth) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aDepth == 0)
    {
        PRETTY_PRINT("EventData = ");
    }
    else
    {
        if (chip::TLV::IsContextTag(aReader.GetTag()))
        {
            PRETTY_PRINT("0x%" PRIx32 " = ", chip::TLV::TagNumFromTag(aReader.GetTag()));
        }
        else if (chip::TLV::IsProfileTag(aReader.GetTag()))
        {
            PRETTY_PRINT("0x%" PRIx32 "::0x%" PRIx32 " = ", chip::TLV::ProfileIdFromTag(aReader.GetTag()),
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
        PRETTY_PRINT("{");
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
        PRETTY_PRINT("\t\t]");
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

        PRETTY_PRINT("%c,", terminating_char);

        err = aReader.ExitContainer(type);
        SuccessOrExit(err);
    }

exit:
    ChipLogFunctError(err);
    return err;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR EventDataElement::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("EventDataElement =");
    PRETTY_PRINT("{");

    // make a copy of the Path reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);
        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_EventPath:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_EventPath)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EventPath);

            VerifyOrExit(chip::TLV::kTLVType_List == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                EventPath::Parser path;
                err = path.Init(reader);
                SuccessOrExit(err);

                PRETTY_PRINT_INCDEPTH();
                err = path.CheckSchemaValidity();
                SuccessOrExit(err);
                PRETTY_PRINT_DECDEPTH();
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_PriorityLevel:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_PriorityLevel)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_PriorityLevel);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\tPriorityLevel = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_Number:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_Number)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_Number);

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\tNumber = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_UTCTimestamp:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_UTCTimestamp)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_UTCTimestamp);

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\tUTCTimestamp = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING
            break;

        case kCsTag_SystemTimestamp:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_SystemTimestamp)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_SystemTimestamp);

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\tSystemTimestamp = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_DeltaUTCTimestamp:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_DeltaUTCTimestamp)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_DeltaUTCTimestamp);

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\tDeltaUTCTimestampstamp= 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_DeltaSystemTimestamp:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_DeltaSystemTimestamp)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_DeltaSystemTimestamp);

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\tDeltaSystemTimestamp = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_Data:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_Data)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_Data);

            PRETTY_PRINT_INCDEPTH();
            err = ParseData(reader, 0);
            SuccessOrExit(err);
            PRETTY_PRINT_DECDEPTH();
            break;
        default:
            ExitNow(err = CHIP_ERROR_INVALID_TLV_TAG);
        }
    }
    PRETTY_PRINT("},");
    PRETTY_PRINT("");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const uint16_t RequiredFields = (1 << kCsTag_EventPath) | (1 << kCsTag_PriorityLevel) | (1 << kCsTag_Data);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_EVENT_DATA_ELEMENT;
        }
    }
    SuccessOrExit(err);
    err = reader.ExitContainer(mOuterContainerType);

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR EventDataElement::Parser::GetEventPath(EventPath::Parser * const apEventPath)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_EventPath), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_List == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apEventPath->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR EventDataElement::Parser::GetPriorityLevel(uint8_t * const apPriorityLevel)
{
    return GetUnsignedInteger(kCsTag_PriorityLevel, apPriorityLevel);
}

CHIP_ERROR EventDataElement::Parser::GetNumber(uint64_t * const apNumber)
{
    return GetUnsignedInteger(kCsTag_Number, apNumber);
}

CHIP_ERROR EventDataElement::Parser::GetUTCTimestamp(uint64_t * const apUTCTimestamp)
{
    return GetUnsignedInteger(kCsTag_UTCTimestamp, apUTCTimestamp);
}

CHIP_ERROR EventDataElement::Parser::GetSystemTimestamp(uint64_t * const apSystemTimestamp)
{
    return GetUnsignedInteger(kCsTag_SystemTimestamp, apSystemTimestamp);
}

CHIP_ERROR EventDataElement::Parser::GetDeltaUTCTimestamp(uint64_t * const apDeltaUTCTimestampstamp)
{
    return GetUnsignedInteger(kCsTag_DeltaUTCTimestamp, apDeltaUTCTimestampstamp);
}

CHIP_ERROR EventDataElement::Parser::GetDeltaSystemTimestamp(uint64_t * const apDeltaSystemTimestamp)
{
    return GetUnsignedInteger(kCsTag_DeltaSystemTimestamp, apDeltaSystemTimestamp);
}

CHIP_ERROR EventDataElement::Parser::GetData(chip::TLV::TLVReader * const apReader) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_Data), *apReader);
    ChipLogFunctError(err);

    return err;
}

CHIP_ERROR EventDataElement::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

EventPath::Builder & EventDataElement::Builder::CreateEventPathBuilder()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mEventPathBuilder.Init(mpWriter, kCsTag_EventPath);
        ChipLogFunctError(mError);
    }
    else
    {
        mEventPathBuilder.ResetError(mError);
    }
    return mEventPathBuilder;
}

EventDataElement::Builder EventDataElement::Builder::PriorityLevel(const uint8_t aPriorityLevel)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_PriorityLevel), aPriorityLevel);
        ChipLogFunctError(mError);
    }
    return *this;
}

EventDataElement::Builder EventDataElement::Builder::Number(const uint64_t aNumber)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_Number), aNumber);
        ChipLogFunctError(mError);
    }
    return *this;
}

EventDataElement::Builder EventDataElement::Builder::UTCTimestamp(const uint64_t aUTCTimestamp)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_UTCTimestamp), aUTCTimestamp);
        ChipLogFunctError(mError);
    }
    return *this;
}

EventDataElement::Builder EventDataElement::Builder::SystemTimestamp(const uint64_t aSystemTimestamp)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_SystemTimestamp), aSystemTimestamp);
        ChipLogFunctError(mError);
    }
    return *this;
}

EventDataElement::Builder EventDataElement::Builder::DeltaUTCTimestamp(const uint64_t aDeltaUTCTimestamp)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_DeltaUTCTimestamp), aDeltaUTCTimestamp);
        ChipLogFunctError(mError);
    }
    return *this;
}

EventDataElement::Builder EventDataElement::Builder::DeltaSystemTimestamp(const uint64_t aDeltaSystemTimestamp)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_DeltaSystemTimestamp), aDeltaSystemTimestamp);
        ChipLogFunctError(mError);
    }
    return *this;
}

// Mark the end of this element and recover the type for outer container
EventDataElement::Builder & EventDataElement::Builder::EndOfEventDataElement()
{
    EndOfContainer();
    return *this;
}
}; // namespace app
}; // namespace chip
