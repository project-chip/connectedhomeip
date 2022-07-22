/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

#include "OTATestEventTriggerDelegate.h"

#include "OTARequestorInterface.h"

#include <lib/support/CodeUtils.h>

namespace chip {

bool OTATestEventTriggerDelegate::DoesEnableKeyMatch(const ByteSpan & enableKey) const
{
    return !mEnableKey.empty() && mEnableKey.data_equal(enableKey);
}

CHIP_ERROR OTATestEventTriggerDelegate::HandleEventTrigger(uint64_t eventTrigger)
{
    if ((eventTrigger & ~kOtaQueryFabricIndexMask) == kOtaQueryTrigger)
    {
        OTARequestorInterface * requestor = GetRequestorInstance();
        const FabricIndex fabricIndex     = eventTrigger & kOtaQueryFabricIndexMask;

        VerifyOrReturnError(requestor != nullptr, CHIP_ERROR_INCORRECT_STATE);
        return requestor->TriggerImmediateQuery(fabricIndex);
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

} // namespace chip
