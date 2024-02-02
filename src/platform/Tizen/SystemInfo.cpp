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

#include "SystemInfo.h"

#include <iostream>

#include <system_info.h>
#include <tizen.h>

#include <lib/support/logging/CHIPLogging.h>

#include "ErrorUtils.h"

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
        ChipLogError(DeviceLayer, "system_info_get_platform_string() failed: %s", get_error_message(ret));
        return TizenToChipError(ret);
    }

    sInstance.mMajor = version.mMajor = (uint8_t) (platformVersion[0] - '0');
    sInstance.mMinor = version.mMinor = (uint8_t) (platformVersion[2] - '0');
    free(platformVersion);
    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
