/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/StorageDelegateWrapper.h>

#include <lib/support/SafeInt.h>

namespace chip {
namespace app {

CHIP_ERROR StorageDelegateWrapper::WriteValue(const StorageKeyName & aKey, const ByteSpan & aValue)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // TODO: we may want to have a small cache for values that change a lot, so
    //  we only write them once a bunch of changes happen or on timer or
    //  shutdown.
    if (!CanCastTo<uint16_t>(aValue.size()))
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    return mStorage->SyncSetKeyValue(aKey.KeyName(), aValue.data(), static_cast<uint16_t>(aValue.size()));
}

CHIP_ERROR StorageDelegateWrapper::ReadValue(const StorageKeyName & aKey, MutableByteSpan & aValue)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint16_t size = static_cast<uint16_t>(std::min(aValue.size(), static_cast<size_t>(UINT16_MAX)));
    ReturnErrorOnFailure(mStorage->SyncGetKeyValue(aKey.KeyName(), aValue.data(), size));
    aValue.reduce_size(size);
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
