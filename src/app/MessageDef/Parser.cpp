/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC.
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 *    @file
 *      This file defines parser in CHIP interaction model
 *
 */

#include "Parser.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

namespace chip {
namespace app {

Parser::Parser() : mOuterContainerType(chip::TLV::kTLVType_NotSpecified) {}

void Parser::Init(const chip::TLV::TLVReader & aReader, chip::TLV::TLVType aOuterContainerType)
{
    mReader.Init(aReader);
    mOuterContainerType = aOuterContainerType;
}

CHIP_ERROR Parser::GetReaderOnTag(const TLV::Tag aTagToFind, chip::TLV::TLVReader * const apReader) const
{
    return mReader.FindElementWithTag(aTagToFind, *apReader);
}

void Parser::GetReader(chip::TLV::TLVReader * const apReader)
{
    apReader->Init(mReader);
}

CHIP_ERROR Parser::Next()
{
    CHIP_ERROR err = mReader.Next();
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));
    return err;
}
} // namespace app
} // namespace chip
