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

#pragma once

#include <cstdint>

#include <gio/gio.h>
#include <glib.h>

#include <ble/CHIPBleServiceData.h>
#include <lib/core/CHIPError.h>

#include "BluezConnection.h"
#include "Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

struct BluezEndpoint
{
    char * mpOwningName; // Bus owning name

    // Adapter properties
    char * mpAdapterName;
    char * mpAdapterAddr;

    // Paths for objects published by this service
    char * mpRootPath;
    char * mpServicePath;

    // Objects (interfaces) subscribed to by this service
    GDBusObjectManager * mpObjMgr = nullptr;
    BluezAdapter1 * mpAdapter     = nullptr;
    BluezDevice1 * mpDevice       = nullptr;

    // Objects (interfaces) published by this service
    GDBusObjectManagerServer * mpRoot;
    BluezGattService1 * mpService;
    BluezGattCharacteristic1 * mpC1;
    BluezGattCharacteristic1 * mpC2;
    // additional data characteristics
    BluezGattCharacteristic1 * mpC3;

    // map device path to the connection
    GHashTable * mpConnMap;
    uint32_t mAdapterId;
    bool mIsCentral;
    char * mpPeerDevicePath;
    GCancellable * mpConnectCancellable = nullptr;
};

CHIP_ERROR InitBluezBleLayer(uint32_t aAdapterId, bool aIsCentral, const char * apBleAddr, const char * apBleName,
                             BluezEndpoint *& apEndpoint);
CHIP_ERROR ShutdownBluezBleLayer(BluezEndpoint * apEndpoint);
CHIP_ERROR BluezGattsAppRegister(BluezEndpoint * apEndpoint);

CHIP_ERROR ConnectDevice(BluezDevice1 & aDevice, BluezEndpoint * apEndpoint);
void CancelConnect(BluezEndpoint * apEndpoint);

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
