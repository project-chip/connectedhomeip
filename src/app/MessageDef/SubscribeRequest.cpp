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
        case kCsTag_MinIntervalSeconds:
            VerifyOrReturnLogError(!(TagPresenceMask & (1 << kCsTag_MinIntervalSeconds)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_MinIntervalSeconds);
            VerifyOrReturnLogError(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t minIntervalSeconds;
                ReturnLogErrorOnFailure(reader.Get(minIntervalSeconds));
                PRETTY_PRINT("\tMinIntervalSeconds = 0x%" PRIx16 ",", minIntervalSeconds);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_MaxIntervalSeconds:
            VerifyOrReturnLogError(!(TagPresenceMask & (1 << kCsTag_MaxIntervalSeconds)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_MaxIntervalSeconds);
            VerifyOrReturnLogError(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t maxIntervalSeconds;
                ReturnLogErrorOnFailure(reader.Get(maxIntervalSeconds));
                PRETTY_PRINT("\tkMaxInterval = 0x%" PRIx16 ",", maxIntervalSeconds);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_KeepSubscriptions:
            VerifyOrReturnLogError(!(TagPresenceMask & (1 << kCsTag_KeepSubscriptions)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_KeepSubscriptions);
            VerifyOrReturnLogError(chip::TLV::kTLVType_Boolean == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool keepSubscriptions;
                ReturnLogErrorOnFailure(reader.Get(keepSubscriptions));
                PRETTY_PRINT("\tKeepSubscriptions = %s, ", keepSubscriptions ? "true" : "false");
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
        const uint16_t RequiredFields = (1 << kCsTag_MinIntervalSeconds) | (1 << kCsTag_MaxIntervalSeconds);

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

CHIP_ERROR SubscribeRequest::Parser::GetMinIntervalSeconds(uint16_t * const apMinIntervalSeconds) const
{
    return GetUnsignedInteger(kCsTag_MinIntervalSeconds, apMinIntervalSeconds);
}

CHIP_ERROR SubscribeRequest::Parser::GetMaxIntervalSeconds(uint16_t * const apMaxIntervalSeconds) const
{
    return GetUnsignedInteger(kCsTag_MaxIntervalSeconds, apMaxIntervalSeconds);
}

CHIP_ERROR SubscribeRequest::Parser::GetKeepSubscriptions(bool * const apKeepExistingSubscription) const
{
    return GetSimpleValue(kCsTag_KeepSubscriptions, chip::TLV::kTLVType_Boolean, apKeepExistingSubscription);
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
    }

    return mAttributePathListBuilder;
}

EventPathList::Builder & SubscribeRequest::Builder::CreateEventPathListBuilder()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mEventPathListBuilder.Init(mpWriter, kCsTag_EventPathList);
    }

    return mEventPathListBuilder;
}

AttributeDataVersionList::Builder & SubscribeRequest::Builder::CreateAttributeDataVersionListBuilder()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributeDataVersionListBuilder.Init(mpWriter, kCsTag_AttributeDataVersionList);
    }

    return mAttributeDataVersionListBuilder;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::EventNumber(const uint64_t aEventNumber)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_EventNumber), aEventNumber);
    }
    return *this;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::MinIntervalSeconds(const uint16_t aMinIntervalSeconds)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_MinIntervalSeconds), aMinIntervalSeconds);
    }
    return *this;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::MaxIntervalSeconds(const uint16_t aMaxIntervalSeconds)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_MaxIntervalSeconds), aMaxIntervalSeconds);
    }
    return *this;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::KeepSubscriptions(const bool aKeepSubscriptions)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_KeepSubscriptions), aKeepSubscriptions);
    }
    return *this;
}

SubscribeRequest::Builder & SubscribeRequest::Builder::IsProxy(const bool aIsProxy)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_IsProxy), aIsProxy);
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
