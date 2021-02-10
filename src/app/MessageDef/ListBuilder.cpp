/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *      This file defines List builder in CHIP interaction model
 *
 */

#include "ListBuilder.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
ListBuilder::ListBuilder() {}

CHIP_ERROR ListBuilder::Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse)
{
    mpWriter = apWriter;
    mError   = mpWriter->StartContainer(chip::TLV::ContextTag(aContextTagToUse), chip::TLV::kTLVType_Array, mOuterContainerType);
    ChipLogFunctError(mError);

    return mError;
}

CHIP_ERROR ListBuilder::Init(chip::TLV::TLVWriter * const apWriter)
{
    mpWriter = apWriter;
    mError   = mpWriter->StartContainer(chip::TLV::AnonymousTag, chip::TLV::kTLVType_Array, mOuterContainerType);
    ChipLogFunctError(mError);

    return mError;
}
}; // namespace app
}; // namespace chip
