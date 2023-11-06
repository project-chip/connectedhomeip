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

#if TIME_SYNC_ENABLE_TSC_FEATURE
#include <app/InteractionModelEngine.h>
#endif

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
#include <platform/RuntimeOptionsProvider.h>

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

#if TIME_SYNC_ENABLE_TSC_FEATURE
void OnDeviceConnectedWrapper(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    TimeSynchronizationServer * server = reinterpret_cast<TimeSynchronizationServer *>(context);
    server->OnDeviceConnectedFn(exchangeMgr, sessionHandle);
}

void OnDeviceConnectionFailureWrapper(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    TimeSynchronizationServer * server = reinterpret_cast<TimeSynchronizationServer *>(context);
    server->OnDeviceConnectionFailureFn();
}

#endif

void OnPlatformEventWrapper(const DeviceLayer::ChipDeviceEvent * event, intptr_t ptr)
{
    TimeSynchronizationServer * server = reinterpret_cast<TimeSynchronizationServer *>(ptr);
    server->OnPlatformEventFn(*event);
}

void OnTimeSyncCompletionWrapper(void * context, TimeSourceEnum timeSource, GranularityEnum granularity)
{
    TimeSynchronizationServer * server = reinterpret_cast<TimeSynchronizationServer *>(context);
    server->OnTimeSyncCompletionFn(timeSource, granularity);
}

void OnFallbackNTPCompletionWrapper(void * context, bool timeSyncSuccessful)
{
    TimeSynchronizationServer * server = reinterpret_cast<TimeSynchronizationServer *>(context);
    server->OnFallbackNTPCompletionFn(timeSyncSuccessful);
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
    // in case chipEpochTime is too big and result overflows return false
    if (chipEpochTime + kChipEpochUsSinceUnixEpoch < kChipEpochUsSinceUnixEpoch)
    {
        return false;
    }
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
    uint64_t UTCTimeInUnixEpochUs;

    VerifyOrReturnError(ChipEpochToUnixEpochMicro(UTCTimeInChipEpochUs, UTCTimeInUnixEpochUs), CHIP_ERROR_INVALID_TIME);
    uint64_t secs = UTCTimeInChipEpochUs / chip::kMicrosecondsPerSecond;
    // https://github.com/project-chip/connectedhomeip/issues/27501
    VerifyOrReturnError(secs <= UINT32_MAX, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
    ReturnErrorOnFailure(Server::GetInstance().GetFabricTable().SetLastKnownGoodChipEpochTime(
        System::Clock::Seconds32(static_cast<uint32_t>(secs))));
    ReturnErrorOnFailure(System::SystemClock().SetClock_RealTime(System::Clock::Microseconds64(UTCTimeInUnixEpochUs)));

    return CHIP_NO_ERROR;
}

static bool emitDSTTableEmptyEvent(EndpointId ep)
{
    Events::DSTTableEmpty::Type event;
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to emit DSTTableEmpty event [ep=%d]", ep);
        return false;
    }
    ChipLogProgress(Zcl, "Emit DSTTableEmpty event [ep=%d]", ep);

    // TODO: re-schedule event for after min 1hr https://github.com/project-chip/connectedhomeip/issues/27200
    // delegate->scheduleDSTTableEmptyEvent()
    return true;
}

static bool emitDSTStatusEvent(EndpointId ep, bool dstOffsetActive)
{
    Events::DSTStatus::Type event;
    event.DSTOffsetActive = dstOffsetActive;
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to emit DSTStatus event [ep=%d]", ep);
        return false;
    }

    ChipLogProgress(Zcl, "Emit DSTStatus event [ep=%d]", ep);
    return true;
}

static bool emitTimeZoneStatusEvent(EndpointId ep)
{
    const auto & tzList = TimeSynchronizationServer::Instance().GetTimeZone();
    VerifyOrReturnValue(tzList.size() != 0, false);
    const auto & tz = tzList[0].timeZone;
    Events::TimeZoneStatus::Type event;

    event.offset = tz.offset;
    if (tz.name.HasValue())
    {
        event.name.SetValue(tz.name.Value());
    }
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to emit TimeZoneStatus event [ep=%d]", ep);
        return false;
    }

    ChipLogProgress(Zcl, "Emit TimeZoneStatus event [ep=%d]", ep);
    return true;
}

static bool emitTimeFailureEvent(EndpointId ep)
{
    Events::TimeFailure::Type event;
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to emit TimeFailure event [ep=%d]", ep);
        return false;
    }

    // TODO: re-schedule event for after min 1hr if no time is still available
    // https://github.com/project-chip/connectedhomeip/issues/27200
    ChipLogProgress(Zcl, "Emit TimeFailure event [ep=%d]", ep);
    return true;
}

static bool emitMissingTrustedTimeSourceEvent(EndpointId ep)
{
    Events::MissingTrustedTimeSource::Type event;
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to emit MissingTrustedTimeSource event [ep=%d]", ep);
        return false;
    }

    // TODO: re-schedule event for after min 1hr if TTS is null or cannot be reached
    // https://github.com/project-chip/connectedhomeip/issues/27200
    ChipLogProgress(Zcl, "Emit MissingTrustedTimeSource event [ep=%d]", ep);
    return true;
}

TimeSynchronizationServer TimeSynchronizationServer::sTimeSyncInstance;

TimeSynchronizationServer & TimeSynchronizationServer::Instance()
{
    return sTimeSyncInstance;
}

TimeSynchronizationServer::TimeSynchronizationServer() :
#if TIME_SYNC_ENABLE_TSC_FEATURE
    mOnDeviceConnectedCallback(OnDeviceConnectedWrapper, this),
    mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureWrapper, this),
#endif
    mOnTimeSyncCompletion(OnTimeSyncCompletionWrapper, this), mOnFallbackNTPCompletion(OnFallbackNTPCompletionWrapper, this)
{}

void TimeSynchronizationServer::AttemptToGetFallbackNTPTimeFromDelegate()
{
    // Sent as a char-string to the delegate so they can read it easily
    char defaultNTP[kMaxDefaultNTPSize];
    MutableCharSpan span(defaultNTP);
    if (GetDefaultNtp(span) != CHIP_NO_ERROR)
    {
        emitTimeFailureEvent(kRootEndpointId);
        return;
    }
    if (span.size() > kMaxDefaultNTPSize)
    {
        emitTimeFailureEvent(kRootEndpointId);
        return;
    }
    if (GetDelegate()->UpdateTimeUsingNTPFallback(span, &mOnFallbackNTPCompletion) != CHIP_NO_ERROR)
    {
        emitTimeFailureEvent(kRootEndpointId);
    }
}

#if TIME_SYNC_ENABLE_TSC_FEATURE
void TimeSynchronizationServer::OnDeviceConnectedFn(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    // Connected to our trusted time source, let's read the time.
    AttributePathParams readPaths[2];
    readPaths[0] = AttributePathParams(kRootEndpointId, Id, Attributes::UTCTime::Id);
    readPaths[1] = AttributePathParams(kRootEndpointId, Id, Attributes::Granularity::Id);

    InteractionModelEngine * engine = InteractionModelEngine::GetInstance();
    ReadPrepareParams readParams(sessionHandle);
    readParams.mpAttributePathParamsList    = readPaths;
    readParams.mAttributePathParamsListSize = 2;

    auto readInfo = Platform::MakeUnique<TimeReadInfo>(engine, &exchangeMgr, *this, ReadClient::InteractionType::Read);
    if (readInfo == nullptr)
    {
        // This is unlikely to work if we don't have memory, but let's try
        OnDeviceConnectionFailureFn();
        return;
    }
    CHIP_ERROR err = readInfo->readClient.SendRequest(readParams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to read UTC time from trusted source");
        OnDeviceConnectionFailureFn();
        return;
    }
    mTimeReadInfo = std::move(readInfo);
}

void TimeSynchronizationServer::OnDeviceConnectionFailureFn()
{
    // No way to read from the TrustedTimeSource, fall back to default NTP
    AttemptToGetFallbackNTPTimeFromDelegate();
}

void TimeSynchronizationServer::OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                                const StatusIB & aStatus)
{
    if (aPath.mClusterId != Id || aStatus.IsFailure())
    {
        return;
    }
    switch (aPath.mAttributeId)
    {
    case Attributes::UTCTime::Id:
        if (DataModel::Decode(*apData, mTimeReadInfo->utcTime) != CHIP_NO_ERROR)
        {
            mTimeReadInfo->utcTime.SetNull();
        }
        break;
    case Attributes::Granularity::Id:
        if (DataModel::Decode(*apData, mTimeReadInfo->granularity) != CHIP_NO_ERROR)
        {
            mTimeReadInfo->granularity = GranularityEnum::kNoTimeGranularity;
        }
        break;
    default:
        break;
    }
}

void TimeSynchronizationServer::OnDone(ReadClient * apReadClient)
{
    if (!mTimeReadInfo->utcTime.IsNull() && mTimeReadInfo->granularity != GranularityEnum::kNoTimeGranularity)
    {
        GranularityEnum ourGranularity;
        // Being conservative with granularity - nothing smaller than seconds because of network delay
        switch (mTimeReadInfo->granularity)
        {
        case GranularityEnum::kMinutesGranularity:
        case GranularityEnum::kSecondsGranularity:
            ourGranularity = GranularityEnum::kMinutesGranularity;
            break;
        default:
            ourGranularity = GranularityEnum::kSecondsGranularity;
            break;
        }

        CHIP_ERROR err =
            SetUTCTime(kRootEndpointId, mTimeReadInfo->utcTime.Value(), ourGranularity, TimeSourceEnum::kNodeTimeCluster);
        if (err == CHIP_NO_ERROR)
        {
            return;
        }
    }
    // We get here if we didn't get a time, or failed to set the time source
    // If we failed to set the UTC time, it doesn't hurt to try the backup - NTP system might have different permissions on the
    // system clock
    AttemptToGetFallbackNTPTimeFromDelegate();
    mTimeReadInfo = nullptr;
}
#endif

void TimeSynchronizationServer::OnTimeSyncCompletionFn(TimeSourceEnum timeSource, GranularityEnum granularity)
{
    if (timeSource != TimeSourceEnum::kNone && granularity == GranularityEnum::kNoTimeGranularity)
    {
        // Unable to get time from the delegate. Try remaining sources.
        CHIP_ERROR err = AttemptToGetTimeFromTrustedNode();
        if (err != CHIP_NO_ERROR)
        {
            AttemptToGetFallbackNTPTimeFromDelegate();
        }
        return;
    }
    mGranularity         = granularity;
    EmberAfStatus status = TimeSource::Set(kRootEndpointId, timeSource);
    if (!(status == EMBER_ZCL_STATUS_SUCCESS || status == EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE))
    {
        ChipLogError(Zcl, "Writing TimeSource failed.");
    }
}

void TimeSynchronizationServer::OnFallbackNTPCompletionFn(bool timeSyncSuccessful)
{
    if (timeSyncSuccessful)
    {
        mGranularity = GranularityEnum::kMillisecondsGranularity;
        // Non-matter SNTP because we know it's external and there's only one source
        EmberAfStatus status = TimeSource::Set(kRootEndpointId, TimeSourceEnum::kNonMatterSNTP);
        if (!(status == EMBER_ZCL_STATUS_SUCCESS || status == EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE))
        {
            ChipLogError(Zcl, "Writing TimeSource failed.");
        }
    }
    else
    {
        emitTimeFailureEvent(kRootEndpointId);
    }
}

CHIP_ERROR TimeSynchronizationServer::AttemptToGetTimeFromTrustedNode()
{
#if TIME_SYNC_ENABLE_TSC_FEATURE
    if (!mTrustedTimeSource.IsNull())
    {
        CASESessionManager * caseSessionManager = Server::GetInstance().GetCASESessionManager();
        ScopedNodeId nodeId(mTrustedTimeSource.Value().nodeID, mTrustedTimeSource.Value().fabricIndex);
        caseSessionManager->FindOrEstablishSession(nodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

void TimeSynchronizationServer::AttemptToGetTime()
{
    // Let's check the delegate and see if can get us a time. Even if the time is already set, we want to ask the delegate so we can
    // set the time source as appropriate.
    CHIP_ERROR err = GetDelegate()->UpdateTimeFromPlatformSource(&mOnTimeSyncCompletion);
    if (err != CHIP_NO_ERROR)
    {
        err = AttemptToGetTimeFromTrustedNode();
    }
    if (err != CHIP_NO_ERROR)
    {
        AttemptToGetFallbackNTPTimeFromDelegate();
    }
}

void TimeSynchronizationServer::Init()
{
    mTimeSyncDataProvider.Init(Server::GetInstance().GetPersistentStorage());

    Structs::TrustedTimeSourceStruct::Type tts;
    if (mTimeSyncDataProvider.LoadTrustedTimeSource(tts) == CHIP_NO_ERROR)
    {
        mTrustedTimeSource.SetNonNull(tts);
    }
    if (LoadTimeZone() != CHIP_NO_ERROR)
    {
        ClearTimeZone();
    }
    if (LoadDSTOffset() != CHIP_NO_ERROR)
    {
        ClearDSTOffset();
    }

    // Set the granularity to none for now - this will force us to go to the delegate so it can
    // properly report the time source
    mGranularity = GranularityEnum::kNoTimeGranularity;

    // This can error, but it's not clear what should happen in this case. For now, just ignore it because we still
    // want time sync even if we can't register the deletgate here.
    CHIP_ERROR err = chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Unable to register Fabric table delegate for time sync");
    }
    PlatformMgr().AddEventHandler(OnPlatformEventWrapper, reinterpret_cast<intptr_t>(this));
}

void TimeSynchronizationServer::Shutdown()
{
    PlatformMgr().RemoveEventHandler(OnPlatformEventWrapper, 0);
}

void TimeSynchronizationServer::OnPlatformEventFn(const DeviceLayer::ChipDeviceEvent & event)
{
    switch (event.Type)
    {
    case DeviceEventType::kServerReady:
        if (mGranularity == GranularityEnum::kNoTimeGranularity)
        {
            AttemptToGetTime();
        }
        break;
    default:
        break;
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
    if (mGranularity == GranularityEnum::kNoTimeGranularity)
    {
        AttemptToGetTime();
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

void TimeSynchronizationServer::InitTimeZone()
{
    mTimeZoneObj.validSize    = 1; // one default time zone item is needed
    mTimeZoneObj.timeZoneList = Span<TimeSyncDataProvider::TimeZoneStore>(mTz);
    for (auto & tzStore : mTimeZoneObj.timeZoneList)
    {
        memset(tzStore.name, 0, sizeof(tzStore.name));
        tzStore.timeZone = { .offset = 0, .validAt = 0, .name = MakeOptional(CharSpan(tzStore.name, sizeof(tzStore.name))) };
    }
}

CHIP_ERROR TimeSynchronizationServer::SetTimeZone(const DataModel::DecodableList<Structs::TimeZoneStruct::Type> & tzL)
{
    size_t items;
    VerifyOrReturnError(CHIP_NO_ERROR == tzL.ComputeSize(&items), CHIP_IM_GLOBAL_STATUS(InvalidCommand));

    if (items > CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    if (items == 0)
    {
        return ClearTimeZone();
    }

    char name[TimeSyncDataProvider::kTimeZoneNameLength];
    Structs::TimeZoneStruct::Type lastTz;
    TimeState lastTzState = UpdateTimeZoneState();

    if (lastTzState != TimeState::kInvalid)
    {
        const auto & tzStore = GetTimeZone()[0];
        lastTz.offset        = tzStore.timeZone.offset;
        if (tzStore.timeZone.name.HasValue())
        {
            lastTz.name.SetValue(CharSpan(name));
            memcpy(name, tzStore.name, sizeof(tzStore.name));
        }
    }

    auto newTzL = tzL.begin();
    uint8_t i   = 0;
    InitTimeZone();

    while (newTzL.Next())
    {
        auto & tzStore     = mTimeZoneObj.timeZoneList[i];
        const auto & newTz = newTzL.GetValue();
        if (newTz.offset < -43200 || newTz.offset > 50400)
        {
            ReturnErrorOnFailure(LoadTimeZone());
            return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB;
        }
        // first element shall have validAt entry of 0
        if (i == 0 && newTz.validAt != 0)
        {
            ReturnErrorOnFailure(LoadTimeZone());
            return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB;
        }
        // if second element, it shall have validAt entry of non-0
        if (i != 0 && newTz.validAt == 0)
        {
            ReturnErrorOnFailure(LoadTimeZone());
            return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB;
        }
        tzStore.timeZone.offset  = newTz.offset;
        tzStore.timeZone.validAt = newTz.validAt;
        if (newTz.name.HasValue() && newTz.name.Value().size() > 0)
        {
            size_t len = newTz.name.Value().size();
            if (len > sizeof(tzStore.name))
            {
                ReturnErrorOnFailure(LoadTimeZone());
                return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB;
            }
            memset(tzStore.name, 0, sizeof(tzStore.name));
            chip::MutableCharSpan tempSpan(tzStore.name, len);
            if (CHIP_NO_ERROR != CopyCharSpanToMutableCharSpan(newTz.name.Value(), tempSpan))
            {
                ReturnErrorOnFailure(LoadTimeZone());
                return CHIP_IM_GLOBAL_STATUS(InvalidCommand);
            }
            tzStore.timeZone.name.SetValue(CharSpan(tzStore.name, len));
        }
        else
        {
            tzStore.timeZone.name.ClearValue();
        }
        i++;
    }
    if (CHIP_NO_ERROR != newTzL.GetStatus())
    {
        ReturnErrorOnFailure(LoadTimeZone());
        return CHIP_IM_GLOBAL_STATUS(InvalidCommand);
    }

    mTimeZoneObj.validSize = i;

    if (lastTzState != TimeState::kInvalid && TimeState::kInvalid != UpdateTimeZoneState())
    {
        bool emit       = false;
        const auto & tz = GetTimeZone()[0].timeZone;
        if (tz.offset != lastTz.offset)
        {
            emit = true;
        }
        if ((tz.name.HasValue() && lastTz.name.HasValue()) && !(tz.name.Value().data_equal(lastTz.name.Value())))
        {
            emit = true;
        }
        if (emit)
        {
            mEventFlag = TimeSyncEventFlag::kTimeZoneStatus;
        }
    }
    return mTimeSyncDataProvider.StoreTimeZone(GetTimeZone());
}

CHIP_ERROR TimeSynchronizationServer::LoadTimeZone()
{
    InitTimeZone();
    return mTimeSyncDataProvider.LoadTimeZone(mTimeZoneObj);
}

CHIP_ERROR TimeSynchronizationServer::ClearTimeZone()
{
    InitTimeZone();
    return mTimeSyncDataProvider.StoreTimeZone(GetTimeZone());
}

void TimeSynchronizationServer::InitDSTOffset()
{
    mDstOffsetObj.validSize     = 0;
    mDstOffsetObj.dstOffsetList = DataModel::List<Structs::DSTOffsetStruct::Type>(mDst);
}

CHIP_ERROR TimeSynchronizationServer::SetDSTOffset(const DataModel::DecodableList<Structs::DSTOffsetStruct::Type> & dstL)
{
    size_t items;
    VerifyOrReturnError(CHIP_NO_ERROR == dstL.ComputeSize(&items), CHIP_IM_GLOBAL_STATUS(InvalidCommand));

    if (items > CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    if (items == 0)
    {
        return ClearDSTOffset();
    }

    auto newDstL = dstL.begin();
    size_t i     = 0;
    InitDSTOffset();

    while (newDstL.Next())
    {
        auto & dst = mDstOffsetObj.dstOffsetList[i];
        dst        = newDstL.GetValue();
        i++;
    }

    if (CHIP_NO_ERROR != newDstL.GetStatus())
    {
        ReturnErrorOnFailure(LoadDSTOffset());
        return CHIP_IM_GLOBAL_STATUS(InvalidCommand);
    }

    mDstOffsetObj.validSize = i;

    // only 1 validuntil null value and shall be last in the list
    uint64_t lastValidUntil = 0;
    for (i = 0; i < mDstOffsetObj.validSize; i++)
    {
        const auto & dstItem = GetDSTOffset()[i];
        // list should be sorted by validStarting
        // validUntil shall be larger than validStarting
        if (!dstItem.validUntil.IsNull() && dstItem.validStarting >= dstItem.validUntil.Value())
        {
            ReturnErrorOnFailure(LoadDSTOffset());
            return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB;
        }
        // validStarting shall not be smaller than validUntil of previous entry
        if (dstItem.validStarting < lastValidUntil)
        {
            ReturnErrorOnFailure(LoadDSTOffset());
            return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB;
        }
        lastValidUntil = !dstItem.validUntil.IsNull() ? dstItem.validUntil.Value() : lastValidUntil;
        // only 1 validUntil null value and shall be last in the list
        if (dstItem.validUntil.IsNull() && (i != mDstOffsetObj.validSize - 1))
        {
            ReturnErrorOnFailure(LoadDSTOffset());
            return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB;
        }
    }

    return mTimeSyncDataProvider.StoreDSTOffset(GetDSTOffset());
}

CHIP_ERROR TimeSynchronizationServer::LoadDSTOffset()
{
    InitDSTOffset();
    return mTimeSyncDataProvider.LoadDSTOffset(mDstOffsetObj);
}

CHIP_ERROR TimeSynchronizationServer::ClearDSTOffset()
{
    InitDSTOffset();
    ReturnErrorOnFailure(mTimeSyncDataProvider.ClearDSTOffset());
    emitDSTTableEmptyEvent(GetDelegate()->GetEndpoint());
    return CHIP_NO_ERROR;
}

DataModel::Nullable<Structs::TrustedTimeSourceStruct::Type> & TimeSynchronizationServer::GetTrustedTimeSource()
{
    return mTrustedTimeSource;
}

CHIP_ERROR TimeSynchronizationServer::GetDefaultNtp(MutableCharSpan & dntp)
{
    return mTimeSyncDataProvider.LoadDefaultNtp(dntp);
}

Span<TimeSyncDataProvider::TimeZoneStore> & TimeSynchronizationServer::GetTimeZone()
{
    mTimeZoneObj.timeZoneList = mTimeZoneObj.timeZoneList.SubSpan(0, mTimeZoneObj.validSize);
    return mTimeZoneObj.timeZoneList;
}

DataModel::List<Structs::DSTOffsetStruct::Type> & TimeSynchronizationServer::GetDSTOffset()
{
    mDstOffsetObj.dstOffsetList = mDstOffsetObj.dstOffsetList.SubSpan(0, mDstOffsetObj.validSize);
    return mDstOffsetObj.dstOffsetList;
}

void TimeSynchronizationServer::ScheduleDelayedAction(System::Clock::Seconds32 delay, System::TimerCompleteCallback action,
                                                      void * aAppState)
{
    if (CHIP_NO_ERROR != SystemLayer().StartTimer(std::chrono::duration_cast<System::Clock::Timeout>(delay), action, aAppState))
    {
        ChipLogError(Zcl, "Time Synchronization failed to schedule timer.");
    }
}

CHIP_ERROR TimeSynchronizationServer::SetUTCTime(EndpointId ep, uint64_t utcTime, GranularityEnum granularity,
                                                 TimeSourceEnum source)
{
    CHIP_ERROR err = UpdateUTCTime(utcTime);
    if (err != CHIP_NO_ERROR && !RuntimeOptionsProvider::Instance().GetSimulateNoInternalTime())
    {
        ChipLogError(Zcl, "Error setting UTC time on the device");
        return err;
    }
    mGranularity         = granularity;
    EmberAfStatus status = TimeSource::Set(ep, source);
    if (!(status == EMBER_ZCL_STATUS_SUCCESS || status == EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE))
    {
        ChipLogError(Zcl, "Writing TimeSource failed.");
        return CHIP_IM_GLOBAL_STATUS(Failure);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeSynchronizationServer::GetLocalTime(EndpointId ep, DataModel::Nullable<uint64_t> & localTime)
{
    int64_t timeZoneOffset = 0, dstOffset = 0;
    System::Clock::Microseconds64 utcTime;
    uint64_t chipEpochTime;
    if (mGranularity == GranularityEnum::kNoTimeGranularity)
    {
        return CHIP_ERROR_INVALID_TIME;
    }
    TimeState newState = UpdateDSTOffsetState();
    VerifyOrReturnError(TimeState::kInvalid != newState, CHIP_ERROR_INVALID_TIME);
    ReturnErrorOnFailure(System::SystemClock().GetClock_RealTime(utcTime));
    VerifyOrReturnError(UnixEpochToChipEpochMicro(utcTime.count(), chipEpochTime), CHIP_ERROR_INVALID_TIME);
    if (TimeState::kChanged == UpdateTimeZoneState())
    {
        emitTimeZoneStatusEvent(ep);
    }
    VerifyOrReturnError(GetTimeZone().size() != 0, CHIP_ERROR_INVALID_TIME);
    const auto & tzStore = GetTimeZone()[0];
    timeZoneOffset       = static_cast<int64_t>(tzStore.timeZone.offset);
    VerifyOrReturnError(GetDSTOffset().size() != 0, CHIP_ERROR_INVALID_TIME);
    const auto & dst = GetDSTOffset()[0];
    if (dst.validStarting <= chipEpochTime)
    {
        dstOffset = static_cast<int64_t>(dst.offset);
    }

    uint64_t usRemainder = chipEpochTime % chip::kMicrosecondsPerSecond;   // microseconds part of chipEpochTime
    chipEpochTime        = (chipEpochTime / chip::kMicrosecondsPerSecond); // make it safe to cast to int64 by converting to seconds

    uint64_t localTimeSec = static_cast<uint64_t>(static_cast<int64_t>(chipEpochTime) + timeZoneOffset + dstOffset);
    localTime.SetNonNull((localTimeSec * chip::kMicrosecondsPerSecond) + usRemainder);
    if (newState == TimeState::kChanged)
    {
        emitDSTStatusEvent(0, dstOffset != 0);
    }
    return CHIP_NO_ERROR;
}

TimeState TimeSynchronizationServer::UpdateTimeZoneState()
{
    System::Clock::Microseconds64 utcTime;
    auto & tzList        = GetTimeZone();
    size_t activeTzIndex = 0;
    uint64_t chipEpochTime;

    // This return allows us to simulate no internal time for testing purposes
    // This will be set once we receive a good time either from the delegate or via a command
    if (mGranularity == GranularityEnum::kNoTimeGranularity)
    {
        return TimeState::kInvalid;
    }

    VerifyOrReturnValue(System::SystemClock().GetClock_RealTime(utcTime) == CHIP_NO_ERROR, TimeState::kInvalid);
    VerifyOrReturnValue(tzList.size() != 0, TimeState::kInvalid);
    VerifyOrReturnValue(UnixEpochToChipEpochMicro(utcTime.count(), chipEpochTime), TimeState::kInvalid);

    for (size_t i = 0; i < tzList.size(); i++)
    {
        auto & tz = tzList[i].timeZone;
        if (tz.validAt != 0 && tz.validAt <= chipEpochTime)
        {
            tz.validAt    = 0;
            activeTzIndex = i;
        }
    }
    if (activeTzIndex != 0)
    {
        mTimeZoneObj.validSize = tzList.size() - activeTzIndex;
        auto newTimeZoneList   = tzList.SubSpan(activeTzIndex);
        VerifyOrReturnValue(mTimeSyncDataProvider.StoreTimeZone(newTimeZoneList) == CHIP_NO_ERROR, TimeState::kInvalid);
        VerifyOrReturnValue(LoadTimeZone() == CHIP_NO_ERROR, TimeState::kInvalid);
        return TimeState::kChanged;
    }
    return TimeState::kActive;
}

TimeState TimeSynchronizationServer::UpdateDSTOffsetState()
{
    System::Clock::Microseconds64 utcTime;
    auto & dstList        = GetDSTOffset();
    size_t activeDstIndex = 0;
    uint64_t chipEpochTime;
    bool dstStopped = true;

    // This return allows us to simulate no internal time for testing purposes
    // This will be set once we receive a good time either from the delegate or via a command
    if (mGranularity == GranularityEnum::kNoTimeGranularity)
    {
        return TimeState::kInvalid;
    }

    VerifyOrReturnValue(System::SystemClock().GetClock_RealTime(utcTime) == CHIP_NO_ERROR, TimeState::kInvalid);
    VerifyOrReturnValue(dstList.size() != 0, TimeState::kInvalid);
    VerifyOrReturnValue(UnixEpochToChipEpochMicro(utcTime.count(), chipEpochTime), TimeState::kInvalid);

    for (size_t i = 0; i < dstList.size(); i++)
    {
        if (dstList[i].validStarting <= chipEpochTime)
        {
            activeDstIndex = i;
            dstStopped     = false;
        }
    }
    VerifyOrReturnValue(!dstStopped, TimeState::kStopped);
    // if offset is zero and validUntil is null then no DST is used
    if (dstList[activeDstIndex].offset == 0 && dstList[activeDstIndex].validUntil.IsNull())
    {
        return TimeState::kStopped;
    }
    if (!dstList[activeDstIndex].validUntil.IsNull() && dstList[activeDstIndex].validUntil.Value() <= chipEpochTime)
    {
        if (activeDstIndex + 1 >= mDstOffsetObj.validSize) // last item in the list
        {
            VerifyOrReturnValue(ClearDSTOffset() == CHIP_NO_ERROR, TimeState::kInvalid);
            return TimeState::kInvalid;
        }
        int32_t previousOffset         = dstList[activeDstIndex].offset;
        dstList[activeDstIndex].offset = 0; // not using dst and last DST item in the list is not active yet
        // TODO: This enum mixes state and transitions in a way that's very confusing. This should return either an active, an
        // inactive or an invalid and the caller should make the judgement about whether that has changed OR this function should
        // just return a bool indicating whether a change happened
        return previousOffset == 0 ? TimeState::kStopped : TimeState::kChanged;
    }
    if (activeDstIndex > 0)
    {
        mDstOffsetObj.validSize = dstList.size() - activeDstIndex;
        auto newDstOffsetList   = dstList.SubSpan(activeDstIndex);
        VerifyOrReturnValue(mTimeSyncDataProvider.StoreDSTOffset(newDstOffsetList) == CHIP_NO_ERROR, TimeState::kInvalid);
        VerifyOrReturnValue(LoadDSTOffset() == CHIP_NO_ERROR, TimeState::kInvalid);
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

void TimeSynchronizationServer::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    if (!mTrustedTimeSource.IsNull() && mTrustedTimeSource.Value().fabricIndex == fabricIndex)
    {
        DataModel::Nullable<Structs::TrustedTimeSourceStruct::Type> tts;
        TimeSynchronizationServer::Instance().SetTrustedTimeSource(tts);
        emitMissingTrustedTimeSourceEvent(0);
    }
}

namespace {

class TimeSynchronizationAttrAccess : public AttributeAccessInterface
{
public:
    // register for the TimeSync cluster on all endpoints
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
    const auto & tts = TimeSynchronizationServer::Instance().GetTrustedTimeSource();
    return aEncoder.Encode(tts);
}

CHIP_ERROR TimeSynchronizationAttrAccess::ReadDefaultNtp(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char buffer[DefaultNTP::TypeInfo::MaxLength()];
    MutableCharSpan dntp(buffer);
    err = TimeSynchronizationServer::Instance().GetDefaultNtp(dntp);
    if (err == CHIP_NO_ERROR)
    {
        err = aEncoder.Encode(CharSpan(buffer, dntp.size()));
    }
    else if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = aEncoder.EncodeNull();
    }
    return err;
}

CHIP_ERROR TimeSynchronizationAttrAccess::ReadTimeZone(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        const auto & tzList = TimeSynchronizationServer::Instance().GetTimeZone();
        for (const auto & tzStore : tzList)
        {
            ReturnErrorOnFailure(encoder.Encode(tzStore.timeZone));
        }

        return CHIP_NO_ERROR;
    });

    return err;
}

CHIP_ERROR TimeSynchronizationAttrAccess::ReadDSTOffset(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        const auto & dstList = TimeSynchronizationServer::Instance().GetDSTOffset();
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
    DataModel::Nullable<uint64_t> localTime;
    CHIP_ERROR err = TimeSynchronizationServer::Instance().GetLocalTime(endpoint, localTime);
    err            = aEncoder.Encode(localTime);
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
        // This return allows us to simulate no internal time for testing purposes
        // This will be set once we receive a good time either from the delegate or via a command
        if (TimeSynchronizationServer::Instance().GetGranularity() == GranularityEnum::kNoTimeGranularity)
        {
            return aEncoder.EncodeNull();
        }
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
    const auto & timeSource  = commandData.timeSource;

    auto currentGranularity = TimeSynchronizationServer::Instance().GetGranularity();
    if (granularity < GranularityEnum::kNoTimeGranularity || granularity > GranularityEnum::kMicrosecondsGranularity)
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }
    if (timeSource.HasValue() && (timeSource.Value() < TimeSourceEnum::kNone || timeSource.Value() > TimeSourceEnum::kGnss))
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    if (granularity != GranularityEnum::kNoTimeGranularity &&
        (currentGranularity == GranularityEnum::kNoTimeGranularity || granularity >= currentGranularity) &&
        CHIP_NO_ERROR ==
            TimeSynchronizationServer::Instance().SetUTCTime(commandPath.mEndpointId, utcTime, granularity, TimeSourceEnum::kAdmin))
    {
        commandObj->AddStatus(commandPath, Status::Success);
    }
    else
    {
        commandObj->AddClusterSpecificFailure(commandPath, to_underlying(StatusCode::kTimeNotAccepted));
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
        // TODO: schedule a utctime read from this time source and emit event only on failure to get time
        emitTimeFailureEvent(commandPath.mEndpointId);
    }
    else
    {
        tts.SetNull();
        emitMissingTrustedTimeSourceEvent(commandPath.mEndpointId);
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

    CHIP_ERROR err = TimeSynchronizationServer::Instance().SetTimeZone(timeZone);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            commandObj->AddStatus(commandPath, Status::ResourceExhausted);
        }
        else if (err == CHIP_IM_GLOBAL_STATUS(InvalidCommand))
        {
            commandObj->AddStatus(commandPath, Status::InvalidCommand);
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
        emitTimeZoneStatusEvent(commandPath.mEndpointId);
    }
    GetDelegate()->TimeZoneListChanged(TimeSynchronizationServer::Instance().GetTimeZone());

    TimeZoneDatabaseEnum tzDb;
    TimeZoneDatabase::Get(commandPath.mEndpointId, &tzDb);
    Commands::SetTimeZoneResponse::Type response;
    TimeSynchronizationServer::Instance().UpdateTimeZoneState();
    const auto & tzList = TimeSynchronizationServer::Instance().GetTimeZone();
    if (GetDelegate()->HasFeature(Feature::kTimeZone) && tzDb != TimeZoneDatabaseEnum::kNone && tzList.size() != 0)
    {
        auto & tz = tzList[0].timeZone;
        if (tz.name.HasValue() && GetDelegate()->HandleUpdateDSTOffset(tz.name.Value()))
        {
            response.DSTOffsetRequired = false;
            emitDSTStatusEvent(commandPath.mEndpointId, true);
        }
        else
        {
            response.DSTOffsetRequired = true;
        }
    }
    else
    {
        response.DSTOffsetRequired = true;
    }

    if (response.DSTOffsetRequired)
    {
        TimeState dstState = TimeSynchronizationServer::Instance().UpdateDSTOffsetState();
        TimeSynchronizationServer::Instance().ClearDSTOffset();
        if (dstState == TimeState::kActive || dstState == TimeState::kChanged)
        {
            emitDSTStatusEvent(commandPath.mEndpointId, false);
        }
    }

    commandObj->AddResponse(commandPath, response);
    return true;
}

bool emberAfTimeSynchronizationClusterSetDSTOffsetCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::TimeSynchronization::Commands::SetDSTOffset::DecodableType & commandData)
{
    const auto & dstOffset = commandData.DSTOffset;

    TimeState dstState = TimeSynchronizationServer::Instance().UpdateDSTOffsetState();

    CHIP_ERROR err = TimeSynchronizationServer::Instance().SetDSTOffset(dstOffset);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            commandObj->AddStatus(commandPath, Status::ResourceExhausted);
        }
        else if (err == CHIP_IM_GLOBAL_STATUS(InvalidCommand))
        {
            commandObj->AddStatus(commandPath, Status::InvalidCommand);
        }
        else
        {
            commandObj->AddStatus(commandPath, Status::ConstraintError);
        }
        return true;
    }
    // if DST state changes, generate DSTStatus event
    if (dstState != TimeSynchronizationServer::Instance().UpdateDSTOffsetState())
    {
        emitDSTStatusEvent(commandPath.mEndpointId,
                           TimeState::kActive == TimeSynchronizationServer::Instance().UpdateDSTOffsetState());
    }

    commandObj->AddStatus(commandPath, Status::Success);
    return true;
}

bool emberAfTimeSynchronizationClusterSetDefaultNTPCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::TimeSynchronization::Commands::SetDefaultNTP::DecodableType & commandData)
{
    Status status         = Status::Success;
    const auto & dNtpChar = commandData.defaultNTP;

    if (!dNtpChar.IsNull() && dNtpChar.Value().size() > 0)
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
            if (EMBER_ZCL_STATUS_SUCCESS != SupportsDNSResolve::Get(commandPath.mEndpointId, &dnsResolve))
            {
                commandObj->AddStatus(commandPath, Status::Failure);
                return true;
            }
            if (!dnsResolve)
            {
                commandObj->AddStatus(commandPath, Status::InvalidCommand);
                return true;
            }
        }
    }

    status = (CHIP_NO_ERROR == TimeSynchronizationServer::Instance().SetDefaultNTP(dNtpChar)) ? Status::Success : Status::Failure;

    commandObj->AddStatus(commandPath, status);
    return true;
}

void MatterTimeSynchronizationPluginServerInitCallback()
{
    TimeSynchronizationServer::Instance().Init();
    registerAttributeAccessOverride(&gAttrAccess);
}
