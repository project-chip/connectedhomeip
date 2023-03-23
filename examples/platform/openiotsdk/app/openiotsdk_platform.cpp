/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *      This file provides the common Open IoT SDK platform functions.
 *      It can be used in Matter examples implementation.
 */

#include "openiotsdk_platform.h"

#include "cmsis_os2.h"
#include "iotsdk/ip_network_api.h"
#include "mbedtls/platform.h"

#include <DeviceInfoProviderImpl.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/openiotsdk/OpenIoTSDKArchUtils.h>

using namespace ::chip;
using namespace ::chip::Platform;
using namespace ::chip::DeviceLayer;

#define NETWORK_UP_FLAG 0x00000001U
#define NETWORK_DOWN_FLAG 0x00000002U
#define ALL_EVENTS_FLAGS (NETWORK_UP_FLAG | NETWORK_DOWN_FLAG)

#define EVENT_TIMEOUT 5000

static osEventFlagsId_t event_flags_id;

static DeviceLayer::DeviceInfoProviderImpl gDeviceInfoProvider;

/** Wait for specific event and check error */
static int wait_for_event(uint32_t event)
{
    int res = EXIT_SUCCESS;
    int ret = osEventFlagsWait(event_flags_id, ALL_EVENTS_FLAGS, osFlagsWaitAny, ms2tick(EVENT_TIMEOUT));
    if (ret < 0)
    {
        ChipLogError(NotSpecified, "osEventFlagsWait failed %d", ret);
        return EXIT_FAILURE;
    }

    if (!(ret & event))
    {
        res = EXIT_FAILURE;
    }

    ret = osEventFlagsClear(event_flags_id, ALL_EVENTS_FLAGS);
    if (ret < 0)
    {
        ChipLogError(NotSpecified, "osEventFlagsClear failed %d", ret);
        return EXIT_FAILURE;
    }

    return res;
}

static void post_network_connect()
{
    // Iterate on the network interface to see if we already have beed assigned addresses.
    for (chip::Inet::InterfaceAddressIterator it; it.HasCurrent(); it.Next())
    {
        char ifName[chip::Inet::InterfaceId::kMaxIfNameLength];
        if (it.IsUp() && CHIP_NO_ERROR == it.GetInterfaceName(ifName, sizeof(ifName)))
        {
            chip::Inet::IPAddress addr;
            if ((it.GetAddress(addr) == CHIP_NO_ERROR))
            {
                char ipStrBuf[chip::Inet::IPAddress::kMaxStringLength] = { 0 };
                addr.ToString(ipStrBuf);

                ChipLogProgress(DeviceLayer, "Got IP address on interface: %s IP: %s", ifName, ipStrBuf);
            }
        }
    }
}

/** This callback is called by the ip network task. It translates from a network event code
 * to platform event and sends it.
 *
 * @param event network up or down event.
 */
static void network_state_callback(network_state_callback_event_t event)
{
    uint32_t event_flag = (event == NETWORK_UP) ? NETWORK_UP_FLAG : NETWORK_DOWN_FLAG;
    ChipLogDetail(NotSpecified, "Network %s", (event == NETWORK_UP) ? "UP" : "DOWN");
    int res = osEventFlagsSet(event_flags_id, event_flag);
    if (res < 0)
    {
        ChipLogError(NotSpecified, "osEventFlagsSet failed %d", res);
    }
}

int openiotsdk_platform_init(void)
{
    int ret;
    osKernelState_t state;

    ret = mbedtls_platform_setup(NULL);
    if (ret)
    {
        ChipLogError(NotSpecified, "Mbed TLS platform initialization failed: %d", ret);
        return EXIT_FAILURE;
    }

    ret = osKernelInitialize();
    if (ret != osOK)
    {
        ChipLogError(NotSpecified, "osKernelInitialize failed: %d", ret);
        return EXIT_FAILURE;
    }

    state = osKernelGetState();
    if (state != osKernelReady)
    {
        ChipLogError(NotSpecified, "Kernel not ready: %d", state);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int openiotsdk_chip_init(void)
{
    CHIP_ERROR err;

#if NDEBUG
    chip::Logging::SetLogFilter(chip::Logging::LogCategory::kLogCategory_Progress);
#endif

    err = MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Memory initialization failed: %s", err.AsString());
        return EXIT_FAILURE;
    }

    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Chip stack initialization failed: %s", err.AsString());
        return EXIT_FAILURE;
    }

    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Chip stack start failed: %s", err.AsString());
        return EXIT_FAILURE;
    }

    DeviceLayer::SetDeviceInfoProvider(&gDeviceInfoProvider);

    return EXIT_SUCCESS;
}

int openiotsdk_platform_run(void)
{
    int ret = osKernelStart();
    if (ret != osOK)
    {
        ChipLogError(NotSpecified, "Failed to start kernel: %d", ret);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int openiotsdk_network_init(bool wait)
{
    int ret;

    event_flags_id = osEventFlagsNew(NULL);
    if (event_flags_id == NULL)
    {
        ChipLogError(NotSpecified, "Create event flags failed");
        return EXIT_FAILURE;
    }

    ret = start_network_task(network_state_callback, NETWORK_THREAD_STACK_SIZE_DEFAULT);
    if (ret != osOK)
    {
        ChipLogError(NotSpecified, "start_network_task failed %d", ret);
        return EXIT_FAILURE;
    }

    if (wait)
    {
        if (wait_for_event(NETWORK_UP_FLAG) != EXIT_SUCCESS)
        {
            ChipLogError(NotSpecified, "Network initialization failed");
            return EXIT_FAILURE;
        }
        post_network_connect();
    }

    return EXIT_SUCCESS;
}
