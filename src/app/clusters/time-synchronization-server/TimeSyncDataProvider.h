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

#include <app/util/af-types.h>
#include <app/util/config.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/TLV.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/Span.h>

namespace chip {

class TimeSyncDataProvider
{

public:
    using TrustedTimeSource = app::Clusters::TimeSynchronization::Structs::TrustedTimeSourceStruct::Type;
    using DefaultNtp        = app::Clusters::TimeSynchronization::Attributes::DefaultNTP::TypeInfo::Type;
    using TimeZone          = app::DataModel::List<app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type>;
    using DSTOffset         = app::DataModel::List<app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type>;

    ~TimeSyncDataProvider() {}

    void Init(PersistentStorageDelegate & persistentStorage) { mPersistentStorage = &persistentStorage; }

    CHIP_ERROR StoreTrustedTimeSource(const TrustedTimeSource & timeSource);
    CHIP_ERROR LoadTrustedTimeSource(TrustedTimeSource & timeSource);
    CHIP_ERROR ClearTrustedTimeSource();

    CHIP_ERROR StoreDefaultNtp(const CharSpan & defaultNtp);
    CHIP_ERROR LoadDefaultNtp(MutableByteSpan & defaultNtp);
    CHIP_ERROR ClearDefaultNtp();

    CHIP_ERROR StoreTimeZone(const TimeZone & timeZoneList);
    CHIP_ERROR LoadTimeZone(TimeZone & timeZoneList, uint8_t & size);
    CHIP_ERROR ClearTimeZone();

    CHIP_ERROR StoreDSTOffset(const DSTOffset & dstOffsetList);
    CHIP_ERROR LoadDSTOffset(DSTOffset & dstOffsetList, uint8_t & size);
    CHIP_ERROR ClearDSTOffset();

private:
    CHIP_ERROR Load(const char * key, MutableByteSpan & buffer);
    PersistentStorageDelegate * mPersistentStorage = nullptr;
};

} // namespace chip
