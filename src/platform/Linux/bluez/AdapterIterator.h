/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "Types.h"

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

namespace chip {
namespace DeviceLayer {
namespace Internal {

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
    BluezAdapter1 * GetAdapter() const { return mCurrent.adapter; }

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
        BluezAdapter1 * adapter;
    } mCurrent = { 0 };
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
