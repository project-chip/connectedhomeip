/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "SilabsTestEventTriggerDelegate.h"

using namespace ::chip::DeviceLayer;

namespace chip {

bool SilabsTestEventTriggerDelegate::DoesEnableKeyMatch(const ByteSpan & enableKey) const
{
    return !mEnableKey.empty() && mEnableKey.data_equal(enableKey);
}

CHIP_ERROR SilabsTestEventTriggerDelegate::HandleEventTrigger(uint64_t eventTrigger)
{
    bool success = emberAfHandleEventTrigger(eventTrigger);
    return success ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT;
}

} // namespace chip
