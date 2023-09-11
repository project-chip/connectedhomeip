/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ArrayBuilder.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

namespace chip {
namespace app {
CHIP_ERROR ArrayBuilder::Init(TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse)
{
    mpWriter = apWriter;
    mError   = mpWriter->StartContainer(TLV::ContextTag(aContextTagToUse), chip::TLV::kTLVType_Array, mOuterContainerType);

    return mError;
}

CHIP_ERROR ArrayBuilder::Init(TLV::TLVWriter * const apWriter)
{
    mpWriter = apWriter;
    mError   = mpWriter->StartContainer(TLV::AnonymousTag(), chip::TLV::kTLVType_Array, mOuterContainerType);

    return mError;
}
} // namespace app
} // namespace chip
