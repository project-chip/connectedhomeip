/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "WriteResponseMessage.h"
#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR WriteResponseMessage::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;
    AttributeStatusIBs::Parser writeResponses;
    PRETTY_PRINT("WriteResponseMessage =");
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
        case to_underlying(Tag::kWriteResponses):
            VerifyOrReturnError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
            ReturnErrorOnFailure(writeResponses.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(writeResponses.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
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

CHIP_ERROR WriteResponseMessage::Parser::GetWriteResponses(AttributeStatusIBs::Parser * const apWriteResponses) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kWriteResponses), reader));
    return apWriteResponses->Init(reader);
}

AttributeStatusIBs::Builder & WriteResponseMessage::Builder::CreateWriteResponses()
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mWriteResponses.Init(mpWriter, to_underlying(Tag::kWriteResponses));
    }
    return mWriteResponses;
}

AttributeStatusIBs::Builder & WriteResponseMessage::Builder::GetWriteResponses()
{
    return mWriteResponses;
}

CHIP_ERROR WriteResponseMessage::Builder::EndOfWriteResponseMessage()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = MessageBuilder::EncodeInteractionModelRevision();
    }
    if (mError == CHIP_NO_ERROR)
    {
        EndOfContainer();
    }
    return GetError();
}
} // namespace app
} // namespace chip
