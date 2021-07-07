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

#include "SubscribeRequest.h"
#include "MessageDefHelper.h"

namespace chip {
namespace app {
CHIP_ERROR SubscribeRequest::Parser::Init(const chip::TLV::TLVReader & aReader)
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
CHIP_ERROR SubscribeRequest::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;
    AttributePathList::Parser attributePathList;
    EventPathList::Parser eventPathList;
    AttributeDataVersionList::Parser attributeDataVersionList;
    PRETTY_PRINT("SubscribeRequest =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);
        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_AttributePathList:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_AttributePathList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributePathList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            attributePathList.Init(reader);

            PRETTY_PRINT_INCDEPTH();

            err = attributePathList.CheckSchemaValidity();
            SuccessOrExit(err);

            PRETTY_PRINT_DECDEPTH();
            break;
        case kCsTag_EventPathList:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_EventPathList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EventPathList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            eventPathList.Init(reader);

            PRETTY_PRINT_INCDEPTH();

            err = eventPathList.CheckSchemaValidity();
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
        case kCsTag_EventNumber:
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
            break;
        case kCsTag_MinInterval:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_MinInterval)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_MinInterval);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t minInterval;
                err = reader.Get(minInterval);
                SuccessOrExit(err);
                PRETTY_PRINT("\tMinInterval = 0x%" PRIx16 ",", minInterval);
            }
            break;
#endif // CHIP_DETAIL_LOGGING
        case kCsTag_MaxInterval:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_MaxInterval)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_MaxInterval);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t maxInterval;
                err = reader.Get(maxInterval);
                SuccessOrExit(err);
                PRETTY_PRINT("\tkMaxInterval = 0x%" PRIx16 ",", maxInterval);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_KeepExistingSubscriptions:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_KeepExistingSubscriptions)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_KeepExistingSubscriptions);
            VerifyOrExit(chip::TLV::kTLVType_Boolean == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool keepExistingSubscriptions;
                err = reader.Get(keepExistingSubscriptions);
                SuccessOrExit(err);
                PRETTY_PRINT("\tKeepExistingSubscriptions = %s, ", keepExistingSubscriptions ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_IsProxy:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_IsProxy)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_IsProxy);
            VerifyOrExit(chip::TLV::kTLVType_Boolean == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool isProxy;
                err = reader.Get(isProxy);
                SuccessOrExit(err);
                PRETTY_PRINT("\tIsProxy = %s, ", isProxy ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            ExitNow(err = CHIP_ERROR_INVALID_TLV_TAG);
        }
    }

    PRETTY_PRINT("}");
    PRETTY_PRINT("");
    if (CHIP_END_OF_TLV == err)
    {
        const uint16_t RequiredFields = (1 << kCsTag_MinInterval) | (1 << kCsTag_MaxInterval);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
    }
    SuccessOrExit(err);
    err = reader.ExitContainer(mOuterContainerType);

exit:
    ChipLogFunctError(err);
    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR SubscribeRequest::Parser::GetAttributePathList(AttributePathList::Parser * const apAttributePathList) const
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

CHIP_ERROR SubscribeRequest::Parser::GetEventPathList(EventPathList::Parser * const apEventPathList) const
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
SubscribeRequest::Parser::GetAttributeDataVersionList(AttributeDataVersionList::Parser * const apAttributeDataVersionList) const
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

CHIP_ERROR SubscribeRequest::Parser::GetEventNumber(uint64_t * const apEventNumber) const
{
    return GetUnsignedInteger(kCsTag_EventNumber, apEventNumber);
}

CHIP_ERROR SubscribeRequest::Parser::GetMinInterval(uint16_t * const apMinInterval) const
{
    return GetUnsignedInteger(kCsTag_EventNumber, apMinInterval);
}

CHIP_ERROR SubscribeRequest::Parser::GetMaxInterval(uint16_t * const apMinInterval) const
{
    return GetUnsignedInteger(kCsTag_EventNumber, apMinInterval);
}

CHIP_ERROR SubscribeRequest::Parser::GetKeepExistingSubscriptions(bool * const apKeepExistingSubscription) const
{
    return GetSimpleValue(kCsTag_KeepExistingSubscriptions, chip::TLV::kTLVType_Boolean, apKeepExistingSubscription);
}

CHIP_ERROR SubscribeRequest::Parser::GetIsProxy(bool * const apIsProxy) const
{
    return GetSimpleValue(kCsTag_IsProxy, chip::TLV::kTLVType_Boolean, apIsProxy);
}

CHIP_ERROR SubscribeRequest::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

AttributePathList::Builder & SubscribeRequest::Builder::CreateAttributePathListBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributePathListBuilder.ResetError(mError));

    mError = mAttributePathListBuilder.Init(mpWriter, kCsTag_AttributePathList);
    ChipLogFunctError(mError);

exit:
    // on error, mAttributePathListBuilder would be un-/partial initialized and cannot be used to write anything
    return mAttributePathListBuilder;
}

EventPathList::Builder & SubscribeRequest::Builder::CreateEventPathListBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mEventPathListBuilder.ResetError(mError));

    mError = mEventPathListBuilder.Init(mpWriter, kCsTag_EventPathList);
    ChipLogFunctError(mError);

exit:
    // on error, mEventPathListBuilder would be un-/partial initialized and cannot be used to write anything
    return mEventPathListBuilder;
}

AttributeDataVersionList::Builder & SubscribeRequest::Builder::CreateAttributeDataVersionListBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributeDataVersionListBuilder.ResetError(mError));

    mError = mAttributeDataVersionListBuilder.Init(mpWriter, kCsTag_AttributeDataVersionList);
    ChipLogFunctError(mError);

exit:
    // on error, mAttributeDataVersionListBuilder would be un-/partial initialized and cannot be used to write anything
    return mAttributeDataVersionListBuilder;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::EventNumber(const uint64_t aEventNumber)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_EventNumber), aEventNumber);
        ChipLogFunctError(mError);
    }
    return *this;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::MinInterval(const uint16_t aMinInterval)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_MinInterval), aMinInterval);
        ChipLogFunctError(mError);
    }
    return *this;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::MaxInterval(const uint16_t aMaxInterval)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_MaxInterval), aMaxInterval);
        ChipLogFunctError(mError);
    }
    return *this;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::KeepExistingSubscriptions(const bool aKeepExistingSubscriptions)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_KeepExistingSubscriptions), aKeepExistingSubscriptions);
        ChipLogFunctError(mError);
    }
    return *this;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::IsProxy(const bool aIsProxy)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_IsProxy), aIsProxy);
        ChipLogFunctError(mError);
    }
    return *this;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::EndOfSubscribeRequest()
{
    EndOfContainer();
    return *this;
}
}; // namespace app
}; // namespace chip
