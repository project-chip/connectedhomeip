/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <lib/shell/Engine.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/RandUtils.h>
#include <support/logging/CHIPLogging.h>

#ifdef MBED_CONF_MBED_TRACE_ENABLE
#include "mbed-trace/mbed_trace.h"
#endif

#include "ChipShellMbedCollection.h"
#include <ChipShellCollection.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::Shell;

int main()
{
    chip::Platform::MemoryInit();
    chip::DeviceLayer::PlatformMgr().InitChipStack();
    chip::DeviceLayer::PlatformMgr().StartEventLoopTask();

    // Initialize the default streamer that was linked.
    const int rc = streamer_init(streamer_get());

    if (rc != 0)
    {
        ChipLogError(Shell, "Streamer initialization failed: %d", rc);
        return rc;
    }

#ifdef MBED_CONF_MBED_TRACE_ENABLE
    mbed_trace_init();
    mbed_trace_include_filters_set("BSDS,NETS");
    mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL | TRACE_MODE_COLOR);
#endif

    cmd_misc_init();
    cmd_otcli_init();
    cmd_mbed_utils_init();

    shell_task(nullptr);
    return 0;
}
