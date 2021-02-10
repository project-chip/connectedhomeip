/* See Project CHIP LICENSE file for licensing information. */

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
Builder::Builder() : mError(CHIP_ERROR_INCORRECT_STATE), mpWriter(NULL), mOuterContainerType(chip::TLV::kTLVType_NotSpecified) {}

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
    mError              = aErr;
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;
}

void Builder::EndOfContainer()
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->EndContainer(mOuterContainerType);
    SuccessOrExit(mError);

    // we've just closed properly
    // mark it so we do not panic when the build object destructor is called
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;

exit:;
}

CHIP_ERROR Builder::InitAnonymousStructure(chip::TLV::TLVWriter * const apWriter)
{
    mpWriter            = apWriter;
    mOuterContainerType = chip::TLV::kTLVType_NotSpecified;
    mError              = mpWriter->StartContainer(chip::TLV::AnonymousTag, chip::TLV::kTLVType_Structure, mOuterContainerType);
    ChipLogFunctError(mError);

    return mError;
}
}; // namespace app
}; // namespace chip
