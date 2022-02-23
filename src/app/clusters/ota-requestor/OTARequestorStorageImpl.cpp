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

#include "OTARequestorStorageImpl.h"
#include "OTARequestorInterface.h"

#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

#include <limits>

namespace chip {

// Calculated with Python code:
//   w = TLVWriter()
//   s = {1:uint(0xffffffffffffffff), 2:uint(0xffff), 254:uint(0xff)}
//   w.put(None, s)
//   len(w.encoding)
constexpr size_t kProviderMaxSerializedSize = 19u;

// Multiply the serialized provider size by the maximum number of fabrics and add 2 bytes for the array start and end.
constexpr size_t kProviderListMaxSerializedSize = kProviderMaxSerializedSize * CHIP_CONFIG_MAX_FABRICS + 2;

CHIP_ERROR OTARequestorStorageImpl::StoreDefaultProviders(const ProviderLocationList & providers)
{
    uint8_t buffer[kProviderListMaxSerializedSize];
    TLV::TLVWriter writer;
    TLV::TLVType outerType;

    writer.Init(buffer);
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType));

    for (auto providerIter = providers.Begin(); providerIter.Next();)
    {
        const auto & provider = providerIter.GetValue();
        ReturnErrorOnFailure(provider.EncodeForRead(writer, TLV::AnonymousTag(), provider.fabricIndex));
    }

    ReturnErrorOnFailure(writer.EndContainer(outerType));

    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator().OTADefaultProviders(), buffer,
                                               static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR OTARequestorStorageImpl::LoadDefaultProviders(ProviderLocationList & providers)
{
    uint8_t buffer[kProviderListMaxSerializedSize];
    uint16_t size = sizeof(buffer);

    ReturnErrorOnFailure(mPersistentStorage->SyncGetKeyValue(DefaultStorageKeyAllocator().OTADefaultProviders(), buffer, size));

    TLV::TLVReader reader;
    TLV::TLVType outerType;

    reader.Init(buffer, size);
    ReturnErrorOnFailure(reader.Next(TLV::TLVType::kTLVType_Array, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    while (reader.Next() != CHIP_ERROR_END_OF_TLV)
    {
        ProviderLocationType provider;
        ReturnErrorOnFailure(provider.Decode(reader));
        providers.Add(provider);
    }

    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestorStorageImpl::StoreCurrentProviderLocation(const ProviderLocationType & provider)
{
    uint8_t buffer[kProviderMaxSerializedSize];
    TLV::TLVWriter writer;

    writer.Init(buffer);
    ReturnErrorOnFailure(provider.EncodeForRead(writer, TLV::AnonymousTag(), provider.fabricIndex));

    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator().OTACurrentProvider(), buffer,
                                               static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR OTARequestorStorageImpl::ClearCurrentProviderLocation()
{
    return mPersistentStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator().OTACurrentProvider());
}

CHIP_ERROR OTARequestorStorageImpl::LoadCurrentProviderLocation(ProviderLocationType & provider)
{
    uint8_t buffer[kProviderMaxSerializedSize];
    uint16_t size = sizeof(buffer);

    ReturnErrorOnFailure(mPersistentStorage->SyncGetKeyValue(DefaultStorageKeyAllocator().OTACurrentProvider(), buffer, size));

    TLV::TLVReader reader;

    reader.Init(buffer, size);
    ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
    ReturnErrorOnFailure(provider.Decode(reader));

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestorStorageImpl::StoreUpdateToken(ByteSpan updateToken)
{
    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator().OTAUpdateToken(), updateToken.data(),
                                               static_cast<uint16_t>(updateToken.size()));
}

CHIP_ERROR OTARequestorStorageImpl::ClearUpdateToken()
{
    return mPersistentStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator().OTAUpdateToken());
}

CHIP_ERROR OTARequestorStorageImpl::LoadUpdateToken(MutableByteSpan & updateToken)
{
    uint16_t size    = static_cast<uint16_t>(updateToken.size());
    CHIP_ERROR error = mPersistentStorage->SyncGetKeyValue(DefaultStorageKeyAllocator().OTAUpdateToken(), updateToken.data(), size);

    updateToken.reduce_size(size);
    return error;
}

} // namespace chip
