/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "EnergyEvseTestEventTriggerDelegate.h"

using namespace chip::app::Clusters::EnergyEvse;

namespace chip {

bool EnergyEvseTestEventTriggerDelegate::DoesEnableKeyMatch(const ByteSpan & enableKey) const
{
    return !mEnableKey.empty() && mEnableKey.data_equal(enableKey);
}

CHIP_ERROR EnergyEvseTestEventTriggerDelegate::HandleEventTrigger(uint64_t eventTrigger)
{
    if (HandleEnergyEvseTestEventTrigger(eventTrigger))
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
