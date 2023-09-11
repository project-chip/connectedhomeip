/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC.
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 *    @file
 *      This file defines builder in CHIP interaction model
 *
 */

#include "Builder.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
Builder::Builder() : mError(CHIP_ERROR_INCORRECT_STATE), mpWriter(nullptr), mOuterContainerType(chip::TLV::kTLVType_NotSpecified) {}

void Builder::Init(chip::TLV::TLVWriter * const apWriter, chip::TLV::TLVType aOuterContainerType)
{
    mpWriter            = apWriter;
    mOuterContainerType = aOuterContainerType;
}

void Builder::ResetError()
{
    ResetError(CHIP_NO_ERROR);
}

void Builder::ResetError(CHIP_ERROR aErr)
{
    mError = aErr;
}

void Builder::EndOfContainer()
{
    // skip if error has already been set
    ReturnOnFailure(mError);
    ReturnOnFailure(mError = mpWriter->EndContainer(mOuterContainerType));
    // we've just closed properly
    // mark it so we do not panic when the build object destructor is called
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;
}
} // namespace app
} // namespace chip
