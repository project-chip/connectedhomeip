/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC.
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 *    @file
 *      This file defines AttributeDataIBs parser and builder in CHIP interaction model
 *
 */

#include "AttributeDataIBs.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR AttributeDataIBs::Parser::PrettyPrint() const
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    size_t numDataElement = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("AttributeDataIBs =");
    PRETTY_PRINT("[");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == reader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

        {
            AttributeDataIB::Parser data;
            ReturnErrorOnFailure(data.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(data.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }

        ++numDataElement;
    }

    PRETTY_PRINT("],");
    PRETTY_PRINT_BLANK_LINE();

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // if we have at least one data element
        if (numDataElement > 0)
        {
            err = CHIP_NO_ERROR;
        }
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

AttributeDataIB::Builder & AttributeDataIBs::Builder::CreateAttributeDataIBBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributeDataIBBuilder.ResetError(mError));

    mError = mAttributeDataIBBuilder.Init(mpWriter);

exit:

    // on error, mAttributeDataIBBuilder would be un-/partial initialized and cannot be used to write anything
    return mAttributeDataIBBuilder;
}

AttributeDataIB::Builder & AttributeDataIBs::Builder::GetAttributeDataIBBuilder()
{
    return mAttributeDataIBBuilder;
}

CHIP_ERROR AttributeDataIBs::Builder::EndOfAttributeDataIBs()
{
    EndOfContainer();

    return GetError();
}

}; // namespace app
}; // namespace chip
