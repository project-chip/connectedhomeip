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
#include "RendezvousMessageHandler.h"
#include "esp_log.h"
#include <platform/ConfigurationManager.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>

extern BluetoothWidget bluetoothLED;
extern NodeId kLocalNodeId;
extern void PairingComplete(SecurePairingSession * pairing);

static const char * TAG = "rendezvous-devicedelegate";

using namespace ::chip;

RendezvousDeviceDelegate::RendezvousDeviceDelegate()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    RendezvousParameters params;

    uint32_t setupPINCode;
    err = DeviceLayer::ConfigurationMgr().GetSetupPinCode(setupPINCode);
    SuccessOrExit(err);

    params.SetSetupPINCode(setupPINCode).SetLocalNodeId(kLocalNodeId).SetBleLayer(DeviceLayer::ConnectivityMgr().GetBleLayer());

    mRendezvousSession = new RendezvousSession(this);
    err                = mRendezvousSession->Init(params);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "RendezvousDeviceDelegate Init failure: %s", ErrorStr(err));
    }
}

CHIP_ERROR RendezvousDeviceDelegate::Send(const char * msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBuffer * buffer;
    const size_t msgLen = strlen(msg);

    VerifyOrExit(mRendezvousSession, err = CHIP_ERROR_INCORRECT_STATE);

    buffer = System::PacketBuffer::NewWithAvailableSize(msgLen);
    memcpy(buffer->Start(), msg, msgLen);
    buffer->SetDataLength(msgLen);

    err = mRendezvousSession->SendMessage(buffer);

exit:
    return err;
}

void RendezvousDeviceDelegate::OnRendezvousError(CHIP_ERROR err)
{
    ESP_LOGI(TAG, "OnRendezvousError: %s", ErrorStr(err));
}

void RendezvousDeviceDelegate::OnRendezvousConnectionOpened()
{
    ESP_LOGI(TAG, "OnRendezvousConnectionOpened");

    PairingComplete(&mRendezvousSession->GetPairingSession());
    bluetoothLED.Set(true);
}

void RendezvousDeviceDelegate::OnRendezvousConnectionClosed()
{
    ESP_LOGI(TAG, "OnRendezvousConnectionClosed");
    bluetoothLED.Set(false);
}

void RendezvousDeviceDelegate::OnRendezvousMessageReceived(PacketBuffer * buffer)
{
    ESP_LOGI(TAG, "OnRendezvousMessageReceived");

    // When paired, offer the RendezvousMessageHandler a chance to process the message.
    CHIP_ERROR err = RendezvousMessageHandler::HandleMessageReceived(buffer);
    if (err == CHIP_ERROR_INVALID_MESSAGE_TYPE)
    {
        // If the handler did not recognize the message, treat it as an echo request.
        mRendezvousSession->SendMessage(buffer);
    }
}
