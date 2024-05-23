/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
 * @file API declarations for time sync cluster.
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/Span.h>

namespace chip {

class TimeSyncDataProvider
{
    using TrustedTimeSource = chip::app::Clusters::TimeSynchronization::Structs::TrustedTimeSourceStruct::Type;
    using TimeZoneStruct    = chip::app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type;
    using DSTOffsets        = chip::app::DataModel::List<chip::app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type>;

public:
    static constexpr size_t kTimeZoneNameLength = 64;
    struct TimeZoneStore
    {
        TimeZoneStruct timeZone;
        char name[kTimeZoneNameLength];
    };
    struct TimeZoneObj
    {
        Span<TimeZoneStore> timeZoneList;
        size_t validSize;
    };
    struct DSTOffsetObj
    {
        DSTOffsets dstOffsetList;
        size_t validSize;
    };

    ~TimeSyncDataProvider() {}

    void Init(PersistentStorageDelegate & persistentStorage) { mPersistentStorage = &persistentStorage; }

    CHIP_ERROR StoreTrustedTimeSource(const TrustedTimeSource & timeSource);
    CHIP_ERROR LoadTrustedTimeSource(TrustedTimeSource & timeSource);
    CHIP_ERROR ClearTrustedTimeSource();

    CHIP_ERROR StoreDefaultNtp(const CharSpan & defaultNtp);
    CHIP_ERROR LoadDefaultNtp(MutableCharSpan & defaultNtp);
    CHIP_ERROR ClearDefaultNtp();

    CHIP_ERROR StoreTimeZone(const chip::Span<TimeZoneStore> & timeZoneList);
    CHIP_ERROR LoadTimeZone(TimeZoneObj & timeZoneObj);
    CHIP_ERROR ClearTimeZone();

    CHIP_ERROR StoreDSTOffset(const DSTOffsets & dstOffsetList);
    CHIP_ERROR LoadDSTOffset(DSTOffsetObj & dstOffsetObj);
    CHIP_ERROR ClearDSTOffset();

private:
    CHIP_ERROR Load(const char * key, MutableByteSpan & buffer);
    PersistentStorageDelegate * mPersistentStorage = nullptr;
    CHIP_ERROR Clear(const char * key);
};

} // namespace chip
