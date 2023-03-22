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

#include "TimeSyncDataProvider.h"

#include <app/server/Server.h>
#include <app/util/af-types.h>
#include <app/util/config.h>
#include <lib/core/TLV.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TimeSynchronization {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TimeSynchronization;
using namespace chip::app::Clusters::TimeSynchronization::Attributes;
using chip::TimeSyncDataProvider;
using chip::Protocols::InteractionModel::Status;

class TimeSynchronizationServer
{
    struct timeZoneName
    {
        char name[64];
    };

public:
    void Init();

    static TimeSynchronizationServer & Instance(void);
    TimeSyncDataProvider GetDataProvider(void) { return mTimeSyncDataProvider; }

    CHIP_ERROR SetTrustedTimeSource(DataModel::Nullable<TimeSynchronization::Structs::TrustedTimeSourceStruct::Type> tts);
    CHIP_ERROR SetDefaultNtp(DataModel::Nullable<chip::CharSpan> & dntp);
    /**
     * @brief Sets TimeZone Attribute. Assumes the size of the list is already validated.
     *
     * @param tz TimeZone list
     * @return CHIP_ERROR
     */
    CHIP_ERROR SetTimeZone(DataModel::DecodableList<TimeSynchronization::Structs::TimeZoneStruct::Type> tz);
    /**
     * @brief Sets DstOffset Attribute. Assumes the size of the list is already validated.
     *
     * @param dst DstOffset list
     * @return CHIP_ERROR
     */
    CHIP_ERROR SetDSTOffset(DataModel::DecodableList<TimeSynchronization::Structs::DSTOffsetStruct::Type> dst);
    CHIP_ERROR ClearDSTOffset(void);
    DataModel::Nullable<TimeSynchronization::Structs::TrustedTimeSourceStruct::Type> & GetTrustedTimeSource(void);
    CHIP_ERROR GetDefaultNtp(MutableByteSpan & dntp);
    DataModel::List<TimeSynchronization::Structs::TimeZoneStruct::Type> & GetTimeZone(void);
    DataModel::List<TimeSynchronization::Structs::DSTOffsetStruct::Type> & GetDSTOffset(void);

    void ScheduleDelayedAction(System::Clock::Seconds32 delay, System::TimerCompleteCallback action, void * aAppState);

private:
    DataModel::Nullable<TimeSynchronization::Structs::TrustedTimeSourceStruct::Type> mTrustedTimeSource;
    DataModel::List<TimeSynchronization::Structs::TimeZoneStruct::Type> mTimeZoneList =
        DataModel::List<TimeSynchronization::Structs::TimeZoneStruct::Type>(mTz);
    DataModel::List<TimeSynchronization::Structs::DSTOffsetStruct::Type> mDstOffsetList =
        DataModel::List<TimeSynchronization::Structs::DSTOffsetStruct::Type>(mDst);

    uint8_t mTimeZoneListSize  = 0;
    uint8_t mDstOffsetListSize = 0;

    TimeSynchronization::Structs::TimeZoneStruct::Type mTz[CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE];
    struct timeZoneName mNames[CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE];
    TimeSynchronization::Structs::DSTOffsetStruct::Type mDst[CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE];

    TimeSyncDataProvider mTimeSyncDataProvider;
    static TimeSynchronizationServer mTimeSyncInstance;
};

} // namespace TimeSynchronization
} // namespace Clusters
} // namespace app
} // namespace chip
