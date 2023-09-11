/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
