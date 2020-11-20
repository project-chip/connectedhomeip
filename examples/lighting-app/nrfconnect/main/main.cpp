/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "pw_log/log.h"
#include "pw_sys_io/sys_io.h"
#include "pw_sys_io_nrfconnect/init.h"
#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPMem.h>

#include <kernel.h>

#ifndef CONFIG_USE_PW_LOG
#include <logging/log.h>
LOG_MODULE_DECLARE(app);
#else
#define LOG_INF(message, ...) PW_LOG_INFO(message, __VA_ARGS__)
#define LOG_ERR(message, ...) PW_LOG_ERROR(message, __VA_ARGS__)
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

namespace chip {
namespace rpc {
extern void RunRpcService(void *, void *, void *);
}
} // namespace chip

namespace {
#define ECHO_STACK_SIZE 4096
#define ECHO_PRIORITY 5

K_THREAD_STACK_DEFINE(echo_stack_area, ECHO_STACK_SIZE);
struct k_thread echo_thread_data;

} // namespace

int main(void)
{
    pw_sys_io_Init();
    k_tid_t my_tid = k_thread_create(&echo_thread_data, echo_stack_area, K_THREAD_STACK_SIZEOF(echo_stack_area),
                                     chip::rpc::RunRpcService, NULL, NULL, NULL, ECHO_PRIORITY, 0, K_NO_WAIT);

    int ret = 0;

    k_thread_priority_set(k_current_get(), K_PRIO_COOP(CONFIG_NUM_COOP_PRIORITIES - 1));

    ret = chip::Platform::MemoryInit();
    if (ret != CHIP_NO_ERROR)
    {
        LOG_ERR("Platform::MemoryInit() failed");
        goto exit;
    }

    LOG_INF("Init CHIP stack");
    ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        LOG_ERR("PlatformMgr().InitChipStack() failed");
        goto exit;
    }

    LOG_INF("Starting CHIP task");
    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        LOG_ERR("PlatformMgr().StartEventLoopTask() failed");
        goto exit;
    }

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    LOG_INF("Init Thread stack");
    ret = ThreadStackMgr().InitThreadStack();
    if (ret != CHIP_NO_ERROR)
    {
        LOG_ERR("ThreadStackMgr().InitThreadStack() failed");
        goto exit;
    }
#endif

    ret = GetAppTask().StartApp();

exit:
    LOG_ERR("Exited with code %d", ret);
    return ret;
}
