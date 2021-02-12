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

CHIP_ERROR InitBluezBleLayer(bool aIsCentral, char * apBleAddr, BLEAdvConfig & aBleAdvConfig, void *& apEndpoint);
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

CHIP_ERROR StartDiscovery(BluezEndpoint * apEndpoint, BluezDiscoveryRequest aRequest = {});

CHIP_ERROR StopDiscovery(BluezEndpoint * apEndpoint);

CHIP_ERROR ConnectDevice(BluezDevice1 * apDevice);

/// Iterates over available BlueZ adapters
///
/// Usage example:
///
///  AdapterIterator iterator;
///  while (iterator.Next()) {
///      std::cout << iterator.GetAddress() << std::endl;
///  }
///
/// Data is provided through the bluez dbus interface. You can view
/// this data in the commandline using commands such as:
///
///    busctl introspect org.bluez /org/bluez/hci0
class AdapterIterator
{
public:
    ~AdapterIterator();

    /// Moves to the next DBUS interface.
    ///
    /// MUST be called before any of the 'current value' methods are
    /// used (iterator gets initialized on the first call of Next).
    bool Next();

    // Information about the current value. Safe to call only after
    // "Next" has returned true.
    uint32_t GetIndex() const { return mCurrent.index; }
    const char * GetAddress() const { return mCurrent.address.c_str(); }
    const char * GetAlias() const { return mCurrent.alias.c_str(); }
    const char * GetName() const { return mCurrent.name.c_str(); }
    bool IsPowered() const { return mCurrent.powered; }

private:
    /// Sets up the DBUS manager and loads the list
    void Initialize();

    /// Loads the next value in the list.
    ///
    /// Returns true if a value could be loaded, false if no more items to
    /// iterate through.
    bool Advance();

    static constexpr size_t kMaxAddressLength = 19; // xx:xx:xx:xx:xx:xx
    static constexpr size_t kMaxNameLength    = 64;

    GDBusObjectManager * mManager = nullptr; // DBus connection
    GList * mObjectList           = nullptr; // listing of objects on the bus
    GList * mCurrentListItem      = nullptr; // current item viewed in the list

    // data valid only if Next() returns true
    struct
    {
        uint32_t index;
        std::string address;
        std::string alias;
        std::string name;
        bool powered;
    } mCurrent = { 0 };
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
