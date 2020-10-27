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

class RendezvousServer : public RendezvousSessionDelegate, public DeviceNetworkProvisioningDelegate
{
public:
    RendezvousServer();

    CHIP_ERROR Init(const RendezvousParameters & params);

    //////////////// RendezvousSessionDelegate Implementation ///////////////////

    void OnRendezvousConnectionOpened() override;
    void OnRendezvousConnectionClosed() override;
    void OnRendezvousError(CHIP_ERROR err) override;
    void OnRendezvousMessageReceived(System::PacketBuffer * buffer) override;
    void OnRendezvousStatusUpdate(Status status, CHIP_ERROR err) override;

    //////////// DeviceNetworkProvisioningDelegate Implementation ///////////////

    void ProvisionThread(const DeviceLayer::Internal::DeviceNetworkInfo & threadData) override;

private:
    RendezvousSession mRendezvousSession;
};

} // namespace chip
