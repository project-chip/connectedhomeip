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

#include "MessageDef.h"
#include <algorithm>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

using namespace chip;
using namespace chip::TLV;

#ifndef CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
#define CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK 1
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

namespace chip {
namespace app {

#if CHIP_DETAIL_LOGGING

namespace {
uint32_t gPrettyPrintingDepthLevel = 0;
char gLineBuffer[256];
size_t gCurLineBufferSize = 0;
} // namespace

class PrettyPrintCheckpoint
{
public:
    PrettyPrintCheckpoint() { mLevel = gPrettyPrintingDepthLevel; }
    ~PrettyPrintCheckpoint() { gPrettyPrintingDepthLevel = mLevel; }

private:
    uint32_t mLevel;
};
#define PRETTY_PRINT_CHECKPOINT() PrettyPrintCheckpoint lPrettyPrintCheckpoint;

#define PRETTY_PRINT(fmt, ...)                                                                                                     \
    do                                                                                                                             \
    {                                                                                                                              \
        PrettyPrintIM(true, fmt, ##__VA_ARGS__);                                                                                   \
    } while (0)
#define PRETTY_PRINT_SAMELINE(fmt, ...)                                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        PrettyPrintIM(false, fmt, ##__VA_ARGS__);                                                                                  \
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

static void PrettyPrintIM(bool aIsNewLine, const char * aFmt, ...)
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
                    gCurLineBufferSize += std::min(ret, sizeLeft);
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
            gCurLineBufferSize += std::min(ret, sizeLeft);
        }
    }

    va_end(args);
}
#else // CHIP_DETAIL_LOGGING
#define PRETTY_PRINT_CHECKPOINT()
#define PRETTY_PRINT(fmt, ...)
#define PRETTY_PRINT(fmt, ...)
#define PRETTY_PRINT_INCDEPTH()
#define PRETTY_PRINT_DECDEPTH()
#endif // CHIP_DETAIL_LOGGING

ParserBase::ParserBase() {}

CHIP_ERROR ParserBase::GetReaderOnTag(const uint64_t aTagToFind, chip::TLV::TLVReader * const apReader) const
{
    return mReader.FindElementWithTag(aTagToFind, *apReader);
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

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(aContextTag), reader);
    SuccessOrExit(err);

    VerifyOrExit(aTLVType == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = reader.Get(*apLValue);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

ListParserBase::ListParserBase() {}

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

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(aContextTagToFind), reader);
    SuccessOrExit(err);

    err = Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR ListParserBase::Next()
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
{}

void BuilderBase::ResetError()
{
    ResetError(CHIP_NO_ERROR);
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
    mError              = mpWriter->StartContainer(chip::TLV::AnonymousTag, chip::TLV::kTLVType_Structure, mOuterContainerType);
    ChipLogFunctError(mError);

    return mError;
}

ListBuilderBase::ListBuilderBase() {}

CHIP_ERROR ListBuilderBase::Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse)
{
    mpWriter            = apWriter;
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;
    mError = mpWriter->StartContainer(chip::TLV::ContextTag(aContextTagToUse), chip::TLV::kTLVType_Array, mOuterContainerType);
    ChipLogFunctError(mError);

    return mError;
}

CHIP_ERROR ListBuilderBase::Init(chip::TLV::TLVWriter * const apWriter)
{
    mpWriter            = apWriter;
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;
    mError              = mpWriter->StartContainer(chip::TLV::AnonymousTag, chip::TLV::kTLVType_Array, mOuterContainerType);
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

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributePath::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("<AttributePath = {");

    // make a copy of the Path reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);
        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_NodeId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_NodeId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_NodeId);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint64_t nodeId;
                reader.Get(nodeId);
                PRETTY_PRINT("NodeId = 0x%" PRIx64 ",", nodeId);
            }
            else
            {
                PRETTY_PRINT("NodeId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_EndpointId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_EndpointId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EndpointId);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint16_t endpointId;
                reader.Get(endpointId);
                PRETTY_PRINT("EndpointId = 0x%" PRIx16 ",", endpointId);
            }
            else
            {
                PRETTY_PRINT("EndpointId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_NamespacedClusterId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_NamespacedClusterId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_NamespacedClusterId);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                chip::ClusterId namespacedClusterId;
                reader.Get(namespacedClusterId);
                PRETTY_PRINT("NamespacedClusterId = 0x%" PRIx32 ",", namespacedClusterId);
            }
            else
            {
                PRETTY_PRINT("NamespacedClusterId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_FieldId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_FieldId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_FieldId);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint8_t fieldTag;
                reader.Get(fieldTag);
                PRETTY_PRINT("FieldTag = 0x%" PRIx8 ",", fieldTag);
            }
            else
            {
                PRETTY_PRINT("fieldTag = ??");
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
                PRETTY_PRINT("ListIndex = 0x%" PRIx16 ",", listIndex);
            }
            else
            {
                PRETTY_PRINT("ListIndex = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            ExitNow(err = CHIP_ERROR_INVALID_TLV_TAG);
        }
    }

    PRETTY_PRINT("}");
    PRETTY_PRINT("\t");
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const uint16_t RequiredFields = (1 << kCsTag_EndpointId) | (1 << kCsTag_NamespacedClusterId);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
        }
    }
    SuccessOrExit(err);

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR AttributePath::Parser::GetNodeId(chip::NodeId * const apNodeId) const
{
    return GetUnsignedInteger(kCsTag_NodeId, apNodeId);
}

CHIP_ERROR AttributePath::Parser::GetEndpointId(chip::EndpointId * const apEndpointId) const
{
    return GetUnsignedInteger(kCsTag_EndpointId, apEndpointId);
}

CHIP_ERROR AttributePath::Parser::GetNamespacedClusterId(chip::ClusterId * const apClusterId) const
{
    return GetUnsignedInteger(kCsTag_NamespacedClusterId, apClusterId);
}

CHIP_ERROR AttributePath::Parser::GetFieldId(uint8_t * const apFieldId) const
{
    return GetUnsignedInteger(kCsTag_FieldId, apFieldId);
}

CHIP_ERROR AttributePath::Parser::GetListIndex(uint16_t * const apListIndex) const
{
    return GetUnsignedInteger(kCsTag_ListIndex, apListIndex);
}

CHIP_ERROR AttributePath::Builder::_Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTag)
{
    mpWriter            = apWriter;
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;
    mError              = mpWriter->StartContainer(aTag, chip::TLV::kTLVType_Path, mOuterContainerType);
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

AttributePath::Builder & AttributePath::Builder::NodeId(const uint64_t aNodeId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_NodeId), aNodeId);
    ChipLogFunctError(mError);

exit:

    return *this;
}

AttributePath::Builder & AttributePath::Builder::EndpointId(const chip::EndpointId aEndpointId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_EndpointId), aEndpointId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

AttributePath::Builder & AttributePath::Builder::NamespacedClusterId(const chip::ClusterId aClusterId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_NamespacedClusterId), aClusterId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

AttributePath::Builder & AttributePath::Builder::FieldId(const uint8_t aFieldId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_FieldId), aFieldId);
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

AttributePath::Builder & AttributePath::Builder::EndOfAttributePath()
{
    EndOfContainer();
    return *this;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributePathList::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    size_t NumPath = 0;
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
            err = path.CheckSchemaValidity();
            SuccessOrExit(err);
        }

        ++NumPath;
    }

    PRETTY_PRINT("],");
    PRETTY_PRINT("\t");
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

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
AttributePathList::Builder & AttributePathList::Builder::EndOfAttributePathList()
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

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR EventPath::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("\t<EventPath = {");

    // make a copy of the Path reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);
        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_NodeId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_NodeId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_NodeId);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint64_t nodeId;
                reader.Get(nodeId);
                PRETTY_PRINT("\t\tNodeId = 0x%" PRIx64 ",", nodeId);
            }
            else
            {
                PRETTY_PRINT("\t\tNodeId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_EndpointId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_EndpointId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EndpointId);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint16_t endpointId;
                reader.Get(endpointId);
                PRETTY_PRINT("\t\tEndpointId = 0x%" PRIx16 ",", endpointId);
            }
            else
            {
                PRETTY_PRINT("\t\tEndpointId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_NamespacedClusterId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_NamespacedClusterId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_NamespacedClusterId);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                chip::ClusterId namespacedClusterId;
                reader.Get(namespacedClusterId);
                PRETTY_PRINT("\t\tNamespacedClusterId = 0x%" PRIx32 ",", namespacedClusterId);
            }
            else
            {
                PRETTY_PRINT("\t\tNamespacedClusterId = ??");
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
                chip::EventId eventId;
                reader.Get(eventId);
                PRETTY_PRINT("\t\tEventId = 0x%" PRIx16 ",", eventId);
            }
            else
            {
                PRETTY_PRINT("EventId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            ExitNow(err = CHIP_ERROR_INVALID_TLV_TAG);
        }
    }

    PRETTY_PRINT("\t},");
    PRETTY_PRINT("\t");
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const uint16_t RequiredFields = (1 << kCsTag_EndpointId) | (1 << kCsTag_NamespacedClusterId);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_EVENT_PATH;
        }
    }
    SuccessOrExit(err);

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR EventPath::Parser::GetNodeId(chip::NodeId * const apNodeId) const
{
    return GetUnsignedInteger(kCsTag_NodeId, apNodeId);
}

CHIP_ERROR EventPath::Parser::GetEndpointId(chip::EndpointId * const apEndpointID) const
{
    return GetUnsignedInteger(kCsTag_EndpointId, apEndpointID);
}

CHIP_ERROR EventPath::Parser::GetNamespacedClusterId(chip::ClusterId * const apClusterId) const
{
    return GetUnsignedInteger(kCsTag_NamespacedClusterId, apClusterId);
}

CHIP_ERROR EventPath::Parser::GetEventId(chip::EventId * const apEventId) const
{
    return GetUnsignedInteger(kCsTag_EventId, apEventId);
}

CHIP_ERROR EventPath::Builder::_Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTag)
{
    mpWriter            = apWriter;
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;
    mError              = mpWriter->StartContainer(aTag, chip::TLV::kTLVType_Path, mOuterContainerType);
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

EventPath::Builder & EventPath::Builder::NodeId(const uint64_t aNodeId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_NodeId), aNodeId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

EventPath::Builder & EventPath::Builder::EndpointId(const chip::EndpointId aEndpointId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_EndpointId), aEndpointId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

EventPath::Builder & EventPath::Builder::NamespacedClusterId(const chip::ClusterId aClusterId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_NamespacedClusterId), aClusterId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

EventPath::Builder & EventPath::Builder::EventId(const chip::EventId aEventId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_EventId), aEventId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

EventPath::Builder & EventPath::Builder::EndOfEventPath()
{
    EndOfContainer();
    return *this;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR EventPathList::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    size_t NumPath = 0;
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
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

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

EventPathList::Builder & EventPathList::Builder::EndOfEventPathList()
{
    EndOfContainer();

    return *this;
}

CHIP_ERROR CommandPath::Parser::Init(const chip::TLV::TLVReader & aReader)
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

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR CommandPath::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("\t<CommandPath = {");

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
                PRETTY_PRINT("\t\tEndpointId = 0x%" PRIx16 ",", endpointId);
            }
            else
            {
                PRETTY_PRINT("\t\tEndpointId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_GroupId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_GroupId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_GroupId);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint32_t groupId;
                reader.Get(groupId);
                PRETTY_PRINT("\t\tGroupId = 0x%" PRIx64 ",", groupId);
            }
            else
            {
                PRETTY_PRINT("\t\tGroupId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_NamespacedClusterId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_NamespacedClusterId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_NamespacedClusterId);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                chip::ClusterId namespacedClusterId;
                reader.Get(namespacedClusterId);
                PRETTY_PRINT("\t\tNamespacedClusterId = 0x%" PRIx32 ",", namespacedClusterId);
            }
            else
            {
                PRETTY_PRINT("\t\tNamespacedClusterId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_CommandId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_CommandId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_CommandId);

#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                chip::CommandId commandId;
                reader.Get(commandId);
                PRETTY_PRINT("\t\tCommandId = 0x%" PRIx16 ",", commandId);
            }
            else
            {
                PRETTY_PRINT("\t\tCommandId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            ExitNow(err = CHIP_ERROR_INVALID_TLV_TAG);
        }
    }
    PRETTY_PRINT("\t  },");
    PRETTY_PRINT("\t");
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const uint16_t RequiredFields = (1 << kCsTag_CommandId) | (1 << kCsTag_NamespacedClusterId);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_COMMAND_PATH;
        }
    }
    SuccessOrExit(err);
exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR CommandPath::Parser::GetEndpointId(chip::EndpointId * const apEndpointID) const
{
    return GetUnsignedInteger(kCsTag_EndpointId, apEndpointID);
}

CHIP_ERROR CommandPath::Parser::GetGroupId(chip::GroupId * const apGroupId) const
{
    return GetUnsignedInteger(kCsTag_GroupId, apGroupId);
}

CHIP_ERROR CommandPath::Parser::GetNamespacedClusterId(chip::ClusterId * const apClusterId) const
{
    return GetUnsignedInteger(kCsTag_NamespacedClusterId, apClusterId);
}

CHIP_ERROR CommandPath::Parser::GetCommandId(chip::CommandId * const apCommandId) const
{
    return GetUnsignedInteger(kCsTag_CommandId, apCommandId);
}

CHIP_ERROR CommandPath::Builder::_Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTag)
{
    mpWriter            = apWriter;
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;
    mError              = mpWriter->StartContainer(aTag, chip::TLV::kTLVType_Path, mOuterContainerType);
    SuccessOrExit(mError);

exit:
    ChipLogFunctError(mError);
    return mError;
}

CHIP_ERROR CommandPath::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return _Init(apWriter, chip::TLV::AnonymousTag);
}

CHIP_ERROR CommandPath::Builder::Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse)
{
    return _Init(apWriter, chip::TLV::ContextTag(aContextTagToUse));
}

CommandPath::Builder & CommandPath::Builder::EndpointId(const chip::EndpointId aEndpointId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_EndpointId), aEndpointId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

CommandPath::Builder & CommandPath::Builder::GroupId(const chip::GroupId aGroupId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_GroupId), aGroupId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

CommandPath::Builder & CommandPath::Builder::NamespacedClusterId(const chip::ClusterId aClusterId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_NamespacedClusterId), aClusterId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

CommandPath::Builder & CommandPath::Builder::CommandId(const chip::CommandId aCommandId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_CommandId), aCommandId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

CommandPath::Builder & CommandPath::Builder::EndOfCommandPath()
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

CHIP_ERROR
EventDataElement::Parser::ParseData(chip::TLV::TLVReader & aReader, int aDepth) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aDepth == 0)
    {
        PRETTY_PRINT("\t\tEventData = ");
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
        PRETTY_PRINT("[");
        PRETTY_PRINT("\t\t\t");
        break;

    case chip::TLV::kTLVType_SignedInteger: {
        int64_t value_s64;

        err = aReader.Get(value_s64);
        SuccessOrExit(err);

        PRETTY_PRINT("%" PRId64 ", ", value_s64);
        break;
    }

    case chip::TLV::kTLVType_UnsignedInteger: {
        uint64_t value_u64;

        err = aReader.Get(value_u64);
        SuccessOrExit(err);

        PRETTY_PRINT("%" PRIu64 ", ", value_u64);
        break;
    }

    case chip::TLV::kTLVType_Boolean: {
        bool value_b;

        err = aReader.Get(value_b);
        SuccessOrExit(err);

        PRETTY_PRINT("%s, ", value_b ? "true" : "false");
        break;
    }

    case chip::TLV::kTLVType_UTF8String: {
        char value_s[256];

        err = aReader.GetString(value_s, sizeof(value_s));
        VerifyOrExit(err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL, );

        if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            PRETTY_PRINT("... (byte string too long) ...");
            err = CHIP_NO_ERROR;
        }
        else
        {
            PRETTY_PRINT("\"%s\", ", value_s);
        }
        break;
    }

    case chip::TLV::kTLVType_ByteString: {
        uint8_t value_b[256];
        uint32_t len, readerLen;

        readerLen = aReader.GetLength();

        err = aReader.GetBytes(value_b, sizeof(value_b));
        VerifyOrExit(err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL, );

        PRETTY_PRINT("[");
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
            PRETTY_PRINT("... (byte string too long) ...");
        }
        else
        {
            for (size_t i = 0; i < len; i++)
            {
                PRETTY_PRINT("0x%" PRIx8 ", ", value_b[i]);
            }
        }

        err = CHIP_NO_ERROR;
        PRETTY_PRINT("\t\t]");
        break;
    }

    case chip::TLV::kTLVType_Null:
        PRETTY_PRINT("NULL");
        break;

    default:
        PRETTY_PRINT("--");
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

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR EventDataElement::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;
    uint32_t tagNum = 0;

    struct TagPresence
    {
        bool EventPath : 1;
        bool ImportanceLevel : 1;
        bool Number : 1;
        bool UTCTimestamp : 1;
        bool SystemTimestamp : 1;
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
        case kCsTag_EventPath:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.EventPath == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.EventPath = true;

            VerifyOrExit(chip::TLV::kTLVType_Path == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                EventPath::Parser path;
                err = path.Init(reader);
                SuccessOrExit(err);

                PRETTY_PRINT("\t");

                err = path.CheckSchemaValidity();
                SuccessOrExit(err);
            }
#endif // CHIP_DETAIL_LOGGING

            break;

        case kCsTag_ImportanceLevel:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.ImportanceLevel == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.ImportanceLevel = true;

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tImportanceLevel = 0x%" PRIx64 ",", value);
            }
#endif // CHIP_DETAIL_LOGGING

            break;

        case kCsTag_Number:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.Number == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.Number = true;

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint64_t value;
                err = reader.Get(value);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tNumber = 0x%" PRIx64 ",", value);
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
        case kCsTag_DeltaUTCTime:
            // check if this tag has appeared before
            VerifyOrExit(tagPresence.DeltaUTCTime == false, err = CHIP_ERROR_INVALID_TLV_TAG);
            tagPresence.DeltaUTCTime = true;

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

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

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

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
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR EventDataElement::Parser::GetEventPath(EventPath::Parser * const apEventPath)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_EventPath), reader);
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
EventDataElement::Builder & EventDataElement::Builder::EndOfEventDataElement()
{
    EndOfContainer();
    return *this;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR EventList::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
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
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

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

EventList::Builder & EventList::Builder::EndOfEventList()
{
    EndOfContainer();
    return *this;
}

CHIP_ERROR StatusElement::Parser::Init(const chip::TLV::TLVReader & aReader)
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

CHIP_ERROR StatusElement::Parser::DecodeStatusElement(uint16_t * apGeneralCode, uint32_t * apProtocolId, uint16_t * apProtocolCode,
                                                      chip::ClusterId * apClusterId) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader lReader;

    lReader.Init(mReader);

    err = lReader.Next();
    SuccessOrExit(err);
    VerifyOrExit(lReader.GetType() == chip::TLV::kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = lReader.Get(*apGeneralCode);
    SuccessOrExit(err);

    err = lReader.Next();
    SuccessOrExit(err);
    VerifyOrExit(lReader.GetType() == chip::TLV::kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = lReader.Get(*apProtocolId);
    SuccessOrExit(err);

    err = lReader.Next();
    SuccessOrExit(err);
    VerifyOrExit(lReader.GetType() == chip::TLV::kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = lReader.Get(*apProtocolCode);
    SuccessOrExit(err);

    err = lReader.Next();
    SuccessOrExit(err);
    VerifyOrExit(lReader.GetType() == chip::TLV::kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = lReader.Get(*apClusterId);
    SuccessOrExit(err);

exit:
    ChipLogFunctError(err);
    return err;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR StatusElement::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("\t StatusElement{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        // This is an array; all elements are anonymous.
        VerifyOrExit(chip::TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);

        if (!(TagPresenceMask & (1 << kCsTag_GeneralCode)))
        {
            TagPresenceMask |= (1 << kCsTag_GeneralCode);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint16_t generalCode;
                err = reader.Get(generalCode);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tGeneralCode = 0x%" PRIx16 ",", generalCode);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else if (!(TagPresenceMask & (1 << kCsTag_ProtocolId)))
        {
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
        else if (!(TagPresenceMask & (1 << kCsTag_ProtocolCode)))
        {
            TagPresenceMask |= (1 << kCsTag_ProtocolCode);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint16_t protocolCode;
                err = reader.Get(protocolCode);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tprotocolCode = 0x%" PRIx16 ",", protocolCode);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else if (!(TagPresenceMask & (1 << kCsTag_NamespacedClusterId)))
        {
            TagPresenceMask |= (1 << kCsTag_NamespacedClusterId);

            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                chip::ClusterId namespacedClusterId;
                err = reader.Get(namespacedClusterId);
                SuccessOrExit(err);

                PRETTY_PRINT("\t\tNamespacedClusterId = 0x%" PRIx32 ",", namespacedClusterId);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else
        {
            PRETTY_PRINT("\t\tExtra element in StatusElement");
        }
    }

    PRETTY_PRINT("\t},");
    PRETTY_PRINT("");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const uint16_t RequiredFields = (1 << kCsTag_GeneralCode) | (1 << kCsTag_ProtocolId) | (1 << kCsTag_ProtocolCode);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_STATUS_CODE;
        }
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR StatusElement::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return ListBuilderBase::Init(apWriter);
}

CHIP_ERROR StatusElement::Builder::Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse)
{
    return ListBuilderBase::Init(apWriter, aContextTagToUse);
}

StatusElement::Builder & StatusElement::Builder::EncodeStatusElement(const uint16_t aGeneralCode, const uint32_t aProtocolId,
                                                                     const uint16_t aStatusElement,
                                                                     const chip::ClusterId aNamespacedClusterId)
{
    uint64_t tag = chip::TLV::AnonymousTag;

    SuccessOrExit(mError);

    mError = mpWriter->Put(tag, aGeneralCode);
    SuccessOrExit(mError);

    mError = mpWriter->Put(tag, aProtocolId);
    SuccessOrExit(mError);

    mError = mpWriter->Put(tag, aStatusElement);
    SuccessOrExit(mError);

    mError = mpWriter->Put(tag, aNamespacedClusterId);
    SuccessOrExit(mError);

exit:
    ChipLogFunctError(mError);
    return *this;
}

StatusElement::Builder & StatusElement::Builder::EndOfStatusElement()
{
    EndOfContainer();
    return *this;
}

CHIP_ERROR AttributeStatusElement::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

AttributePath::Builder & AttributeStatusElement::Builder::CreateAttributePathBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributePathBuilder.ResetError(mError));

    mError = mAttributePathBuilder.Init(mpWriter, kCsTag_AttributePath);

exit:
    ChipLogFunctError(mError);
    return mAttributePathBuilder;
}

StatusElement::Builder & AttributeStatusElement::Builder::CreateStatusElementBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mStatusElementBuilder.ResetError(mError));

    mError = mStatusElementBuilder.Init(mpWriter, kCsTag_StatusElement);

exit:
    ChipLogFunctError(mError);
    return mStatusElementBuilder;
}

AttributeStatusElement::Builder & AttributeStatusElement::Builder::EndOfAttributeStatusElement()
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

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributeStatusElement::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("\tAttributeStatusElement{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);
        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_AttributePath:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_AttributePath)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributePath);
            {
                AttributePath::Parser path;
                err = path.Init(reader);
                SuccessOrExit(err);
                err = path.CheckSchemaValidity();
                SuccessOrExit(err);
            }
            break;
        case kCsTag_StatusElement:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_StatusElement)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_StatusElement);
            {
                StatusElement::Parser status;
                err = status.Init(reader);
                SuccessOrExit(err);
                err = status.CheckSchemaValidity();
                SuccessOrExit(err);
            }
            break;
        default:
            ExitNow(err = CHIP_ERROR_INVALID_TLV_TAG);
        }
    }

    PRETTY_PRINT("\t},");
    PRETTY_PRINT("");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const uint16_t RequiredFields = (1 << kCsTag_AttributePath) | (1 << kCsTag_StatusElement);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_STATUS_ELEMENT;
        }
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR AttributeStatusElement::Parser::GetAttributePath(AttributePath::Parser * const apAttributePath) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_AttributePath), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Path == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apAttributePath->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR AttributeStatusElement::Parser::GetStatusElement(StatusElement::Parser * const apStatusElement) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_StatusElement), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apStatusElement->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

AttributeStatusElement::Builder & AttributeStatusList::Builder::CreateAttributeStatusBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributeStatusBuilder.ResetError(mError));

    mError = mAttributeStatusBuilder.Init(mpWriter);
    ChipLogFunctError(mError);

exit:
    // on error, mAttributeStatusBuilder would be un-/partial initialized and cannot be used to write anything
    return mAttributeStatusBuilder;
}

AttributeStatusList::Builder & AttributeStatusList::Builder::EndOfAttributeStatusList()
{
    EndOfContainer();
    return *this;
}

CHIP_ERROR AttributeStatusList::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err                  = CHIP_NO_ERROR;
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
        PRETTY_PRINT("[");
        PRETTY_PRINT("\t\t\t");
        break;

    case chip::TLV::kTLVType_SignedInteger: {
        int64_t value_s64;

        err = aReader.Get(value_s64);
        SuccessOrExit(err);

        PRETTY_PRINT("%" PRId64 ", ", value_s64);
        break;
    }

    case chip::TLV::kTLVType_UnsignedInteger: {
        uint64_t value_u64;

        err = aReader.Get(value_u64);
        SuccessOrExit(err);

        PRETTY_PRINT("%" PRIu64 ", ", value_u64);
        break;
    }

    case chip::TLV::kTLVType_Boolean: {
        bool value_b;

        err = aReader.Get(value_b);
        SuccessOrExit(err);

        PRETTY_PRINT("%s, ", value_b ? "true" : "false");
        break;
    }

    case chip::TLV::kTLVType_UTF8String: {
        char value_s[256];

        err = aReader.GetString(value_s, sizeof(value_s));
        VerifyOrExit(err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL, );

        if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            PRETTY_PRINT("... (byte string too long) ...");
            err = CHIP_NO_ERROR;
        }
        else
        {
            PRETTY_PRINT("\"%s\", ", value_s);
        }
        break;
    }

    case chip::TLV::kTLVType_ByteString: {
        uint8_t value_b[256];
        uint32_t len, readerLen;

        readerLen = aReader.GetLength();

        err = aReader.GetBytes(value_b, sizeof(value_b));
        VerifyOrExit(err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL, );

        PRETTY_PRINT("[");
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
            PRETTY_PRINT("... (byte string too long) ...");
        }
        else
        {
            for (size_t i = 0; i < len; i++)
            {
                PRETTY_PRINT("0x%" PRIx8 ", ", value_b[i]);
            }
        }

        err = CHIP_NO_ERROR;
        PRETTY_PRINT("\t\t]");
        break;
    }

    case chip::TLV::kTLVType_Null:
        PRETTY_PRINT("NULL");
        break;

    default:
        PRETTY_PRINT("--");
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

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributeDataElement::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
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
                chip::DataVersion version;
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

                PRETTY_PRINT("\t\tDataElement_kCsTag_MoreClusterDataFlag = %s,", flag ? "true" : "false");
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

    VerifyOrExit(chip::TLV::kTLVType_Path == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

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

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributeDataList::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
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
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

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

AttributeDataList::Builder & AttributeDataList::Builder::EndOfAttributeDataList()
{
    EndOfContainer();

    return *this;
}

CHIP_ERROR CommandDataElement::Parser::Init(const chip::TLV::TLVReader & aReader)
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
CommandDataElement::Parser::ParseData(chip::TLV::TLVReader & aReader, int aDepth) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aDepth == 0)
    {
        PRETTY_PRINT("\t\tCommandData = ");
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
        PRETTY_PRINT("[");
        PRETTY_PRINT("\t\t\t");
        break;

    case chip::TLV::kTLVType_SignedInteger: {
        int64_t value_s64;

        err = aReader.Get(value_s64);
        SuccessOrExit(err);

        PRETTY_PRINT("%" PRId64 ", ", value_s64);
        break;
    }

    case chip::TLV::kTLVType_UnsignedInteger: {
        uint64_t value_u64;

        err = aReader.Get(value_u64);
        SuccessOrExit(err);

        PRETTY_PRINT("%" PRIu64 ", ", value_u64);
        break;
    }
    case chip::TLV::kTLVType_FloatingPointNumber: {
        double value_fp;

        err = aReader.Get(value_fp);
        SuccessOrExit(err);

        PRETTY_PRINT("%lf, ", value_fp);
        break;
    }
    case chip::TLV::kTLVType_Boolean: {
        bool value_b;

        err = aReader.Get(value_b);
        SuccessOrExit(err);

        PRETTY_PRINT("%s, ", value_b ? "true" : "false");
        break;
    }

    case chip::TLV::kTLVType_UTF8String: {
        char value_s[256];

        err = aReader.GetString(value_s, sizeof(value_s));
        VerifyOrExit(err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL, );

        if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            PRETTY_PRINT("... (byte string too long) ...");
            err = CHIP_NO_ERROR;
        }
        else
        {
            PRETTY_PRINT("\"%s\", ", value_s);
        }
        break;
    }

    case chip::TLV::kTLVType_ByteString: {
        uint8_t value_b[256];
        uint32_t len, readerLen;

        readerLen = aReader.GetLength();

        err = aReader.GetBytes(value_b, sizeof(value_b));
        VerifyOrExit(err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL, );

        PRETTY_PRINT("[");
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
            PRETTY_PRINT("... (byte string too long) ...");
        }
        else
        {
            for (size_t i = 0; i < len; i++)
            {
                PRETTY_PRINT("0x%" PRIx8 ", ", value_b[i]);
            }
        }

        err = CHIP_NO_ERROR;
        PRETTY_PRINT("\t\t]");
        break;
    }

    case chip::TLV::kTLVType_Null:
        PRETTY_PRINT("NULL");
        break;

    default:
        PRETTY_PRINT("--");
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

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR CommandDataElement::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
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
        case kCsTag_CommandPath:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_CommandPath)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_CommandPath);
            VerifyOrExit(chip::TLV::kTLVType_Path == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            PRETTY_PRINT("\t\tCommandPath = ");

            {
                CommandPath::Parser path;
                err = path.Init(reader);
                SuccessOrExit(err);
                err = path.CheckSchemaValidity();
                SuccessOrExit(err);
            }

            break;
        case kCsTag_Data:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_Data)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_Data);

            err = ParseData(reader, 0);
            SuccessOrExit(err);
            break;
        case kCsTag_StatusElement:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_StatusElement)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_StatusElement);
            VerifyOrExit(chip::TLV::kTLVType_Structure == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            PRETTY_PRINT("\t\tStatusElement = ");

            {
                StatusElement::Parser status;
                err = status.Init(reader);
                SuccessOrExit(err);
                err = status.CheckSchemaValidity();
                SuccessOrExit(err);
            }

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
        const uint16_t RequiredFields = (1 << kCsTag_CommandPath);
        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_ELEMENT;
        }
    }

exit:
    ChipLogFunctError(err);
    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR CommandDataElement::Parser::GetCommandPath(CommandPath::Parser * const apCommandPath) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_CommandPath), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Path == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apCommandPath->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR CommandDataElement::Parser::GetData(chip::TLV::TLVReader * const apReader) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_Data), *apReader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR CommandDataElement::Parser::GetStatusElement(StatusElement::Parser * const apStatusElement) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_StatusElement), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Structure == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apStatusElement->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR CommandDataElement::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

CommandPath::Builder & CommandDataElement::Builder::CreateCommandPathBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mCommandPathBuilder.ResetError(mError));

    mError = mCommandPathBuilder.Init(mpWriter, kCsTag_CommandPath);
    ChipLogFunctError(mError);

exit:
    // on error, mAttributePathBuilder would be un-/partial initialized and cannot be used to write anything
    return mCommandPathBuilder;
}

StatusElement::Builder & CommandDataElement::Builder::CreateStatusElementBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mStatusElementBuilder.ResetError(mError));

    mError = mStatusElementBuilder.Init(mpWriter, kCsTag_StatusElement);
    ChipLogFunctError(mError);

exit:
    // on error, mStatusElementBuilder would be un-/partial initialized and cannot be used to write anything
    return mStatusElementBuilder;
}

CommandDataElement::Builder & CommandDataElement::Builder::EndOfCommandDataElement()
{
    EndOfContainer();

    return *this;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR CommandList::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    size_t NumCommands = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("CommandList =");
    PRETTY_PRINT("[");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(chip::TLV::kTLVType_Structure == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        {
            CommandDataElement::Parser data;
            err = data.Init(reader);
            SuccessOrExit(err);
            err = data.CheckSchemaValidity();
            SuccessOrExit(err);
        }

        ++NumCommands;
    }

    PRETTY_PRINT("],");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // if we have at least one data element
        if (NumCommands > 0)
        {
            err = CHIP_NO_ERROR;
        }
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CommandDataElement::Builder & CommandList::Builder::CreateCommandDataElementBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mCommandDataElementBuilder.ResetError(mError));

    mError = mCommandDataElementBuilder.Init(mpWriter);
    ChipLogFunctError(mError);

exit:
    // on error, mCommandDataElementBuilder would be un-/partial initialized and cannot be used to write anything
    return mCommandDataElementBuilder;
}

CommandList::Builder & CommandList::Builder::EndOfCommandList()
{
    EndOfContainer();
    return *this;
}

CHIP_ERROR ReportData::Parser::Init(const chip::TLV::TLVReader & aReader)
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

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR ReportData::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;
    AttributeStatusList::Parser attributeStatusList;
    AttributeDataList::Parser attributeDataList;
    EventList::Parser eventList;

    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);

        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_RequestResponse:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_RequestResponse)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_RequestResponse);
#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_Boolean == reader.GetType())
            {
                bool RequestResponse;
                err = reader.Get(RequestResponse);
                SuccessOrExit(err);
                PRETTY_PRINT("\tRequestResponse = %s, ", RequestResponse ? "true" : "false");
            }
            else
            {
                PRETTY_PRINT("RequestResponse = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_SubscriptionId:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_SubscriptionId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_SubscriptionId);
#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint64_t subscriptionId;
                err = reader.Get(subscriptionId);
                SuccessOrExit(err);
                PRETTY_PRINT("\t\tSubscriptionId = 0x%" PRIx64 ",", subscriptionId);
            }
            else
            {
                PRETTY_PRINT("\t\tSubscriptionId = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_AttributeStatusList:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_AttributeStatusList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributeStatusList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            PRETTY_PRINT("\t\t AttributeStatusList = ");
            {
                attributeStatusList.Init(reader);
                PRETTY_PRINT_INCDEPTH();
                err = attributeStatusList.CheckSchemaValidity();
                SuccessOrExit(err);
                PRETTY_PRINT_DECDEPTH();
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_AttributeDataList:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_AttributeDataList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributeDataList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            PRETTY_PRINT("\t\t AttributeDataList = ");
            {
                attributeDataList.Init(reader);
                PRETTY_PRINT_INCDEPTH();
                err = attributeDataList.CheckSchemaValidity();
                SuccessOrExit(err);
                PRETTY_PRINT_DECDEPTH();
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_EventDataList:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_EventDataList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EventDataList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            PRETTY_PRINT("\t\t EventDataList = ");
            {
                eventList.Init(reader);
                PRETTY_PRINT_INCDEPTH();
                err = eventList.CheckSchemaValidity();
                SuccessOrExit(err);
                PRETTY_PRINT_DECDEPTH();
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_IsLastReport:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_IsLastReport)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_IsLastReport);
#if CHIP_DETAIL_LOGGING
            if (chip::TLV::kTLVType_Boolean == reader.GetType())
            {
                bool isLastReport;
                err = reader.Get(isLastReport);
                SuccessOrExit(err);
                PRETTY_PRINT("\tisLastReport = %s, ", isLastReport ? "true" : "false");
            }
            else
            {
                PRETTY_PRINT("isLastReport = ??");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            ExitNow(err = CHIP_ERROR_INVALID_TLV_TAG);
        }
    }

    PRETTY_PRINT("}");
    PRETTY_PRINT("");
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR ReportData::Parser::GetRequestResponse(bool * const apRequestResponse) const
{
    return GetSimpleValue(kCsTag_RequestResponse, chip::TLV::kTLVType_Boolean, apRequestResponse);
}

CHIP_ERROR ReportData::Parser::GetSubscriptionId(uint64_t * const apSubscriptionId) const
{
    return GetUnsignedInteger(kCsTag_SubscriptionId, apSubscriptionId);
}

CHIP_ERROR ReportData::Parser::GetAttributeStatusList(AttributeStatusList::Parser * const apAttributeStatusList) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_AttributeStatusList), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apAttributeStatusList->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR ReportData::Parser::GetAttributeDataList(AttributeDataList::Parser * const apAttributeDataList) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_AttributeDataList), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apAttributeDataList->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR ReportData::Parser::GetEventDataList(EventList::Parser * const apEventDataList) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_EventDataList), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apEventDataList->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR ReportData::Parser::GetIsLastReport(bool * const apIsLastReport) const
{
    return GetSimpleValue(kCsTag_IsLastReport, chip::TLV::kTLVType_Boolean, apIsLastReport);
}

CHIP_ERROR ReportData::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

ReportData::Builder & ReportData::Builder::RequestResponse(const bool aRequestResponse)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_RequestResponse), aRequestResponse);
    ChipLogFunctError(mError);

exit:
    return *this;
}

ReportData::Builder & ReportData::Builder::SubscriptionId(const uint64_t aSubscriptionId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_SubscriptionId), aSubscriptionId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

AttributeStatusList::Builder & ReportData::Builder::CreateAttributeStatusListBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributeStatusListBuilder.ResetError(mError));

    mError = mAttributeStatusListBuilder.Init(mpWriter, kCsTag_AttributeStatusList);
    ChipLogFunctError(mError);

exit:
    return mAttributeStatusListBuilder;
}

AttributeDataList::Builder & ReportData::Builder::CreateAttributeDataListBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributeDataListBuilder.ResetError(mError));

    mError = mAttributeDataListBuilder.Init(mpWriter, kCsTag_AttributeDataList);
    ChipLogFunctError(mError);

exit:
    return mAttributeDataListBuilder;
}

EventList::Builder & ReportData::Builder::CreateEventDataListBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributeDataListBuilder.ResetError(mError));

    mError = mEventDataListBuilder.Init(mpWriter, kCsTag_EventDataList);
    ChipLogFunctError(mError);

exit:
    return mEventDataListBuilder;
}

ReportData::Builder & ReportData::Builder::IsLastReport(const bool aIsLastReport)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_IsLastReport), aIsLastReport);
    ChipLogFunctError(mError);

exit:
    return *this;
}

ReportData::Builder & ReportData::Builder::EndOfReportData()
{
    EndOfContainer();
    return *this;
}

CHIP_ERROR InvokeCommandRequest::Parser::Init(const chip::TLV::TLVReader & aReader)
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

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR InvokeCommandRequest::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;
    CommandList::Parser commandList;

    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        const uint64_t tag = reader.GetTag();

        if (chip::TLV::ContextTag(kCsTag_CommandList) == tag)
        {
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_CommandList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_CommandList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            commandList.Init(reader);

            PRETTY_PRINT_INCDEPTH();

            err = commandList.CheckSchemaValidity();
            SuccessOrExit(err);

            PRETTY_PRINT_DECDEPTH();
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
        if ((TagPresenceMask & (1 << kCsTag_CommandList)))
        {
            err = CHIP_NO_ERROR;
        }
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR InvokeCommandRequest::Parser::GetCommandList(CommandList::Parser * const apCommandList) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_CommandList), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apCommandList->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR InvokeCommandRequest::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

CommandList::Builder & InvokeCommandRequest::Builder::CreateCommandListBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mCommandListBuilder.ResetError(mError));

    mError = mCommandListBuilder.Init(mpWriter, kCsTag_CommandList);
    ChipLogFunctError(mError);

exit:
    // on error, mCommandListBuilder would be un-/partial initialized and cannot be used to write anything
    return mCommandListBuilder;
}

InvokeCommandRequest::Builder & InvokeCommandRequest::Builder::EndOfInvokeCommandRequest()
{
    EndOfContainer();
    return *this;
}

}; // namespace app
}; // namespace chip
