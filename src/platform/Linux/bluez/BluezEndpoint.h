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
#include <string>
#include <unordered_map>

#include <gio/gio.h>
#include <glib.h>

#include <ble/Ble.h>
#include <lib/core/CHIPError.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/Linux/dbus/bluez/DBusBluez.h>

#include "BluezConnection.h"
#include "BluezObjectManager.h"
#include "Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

class BluezEndpoint : public BluezObjectManagerAdapterNotificationsDelegate
{
public:
    BluezEndpoint(BluezObjectManager & aObjectManager) : mObjectManager(aObjectManager) {}
    ~BluezEndpoint() = default;

    CHIP_ERROR Init(BluezAdapter1 * apAdapter, bool aIsCentral);
    void Shutdown();

    CHIP_ERROR RegisterGattApplication();
    GDBusObjectManagerServer * GetGattApplicationObjectManager() const { return mRoot.get(); }

    CHIP_ERROR ConnectDevice(BluezDevice1 & aDevice);
    void CancelConnect();

    // Members that implement virtual methods on BluezObjectManagerAdapterNotificationsDelegate
    void OnDeviceAdded(BluezDevice1 & device) override;
    void OnDevicePropertyChanged(BluezDevice1 & device, GVariant * changedProps, const char * const * invalidatedProps) override;
    void OnDeviceRemoved(BluezDevice1 & device) override;

private:
    CHIP_ERROR SetupEndpointBindings();
    void SetupGattServer(GDBusConnection * aConn);
    void SetupGattService();

    BluezGattService1 * CreateGattService(const char * aUUID);
    BluezGattCharacteristic1 * CreateGattCharacteristic(BluezGattService1 * aService, const char * aCharName, const char * aUUID,
                                                        const char * const * aFlags);

    void HandleNewDevice(BluezDevice1 & aDevice);
    void UpdateConnectionTable(BluezDevice1 & aDevice);
    BluezConnection * GetBluezConnection(const char * aPath);
    BluezConnection * GetBluezConnectionViaDevice();

    gboolean BluezCharacteristicReadValue(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInv, GVariant * aOptions);
    gboolean BluezCharacteristicAcquireWrite(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInv, GUnixFDList * aFDList,
                                             GVariant * aOptions);
    gboolean BluezCharacteristicAcquireNotify(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInv, GUnixFDList * aFDList,
                                              GVariant * aOptions);
    gboolean BluezCharacteristicConfirm(BluezGattCharacteristic1 * aChar, GDBusMethodInvocation * aInv);

    void RegisterGattApplicationDone(GObject * aObject, GAsyncResult * aResult);
    CHIP_ERROR RegisterGattApplicationImpl();

    CHIP_ERROR ConnectDeviceImpl(BluezDevice1 & aDevice);

    BluezObjectManager & mObjectManager;
    GAutoPtr<BluezAdapter1> mAdapter;

    bool mIsCentral     = false;
    bool mIsInitialized = false;

    // Paths for objects published by this service
    char * mpRootPath    = nullptr;
    char * mpServicePath = nullptr;

    // Objects (interfaces) published by this service
    GAutoPtr<GDBusObjectManagerServer> mRoot;
    GAutoPtr<BluezGattService1> mService;
    GAutoPtr<BluezGattCharacteristic1> mC1;
    GAutoPtr<BluezGattCharacteristic1> mC2;
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    // additional data characteristics
    GAutoPtr<BluezGattCharacteristic1> mC3;
#endif

    std::unordered_map<std::string, BluezConnection *> mConnMap;
    GAutoPtr<GCancellable> mConnectCancellable;
    char * mpPeerDevicePath = nullptr;

    // Allow BluezConnection to access our private members
    friend class BluezConnection;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
