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

#include "SubscribeRequest.h"
#include "MessageDefHelper.h"

namespace chip {
namespace app {
CHIP_ERROR SubscribeRequest::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    // make a copy of the reader here
    mReader.Init(aReader);

    VerifyOrReturnLogError(chip::TLV::kTLVType_Structure == mReader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    ReturnLogErrorOnFailure(mReader.EnterContainer(mOuterContainerType));
    return CHIP_NO_ERROR;
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
        VerifyOrReturnLogError(chip::TLV::IsContextTag(reader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_AttributePathList:
            VerifyOrReturnLogError(!(TagPresenceMask & (1 << kCsTag_AttributePathList)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributePathList);
            VerifyOrReturnLogError(chip::TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

            attributePathList.Init(reader);

            PRETTY_PRINT_INCDEPTH();
            ReturnLogErrorOnFailure(attributePathList.CheckSchemaValidity());
            PRETTY_PRINT_DECDEPTH();
            break;
        case kCsTag_EventPathList:
            VerifyOrReturnLogError(!(TagPresenceMask & (1 << kCsTag_EventPathList)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EventPathList);
            VerifyOrReturnLogError(chip::TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

            eventPathList.Init(reader);

            PRETTY_PRINT_INCDEPTH();

            ReturnLogErrorOnFailure(eventPathList.CheckSchemaValidity());

            PRETTY_PRINT_DECDEPTH();
            break;
        case kCsTag_AttributeDataVersionList:
            VerifyOrReturnLogError(!(TagPresenceMask & (1 << kCsTag_AttributeDataVersionList)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributeDataVersionList);
            VerifyOrReturnLogError(chip::TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

            attributeDataVersionList.Init(reader);

            PRETTY_PRINT_INCDEPTH();
            ReturnLogErrorOnFailure(attributeDataVersionList.CheckSchemaValidity());
            PRETTY_PRINT_DECDEPTH();
            break;
        case kCsTag_EventNumber:
            VerifyOrReturnLogError(!(TagPresenceMask & (1 << kCsTag_EventNumber)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EventNumber);
            VerifyOrReturnLogError(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint64_t eventNumber;
                ReturnLogErrorOnFailure(reader.Get(eventNumber));
                PRETTY_PRINT("\tEventNumber = 0x%" PRIx64 ",", eventNumber);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_MinIntervalMs:
            VerifyOrReturnLogError(!(TagPresenceMask & (1 << kCsTag_MinIntervalMs)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_MinIntervalMs);
            VerifyOrReturnLogError(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t minInterval;
                ReturnLogErrorOnFailure(reader.Get(minInterval));
                PRETTY_PRINT("\tMinInterval = 0x%" PRIx16 ",", minInterval);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_MaxIntervalMs:
            VerifyOrReturnLogError(!(TagPresenceMask & (1 << kCsTag_MaxIntervalMs)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_MaxIntervalMs);
            VerifyOrReturnLogError(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t maxInterval;
                ReturnLogErrorOnFailure(reader.Get(maxInterval));
                PRETTY_PRINT("\tkMaxInterval = 0x%" PRIx16 ",", maxInterval);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_KeepExistingSubscriptions:
            VerifyOrReturnLogError(!(TagPresenceMask & (1 << kCsTag_KeepExistingSubscriptions)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_KeepExistingSubscriptions);
            VerifyOrReturnLogError(chip::TLV::kTLVType_Boolean == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool keepExistingSubscriptions;
                ReturnLogErrorOnFailure(reader.Get(keepExistingSubscriptions));
                PRETTY_PRINT("\tKeepExistingSubscriptions = %s, ", keepExistingSubscriptions ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_IsProxy:
            VerifyOrReturnLogError(!(TagPresenceMask & (1 << kCsTag_IsProxy)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_IsProxy);
            VerifyOrReturnLogError(chip::TLV::kTLVType_Boolean == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool isProxy;
                ReturnLogErrorOnFailure(reader.Get(isProxy));
                PRETTY_PRINT("\tIsProxy = %s, ", isProxy ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            ReturnLogErrorOnFailure(CHIP_ERROR_INVALID_TLV_TAG);
        }
    }

    PRETTY_PRINT("}");
    PRETTY_PRINT("");
    if (CHIP_END_OF_TLV == err)
    {
        const uint16_t RequiredFields = (1 << kCsTag_MinIntervalMs) | (1 << kCsTag_MaxIntervalMs);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
    }
    ReturnLogErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR SubscribeRequest::Parser::GetAttributePathList(AttributePathList::Parser * const apAttributePathList) const
{
    TLV::TLVReader reader;
    ReturnLogErrorOnFailure(mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_AttributePathList), reader));
    VerifyOrReturnLogError(chip::TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    return apAttributePathList->Init(reader);
}

CHIP_ERROR SubscribeRequest::Parser::GetEventPathList(EventPathList::Parser * const apEventPathList) const
{
    TLV::TLVReader reader;
    ReturnLogErrorOnFailure(mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_EventPathList), reader));
    VerifyOrReturnLogError(chip::TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    return apEventPathList->Init(reader);
}

CHIP_ERROR
SubscribeRequest::Parser::GetAttributeDataVersionList(AttributeDataVersionList::Parser * const apAttributeDataVersionList) const
{
    TLV::TLVReader reader;
    ReturnLogErrorOnFailure(mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_AttributeDataVersionList), reader));
    VerifyOrReturnLogError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    return apAttributeDataVersionList->Init(reader);
}

CHIP_ERROR SubscribeRequest::Parser::GetEventNumber(uint64_t * const apEventNumber) const
{
    return GetUnsignedInteger(kCsTag_EventNumber, apEventNumber);
}

CHIP_ERROR SubscribeRequest::Parser::GetMinIntervalMs(uint16_t * const apMinIntervalMs) const
{
    return GetUnsignedInteger(kCsTag_EventNumber, apMinIntervalMs);
}

CHIP_ERROR SubscribeRequest::Parser::GetMaxIntervalMs(uint16_t * const apMaxIntervalMs) const
{
    return GetUnsignedInteger(kCsTag_EventNumber, apMaxIntervalMs);
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
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributePathListBuilder.Init(mpWriter, kCsTag_AttributePathList);
        ChipLogFunctError(mError);
    }

    return mAttributePathListBuilder;
}

EventPathList::Builder & SubscribeRequest::Builder::CreateEventPathListBuilder()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mEventPathListBuilder.Init(mpWriter, kCsTag_EventPathList);
        ChipLogFunctError(mError);
    }

    return mEventPathListBuilder;
}

AttributeDataVersionList::Builder & SubscribeRequest::Builder::CreateAttributeDataVersionListBuilder()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributeDataVersionListBuilder.Init(mpWriter, kCsTag_AttributeDataVersionList);
        ChipLogFunctError(mError);
    }

    return mAttributeDataVersionListBuilder;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::EventNumber(const uint64_t aEventNumber)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_EventNumber), aEventNumber);
        ChipLogFunctError(mError);
    }
    return *this;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::MinIntervalMs(const uint16_t aMinIntervalMs)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_MinIntervalMs), aMinIntervalMs);
        ChipLogFunctError(mError);
    }
    return *this;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::MaxIntervalMs(const uint16_t aMaxIntervalMs)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_MaxIntervalMs), aMaxIntervalMs);
        ChipLogFunctError(mError);
    }
    return *this;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::KeepExistingSubscriptions(const bool aKeepExistingSubscriptions)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_KeepExistingSubscriptions), aKeepExistingSubscriptions);
        ChipLogFunctError(mError);
    }
    return *this;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::IsProxy(const bool aIsProxy)
{
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
} // namespace app
} // namespace chip
