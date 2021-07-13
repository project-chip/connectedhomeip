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

#include "SubscribeResponse.h"
#include "MessageDefHelper.h"

namespace chip {
namespace app {
CHIP_ERROR SubscribeResponse::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    // make a copy of the reader here
    mReader.Init(aReader);

    VerifyOrReturnLogError(chip::TLV::kTLVType_Structure == mReader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

    ReturnLogErrorOnFailure(mReader.EnterContainer(mOuterContainerType));
    return CHIP_NO_ERROR;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR SubscribeResponse::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;
    PRETTY_PRINT("SubscribeResponse =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnLogError(chip::TLV::IsContextTag(reader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_SubscriptionId:
            VerifyOrReturnLogError(!(TagPresenceMask & (1 << kCsTag_SubscriptionId)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_SubscriptionId);
            VerifyOrReturnLogError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint64_t subscriptionId;
                ReturnLogErrorOnFailure(reader.Get(subscriptionId));
                PRETTY_PRINT("\tSubscriptionId = 0x%" PRIx64 ",", subscriptionId);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_FinalSyncInterval:
            VerifyOrReturnLogError(!(TagPresenceMask & (1 << kCsTag_FinalSyncInterval)), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_FinalSyncInterval);
            VerifyOrReturnLogError(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t finalSyncInterval;
                ReturnLogErrorOnFailure(reader.Get(finalSyncInterval));
                PRETTY_PRINT("\tFinalSyncInterval = 0x%" PRIx16 ",", finalSyncInterval);
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
        const uint16_t RequiredFields = (1 << kCsTag_SubscriptionId) | (1 << kCsTag_FinalSyncInterval);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
    }
    ReturnLogErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR SubscribeResponse::Parser::GetSubscriptionId(uint64_t * const apSubscribeId) const
{
    return GetUnsignedInteger(kCsTag_SubscriptionId, apSubscribeId);
}

CHIP_ERROR SubscribeResponse::Parser::GetFinalSyncIntervalSeconds(uint16_t * const apFinalSyncIntervalSeconds) const
{
    return GetUnsignedInteger(kCsTag_FinalSyncInterval, apFinalSyncIntervalSeconds);
}

CHIP_ERROR SubscribeResponse::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

SubscribeResponse::Builder & SubscribeResponse::Builder::SubscriptionId(const uint64_t aSubscribeId)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_SubscriptionId), aSubscribeId);
        ChipLogFunctError(mError);
    }
    return *this;
}

SubscribeResponse::Builder & SubscribeResponse::Builder::FinalSyncIntervalSeconds(const uint16_t aFinalSyncIntervalSeconds)
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_FinalSyncInterval), aFinalSyncIntervalSeconds);
        ChipLogFunctError(mError);
    }
    return *this;
}

SubscribeResponse::Builder & SubscribeResponse::Builder::EndOfSubscribeResponse()
{
    EndOfContainer();
    return *this;
}
} // namespace app
} // namespace chip
