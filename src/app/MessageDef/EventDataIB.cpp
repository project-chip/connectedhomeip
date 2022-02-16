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
 *      This file defines EventDataIB parser and builder in CHIP interaction model
 *
 */

#include "EventDataIB.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>

namespace chip {
namespace app {
CHIP_ERROR
EventDataIB::Parser::ParseData(TLV::TLVReader & aReader, int aDepth) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aDepth == 0)
    {
        PRETTY_PRINT("EventData = ");
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
        PRETTY_PRINT_SAMELINE("%" PRId64 ", ", value_s64);
        break;
    }

    case TLV::kTLVType_UnsignedInteger: {
        uint64_t value_u64;
        ReturnErrorOnFailure(aReader.Get(value_u64));
        PRETTY_PRINT_SAMELINE("%" PRIu64 ", ", value_u64);
        break;
    }

    case TLV::kTLVType_Boolean: {
        bool value_b;
        ReturnErrorOnFailure(aReader.Get(value_b));
        PRETTY_PRINT_SAMELINE("%s, ", value_b ? "true" : "false");
        break;
    }

    case TLV::kTLVType_UTF8String: {
        char value_s[256];

        err = aReader.GetString(value_s, sizeof(value_s));
        VerifyOrReturnError(err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL, err);

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
        uint8_t value_b[256];
        uint32_t len, readerLen;

        readerLen = aReader.GetLength();

        err = aReader.GetBytes(value_b, sizeof(value_b));
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
                PRETTY_PRINT_SAMELINE("0x%x, ", value_b[i]);
            }
        }

        err = CHIP_NO_ERROR;
        PRETTY_PRINT("\t\t]");
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

        while ((err = aReader.Next()) == CHIP_NO_ERROR)
        {
            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(ParseData(aReader, aDepth + 1));
            PRETTY_PRINT_DECDEPTH();
        }

        PRETTY_PRINT("%c,", terminating_char);
        ReturnErrorOnFailure(aReader.ExitContainer(type));
    }
    return CHIP_NO_ERROR;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR EventDataIB::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int TagPresenceMask = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("EventDataIB =");
    PRETTY_PRINT("{");

    // make a copy of the Path reader
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
                EventPathIB::Parser path;
                ReturnErrorOnFailure(path.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(path.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case to_underlying(Tag::kEventNumber):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kEventNumber))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kEventNumber));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                EventNumber number;
                ReturnErrorOnFailure(reader.Get(number));
                PRETTY_PRINT("\tEventNumber = 0x%" PRIx64 ",", number);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kPriority):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kPriority))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kPriority));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                ReturnErrorOnFailure(reader.Get(value));
                PRETTY_PRINT("\tPriorityLevel = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kEpochTimestamp):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kEpochTimestamp))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kEpochTimestamp));

            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                ReturnErrorOnFailure(reader.Get(value));
                PRETTY_PRINT("\tEpochTimestamp = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING
            break;

        case to_underlying(Tag::kSystemTimestamp):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kSystemTimestamp))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kSystemTimestamp));

            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                ReturnErrorOnFailure(reader.Get(value));
                PRETTY_PRINT("\tSystemTimestamp = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kDeltaEpochTimestamp):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kDeltaEpochTimestamp))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kDeltaEpochTimestamp));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                ReturnErrorOnFailure(reader.Get(value));
                PRETTY_PRINT("\tDeltaEpochTimestampstamp= 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kDeltaSystemTimestamp):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kDeltaSystemTimestamp))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kDeltaSystemTimestamp));

            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                ReturnErrorOnFailure(reader.Get(value));
                PRETTY_PRINT("\tDeltaSystemTimestamp = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kData):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kData))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kData));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(ParseData(reader, 0));
            PRETTY_PRINT_DECDEPTH();
            break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }
    PRETTY_PRINT("},");
    PRETTY_PRINT("");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const int RequiredFields =
            (1 << to_underlying(Tag::kPath)) | (1 << to_underlying(Tag::kPriority)) | (1 << to_underlying(Tag::kData));

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_EVENT_DATA_ELEMENT;
        }
    }
    ReturnErrorOnFailure(err);
    ReturnErrorOnFailure(reader.ExitContainer(mOuterContainerType));
    return CHIP_NO_ERROR;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR EventDataIB::Parser::GetPath(EventPathIB::Parser * const apPath)
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kPath)), reader));
    ReturnErrorOnFailure(apPath->Init(reader));
    return CHIP_NO_ERROR;
}

CHIP_ERROR EventDataIB::Parser::GetPriority(uint8_t * const apPriority)
{
    return GetUnsignedInteger(to_underlying(Tag::kPriority), apPriority);
}

CHIP_ERROR EventDataIB::Parser::GetEventNumber(EventNumber * const apEventNumber)
{
    return GetUnsignedInteger(to_underlying(Tag::kEventNumber), apEventNumber);
}

CHIP_ERROR EventDataIB::Parser::GetEpochTimestamp(uint64_t * const apEpochTimestamp)
{
    return GetUnsignedInteger(to_underlying(Tag::kEpochTimestamp), apEpochTimestamp);
}

CHIP_ERROR EventDataIB::Parser::GetSystemTimestamp(uint64_t * const apSystemTimestamp)
{
    return GetUnsignedInteger(to_underlying(Tag::kSystemTimestamp), apSystemTimestamp);
}

CHIP_ERROR EventDataIB::Parser::GetDeltaEpochTimestamp(uint64_t * const apDeltaEpochTimestampstamp)
{
    return GetUnsignedInteger(to_underlying(Tag::kDeltaEpochTimestamp), apDeltaEpochTimestampstamp);
}

CHIP_ERROR EventDataIB::Parser::GetDeltaSystemTimestamp(uint64_t * const apDeltaSystemTimestamp)
{
    return GetUnsignedInteger(to_underlying(Tag::kDeltaSystemTimestamp), apDeltaSystemTimestamp);
}

CHIP_ERROR EventDataIB::Parser::GetData(TLV::TLVReader * const apReader) const
{
    return mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kData)), *apReader);
}

CHIP_ERROR EventDataIB::Parser::ProcessEventPath(EventPathIB::Parser & aEventPath, ConcreteEventPath & aConcreteEventPath)
{
    // The ReportData must contain a concrete event path
    CHIP_ERROR err = aEventPath.GetEndpoint(&(aConcreteEventPath.mEndpointId));
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_IM_MALFORMED_EVENT_PATH);

    err = aEventPath.GetCluster(&(aConcreteEventPath.mClusterId));
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_IM_MALFORMED_EVENT_PATH);

    err = aEventPath.GetEvent(&(aConcreteEventPath.mEventId));
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_IM_MALFORMED_EVENT_PATH);

    return CHIP_NO_ERROR;
}

CHIP_ERROR EventDataIB::Parser::ProcessEventTimestamp(EventHeader & aEventHeader)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    uint64_t timeStampVal        = 0;
    bool hasSystemTimestamp      = false;
    bool hasEpochTimestamp       = false;
    bool hasDeltaSystemTimestamp = false;
    bool hasDeltaEpochTimestamp  = false;

    err = GetDeltaSystemTimestamp(&timeStampVal);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(aEventHeader.mTimestamp.IsSystem(), CHIP_ERROR_IM_MALFORMED_EVENT_DATA_ELEMENT);
        aEventHeader.mTimestamp.mValue += timeStampVal;
        hasDeltaSystemTimestamp = true;
    }
    ReturnErrorOnFailure(err);

    err = GetDeltaEpochTimestamp(&timeStampVal);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(aEventHeader.mTimestamp.IsEpoch(), CHIP_ERROR_IM_MALFORMED_EVENT_DATA_ELEMENT);
        aEventHeader.mTimestamp.mValue += timeStampVal;
        hasDeltaEpochTimestamp = true;
    }
    ReturnErrorOnFailure(err);

    err = GetSystemTimestamp(&timeStampVal);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else if (err == CHIP_NO_ERROR)
    {
        aEventHeader.mTimestamp.mType  = Timestamp::Type::kSystem;
        aEventHeader.mTimestamp.mValue = timeStampVal;
        hasSystemTimestamp             = true;
    }
    ReturnErrorOnFailure(err);

    err = GetEpochTimestamp(&timeStampVal);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    else if (err == CHIP_NO_ERROR)
    {
        aEventHeader.mTimestamp.mType  = Timestamp::Type::kEpoch;
        aEventHeader.mTimestamp.mValue = timeStampVal;
        hasEpochTimestamp              = true;
    }

    if (hasSystemTimestamp + hasEpochTimestamp + hasDeltaSystemTimestamp + hasDeltaEpochTimestamp == 1)
    {
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_IM_MALFORMED_EVENT_DATA_ELEMENT;
}

CHIP_ERROR EventDataIB::Parser::DecodeEventHeader(EventHeader & aEventHeader)
{
    uint8_t priorityLevel = 0;
    EventPathIB::Parser path;
    ReturnErrorOnFailure(GetPath(&path));
    ReturnErrorOnFailure(ProcessEventPath(path, aEventHeader.mPath));
    ReturnErrorOnFailure(GetEventNumber(&(aEventHeader.mEventNumber)));
    ReturnErrorOnFailure(GetPriority(&priorityLevel));
    aEventHeader.mPriorityLevel = static_cast<PriorityLevel>(priorityLevel);
    ReturnErrorOnFailure(ProcessEventTimestamp(aEventHeader));
    return CHIP_NO_ERROR;
}

EventPathIB::Builder & EventDataIB::Builder::CreatePath()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mPath.Init(mpWriter, to_underlying(Tag::kPath));
    }
    return mPath;
}

EventDataIB::Builder & EventDataIB::Builder::Priority(const uint8_t aPriority)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kPriority)), aPriority);
    }
    return *this;
}

EventDataIB::Builder & EventDataIB::Builder::EventNumber(const uint64_t aEventNumber)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kEventNumber)), aEventNumber);
    }
    return *this;
}

EventDataIB::Builder & EventDataIB::Builder::EpochTimestamp(const uint64_t aEpochTimestamp)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kEpochTimestamp)), aEpochTimestamp);
    }
    return *this;
}

EventDataIB::Builder & EventDataIB::Builder::SystemTimestamp(const uint64_t aSystemTimestamp)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kSystemTimestamp)), aSystemTimestamp);
    }
    return *this;
}

EventDataIB::Builder & EventDataIB::Builder::DeltaEpochTimestamp(const uint64_t aDeltaEpochTimestamp)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kDeltaEpochTimestamp)), aDeltaEpochTimestamp);
    }
    return *this;
}

EventDataIB::Builder & EventDataIB::Builder::DeltaSystemTimestamp(const uint64_t aDeltaSystemTimestamp)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kDeltaSystemTimestamp)), aDeltaSystemTimestamp);
    }
    return *this;
}

// Mark the end of this element and recover the type for outer container
EventDataIB::Builder & EventDataIB::Builder::EndOfEventDataIB()
{
    EndOfContainer();
    return *this;
}
} // namespace app
} // namespace chip
