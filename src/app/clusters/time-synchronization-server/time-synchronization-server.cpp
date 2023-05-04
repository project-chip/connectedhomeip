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
    mTimeSyncDataProvider.LoadDSTOffset(mDstOffsetList, mDstOffsetListSize);
    // TODO if trusted time source is available schedule a time read
    if (!mTrustedTimeSource.IsNull())
    {
    }
}

CHIP_ERROR TimeSynchronizationServer::SetTrustedTimeSource(
    DataModel::Nullable<TimeSynchronization::Structs::TrustedTimeSourceStruct::Type> tts)
{
    mTrustedTimeSource = tts;
    if (!mTrustedTimeSource.IsNull())
    {
        return mTimeSyncDataProvider.StoreTrustedTimeSource(mTrustedTimeSource.Value());
    }
    else
    {
        return mTimeSyncDataProvider.ClearTrustedTimeSource();
    }
}

CHIP_ERROR TimeSynchronizationServer::SetDefaultNtp(DataModel::Nullable<chip::CharSpan> & dntp)
{
    if (!dntp.IsNull())
    {
        return mTimeSyncDataProvider.StoreDefaultNtp(dntp.Value());
    }
    else
    {
        return mTimeSyncDataProvider.ClearDefaultNtp();
    }
}

CHIP_ERROR TimeSynchronizationServer::SetTimeZone(DataModel::DecodableList<TimeSynchronization::Structs::TimeZoneStruct::Type> tz)
{
    auto tzL    = mTimeZoneList.begin();
    auto newTzL = tz.begin();
    uint8_t i   = 0;

    while (newTzL.Next() && i < CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE)
    {
        tzL[i].offset  = newTzL.GetValue().offset;
        tzL[i].validAt = newTzL.GetValue().validAt;
        if (newTzL.GetValue().name.HasValue())
        {
            const char * buf = newTzL.GetValue().name.Value().data();
            size_t len       = newTzL.GetValue().name.Value().size();
            memset(mNames[i].name, 0, sizeof(mNames[i].name));
            memcpy(mNames[i].name, buf, len);
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
    auto dstL    = mDstOffsetList.begin();
    auto newDstL = dst.begin();
    uint8_t i    = 0;

    while (newDstL.Next() && i < CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE)
    {
        dstL[i] = newDstL.GetValue();
        i++;
    }
    ReturnErrorOnFailure(newDstL.GetStatus());
    mDstOffsetListSize = i;

    return mTimeSyncDataProvider.StoreDSTOffset(TimeSynchronizationServer::Instance().GetDSTOffset());
}

CHIP_ERROR TimeSynchronizationServer::ClearDSTOffset()
{
    mDstOffsetListSize = 0;
    memset((void *) mDst, 0, sizeof(mDst) * CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE);

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

namespace {

static bool computeLocalTime(chip::EndpointId ep, uint64_t & localTime)
{
    int32_t timeZoneOffset = 0, dstOffset = 0;
    System::Clock::Microseconds64 utcTime;
    VerifyOrReturnValue(System::SystemClock().GetClock_RealTime(utcTime) == CHIP_NO_ERROR, false);
    auto tz  = TimeSynchronizationServer::Instance().GetTimeZone().begin();
    auto dst = TimeSynchronizationServer::Instance().GetDSTOffset().begin();
    if (tz->validAt <= utcTime.count())
    {
        timeZoneOffset = tz->offset;
    }
    if (dst->validStarting <= utcTime.count())
    {
        dstOffset = dst->offset;
    }
    localTime = (utcTime.count() + static_cast<uint64_t>(timeZoneOffset) + static_cast<uint64_t>(dstOffset));
    return true;
}

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
};

TimeSynchronizationAttrAccess gAttrAccess;

CHIP_ERROR TimeSynchronizationAttrAccess::ReadTrustedTimeSource(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    auto tts = TimeSynchronizationServer::Instance().GetTrustedTimeSource();
    return aEncoder.Encode(tts);
}

CHIP_ERROR TimeSynchronizationAttrAccess::ReadDefaultNtp(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    uint8_t buffer[DefaultNTP::TypeInfo::MaxLength()];
    MutableByteSpan dntp(buffer);
    if (TimeSynchronizationServer::Instance().GetDefaultNtp(dntp) == CHIP_NO_ERROR && dntp.size() != 0)
    {
        const char * charBuf = reinterpret_cast<const char *>(buffer);
        return aEncoder.Encode(chip::CharSpan(charBuf, dntp.size()));
    }
    else
    {
        return aEncoder.EncodeNull();
    }
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
        System::Clock::Microseconds64 utcTime;
        VerifyOrReturnError(System::SystemClock().GetClock_RealTime(utcTime) == CHIP_NO_ERROR, aEncoder.EncodeNull());
        return aEncoder.Encode(utcTime.count());
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
    case TimeZoneListMaxSize::Id: { // can't find a way to initialize default value for F quality
        uint8_t max = CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE;
        return aEncoder.Encode(max);
    }
    case DSTOffsetListMaxSize::Id: {
        uint8_t max = CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE;
        return aEncoder.Encode(max);
    }
    case LocalTime::Id: {
        uint64_t localTime = 0;
        VerifyOrReturnError(computeLocalTime(aPath.mEndpointId, localTime), aEncoder.EncodeNull());
        return aEncoder.Encode(localTime);
    }
    default: {
        break;
    }
    }

    return err;
}
} // anonymous namespace

static bool sendDSTTableEmpty(chip::EndpointId endpointId)
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

static bool sendDSTStatus(chip::EndpointId endpointId, bool dstOffsetActive)
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

static bool sendTimeZoneStatus(chip::EndpointId endpointId, uint8_t listIndex)
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

static bool sendTimeFailure(chip::EndpointId endpointId)
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

static bool sendMissingTrustedTimeSource(chip::EndpointId endpointId)
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

static bool utcTimeChanged(uint64_t utcTime)
{
    System::Clock::Seconds32 lastKnownGoodChipEpoch;
    System::Clock::Microseconds64 realTime;
    uint32_t utcTimetoChipEpoch;

    Server::GetInstance().GetFabricTable().GetLastKnownGoodChipEpochTime(lastKnownGoodChipEpoch);
    System::SystemClock().GetClock_RealTime(realTime);
    chip::UnixEpochToChipEpochTime((uint32_t)(utcTime / chip::kMicrosecondsPerSecond), utcTimetoChipEpoch);

    VerifyOrReturnValue(Server::GetInstance().GetFabricTable().SetLastKnownGoodChipEpochTime(
                            System::Clock::Seconds32(utcTimetoChipEpoch)) == CHIP_NO_ERROR,
                        false);
    VerifyOrReturnValue(System::SystemClock().SetClock_RealTime(System::Clock::Microseconds64(utcTime)) == CHIP_NO_ERROR, false);

    Server::GetInstance().GetFabricTable().GetLastKnownGoodChipEpochTime(lastKnownGoodChipEpoch);
    System::SystemClock().GetClock_RealTime(realTime);
    return true;
}

bool emberAfTimeSynchronizationClusterSetUTCTimeCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::TimeSynchronization::Commands::SetUTCTime::DecodableType & commandData)
{
    const auto & utcTime     = commandData.UTCTime;
    const auto & granularity = commandData.granularity;

    TimeSynchronization::GranularityEnum currentGranularity;
    Granularity::Get(commandPath.mEndpointId, &currentGranularity);

    if ((currentGranularity == TimeSynchronization::GranularityEnum::kNoTimeGranularity || granularity >= currentGranularity) &&
        utcTimeChanged(utcTime))
    {
        Granularity::Set(commandPath.mEndpointId, granularity);
        TimeSource::Set(commandPath.mEndpointId, TimeSynchronization::TimeSourceEnum::kAdmin);
        commandObj->AddStatus(commandPath, Status::Success);
        return true;
    }
    else
    {
        commandObj->AddClusterSpecificFailure(commandPath, to_underlying(TimeSynchronization::StatusCode::kTimeNotAccepted));
        return true;
    }
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
        sendMissingTrustedTimeSource(commandPath.mEndpointId);
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
    size_t items;
    uint8_t maxSize = 0;
    uint64_t localTime;
    VerifyOrReturnValue(timeZone.ComputeSize(&items) == CHIP_NO_ERROR, false);
    TimeZoneListMaxSize::Get(commandPath.mEndpointId, &maxSize);

    if (items > maxSize || items > CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE)
    {
        commandObj->AddStatus(commandPath, Status::ResourceExhausted);
        return true;
    }
    // first element shall have validAt entry of 0
    // if second element, it shall have validAt entry of non-0

    if (CHIP_NO_ERROR != TimeSynchronizationServer::Instance().SetTimeZone(timeZone))
    {
        commandObj->AddStatus(commandPath, Status::ConstraintError);
        return true;
    }
    sendTimeZoneStatus(commandPath.mEndpointId, 0);
    sendTimeFailure(commandPath.mEndpointId); // TODO remove
    GetDelegate()->HandleTimeZoneChanged(TimeSynchronizationServer::Instance().GetTimeZone());
    GetDelegate()->HandleDstoffsetlookup();

    TimeSynchronization::TimeZoneDatabaseEnum tzDb;
    TimeZoneDatabase::Get(commandPath.mEndpointId, &tzDb);
    Commands::SetTimeZoneResponse::Type response;
    if (GetDelegate()->HasFeature(0, TimeSynchronization::TimeSynchronizationFeature::kTimeZone) &&
        tzDb != TimeSynchronization::TimeZoneDatabaseEnum::kNone)
    {
        auto tz = TimeSynchronizationServer::Instance().GetTimeZone().begin();
        if (tz->name.HasValue() && GetDelegate()->HandleDstoffsetavailable(tz->name.Value()) == true)
        {
            GetDelegate()->HandleGetdstoffset();
            response.DSTOffsetRequired = false;
            sendDSTStatus(commandPath.mEndpointId, true);
        }
        else
        {
            TimeSynchronizationServer::Instance().ClearDSTOffset();
            sendDSTTableEmpty(commandPath.mEndpointId);
            sendDSTStatus(commandPath.mEndpointId, false);
        }
    }
    computeLocalTime(commandPath.mEndpointId, localTime);
    response.DSTOffsetRequired = true;
    commandObj->AddResponse(commandPath, response);
    return true;
}

bool emberAfTimeSynchronizationClusterSetDSTOffsetCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::TimeSynchronization::Commands::SetDSTOffset::DecodableType & commandData)
{
    const auto & dstOffset = commandData.DSTOffset;
    size_t items;
    uint8_t maxSize = 0;
    VerifyOrReturnValue(dstOffset.ComputeSize(&items) == CHIP_NO_ERROR, false);
    DSTOffsetListMaxSize::Get(commandPath.mEndpointId, &maxSize);

    if (items > maxSize && items > CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE)
    {
        commandObj->AddStatus(commandPath, Status::ResourceExhausted);
        return true;
    }

    // sorted by ValidStarting time
    // ValidStartingTime shall not be smaller than ValidUntil time of previous entry
    // only 1 validuntil null value and shall be last in the list
    // remove entries which are no longer active
    // if offset == 0 && ValidUntil == null then no DST is used
    if (CHIP_NO_ERROR != TimeSynchronizationServer::Instance().SetDSTOffset(dstOffset))
    {
        commandObj->AddStatus(commandPath, Status::ConstraintError);
        return true;
    }
    // if DST state changes, generate DSTStatus event
    sendDSTStatus(commandPath.mEndpointId, true);
    // if list is empty, generate DSTTableEmpty event
    sendDSTTableEmpty(commandPath.mEndpointId);

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
        TimeSynchronization::GranularityEnum granularity = TimeSynchronization::GranularityEnum::kNoTimeGranularity;
        TimeSynchronization::Attributes::Granularity::Set(0, granularity);
        // System::SystemClock().SetClock_RealTime(System::Clock::Microseconds64(1679668000000000));
#endif
    }
}
