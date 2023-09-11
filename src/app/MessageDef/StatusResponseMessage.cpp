/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "StatusResponseMessage.h"
#include "MessageDefHelper.h"
#include "protocols/interaction_model/Constants.h"

using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR StatusResponseMessage::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
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
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

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
        mError = mpWriter->Put(TLV::ContextTag(Tag::kStatus), aStatus);
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
