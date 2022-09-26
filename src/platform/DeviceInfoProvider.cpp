/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *          Provides implementations for the DeviceInfoProvider APIs.
 *          This implementation is common across all platforms.
 */

#include <lib/support/CodeUtils.h>
#include <platform/DeviceInfoProvider.h>

namespace chip {
namespace DeviceLayer {

class DeviceInfoProvider;

namespace {

DeviceInfoProvider * gDeviceInfoProvider = nullptr;

} // namespace

CHIP_ERROR DeviceInfoProvider::SetUserLabelList(EndpointId endpoint,
                                                const AttributeList<UserLabelType, kMaxUserLabelListLength> & labelList)
{
    size_t index          = 0;
    size_t previousLength = 0;
    size_t currentLength  = labelList.size();

    CHIP_ERROR err = GetUserLabelLength(endpoint, previousLength);
    VerifyOrReturnError(err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND || err == CHIP_NO_ERROR, err);

    ReturnErrorOnFailure(SetUserLabelLength(endpoint, currentLength));

    for (const UserLabelType & label : labelList)
    {
        ReturnErrorOnFailure(SetUserLabelAt(endpoint, index++, label));
    }

    // If the list becomes smaller than previous list for a given endpoint, all "over-size" keys
    // (keys for [current_length..previous_length-1]) should be deleted, to recover space.
    for (size_t i = currentLength; i < previousLength; i++)
    {
        ReturnErrorOnFailure(DeleteUserLabelAt(endpoint, i));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInfoProvider::ClearUserLabelList(EndpointId endpoint)
{
    size_t length;

    CHIP_ERROR err = GetUserLabelLength(endpoint, length);
    VerifyOrReturnError(err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, CHIP_NO_ERROR);
    ReturnErrorOnFailure(err);

    for (size_t i = 0; i < length; i++)
    {
        ReturnErrorOnFailure(DeleteUserLabelAt(endpoint, i));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInfoProvider::AppendUserLabel(EndpointId endpoint, const UserLabelType & label)
{
    size_t length;

    // Increase the size of UserLabelList by 1
    ReturnErrorOnFailure(GetUserLabelLength(endpoint, length));
    ReturnErrorOnFailure(SetUserLabelLength(endpoint, length + 1));

    // Append the user label at the end of UserLabelList
    ReturnErrorOnFailure(SetUserLabelAt(endpoint, length, label));

    return CHIP_NO_ERROR;
}

void DeviceInfoProvider::SetStorageDelegate(PersistentStorageDelegate * storage)
{
    VerifyOrDie(storage != nullptr);
    mStorage = storage;
}

/**
 * Instance getter for the global DeviceInfoProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global Device Info provider. Assume never null.
 */
DeviceInfoProvider * GetDeviceInfoProvider()
{
    return gDeviceInfoProvider;
}

/**
 * Instance setter for the global DeviceInfoProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, no change is done.
 *
 * @param[in] provider the global Device Info Provider.
 */
void SetDeviceInfoProvider(DeviceInfoProvider * provider)
{
    if (provider)
    {
        gDeviceInfoProvider = provider;
    }
}

} // namespace DeviceLayer
} // namespace chip
