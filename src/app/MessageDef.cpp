/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/*
 *
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      This file defines CHIP interaction model message parsers and encoders
 *      Interaction model profile.
 *
 */

// __STDC_FORMAT_MACROS must be defined for PRIX64 to be defined for pre-C++11 clib
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif // __STDC_FORMAT_MACROS

// __STDC_LIMIT_MACROS must be defined for UINT8_MAX and INT32_MAX to be defined for pre-C++11 clib
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif // __STDC_LIMIT_MACROS

// __STDC_CONSTANT_MACROS must be defined for INT64_C and UINT64_C to be defined for pre-C++11 clib
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif // __STDC_CONSTANT_MACROS

#include <inttypes.h>
#include <stdio.h>
#include <stdarg.h>
#include "MessageDef.h"

using namespace chip;
using namespace chip::TLV;

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

namespace chip {
namespace app {

#if CHIP_DETAIL_LOGGING

static uint32_t gPrettyPrintingDepthLevel = 0;
static char gLineBuffer[256];
static size_t gCurLineBufferSize = 0;

/**
 * Simple object to checkpoint the pretty-print indentation level and make
 * sure it is restored at the end of a block (even in case of early exit).
 */
class PrettyPrintCheckpoint
{
public:
    PrettyPrintCheckpoint()
    {
        mLevel = gPrettyPrintingDepthLevel;
    }
    ~PrettyPrintCheckpoint()
    {
        gPrettyPrintingDepthLevel = mLevel;
    }
private:
    uint32_t mLevel;
};
#define PRETTY_PRINT_CHECKPOINT()  PrettyPrintCheckpoint lPrettyPrintCheckpoint;

#define PRETTY_PRINT(fmt, ...)                                                                                                     \
    do                                                                                                                             \
    {                                                                                                                              \
        PrettyPrintDM(true, fmt, ##__VA_ARGS__);                                                                                  \
    } while (0)
#define PRETTY_PRINT_SAMELINE(fmt, ...)                                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        PrettyPrintDM(false, fmt, ##__VA_ARGS__);                                                                                 \
    } while (0)
#define PRETTY_PRINT_INCDEPTH()                                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        gPrettyPrintingDepthLevel++;                                                                                               \
    } while (0)
#define PRETTY_PRINT_DECDEPTH()                                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        gPrettyPrintingDepthLevel--;                                                                                               \
    } while (0)

static void PrettyPrintDM(bool aIsNewLine, const char * aFmt, ...)
{
    va_list args;
    size_t ret;
    size_t sizeLeft;

    va_start(args, aFmt);

    if (aIsNewLine)
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
                sizeLeft = sizeof(gLineBuffer) - gCurLineBufferSize;
                ret      = (size_t)(snprintf(gLineBuffer + gCurLineBufferSize, sizeLeft, "\t"));
                if (ret > 0)
                {
                    gCurLineBufferSize += MIN(ret, sizeLeft);
                }
            }
        }
    }

    if (sizeof(gLineBuffer) > gCurLineBufferSize)
    {
        sizeLeft = sizeof(gLineBuffer) - gCurLineBufferSize;
        ret      = (size_t)(vsnprintf(gLineBuffer + gCurLineBufferSize, sizeLeft, aFmt, args));
        if (ret > 0)
        {
            gCurLineBufferSize += MIN(ret, sizeLeft);
        }
    }

    va_end(args);
}
#else // CHIP_DETAIL_LOGGING
#define PRETTY_PRINT_CHECKPOINT()
#define PRETTY_PRINT(fmt, ...)
#define PRETTY_PRINT_SAMELINE(fmt, ...)
#define PRETTY_PRINT_INCDEPTH()
#define PRETTY_PRINT_DECDEPTH()
#endif // CHIP_DETAIL_LOGGING

CHIP_ERROR LookForElementWithTag(const chip::TLV::TLVReader & aSrcReader, const uint64_t aTagInApiForm,
                                  chip::TLV::TLVReader * apDstReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the Path reader
    chip::TLV::TLVReader reader;
    reader.Init(aSrcReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        // Documentation says the result of GetType must be verified before calling GetTag
        VerifyOrExit(chip::TLV::kTLVType_NotSpecified != reader.GetType(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);

        if (aTagInApiForm == reader.GetTag())
        {
            apDstReader->Init(reader);
            break;
        }
    }

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

ParserBase::ParserBase() { }

CHIP_ERROR ParserBase::GetReaderOnTag(const uint64_t aTagToFind, chip::TLV::TLVReader * const apReader) const
{
    return LookForElementWithTag(mReader, aTagToFind, apReader);
}

template <typename T>
CHIP_ERROR ParserBase::GetUnsignedInteger(const uint8_t aContextTag, T * const apLValue) const
{
    return GetSimpleValue(aContextTag, chip::TLV::kTLVType_UnsignedInteger, apLValue);
}

template <typename T>
CHIP_ERROR ParserBase::GetSimpleValue(const uint8_t aContextTag, const chip::TLV::TLVType aTLVType, T * const apLValue) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    *apLValue = 0;

    err = LookForElementWithTag(mReader, chip::TLV::ContextTag(aContextTag), &reader);
    SuccessOrExit(err);

    VerifyOrExit(aTLVType == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = reader.Get(*apLValue);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

ListParserBase::ListParserBase() { }

CHIP_ERROR ListParserBase::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the reader here
    mReader.Init(aReader);

    VerifyOrExit(chip::TLV::kTLVType_Array == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    // This is just a dummy, as we're not going to exit this container ever
    chip::TLV::TLVType OuterContainerType;
    err = mReader.EnterContainer(OuterContainerType);

exit:
    ChipLogFunctError(err);

    return err;
}

CHIP_ERROR ListParserBase::InitIfPresent(const chip::TLV::TLVReader & aReader, const uint8_t aContextTagToFind)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = LookForElementWithTag(aReader, chip::TLV::ContextTag(aContextTagToFind), &reader);
    SuccessOrExit(err);

    err = Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR ListParserBase::Next(void)
{
    CHIP_ERROR err = mReader.Next();

    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

void ListParserBase::GetReader(chip::TLV::TLVReader * const apReader)
{
    apReader->Init(mReader);
}

BuilderBase::BuilderBase() :
    mError(CHIP_ERROR_INCORRECT_STATE), mpWriter(NULL), mOuterContainerType(chip::TLV::kTLVType_NotSpecified)
{ }

void BuilderBase::ResetError()
{
    return ResetError(CHIP_NO_ERROR);
}

void BuilderBase::ResetError(CHIP_ERROR aErr)
{
    mError              = aErr;
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;
}

void BuilderBase::EndOfContainer()
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->EndContainer(mOuterContainerType);
    SuccessOrExit(mError);

    // we've just closed properly
    // mark it so we do not panic when the build object destructor is called
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;

exit:;
}

CHIP_ERROR BuilderBase::InitAnonymousStructure(chip::TLV::TLVWriter * const apWriter)
{
    mpWriter            = apWriter;
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;
    mError = mpWriter->StartContainer(chip::TLV::AnonymousTag, chip::TLV::kTLVType_Structure, mOuterContainerType);
    ChipLogFunctError(mError);

    return mError;
}

ListBuilderBase::ListBuilderBase(void) { }

CHIP_ERROR ListBuilderBase::Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse)
{
    mpWriter            = apWriter;
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;
    mError =
        mpWriter->StartContainer(chip::TLV::ContextTag(aContextTagToUse), chip::TLV::kTLVType_Array, mOuterContainerType);
    ChipLogFunctError(mError);

    return mError;
}

/**
 * Init the TLV array container with an anonymous tag.
 * Required to implement arrays of arrays, and to test ListBuilderBase.
 * There is no chip interaction model message that has an array as the outermost container.
 *
 * @param[in]   apWriter    Pointer to the TLVWriter that is encoding the message.
 *
 * @return                  CHIP_ERROR codes returned by Chip::TLV objects.
 */
CHIP_ERROR ListBuilderBase::Init(chip::TLV::TLVWriter * const apWriter)
{
    mpWriter            = apWriter;
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;
    mError =
        mpWriter->StartContainer(chip::TLV::AnonymousTag, chip::TLV::kTLVType_Array, mOuterContainerType);
    ChipLogFunctError(mError);

    return mError;
}

CHIP_ERROR AttributePath::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the reader here
    mReader.Init(aReader);

    VerifyOrExit(chip::TLV::kTLVType_Path == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    // This is just a dummy, as we're not going to exit this container ever
    chip::TLV::TLVType dummyContainerType;
    // enter into the Path
    err = mReader.EnterContainer(dummyContainerType);
    SuccessOrExit(err);

exit:
    ChipLogFunctError(err);

    return err;
}

#if CHIP_CONFIG_INTERACTION_MODEL_ENABLE_SCHEMA_CHECK
// Roughly verify the schema is right, including
// 1) all mandatory tags are present
// 2) no unknown tags
// 3) all elements have expected data type
// 4) any tag can only appear once
CHIP_ERROR AttributePath::Parser::CheckSchemaValidity(void) const
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT_SAMELINE("<AttributePath = {");

    // make a copy of the Path reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);
        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_EndpointId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_EndpointId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EndpointId);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint16_t endpointId;
                reader.Get(endpointId);
                PRETTY_PRINT_SAMELINE("EndpointId = 0x%" PRIx16 ",", endpointId);
            }
            else
            {
                PRETTY_PRINT_SAMELINE("EndpointId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_ClusterId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_ClusterId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_ClusterId);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint32_t clusterId;
                reader.Get(clusterId);
                PRETTY_PRINT_SAMELINE("ClusterId = 0x%" PRIx32 ",", clusterId);
            }
            else
            {
                PRETTY_PRINT_SAMELINE("ClusterId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_FieldTag:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_FieldTag)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_FieldTag);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint8_t fieldTag;
                reader.Get(fieldTag);
                PRETTY_PRINT_SAMELINE("FieldTag = 0x%" PRIx8 ",", fieldTag);
            }
            else
            {
                PRETTY_PRINT_SAMELINE("fieldTag = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_ListIndex:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_ListIndex)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_ListIndex);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint16_t listIndex;
                reader.Get(listIndex);
                PRETTY_PRINT_SAMELINE("ListIndex = 0x%" PRIx16 ",", listIndex);
            }
            else
            {
                PRETTY_PRINT_SAMELINE("ListIndex = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            ExitNow(err = CHIP_ERROR_INVALID_TLV_TAG);
        }
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const uint16_t RequiredFields = (1 << kCsTag_EndpointId) | (1 << kCsTag_ClusterId);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_INTERACTION_MODEL_MALFORMED_EVENT_PATH;
        }
    }
    SuccessOrExit(err);

    PRETTY_PRINT_SAMELINE("}");

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_INTERACTION_MODEL_ENABLE_SCHEMA_CHECK

CHIP_ERROR AttributePath::Parser::GetEndpointId(uint32_t * const apEndpointId) const
{
    return GetUnsignedInteger(kCsTag_EndpointId, apEndpointId);
}

CHIP_ERROR AttributePath::Parser::GetClusterId(uint32_t * const apClusterId) const
{
    return GetUnsignedInteger(kCsTag_ClusterId, apClusterId);
}

CHIP_ERROR AttributePath::Parser::GetFieldTag(uint8_t * const apFieldTag) const
{
    return GetUnsignedInteger(kCsTag_FieldTag, apFieldTag);
}

CHIP_ERROR AttributePath::Parser::GetListIndex(uint8_t * const apListIndex) const
{
    return GetUnsignedInteger(kCsTag_ListIndex, apListIndex);
}

CHIP_ERROR AttributePath::Builder::_Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTagInApiForm)
{
    mpWriter            = apWriter;
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;
    mError              = mpWriter->StartContainer(aTagInApiForm, chip::TLV::kTLVType_Path, mOuterContainerType);
    SuccessOrExit(mError);

exit:
    ChipLogFunctError(mError);
    return mError;
}

CHIP_ERROR AttributePath::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return _Init(apWriter, chip::TLV::AnonymousTag);
}

CHIP_ERROR AttributePath::Builder::Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse)
{
    return _Init(apWriter, chip::TLV::ContextTag(aContextTagToUse));
}

AttributePath::Builder & AttributePath::Builder::EndpointId(const uint32_t aEndpointId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_EndpointId), aEndpointId);
    ChipLogFunctError(mError);

exit:

    return *this;
}

AttributePath::Builder & AttributePath::Builder::ClusterId(const uint32_t aClusterId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_ClusterId), aClusterId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

AttributePath::Builder & AttributePath::Builder::FieldTag(const uint8_t aFieldTag)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_FieldTag), aFieldTag);
    ChipLogFunctError(mError);

exit:
    return *this;
}

AttributePath::Builder & AttributePath::Builder::ListIndex(const uint16_t aListIndex)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_ListIndex), aListIndex);
    ChipLogFunctError(mError);

exit:
    return *this;
}

AttributePath::Builder & AttributePath::Builder::EndOfAttributePath(void)
{
    EndOfContainer();
    return *this;
}

#if CHIP_CONFIG_DATA_MANAGEMENT_ENABLE_SCHEMA_CHECK
        // Roughly verify the schema is right, including
// 1) at least one element is there
//    (not checked since we want to subscribe/read to events only some times)
// 2) all elements are anonymous and of Path type
// 3) every path is also valid in schema
CHIP_ERROR AttributePathList::Parser::CheckSchemaValidity(void) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    size_t NumPath  = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("AttributePathList =");
    PRETTY_PRINT("[");

    // make a copy of the AttributePathList reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(chip::TLV::kTLVType_Path == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        {
            AttributePath::Parser path;
            err = path.Init(reader);
            SuccessOrExit(err);

            PRETTY_PRINT("\t");

            err = path.CheckSchemaValidity();
            SuccessOrExit(err);
        }

        ++NumPath;
    }

    PRETTY_PRINT("],");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_DATA_MANAGEMENT_ENABLE_SCHEMA_CHECK

// Re-initialize the shared PathBuilder with anonymous tag
AttributePath::Builder & AttributePathList::Builder::CreateAttributePathBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributePathBuilder.ResetError(mError));

    mError = mAttributePathBuilder.Init(mpWriter);
    ChipLogFunctError(mError);

exit:
    // on error, mAttributePathBuilder would be un-/partial initialized and cannot be used to write anything
    return mAttributePathBuilder;
}

// Mark the end of this array and recover the type for outer container
AttributePathList::Builder & AttributePathList::Builder::EndOfAttributePathList(void)
{
    EndOfContainer();

    return *this;
}

CHIP_ERROR EventPath::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the reader here
    mReader.Init(aReader);

    VerifyOrExit(chip::TLV::kTLVType_Path == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    // This is just a dummy, as we're not going to exit this container ever
    chip::TLV::TLVType dummyContainerType;
    // enter into the Path
    err = mReader.EnterContainer(dummyContainerType);
    SuccessOrExit(err);

    exit:
    ChipLogFunctError(err);

    return err;
}

#if CHIP_CONFIG_INTERACTION_MODEL_ENABLE_SCHEMA_CHECK
// Roughly verify the schema is right, including
// 1) all mandatory tags are present
// 2) no unknown tags
// 3) all elements have expected data type
// 4) any tag can only appear once
CHIP_ERROR EventPath::Parser::CheckSchemaValidity(void) const
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT_SAMELINE("<EventPath = {");

    // make a copy of the Path reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);
        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_EndpointId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_EndpointId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EndpointId);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint16_t endpointId;
                reader.Get(clusterId);
                PRETTY_PRINT_SAMELINE("EndpointId = 0x%" PRIx16 ",", endpointId);
            }
            else
            {
                PRETTY_PRINT_SAMELINE("EndpointId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_ClusterId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_ClusterId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_ClusterId);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint32_t clusterId;
                reader.Get(clusterId);
                PRETTY_PRINT_SAMELINE("ClusterId = 0x%" PRIx32 ",", clusterId);
            }
            else
            {
                PRETTY_PRINT_SAMELINE("ClusterId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case chip::app::EventPath::kCsTag_EventId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << chip::app::EventPath::kCsTag_EventId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << chip::app::EventPath::kCsTag_EventId);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint16_t eventId;
                reader.Get(eventId);
                PRETTY_PRINT_SAMELINE("EventId = 0x%" PRIx16 ",", eventId);
            }
            else
            {
                PRETTY_PRINT_SAMELINE("EventId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            ExitNow(err = CHIP_ERROR_INVALID_TLV_TAG);
        }
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const uint16_t RequiredFields = (1 << kCsTag_EndpointId) | (1 << kCsTag_ClusterId);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_INTERACTION_MODEL_MALFORMED_EVENT_PATH;
        }
    }
    SuccessOrExit(err);

    PRETTY_PRINT_SAMELINE("}");

exit:
    ChipLogFunctError(err);

return err;
}
#endif // CHIP_CONFIG_INTERACTION_MODEL_ENABLE_SCHEMA_CHECK

CHIP_ERROR EventPath::Parser::GetEndpointId(uint32_t * const apEndpointID) const
{
    return GetUnsignedInteger(kCsTag_EndpointId, apEndpointID);
}

CHIP_ERROR EventPath::Parser::GetClusterId(uint32_t * const apClusterId) const
{
    return GetUnsignedInteger(kCsTag_ClusterId, apClusterId);
}

CHIP_ERROR EventPath::Parser::GetEventId(uint16_t * const apEventId) const
{
    return GetUnsignedInteger(kCsTag_EventId, apEventId);
}

CHIP_ERROR EventPath::Builder::_Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTagInApiForm)
{
    mpWriter            = apWriter;
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;
    mError              = mpWriter->StartContainer(aTagInApiForm, chip::TLV::kTLVType_Path, mOuterContainerType);
    SuccessOrExit(mError);

    exit:
    ChipLogFunctError(mError);
    return mError;
}

CHIP_ERROR EventPath::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return _Init(apWriter, chip::TLV::AnonymousTag);
}

CHIP_ERROR EventPath::Builder::Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse)
{
    return _Init(apWriter, chip::TLV::ContextTag(aContextTagToUse));
}

EventPath::Builder & EventPath::Builder::EndpointId(const uint32_t aEndpointId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_EndpointId), aEndpointId);
    ChipLogFunctError(mError);

    exit:

    return *this;
}

EventPath::Builder & EventPath::Builder::ClusterId(const uint32_t aClusterId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_ClusterId), aClusterId);
    ChipLogFunctError(mError);

    exit:
    return *this;
}

EventPath::Builder & EventPath::Builder::EventId(const uint16_t aEventId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_EventId), aEventId);
    ChipLogFunctError(mError);

    exit:
    return *this;
}

EventPath::Builder & EventPath::Builder::EndOfEventPath(void)
{
    EndOfContainer();
    return *this;
}

#if CHIP_CONFIG_DATA_MANAGEMENT_ENABLE_SCHEMA_CHECK
// Roughly verify the schema is right, including
// 1) at least one element is there
//    (not checked since we want to subscribe/read to events only some times)
// 2) all elements are anonymous and of Path type
// 3) every path is also valid in schema
CHIP_ERROR EventPathList::Parser::CheckSchemaValidity(void) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    size_t NumPath  = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("EventPathList =");
    PRETTY_PRINT("[");

    // make a copy of the EventPathList reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(chip::TLV::kTLVType_Path == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        {
            EventPath::Parser path;
            err = path.Init(reader);
            SuccessOrExit(err);

            PRETTY_PRINT("\t");

            err = path.CheckSchemaValidity();
            SuccessOrExit(err);
        }

        ++NumPath;
    }

    PRETTY_PRINT("],");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_DATA_MANAGEMENT_ENABLE_SCHEMA_CHECK

// Re-initialize the shared PathBuilder with anonymous tag
EventPath::Builder & EventPathList::Builder::CreateEventPathBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mEventPathBuilder.ResetError(mError));

    mError = mEventPathBuilder.Init(mpWriter);
    ChipLogFunctError(mError);

exit:
    // on error, mPathBuilder would be un-/partial initialized and cannot be used to write anything
    return mEventPathBuilder;
}

// Mark the end of this array and recover the type for outer container
EventPathList::Builder & EventPathList::Builder::EndOfEventPathList(void)
{
    EndOfContainer();

    return *this;
}

CHIP_ERROR EventDataElement::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the reader here
    mReader.Init(aReader);

    VerifyOrExit(chip::TLV::kTLVType_Structure == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    // This is just a dummy, as we're not going to exit this container ever
    chip::TLV::TLVType OuterContainerType;
    err = mReader.EnterContainer(OuterContainerType);

exit:
    ChipLogFunctError(err);

    return err;
}

#if CHIP_CONFIG_DATA_MANAGEMENT_ENABLE_SCHEMA_CHECK
        // Roughly verify the schema is right
CHIP_ERROR EventDataElement::Parser::CheckSchemaValidity(void) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;
    uint32_t tagNum = 0;

    struct TagPresence
    {
        bool Source : 1;
        bool Importance : 1;
        bool Id : 1;

        bool RelatedImportance : 1;
        bool RelatedId : 1;
        bool UTCTimestamp : 1;
        bool SystemTimestamp : 1;
        bool ResourceId : 1;
        bool TraitProfileId : 1;
        bool TraitInstanceId : 1;
        bool Type : 1;

        bool DeltaUTCTime : 1;
        bool DeltaSystemTime : 1;

        bool Data : 1;
    };

    TagPresence tagPresence = { 0 };

    PRETTY_PRINT("\t{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);

        tagNum = chip::TLV::TagNumFromTag(reader.GetTag());

        switch (tagNum)
        {
        case kCsTag_Source:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.Source == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.Source = true;

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tSource = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING

            break;

        case kCsTag_ImportanceLevel:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.Importance == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.Importance = true;

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tImportance = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING

            break;

        case kCsTag_Id:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.Id == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.Id = true;

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tId = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING

            break;

        case kCsTag_RelatedImportance:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.RelatedImportance == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.RelatedImportance = true;

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tRelatedImportance = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING

            break;

        case kCsTag_RelatedId:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.RelatedId == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.RelatedId = true;

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tRelatedId = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING

            break;

        case kCsTag_UTCTimestamp:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.UTCTimestamp == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.UTCTimestamp = true;

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tUTCTimestamp = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING

            break;

        case kCsTag_SystemTimestamp:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.SystemTimestamp == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.SystemTimestamp = true;

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tSystemTimestamp = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING

            break;

        case kCsTag_ResourceId:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.ResourceId == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.ResourceId = true;

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType() || chip::TLV::kTLVType_ByteString == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                ResourceIdentifier resourceId;
                char strResource[ResourceIdentifier::MAX_STRING_LENGTH];
                err = resourceId.FromTLV(reader);
                if (err == CHIP_NO_ERROR)
                {
                    resourceId.ToString(strResource, sizeof(strResource));
                    PRETTY_PRINT("\t\t%s,", strResource);
                }
                else
                {
                    PRETTY_PRINT("\t\tResourceId = ??,");
                }
                SuccessOrExit(err);
            }
#endif // CHIP_DETAIL_LOGGING

            break;

        case kCsTag_TraitProfileId:
        {
            SchemaVersionRange requestedVersion;
            uint32_t ProfileID;

            // check if this tag has appeared before
            VerifyOrExit(tagPresence.TraitProfileId == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.TraitProfileId = true;

            if (reader.GetType() == chip::TLV::kTLVType_Array)
            {
                TLV::TLVType type;

                err = reader.EnterContainer(type);
                SuccessOrExit(err);

                err = reader.Next();
                SuccessOrExit(err);

                VerifyOrExit(reader.GetType() == chip::TLV::kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);

                err = reader.Get(ProfileID);
                SuccessOrExit(err);

                err = reader.Next();
                VerifyOrExit(err == CHIP_NO_ERROR || err == CHIP_END_OF_TLV, );

                if (err == CHIP_NO_ERROR)
                {
                    VerifyOrExit(reader.GetType() == chip::TLV::kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);

                    err = reader.Get(requestedVersion.mMaxVersion);
                    SuccessOrExit(err);
                }

                err = reader.Next();
                VerifyOrExit(err == CHIP_NO_ERROR || err == CHIP_END_OF_TLV, );

                if (err == CHIP_NO_ERROR)
                {
                    VerifyOrExit(reader.GetType() == chip::TLV::kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);

                    err = reader.Get(requestedVersion.mMinVersion);
                    SuccessOrExit(err);
                }

                err = reader.Next();
                VerifyOrExit(err == CHIP_END_OF_TLV, err = CHIP_ERROR_WDM_MALFORMED_DATA_ELEMENT);

                err = reader.ExitContainer(type);
            }
            else
            {
                VerifyOrExit(reader.GetType() == chip::TLV::kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);

                err = reader.Get(ProfileID);
                SuccessOrExit(err);
            }

#if CHIP_DETAIL_LOGGING
            {
                if (requestedVersion.mMaxVersion > 1 || requestedVersion.mMinVersion > 1)
                {
                    PRETTY_PRINT("\t\tTraitProfileId = 0x%" PRIx32, ProfileID);

                    if (requestedVersion.mMaxVersion > 1)
                    {
                        PRETTY_PRINT_SAMELINE(", MaxVersion = %" PRIu32, requestedVersion.mMaxVersion);
                    }

                    if (requestedVersion.mMaxVersion > 1)
                    {
                        PRETTY_PRINT_SAMELINE(", MinVersion = %" PRIu32 ",", requestedVersion.mMinVersion);
                    }
                    else
                    {
                        PRETTY_PRINT_SAMELINE(",");
                    }
                }
                else
                {
                    PRETTY_PRINT("\t\tTraitProfileId = 0x%" PRIx32 ",", ProfileID);
                }
            }
#endif // CHIP_DETAIL_LOGGING

            break;
        }

        case kCsTag_TraitInstanceId:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.TraitInstanceId == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.TraitInstanceId = true;

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tTraitInstanceId = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING

            break;

        case kCsTag_Type:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.Type == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.Type = true;

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tType = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING

            break;

        case kCsTag_DeltaUTCTime:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.DeltaUTCTime == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.DeltaUTCTime = true;

            VerifyOrExit(chip::TLV::kTLVType_SignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tDeltaUTCTime = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING

            break;

        case kCsTag_DeltaSystemTime:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.DeltaSystemTime == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.DeltaSystemTime = true;

            VerifyOrExit(chip::TLV::kTLVType_SignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tDeltaSystemTime = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING

            break;

        case kCsTag_Data:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.Data == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.Data = true;

            err = ParseData(reader, 0);
            SuccessOrExit(err);
            break;

        default:
            PRETTY_PRINT("\t\tUnknown tag num %" PRIu32, tagNum);
            break;
        }
    }

    PRETTY_PRINT("\t},");
    PRETTY_PRINT("");

    // almost all fields in an Event are optional
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_DATA_MANAGEMENT_ENABLE_SCHEMA_CHECK

CHIP_ERROR EventDataElement::Parser::GetEventPath(EventPath::Parser * const apEventPath)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = LookForElementWithTag(mReader, chip::TLV::ContextTag(kCsTag_EventPath), &reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Path == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apEventPath->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR EventDataElement::Parser::GetImportanceLevel(uint8_t * const apImportanceLevel)
{
    return GetUnsignedInteger(kCsTag_ImportanceLevel, apImportanceLevel);
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

CHIP_ERROR EventDataElement::Parser::GetDeltaUTCTime(uint64_t * const apDeltaUTCTime)
{
    return GetUnsignedInteger(kCsTag_DeltaUTCTime, apDeltaUTCTime);
}

CHIP_ERROR EventDataElement::Parser::GetDeltaSystemTime(uint64_t * const apDeltaSystemTime)
{
    return GetUnsignedInteger(kCsTag_DeltaSystemTime, apDeltaSystemTime);
}

CHIP_ERROR EventDataElement::Parser::GetReaderOnEvent(chip::TLV::TLVReader * const apReader) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = LookForElementWithTag(mReader, chip::TLV::ContextTag(kCsTag_Data), apReader);
    ChipLogFunctError(err);

    return err;
}

CHIP_ERROR EventDataElement::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

EventPath::Builder & EventDataElement::Builder::CreateEventPathBuilder(void)
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mEventPathBuilder.ResetError(mError));

    mError = mEventPathBuilder.Init(mpWriter, kCsTag_EventPath);
    ChipLogFunctError(mError);

exit:
    return mEventPathBuilder;
}

EventDataElement::Builder EventDataElement::Builder::ImportanceLevel(const uint8_t aImportanceLevel)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_ImportanceLevel), aImportanceLevel);
    ChipLogFunctError(mError);

exit:
    return *this;
}

EventDataElement::Builder EventDataElement::Builder::Number(const uint64_t aNumber)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_Number), aNumber);
    ChipLogFunctError(mError);

exit:
    return *this;
}

EventDataElement::Builder EventDataElement::Builder::UTCTimestamp(const uint64_t aUTCTimestamp)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_UTCTimestamp), aUTCTimestamp);
    ChipLogFunctError(mError);

exit:
    return *this;
}

EventDataElement::Builder EventDataElement::Builder::SystemTimestamp(const uint64_t aSystemTimestamp)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_SystemTimestamp), aSystemTimestamp);
    ChipLogFunctError(mError);

exit:
    return *this;
}

EventDataElement::Builder EventDataElement::Builder::DeltaUTCTime(const uint64_t aDeltaUTCTime)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_DeltaUTCTime), aDeltaUTCTime);
    ChipLogFunctError(mError);

exit:
    return *this;
}

EventDataElement::Builder EventDataElement::Builder::DeltaSystemTime(const uint64_t aDeltaSystemTime)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_DeltaSystemTime), aDeltaSystemTime);
    ChipLogFunctError(mError);

exit:
    return *this;
}

// Mark the end of this element and recover the type for outer container
EventDataElement::Builder & EventDataElement::Builder::EndOfEventDataElement(void)
{
    EndOfContainer();
    return *this;
}

#if CHIP_CONFIG_DATA_MANAGEMENT_ENABLE_SCHEMA_CHECK
CHIP_ERROR EventList::Parser::CheckSchemaValidity(void) const
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    size_t NumDataElement = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("EventList =");
    PRETTY_PRINT("[");

    // make a copy of the EventList reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(chip::TLV::kTLVType_Structure == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        {
            EventDataElement::Parser event;
            err = event.Init(reader);
            SuccessOrExit(err);
            err = event.CheckSchemaValidity();
            SuccessOrExit(err);
        }

        ++NumDataElement;
    }

    PRETTY_PRINT("],");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // if we have at least one data element
        if (NumDataElement > 0)
        {
            err = CHIP_NO_ERROR;
        }
        // NOTE: temporarily disable this check, to allow test to continue
        else
        {
            ChipLogError(DataManagement, "PROTOCOL ERROR: Empty event list");
            err = CHIP_NO_ERROR;
        }
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_DATA_MANAGEMENT_ENABLE_SCHEMA_CHECK

EventDataElement::Builder & EventList::Builder::CreateEventBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mEventDataElementBuilder.ResetError(mError));

    mError = mEventDataElementBuilder.Init(mpWriter);
    ChipLogFunctError(mError);

exit:
    // on error, mEventDataElementBuilder would be un-/partial initialized and cannot be used to write anything
    return mEventDataElementBuilder;
}

// Mark the end of this array and recover the type for outer container
EventList::Builder & EventList::Builder::EndOfEventList(void)
{
    EndOfContainer();
    return *this;
}

// aReader has to be on the element of StatusElement
CHIP_ERROR StatusCode::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the reader here
    mReader.Init(aReader);
    VerifyOrExit(chip::TLV::kTLVType_Structure == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    // This is just a dummy, as we're not going to exit this container ever
    chip::TLV::TLVType OuterContainerType;
    err = mReader.EnterContainer(OuterContainerType);

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR StatusCode::Parser::GetProtocolId(uint32_t * apProtocolId) const
{
    return GetUnsignedInteger(kCsTag_ProtocolId, apProtocolId);
}

CHIP_ERROR StatusCode::Parser::GetClusterId(uint32_t * apClusterId) const
{
    return GetUnsignedInteger(kCsTag_ProtocolId, apClusterId);
}

CHIP_ERROR StatusCode::Parser::GetDetailedCode(uint16_t * apDetailedCode) const
{
    return GetUnsignedInteger(kCsTag_ProtocolId, apDetailedCode);
}

CHIP_ERROR StatusCode::Parser::GetGeneralCode(uint16_t * apGeneralCode) const
{
    return GetUnsignedInteger(kCsTag_ProtocolId, apGeneralCode);
}

#if CHIP_CONFIG_INTERACTION_MODEL_ENABLE_SCHEMA_CHECK
/**
 * Check the StatusElement is an array with at least two unsigned integers.
 * The first one is the ProfileID, the second one is the StatusCode.
 *
 * @return  CHIP_NO_ERROR in case of success; CHIP_ERROR codes
 *          returned by Chip::TLV objects otherwise.
 */
CHIP_ERROR StatusCode::Parser::CheckSchemaValidity(void) const
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("\t{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        // This is an array; all elements are anonymous.
        VerifyOrExit(chip::TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);

        if (!(TagPresenceMask & (1 << kCsTag_ProtocolId)))
        {
            // The first element has to be the ProfileID.
            TagPresenceMask |= (1 << kCsTag_ProtocolId);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint32_t kCsTag_ProtocolId;
                err = reader.Get(kCsTag_ProtocolId);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tProtocolId = 0x%" PRIx32 ",", kCsTag_ProtocolId);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else if (!(TagPresenceMask & (1 << kCsTag_ClusterId)))
        {
            // The second element has to be the StatusCode.
            TagPresenceMask |= (1 << kCsTag_ClusterId);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint32_t clusterId;
                err = reader.Get(clusterId);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tClusterId = 0x%" PRIx32 ",", clusterId);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else if (!(TagPresenceMask & (1 << kCsTag_DetailedId)))
        {
            // The second element has to be the StatusCode.
            TagPresenceMask |= (1 << kCsTag_DetailedId);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint16_t detailedCode;
                err = reader.Get(detailedCode);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tDetailedCode = 0x%" PRIx16 ",", detailedCode);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else if (!(TagPresenceMask & (1 << kCsTag_GeneralCode)))
        {
            // The second element has to be the StatusCode.
            TagPresenceMask |= (1 << kCsTag_GeneralCode);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint32_t generalCode;
                err = reader.Get(generalCode);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tGeneralCode = 0x%" PRIx16 ",", generalCode);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else
        {
            PRETTY_PRINT("\t\tExtra element in StatusCode");
        }
    }

    PRETTY_PRINT("\t},");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const uint16_t RequiredFields = (1 << kCsTag_DetailedCode);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_INTERACTIVE_MODEL_MALFORMED_STATUS_ELEMENT;
        }
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_DATA_MANAGEMENT_ENABLE_SCHEMA_CHECK

CHIP_ERROR StatusCode::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

StatusCode::Builder & StatusCode::Builder::ProtocolId(const uint32_t aProtocolId)
{
    SuccessOrExit(mError);
    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_ProtocolId), aProtocolId);

exit:
    ChipLogFunctError(mError);
    return *this;
}

StatusCode::Builder & StatusCode::Builder::ClusterId(const uint32_t aClusterId)
{
    SuccessOrExit(mError);
    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_ClusterId), aClusterId);

exit:
    ChipLogFunctError(mError);
    return *this;
}

StatusCode::Builder & StatusCode::Builder::DetailedCode(const uint16_t aDetailedCode)
{
    SuccessOrExit(mError);
    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_DetailedCode), aDetailedCode);

exit:
    ChipLogFunctError(mError);
    return *this;
}

StatusCode::Builder & StatusCode::Builder::GeneralCode(const uint16_t aGeneralCode)
{
    SuccessOrExit(mError);
    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_GeneralCode), aGeneralCode);

exit:
    ChipLogFunctError(mError);
    return *this;
}

StatusCode::Builder & StatusCode::Builder::EndOfStatusCode(void)
{
    EndOfContainer();
    return *this;
}

CHIP_ERROR AttributeStatusElement::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

AttributePath::Builder & AttributeStatusElement::Builder::CreateAttributePathBuilder(void)
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributePathBuilder.ResetError(mError));

    mError = mAttributePathBuilder.Init(mpWriter);

exit:
    ChipLogFunctError(mError);
    return mAttributePathBuilder;
}

StatusCode::Builder & AttributeStatusElement::Builder::CreateStatusCodeBuilder(void)
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mStatusCodeBuilder.ResetError(mError));

    mError = mStatusCodeBuilder.Init(mpWriter);

exit:
    ChipLogFunctError(mError);
    return mStatusCodeBuilder;
}

AttributeStatusElement::Builder & AttributeStatusElement::Builder::EndOfAttributeStatusElement(void)
{
    EndOfContainer();
    return *this;
}

CHIP_ERROR AttributeStatusElement::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the reader here
    mReader.Init(aReader);
    VerifyOrExit(chip::TLV::kTLVType_Structure == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    // This is just a dummy, as we're not going to exit this container ever
    chip::TLV::TLVType OuterContainerType;
    err = mReader.EnterContainer(OuterContainerType);

exit:
    ChipLogFunctError(err);
    return err;
}

#if CHIP_CONFIG_INTERACTION_MODEL_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributeStatusElement::Parser::CheckSchemaValidity(void) const
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("\tAttributeStatusElement{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        // This is an array; all elements are anonymous.
        VerifyOrExit(chip::TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);

        if (!(TagPresenceMask & (1 << kCsTag_AttributePath)))
        {
            // The first element has to be the ProfileID.
            TagPresenceMask |= (1 << kCsTag_AttributePath);

            VerifyOrExit(chip::TLV::kTLVType_Structure == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            {
                AttributePath::Parser path;
                err = path.Init(reader);
                SuccessOrExit(err);
                err = path.CheckSchemaValidity();
                SuccessOrExit(err);
            }
        }
        else if (!(TagPresenceMask & (1 << kCsTag_StatusCode)))
        {
            // The second element has to be the StatusCode.
            TagPresenceMask |= (1 << kCsTag_StatusCode);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            VerifyOrExit(chip::TLV::kTLVType_Structure == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
            {
                StatusCode::Parser status;
                err = status.Init(reader);
                SuccessOrExit(err);
                err = status.CheckSchemaValidity();
                SuccessOrExit(err);
            }
        }
        else
        {
            PRETTY_PRINT("\t\tExtra element in StatusCode");
        }
    }

    PRETTY_PRINT("\t},");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const uint16_t RequiredFields = (1 << kCsTag_AttributePath) | (1 << kCsTag_StatusCode);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_INTERACTIVE_MODEL_MALFORMED_STATUS_ELEMENT;
        }
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_INTERACTION_MODEL_ENABLE_SCHEMA_CHECK

CHIP_ERROR AttributeStatusElement::Parser::GetAttributePath(AttributePath::Parser * const apAttributePath) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = LookForElementWithTag(mReader, chip::TLV::ContextTag(kCsTag_AttributePath), &reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Structure == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apAttributePath->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}


CHIP_ERROR AttributeStatusElement::Parser::GetStatusCode(StatusCode::Parser * const apStatusCode) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = LookForElementWithTag(mReader, chip::TLV::ContextTag(kCsTag_StatusCode), &reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Structure == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apStatusCode->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

AttributeStatusElement::Builder & AttributeStatusList::Builder::CreateAttributeStatusBuilder(void)
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributeStatusBuilder.ResetError(mError));

    mError = mAttributeStatusBuilder.Init(mpWriter);
    ChipLogFunctError(mError);

exit:
    // on error, mAttributeStatusBuilder would be un-/partial initialized and cannot be used to write anything
    return mAttributeStatusBuilder;
}

AttributeStatusList::Builder & AttributeStatusList::Builder::EndOfStatusList(void)
{
    EndOfContainer();
    return *this;
}

CHIP_ERROR AttributeStatusList::Parser::CheckSchemaValidity(void) const
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    size_t NumAttributeStateElement = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("AttributeStatusList =");
    PRETTY_PRINT("[");

    // make a copy of the EventList reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(chip::TLV::kTLVType_Structure == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        {
            AttributeStatusElement::Parser status;
            err = status.Init(reader);
            SuccessOrExit(err);
            err = status.CheckSchemaValidity();
            SuccessOrExit(err);
        }

        ++NumAttributeStateElement;
    }

    PRETTY_PRINT("],");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // if we have at least one data element
        if (NumAttributeStateElement > 0)
        {
            err = CHIP_NO_ERROR;
        }
            // NOTE: temporarily disable this check, to allow test to continue
        else
        {
            ChipLogError(DataManagement, "PROTOCOL ERROR: Empty attribute status list");
            err = CHIP_NO_ERROR;
        }
    }

exit:
    ChipLogFunctError(err);

    return err;
}

CHIP_ERROR AttributeDataElement::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the reader here
    mReader.Init(aReader);

    VerifyOrExit(chip::TLV::kTLVType_Structure == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    // This is just a dummy, as we're not going to exit this container ever
    chip::TLV::TLVType OuterContainerType;
    err = mReader.EnterContainer(OuterContainerType);

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
        PRETTY_PRINT("\t\tData = ");
    }
    else
    {
        if (chip::TLV::IsContextTag(aReader.GetTag()))
        {
            PRETTY_PRINT("\t\t0x%" PRIx32 " = ", chip::TLV::TagNumFromTag(aReader.GetTag()));
        }
        else if (chip::TLV::IsProfileTag(aReader.GetTag()))
        {
            PRETTY_PRINT("\t\t0x%" PRIx32 "::0x%" PRIx32 " = ", chip::TLV::ProfileIdFromTag(aReader.GetTag()),
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
        PRETTY_PRINT("\t\t{");
        break;

    case chip::TLV::kTLVType_Array:
        PRETTY_PRINT_SAMELINE("[");
        PRETTY_PRINT("\t\t\t");
        break;

    case chip::TLV::kTLVType_SignedInteger:
    {
        int64_t value_s64;

        err = aReader.Get(value_s64);
        SuccessOrExit(err);

        PRETTY_PRINT_SAMELINE("%" PRId64 ", ", value_s64);
        break;
    }

    case chip::TLV::kTLVType_UnsignedInteger:
    {
        uint64_t value_u64;

        err = aReader.Get(value_u64);
        SuccessOrExit(err);

        PRETTY_PRINT_SAMELINE("%" PRIu64 ", ", value_u64);
        break;
    }

    case chip::TLV::kTLVType_Boolean:
    {
        bool value_b;

        err = aReader.Get(value_b);
        SuccessOrExit(err);

        PRETTY_PRINT_SAMELINE("%s, ", value_b ? "true" : "false");
        break;
    }

    case chip::TLV::kTLVType_UTF8String:
    {
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

    case chip::TLV::kTLVType_ByteString:
    {
        uint8_t value_b[256];
        uint32_t len, readerLen;

        readerLen = aReader.GetLength();

        err = aReader.GetBytes(value_b, sizeof(value_b));
        VerifyOrExit(err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL, );

        PRETTY_PRINT_SAMELINE("[");
        PRETTY_PRINT("\t\t\t");

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

        PRETTY_PRINT("\t\t%c,", terminating_char);

        err = aReader.ExitContainer(type);
        SuccessOrExit(err);
    }

exit:
    ChipLogFunctError(err);
    return err;
}

#if CHIP_CONFIG_DATA_MANAGEMENT_ENABLE_SCHEMA_CHECK
// Roughly verify the schema is right, including
// 1) all mandatory tags are present
// 2) all elements have expected data type
// 3) any tag can only appear once
// At the top level of the structure, unknown tags are ignored for foward compatibility
CHIP_ERROR AttributeDataElement::Parser::CheckSchemaValidity(void) const
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;
    uint32_t tagNum = 0;

    PRETTY_PRINT("\t{");

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
            VerifyOrExit(chip::TLV::kTLVType_Path == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            PRETTY_PRINT("\t\tAttributePath = ");

            {
                AttributePath::Parser path;
                err = path.Init(reader);
                SuccessOrExit(err);
                err = path.CheckSchemaValidity();
                SuccessOrExit(err);
            }

            break;
        case kCsTag_DataVersion:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_DataVersion)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_DataVersion);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t version;
                err = reader.Get(version);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tDataElementVersion = 0x%" PRIx64 ",", version);
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
        case kCsTag_MoreToComeFlag:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_MoreToComeFlag)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_MoreToComeFlag);
            VerifyOrExit(chip::TLV::kTLVType_Boolean == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                bool flag;
                err = reader.Get(flag);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tDataElement_kCsTag_MoreToComeFlag = %s,", flag ? "true" : "false");
            }

#endif // CHIP_DETAIL_LOGGING
            break;

        default:
            PRETTY_PRINT("\t\tUnknown tag num %" PRIu32, tagNum);
            break;
        }
    }

    PRETTY_PRINT("\t},");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const uint16_t RequiredFields = (1 << kCsTag_Path);

        // Either the data or deleted keys should be present.
        const uint16_t DataElementTypeMask = (1 << kCsTag_AttributePath) | (1 << kCsTag_DataVersion) | (1 << kCsTag_Data);
        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_WDM_MALFORMED_DATA_ELEMENT;
        }
        if ((TagPresenceMask & DataElementTypeMask) == 0)
        {
            err = CHIP_ERROR_WDM_MALFORMED_DATA_ELEMENT;
        }
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_INTERACTION_MODEL_ENABLE_SCHEMA_CHECK

CHIP_ERROR AttributeDataElement::Parser::GetAttributePath(AttributePath::Parser * const apAttributePath) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = LookForElementWithTag(mReader, chip::TLV::ContextTag(kCsTag_AttributePath), &reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Structure == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apAttributePath->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

// CHIP_END_OF_TLV if there is no such element
// CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
CHIP_ERROR AttributeDataElement::Parser::GetDataVersion(uint64_t * const apVersion) const
{
    return GetUnsignedInteger(kCsTag_DataVersion, apVersion);
}

// Data could be of any type, so we can only position the reader so the caller has
// full information of tag, element type, length, and value
CHIP_ERROR AttributeDataElement::Parser::GetData(chip::TLV::TLVReader * const apReader) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = LookForElementWithTag(mReader, chip::TLV::ContextTag(kCsTag_Data), apReader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

// Default is false if there is no such element
// CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a boolean
CHIP_ERROR AttributeDataElement::Parser::GetMoreToComeFlag(bool * const apGetMoreToComeFlag) const
{
    return GetSimpleValue(kCsTag_MoreToComeFlag, chip::TLV::kTLVType_Boolean, apGetMoreToComeFlag);
}

// DataElement is only used in a Data List, which requires every path to be anonymous
// Note that both mWriter and mPathBuilder only hold reference to the actual TLVWriter
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

AttributeDataElement::Builder & AttributeDataElement::Builder::DataVersion(const uint64_t aDataVersion)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_DataVersion), aDataVersion);
    ChipLogFunctError(mError);

exit:
    return *this;
}

AttributeDataElement::Builder & AttributeDataElement::Builder::MoreToCome(const bool aMoreToCome)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    if (aMoreToCome)
    {
        mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_MoreToComeFlag), true);
        ChipLogFunctError(mError);
    }

exit:
    return *this;
}

AttributeDataElement::Builder & AttributeDataElement::Builder::EndOfAttributeDataElement(void)
{
    EndOfContainer();

    return *this;
}

#if CHIP_CONFIG_INTERACTION_MODEL_ENABLE_SCHEMA_CHECK
// Roughly verify the schema is right, including
// 1) at least one element is there
// 2) all elements are anonymous and of Structure type
// 3) every Data Element is also valid in schemasd
CHIP_ERROR AttributeDataList::Parser::CheckSchemaValidity(void) const
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    size_t NumDataElement = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("AttributeDataList =");
    PRETTY_PRINT("[");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(chip::TLV::kTLVType_Structure == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        {
            AttributeDataElement::Parser data;
            err = data.Init(reader);
            SuccessOrExit(err);
            err = data.CheckSchemaValidity();
            SuccessOrExit(err);
        }

        ++NumDataElement;
    }

    PRETTY_PRINT("],");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // if we have at least one data element
        if (NumDataElement > 0)
        {
            err = CHIP_NO_ERROR;
        }
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_DATA_MANAGEMENT_ENABLE_SCHEMA_CHECK

// Re-initialize the shared PathBuilder with anonymous tag
AttributeDataElement::Builder & AttributeDataList::Builder::CreateAttributeDataElementBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributeDataElementBuilder.ResetError(mError));

    mError = mAttributeDataElementBuilder.Init(mpWriter);
    ChipLogFunctError(mError);

exit:

    // on error, mAttributeDataElementBuilder would be un-/partial initialized and cannot be used to write anything
    return mAttributeDataElementBuilder;
}

// Mark the end of this array and recover the type for outer container
AttributeDataList::Builder & AttributeDataList::Builder::EndOfAttributeDataList(void)
{
    EndOfContainer();

    return *this;
}

#if CHIP_CONFIG_INTERACTION_MODEL_ENABLE_SCHEMA_CHECK
// Roughly verify the schema is right, including
// 1) all mandatory tags are present
// 2) all elements have expected data type
// 3) any tag can only appear once
// At the top level of the message, unknown tags are ignored for foward compatibility
CHIP_ERROR ReportDataRequest::Parser::CheckSchemaValidity(void) const
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;
    AttributeStatusList::Parser attributeStatusList;
    AttributeDataList::Parser attributeDataList;
    EventList::Parser eventList;

    enum
    {
        kBit_RequestResponse       = 0,
        kBit_SubscriptionId         = 1,
        kBit_AttributeStatusList            = 2,
        kBit_AttributeDataList = 3,
        kBit_EventDataList        = 4,
        kBit_IsLastReport     = 5,
    };

    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        const uint64_t tag = reader.GetTag();

        if (chip::TLV::ContextTag(kBit_RequestResponse) == tag)
        {
            VerifyOrExit(!(TagPresenceMask & (1 << kBit_RequestResponse)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kBit_RequestResponse);
            VerifyOrExit(chip::TLV::kTLVType_Boolean == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                bool RequestResponse;
                err = reader.Get(RequestResponse);
                SuccessOrExit(err);
                PRETTY_PRINT("\tRequestResponse = %u,", PossibleLossOfEvent);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else if (chip::TLV::ContextTag(kCsTag_SubscriptionId) == tag)
        {
            VerifyOrExit(!(TagPresenceMask & (1 << kBit_SubscriptionId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kBit_SubscriptionId);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t id;
                err = reader.Get(id);
                SuccessOrExit(err);
                PRETTY_PRINT("\tSubscriptionId = 0x%" PRIx64 ",", id);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else if (chip::TLV::ContextTag(kCsTag_AttributeStatusList) == tag)
        {
            VerifyOrExit(!(TagPresenceMask & (1 << kBit_AttributeStatusList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kBit_AttributeStatusList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            attributeStatusList.Init(reader);

            PRETTY_PRINT_INCDEPTH();

            err = attributeStatusList.CheckSchemaValidity();
            SuccessOrExit(err);

            PRETTY_PRINT_DECDEPTH();
        }
        else if (chip::TLV::ContextTag(kCsTag_AttributeDataList) == tag)
        {
            VerifyOrExit(!(TagPresenceMask & (1 << kBit_AttributeDataList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kBit_AttributeDataList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            attributeDataList.Init(reader);

            PRETTY_PRINT_INCDEPTH();

            err = attributeDataList.CheckSchemaValidity();
            SuccessOrExit(err);

            PRETTY_PRINT_DECDEPTH();
        }
        else if (chip::TLV::ContextTag(kCsTag_EventDataList) == tag)
        {
            VerifyOrExit(!(TagPresenceMask & (1 << kBit_EventDataList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kBit_EventDataList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            eventList.Init(reader);

            PRETTY_PRINT_INCDEPTH();

            err = eventList.CheckSchemaValidity();
            SuccessOrExit(err);

            PRETTY_PRINT_DECDEPTH();
        }
        else if (chip::TLV::ContextTag(kBit_IsLastReport) == tag)
        {
            VerifyOrExit(!(TagPresenceMask & (1 << kBit_IsLastReport)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kBit_IsLastReport);
            VerifyOrExit(chip::TLV::kTLVType_Boolean == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                bool isLastReport;
                err = reader.Get(isLastReport);
                SuccessOrExit(err);
                PRETTY_PRINT("\tisLastReport = %u,", isLastReport);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else
        {
            PRETTY_PRINT("\tUnknown tag 0x%" PRIx64, tag);
        }

    }

    PRETTY_PRINT("}");
    PRETTY_PRINT("");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // if we have at least the DataList or EventList field
        if ((TagPresenceMask & (1 << kBit_RequestResponse)) ||
            (TagPresenceMask & (1 << kBit_IsLastReport)))
        {
            err = CHIP_NO_ERROR;
        }
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_DATA_MANAGEMENT_ENABLE_SCHEMA_CHECK

CHIP_ERROR ReportDataRequest::Parser::GetRequestResponse(bool * const apRequestResponse) const
{
    return GetSimpleValue(kCsTag_RequestResponse, chip::TLV::kTLVType_Boolean, apRequestResponse);
}

CHIP_ERROR ReportDataRequest::Parser::GetSubscriptionID(uint64_t * const apSubscriptionID) const
{
    return GetUnsignedInteger(kCsTag_SubscriptionId, apSubscriptionID);
}

CHIP_ERROR ReportDataRequest::Parser::GetAttributeStatusList(AttributeStatusList::Parser * const apAttributeStatusList) const
{
    return apAttributeStatusList->InitIfPresent(mReader, kCsTag_AttributeStatusList);
}

// Get a TLVReader for the Paths. Next() must be called before accessing them.
CHIP_ERROR ReportDataRequest::Parser::GetAttributeDataList(AttributeDataList::Parser * const apAttributeDataList) const
{
    return apAttributeDataList->InitIfPresent(mReader, kCsTag_AttributeDataList);
}

CHIP_ERROR ReportDataRequest::Parser::GetEventDataList(EventList::Parser * const apEventDataList) const
{
    return apEventDataList->InitIfPresent(mReader, kCsTag_EventDataList);
}

CHIP_ERROR ReportDataRequest::Parser::GetIsLastReport(bool * const apIsLastReport) const
{
    return GetSimpleValue(kCsTag_IsLastReport, chip::TLV::kTLVType_Boolean, apIsLastReport);
}

}; // namespace app
}; // namespace chip
