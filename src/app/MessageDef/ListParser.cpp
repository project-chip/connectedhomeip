/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *     This file defines List parser in CHIP interaction model
 *
 */

#include "ListParser.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
ListParser::ListParser() {}

CHIP_ERROR ListParser::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the reader here
    mReader.Init(aReader);

    VerifyOrExit(chip::TLV::kTLVType_Array == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    // This is just a dummy, as we're not going to exit this container ever
    chip::TLV::TLVType OuterContainerType;
    err = mReader.EnterContainer(OuterContainerType);

exit:
    ChipLogFunctError(err);

    return err;
}

CHIP_ERROR ListParser::InitIfPresent(const chip::TLV::TLVReader & aReader, const uint8_t aContextTagToFind)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(aContextTagToFind), reader);
    SuccessOrExit(err);

    err = Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR ListParser::Next()
{
    CHIP_ERROR err = mReader.Next();

    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}
}; // namespace app
}; // namespace chip
