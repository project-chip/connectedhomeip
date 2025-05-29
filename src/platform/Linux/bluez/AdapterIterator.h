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

#include <cstdint>

#include <platform/GLibTypeDeleter.h>
#include <platform/Linux/dbus/bluez/DBusBluez.h>

#include "BluezObjectIterator.h"
#include "BluezObjectManager.h"
#include "Types.h"

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
    AdapterIterator() = default;
    ~AdapterIterator() { mObjectManager.Shutdown(); }

    /// Moves to the next DBUS interface.
    ///
    /// MUST be called before any of the 'current value' methods are
    /// used (iterator gets initialized on the first call of Next).
    bool Next();

    // Information about the current value. Safe to call only after
    // "Next" has returned true.
    uint32_t GetIndex() const;
    const char * GetAddress() const { return bluez_adapter1_get_address(mCurrentAdapter.get()); }
    const char * GetAlias() const { return bluez_adapter1_get_alias(mCurrentAdapter.get()); }
    const char * GetName() const { return bluez_adapter1_get_name(mCurrentAdapter.get()); }
    bool IsPowered() const { return bluez_adapter1_get_powered(mCurrentAdapter.get()); }
    BluezAdapter1 * GetAdapter() const { return mCurrentAdapter.get(); }

private:
    /// Loads the next value in the list.
    ///
    /// Returns true if a value could be loaded, false if no more items to
    /// iterate through.
    bool Advance();

    BluezObjectManager mObjectManager;
    bool mIsInitialized = false;

    BluezObjectList mObjectList;
    BluezObjectIterator mIterator;
    // Data valid only if Next() returns true
    GAutoPtr<BluezAdapter1> mCurrentAdapter;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
