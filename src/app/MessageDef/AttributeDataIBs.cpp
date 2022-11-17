/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

AttributeDataIBs::Builder & AttributeDataIBs::Builder::EndOfAttributeDataIBs()
{
    EndOfContainer();

    return *this;
}

}; // namespace app
}; // namespace chip
