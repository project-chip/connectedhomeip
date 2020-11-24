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

#pragma once

#include <core/CHIPCore.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

class RendezvousSessionDelegate
{
public:
    virtual ~RendezvousSessionDelegate() {}

    enum Status : uint8_t
    {
        SecurePairingSuccess = 0,
        SecurePairingFailed,
        NetworkProvisioningSuccess,
        NetworkProvisioningFailed,
    };

    virtual void OnRendezvousConnectionOpened() {}
    virtual void OnRendezvousConnectionClosed() {}
    virtual void OnRendezvousError(CHIP_ERROR err) {}
    virtual void OnRendezvousComplete() {}
    virtual void OnRendezvousMessageReceived(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                             System::PacketBufferHandle buffer){};
    virtual void OnRendezvousStatusUpdate(Status status, CHIP_ERROR err) {}
};

class DLL_EXPORT RendezvousDeviceCredentialsDelegate
{
public:
    virtual ~RendezvousDeviceCredentialsDelegate() {}

    virtual void SendNetworkCredentials(const char * ssid, const char * passwd)          = 0;
    virtual void SendThreadCredentials(const DeviceLayer::Internal::DeviceNetworkInfo &) = 0;
    virtual void SendOperationalCredentials()                                            = 0;
};

} // namespace chip
