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

#include "RendezvousDeviceDelegate.h"

#include "BluetoothWidget.h"
#include "Globals.h"
#include "esp_log.h"
#include <platform/ConfigurationManager.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;

extern void PairingComplete(NodeId assignedNodeId, NodeId peerNodeId, SecurePairingSession * pairing);

static const char * TAG = "rendezvous-devicedelegate";

RendezvousDeviceDelegate::RendezvousDeviceDelegate()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    RendezvousParameters params;

    uint32_t setupPINCode;
    err = DeviceLayer::ConfigurationMgr().GetSetupPinCode(setupPINCode);
    SuccessOrExit(err);

    params.SetSetupPINCode(setupPINCode).SetBleLayer(DeviceLayer::ConnectivityMgr().GetBleLayer());

    mRendezvousSession = chip::Platform::New<RendezvousSession>(this);
    err                = mRendezvousSession->Init(params);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "RendezvousDeviceDelegate Init failure: %s", ErrorStr(err));
    }
}

void RendezvousDeviceDelegate::OnRendezvousComplete()
{
    ESP_LOGI(TAG, "Device completed Rendezvous process\n");
    if (mRendezvousSession->GetLocalNodeId().HasValue() && mRendezvousSession->GetRemoteNodeId().HasValue())
    {
        PairingComplete(mRendezvousSession->GetLocalNodeId().Value(), mRendezvousSession->GetRemoteNodeId().Value(),
                        &mRendezvousSession->GetPairingSession());
    }
    else
    {
        ESP_LOGI(TAG, "Commissioner did not assign a node ID to the device!!!\n");
    }
}

void RendezvousDeviceDelegate::OnRendezvousStatusUpdate(RendezvousSessionDelegate::Status status, CHIP_ERROR err)
{
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "OnRendezvousStatusUpdate: %s, status %d", ErrorStr(err), status);
    }

    switch (status)
    {
    case RendezvousSessionDelegate::SecurePairingSuccess:
        ESP_LOGI(TAG, "Device completed SPAKE2+ handshake\n");
        bluetoothLED.Set(true);
        break;

    case RendezvousSessionDelegate::SecurePairingFailed:
        ESP_LOGI(TAG, "Failed in SPAKE2+ handshake\n");
        bluetoothLED.Set(false);
        break;

    case RendezvousSessionDelegate::NetworkProvisioningSuccess:

        ESP_LOGI(TAG, "Device was assigned an ip address\n");
        bluetoothLED.Set(false);
        break;

    case RendezvousSessionDelegate::NetworkProvisioningFailed:

        ESP_LOGI(TAG, "Failed in network provisioning\n");
        bluetoothLED.Set(false);
        break;

    default:
        break;
    };
}
