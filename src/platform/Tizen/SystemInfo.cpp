/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "SystemInfo.h"

#include <iostream>

#include <system_info.h>
#include <tizen.h>

#include <lib/support/logging/CHIPLogging.h>

using namespace std;

namespace chip {
namespace DeviceLayer {
namespace Internal {

SystemInfo SystemInfo::sInstance;

CHIP_ERROR SystemInfo::GetPlatformVersion(PlatformVersion & version)
{
    char * platformVersion;
    int ret;

    if (sInstance.mMajor > 0)
    {
        version.mMajor = sInstance.mMajor;
        version.mMinor = sInstance.mMinor;
        return CHIP_NO_ERROR;
    }

    ret = system_info_get_platform_string("http://tizen.org/feature/platform.version", &platformVersion);
    if (ret != SYSTEM_INFO_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "system_info_get_platform_string() failed. %s", get_error_message(ret));
        return CHIP_ERROR_INTERNAL;
    }

    sInstance.mMajor = version.mMajor = (uint8_t)(platformVersion[0] - '0');
    sInstance.mMinor = version.mMinor = (uint8_t)(platformVersion[2] - '0');
    free(platformVersion);
    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
