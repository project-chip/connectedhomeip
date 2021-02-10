/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *      This file defines parser in CHIP interaction model
 *
 */

#include "Parser.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {

Parser::Parser() : mOuterContainerType(chip::TLV::kTLVType_NotSpecified) {}

void Parser::Init(const chip::TLV::TLVReader & aReader, chip::TLV::TLVType aOuterContainerType)
{
    mReader.Init(aReader);
    mOuterContainerType = aOuterContainerType;
}

CHIP_ERROR Parser::GetReaderOnTag(const uint64_t aTagToFind, chip::TLV::TLVReader * const apReader) const
{
    return mReader.FindElementWithTag(aTagToFind, *apReader);
}

void Parser::GetReader(chip::TLV::TLVReader * const apReader)
{
    apReader->Init(mReader);
}
}; // namespace app
}; // namespace chip
