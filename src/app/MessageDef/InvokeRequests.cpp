/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "InvokeRequests.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR InvokeRequests::Parser::PrettyPrint() const
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    size_t numCommandDatas = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("InvokeRequests =");
    PRETTY_PRINT("[");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == reader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        {
            CommandDataIB::Parser commandData;
            ReturnErrorOnFailure(commandData.Init(reader));
            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(commandData.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }

        ++numCommandDatas;
    }

    PRETTY_PRINT("],");
    PRETTY_PRINT_BLANK_LINE();

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // if we have at least one data element
        if (numCommandDatas > 0)
        {
            err = CHIP_NO_ERROR;
        }
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

CommandDataIB::Builder & InvokeRequests::Builder::CreateCommandData()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mCommandData.Init(mpWriter);
    }
    return mCommandData;
}

CHIP_ERROR InvokeRequests::Builder::EndOfInvokeRequests()
{
    EndOfContainer();
    return GetError();
}
} // namespace app
} // namespace chip
