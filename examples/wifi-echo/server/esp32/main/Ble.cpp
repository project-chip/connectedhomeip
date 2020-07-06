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

#include <platform/CHIPDeviceLayer.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

using namespace ::chip;
using namespace ::chip::Ble;

void HandleBleMessageReceived(BLEEndPoint * endPoint, PacketBuffer * buffer)
{
    const size_t bufferLen = buffer->DataLength();
    char msg[bufferLen];
    msg[bufferLen] = 0;
    memcpy(msg, buffer->Start(), bufferLen);

    ChipLogProgress(Ble, "BLEEndPoint: Receive message: %s", msg);

    endPoint->Send(buffer);
}

void HandleBleConnectionClosed(BLEEndPoint * endPoint, BLE_ERROR err)
{
    ChipLogProgress(Ble, "BLEEndPoint: Connection closed (%s)", ErrorStr(err));
}

void HandleBleNewConnection(BLEEndPoint * endPoint)
{
    ChipLogProgress(Ble, "BLEEndPoint: Connection opened");
    endPoint->OnMessageReceived  = HandleBleMessageReceived;
    endPoint->OnConnectionClosed = HandleBleConnectionClosed;
}

void startBle()
{
    DeviceLayer::ConnectivityMgr().AddCHIPoBLEConnectionHandler(HandleBleNewConnection);
}
