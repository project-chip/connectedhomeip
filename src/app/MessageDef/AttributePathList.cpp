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
 *      This file defines AttributePathList parser and builder in CHIP interaction model
 *
 */

#include "AttributePathList.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributePathList::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    size_t NumPath = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("AttributePathList =");
    PRETTY_PRINT("[");

    // make a copy of the AttributePathList reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(chip::TLV::kTLVType_List == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
        {
            AttributePath::Parser path;
            err = path.Init(reader);
            SuccessOrExit(err);

            PRETTY_PRINT_INCDEPTH();
            err = path.CheckSchemaValidity();
            SuccessOrExit(err);
            PRETTY_PRINT_DECDEPTH();
        }

        ++NumPath;
    }

    PRETTY_PRINT("],");
    PRETTY_PRINT("\t");
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);
    err = reader.ExitContainer(mOuterContainerType);

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

// Re-initialize the shared PathBuilder with anonymous tag
AttributePath::Builder & AttributePathList::Builder::CreateAttributePathBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributePathBuilder.ResetError(mError));

    mError = mAttributePathBuilder.Init(mpWriter);
    ChipLogFunctError(mError);

exit:
    // on error, mAttributePathBuilder would be un-/partial initialized and cannot be used to write anything
    return mAttributePathBuilder;
}

// Mark the end of this array and recover the type for outer container
AttributePathList::Builder & AttributePathList::Builder::EndOfAttributePathList()
{
    EndOfContainer();

    return *this;
}
}; // namespace app
}; // namespace chip
