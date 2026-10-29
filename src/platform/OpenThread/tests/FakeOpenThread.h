/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <openthread/instance.h>
#include <openthread/link.h>
#include <openthread/thread.h>

#include <vector>

namespace chip {
namespace Testing {

// State behind the OpenThread API fakes defined in FakeOpenThread.cpp.
struct FakeOpenThreadState
{
    otLinkModeConfig linkMode      = {};
    bool ip6Enabled                = true;
    otError ip6EnableResult        = OT_ERROR_NONE;
    otError ip6DisableResult       = OT_ERROR_NONE;
    otError discoverResult         = OT_ERROR_NONE;
    otError threadSetEnabledResult = OT_ERROR_NONE;
    otDeviceRole role              = OT_DEVICE_ROLE_DISABLED;
    bool commissioned              = false;
    unsigned discoverCalls         = 0;
    int threadStackLockDepth       = 0;
    unsigned threadStackLockCount  = 0;

    // Consumed front to back by otThreadSetLinkMode; empty means OT_ERROR_NONE.
    std::vector<otError> setLinkModeResults;
    std::vector<otLinkModeConfig> setLinkModeLog;
    std::vector<int> setLinkModeLockDepth;
    std::vector<bool> ip6SetEnabledLog;
    std::vector<int> ip6SetEnabledLockDepth;
};

FakeOpenThreadState & FakeOpenThread();
void ResetFakeOpenThread();
otInstance * FakeOpenThreadInstance();

} // namespace Testing
} // namespace chip
