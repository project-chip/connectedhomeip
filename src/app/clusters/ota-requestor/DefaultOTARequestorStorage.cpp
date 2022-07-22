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

#include "DefaultOTARequestorStorage.h"
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

CHIP_ERROR DefaultOTARequestorStorage::StoreDefaultProviders(const ProviderLocationList & providers)
{
    DefaultStorageKeyAllocator key;
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

    return mPersistentStorage->SyncSetKeyValue(key.OTADefaultProviders(), buffer, static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR DefaultOTARequestorStorage::LoadDefaultProviders(ProviderLocationList & providers)
{
    DefaultStorageKeyAllocator key;
    uint8_t buffer[kProviderListMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);

    ReturnErrorOnFailure(Load(key.OTADefaultProviders(), bufferSpan));

    TLV::TLVReader reader;
    TLV::TLVType outerType;

    reader.Init(bufferSpan.data(), bufferSpan.size());
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

CHIP_ERROR DefaultOTARequestorStorage::StoreCurrentProviderLocation(const ProviderLocationType & provider)
{
    DefaultStorageKeyAllocator key;
    uint8_t buffer[kProviderMaxSerializedSize];
    TLV::TLVWriter writer;

    writer.Init(buffer);
    ReturnErrorOnFailure(provider.EncodeForRead(writer, TLV::AnonymousTag(), provider.fabricIndex));

    return mPersistentStorage->SyncSetKeyValue(key.OTACurrentProvider(), buffer, static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR DefaultOTARequestorStorage::ClearCurrentProviderLocation()
{
    DefaultStorageKeyAllocator key;

    return mPersistentStorage->SyncDeleteKeyValue(key.OTACurrentProvider());
}

CHIP_ERROR DefaultOTARequestorStorage::LoadCurrentProviderLocation(ProviderLocationType & provider)
{
    DefaultStorageKeyAllocator key;
    uint8_t buffer[kProviderMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);

    ReturnErrorOnFailure(Load(key.OTACurrentProvider(), bufferSpan));

    TLV::TLVReader reader;

    reader.Init(bufferSpan.data(), bufferSpan.size());
    ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
    ReturnErrorOnFailure(provider.Decode(reader));

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultOTARequestorStorage::StoreUpdateToken(ByteSpan updateToken)
{
    DefaultStorageKeyAllocator key;

    return mPersistentStorage->SyncSetKeyValue(key.OTAUpdateToken(), updateToken.data(), static_cast<uint16_t>(updateToken.size()));
}

CHIP_ERROR DefaultOTARequestorStorage::ClearUpdateToken()
{
    DefaultStorageKeyAllocator key;

    return mPersistentStorage->SyncDeleteKeyValue(key.OTAUpdateToken());
}

CHIP_ERROR DefaultOTARequestorStorage::LoadUpdateToken(MutableByteSpan & updateToken)
{
    DefaultStorageKeyAllocator key;

    return Load(key.OTAUpdateToken(), updateToken);
}

CHIP_ERROR DefaultOTARequestorStorage::StoreCurrentUpdateState(OTAUpdateStateEnum currentUpdateState)
{
    DefaultStorageKeyAllocator key;

    return mPersistentStorage->SyncSetKeyValue(key.OTACurrentUpdateState(), &currentUpdateState, sizeof(currentUpdateState));
}

CHIP_ERROR DefaultOTARequestorStorage::LoadCurrentUpdateState(OTAUpdateStateEnum & currentUpdateState)
{
    DefaultStorageKeyAllocator key;
    uint16_t size = static_cast<uint16_t>(sizeof(currentUpdateState));

    return mPersistentStorage->SyncGetKeyValue(key.OTACurrentUpdateState(), &currentUpdateState, size);
}

CHIP_ERROR DefaultOTARequestorStorage::ClearCurrentUpdateState()
{
    DefaultStorageKeyAllocator key;

    return mPersistentStorage->SyncDeleteKeyValue(key.OTACurrentUpdateState());
}

CHIP_ERROR DefaultOTARequestorStorage::StoreTargetVersion(uint32_t targetVersion)
{
    DefaultStorageKeyAllocator key;

    return mPersistentStorage->SyncSetKeyValue(key.OTATargetVersion(), &targetVersion, sizeof(targetVersion));
}

CHIP_ERROR DefaultOTARequestorStorage::LoadTargetVersion(uint32_t & targetVersion)
{
    DefaultStorageKeyAllocator key;
    uint16_t size = static_cast<uint16_t>(sizeof(targetVersion));

    return mPersistentStorage->SyncGetKeyValue(key.OTATargetVersion(), &targetVersion, size);
}

CHIP_ERROR DefaultOTARequestorStorage::ClearTargetVersion()
{
    DefaultStorageKeyAllocator key;

    return mPersistentStorage->SyncDeleteKeyValue(key.OTATargetVersion());
}

CHIP_ERROR DefaultOTARequestorStorage::Load(const char * key, MutableByteSpan & buffer)
{
    uint16_t size = static_cast<uint16_t>(buffer.size());
    ReturnErrorOnFailure(mPersistentStorage->SyncGetKeyValue(key, buffer.data(), size));

    buffer = MutableByteSpan(buffer.data(), size);
    return CHIP_NO_ERROR;
}

} // namespace chip
