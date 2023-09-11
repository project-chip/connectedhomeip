/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/support/CodeUtils.h>
#include <platform/DeviceInstanceInfoProvider.h>

namespace chip {
namespace DeviceLayer {

namespace {

DeviceInstanceInfoProvider * gDeviceInstanceInfoProvider = nullptr;

} // namespace

DeviceInstanceInfoProvider * GetDeviceInstanceInfoProvider()
{
    VerifyOrDie(gDeviceInstanceInfoProvider != nullptr);
    return gDeviceInstanceInfoProvider;
}

void SetDeviceInstanceInfoProvider(DeviceInstanceInfoProvider * provider)
{
    if (provider == nullptr)
    {
        return;
    }

    gDeviceInstanceInfoProvider = provider;
}

} // namespace DeviceLayer
} // namespace chip
