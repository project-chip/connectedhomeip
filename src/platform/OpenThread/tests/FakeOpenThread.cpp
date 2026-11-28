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

#include "FakeOpenThread.h"

#include <lib/core/CHIPError.h>
#include <platform/OpenThread/OpenThreadUtils.h>

#include <openthread/dataset.h>
#include <openthread/error.h>
#include <openthread/ip6.h>

#include <cstdint>

namespace chip {
namespace Testing {

namespace {
FakeOpenThreadState gState;
} // namespace

FakeOpenThreadState & FakeOpenThread()
{
    return gState;
}

void ResetFakeOpenThread()
{
    gState = FakeOpenThreadState();
}

otInstance * FakeOpenThreadInstance()
{
    static uint8_t storage = 0;
    return reinterpret_cast<otInstance *>(&storage);
}

} // namespace Testing

namespace DeviceLayer {
namespace Internal {

CHIP_ERROR MapOpenThreadError(otError otErr)
{
    return (otErr == OT_ERROR_NONE) ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

using chip::Testing::gState;

extern "C" {

otLinkModeConfig otThreadGetLinkMode(otInstance *)
{
    return gState.linkMode;
}

otError otThreadSetLinkMode(otInstance *, otLinkModeConfig aConfig)
{
    gState.setLinkModeLog.push_back(aConfig);
    gState.setLinkModeLockDepth.push_back(gState.threadStackLockDepth);
    otError result = OT_ERROR_NONE;
    if (!gState.setLinkModeResults.empty())
    {
        result = gState.setLinkModeResults.front();
        gState.setLinkModeResults.erase(gState.setLinkModeResults.begin());
    }
    if (result == OT_ERROR_NONE)
    {
        gState.linkMode = aConfig;
    }
    return result;
}

bool otIp6IsEnabled(otInstance *)
{
    return gState.ip6Enabled;
}

otError otIp6SetEnabled(otInstance *, bool aEnabled)
{
    gState.ip6SetEnabledLog.push_back(aEnabled);
    gState.ip6SetEnabledLockDepth.push_back(gState.threadStackLockDepth);
    otError result = aEnabled ? gState.ip6EnableResult : gState.ip6DisableResult;
    if (result == OT_ERROR_NONE)
    {
        gState.ip6Enabled = aEnabled;
    }
    return result;
}

otError otThreadDiscover(otInstance *, uint32_t, uint16_t, bool, bool, otHandleActiveScanResult, void *)
{
    gState.discoverCalls++;
    return gState.discoverResult;
}

otDeviceRole otThreadGetDeviceRole(otInstance *)
{
    return gState.role;
}

bool otDatasetIsCommissioned(otInstance *)
{
    return gState.commissioned;
}

otError otSetStateChangedCallback(otInstance *, otStateChangedCallback, void *)
{
    return OT_ERROR_NONE;
}

otError otThreadSetEnabled(otInstance *, bool aEnabled)
{
    if (gState.threadSetEnabledResult == OT_ERROR_NONE)
    {
        gState.role = aEnabled ? OT_DEVICE_ROLE_DETACHED : OT_DEVICE_ROLE_DISABLED;
    }
    return gState.threadSetEnabledResult;
}

otError otInstanceErasePersistentInfo(otInstance *)
{
    return OT_ERROR_NONE;
}

const char * otThreadErrorToString(otError)
{
    return "fake";
}

} // extern "C"
