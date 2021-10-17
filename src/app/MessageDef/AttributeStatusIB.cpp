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
 *      This file defines AttributeStatusIB parser and builder in CHIP interaction model
 *
 */

#include "AttributeStatusIB.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
CHIP_ERROR AttributeStatusIB::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

AttributePath::Builder & AttributeStatusIB::Builder::CreateAttributePathBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributePathBuilder.ResetError(mError));

    mError = mAttributePathBuilder.Init(mpWriter, kCsTag_AttributePath);

exit:
    return mAttributePathBuilder;
}

StatusIB::Builder & AttributeStatusIB::Builder::CreateStatusIBBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mStatusIBBuilder.ResetError(mError));

    mError = mStatusIBBuilder.Init(mpWriter, kCsTag_StatusIB);

exit:
    return mStatusIBBuilder;
}

AttributeStatusIB::Builder & AttributeStatusIB::Builder::EndOfAttributeStatusIB()
{
    EndOfContainer();
    return *this;
}

CHIP_ERROR AttributeStatusIB::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the reader here
    mReader.Init(aReader);
    VerifyOrExit(chip::TLV::kTLVType_Structure == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = mReader.EnterContainer(mOuterContainerType);

exit:
    return err;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributeStatusIB::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("AttributeStatusIB =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);
        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_AttributePath:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_AttributePath)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_AttributePath);
            {
                AttributePath::Parser path;
                err = path.Init(reader);
                SuccessOrExit(err);

                PRETTY_PRINT_INCDEPTH();
                err = path.CheckSchemaValidity();
                SuccessOrExit(err);
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case kCsTag_StatusIB:
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_StatusIB)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_StatusIB);
            {
                StatusIB::Parser status;
                err = status.Init(reader);
                SuccessOrExit(err);

                PRETTY_PRINT_INCDEPTH();
                err = status.CheckSchemaValidity();
                SuccessOrExit(err);
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        default:
            ExitNow(err = CHIP_ERROR_INVALID_TLV_TAG);
        }
    }

    PRETTY_PRINT("},");
    PRETTY_PRINT("");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const uint16_t RequiredFields = (1 << kCsTag_AttributePath) | (1 << kCsTag_StatusIB);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_STATUS_ELEMENT;
        }
    }
    SuccessOrExit(err);
    err = reader.ExitContainer(mOuterContainerType);

exit:

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR AttributeStatusIB::Parser::GetAttributePath(AttributePath::Parser * const apAttributePath) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_AttributePath), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_List == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apAttributePath->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR AttributeStatusIB::Parser::GetStatusIB(StatusIB::Parser * const apStatusIB) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_StatusIB), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Structure == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apStatusIB->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}
}; // namespace app
}; // namespace chip
