/**
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC.
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AttributeStatusIBs.h"
#include "AttributeStatusIB.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
AttributeStatusIB::Builder & AttributeStatusIBs::Builder::CreateAttributeStatus()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributeStatus.Init(mpWriter);
    }
    return mAttributeStatus;
}

CHIP_ERROR AttributeStatusIBs::Builder::EndOfAttributeStatuses()
{
    EndOfContainer();
    return GetError();
}

#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR AttributeStatusIBs::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;

    PRETTY_PRINT("AttributeStatusIBs =");
    PRETTY_PRINT("[");

    // make a copy of the AttributeStatusIBs reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == reader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        {
            AttributeStatusIB::Parser status;
            ReturnErrorOnFailure(status.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(status.PrettyPrint());
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
};     // namespace app
};     // namespace chip
