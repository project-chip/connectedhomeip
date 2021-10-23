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

#include "ThreadUtil.h"

#include <lib/support/ThreadOperationalDataset.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/internal/DeviceNetworkInfo.h>

#include <zephyr.h>

#include <cstring>
#include <stdio.h>
#include <stdlib.h>

void StartDefaultThreadNetwork(void)
{
    chip::Thread::OperationalDataset dataset{};
    uint8_t xpanid[8];
    constexpr uint8_t masterkey[] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
    };
    constexpr uint8_t meshLocalPrefix[] = { 0xfd, 0x11, 0x11, 0x11, 0x11, 0x22, 0x00, 0x00 };

    net_bytes_from_str(xpanid, sizeof(xpanid), CONFIG_OPENTHREAD_XPANID);

    dataset.SetChannel(CONFIG_OPENTHREAD_CHANNEL);
    dataset.SetExtendedPanId(xpanid);
    dataset.SetMasterKey(masterkey);
    dataset.SetMeshLocalPrefix(meshLocalPrefix);
    dataset.SetNetworkName(CONFIG_OPENTHREAD_NETWORK_NAME);
    dataset.SetPanId(CONFIG_OPENTHREAD_PANID);

    chip::DeviceLayer::ThreadStackMgr().SetThreadEnabled(false);
    chip::DeviceLayer::ThreadStackMgr().SetThreadProvision(dataset.AsByteSpan());
    chip::DeviceLayer::ThreadStackMgr().SetThreadEnabled(true);
}

void tlOtPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char * aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    vprintf(aFormat, args);

    printf("\r\n");

    va_end(args);
}
