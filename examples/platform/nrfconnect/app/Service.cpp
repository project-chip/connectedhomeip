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
 * @file
 *   This file implements the service publishing code for example usage.
 */

#include "Service.h"

#include <string.h>

#include <logging/log.h>
#include <platform/CHIPDeviceLayer.h>
#include <zephyr.h>

#if CHIP_ENABLE_OPENTHREAD
#include <openthread/message.h>
#include <openthread/udp.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <platform/nrfconnect/ThreadStackManagerImpl.h>
#endif

LOG_MODULE_DECLARE(app);

// Transport Callbacks
namespace {

char deviceName[CONFIG_BT_DEVICE_NAME_MAX];
constexpr uint16_t kUDPBroadcastPort = 23367;

} // namespace

void PublishService()
{
    chip::Inet::IPAddress addr;
    if (!chip::DeviceLayer::ConnectivityMgrImpl().IsThreadAttached())
    {
        return;
    }
    chip::DeviceLayer::ThreadStackMgrImpl().LockThreadStack();
    otError error = OT_ERROR_NONE;
    otMessageInfo messageInfo;
    otUdpSocket mSocket;
    otMessage * message = nullptr;

    memset(&mSocket, 0, sizeof(mSocket));
    memset(&messageInfo, 0, sizeof(messageInfo));

    // Use mesh local EID by default, if we have GUA, use that IP address.
    memcpy(&messageInfo.mSockAddr, otThreadGetMeshLocalEid(chip::DeviceLayer::ThreadStackMgrImpl().OTInstance()),
           sizeof(messageInfo.mSockAddr));

    // Select a address to send
    const otNetifAddress * otAddrs = otIp6GetUnicastAddresses(chip::DeviceLayer::ThreadStackMgrImpl().OTInstance());
    for (const otNetifAddress * otAddr = otAddrs; otAddr != NULL; otAddr = otAddr->mNext)
    {
        addr = chip::DeviceLayer::Internal::ToIPAddress(otAddr->mAddress);
        if (otAddr->mValid && addr.IsIPv6GlobalUnicast())
        {
            memcpy(&messageInfo.mSockAddr, &(otAddr->mAddress), sizeof(otAddr->mAddress));
            break;
        }
    }

    message = otUdpNewMessage(chip::DeviceLayer::ThreadStackMgrImpl().OTInstance(), nullptr);
    otIp6AddressFromString("ff03::1", &messageInfo.mPeerAddr);
    messageInfo.mPeerPort = kUDPBroadcastPort;
    otMessageAppend(message, deviceName, static_cast<uint16_t>(strlen(deviceName)));

    error = otUdpSend(chip::DeviceLayer::ThreadStackMgrImpl().OTInstance(), &mSocket, message, &messageInfo);

    if (error != OT_ERROR_NONE && message != nullptr)
    {
        otMessageFree(message);
        LOG_INF("Failed to otUdpSend: %d", error);
    }
    chip::DeviceLayer::ThreadStackMgrImpl().UnlockThreadStack();
}
