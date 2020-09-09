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

#include "RendezvousMessageHandler.h"
#include <platform/ESP32/ESP32Utils.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

using namespace ::chip;

extern CHIP_ERROR SetWiFiStationProvisioning(char * ssid, char * key);

CHIP_ERROR RendezvousMessageHandler::HandleMessageReceived(System::PacketBuffer * buffer)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    bool isWiFiCredentials = false;
    char * key             = nullptr;
    char * ssid            = nullptr;

    // Pending definition of an actual message format, WiFi credentials have the form
    // ‘::SSID:password:’, where ‘:’ can be any single ASCII character.
    constexpr size_t maxBufferLen =
        4 + chip::DeviceLayer::Internal::kMaxWiFiSSIDLength + chip::DeviceLayer::Internal::kMaxWiFiKeyLength;
    char msg[maxBufferLen];

    const size_t bufferLen = buffer->DataLength();
    VerifyOrExit(bufferLen < sizeof msg, err = CHIP_ERROR_INVALID_MESSAGE_TYPE);
    memcpy(msg, buffer->Start(), bufferLen);
    msg[bufferLen] = 0;
    ChipLogProgress(NetworkProvisioning, "RendezvousMessageHandler: Receive message: %s", msg);

    isWiFiCredentials = ((bufferLen > 3) && (msg[0] == msg[1]) && (msg[0] == msg[bufferLen - 1]));
    VerifyOrExit(isWiFiCredentials, err = CHIP_ERROR_INVALID_MESSAGE_TYPE);

    msg[1] = 0;
    ssid   = strtok(&msg[2], msg);
    key    = strtok(NULL, msg);
    VerifyOrExit(ssid && key, err = CHIP_ERROR_INVALID_MESSAGE_TYPE);

    ChipLogProgress(NetworkProvisioning, "RendezvousMessageHandler: SSID: %s, key: %s", ssid, key);
    err = SetWiFiStationProvisioning(ssid, key);
    SuccessOrExit(err);

exit:
    return err;
}
