/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "AppTask.h"

#include "mbedtls/platform.h"
#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPMem.h>
#include <support/logging/CHIPLogging.h>

#ifdef MBED_CONF_MBED_TRACE_ENABLE
#include "mbed-trace/mbed_trace.h"
#endif

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;

int main(int argc, char * argv[])
{
    int ret;
    CHIP_ERROR err;
#ifdef MBED_CONF_MBED_TRACE_ENABLE
    mbed_trace_init();
    mbed_trace_include_filters_set("BSDS,NETS");
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL | TRACE_MODE_COLOR);
#endif

    // note: Make sure to turn the filtering on with CHIP_LOG_FILTERING=1
    chip::Logging::SetLogFilter(chip::Logging::LogCategory::kLogCategory_Progress);

    ret = mbedtls_platform_setup(NULL);
    if (ret)
    {
        ChipLogError(NotSpecified, "Mbed TLS platform initialization failed with error %d", ret);
        goto exit;
    }

    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Platform::MemoryInit() failed");
        ret = static_cast<int>(ChipError::GetValue(err));
        goto exit;
    }

    ChipLogProgress(NotSpecified, "Init CHIP Stack\r\n");
    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "PlatformMgr().InitChipStack() failed");
        ret = static_cast<int>(ChipError::GetValue(err));
        goto exit;
    }

    ChipLogProgress(NotSpecified, "Starting CHIP task");
    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "PlatformMgr().StartEventLoopTask() failed");
        ret = static_cast<int>(ChipError::GetValue(err));
        goto exit;
    }

    ret = GetAppTask().StartApp();

exit:
    ChipLogProgress(NotSpecified, "Exited with code %d", ret);
    return ret;
}
