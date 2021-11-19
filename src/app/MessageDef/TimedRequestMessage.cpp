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

#include "TimedRequestMessage.h"
#include "MessageDefHelper.h"

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR TimedRequestMessage::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int TagPresenceMask = 0;
    TLV::TLVReader reader;
    PRETTY_PRINT("TimedRequestMessage =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::IsContextTag(reader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
        switch (TLV::TagNumFromTag(reader.GetTag()))
        {
        case to_underlying(Tag::kTimeoutMs):
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kTimeoutMs))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kTimeoutMs));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t timeout;
                ReturnErrorOnFailure(reader.Get(timeout));
                PRETTY_PRINT("\tTimeoutMs = 0x%" PRIx8 ",", timeout);
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
        const int RequiredFields = (1 << to_underlying(Tag::kTimeoutMs));

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR TimedRequestMessage::Parser::GetTimeoutMs(uint16_t * const apTimeoutMs) const
{
    return GetUnsignedInteger(to_underlying(Tag::kTimeoutMs), apTimeoutMs);
}

TimedRequestMessage::Builder & TimedRequestMessage::Builder::TimeoutMs(const uint16_t aTimeoutMs)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kTimeoutMs)), aTimeoutMs);
    }
    EndOfContainer();
    return *this;
}
} // namespace app
} // namespace chip
