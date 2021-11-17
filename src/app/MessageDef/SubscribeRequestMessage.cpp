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

#include "SubscribeRequestMessage.h"
#include "MessageDefHelper.h"

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR SubscribeRequestMessage::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;
    AttributePathIBs::Parser AttributePathIBs;
    EventPaths::Parser eventPathList;
    AttributeDataVersionList::Parser attributeDataVersionList;
    PRETTY_PRINT("SubscribeRequestMessage =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(chip::TLV::IsContextTag(reader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_AttributePathList:
            VerifyOrReturnError(!(TagPresenceMask & (1 << kCsTag_AttributePathList)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributePathList);
            VerifyOrReturnError(chip::TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

            AttributePathIBs.Init(reader);

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(AttributePathIBs.CheckSchemaValidity());
            PRETTY_PRINT_DECDEPTH();
            break;
        case kCsTag_EventPaths:
            VerifyOrReturnError(!(TagPresenceMask & (1 << kCsTag_EventPaths)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EventPaths);
            VerifyOrReturnError(chip::TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

            eventPathList.Init(reader);

            PRETTY_PRINT_INCDEPTH();

            ReturnErrorOnFailure(eventPathList.CheckSchemaValidity());

            PRETTY_PRINT_DECDEPTH();
            break;
        case kCsTag_AttributeDataVersionList:
            VerifyOrReturnError(!(TagPresenceMask & (1 << kCsTag_AttributeDataVersionList)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributeDataVersionList);
            VerifyOrReturnError(chip::TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

            attributeDataVersionList.Init(reader);

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(attributeDataVersionList.CheckSchemaValidity());
            PRETTY_PRINT_DECDEPTH();
            break;
        case kCsTag_EventNumber:
            VerifyOrReturnError(!(TagPresenceMask & (1 << kCsTag_EventNumber)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EventNumber);
            VerifyOrReturnError(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint64_t eventNumber;
                ReturnErrorOnFailure(reader.Get(eventNumber));
                PRETTY_PRINT("\tEventNumber = 0x%" PRIx64 ",", eventNumber);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_MinIntervalSeconds:
            VerifyOrReturnError(!(TagPresenceMask & (1 << kCsTag_MinIntervalSeconds)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_MinIntervalSeconds);
            VerifyOrReturnError(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t minIntervalSeconds;
                ReturnErrorOnFailure(reader.Get(minIntervalSeconds));
                PRETTY_PRINT("\tMinIntervalSeconds = 0x%" PRIx16 ",", minIntervalSeconds);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_MaxIntervalSeconds:
            VerifyOrReturnError(!(TagPresenceMask & (1 << kCsTag_MaxIntervalSeconds)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_MaxIntervalSeconds);
            VerifyOrReturnError(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t maxIntervalSeconds;
                ReturnErrorOnFailure(reader.Get(maxIntervalSeconds));
                PRETTY_PRINT("\tkMaxInterval = 0x%" PRIx16 ",", maxIntervalSeconds);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_KeepSubscriptions:
            VerifyOrReturnError(!(TagPresenceMask & (1 << kCsTag_KeepSubscriptions)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_KeepSubscriptions);
            VerifyOrReturnError(chip::TLV::kTLVType_Boolean == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool keepSubscriptions;
                ReturnErrorOnFailure(reader.Get(keepSubscriptions));
                PRETTY_PRINT("\tKeepSubscriptions = %s, ", keepSubscriptions ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_IsProxy:
            VerifyOrReturnError(!(TagPresenceMask & (1 << kCsTag_IsProxy)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_IsProxy);
            VerifyOrReturnError(chip::TLV::kTLVType_Boolean == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                bool isProxy;
                ReturnErrorOnFailure(reader.Get(isProxy));
                PRETTY_PRINT("\tIsProxy = %s, ", isProxy ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            ReturnErrorOnFailure(CHIP_ERROR_INVALID_TLV_TAG);
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
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR SubscribeRequestMessage::Parser::GetPathList(AttributePathIBs::Parser * const apAttributePathList) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_AttributePathList), reader));
    VerifyOrReturnError(chip::TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    return apAttributePathList->Init(reader);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetEventPaths(EventPaths::Parser * const apEventPaths) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_EventPaths), reader));
    VerifyOrReturnError(chip::TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    return apEventPaths->Init(reader);
}

CHIP_ERROR
SubscribeRequestMessage::Parser::GetAttributeDataVersionList(
    AttributeDataVersionList::Parser * const apAttributeDataVersionList) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_AttributeDataVersionList), reader));
    VerifyOrReturnError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    return apAttributeDataVersionList->Init(reader);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetEventNumber(uint64_t * const apEventNumber) const
{
    return GetUnsignedInteger(kCsTag_EventNumber, apEventNumber);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetMinIntervalSeconds(uint16_t * const apMinIntervalSeconds) const
{
    return GetUnsignedInteger(kCsTag_MinIntervalSeconds, apMinIntervalSeconds);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetMaxIntervalSeconds(uint16_t * const apMaxIntervalSeconds) const
{
    return GetUnsignedInteger(kCsTag_MaxIntervalSeconds, apMaxIntervalSeconds);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetKeepSubscriptions(bool * const apKeepExistingSubscription) const
{
    return GetSimpleValue(kCsTag_KeepSubscriptions, chip::TLV::kTLVType_Boolean, apKeepExistingSubscription);
}

CHIP_ERROR SubscribeRequestMessage::Parser::GetIsProxy(bool * const apIsProxy) const
{
    return GetSimpleValue(kCsTag_IsProxy, chip::TLV::kTLVType_Boolean, apIsProxy);
}

AttributePathIBs::Builder & SubscribeRequestMessage::Builder::CreateAttributePathListBuilder()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributePathListBuilder.Init(mpWriter, kCsTag_AttributePathList);
    }

    return mAttributePathListBuilder;
}

EventPaths::Builder & SubscribeRequestMessage::Builder::CreateEventPathsBuilder()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mEventPathsBuilder.Init(mpWriter, kCsTag_EventPaths);
    }

    return mEventPathsBuilder;
}

AttributeDataVersionList::Builder & SubscribeRequestMessage::Builder::CreateAttributeDataVersionListBuilder()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributeDataVersionListBuilder.Init(mpWriter, kCsTag_AttributeDataVersionList);
    }

    return mAttributeDataVersionListBuilder;
}

SubscribeRequestMessage::Builder & SubscribeRequestMessage::Builder::EventNumber(const uint64_t aEventNumber)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_EventNumber), aEventNumber);
    }
    return *this;
}

SubscribeRequestMessage::Builder & SubscribeRequestMessage::Builder::MinIntervalSeconds(const uint16_t aMinIntervalSeconds)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_MinIntervalSeconds), aMinIntervalSeconds);
    }
    return *this;
}

SubscribeRequestMessage::Builder & SubscribeRequestMessage::Builder::MaxIntervalSeconds(const uint16_t aMaxIntervalSeconds)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_MaxIntervalSeconds), aMaxIntervalSeconds);
    }
    return *this;
}

SubscribeRequestMessage::Builder & SubscribeRequestMessage::Builder::KeepSubscriptions(const bool aKeepSubscriptions)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_KeepSubscriptions), aKeepSubscriptions);
    }
    return *this;
}

SubscribeRequestMessage::Builder & SubscribeRequestMessage::Builder::IsProxy(const bool aIsProxy)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(chip::TLV::ContextTag(kCsTag_IsProxy), aIsProxy);
    }
    return *this;
}

SubscribeRequestMessage::Builder & SubscribeRequestMessage::Builder::EndOfSubscribeRequestMessage()
{
    EndOfContainer();
    return *this;
}
} // namespace app
} // namespace chip
