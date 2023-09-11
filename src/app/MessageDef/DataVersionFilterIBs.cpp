/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "DataVersionFilterIBs.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR DataVersionFilterIBs::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;

    PRETTY_PRINT("DataVersionFilterIBs =");
    PRETTY_PRINT("[");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == reader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        {
            DataVersionFilterIB::Parser DataVersionFilter;
            ReturnErrorOnFailure(DataVersionFilter.Init(reader));
            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(DataVersionFilter.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }
    }

    PRETTY_PRINT("],");
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

DataVersionFilterIB::Builder & DataVersionFilterIBs::Builder::CreateDataVersionFilter()
{
    mError = mDataVersionFilter.Init(mpWriter);
    return mDataVersionFilter;
}

CHIP_ERROR DataVersionFilterIBs::Builder::EndOfDataVersionFilterIBs()
{
    EndOfContainer();
    return GetError();
}
} // namespace app
} // namespace chip
