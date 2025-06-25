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
#include <lib/core/TLV.h>
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

    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::OTADefaultProviders().KeyName(), buffer,
                                               static_cast<uint16_t>(writer.GetLengthWritten()));
}

bool DefaultOTARequestorStorage::ProviderAlreadyInList(ProviderLocationList & listProviders, ProviderLocationType provider)
{
    auto iterator = listProviders.Begin();

    while (iterator.Next())
    {
        ProviderLocationType pl = iterator.GetValue();

        if ((pl.providerNodeID == provider.providerNodeID) && (pl.fabricIndex == provider.fabricIndex) &&
            (pl.endpoint == provider.endpoint))
        {
            return true;
        }
    }

    return false;
}

CHIP_ERROR DefaultOTARequestorStorage::LoadDefaultProviders(ProviderLocationList & providers)
{
    uint8_t buffer[kProviderListMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);

    ReturnErrorOnFailure(Load(DefaultStorageKeyAllocator::OTADefaultProviders().KeyName(), bufferSpan));

    TLV::TLVReader reader;
    TLV::TLVType outerType;

    reader.Init(bufferSpan.data(), bufferSpan.size());
    ReturnErrorOnFailure(reader.Next(TLV::TLVType::kTLVType_Array, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    while (reader.Next() != CHIP_ERROR_END_OF_TLV)
    {
        ProviderLocationType provider;
        ReturnErrorOnFailure(provider.Decode(reader));
        if (!ProviderAlreadyInList(providers, provider))
        {
            providers.Add(provider);
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultOTARequestorStorage::StoreCurrentProviderLocation(const ProviderLocationType & provider)
{
    uint8_t buffer[kProviderMaxSerializedSize];
    TLV::TLVWriter writer;

    writer.Init(buffer);
    ReturnErrorOnFailure(provider.EncodeForRead(writer, TLV::AnonymousTag(), provider.fabricIndex));

    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::OTACurrentProvider().KeyName(), buffer,
                                               static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR DefaultOTARequestorStorage::ClearCurrentProviderLocation()
{
    return mPersistentStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::OTACurrentProvider().KeyName());
}

CHIP_ERROR DefaultOTARequestorStorage::LoadCurrentProviderLocation(ProviderLocationType & provider)
{
    uint8_t buffer[kProviderMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);

    ReturnErrorOnFailure(Load(DefaultStorageKeyAllocator::OTACurrentProvider().KeyName(), bufferSpan));

    TLV::TLVReader reader;

    reader.Init(bufferSpan.data(), bufferSpan.size());
    ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
    ReturnErrorOnFailure(provider.Decode(reader));

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultOTARequestorStorage::StoreUpdateToken(ByteSpan updateToken)
{
    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::OTAUpdateToken().KeyName(), updateToken.data(),
                                               static_cast<uint16_t>(updateToken.size()));
}

CHIP_ERROR DefaultOTARequestorStorage::ClearUpdateToken()
{
    return mPersistentStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::OTAUpdateToken().KeyName());
}

CHIP_ERROR DefaultOTARequestorStorage::LoadUpdateToken(MutableByteSpan & updateToken)
{
    return Load(DefaultStorageKeyAllocator::OTAUpdateToken().KeyName(), updateToken);
}

CHIP_ERROR DefaultOTARequestorStorage::StoreCurrentUpdateState(OTAUpdateStateEnum currentUpdateState)
{
    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::OTACurrentUpdateState().KeyName(), &currentUpdateState,
                                               sizeof(currentUpdateState));
}

CHIP_ERROR DefaultOTARequestorStorage::LoadCurrentUpdateState(OTAUpdateStateEnum & currentUpdateState)
{
    uint16_t size = static_cast<uint16_t>(sizeof(currentUpdateState));

    return mPersistentStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::OTACurrentUpdateState().KeyName(), &currentUpdateState,
                                               size);
}

CHIP_ERROR DefaultOTARequestorStorage::ClearCurrentUpdateState()
{
    return mPersistentStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::OTACurrentUpdateState().KeyName());
}

CHIP_ERROR DefaultOTARequestorStorage::StoreTargetVersion(uint32_t targetVersion)
{
    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::OTATargetVersion().KeyName(), &targetVersion,
                                               sizeof(targetVersion));
}

CHIP_ERROR DefaultOTARequestorStorage::LoadTargetVersion(uint32_t & targetVersion)
{
    uint16_t size = static_cast<uint16_t>(sizeof(targetVersion));

    return mPersistentStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::OTATargetVersion().KeyName(), &targetVersion, size);
}

CHIP_ERROR DefaultOTARequestorStorage::ClearTargetVersion()
{
    return mPersistentStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::OTATargetVersion().KeyName());
}

CHIP_ERROR DefaultOTARequestorStorage::Load(const char * key, MutableByteSpan & buffer)
{
    uint16_t size = static_cast<uint16_t>(buffer.size());
    ReturnErrorOnFailure(mPersistentStorage->SyncGetKeyValue(key, buffer.data(), size));

    buffer = MutableByteSpan(buffer.data(), size);
    return CHIP_NO_ERROR;
}

} // namespace chip
