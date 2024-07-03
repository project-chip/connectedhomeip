/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "SilabsTestEventTriggerDelegate.h"
#include <ProvisionManager.h>

using namespace ::chip::DeviceLayer;

namespace chip {

bool SilabsTestEventTriggerDelegate::DoesEnableKeyMatch(const ByteSpan & enableKey) const
{
    uint8_t storedEnableKey[TestEventTriggerDelegate::kEnableKeyLength];
    MutableByteSpan enableKeySpan(storedEnableKey);

    // Return false if we were not able to get the enableKey
    VerifyOrReturnValue(
        Silabs::Provision::Manager::GetInstance().GetStorage().GetTestEventTriggerKey(enableKeySpan) == CHIP_NO_ERROR, false);

    return (!enableKeySpan.empty() && enableKeySpan.data_equal(enableKey));
}

} // namespace chip
