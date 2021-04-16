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

/*
 *  Copyright (c) 2016-2019, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *    @file
 *          Provides Bluez dbus implementation for BLE
 */

#pragma once

#include "Types.h"

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR InitBluezBleLayer(bool aIsCentral, char * apBleAddr, BLEAdvConfig & aBleAdvConfig, BluezEndpoint *& apEndpoint);
bool BluezRunOnBluezThread(int (*aCallback)(void *), void * apClosure);
bool SendBluezIndication(BLE_CONNECTION_OBJECT apConn, chip::System::PacketBufferHandle apBuf);
bool CloseBluezConnection(BLE_CONNECTION_OBJECT apConn);
CHIP_ERROR StartBluezAdv(BluezEndpoint * apEndpoint);
CHIP_ERROR StopBluezAdv(BluezEndpoint * apEndpoint);
CHIP_ERROR BluezGattsAppRegister(BluezEndpoint * apEndpoint);
CHIP_ERROR BluezAdvertisementSetup(BluezEndpoint * apEndpoint);

/// Write to the CHIP RX characteristic on the remote peripheral device
bool BluezSendWriteRequest(BLE_CONNECTION_OBJECT apConn, chip::System::PacketBufferHandle apBuf);
/// Subscribe to the CHIP TX characteristic on the remote peripheral device
bool BluezSubscribeCharacteristic(BLE_CONNECTION_OBJECT apConn);
/// Unsubscribe from the CHIP TX characteristic on the remote peripheral device
bool BluezUnsubscribeCharacteristic(BLE_CONNECTION_OBJECT apConn);

CHIP_ERROR ConnectDevice(BluezDevice1 * apDevice, BluezEndpoint * apEndpoint);
void CancelConnect(BluezEndpoint * apEndpoint);

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
