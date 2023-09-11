/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC.
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ListBuilder.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

namespace chip {
namespace app {
CHIP_ERROR ListBuilder::Init(TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse)
{
    mpWriter = apWriter;
    mError   = mpWriter->StartContainer(TLV::ContextTag(aContextTagToUse), TLV::kTLVType_List, mOuterContainerType);

    return mError;
}

CHIP_ERROR ListBuilder::Init(TLV::TLVWriter * const apWriter)
{
    mpWriter = apWriter;
    mError   = mpWriter->StartContainer(TLV::AnonymousTag(), TLV::kTLVType_List, mOuterContainerType);

    return mError;
}
} // namespace app
} // namespace chip
