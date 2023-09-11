/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "StructBuilder.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

namespace chip {
namespace app {
CHIP_ERROR StructBuilder::Init(TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse)
{
    mpWriter = apWriter;
    mError   = mpWriter->StartContainer(TLV::ContextTag(aContextTagToUse), TLV::kTLVType_Structure, mOuterContainerType);
    return mError;
}

CHIP_ERROR StructBuilder::Init(TLV::TLVWriter * const apWriter)
{
    mpWriter = apWriter;
    mError   = mpWriter->StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, mOuterContainerType);
    return mError;
}
} // namespace app
} // namespace chip
