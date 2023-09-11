/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "TimedRequestMessage.h"
#include "MessageDefHelper.h"

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR TimedRequestMessage::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;
    PRETTY_PRINT("TimedRequestMessage =");
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
        case to_underlying(Tag::kTimeoutMs):
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t timeout;
                ReturnErrorOnFailure(reader.Get(timeout));
                PRETTY_PRINT("\tTimeoutMs = 0x%x,", timeout);
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

CHIP_ERROR TimedRequestMessage::Parser::GetTimeoutMs(uint16_t * const apTimeoutMs) const
{
    return GetUnsignedInteger(to_underlying(Tag::kTimeoutMs), apTimeoutMs);
}

TimedRequestMessage::Builder & TimedRequestMessage::Builder::TimeoutMs(const uint16_t aTimeoutMs)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(Tag::kTimeoutMs), aTimeoutMs);
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
