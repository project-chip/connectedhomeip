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

/**
 *    @file
 *      This file implements constants, globals and interfaces common
 *      to and used by CHIP example applications.
 *
 */

#include <errno.h>

#include "common.h"
#include <core/CHIPCore.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/ErrorStr.h>

// The ExchangeManager global object.
chip::Messaging::ExchangeManager gExchangeManager;

void InitializeChip(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    printf("Init CHIP Stack\r\n");

    // Initialize System memory and resources
    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

    // Initialize the CHIP stack.
    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("Failed to init CHIP Stack with err: %s\r\n", chip::ErrorStr(err));
        exit(EXIT_FAILURE);
    }
}

void ShutdownChip(void)
{
    gExchangeManager.Shutdown();
    chip::DeviceLayer::SystemLayer.Shutdown();
}

void DriveIO(void)
{
    struct timeval sleepTime;
    fd_set readFDs, writeFDs, exceptFDs;
    int numFDs = 0;
    int selectRes;

    sleepTime.tv_sec  = 0;
    sleepTime.tv_usec = kNetworkSleepTimeMsecs;

    FD_ZERO(&readFDs);
    FD_ZERO(&writeFDs);
    FD_ZERO(&exceptFDs);

    if (chip::DeviceLayer::SystemLayer.State() == chip::System::kLayerState_Initialized)
        chip::DeviceLayer::SystemLayer.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, sleepTime);

    if (chip::DeviceLayer::InetLayer.State == chip::Inet::InetLayer::kState_Initialized)
        chip::DeviceLayer::InetLayer.PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, sleepTime);

    selectRes = select(numFDs, &readFDs, &writeFDs, &exceptFDs, &sleepTime);
    if (selectRes < 0)
    {
        printf("select failed: %s\n", chip::ErrorStr(chip::System::MapErrorPOSIX(errno)));
        return;
    }

    if (chip::DeviceLayer::SystemLayer.State() == chip::System::kLayerState_Initialized)
    {
        chip::DeviceLayer::SystemLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }

    if (chip::DeviceLayer::InetLayer.State == chip::Inet::InetLayer::kState_Initialized)
    {
        chip::DeviceLayer::InetLayer.HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }
}
