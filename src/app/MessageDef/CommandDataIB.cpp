/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC.
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "CommandDataIB.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR CommandDataIB::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;

    PRETTY_PRINT("CommandDataIB =");
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
        case to_underlying(Tag::kPath): {
            CommandPathIB::Parser path;
            ReturnErrorOnFailure(path.Init(reader));
            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(path.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }

        break;
        case to_underlying(Tag::kFields):
            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(CheckIMPayload(reader, 0, "CommandFields"));
            PRETTY_PRINT_DECDEPTH();
            break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }

    PRETTY_PRINT("},");
    PRETTY_PRINT_BLANK_LINE();

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

CHIP_ERROR CommandDataIB::Parser::GetPath(CommandPathIB::Parser * const apPath) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kPath), reader));
    return apPath->Init(reader);
}

CHIP_ERROR CommandDataIB::Parser::GetFields(TLV::TLVReader * const apReader) const
{
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kFields), *apReader));
    return CHIP_NO_ERROR;
}

CommandPathIB::Builder & CommandDataIB::Builder::CreatePath()
{
    mError = mPath.Init(mpWriter, to_underlying(Tag::kPath));
    return mPath;
}

CHIP_ERROR CommandDataIB::Builder::EndOfCommandDataIB()
{
    EndOfContainer();
    return GetError();
}
} // namespace app
} // namespace chip
