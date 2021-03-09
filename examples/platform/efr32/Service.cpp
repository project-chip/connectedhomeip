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

#include "Service.h"

#include "AppConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <sys/param.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <inet/InetLayer.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/ThreadOperationalDataset.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

#if CHIP_ENABLE_OPENTHREAD
#include <openthread/message.h>
#include <openthread/udp.h>
#include <platform/EFR32/ThreadStackManagerImpl.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <platform/internal/DeviceNetworkInfo.h>
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::DeviceLayer;

static char sDeviceName[128] = { 0 };
// Hardcode UDP BroadcastPort. Temporary use for demo with OTBR
constexpr uint16_t kUDPBroadcastPort = 23367;

void SetDeviceName(const char * newDeviceName)
{
    strncpy(sDeviceName, newDeviceName, sizeof(sDeviceName) - 1);
}

void PublishService()
{
    chip::Inet::IPAddress addr;
    if (!ConnectivityMgrImpl().IsThreadAttached())
    {
        return;
    }
    ThreadStackMgrImpl().LockThreadStack();
    otError error = OT_ERROR_NONE;
    otMessageInfo messageInfo;
    otUdpSocket mSocket;
    otMessage * message = nullptr;

    memset(&mSocket, 0, sizeof(mSocket));
    memset(&messageInfo, 0, sizeof(messageInfo));

    // Use mesh local EID by default, if we have GUA, use that IP address.
    memcpy(&messageInfo.mSockAddr, otThreadGetMeshLocalEid(ThreadStackMgrImpl().OTInstance()), sizeof(messageInfo.mSockAddr));

    // Select a address to send
    const otNetifAddress * otAddrs = otIp6GetUnicastAddresses(ThreadStackMgrImpl().OTInstance());
    for (const otNetifAddress * otAddr = otAddrs; otAddr != NULL; otAddr = otAddr->mNext)
    {
        addr = chip::DeviceLayer::Internal::ToIPAddress(otAddr->mAddress);
        if (otAddr->mValid && addr.IsIPv6GlobalUnicast())
        {
            memcpy(&messageInfo.mSockAddr, &(otAddr->mAddress), sizeof(otAddr->mAddress));
            break;
        }
    }

    message = otUdpNewMessage(ThreadStackMgrImpl().OTInstance(), nullptr);
    otIp6AddressFromString("ff03::1", &messageInfo.mPeerAddr);
    messageInfo.mPeerPort = kUDPBroadcastPort;
    otMessageAppend(message, sDeviceName, static_cast<uint16_t>(strlen(sDeviceName)));

    error = otUdpSend(ThreadStackMgrImpl().OTInstance(), &mSocket, message, &messageInfo);

    if (error != OT_ERROR_NONE && message != nullptr)
    {
        otMessageFree(message);
        EFR32_LOG("Failed to otUdpSend: %d", error);
    }
    ThreadStackMgrImpl().UnlockThreadStack();
}

void StartDefaultThreadNetwork(void)
{
    // Set default thread network Info and enable/start thread

    chip::Thread::OperationalDataset dataset;

    uint8_t xpanid[]    = { 0xde, 0xad, 0x00, 0xbe, 0xef, 0x00, 0xca, 0xfe };
    uint8_t masterkey[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    const uint8_t meshLocalPrefix[chip::Thread::kSizeMeshLocalPrefix] = { 0xfd, 0xde, 0xad, 0x00, 0xbe, 0xef, 0x00, 0x00 };

    dataset.SetChannel(11);
    dataset.SetExtendedPanId(xpanid);
    dataset.SetMasterKey(masterkey);
    dataset.SetMeshLocalPrefix(meshLocalPrefix);
    dataset.SetNetworkName("OpenThread");
    dataset.SetPanId(0x1234);

    chip::DeviceLayer::ThreadStackMgr().SetThreadEnabled(false);
    chip::DeviceLayer::ThreadStackMgr().SetThreadProvision(dataset);
    chip::DeviceLayer::ThreadStackMgr().SetThreadEnabled(true);
}
