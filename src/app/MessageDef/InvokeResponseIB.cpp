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

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include "InvokeResponseIB.h"
#include "MessageDefHelper.h"

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR InvokeResponseIB::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;

    PRETTY_PRINT("InvokeResponseIB =");
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
        case to_underlying(Tag::kCommand): {
            CommandDataIB::Parser command;
            ReturnErrorOnFailure(command.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(command.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }
        break;
        case to_underlying(Tag::kStatus): {
            CommandStatusIB::Parser status;
            ReturnErrorOnFailure(status.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(status.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }
        break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }

    PRETTY_PRINT("},");
    PRETTY_PRINT_BLANK_LINE();

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

CHIP_ERROR InvokeResponseIB::Parser::GetCommand(CommandDataIB::Parser * const apCommand) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kCommand), reader));
    return apCommand->Init(reader);
}

CHIP_ERROR InvokeResponseIB::Parser::GetStatus(CommandStatusIB::Parser * const apStatus) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kStatus), reader));
    return apStatus->Init(reader);
}

CommandDataIB::Builder & InvokeResponseIB::Builder::CreateCommand()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mCommand.Init(mpWriter, to_underlying(Tag::kCommand));
    }
    return mCommand;
}

CommandStatusIB::Builder & InvokeResponseIB::Builder::CreateStatus()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mStatus.Init(mpWriter, to_underlying(Tag::kStatus));
    }
    return mStatus;
}

CHIP_ERROR InvokeResponseIB::Builder::EndOfInvokeResponseIB()
{
    EndOfContainer();
    return GetError();
}
} // namespace app
} // namespace chip
