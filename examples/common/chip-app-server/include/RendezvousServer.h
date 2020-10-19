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

#include <platform/CHIPDeviceLayer.h>
#include <transport/RendezvousSession.h>

namespace chip {

class AccesoryNetworkProvisioningDelegate : public DeviceNetworkProvisioningDelegate
{
    void ProvisionThread(const DeviceLayer::Internal::DeviceNetworkInfo & threadData) override;
};

class RendezvousServer : public RendezvousSessionDelegate
{
public:
    RendezvousServer();

    CHIP_ERROR Init(const RendezvousParameters & params);

    void OnRendezvousConnectionOpened() override;
    void OnRendezvousConnectionClosed() override;
    void OnRendezvousError(CHIP_ERROR err) override;
    void OnRendezvousMessageReceived(System::PacketBuffer * buffer) override;
    void OnRendezvousStatusUpdate(Status status, CHIP_ERROR err) override;

private:
    AccesoryNetworkProvisioningDelegate mNetworkProvisioningDelegate;
    RendezvousSession mRendezvousSession;
};

} // namespace chip
