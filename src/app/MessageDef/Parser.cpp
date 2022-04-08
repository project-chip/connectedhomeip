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
