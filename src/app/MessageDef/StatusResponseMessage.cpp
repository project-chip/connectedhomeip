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

#include "StatusResponseMessage.h"
#include "MessageDefHelper.h"
#include "protocols/interaction_model/Constants.h"

using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR StatusResponseMessage::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int tagPresenceMask = 0;
    TLV::TLVReader reader;
    PRETTY_PRINT("StatusResponseMessage =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        if (!TLV::IsContextTag(reader.GetTag()))
        {
            continue;
        }
        uint32_t tagNum = TLV::TagNumFromTag(reader.GetTag());
        switch (tagNum)
        {
        case to_underlying(Tag::kStatus):
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kStatus))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kStatus));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint8_t status;
                ReturnErrorOnFailure(reader.Get(status));
                PRETTY_PRINT("\tStatus = " ChipLogFormatIMStatus ",", ChipLogValueIMStatus(static_cast<Status>(status)));
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kInteractionModelRevisionTag:
            ReturnErrorOnFailure(MessageParser::CheckInteractionModelRevision(reader));
            break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }
    PRETTY_PRINT("}");
    PRETTY_PRINT_BLANK_LINE();

    if (CHIP_END_OF_TLV == err)
    {
        const int requiredFields = (1 << to_underlying(Tag::kStatus));
        err =
            (tagPresenceMask & requiredFields) == requiredFields ? CHIP_NO_ERROR : CHIP_ERROR_IM_MALFORMED_STATUS_RESPONSE_MESSAGE;
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR StatusResponseMessage::Parser::GetStatus(Protocols::InteractionModel::Status & aStatus) const
{
    uint16_t status = 0;
    CHIP_ERROR err  = GetUnsignedInteger(to_underlying(Tag::kStatus), &status);
    aStatus         = static_cast<Protocols::InteractionModel::Status>(status);
    return err;
}

StatusResponseMessage::Builder & StatusResponseMessage::Builder::Status(const Protocols::InteractionModel::Status aStatus)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kStatus)), aStatus);
    }
    if (mError == CHIP_NO_ERROR)
    {
        mError = MessageBuilder::EncodeInteractionModelRevision();
    }
    if (mError == CHIP_NO_ERROR)
    {
        EndOfContainer();
    }
    return *this;
}

} // namespace app
} // namespace chip
