/**
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC.
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AttributePathIBs.h"
#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR AttributePathIBs::Parser::PrettyPrint() const
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    size_t numAttributePath = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("AttributePathIBs =");
    PRETTY_PRINT("[");

    // make a copy of the AttributePathIBs reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == reader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrReturnError(TLV::kTLVType_List == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
        {
            AttributePathIB::Parser path;
            ReturnErrorOnFailure(path.Init(reader));
            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(path.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }

        ++numAttributePath;
    }

    PRETTY_PRINT("],");
    PRETTY_PRINT("\t");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // if we have at least one data element
        if (numAttributePath > 0)
        {
            err = CHIP_NO_ERROR;
        }
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

AttributePathIB::Builder & AttributePathIBs::Builder::CreatePath()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributePath.Init(mpWriter);
    }
    return mAttributePath;
}

// Mark the end of this array and recover the type for outer container
CHIP_ERROR AttributePathIBs::Builder::EndOfAttributePathIBs()
{
    EndOfContainer();
    return GetError();
}
} // namespace app
} // namespace chip
