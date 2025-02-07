/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include "AdapterIterator.h"

#include <cstdio>

#include <gio/gio.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/Linux/dbus/bluez/DBusBluez.h>

#include "BluezObjectIterator.h"
#include "BluezObjectList.h"
#include "BluezObjectManager.h"
#include "Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

bool AdapterIterator::Advance()
{
    for (; mIterator != BluezObjectList::end(); ++mIterator)
    {
        BluezAdapter1 * adapter = bluez_object_get_adapter1(&(*mIterator));
        if (adapter != nullptr)
        {
            mCurrentAdapter.reset(adapter);
            ++mIterator;
            return true;
        }
    }

    return false;
}

uint32_t AdapterIterator::GetIndex() const
{
    // PATH is of the for  BLUEZ_PATH / hci<nr>, i.e. like '/org/bluez/hci0'
    // Index represents the number after hci
    const char * path = g_dbus_proxy_get_object_path(G_DBUS_PROXY(mCurrentAdapter.get()));
    unsigned index    = 0;

    if (sscanf(path, BLUEZ_PATH "/hci%u", &index) != 1)
    {
        ChipLogError(DeviceLayer, "Failed to extract HCI index from '%s'", StringOrNullMarker(path));
        index = 0;
    }

    return index;
}

bool AdapterIterator::Next()
{
    if (!mIsInitialized)
    {
        CHIP_ERROR err = mObjectManager.Init();
        VerifyOrReturnError(
            err == CHIP_NO_ERROR, false,
            ChipLogError(DeviceLayer, "Failed to initialize BlueZ object manager: %" CHIP_ERROR_FORMAT, err.Format()));
        mObjectList.Init(mObjectManager.GetObjectManager());
        mIterator      = mObjectList.begin();
        mIsInitialized = true;
    }

    return Advance();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
