/*
 * SPDX-FileCopyrightText: (c) 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2018 Google LLC.
 * SPDX-FileCopyrightText: (c) 2016-2017 Nest Labs, Inc.
 *
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

AttributeStatusIBs::Builder & AttributeStatusIBs::Builder::EndOfAttributeStatuses()
{
    EndOfContainer();
    return *this;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributeStatusIBs::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    size_t NumAttributeStatus = 0;
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
            ReturnErrorOnFailure(status.CheckSchemaValidity());
            PRETTY_PRINT_DECDEPTH();
        }

        ++NumAttributeStatus;
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
#endif
}; // namespace app
}; // namespace chip
