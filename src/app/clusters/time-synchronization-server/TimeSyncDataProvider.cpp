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

#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/TLV.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

#include <limits>

namespace chip {

// Calculated with Python code:
//   w = TLVWriter()
//   dict([(0 , 0xff),(1, bytearray([0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff])), (2, 0xffff), ])
//   w.put(None, s)
//   len(w.encoding)
constexpr size_t kTrustedTimeSourceMaxSerializedSize = 23u + 9u;   // adding 9 as buffer
constexpr size_t kTimeZoneMaxSerializedSize          = 90u + 6u;   // adding 6 as buffer
constexpr size_t kDSTOffsetMaxSerializedSize         = 34u + 6u;   // adding 6 as buffer

// user TZ feature #if 1
// Multiply the serialized size by the maximum number of list size and add 2 bytes for the array start and end.
constexpr size_t kTimeZoneListMaxSerializedSize = kTimeZoneMaxSerializedSize * CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE + 2;
constexpr size_t kDSTOffsetListMaxSerializedSize = kDSTOffsetMaxSerializedSize * CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE + 2;

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

    reader.Init(bufferSpan.data(), bufferSpan.size());
    ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
    ReturnErrorOnFailure(timeSource.Decode(reader));

    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeSyncDataProvider::ClearTrustedTimeSource()
{
    return mPersistentStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::TSTrustedTimeSource().KeyName());
}

CHIP_ERROR TimeSyncDataProvider::StoreDefaultNtp(const CharSpan & defaultNtp)
{
    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::TSDefaultNTP().KeyName(), defaultNtp.data(),
                                               static_cast<uint16_t>(defaultNtp.size()));
}

CHIP_ERROR TimeSyncDataProvider::LoadDefaultNtp(MutableByteSpan & defaultNtp)
{
    return Load(DefaultStorageKeyAllocator::TSDefaultNTP().KeyName(), defaultNtp);
}

CHIP_ERROR TimeSyncDataProvider::ClearDefaultNtp()
{
    return mPersistentStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::TSDefaultNTP().KeyName());
}

CHIP_ERROR TimeSyncDataProvider::StoreTimeZone(const TimeZone & timeZoneList)
{
    uint8_t buffer[kTimeZoneListMaxSerializedSize];
    TLV::TLVWriter writer;
    TLV::TLVType outerType;

    writer.Init(buffer);
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType));

    for (auto timeZoneIter : timeZoneList)
    {
        ReturnErrorOnFailure(timeZoneIter.Encode(writer, TLV::AnonymousTag()));
    }

    ReturnErrorOnFailure(writer.EndContainer(outerType));

    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::TSTimeZone().KeyName(), buffer,
                                               static_cast<uint16_t>(writer.GetLengthWritten()));
}
CHIP_ERROR TimeSyncDataProvider::LoadTimeZone(TimeZone & timeZoneList, uint8_t & size)
{
    uint8_t buffer[kTimeZoneListMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);
    size = 0;

    ReturnErrorOnFailure(Load(DefaultStorageKeyAllocator::TSTimeZone().KeyName(), bufferSpan));

    TLV::TLVReader reader;
    TLV::TLVType outerType;

    reader.Init(bufferSpan.data(), bufferSpan.size());
    ReturnErrorOnFailure(reader.Next(TLV::TLVType::kTLVType_Array, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerType));
    auto tz  = timeZoneList.begin();
    uint8_t i = 0;

    while (reader.Next() != CHIP_ERROR_END_OF_TLV && i < timeZoneList.size())
    {
        app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type timeZone;
        ReturnErrorOnFailure(timeZone.Decode(reader));
        tz[i].offset  = timeZone.offset;
        tz[i].validAt = timeZone.validAt;
        if (timeZone.name.HasValue())
        {
            char * dest = const_cast<char *>(tz[i].name.Value().data());
            size_t len  = timeZone.name.Value().size();
            Platform::CopyString(dest, len, timeZone.name.Value().data());
        }
        i++;
    }

    ReturnErrorOnFailure(reader.ExitContainer(outerType));
    size = i;

    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeSyncDataProvider::ClearTimeZone()
{
    return mPersistentStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::TSTimeZone().KeyName());
}

CHIP_ERROR TimeSyncDataProvider::StoreDSTOffset(const DSTOffset & dstOffsetList)
{
    uint8_t buffer[kDSTOffsetListMaxSerializedSize];
    TLV::TLVWriter writer;
    TLV::TLVType outerType;

    writer.Init(buffer);
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType));

    for (auto dstIter = dstOffsetList.begin(); dstIter != dstOffsetList.end(); dstIter++)
    {
        ReturnErrorOnFailure(dstIter->Encode(writer, TLV::AnonymousTag()));
    }

    ReturnErrorOnFailure(writer.EndContainer(outerType));

    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::TSDSTOffset().KeyName(), buffer,
                                               static_cast<uint16_t>(writer.GetLengthWritten()));
    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeSyncDataProvider::LoadDSTOffset(DSTOffset & dstOffsetList, uint8_t & size)
{
    uint8_t buffer[kDSTOffsetListMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);
    size = 0;

    ReturnErrorOnFailure(Load(DefaultStorageKeyAllocator::TSDSTOffset().KeyName(), bufferSpan));

    TLV::TLVReader reader;
    TLV::TLVType outerType;

    reader.Init(bufferSpan.data(), bufferSpan.size());
    ReturnErrorOnFailure(reader.Next(TLV::TLVType::kTLVType_Array, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerType));
    auto dst = dstOffsetList.begin();
    uint8_t i = 0;

    while (reader.Next() != CHIP_ERROR_END_OF_TLV && i < dstOffsetList.size())
    {
        app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type dstOffset;
        ReturnErrorOnFailure(dstOffset.Decode(reader));
        dst[i] = dstOffset;
        i++;
    }

    ReturnErrorOnFailure(reader.ExitContainer(outerType));
    size = i;

    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeSyncDataProvider::ClearDSTOffset()
{
    return mPersistentStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::TSDSTOffset().KeyName());
}

CHIP_ERROR TimeSyncDataProvider::Load(const char * key, MutableByteSpan & buffer)
{
    uint16_t size = static_cast<uint16_t>(buffer.size());
    ReturnErrorOnFailure(mPersistentStorage->SyncGetKeyValue(key, buffer.data(), size));

    buffer = MutableByteSpan(buffer.data(), size);
    return CHIP_NO_ERROR;
}

} // namespace chip
