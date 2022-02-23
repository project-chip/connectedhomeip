/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

#include "GenericOTARequestorStorage.h"

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <platform/OTARequestorInterface.h>

#include <limits>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR GenericOTARequestorStorage::StoreProviderLocation(const ProviderLocationType & provider)
{
    uint8_t buffer[kMaxSerializedSize];
    TLV::TLVWriter writer;

    writer.Init(buffer);
    ReturnErrorOnFailure(provider.EncodeForRead(writer, TLV::AnonymousTag(), provider.fabricIndex));

    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator().OTAProviderLocation(), buffer,
                                               static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR GenericOTARequestorStorage::LoadProviderLocation(ProviderLocationType & provider)
{
    uint8_t buffer[kMaxSerializedSize];
    uint16_t size = sizeof(buffer);

    ReturnErrorOnFailure(mPersistentStorage->SyncGetKeyValue(DefaultStorageKeyAllocator().OTAProviderLocation(), buffer, size));

    TLV::TLVReader reader;

    reader.Init(buffer, size);
    ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
    ReturnErrorOnFailure(provider.Decode(reader));

    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericOTARequestorStorage::StoreUpdateToken(ByteSpan updateToken)
{
    VerifyOrReturnError(updateToken.size() <= std::numeric_limits<uint16_t>::max(), CHIP_ERROR_INVALID_ARGUMENT);

    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator().OTAUpdateToken(), updateToken.data(),
                                               static_cast<uint16_t>(updateToken.size()));
}

CHIP_ERROR GenericOTARequestorStorage::LoadUpdateToken(MutableByteSpan & updateToken)
{
    VerifyOrReturnError(updateToken.size() <= std::numeric_limits<uint16_t>::max(), CHIP_ERROR_INVALID_ARGUMENT);

    uint16_t size    = static_cast<uint16_t>(updateToken.size());
    CHIP_ERROR error = mPersistentStorage->SyncGetKeyValue(DefaultStorageKeyAllocator().OTAUpdateToken(), updateToken.data(), size);

    updateToken.reduce_size(size);
    return error;
}

} // namespace DeviceLayer
} // namespace chip
