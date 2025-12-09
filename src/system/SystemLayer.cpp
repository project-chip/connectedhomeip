/*
 *    Copyright (c) 2021 Project CHIP Authors
 *
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

#include <lib/support/CodeUtils.h>
#include <system/PlatformEventSupport.h>
#include <system/SystemLayer.h>

namespace chip {
namespace System {

CHIP_ERROR Layer::ScheduleLambdaBridge(LambdaBridge && bridge)
{
    CHIP_ERROR lReturn = PlatformEventing::ScheduleLambdaBridge(*this, std::move(bridge));
    if (lReturn != CHIP_NO_ERROR)
    {
        ChipLogError(chipSystemLayer, "Failed to queue CHIP System Layer lambda event: %" CHIP_ERROR_FORMAT, lReturn.Format());
    }
    return lReturn;
}

} // namespace System
} // namespace chip
