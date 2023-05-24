/*
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

#include "time-synchronization-server.h"
#include "TimeSyncManager.h"
#include "time-synchronization-delegate.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/EventLogging.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SortUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#include <app-common/zap-generated/cluster-enums.h>

#include <system/SystemClock.h>

using namespace chip;
using namespace chip::DeviceLayer;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TimeSynchronization;
using namespace chip::app::Clusters::TimeSynchronization::Attributes;
using chip::TimeSyncDataProvider;
using chip::Protocols::InteractionModel::Status;

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::TimeSynchronization::Delegate;

namespace {

Delegate * gDelegate = nullptr;

Delegate * GetDelegate()
{
    if (gDelegate == nullptr)
    {
        static TimeSyncManager dg;
        gDelegate = &dg;
    }
    return gDelegate;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace TimeSynchronization {

void SetDefaultDelegate(Delegate * delegate)
{
    gDelegate = delegate;
}

Delegate * GetDefaultDelegate()
{
    return GetDelegate();
}

} // namespace TimeSynchronization
} // namespace Clusters
} // namespace app
} // namespace chip

static bool ChipEpochToUnixEpochTime(uint32_t chipEpochTime, uint32_t & unixEpochTime)
{
    // VerifyOrReturnError(chipEpochTime <= kChipEpochSecondsSinceUnixEpoch, false);

    unixEpochTime = chipEpochTime + kChipEpochSecondsSinceUnixEpoch;

    return true;
}

static bool UnixEpochToChipEpochMicro(uint64_t unixEpochTime, uint64_t & chipEpochTime)
{
    uint32_t chipEpochSec;
    chip::UnixEpochToChipEpochTime((uint32_t)(unixEpochTime / chip::kMicrosecondsPerSecond), chipEpochSec);
    chipEpochTime = (uint64_t) chipEpochSec * chip::kMicrosecondsPerSecond;

    return true;
}

static CHIP_ERROR utcTimeChanged(uint64_t utcTime)
{
    System::Clock::Seconds32 lastKnownGoodChipEpoch;
    uint32_t chipEpochInUtcTime;

    ReturnErrorOnFailure(Server::GetInstance().GetFabricTable().GetLastKnownGoodChipEpochTime(lastKnownGoodChipEpoch));
    ChipEpochToUnixEpochTime((uint32_t)(utcTime / chip::kMicrosecondsPerSecond), chipEpochInUtcTime);

    // if ( (utcTime / chip::kMicrosecondsPerSecond) < lastKnownGoodChipEpoch) return CHIP_ERROR_INVALID_TIME;
    ReturnErrorOnFailure(
        Server::GetInstance().GetFabricTable().SetLastKnownGoodChipEpochTime(System::Clock::Seconds32(chipEpochInUtcTime)));
    ReturnErrorOnFailure(System::SystemClock().SetClock_RealTime(
        System::Clock::Microseconds64((uint64_t) chipEpochInUtcTime * chip::kMicrosecondsPerSecond)));

    return CHIP_NO_ERROR;
}

TimeSynchronizationServer TimeSynchronizationServer::mTimeSyncInstance;

TimeSynchronizationServer & TimeSynchronizationServer::Instance()
{
    return mTimeSyncInstance;
}

void TimeSynchronizationServer::Init()
{
    mTimeSyncDataProvider.Init(Server::GetInstance().GetPersistentStorage());

    TimeSynchronization::Structs::TrustedTimeSourceStruct::Type tts;
    (CHIP_NO_ERROR == mTimeSyncDataProvider.LoadTrustedTimeSource(tts)) ? (void) mTrustedTimeSource.SetNonNull(tts)
                                                                        : mTrustedTimeSource.SetNull();
    auto tz = mTimeZoneList.begin();
    for (size_t i = 0; i < mTimeZoneList.size(); i++)
    {
        memset(mNames[i].name, 0, sizeof(mNames[i].name));
        const char * buf = reinterpret_cast<const char *>(mNames[i].name);
        tz[i].name.SetValue(chip::CharSpan(buf, sizeof(mNames[i].name)));
    }
    mTimeSyncDataProvider.LoadTimeZone(mTimeZoneList, mTimeZoneListSize);
    if (mTimeZoneListSize == 0) // initialize default value
    {
        mTimeZoneList[0].offset  = 0;
        mTimeZoneList[0].validAt = 0;
        mTimeZoneListSize        = 1;
    }
    mTimeSyncDataProvider.LoadDSTOffset(mDstOffsetList, mDstOffsetListSize);
    // TODO if trusted time source is available schedule a time read
    if (!mTrustedTimeSource.IsNull())
    {
    }
    System::Clock::Microseconds64 utcTime;
    if (System::SystemClock().GetClock_RealTime(utcTime) == CHIP_NO_ERROR)
    {
        mGranularity = TimeSynchronization::GranularityEnum::kMinutesGranularity;
    }
    else
    {
        mGranularity = TimeSynchronization::GranularityEnum::kNoTimeGranularity;
    }
}

CHIP_ERROR TimeSynchronizationServer::SetTrustedTimeSource(
    DataModel::Nullable<TimeSynchronization::Structs::TrustedTimeSourceStruct::Type> tts)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    mTrustedTimeSource = tts;
    if (!mTrustedTimeSource.IsNull())
    {
        err = mTimeSyncDataProvider.StoreTrustedTimeSource(mTrustedTimeSource.Value());
    }
    else
    {
        err = mTimeSyncDataProvider.ClearTrustedTimeSource();
    }
    return err;
}

CHIP_ERROR TimeSynchronizationServer::SetDefaultNtp(DataModel::Nullable<chip::CharSpan> & dntp)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (!dntp.IsNull())
    {
        err = mTimeSyncDataProvider.StoreDefaultNtp(dntp.Value());
    }
    else
    {
        err = mTimeSyncDataProvider.ClearDefaultNtp();
    }
    return err;
}

CHIP_ERROR TimeSynchronizationServer::SetTimeZone(DataModel::DecodableList<TimeSynchronization::Structs::TimeZoneStruct::Type> tz)
{
    size_t items;
    ReturnErrorOnFailure(tz.ComputeSize(&items));

    if (items > CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    auto tzL    = mTimeZoneList.begin();
    auto newTzL = tz.begin();
    uint8_t i   = 0;

    while (newTzL.Next() && i < CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE)
    {
        if (newTzL.GetValue().offset < -43200 || newTzL.GetValue().offset > 50400)
        {
            mTimeSyncDataProvider.LoadTimeZone(mTimeZoneList, mTimeZoneListSize);
            return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB;
        }
        // first element shall have validAt entry of 0
        if (i == 0 && newTzL.GetValue().validAt != 0)
        {
            return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB;
        }
        // if second element, it shall have validAt entry of non-0
        if (i == 1 && newTzL.GetValue().validAt == 0)
        {
            mTimeSyncDataProvider.LoadTimeZone(mTimeZoneList, mTimeZoneListSize);
            return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB;
        }
        tzL[i].offset  = newTzL.GetValue().offset;
        tzL[i].validAt = newTzL.GetValue().validAt;
        if (newTzL.GetValue().name.HasValue())
        {
            if (newTzL.GetValue().name.Value().size() > sizeof(mNames[i].name))
            {
                mTimeSyncDataProvider.LoadTimeZone(mTimeZoneList, mTimeZoneListSize);
                return CHIP_ERROR_BUFFER_TOO_SMALL;
            }
            const char * buf = newTzL.GetValue().name.Value().data();
            size_t len       = newTzL.GetValue().name.Value().size();
            memset(mNames[i].name, 0, sizeof(mNames[i].name));
            memcpy(mNames[i].name, buf, len);
            tzL[i].name.SetValue(chip::CharSpan(mNames[i].name, strnlen(mNames[i].name, sizeof(mNames[i].name))));
        }
        else
        {
            tzL[i].name.ClearValue();
        }
        i++;
    }
    ReturnErrorOnFailure(newTzL.GetStatus());
    mTimeZoneListSize = i;

    return mTimeSyncDataProvider.StoreTimeZone(TimeSynchronizationServer::Instance().GetTimeZone());
}

CHIP_ERROR
TimeSynchronizationServer::SetDSTOffset(DataModel::DecodableList<TimeSynchronization::Structs::DSTOffsetStruct::Type> dst)
{
    size_t items;
    ReturnErrorOnFailure(dst.ComputeSize(&items));

    if (items > CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    auto dstL    = mDstOffsetList.begin();
    auto newDstL = dst.begin();
    uint8_t i    = 0;

    while (newDstL.Next() && i < CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE)
    {
        dstL[i] = newDstL.GetValue();
        i++;
    }

    if (CHIP_NO_ERROR != newDstL.GetStatus())
    {
        mTimeSyncDataProvider.LoadDSTOffset(mDstOffsetList, mDstOffsetListSize);
        return newDstL.GetStatus();
    }

    mDstOffsetListSize = i;

    // sorted by ValidStarting time
    Sorting::BubbleSort(TimeSynchronizationServer::Instance().GetDSTOffset().begin(), mDstOffsetListSize,
                        [](TimeSynchronization::Structs::DSTOffsetStruct::Type a,
                           TimeSynchronization::Structs::DSTOffsetStruct::Type b) { return a.validStarting < b.validStarting; });

    // only 1 validuntil null value and shall be last in the list
    uint64_t lastValidUntil = 0;
    for (i = 0; i < mDstOffsetListSize; i++)
    {
        const auto & dstItem = TimeSynchronizationServer::Instance().GetDSTOffset()[i];
        // validUntil shall be larger than validStarting
        if (!dstItem.validUntil.IsNull() && dstItem.validStarting >= dstItem.validUntil.Value())
        {
            mTimeSyncDataProvider.LoadDSTOffset(mDstOffsetList, mDstOffsetListSize);
            return CHIP_ERROR_INVALID_TIME;
        }
        // validStarting shall not be smaller than validUntil of previous entry
        if (dstItem.validStarting < lastValidUntil)
        {
            mTimeSyncDataProvider.LoadDSTOffset(mDstOffsetList, mDstOffsetListSize);
            return CHIP_ERROR_INVALID_TIME;
        }
        lastValidUntil = !dstItem.validUntil.IsNull() ? dstItem.validUntil.Value() : lastValidUntil;
        // only 1 validuntil null value and shall be last in the list
        if (dstItem.validUntil.IsNull() && (i != mDstOffsetListSize - 1))
        {
            mTimeSyncDataProvider.LoadDSTOffset(mDstOffsetList, mDstOffsetListSize);
            return CHIP_ERROR_INVALID_TIME;
        }
    }

    return mTimeSyncDataProvider.StoreDSTOffset(TimeSynchronizationServer::Instance().GetDSTOffset());
}

CHIP_ERROR TimeSynchronizationServer::ClearDSTOffset()
{
    mDstOffsetListSize = 0;
    for (uint8_t i = 0; i < CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE; i++)
    {
        mDst[i] = {};
    }
    return mTimeSyncDataProvider.ClearDSTOffset();
}

DataModel::Nullable<TimeSynchronization::Structs::TrustedTimeSourceStruct::Type> & TimeSynchronizationServer::GetTrustedTimeSource()
{
    return mTrustedTimeSource;
}

CHIP_ERROR TimeSynchronizationServer::GetDefaultNtp(MutableByteSpan & dntp)
{
    return mTimeSyncDataProvider.LoadDefaultNtp(dntp);
}

DataModel::List<TimeSynchronization::Structs::TimeZoneStruct::Type> & TimeSynchronizationServer::GetTimeZone()
{
    mTimeZoneList = DataModel::List<TimeSynchronization::Structs::TimeZoneStruct::Type>(mTz, mTimeZoneListSize);
    return mTimeZoneList;
}

DataModel::List<TimeSynchronization::Structs::DSTOffsetStruct::Type> & TimeSynchronizationServer::GetDSTOffset()
{
    mDstOffsetList = DataModel::List<TimeSynchronization::Structs::DSTOffsetStruct::Type>(mDst, mDstOffsetListSize);
    return mDstOffsetList;
}

void TimeSynchronizationServer::ScheduleDelayedAction(System::Clock::Seconds32 delay, System::TimerCompleteCallback action,
                                                      void * aAppState)
{
    VerifyOrDie(SystemLayer().StartTimer(std::chrono::duration_cast<System::Clock::Timeout>(delay), action, aAppState) ==
                CHIP_NO_ERROR);
}

CHIP_ERROR TimeSynchronizationServer::SetUTCTime(chip::EndpointId ep, uint64_t utcTime,
                                                 TimeSynchronization::GranularityEnum granularity,
                                                 TimeSynchronization::TimeSourceEnum source)
{
    ReturnErrorOnFailure(utcTimeChanged(utcTime));
    mGranularity = granularity;
    TimeSource::Set(ep, source);
    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeSynchronizationServer::GetLocalTime(chip::EndpointId ep, uint64_t & localTime)
{
    uint64_t timeZoneOffset = 0, dstOffset = 0;
    System::Clock::Microseconds64 utcTime;
    uint64_t chipEpochTime;
    ReturnErrorOnFailure(System::SystemClock().GetClock_RealTime(utcTime));
    UnixEpochToChipEpochMicro(utcTime.count(), chipEpochTime);

    if (isTimeZoneAvailable())
    {
        auto tz        = TimeSynchronizationServer::Instance().GetTimeZone().begin();
        timeZoneOffset = chip::kMicrosecondsPerSecond * static_cast<uint64_t>(tz->offset);
    }
    VerifyOrReturnError(isDSTOffsetAvailable() == true, CHIP_ERROR_INVALID_TIME);
    auto dst  = TimeSynchronizationServer::Instance().GetDSTOffset().begin();
    dstOffset = chip::kMicrosecondsPerSecond * static_cast<uint64_t>(dst->offset);

    localTime = chipEpochTime + timeZoneOffset + dstOffset;
    return CHIP_NO_ERROR;
}

bool TimeSynchronizationServer::isTimeZoneAvailable()
{
    System::Clock::Microseconds64 utcTime;
    auto tzList           = TimeSynchronizationServer::Instance().GetTimeZone();
    uint8_t activeTzIndex = 0;
    uint64_t chipEpochTime;

    VerifyOrReturnValue(System::SystemClock().GetClock_RealTime(utcTime) == CHIP_NO_ERROR, false);
    VerifyOrReturnValue(tzList.size() != 0, false);
    UnixEpochToChipEpochMicro(utcTime.count(), chipEpochTime);

    for (uint8_t i = 0; i < tzList.size(); i++)
    {
        if (tzList[i].validAt != 0 && tzList[i].validAt <= chipEpochTime)
        {
            tzList[i].validAt = 0;
            activeTzIndex     = i;
        }
    }
    if (activeTzIndex != 0)
    {
        mTimeZoneListSize    = static_cast<uint8_t>(tzList.size() - activeTzIndex);
        auto newTimeZoneList = tzList.SubSpan(activeTzIndex);
        VerifyOrReturnValue(mTimeSyncDataProvider.StoreTimeZone(newTimeZoneList) == CHIP_NO_ERROR, false);
        VerifyOrReturnValue(mTimeSyncDataProvider.LoadTimeZone(TimeSynchronizationServer::Instance().GetTimeZone(),
                                                               mTimeZoneListSize) == CHIP_NO_ERROR,
                            false);
    }
    return true;
}

bool TimeSynchronizationServer::isDSTOffsetAvailable()
{
    System::Clock::Microseconds64 utcTime;
    uint8_t activeDstIndex = 0;
    auto dstList           = TimeSynchronizationServer::Instance().GetDSTOffset();
    uint64_t chipEpochTime;

    VerifyOrReturnValue(System::SystemClock().GetClock_RealTime(utcTime) == CHIP_NO_ERROR, false);
    VerifyOrReturnValue(dstList.size() != 0, false);
    UnixEpochToChipEpochMicro(utcTime.count(), chipEpochTime);

    for (uint8_t i = 0; i < dstList.size(); i++)
    {
        if (dstList[i].validStarting <= chipEpochTime)
        {
            activeDstIndex = i;
        }
    }
    // if offset is zero and validUntil is null then no DST is used
    if (dstList[activeDstIndex].offset == 0 && dstList[activeDstIndex].validUntil.IsNull())
    {
        return false;
    }
    if (activeDstIndex != 0)
    {
        mDstOffsetListSize    = static_cast<uint8_t>(dstList.size() - activeDstIndex);
        auto newDstOffsetList = dstList.SubSpan(activeDstIndex);
        VerifyOrReturnValue(mTimeSyncDataProvider.StoreDSTOffset(newDstOffsetList) == CHIP_NO_ERROR, false);
        VerifyOrReturnValue(mTimeSyncDataProvider.LoadDSTOffset(TimeSynchronizationServer::Instance().GetDSTOffset(),
                                                                mDstOffsetListSize) == CHIP_NO_ERROR,
                            false);
    }
    return true;
}

namespace {

class TimeSynchronizationAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the TimeSync cluster on all endpoints
    TimeSynchronizationAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), TimeSynchronization::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadTrustedTimeSource(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadDefaultNtp(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadTimeZone(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadDSTOffset(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadLocalTime(EndpointId endpoint, AttributeValueEncoder & aEncoder);
};

TimeSynchronizationAttrAccess gAttrAccess;

CHIP_ERROR TimeSynchronizationAttrAccess::ReadTrustedTimeSource(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    auto tts = TimeSynchronizationServer::Instance().GetTrustedTimeSource();
    return aEncoder.Encode(tts);
}

CHIP_ERROR TimeSynchronizationAttrAccess::ReadDefaultNtp(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t buffer[DefaultNTP::TypeInfo::MaxLength()];
    MutableByteSpan dntp(buffer);
    if (TimeSynchronizationServer::Instance().GetDefaultNtp(dntp) == CHIP_NO_ERROR && dntp.size() != 0)
    {
        const char * charBuf = reinterpret_cast<const char *>(buffer);
        err                  = aEncoder.Encode(chip::CharSpan(charBuf, dntp.size()));
    }
    else
    {
        err = aEncoder.EncodeNull();
    }
    return err;
}

CHIP_ERROR TimeSynchronizationAttrAccess::ReadTimeZone(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        TimeZone::TypeInfo::Type tzList = TimeSynchronizationServer::Instance().GetTimeZone();
        for (const auto & timeZone : tzList)
        {
            ReturnErrorOnFailure(encoder.Encode(timeZone));
        }

        return CHIP_NO_ERROR;
    });

    return err;
}

CHIP_ERROR TimeSynchronizationAttrAccess::ReadDSTOffset(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        DSTOffset::TypeInfo::Type dstList = TimeSynchronizationServer::Instance().GetDSTOffset();
        for (const auto & dstOffset : dstList)
        {
            ReturnErrorOnFailure(encoder.Encode(dstOffset));
        }

        return CHIP_NO_ERROR;
    });

    return err;
}

CHIP_ERROR TimeSynchronizationAttrAccess::ReadLocalTime(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    uint64_t localTime = 0;
    CHIP_ERROR err     = TimeSynchronizationServer::Instance().GetLocalTime(endpoint, localTime);
    if (err != CHIP_NO_ERROR)
    {
        err = aEncoder.EncodeNull();
    }
    else
    {
        err = aEncoder.Encode(localTime);
    }
    return err;
}

CHIP_ERROR TimeSynchronizationAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aPath.mClusterId != TimeSynchronization::Id)
    {
        return CHIP_ERROR_INVALID_PATH_LIST;
    }

    switch (aPath.mAttributeId)
    {
    case UTCTime::Id: {
        System::Clock::Microseconds64 utcTimeUnix;
        uint64_t chipEpochTime;

        VerifyOrReturnError(System::SystemClock().GetClock_RealTime(utcTimeUnix) == CHIP_NO_ERROR, aEncoder.EncodeNull());
        UnixEpochToChipEpochMicro(utcTimeUnix.count(), chipEpochTime);
        return aEncoder.Encode(chipEpochTime);
    }
    case Granularity::Id: {
        return aEncoder.Encode(TimeSynchronizationServer::Instance().GetGranularity());
    }
    case TrustedTimeSource::Id: {
        return ReadTrustedTimeSource(aPath.mEndpointId, aEncoder);
    }
    case DefaultNTP::Id: {
        return ReadDefaultNtp(aPath.mEndpointId, aEncoder);
    }
    case TimeZone::Id: {
        return ReadTimeZone(aPath.mEndpointId, aEncoder);
    }
    case DSTOffset::Id: {
        return ReadDSTOffset(aPath.mEndpointId, aEncoder);
    }
    case TimeZoneListMaxSize::Id: {
        uint8_t max = CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE;
        return aEncoder.Encode(max);
    }
    case DSTOffsetListMaxSize::Id: {
        uint8_t max = CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE;
        return aEncoder.Encode(max);
    }
    case LocalTime::Id: {
        return ReadLocalTime(aPath.mEndpointId, aEncoder);
    }
    default: {
        break;
    }
    }

    return err;
}
} // anonymous namespace

static bool sendDSTTableEmptyEvent(chip::EndpointId endpointId)
{
    Events::DSTTableEmpty::Type event;
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to send DSTTableEmpty event [endpointId=%d]", endpointId);
        return false;
    }
    ChipLogProgress(Zcl, "Emit DSTTableEmpty event [endpointId=%d]", endpointId);

    // re-schedule event for after min 1hr
    // delegate->scheduleDSTTableEmptyEvent()
    return true;
}

static bool sendDSTStatusEvent(chip::EndpointId endpointId, bool dstOffsetActive)
{
    Events::DSTStatus::Type event;
    event.DSTOffsetActive = dstOffsetActive;
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to send sendDSTStatus event [endpointId=%d]", endpointId);
        return false;
    }

    ChipLogProgress(Zcl, "Emit sendDSTStatus event [endpointId=%d]", endpointId);
    return true;
}

static bool sendTimeZoneStatusEvent(chip::EndpointId endpointId, uint8_t listIndex)
{
    Events::TimeZoneStatus::Type event;
    auto tz      = TimeSynchronizationServer::Instance().GetTimeZone().begin();
    event.offset = tz[listIndex].offset;
    if (tz[listIndex].name.HasValue())
    {
        event.name.SetValue(tz[listIndex].name.Value());
    }
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to send sendTimeZoneStatus event [endpointId=%d]", endpointId);
        return false;
    }

    ChipLogProgress(Zcl, "Emit sendTimeZoneStatus event [endpointId=%d]", endpointId);
    return true;
}

static bool sendTimeFailureEvent(chip::EndpointId endpointId)
{
    Events::TimeFailure::Type event;
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to send sendTimeFailure event [endpointId=%d]", endpointId);
        return false;
    }

    // re-schedule event for after min 1hr if no time is still available
    ChipLogProgress(Zcl, "Emit sendTimeFailure event [endpointId=%d]", endpointId);
    return true;
}

static bool sendMissingTrustedTimeSourceEvent(chip::EndpointId endpointId)
{
    Events::MissingTrustedTimeSource::Type event;
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to send sendMissingTrustedTimeSource event [endpointId=%d]", endpointId);
        return false;
    }

    // re-schedule event for after min 1hr if TTS is null or cannot be reached
    ChipLogProgress(Zcl, "Emit sendMissingTrustedTimeSource event [endpointId=%d]", endpointId);
    return true;
}

bool emberAfTimeSynchronizationClusterSetUTCTimeCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::TimeSynchronization::Commands::SetUTCTime::DecodableType & commandData)
{
    const auto & utcTime     = commandData.UTCTime;
    const auto & granularity = commandData.granularity;

    auto currentGranularity = TimeSynchronizationServer::Instance().GetGranularity();

    if ((currentGranularity == TimeSynchronization::GranularityEnum::kNoTimeGranularity || granularity >= currentGranularity) &&
        CHIP_NO_ERROR ==
            TimeSynchronizationServer::Instance().SetUTCTime(commandPath.mEndpointId, utcTime, granularity,
                                                             TimeSynchronization::TimeSourceEnum::kAdmin))
    {
        commandObj->AddStatus(commandPath, Status::Success);
    }
    else
    {
        commandObj->AddClusterSpecificFailure(commandPath, to_underlying(TimeSynchronization::StatusCode::kTimeNotAccepted));
    }
    return true;
}

bool emberAfTimeSynchronizationClusterSetTrustedTimeSourceCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::TimeSynchronization::Commands::SetTrustedTimeSource::DecodableType & commandData)
{
    const auto & timeSource = commandData.trustedTimeSource;
    DataModel::Nullable<TimeSynchronization::Structs::TrustedTimeSourceStruct::Type> tts;

    if (!timeSource.IsNull())
    {

        TimeSynchronization::Structs::TrustedTimeSourceStruct::Type ts = { commandObj->GetAccessingFabricIndex(),
                                                                           timeSource.Value().nodeID, timeSource.Value().endpoint };
        tts.SetNonNull(ts);
        // TODO schedule a utctime read from this time source
    }
    else
    {
        tts.SetNull();
        sendMissingTrustedTimeSourceEvent(commandPath.mEndpointId);
    }

    TimeSynchronizationServer::Instance().SetTrustedTimeSource(tts);
    commandObj->AddStatus(commandPath, Status::Success);
    return true;
}

bool emberAfTimeSynchronizationClusterSetTimeZoneCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::TimeSynchronization::Commands::SetTimeZone::DecodableType & commandData)
{
    const auto & timeZone = commandData.timeZone;
    uint64_t localTime;

    CHIP_ERROR err = TimeSynchronizationServer::Instance().SetTimeZone(timeZone);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            commandObj->AddStatus(commandPath, Status::ResourceExhausted);
        }
        else
        {
            commandObj->AddStatus(commandPath, Status::ConstraintError);
        }
        return true;
    }
    sendTimeZoneStatusEvent(commandPath.mEndpointId, 0);
    sendTimeFailureEvent(commandPath.mEndpointId); // TODO remove
    GetDelegate()->HandleTimeZoneChanged(TimeSynchronizationServer::Instance().GetTimeZone());
    GetDelegate()->HandleDstoffsetlookup();

    TimeSynchronization::TimeZoneDatabaseEnum tzDb;
    TimeZoneDatabase::Get(commandPath.mEndpointId, &tzDb);
    Commands::SetTimeZoneResponse::Type response;
    if (GetDelegate()->HasFeature(commandPath.mEndpointId, TimeSynchronization::Feature::kTimeZone) &&
        tzDb != TimeSynchronization::TimeZoneDatabaseEnum::kNone)
    {
        auto tzL = TimeSynchronizationServer::Instance().GetTimeZone();
        auto tz  = tzL.begin();
        if (tz != tzL.end() && tz->name.HasValue() && GetDelegate()->HandleDstoffsetavailable(tz->name.Value()))
        {
            GetDelegate()->HandleGetdstoffset();
            response.DSTOffsetRequired = false;
            sendDSTStatusEvent(commandPath.mEndpointId, true);
        }
        else
        {
            TimeSynchronizationServer::Instance().ClearDSTOffset();
            sendDSTTableEmptyEvent(commandPath.mEndpointId);
            sendDSTStatusEvent(commandPath.mEndpointId, false);
            response.DSTOffsetRequired = true;
        }
    }
    else
    {
        response.DSTOffsetRequired = true;
    }
    TimeSynchronizationServer::Instance().GetLocalTime(commandPath.mEndpointId, localTime);
    commandObj->AddResponse(commandPath, response);
    return true;
}

bool emberAfTimeSynchronizationClusterSetDSTOffsetCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::TimeSynchronization::Commands::SetDSTOffset::DecodableType & commandData)
{
    const auto & dstOffset = commandData.DSTOffset;

    CHIP_ERROR err = TimeSynchronizationServer::Instance().SetDSTOffset(dstOffset);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            commandObj->AddStatus(commandPath, Status::ResourceExhausted);
        }
        else
        {
            commandObj->AddStatus(commandPath, Status::ConstraintError);
        }
        return true;
    }
    // if DST state changes, generate DSTStatus event
    sendDSTStatusEvent(commandPath.mEndpointId, true);
    // if list is empty, generate DSTTableEmpty event
    sendDSTTableEmptyEvent(commandPath.mEndpointId);

    commandObj->AddStatus(commandPath, Status::Success);
    return true;
}

bool emberAfTimeSynchronizationClusterSetDefaultNTPCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::TimeSynchronization::Commands::SetDefaultNTP::DecodableType & commandData)
{
    Status status = Status::Success;
    auto dNtpChar = commandData.defaultNTP;

    if (!dNtpChar.IsNull())
    {
        if (!GetDelegate()->isNTPAddressValid(dNtpChar.Value()))
        {
            commandObj->AddStatus(commandPath, Status::InvalidCommand);
            return true;
        }
        if (GetDelegate()->isNTPAddressDomain(dNtpChar.Value()))
        {
            bool dnsResolve;
            SupportsDNSResolve::Get(commandPath.mEndpointId, &dnsResolve);
            if (!dnsResolve)
            {
                commandObj->AddStatus(commandPath, Status::InvalidCommand);
                return true;
            }
        }

        size_t len = dNtpChar.Value().size();

        if (len > DefaultNTP::TypeInfo::MaxLength())
        {
            commandObj->AddStatus(commandPath, Status::ConstraintError);
            return true;
        }
    }

    status =
        (CHIP_NO_ERROR == TimeSynchronizationServer::Instance().SetDefaultNtp(dNtpChar)) ? Status::Success : Status::InvalidCommand;

    commandObj->AddStatus(commandPath, status);
    return true;
}

void MatterTimeSynchronizationPluginServerInitCallback()
{
    static bool attrAccessRegistered = false;
    TimeSynchronizationServer::Instance().Init();
    if (!attrAccessRegistered)
    {
        attrAccessRegistered = true;
        registerAttributeAccessOverride(&gAttrAccess);
#if 0
        // System::SystemClock().SetClock_RealTime(System::Clock::Microseconds64(1679668000000000));
#endif
    }
}
