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

#include "ListParser.h"

namespace chip {
namespace app {
CHIP_ERROR ListParser::Init(const TLV::TLVReader & aReader)
{
    mReader.Init(aReader);
    VerifyOrReturnError(TLV::kTLVType_List == mReader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    return mReader.EnterContainer(mOuterContainerType);
}
} // namespace app
} // namespace chip
