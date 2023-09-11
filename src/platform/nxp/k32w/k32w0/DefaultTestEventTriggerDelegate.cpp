/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "DefaultTestEventTriggerDelegate.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {

bool DefaultTestEventTriggerDelegate::DoesEnableKeyMatch(const ByteSpan & enableKey) const
{
    return !mEnableKey.empty() && mEnableKey.data_equal(enableKey);
}

CHIP_ERROR DefaultTestEventTriggerDelegate::HandleEventTrigger(uint64_t eventTrigger)
{
    if (eventTrigger == kQueryTrigger)
    {
        ChipLogProgress(DeviceLayer, "DefaultTestEventTriggerDelegate: event triggered");
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

} // namespace chip
