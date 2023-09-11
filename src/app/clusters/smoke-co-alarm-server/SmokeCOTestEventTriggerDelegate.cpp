/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "SmokeCOTestEventTriggerDelegate.h"

using namespace chip::app::Clusters::SmokeCoAlarm;

namespace chip {

bool SmokeCOTestEventTriggerDelegate::DoesEnableKeyMatch(const ByteSpan & enableKey) const
{
    return !mEnableKey.empty() && mEnableKey.data_equal(enableKey);
}

CHIP_ERROR SmokeCOTestEventTriggerDelegate::HandleEventTrigger(uint64_t eventTrigger)
{
    if (HandleSmokeCOTestEventTrigger(eventTrigger))
    {
        return CHIP_NO_ERROR;
    }
    if (mOtherDelegate != nullptr)
    {
        return mOtherDelegate->HandleEventTrigger(eventTrigger);
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}

} // namespace chip
