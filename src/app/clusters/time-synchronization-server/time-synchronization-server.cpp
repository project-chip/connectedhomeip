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
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/EventLogging.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/RuntimeOptionsProvider.h>

#include <system/SystemClock.h>

#if TIME_SYNC_ENABLE_TSC_FEATURE
#include <app/InteractionModelEngine.h>
#endif

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

static CHIP_ERROR UpdateUTCTime(uint64_t UTCTimeInChipEpochUs)
{
    uint64_t UTCTimeInUnixEpochUs;

    VerifyOrReturnError(ChipEpochToUnixEpochMicros(UTCTimeInChipEpochUs, UTCTimeInUnixEpochUs), CHIP_ERROR_INVALID_TIME);
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
    GetDelegate()->NotifyTimeFailure();
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
            mTimeReadInfo = nullptr;
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
    mGranularity = granularity;
    mTimeSource  = timeSource;
}

void TimeSynchronizationServer::OnFallbackNTPCompletionFn(bool timeSyncSuccessful)
{
    if (timeSyncSuccessful)
    {
        mGranularity = GranularityEnum::kMillisecondsGranularity;
        // Non-matter SNTP because we know it's external and there's only one source
        mTimeSource = TimeSourceEnum::kNonMatterSNTP;
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

void TimeSynchronizationServer::Init(PersistentStorageDelegate & persistentStorage)
{
    mTimeSyncDataProvider.Init(persistentStorage);

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
    GetDelegate()->TrustedTimeSourceAvailabilityChanged(!mTrustedTimeSource.IsNull(), mGranularity);
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
        tzStore.timeZone = { .offset = 0, .validAt = 0, .name = chip::NullOptional };
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
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    char name[TimeSyncDataProvider::kTimeZoneNameLength];
    Structs::TimeZoneStruct::Type lastTz;
    TimeState lastTzState = UpdateTimeZoneState();

    if (lastTzState != TimeState::kInvalid)
    {
        const TimeSyncDataProvider::TimeZoneStore & tzStore = GetTimeZone()[0];
        lastTz.offset                                       = tzStore.timeZone.offset;
        chip::MutableCharSpan tempSpan(name, sizeof(name));
        if (tzStore.timeZone.name.HasValue() &&
            CHIP_NO_ERROR == CopyCharSpanToMutableCharSpan(tzStore.timeZone.name.Value(), tempSpan))
        {
            lastTz.name.SetValue(CharSpan(tempSpan.data(), tempSpan.size()));
        }
    }

    CHIP_ERROR err = CHIP_NO_ERROR;
    TimeState currentTzState;
    auto newTzL = tzL.begin();
    uint8_t i   = 0;
    InitTimeZone();

    while (newTzL.Next())
    {
        auto & tzStore     = mTimeZoneObj.timeZoneList[i];
        const auto & newTz = newTzL.GetValue();
        if (newTz.offset < -43200 || newTz.offset > 50400)
        {
            ExitNow(err = CHIP_IM_GLOBAL_STATUS(ConstraintError));
        }
        // first element shall have validAt entry of 0
        if (i == 0 && newTz.validAt != 0)
        {
            ExitNow(err = CHIP_IM_GLOBAL_STATUS(ConstraintError));
        }
        // if second element, it shall have validAt entry of non-0
        if (i != 0 && newTz.validAt == 0)
        {
            ExitNow(err = CHIP_IM_GLOBAL_STATUS(ConstraintError));
        }
        tzStore.timeZone.offset  = newTz.offset;
        tzStore.timeZone.validAt = newTz.validAt;
        if (newTz.name.HasValue() && newTz.name.Value().size() > 0)
        {
            size_t len = newTz.name.Value().size();
            if (len > sizeof(tzStore.name))
            {
                ExitNow(err = CHIP_IM_GLOBAL_STATUS(ConstraintError));
            }
            memset(tzStore.name, 0, sizeof(tzStore.name));
            chip::MutableCharSpan tempSpan(tzStore.name, len);
            if (CHIP_NO_ERROR != CopyCharSpanToMutableCharSpan(newTz.name.Value(), tempSpan))
            {
                ExitNow(err = CHIP_IM_GLOBAL_STATUS(ConstraintError));
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
        ExitNow(err = CHIP_IM_GLOBAL_STATUS(InvalidCommand));
    }

    mTimeZoneObj.validSize = i;

    currentTzState = UpdateTimeZoneState();
    if (lastTzState == TimeState::kActive && TimeState::kActive == currentTzState)
    {
        bool emit       = false;
        const auto & tz = GetTimeZone()[0].timeZone;
        if (tz.offset != lastTz.offset)
        {
            emit = true;
        }
        if (tz.name.HasValue() != lastTz.name.HasValue() ||
            ((tz.name.HasValue() && lastTz.name.HasValue()) && !(tz.name.Value().data_equal(lastTz.name.Value()))))
        {
            emit = true;
        }
        if (emit)
        {
            emitTimeZoneStatusEvent(GetDelegate()->GetEndpoint());
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        // do not propagate if an error occurs here because it is not the user relevant error
        LoadTimeZone();
    }
    else
    {
        err = mTimeSyncDataProvider.StoreTimeZone(GetTimeZone());
    }
    return err;
}

CHIP_ERROR TimeSynchronizationServer::LoadTimeZone()
{
    InitTimeZone();
    return mTimeSyncDataProvider.LoadTimeZone(mTimeZoneObj);
}

CHIP_ERROR TimeSynchronizationServer::ClearTimeZone()
{
    InitTimeZone();
    ReturnErrorOnFailure(mTimeSyncDataProvider.StoreTimeZone(GetTimeZone()));
    return CHIP_NO_ERROR;
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

    CHIP_ERROR err          = CHIP_NO_ERROR;
    uint64_t lastValidUntil = 0;
    auto newDstL            = dstL.begin();
    size_t i                = 0;
    InitDSTOffset();

    while (newDstL.Next())
    {
        auto & dst = mDstOffsetObj.dstOffsetList[i];
        dst        = newDstL.GetValue();
        i++;
    }

    VerifyOrExit(CHIP_NO_ERROR == newDstL.GetStatus(), err = CHIP_IM_GLOBAL_STATUS(InvalidCommand));

    mDstOffsetObj.validSize = i;

    for (i = 0; i < mDstOffsetObj.validSize; i++)
    {
        const auto & dstItem = mDstOffsetObj.dstOffsetList[i];
        // list should be sorted by validStarting
        // validUntil shall be larger than validStarting
        if (!dstItem.validUntil.IsNull() && dstItem.validStarting >= dstItem.validUntil.Value())
        {
            ExitNow(err = CHIP_IM_GLOBAL_STATUS(ConstraintError));
        }
        // validStarting shall not be smaller than validUntil of previous entry
        if (dstItem.validStarting < lastValidUntil)
        {
            ExitNow(err = CHIP_IM_GLOBAL_STATUS(ConstraintError));
        }
        lastValidUntil = !dstItem.validUntil.IsNull() ? dstItem.validUntil.Value() : lastValidUntil;
        // only 1 validUntil null value and shall be last in the list
        if (dstItem.validUntil.IsNull() && (i != mDstOffsetObj.validSize - 1))
        {
            ExitNow(err = CHIP_IM_GLOBAL_STATUS(ConstraintError));
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        // do not propagate if an error occurs here because it is not the user relevant error
        LoadDSTOffset();
    }
    else
    {
        err = mTimeSyncDataProvider.StoreDSTOffset(GetDSTOffset());
    }

    return err;
}

CHIP_ERROR TimeSynchronizationServer::LoadDSTOffset()
{
    InitDSTOffset();
    return mTimeSyncDataProvider.LoadDSTOffset(mDstOffsetObj);
}

CHIP_ERROR TimeSynchronizationServer::ClearDSTOffset()
{
    InitDSTOffset();
    mDSTOffsetState = TimeState::kInvalid;
    emitDSTTableEmptyEvent(GetDelegate()->GetEndpoint());
    ReturnErrorOnFailure(mTimeSyncDataProvider.ClearDSTOffset());
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
        ChipLogError(Zcl, "Error setting UTC time on the device: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    GetDelegate()->UTCTimeAvailabilityChanged(utcTime);
    mGranularity = static_cast<GranularityEnum>(to_underlying(granularity) - 1);
    mTimeSource  = source;
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
    VerifyOrReturnError(TimeState::kInvalid != UpdateDSTOffsetState(), CHIP_ERROR_INVALID_TIME);
    ReturnErrorOnFailure(System::SystemClock().GetClock_RealTime(utcTime));
    VerifyOrReturnError(UnixEpochToChipEpochMicros(utcTime.count(), chipEpochTime), CHIP_ERROR_INVALID_TIME);
    TimeState tzState = UpdateTimeZoneState();
    VerifyOrReturnError(TimeState::kInvalid != tzState, CHIP_ERROR_INVALID_TIME);
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
        mTimeZoneState = TimeState::kInvalid;
        return mTimeZoneState;
    }

    VerifyOrExit(System::SystemClock().GetClock_RealTime(utcTime) == CHIP_NO_ERROR, mTimeZoneState = TimeState::kInvalid);
    VerifyOrExit(tzList.size() != 0, mTimeZoneState = TimeState::kInvalid);
    VerifyOrExit(UnixEpochToChipEpochMicros(utcTime.count(), chipEpochTime), mTimeZoneState = TimeState::kInvalid);

    for (size_t i = 0; i < tzList.size(); i++)
    {
        auto & tz = tzList[i].timeZone;
        if (tz.validAt != 0 && tz.validAt <= chipEpochTime)
        {
            tz.validAt    = 0;
            activeTzIndex = i;
        }
    }

    if (activeTzIndex != 0) // bring item to the front of the list
    {
        auto newTimeZoneList = tzList.SubSpan(activeTzIndex);
        VerifyOrExit(mTimeSyncDataProvider.StoreTimeZone(newTimeZoneList) == CHIP_NO_ERROR, mTimeZoneState = TimeState::kInvalid);
        VerifyOrExit(LoadTimeZone() == CHIP_NO_ERROR, mTimeZoneState = TimeState::kInvalid);
    }
    if (activeTzIndex || (TimeState::kActive != mTimeZoneState))
    {
        emitTimeZoneStatusEvent(GetDelegate()->GetEndpoint());
    }

    mTimeZoneState = TimeState::kActive;

exit:
    return mTimeZoneState;
}

TimeState TimeSynchronizationServer::UpdateDSTOffsetState()
{
    System::Clock::Microseconds64 utcTime;
    auto & dstList        = GetDSTOffset();
    size_t activeDstIndex = 0;
    uint64_t chipEpochTime;
    bool dstStopped = true;
    TimeState state;

    // This return allows us to simulate no internal time for testing purposes
    // This will be set once we receive a good time either from the delegate or via a command
    if (mGranularity == GranularityEnum::kNoTimeGranularity)
    {
        mDSTOffsetState = TimeState::kInvalid;
        return mDSTOffsetState;
    }

    VerifyOrExit(System::SystemClock().GetClock_RealTime(utcTime) == CHIP_NO_ERROR, mDSTOffsetState = TimeState::kInvalid);
    VerifyOrExit(dstList.size() != 0, mDSTOffsetState = TimeState::kInvalid);
    VerifyOrExit(UnixEpochToChipEpochMicros(utcTime.count(), chipEpochTime), mDSTOffsetState = TimeState::kInvalid);

    for (size_t i = 0; i < dstList.size(); i++)
    {
        if (dstList[i].validStarting <= chipEpochTime)
        {
            activeDstIndex = i;
            dstStopped     = false;
        }
    }

    if (dstStopped)
    {
        if (mDSTOffsetState != TimeState::kStopped)
        {
            mDSTOffsetState = TimeState::kStopped;
            emitDSTStatusEvent(GetDelegate()->GetEndpoint(), false);
        }
        return mDSTOffsetState;
    }

    if (dstList[activeDstIndex].validUntil.IsNull())
    {
        if (dstList[activeDstIndex].offset == 0)
        {
            // no DST is used
            state = TimeState::kStopped;
        }
        else
        {
            // permanent DST is used
            state = TimeState::kActive;
        }
    }
    else if (dstList[activeDstIndex].validUntil.Value() <= chipEpochTime)
    {
        if (activeDstIndex + 1 >= mDstOffsetObj.validSize)
        {
            // this is the last item in the list that has already expired
            ClearDSTOffset();
            // no DST available
            return TimeState::kInvalid;
        }
        // current DST has expired
        // not using DST and next DST item in the list is not active yet
        dstList[activeDstIndex].offset = 0;
        state                          = TimeState::kStopped;
    }
    else
    {
        state = TimeState::kActive;
    }

    if (activeDstIndex > 0)
    {
        auto newDstOffsetList = dstList.SubSpan(activeDstIndex);
        VerifyOrReturnValue(mTimeSyncDataProvider.StoreDSTOffset(newDstOffsetList) == CHIP_NO_ERROR, TimeState::kInvalid);
        VerifyOrReturnValue(LoadDSTOffset() == CHIP_NO_ERROR, TimeState::kInvalid);
        // DST could have been in the same state but the DST item has changed. Therefore, emit the event.
        if (mDSTOffsetState == state)
        {
            emitDSTStatusEvent(GetDelegate()->GetEndpoint(), state == TimeState::kActive);
        }
    }

    if (state != mDSTOffsetState)
    {
        emitDSTStatusEvent(GetDelegate()->GetEndpoint(), state == TimeState::kActive);
        mDSTOffsetState = state;
    }

exit:
    return mDSTOffsetState;
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
        VerifyOrReturnError(UnixEpochToChipEpochMicros(utcTimeUnix.count(), chipEpochTime), aEncoder.EncodeNull());
        return aEncoder.Encode(chipEpochTime);
    }
    case Granularity::Id: {
        return aEncoder.Encode(TimeSynchronizationServer::Instance().GetGranularity());
    }
    case TimeSource::Id: {
        return aEncoder.Encode(TimeSynchronizationServer::Instance().GetTimeSource());
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
        commandObj->AddStatus(commandPath, Status::ConstraintError);
        return true;
    }
    if (timeSource.HasValue() && (timeSource.Value() < TimeSourceEnum::kNone || timeSource.Value() > TimeSourceEnum::kGnss))
    {
        commandObj->AddStatus(commandPath, Status::ConstraintError);
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

    GetDelegate()->TimeZoneListChanged(TimeSynchronizationServer::Instance().GetTimeZone());

    TimeZoneDatabaseEnum tzDb;
    if (Status::Success != TimeZoneDatabase::Get(commandPath.mEndpointId, &tzDb))
    {
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }
    Commands::SetTimeZoneResponse::Type response;
    const auto & tzList = TimeSynchronizationServer::Instance().GetTimeZone();
    if (GetDelegate()->HasFeature(Feature::kTimeZone) && tzDb != TimeZoneDatabaseEnum::kNone && tzList.size() != 0)
    {
        auto & tz = tzList[0].timeZone;
        if (tz.name.HasValue() && GetDelegate()->HandleUpdateDSTOffset(tz.name.Value()))
        {
            response.DSTOffsetRequired = false;
            // update DST to trigger generating DSTStatus event after a new DST setting
            (void) TimeSynchronizationServer::Instance().UpdateDSTOffsetState();
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
        // attempt to emit DSTStatus event if DST was previously available
        (void) TimeSynchronizationServer::Instance().UpdateDSTOffsetState();
        // clear DSTOffset and emit DSTTableEmpty event
        TimeSynchronizationServer::Instance().ClearDSTOffset();
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
    TimeState newDstState = TimeSynchronizationServer::Instance().UpdateDSTOffsetState();
    // if DST state stays in the same state before and after calling SetDSTOffset(), emit DSTStatus event
    // because UpdateDSTOffsetState() has no way of knowing that the state has changed
    if (dstState == newDstState)
    {
        emitDSTStatusEvent(commandPath.mEndpointId, TimeState::kActive == newDstState);
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
        bool dnsResolve;
        if (Status::Success != SupportsDNSResolve::Get(commandPath.mEndpointId, &dnsResolve))
        {
            commandObj->AddStatus(commandPath, Status::Failure);
            return true;
        }
        bool isDomain = GetDelegate()->IsNTPAddressDomain(dNtpChar.Value());
        bool isIPv6   = GetDelegate()->IsNTPAddressValid(dNtpChar.Value());
        bool useable  = isIPv6 || (isDomain && dnsResolve);
        if (!useable)
        {
            commandObj->AddStatus(commandPath, Status::InvalidCommand);
            return true;
        }
    }

    status = (CHIP_NO_ERROR == TimeSynchronizationServer::Instance().SetDefaultNTP(dNtpChar)) ? Status::Success : Status::Failure;

    commandObj->AddStatus(commandPath, status);
    return true;
}

void MatterTimeSynchronizationPluginServerInitCallback()
{
    TimeSynchronizationServer::Instance().Init(Server::GetInstance().GetPersistentStorage());
    registerAttributeAccessOverride(&gAttrAccess);
}
