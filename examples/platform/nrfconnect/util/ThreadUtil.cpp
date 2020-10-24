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

#include "ThreadUtil.h"

#include <platform/CHIPDeviceLayer.h>
#include <platform/internal/DeviceNetworkInfo.h>

#include <zephyr.h>

#include <cstring>

void StartDefaultThreadNetwork(void)
{
    chip::DeviceLayer::Internal::DeviceNetworkInfo deviceNetworkInfo;
    memset(&deviceNetworkInfo, 0, sizeof(deviceNetworkInfo));

    const uint8_t masterKey[chip::DeviceLayer::Internal::kThreadMasterKeyLength] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
    };
    const uint8_t threadMeshPrefix[chip::DeviceLayer::Internal::kThreadMeshPrefixLength] = { 0xfd, 0x11, 0x11, 0x11,
                                                                                             0x11, 0x22, 0x00, 0x00 };

    memcpy(deviceNetworkInfo.ThreadNetworkName, CONFIG_OPENTHREAD_NETWORK_NAME, strlen(CONFIG_OPENTHREAD_NETWORK_NAME));
    net_bytes_from_str(deviceNetworkInfo.ThreadExtendedPANId, 8, CONFIG_OPENTHREAD_XPANID);
    deviceNetworkInfo.FieldPresent.ThreadExtendedPANId = true;
    memcpy(deviceNetworkInfo.ThreadMasterKey, masterKey, sizeof(masterKey));
    deviceNetworkInfo.FieldPresent.ThreadMeshPrefix = true;
    memcpy(deviceNetworkInfo.ThreadMeshPrefix, threadMeshPrefix, sizeof(threadMeshPrefix));
    deviceNetworkInfo.ThreadPANId   = CONFIG_OPENTHREAD_PANID;
    deviceNetworkInfo.ThreadChannel = CONFIG_OPENTHREAD_CHANNEL;

    chip::DeviceLayer::ThreadStackMgr().SetThreadEnabled(false);
    chip::DeviceLayer::ThreadStackMgr().SetThreadProvision(deviceNetworkInfo);
    chip::DeviceLayer::ThreadStackMgr().SetThreadEnabled(true);
}
