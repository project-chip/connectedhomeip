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

#ifndef __RENDEZVOUSDEVICEDELEGATE_H__
#define __RENDEZVOUSDEVICEDELEGATE_H__

#include "DeviceNetworkProvisioningDelegate.h"

#include <platform/CHIPDeviceLayer.h>
#include <transport/RendezvousSession.h>

class RendezvousDeviceDelegate : public chip::RendezvousSessionDelegate
{
public:
    RendezvousDeviceDelegate();

    //////////// RendezvousSession callback Implementation ///////////////
    void OnRendezvousMessageReceived(chip::System::PacketBuffer * buffer) override;
    void OnRendezvousStatusUpdate(RendezvousSessionDelegate::Status status, CHIP_ERROR err) override;

private:
    chip::RendezvousSession * mRendezvousSession;
    ESP32NetworkProvisioningDelegate mDeviceNetworkProvisioningDelegate;
};

#endif // __RENDEZVOUSDEVICEDELEGATE_H__
