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
