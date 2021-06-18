/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file defines WriteRequest parser and builder in CHIP interaction model
 *
 */

#include "WriteRequest.h"
#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
CHIP_ERROR WriteRequest::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mReader.Init(aReader);

    VerifyOrExit(chip::TLV::kTLVType_Structure == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = mReader.EnterContainer(mOuterContainerType);

exit:
    ChipLogFunctError(err);

    return err;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR WriteRequest::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;
    AttributeDataList::Parser attributeDataList;
    AttributeDataVersionList::Parser attributeDataVersionList;
    PRETTY_PRINT("WriteRequest =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);
        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_SuppressResponse:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_SuppressResponse)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_SuppressResponse);
            VerifyOrExit(chip::TLV::kTLVType_Boolean == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool SuppressResponse;
                err = reader.Get(SuppressResponse);
                SuccessOrExit(err);
                PRETTY_PRINT("\tSuppressResponse = %s, ", SuppressResponse ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_AttributeDataList:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_AttributeDataList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributeDataList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            attributeDataList.Init(reader);

            PRETTY_PRINT_INCDEPTH();

            err = attributeDataList.CheckSchemaValidity();
            SuccessOrExit(err);

            PRETTY_PRINT_DECDEPTH();
            break;
        case kCsTag_AttributeDataVersionList:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_AttributeDataVersionList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributeDataVersionList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            attributeDataVersionList.Init(reader);

            PRETTY_PRINT_INCDEPTH();

            err = attributeDataVersionList.CheckSchemaValidity();
            SuccessOrExit(err);

            PRETTY_PRINT_DECDEPTH();
            break;
        case kCsTag_MoreChunkedMessages:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_MoreChunkedMessages)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_MoreChunkedMessages);
            VerifyOrExit(chip::TLV::kTLVType_Boolean == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool moreChunkedMessages;
                err = reader.Get(moreChunkedMessages);
                SuccessOrExit(err);
                PRETTY_PRINT("\tMoreChunkedMessages = %s, ", moreChunkedMessages ? "true" : "false");
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
    SuccessOrExit(err);
    err = reader.ExitContainer(mOuterContainerType);

exit:
    ChipLogFunctError(err);
    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR WriteRequest::Parser::GetSuppressResponse(bool * const apSuppressResponse) const
{
    return GetSimpleValue(kCsTag_SuppressResponse, chip::TLV::kTLVType_Boolean, apSuppressResponse);
}

CHIP_ERROR WriteRequest::Parser::GetAttributeDataList(AttributeDataList::Parser * const apAttributeDataList) const
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

CHIP_ERROR
WriteRequest::Parser::GetAttributeDataVersionList(AttributeDataVersionList::Parser * const apAttributeDataVersionList) const
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

CHIP_ERROR WriteRequest::Parser::GetMoreChunkedMessages(bool * const apMoreChunkedMessages) const
{
    return GetSimpleValue(kCsTag_MoreChunkedMessages, chip::TLV::kTLVType_Boolean, apMoreChunkedMessages);
}

CHIP_ERROR WriteRequest::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

WriteRequest::Builder & WriteRequest::Builder::SuppressResponse(const bool aSuppressResponse)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_SuppressResponse), aSuppressResponse);
    ChipLogFunctError(mError);

exit:
    return *this;
}

AttributeDataList::Builder & WriteRequest::Builder::CreateAttributeDataListBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributeDataListBuilder.ResetError(mError));

    mError = mAttributeDataListBuilder.Init(mpWriter, kCsTag_AttributeDataList);
    ChipLogFunctError(mError);

exit:
    return mAttributeDataListBuilder;
}

AttributeDataVersionList::Builder & WriteRequest::Builder::CreateAttributeDataVersionListBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributeDataVersionListBuilder.ResetError(mError));

    mError = mAttributeDataVersionListBuilder.Init(mpWriter, kCsTag_AttributeDataVersionList);
    ChipLogFunctError(mError);

exit:
    return mAttributeDataVersionListBuilder;
}

WriteRequest::Builder & WriteRequest::Builder::MoreChunkedMessages(const bool aMoreChunkedMessages)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_MoreChunkedMessages), aMoreChunkedMessages);
    ChipLogFunctError(mError);

exit:
    return *this;
}

AttributeDataList::Builder & WriteRequest::Builder::GetAttributeDataListBuilder()
{
    return mAttributeDataListBuilder;
}

WriteRequest::Builder & WriteRequest::Builder::EndOfWriteRequest()
{
    EndOfContainer();
    return *this;
}
}; // namespace app
}; // namespace chip
