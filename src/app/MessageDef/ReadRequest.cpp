/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines ReadRequest parser and builder in CHIP interaction model
 *
 */

#include "ReadRequest.h"
#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
CHIP_ERROR ReadRequest::Parser::Init(const chip::TLV::TLVReader & aReader)
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

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR ReadRequest::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;
    AttributePathList::Parser attributePathList;
    EventPathList::Parser eventPathList;
    AttributeDataVersionList::Parser attributeDataVersionList;
    PRETTY_PRINT("ReadRequest =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        const uint64_t tag = reader.GetTag();

        if (chip::TLV::ContextTag(kCsTag_AttributePathList) == tag)
        {
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_AttributePathList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributePathList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            attributePathList.Init(reader);

            PRETTY_PRINT_INCDEPTH();

            err = attributePathList.CheckSchemaValidity();
            SuccessOrExit(err);

            PRETTY_PRINT_DECDEPTH();
        }
        else if (chip::TLV::ContextTag(kCsTag_EventPathList) == tag)
        {
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_EventPathList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EventPathList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            eventPathList.Init(reader);

            PRETTY_PRINT_INCDEPTH();

            err = eventPathList.CheckSchemaValidity();
            SuccessOrExit(err);

            PRETTY_PRINT_DECDEPTH();
        }
        else if (chip::TLV::ContextTag(kCsTag_AttributeDataVersionList) == tag)
        {
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_AttributeDataVersionList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributeDataVersionList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            attributeDataVersionList.Init(reader);

            PRETTY_PRINT_INCDEPTH();

            err = attributeDataVersionList.CheckSchemaValidity();
            SuccessOrExit(err);

            PRETTY_PRINT_DECDEPTH();
        }
        else if (chip::TLV::ContextTag(kCsTag_AttributePathList) == tag)
        {
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_AttributePathList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributePathList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            attributePathList.Init(reader);

            PRETTY_PRINT_INCDEPTH();

            err = attributePathList.CheckSchemaValidity();
            SuccessOrExit(err);

            PRETTY_PRINT_DECDEPTH();
        }
        else if (chip::TLV::ContextTag(kCsTag_EventNumber) == tag)
        {
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_EventNumber)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EventNumber);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint64_t eventNumber;
                err = reader.Get(eventNumber);
                SuccessOrExit(err);
                PRETTY_PRINT("\tEventNumber = 0x%" PRIx64 ",", eventNumber);
            }
#endif // CHIP_DETAIL_LOGGING
        }
    }

    PRETTY_PRINT("}");
    PRETTY_PRINT("");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);
    err = reader.ExitContainer(mOuterContainerType);

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR ReadRequest::Parser::GetAttributePathList(AttributePathList::Parser * const apAttributePathList) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_AttributePathList), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apAttributePathList->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR ReadRequest::Parser::GetEventPathList(EventPathList::Parser * const apEventPathList) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_EventPathList), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apEventPathList->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR
ReadRequest::Parser::GetAttributeDataVersionList(AttributeDataVersionList::Parser * const apAttributeDataVersionList) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_AttributeDataVersionList), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apAttributeDataVersionList->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR ReadRequest::Parser::GetEventNumber(uint64_t * const apEventNumber) const
{
    return GetUnsignedInteger(kCsTag_EventNumber, apEventNumber);
}

CHIP_ERROR ReadRequest::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

AttributePathList::Builder & ReadRequest::Builder::CreateAttributePathListBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributePathListBuilder.ResetError(mError));

    mError = mAttributePathListBuilder.Init(mpWriter, kCsTag_AttributePathList);
    ChipLogFunctError(mError);

exit:
    // on error, mAttributePathListBuilder would be un-/partial initialized and cannot be used to write anything
    return mAttributePathListBuilder;
}

EventPathList::Builder & ReadRequest::Builder::CreateEventPathListBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mEventPathListBuilder.ResetError(mError));

    mError = mEventPathListBuilder.Init(mpWriter, kCsTag_EventPathList);
    ChipLogFunctError(mError);

exit:
    // on error, mEventPathListBuilder would be un-/partial initialized and cannot be used to write anything
    return mEventPathListBuilder;
}

AttributeDataVersionList::Builder & ReadRequest::Builder::CreateAttributeDataVersionListBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributeDataVersionListBuilder.ResetError(mError));

    mError = mAttributeDataVersionListBuilder.Init(mpWriter, kCsTag_AttributeDataVersionList);
    ChipLogFunctError(mError);

exit:
    // on error, mAttributeDataVersionListBuilder would be un-/partial initialized and cannot be used to write anything
    return mAttributeDataVersionListBuilder;
}

ReadRequest::Builder & ReadRequest::Builder::EventNumber(const uint64_t aEventNumber)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_EventNumber), aEventNumber);
        ChipLogFunctError(mError);
    }
    return *this;
}

ReadRequest::Builder & ReadRequest::Builder::EndOfReadRequest()
{
    EndOfContainer();
    return *this;
}
}; // namespace app
}; // namespace chip
