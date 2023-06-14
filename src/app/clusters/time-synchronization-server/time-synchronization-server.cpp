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
#include "DefaultTimeSyncDelegate.h"
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
using namespace chip::app;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TimeSynchronization;
using namespace chip::app::Clusters::TimeSynchronization::Attributes;
using chip::TimeSyncDataProvider;
using chip::Protocols::InteractionModel::Status;

// -----------------------------------------------------------------------------
// Delegate Implementation

namespace {

Delegate * gDelegate = nullptr;

Delegate * GetDelegate()
{
    if (gDelegate == nullptr)
    {
        static DefaultTimeSyncDelegate dg;
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

constexpr uint64_t kChipEpochUsSinceUnixEpoch =
    static_cast<uint64_t>(kChipEpochSecondsSinceUnixEpoch) * chip::kMicrosecondsPerSecond;

static bool ChipEpochToUnixEpochMicro(uint64_t chipEpochTime, uint64_t & unixEpochTime)
{
    unixEpochTime = chipEpochTime + kChipEpochUsSinceUnixEpoch;
    return true;
}

static bool UnixEpochToChipEpochMicro(uint64_t unixEpochTime, uint64_t & chipEpochTime)
{
    VerifyOrReturnValue(unixEpochTime >= kChipEpochUsSinceUnixEpoch, false);
    chipEpochTime = unixEpochTime - kChipEpochUsSinceUnixEpoch;

    return true;
}

static CHIP_ERROR UpdateUTCTime(uint64_t UTCTimeInChipEpochUs)
{
    System::Clock::Seconds32 lastKnownGoodChipEpoch;
    uint64_t UTCTimeInUnixEpochUs;

    ReturnErrorOnFailure(Server::GetInstance().GetFabricTable().GetLastKnownGoodChipEpochTime(lastKnownGoodChipEpoch));
    VerifyOrReturnError(ChipEpochToUnixEpochMicro(UTCTimeInChipEpochUs, UTCTimeInUnixEpochUs), CHIP_ERROR_INVALID_TIME);

    ReturnErrorOnFailure(Server::GetInstance().GetFabricTable().SetLastKnownGoodChipEpochTime(
        System::Clock::Seconds32(static_cast<uint32_t>(UTCTimeInChipEpochUs / chip::kMicrosecondsPerSecond))));
    ReturnErrorOnFailure(System::SystemClock().SetClock_RealTime(System::Clock::Microseconds64(UTCTimeInUnixEpochUs)));

    return CHIP_NO_ERROR;
}

static bool sendDSTTableEmptyEvent(EndpointId ep)
{
    Events::DSTTableEmpty::Type event;
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to send DSTTableEmpty event [ep=%d]", ep);
        return false;
    }
    ChipLogProgress(Zcl, "Emit DSTTableEmpty event [ep=%d]", ep);

    // TODO: re-schedule event for after min 1hr https://github.com/project-chip/connectedhomeip/issues/27200
    // delegate->scheduleDSTTableEmptyEvent()
    return true;
}

static bool sendDSTStatusEvent(EndpointId ep, bool dstOffsetActive)
{
    Events::DSTStatus::Type event;
    event.DSTOffsetActive = dstOffsetActive;
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to send sendDSTStatus event [ep=%d]", ep);
        return false;
    }

    ChipLogProgress(Zcl, "Emit sendDSTStatus event [ep=%d]", ep);
    return true;
}

static bool sendTimeZoneStatusEvent(EndpointId ep)
{
    Events::TimeZoneStatus::Type event;
    const auto & tz = TimeSynchronizationServer::Instance().GetTimeZone()[0];
    event.offset    = tz.offset;
    if (tz.name.HasValue())
    {
        event.name.SetValue(tz.name.Value());
    }
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to send sendTimeZoneStatus event [ep=%d]", ep);
        return false;
    }

    ChipLogProgress(Zcl, "Emit sendTimeZoneStatus event [ep=%d]", ep);
    return true;
}

static bool sendTimeFailureEvent(EndpointId ep)
{
    Events::TimeFailure::Type event;
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to send sendTimeFailure event [ep=%d]", ep);
        return false;
    }

    // TODO: re-schedule event for after min 1hr if no time is still available
    // https://github.com/project-chip/connectedhomeip/issues/27200
    ChipLogProgress(Zcl, "Emit sendTimeFailure event [ep=%d]", ep);
    return true;
}

static bool sendMissingTrustedTimeSourceEvent(EndpointId ep)
{
    Events::MissingTrustedTimeSource::Type event;
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to send sendMissingTrustedTimeSource event [ep=%d]", ep);
        return false;
    }

    // TODO: re-schedule event for after min 1hr if TTS is null or cannot be reached
    // https://github.com/project-chip/connectedhomeip/issues/27200
    ChipLogProgress(Zcl, "Emit sendMissingTrustedTimeSource event [ep=%d]", ep);
    return true;
}

TimeSynchronizationServer TimeSynchronizationServer::sTimeSyncInstance;

TimeSynchronizationServer & TimeSynchronizationServer::Instance()
{
    return sTimeSyncInstance;
}

void TimeSynchronizationServer::Init()
{
    mTimeSyncDataProvider.Init(Server::GetInstance().GetPersistentStorage());

    Structs::TrustedTimeSourceStruct::Type tts;
    (CHIP_NO_ERROR == mTimeSyncDataProvider.LoadTrustedTimeSource(tts)) ? (void) mTrustedTimeSource.SetNonNull(tts)
                                                                        : mTrustedTimeSource.SetNull();
    for (size_t i = 0; i < mTimeZoneList.size(); i++)
    {
        memset(mNames[i].name, 0, sizeof(mNames[i].name));
        mTimeZoneList[i] = { .offset  = 0,
                             .validAt = 0,
                             .name    = MakeOptional(CharSpan(mNames[i].name, sizeof(mNames[i].name))) };
    }
    mTimeSyncDataProvider.LoadTimeZone(mTimeZoneList, mTimeZoneListSize);
    mTimeZoneListSize = (mTimeZoneListSize == 0) ? 1 : mTimeZoneListSize; // initialize default value
    mTimeSyncDataProvider.LoadDSTOffset(mDstOffsetList, mDstOffsetListSize);
    // TODO: if trusted time source is available schedule a time read https://github.com/project-chip/connectedhomeip/issues/27201
    if (!mTrustedTimeSource.IsNull())
    {
    }
    System::Clock::Microseconds64 utcTime;
    if (System::SystemClock().GetClock_RealTime(utcTime) == CHIP_NO_ERROR)
    {
        mGranularity = GranularityEnum::kMinutesGranularity;
    }
    else
    {
        mGranularity = GranularityEnum::kNoTimeGranularity;
    }
}

CHIP_ERROR TimeSynchronizationServer::SetTrustedTimeSource(const DataModel::Nullable<Structs::TrustedTimeSourceStruct::Type> & tts)
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

CHIP_ERROR TimeSynchronizationServer::SetDefaultNTP(const DataModel::Nullable<CharSpan> & dntp)
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

CHIP_ERROR TimeSynchronizationServer::SetTimeZone(const DataModel::DecodableList<Structs::TimeZoneStruct::Type> & tzL)
{
    size_t items;
    ReturnErrorOnFailure(tzL.ComputeSize(&items));

    if (items > CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    char name[64];
    Structs::TimeZoneStruct::Type lastTz;
    TimeState lastTzState = GetUpdatedTimeZoneState();

    if (lastTzState != TimeState::kInvalid)
    {
        lastTz.offset = mTimeZoneList[0].offset;
        if (mTimeZoneList[0].name.HasValue())
        {
            lastTz.name.SetValue(CharSpan(name, sizeof(name)));
            memcpy(name, mTimeZoneList[0].name.Value().data(), mTimeZoneList[0].name.Value().size());
        }
    }

    auto newTzL = tzL.begin();
    uint8_t i   = 0;

    while (newTzL.Next())
    {
        auto & tz = mTimeZoneList[i];
        auto & newTz = newTzL.GetValue();
        if (newTz.offset < -43200 || newTz.offset > 50400)
        {
            mTimeSyncDataProvider.LoadTimeZone(mTimeZoneList, mTimeZoneListSize);
            return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB;
        }
        // first element shall have validAt entry of 0
        if (i == 0 && newTz.validAt != 0)
        {
            return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB;
        }
        // if second element, it shall have validAt entry of non-0
        if (i == 1 && newTz.validAt == 0)
        {
            mTimeSyncDataProvider.LoadTimeZone(mTimeZoneList, mTimeZoneListSize);
            return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB;
        }
        tz.offset  = newTz.offset;
        tz.validAt = newTz.validAt;
        if (newTz.name.HasValue())
        {
            if (newTz.name.Value().size() > sizeof(mNames[i].name))
            {
                mTimeSyncDataProvider.LoadTimeZone(mTimeZoneList, mTimeZoneListSize);
                return CHIP_ERROR_BUFFER_TOO_SMALL;
            }
            const char * buf = newTz.name.Value().data();
            size_t len       = newTz.name.Value().size();
            memset(mNames[i].name, 0, sizeof(mNames[i].name));
            memcpy(mNames[i].name, buf, len);
            tz.name.SetValue(CharSpan(mNames[i].name, strnlen(mNames[i].name, sizeof(mNames[i].name))));
        }
        else
        {
            tz.name.ClearValue();
        }
        i++;
    }
    if (CHIP_NO_ERROR != newTzL.GetStatus())
    {
        mTimeSyncDataProvider.LoadTimeZone(mTimeZoneList, mTimeZoneListSize);
        return newTzL.GetStatus();
    }
    if (i == 0)
    {
        return ClearTimeZone();
    }

    mTimeZoneListSize = i;

    if (lastTzState != TimeState::kInvalid && TimeState::kInvalid != GetUpdatedTimeZoneState())
    {
        bool emit = false;
        if (mTimeZoneList[0].offset != lastTz.offset)
        {
            emit = true;
        }
        if ((mTimeZoneList[0].name.HasValue() && lastTz.name.HasValue()) &&
            !(mTimeZoneList[0].name.Value().data_equal(lastTz.name.Value())))
        {
            emit = true;
        }
        if (emit)
            mEventFlag = TimeSyncEventFlag::kTimeZoneStatus;
    }
    return mTimeSyncDataProvider.StoreTimeZone(GetTimeZone());
}

CHIP_ERROR TimeSynchronizationServer::SetDSTOffset(const DataModel::DecodableList<Structs::DSTOffsetStruct::Type> & dstL)
{
    size_t items;
    ReturnErrorOnFailure(dstL.ComputeSize(&items));

    if (items > CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    auto newDstL = dstL.begin();
    uint8_t i    = 0;

    while (newDstL.Next())
    {
        mDstOffsetList[i] = newDstL.GetValue();
        i++;
    }

    if (CHIP_NO_ERROR != newDstL.GetStatus())
    {
        mTimeSyncDataProvider.LoadDSTOffset(mDstOffsetList, mDstOffsetListSize);
        return newDstL.GetStatus();
    }
    if (i == 0)
    {
        return ClearDSTOffset();
    }

    mDstOffsetListSize = i;

    // only 1 validuntil null value and shall be last in the list
    uint64_t lastValidUntil = 0;
    for (i = 0; i < mDstOffsetListSize; i++)
    {
        const auto & dstItem = GetDSTOffset()[i];
        // list should be sorted by validStarting
        for (uint8_t j = static_cast<uint8_t>(i + 1); j < mDstOffsetListSize; j++)
        {
            const auto & nextDstItem = GetDSTOffset()[j];
            if (dstItem.validStarting > nextDstItem.validStarting)
            {
                return CHIP_ERROR_INVALID_TIME;
            }
        }
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

    return mTimeSyncDataProvider.StoreDSTOffset(GetDSTOffset());
}

CHIP_ERROR TimeSynchronizationServer::ClearTimeZone()
{
    mTimeZoneListSize = 1; // default time zone needed
    for (uint8_t i = 0; i < CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE; i++)
    {
        memset(mNames[i].name, 0, sizeof(mNames[i].name));
        mTz[i] = { .offset = 0, .validAt = 0, .name = MakeOptional(CharSpan(mNames[i].name, sizeof(mNames[i].name))) };
    }
    return mTimeSyncDataProvider.StoreTimeZone(GetTimeZone());
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

DataModel::Nullable<Structs::TrustedTimeSourceStruct::Type> & TimeSynchronizationServer::GetTrustedTimeSource()
{
    return mTrustedTimeSource;
}

CHIP_ERROR TimeSynchronizationServer::GetDefaultNtp(MutableCharSpan & dntp)
{
    return mTimeSyncDataProvider.LoadDefaultNtp(dntp);
}

DataModel::List<Structs::TimeZoneStruct::Type> TimeSynchronizationServer::GetTimeZone()
{
    return mTimeZoneList.SubSpan(0, mTimeZoneListSize);
}

DataModel::List<Structs::DSTOffsetStruct::Type> TimeSynchronizationServer::GetDSTOffset()
{
    return mDstOffsetList.SubSpan(0, mDstOffsetListSize);
}

void TimeSynchronizationServer::ScheduleDelayedAction(System::Clock::Seconds32 delay, System::TimerCompleteCallback action,
                                                      void * aAppState)
{
    VerifyOrDie(SystemLayer().StartTimer(std::chrono::duration_cast<System::Clock::Timeout>(delay), action, aAppState) ==
                CHIP_NO_ERROR);
}

CHIP_ERROR TimeSynchronizationServer::SetUTCTime(EndpointId ep, uint64_t utcTime, GranularityEnum granularity,
                                                 TimeSourceEnum source)
{
    ReturnErrorOnFailure(UpdateUTCTime(utcTime));
    mGranularity = granularity;
    TimeSource::Set(ep, source);
    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeSynchronizationServer::GetLocalTime(EndpointId ep, uint64_t & localTime)
{
    uint64_t timeZoneOffset = 0, dstOffset = 0;
    System::Clock::Microseconds64 utcTime;
    uint64_t chipEpochTime;
    VerifyOrReturnError(TimeState::kInvalid != GetUpdatedDSTOffsetState(), CHIP_ERROR_INVALID_TIME);
    ReturnErrorOnFailure(System::SystemClock().GetClock_RealTime(utcTime));
    VerifyOrReturnError(UnixEpochToChipEpochMicro(utcTime.count(), chipEpochTime), CHIP_ERROR_INVALID_TIME);
    if (TimeState::kChanged == GetUpdatedTimeZoneState())
    {
        sendTimeZoneStatusEvent(ep);
    }
    const auto & tz  = GetTimeZone()[0];
    timeZoneOffset   = chip::kMicrosecondsPerSecond * static_cast<uint64_t>(tz.offset);
    const auto & dst = GetDSTOffset()[0];
    dstOffset        = chip::kMicrosecondsPerSecond * static_cast<uint64_t>(dst.offset);

    localTime = chipEpochTime + timeZoneOffset + dstOffset;
    return CHIP_NO_ERROR;
}

TimeState TimeSynchronizationServer::GetUpdatedTimeZoneState()
{
    System::Clock::Microseconds64 utcTime;
    auto tzList           = GetTimeZone();
    uint8_t activeTzIndex = 0;
    uint64_t chipEpochTime;

    VerifyOrReturnValue(System::SystemClock().GetClock_RealTime(utcTime) == CHIP_NO_ERROR, TimeState::kInvalid);
    VerifyOrReturnValue(tzList.size() != 0, TimeState::kInvalid);
    VerifyOrReturnValue(UnixEpochToChipEpochMicro(utcTime.count(), chipEpochTime), TimeState::kInvalid);

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
        VerifyOrReturnValue(mTimeSyncDataProvider.StoreTimeZone(newTimeZoneList) == CHIP_NO_ERROR, TimeState::kInvalid);
        VerifyOrReturnValue(mTimeSyncDataProvider.LoadTimeZone(mTimeZoneList, mTimeZoneListSize) == CHIP_NO_ERROR,
                            TimeState::kInvalid);
        return TimeState::kChanged;
    }
    return TimeState::kActive;
}

TimeState TimeSynchronizationServer::GetUpdatedDSTOffsetState()
{
    System::Clock::Microseconds64 utcTime;
    uint8_t activeDstIndex = 0;
    auto dstList           = GetDSTOffset();
    uint64_t chipEpochTime;

    VerifyOrReturnValue(System::SystemClock().GetClock_RealTime(utcTime) == CHIP_NO_ERROR, TimeState::kInvalid);
    VerifyOrReturnValue(dstList.size() != 0, TimeState::kInvalid);
    VerifyOrReturnValue(UnixEpochToChipEpochMicro(utcTime.count(), chipEpochTime), TimeState::kInvalid);

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
        return TimeState::kStopped;
    }
    if (activeDstIndex != 0)
    {
        mDstOffsetListSize    = static_cast<uint8_t>(dstList.size() - activeDstIndex);
        auto newDstOffsetList = dstList.SubSpan(activeDstIndex);
        VerifyOrReturnValue(mTimeSyncDataProvider.StoreDSTOffset(newDstOffsetList) == CHIP_NO_ERROR, TimeState::kInvalid);
        VerifyOrReturnValue(mTimeSyncDataProvider.LoadDSTOffset(mDstOffsetList, mDstOffsetListSize) == CHIP_NO_ERROR,
                            TimeState::kInvalid);
        return TimeState::kChanged;
    }
    return TimeState::kActive;
}

TimeSyncEventFlag TimeSynchronizationServer::GetEventFlag()
{
    return mEventFlag;
}

void TimeSynchronizationServer::ClearEventFlag(TimeSyncEventFlag flag)
{
    uint8_t eventFlag = to_underlying(mEventFlag) ^ to_underlying(flag);
    mEventFlag        = static_cast<TimeSyncEventFlag>(eventFlag);
}

namespace {

class TimeSynchronizationAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the TimeSync cluster on all endpoints
    TimeSynchronizationAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), Id) {}

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
    char buffer[DefaultNTP::TypeInfo::MaxLength()];
    MutableCharSpan dntp(buffer);
    if (TimeSynchronizationServer::Instance().GetDefaultNtp(dntp) == CHIP_NO_ERROR && dntp.size() != 0)
    {
        err = aEncoder.Encode(CharSpan(buffer, dntp.size()));
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

    if (aPath.mClusterId != Id)
    {
        return CHIP_ERROR_INVALID_PATH_LIST;
    }

    switch (aPath.mAttributeId)
    {
    case UTCTime::Id: {
        System::Clock::Microseconds64 utcTimeUnix;
        uint64_t chipEpochTime;

        VerifyOrReturnError(System::SystemClock().GetClock_RealTime(utcTimeUnix) == CHIP_NO_ERROR, aEncoder.EncodeNull());
        VerifyOrReturnError(UnixEpochToChipEpochMicro(utcTimeUnix.count(), chipEpochTime), aEncoder.EncodeNull());
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

bool emberAfTimeSynchronizationClusterSetUTCTimeCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::TimeSynchronization::Commands::SetUTCTime::DecodableType & commandData)
{
    const auto & utcTime     = commandData.UTCTime;
    const auto & granularity = commandData.granularity;

    auto currentGranularity = TimeSynchronizationServer::Instance().GetGranularity();

    if (granularity != GranularityEnum::kNoTimeGranularity &&
        (currentGranularity == GranularityEnum::kNoTimeGranularity || granularity >= currentGranularity) &&
        CHIP_NO_ERROR ==
            TimeSynchronizationServer::Instance().SetUTCTime(commandPath.mEndpointId, utcTime, granularity, TimeSourceEnum::kAdmin))
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
    DataModel::Nullable<Structs::TrustedTimeSourceStruct::Type> tts;

    if (!timeSource.IsNull())
    {

        Structs::TrustedTimeSourceStruct::Type ts = { commandObj->GetAccessingFabricIndex(), timeSource.Value().nodeID,
                                                      timeSource.Value().endpoint };
        tts.SetNonNull(ts);
        // : schedule a utctime read from this time source
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

    if (to_underlying(TimeSynchronizationServer::Instance().GetEventFlag()) & to_underlying(TimeSyncEventFlag::kTimeZoneStatus))
    {
        TimeSynchronizationServer::Instance().ClearEventFlag(TimeSyncEventFlag::kTimeZoneStatus);
        sendTimeZoneStatusEvent(commandPath.mEndpointId);
    }
    sendTimeFailureEvent(commandPath.mEndpointId); // TODO remove
    GetDelegate()->HandleTimeZoneChanged(TimeSynchronizationServer::Instance().GetTimeZone());

    TimeZoneDatabaseEnum tzDb;
    TimeZoneDatabase::Get(commandPath.mEndpointId, &tzDb);
    Commands::SetTimeZoneResponse::Type response;
    if (GetDelegate()->HasFeature(commandPath.mEndpointId, Feature::kTimeZone) && tzDb != TimeZoneDatabaseEnum::kNone)
    {
        TimeSynchronizationServer::Instance().GetUpdatedTimeZoneState();
        const auto & tz = TimeSynchronizationServer::Instance().GetTimeZone()[0];
        if (tz.name.HasValue() && GetDelegate()->HandleUpdateDSTOffset(tz.name.Value()))
        {
            response.DSTOffsetRequired = false;
            sendDSTStatusEvent(commandPath.mEndpointId, true);
        }
        else
        {
            TimeState dstState = TimeSynchronizationServer::Instance().GetUpdatedDSTOffsetState();
            TimeSynchronizationServer::Instance().ClearDSTOffset();
            sendDSTTableEmptyEvent(commandPath.mEndpointId);
            if (dstState == TimeState::kActive || dstState == TimeState::kChanged)
            {
                sendDSTStatusEvent(commandPath.mEndpointId, false);
            }
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

    TimeState dstState = TimeSynchronizationServer::Instance().GetUpdatedDSTOffsetState();

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
    if (dstState != TimeSynchronizationServer::Instance().GetUpdatedDSTOffsetState())
    {
        sendDSTStatusEvent(commandPath.mEndpointId,
                           TimeState::kActive == TimeSynchronizationServer::Instance().GetUpdatedDSTOffsetState());
    }
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
        size_t len = dNtpChar.Value().size();
        if (len > DefaultNTP::TypeInfo::MaxLength())
        {
            commandObj->AddStatus(commandPath, Status::ConstraintError);
            return true;
        }
        if (!GetDelegate()->IsNTPAddressValid(dNtpChar.Value()))
        {
            commandObj->AddStatus(commandPath, Status::InvalidCommand);
            return true;
        }
        if (GetDelegate()->IsNTPAddressDomain(dNtpChar.Value()))
        {
            bool dnsResolve;
            SupportsDNSResolve::Get(commandPath.mEndpointId, &dnsResolve);
            if (!dnsResolve)
            {
                commandObj->AddStatus(commandPath, Status::InvalidCommand);
                return true;
            }
        }
    }

    status =
        (CHIP_NO_ERROR == TimeSynchronizationServer::Instance().SetDefaultNTP(dNtpChar)) ? Status::Success : Status::InvalidCommand;

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
    }
}
