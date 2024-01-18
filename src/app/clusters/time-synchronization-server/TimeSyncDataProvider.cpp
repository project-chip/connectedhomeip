/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "TimeSyncDataProvider.h"
#include <lib/support/DefaultStorageKeyAllocator.h>

#include <lib/core/TLV.h>
namespace chip {

constexpr size_t kTrustedTimeSourceMaxSerializedSize =
    TLV::EstimateStructOverhead(sizeof(FabricIndex), sizeof(NodeId), sizeof(EndpointId));
constexpr size_t kTimeZoneMaxSerializedSize =
    TLV::EstimateStructOverhead(sizeof(int32_t), sizeof(uint64_t), TimeSyncDataProvider::kTimeZoneNameLength);
constexpr size_t kDSTOffsetMaxSerializedSize = TLV::EstimateStructOverhead(sizeof(int32_t), sizeof(uint64_t), sizeof(uint64_t));

// Multiply the serialized size by the maximum number of list size and add 2 bytes for the array start and end.
constexpr size_t kTimeZoneListMaxSerializedSize =
    kTimeZoneMaxSerializedSize * CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE + TLV::EstimateStructOverhead();
constexpr size_t kDSTOffsetListMaxSerializedSize =
    kDSTOffsetMaxSerializedSize * CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE + TLV::EstimateStructOverhead();

CHIP_ERROR TimeSyncDataProvider::StoreTrustedTimeSource(const TrustedTimeSource & timeSource)
{
    uint8_t buffer[kTrustedTimeSourceMaxSerializedSize];
    TLV::TLVWriter writer;

    writer.Init(buffer);
    ReturnErrorOnFailure(timeSource.Encode(writer, TLV::AnonymousTag()));

    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::TSTrustedTimeSource().KeyName(), buffer,
                                               static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR TimeSyncDataProvider::LoadTrustedTimeSource(TrustedTimeSource & timeSource)
{
    uint8_t buffer[kTrustedTimeSourceMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);

    ReturnErrorOnFailure(Load(DefaultStorageKeyAllocator::TSTrustedTimeSource().KeyName(), bufferSpan));

    TLV::TLVReader reader;

    reader.Init(bufferSpan);
    ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
    ReturnErrorOnFailure(timeSource.Decode(reader));

    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeSyncDataProvider::ClearTrustedTimeSource()
{
    return Clear(DefaultStorageKeyAllocator::TSTrustedTimeSource().KeyName());
}

CHIP_ERROR TimeSyncDataProvider::StoreDefaultNtp(const CharSpan & defaultNtp)
{
    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::TSDefaultNTP().KeyName(), defaultNtp.data(),
                                               static_cast<uint16_t>(defaultNtp.size()));
}

CHIP_ERROR TimeSyncDataProvider::LoadDefaultNtp(MutableCharSpan & defaultNtp)
{
    MutableByteSpan byteSpan(Uint8::from_char(defaultNtp.data()), defaultNtp.size());
    ReturnErrorOnFailure(Load(DefaultStorageKeyAllocator::TSDefaultNTP().KeyName(), byteSpan));
    defaultNtp.reduce_size(byteSpan.size());
    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeSyncDataProvider::ClearDefaultNtp()
{
    return Clear(DefaultStorageKeyAllocator::TSDefaultNTP().KeyName());
}

CHIP_ERROR TimeSyncDataProvider::StoreTimeZone(const Span<TimeZoneStore> & timeZoneList)
{
    uint8_t buffer[kTimeZoneListMaxSerializedSize];
    TLV::TLVWriter writer;
    TLV::TLVType outerType;

    writer.Init(buffer);
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType));

    for (auto const & tzStore : timeZoneList)
    {
        ReturnErrorOnFailure(tzStore.timeZone.Encode(writer, TLV::AnonymousTag()));
    }

    ReturnErrorOnFailure(writer.EndContainer(outerType));

    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::TSTimeZone().KeyName(), buffer,
                                               static_cast<uint16_t>(writer.GetLengthWritten()));
}
CHIP_ERROR TimeSyncDataProvider::LoadTimeZone(TimeZoneObj & timeZoneObj)
{
    uint8_t buffer[kTimeZoneListMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);
    CHIP_ERROR err;
    timeZoneObj.validSize = 0;
    auto & tzStoreList    = timeZoneObj.timeZoneList;

    ReturnErrorOnFailure(Load(DefaultStorageKeyAllocator::TSTimeZone().KeyName(), bufferSpan));

    TLV::TLVReader reader;
    TLV::TLVType outerType;
    size_t count, i = 0;

    reader.Init(bufferSpan);
    ReturnErrorOnFailure(reader.Next(TLV::TLVType::kTLVType_Array, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerType));
    reader.CountRemainingInContainer(&count);
    VerifyOrReturnError(count <= tzStoreList.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        auto & tzStore = tzStoreList[i];
        app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type tz;
        ReturnErrorOnFailure(tz.Decode(reader));
        tzStore.timeZone.offset  = tz.offset;
        tzStore.timeZone.validAt = tz.validAt;
        if (tz.name.HasValue())
        {
            MutableCharSpan tempSpan(tzStore.name);
            ReturnErrorOnFailure(CopyCharSpanToMutableCharSpan(tz.name.Value(), tempSpan));
            tzStore.timeZone.name.SetValue(tempSpan);
        }
        else
        {
            tzStore.timeZone.name.ClearValue();
        }
        i++;
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerType));
    err = reader.Next();
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);

    timeZoneObj.validSize = i;

    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeSyncDataProvider::ClearTimeZone()
{
    return Clear(DefaultStorageKeyAllocator::TSTimeZone().KeyName());
}

CHIP_ERROR TimeSyncDataProvider::StoreDSTOffset(const DSTOffsets & dstOffsetList)
{
    uint8_t buffer[kDSTOffsetListMaxSerializedSize];
    TLV::TLVWriter writer;
    TLV::TLVType outerType;

    writer.Init(buffer);
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType));

    for (auto const & dstIter : dstOffsetList)
    {
        ReturnErrorOnFailure(dstIter.Encode(writer, TLV::AnonymousTag()));
    }

    ReturnErrorOnFailure(writer.EndContainer(outerType));

    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::TSDSTOffset().KeyName(), buffer,
                                               static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR TimeSyncDataProvider::LoadDSTOffset(DSTOffsetObj & dstOffsetObj)
{
    uint8_t buffer[kDSTOffsetListMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);
    CHIP_ERROR err;
    dstOffsetObj.validSize = 0;

    ReturnErrorOnFailure(Load(DefaultStorageKeyAllocator::TSDSTOffset().KeyName(), bufferSpan));

    TLV::TLVReader reader;
    TLV::TLVType outerType;
    size_t count, i = 0;

    reader.Init(bufferSpan);
    ReturnErrorOnFailure(reader.Next(TLV::TLVType::kTLVType_Array, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerType));
    reader.CountRemainingInContainer(&count);
    VerifyOrReturnError(count <= dstOffsetObj.dstOffsetList.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    auto dst = dstOffsetObj.dstOffsetList.begin();

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        ReturnErrorOnFailure(dst[i].Decode(reader));
        i++;
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerType));
    err = reader.Next();
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    dstOffsetObj.validSize = i;

    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeSyncDataProvider::ClearDSTOffset()
{
    return Clear(DefaultStorageKeyAllocator::TSDSTOffset().KeyName());
}

CHIP_ERROR TimeSyncDataProvider::Load(const char * key, MutableByteSpan & buffer)
{
    uint16_t size = static_cast<uint16_t>(buffer.size());
    ReturnErrorOnFailure(mPersistentStorage->SyncGetKeyValue(key, buffer.data(), size));

    buffer.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeSyncDataProvider::Clear(const char * key)
{
    CHIP_ERROR err = mPersistentStorage->SyncDeleteKeyValue(key);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // This value isn't in the storage yet, so consider it deleted
        return CHIP_NO_ERROR;
    }
    return err;
}

} // namespace chip
