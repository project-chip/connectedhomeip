/*
 *    Copyright (c) 2023-2025 Project CHIP Authors
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

#include <app/clusters/time-synchronization-server/CodegenIntegration.h>
#include <app/clusters/time-synchronization-server/time-synchronization-cluster.h>
#include <app/clusters/time-synchronization-server/time-synchronization-delegate.h>
#include <app/server-cluster/AttributeListBuilder.h>

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

namespace {

CHIP_ERROR UpdateUTCTime(uint64_t UTCTimeInChipEpochUs)
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

} // namespace

namespace chip::app::Clusters {

namespace TimeSynchronization {

// -----------------------------------------------------------------------------
// Event generation functions

bool emitDSTTableEmptyEvent(EndpointId ep, DataModel::EventsGenerator * eventsGenerator)
{
    Events::DSTTableEmpty::Type event;
    std::optional<EventNumber> eventNumber;

    if (eventsGenerator != nullptr)
    {
        eventNumber = eventsGenerator->GenerateEvent(event, ep);
    }
    if (eventNumber == std::nullopt)
    {
        ChipLogError(Zcl, "Unable to emit DSTTableEmpty event [ep=%d]", ep);
        return false;
    }
    ChipLogProgress(Zcl, "Emit DSTTableEmpty event [ep=%d]", ep);

    // TODO: re-schedule event for after min 1hr https://github.com/project-chip/connectedhomeip/issues/27200
    // delegate->scheduleDSTTableEmptyEvent()
    return true;
}

bool emitDSTStatusEvent(EndpointId ep, bool dstOffsetActive, DataModel::EventsGenerator * eventsGenerator)
{
    Events::DSTStatus::Type event;
    event.DSTOffsetActive = dstOffsetActive;
    std::optional<EventNumber> eventNumber;

    if (eventsGenerator != nullptr)
    {
        eventNumber = eventsGenerator->GenerateEvent(event, ep);
    }
    if (eventNumber == std::nullopt)
    {
        ChipLogError(Zcl, "Unable to emit DSTStatus event [ep=%d]", ep);
        return false;
    }

    ChipLogProgress(Zcl, "Emit DSTStatus event [ep=%d]", ep);
    return true;
}

bool emitTimeZoneStatusEvent(EndpointId ep, DataModel::EventsGenerator * eventsGenerator)
{
    auto timeSynchronization = GetClusterInstance();
    VerifyOrReturnValue(timeSynchronization != nullptr, false);

    const auto & tzList = timeSynchronization->GetTimeZone();
    VerifyOrReturnValue(tzList.size() != 0, false);
    const auto & tz = tzList[0].timeZone;

    Events::TimeZoneStatus::Type event;

    event.offset = tz.offset;
    if (tz.name.HasValue())
    {
        event.name.SetValue(tz.name.Value());
    }

    std::optional<EventNumber> eventNumber;
    if (eventsGenerator != nullptr)
    {
        eventNumber = eventsGenerator->GenerateEvent(event, ep);
    }
    if (eventNumber == std::nullopt)
    {
        ChipLogError(Zcl, "Unable to emit TimeZoneStatus event [ep=%d]", ep);
        return false;
    }

    ChipLogProgress(Zcl, "Emit TimeZoneStatus event [ep=%d]", ep);
    return true;
}

bool emitTimeFailureEvent(EndpointId ep, DataModel::EventsGenerator * eventsGenerator)
{
    auto timeSynchronization = GetClusterInstance();
    VerifyOrReturnValue(timeSynchronization != nullptr, false);

    Events::TimeFailure::Type event;

    std::optional<EventNumber> eventNumber;
    if (eventsGenerator != nullptr)
    {
        eventNumber = eventsGenerator->GenerateEvent(event, ep);
    }
    if (eventNumber == std::nullopt)
    {
        ChipLogError(Zcl, "Unable to emit TimeFailure event [ep=%d]", ep);
        return false;
    }

    // TODO: re-schedule event for after min 1hr if no time is still available
    // https://github.com/project-chip/connectedhomeip/issues/27200
    ChipLogProgress(Zcl, "Emit TimeFailure event [ep=%d]", ep);
    timeSynchronization->GetDelegate()->NotifyTimeFailure();
    return true;
}

bool emitMissingTrustedTimeSourceEvent(EndpointId ep, DataModel::EventsGenerator * eventsGenerator)
{
    Events::MissingTrustedTimeSource::Type event;

    std::optional<EventNumber> eventNumber;
    if (eventsGenerator != nullptr)
    {
        eventNumber = eventsGenerator->GenerateEvent(event, ep);
    }
    if (eventNumber == std::nullopt)
    {
        ChipLogError(Zcl, "Unable to emit MissingTrustedTimeSource event [ep=%d]", ep);
        return false;
    }

    // TODO: re-schedule event for after min 1hr if TTS is null or cannot be reached
    // https://github.com/project-chip/connectedhomeip/issues/27200
    ChipLogProgress(Zcl, "Emit MissingTrustedTimeSource event [ep=%d]", ep);
    return true;
}

} // namespace TimeSynchronization

#if TIME_SYNC_ENABLE_TSC_FEATURE
void OnDeviceConnectedWrapper(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    TimeSynchronizationCluster * timeSynchronization = reinterpret_cast<TimeSynchronizationCluster *>(context);
    timeSynchronization->OnDeviceConnectedFn(exchangeMgr, sessionHandle);
}

void OnDeviceConnectionFailureWrapper(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    TimeSynchronizationCluster * timeSynchronization = reinterpret_cast<TimeSynchronizationCluster *>(context);
    timeSynchronization->OnDeviceConnectionFailureFn();
}
#endif

void OnPlatformEventWrapper(const DeviceLayer::ChipDeviceEvent * event, intptr_t ptr)
{
    TimeSynchronizationCluster * timeSynchronization = reinterpret_cast<TimeSynchronizationCluster *>(ptr);
    timeSynchronization->OnPlatformEventFn(*event);
}

void OnTimeSyncCompletionWrapper(void * context, TimeSourceEnum timeSource, GranularityEnum granularity)
{
    TimeSynchronizationCluster * timeSynchronization = reinterpret_cast<TimeSynchronizationCluster *>(context);
    timeSynchronization->OnTimeSyncCompletionFn(timeSource, granularity);
}

void OnFallbackNTPCompletionWrapper(void * context, bool timeSyncSuccessful)
{
    TimeSynchronizationCluster * timeSynchronization = reinterpret_cast<TimeSynchronizationCluster *>(context);
    timeSynchronization->OnFallbackNTPCompletionFn(timeSyncSuccessful);
}

TimeSynchronizationCluster::TimeSynchronizationCluster(
    EndpointId endpoint, const TimeSynchronizationCluster::OptionalAttributeSet & optionalAttributeSet,
    const BitFlags<TimeSynchronization::Feature> features,
    TimeSynchronization::Attributes::SupportsDNSResolve::TypeInfo::Type supportsDNSResolve, TimeZoneDatabaseEnum timeZoneDatabase,
    TimeSynchronization::TimeSourceEnum timeSource) :
    DefaultServerCluster({ endpoint, TimeSynchronization::Id }),
    mOptionalAttributeSet(optionalAttributeSet), mFeatures(features), mSupportsDNSResolve(supportsDNSResolve),
    mTimeZoneDatabase(timeZoneDatabase), mTimeSource(timeSource),
#if TIME_SYNC_ENABLE_TSC_FEATURE
    mOnDeviceConnectedCallback(OnDeviceConnectedWrapper, this),
    mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureWrapper, this),
#endif
    mOnTimeSyncCompletion(OnTimeSyncCompletionWrapper, this), mOnFallbackNTPCompletion(OnFallbackNTPCompletionWrapper, this)
{}

CHIP_ERROR TimeSynchronizationCluster::Attributes(const ConcreteClusterPath & path,
                                                  ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder attributeListBuilder(builder);

    const DataModel::AttributeEntry optionalAttributes[] = {
        TrustedTimeSource::kMetadataEntry,    //
        DefaultNTP::kMetadataEntry,           //
        TimeZone::kMetadataEntry,             //
        DSTOffset::kMetadataEntry,            //
        LocalTime::kMetadataEntry,            //
        TimeZoneDatabase::kMetadataEntry,     //
        NTPServerAvailable::kMetadataEntry,   //
        TimeZoneListMaxSize::kMetadataEntry,  //
        DSTOffsetListMaxSize::kMetadataEntry, //
        SupportsDNSResolve::kMetadataEntry,   //
        TimeSource::kMetadataEntry            //
    };

    // Full attribute set, to combine real "optional" attributes but also
    // attributes controlled by feature flags.
    chip::app::OptionalAttributeSet<TimeSynchronization::Attributes::TrustedTimeSource::Id,    //
                                    TimeSynchronization::Attributes::DefaultNTP::Id,           //
                                    TimeSynchronization::Attributes::TimeZone::Id,             //
                                    TimeSynchronization::Attributes::DSTOffset::Id,            //
                                    TimeSynchronization::Attributes::LocalTime::Id,            //
                                    TimeSynchronization::Attributes::TimeZoneDatabase::Id,     //
                                    TimeSynchronization::Attributes::NTPServerAvailable::Id,   //
                                    TimeSynchronization::Attributes::TimeZoneListMaxSize::Id,  //
                                    TimeSynchronization::Attributes::DSTOffsetListMaxSize::Id, //
                                    TimeSynchronization::Attributes::SupportsDNSResolve::Id,   //
                                    TimeSynchronization::Attributes::TimeSource::Id            //
                                    >
        optionalAttributeSet(mOptionalAttributeSet);

    if (mFeatures.Has(Feature::kTimeSyncClient))
    {
        optionalAttributeSet.Set<TrustedTimeSource::Id>();
    }

    if (mFeatures.Has(Feature::kNTPClient))
    {
        optionalAttributeSet.Set<DefaultNTP::Id>();
        optionalAttributeSet.Set<SupportsDNSResolve::Id>();
    }

    if (mFeatures.Has(Feature::kNTPServer))
    {
        optionalAttributeSet.Set<NTPServerAvailable::Id>();
    }

    if (mFeatures.Has(Feature::kTimeZone))
    {
        optionalAttributeSet.Set<TimeZone::Id>();
        optionalAttributeSet.Set<DSTOffset::Id>();
        optionalAttributeSet.Set<LocalTime::Id>();
        optionalAttributeSet.Set<TimeZoneDatabase::Id>();
        optionalAttributeSet.Set<TimeZoneListMaxSize::Id>();
        optionalAttributeSet.Set<DSTOffsetListMaxSize::Id>();
    }
    return attributeListBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet);
}

DataModel::ActionReturnStatus TimeSynchronizationCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                        AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case UTCTime::Id: {
        System::Clock::Microseconds64 utcTimeUnix;
        uint64_t chipEpochTime;
        // This return allows us to simulate no internal time for testing purposes
        // This will be set once we receive a good time either from the delegate or via a command
        if (mGranularity == GranularityEnum::kNoTimeGranularity)
        {
            return encoder.EncodeNull();
        }
        VerifyOrReturnError(System::SystemClock().GetClock_RealTime(utcTimeUnix) == CHIP_NO_ERROR, encoder.EncodeNull());
        VerifyOrReturnError(UnixEpochToChipEpochMicros(utcTimeUnix.count(), chipEpochTime), encoder.EncodeNull());
        return encoder.Encode(chipEpochTime);
    }
    case Granularity::Id:
        return encoder.Encode(mGranularity);
    case TrustedTimeSource::Id:
        return ReadTrustedTimeSource(encoder);
    case DefaultNTP::Id:
        return ReadDefaultNtp(encoder);
    case TimeZone::Id:
        return ReadTimeZone(encoder);
    case DSTOffset::Id:
        return ReadDSTOffset(encoder);
    case TimeZoneListMaxSize::Id:
        return encoder.Encode<uint8_t>(CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE);
    case DSTOffsetListMaxSize::Id:
        return encoder.Encode<uint8_t>(CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE);
    case LocalTime::Id:
        return ReadLocalTime(encoder);
    case ClusterRevision::Id:
        return encoder.Encode(TimeSynchronization::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case SupportsDNSResolve::Id:
        return encoder.Encode(mSupportsDNSResolve);
    case TimeZoneDatabase::Id:
        return encoder.Encode(mTimeZoneDatabase);
    case TimeSource::Id:
        return encoder.Encode(mTimeSource);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR TimeSynchronizationCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    mTimeSyncDataProvider.Init(context.storage);

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
    // want time sync even if we can't register the delegate here.
    CHIP_ERROR err = Server::GetInstance().GetFabricTable().AddFabricDelegate(this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Unable to register Fabric table delegate for time sync");
    }
    PlatformMgr().AddEventHandler(OnPlatformEventWrapper, reinterpret_cast<intptr_t>(this));

    return CHIP_NO_ERROR;
}

void TimeSynchronizationCluster::Shutdown()
{
    PlatformMgr().RemoveEventHandler(OnPlatformEventWrapper, 0);
    Server::GetInstance().GetFabricTable().RemoveFabricDelegate(this);
    DefaultServerCluster::Shutdown();
}

void TimeSynchronizationCluster::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    if (!mTrustedTimeSource.IsNull() && mTrustedTimeSource.Value().fabricIndex == fabricIndex)
    {
        DataModel::Nullable<Structs::TrustedTimeSourceStruct::Type> tts;
        SetTrustedTimeSource(tts);
        emitMissingTrustedTimeSourceEvent(0, GetEventsGenerator());
    }
}

void TimeSynchronizationCluster::AttemptToGetFallbackNTPTimeFromDelegate()
{
    // Sent as a char-string to the delegate so they can read it easily
    char defaultNTP[kMaxDefaultNTPSize];
    MutableCharSpan span(defaultNTP);
    if (GetDefaultNtp(span) != CHIP_NO_ERROR)
    {
        emitTimeFailureEvent(kRootEndpointId, GetEventsGenerator());
        return;
    }
    if (span.size() > kMaxDefaultNTPSize)
    {
        emitTimeFailureEvent(kRootEndpointId, GetEventsGenerator());
        return;
    }
    if (GetDelegate()->UpdateTimeUsingNTPFallback(span, &mOnFallbackNTPCompletion) != CHIP_NO_ERROR)
    {
        emitTimeFailureEvent(kRootEndpointId, GetEventsGenerator());
    }
}

#if TIME_SYNC_ENABLE_TSC_FEATURE
void TimeSynchronizationCluster::OnDeviceConnectedFn(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    // Connected to our trusted time source, let's read the time.
    AttributePathParams readPaths[2];
    readPaths[0] = AttributePathParams(kRootEndpointId, Id, UTCTime::Id);
    readPaths[1] = AttributePathParams(kRootEndpointId, Id, Granularity::Id);

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

void TimeSynchronizationCluster::OnDeviceConnectionFailureFn()
{
    // No way to read from the TrustedTimeSource, fall back to default NTP
    AttemptToGetFallbackNTPTimeFromDelegate();
}

void TimeSynchronizationCluster::OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                                 const StatusIB & aStatus)
{
    if (aPath.mClusterId != Id || aStatus.IsFailure())
    {
        return;
    }
    switch (aPath.mAttributeId)
    {
    case UTCTime::Id:
        if (DataModel::Decode(*apData, mTimeReadInfo->utcTime) != CHIP_NO_ERROR)
        {
            mTimeReadInfo->utcTime.SetNull();
        }
        break;
    case Granularity::Id:
        if (DataModel::Decode(*apData, mTimeReadInfo->granularity) != CHIP_NO_ERROR)
        {
            mTimeReadInfo->granularity = GranularityEnum::kNoTimeGranularity;
        }
        break;
    default:
        break;
    }
}

void TimeSynchronizationCluster::OnDone(ReadClient * apReadClient)
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

void TimeSynchronizationCluster::OnTimeSyncCompletionFn(TimeSourceEnum timeSource, GranularityEnum granularity)
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
    mGranularity  = granularity;
    Status status = TimeSource::Set(kRootEndpointId, timeSource);
    if (!(status == Status::Success || status == Status::UnsupportedAttribute))
    {
        ChipLogError(Zcl, "Writing TimeSource failed.");
    }
}

void TimeSynchronizationCluster::OnFallbackNTPCompletionFn(bool timeSyncSuccessful)
{
    if (timeSyncSuccessful)
    {
        mGranularity = GranularityEnum::kMillisecondsGranularity;
        // Non-matter SNTP because we know it's external and there's only one source
        Status status = TimeSource::Set(kRootEndpointId, TimeSourceEnum::kNonMatterSNTP);
        if (!(status == Status::Success || status == Status::UnsupportedAttribute))
        {
            ChipLogError(Zcl, "Writing TimeSource failed.");
        }
    }
    else
    {
        emitTimeFailureEvent(kRootEndpointId, GetEventsGenerator());
    }
}

CHIP_ERROR TimeSynchronizationCluster::AttemptToGetTimeFromTrustedNode()
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

void TimeSynchronizationCluster::AttemptToGetTime()
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

void TimeSynchronizationCluster::OnPlatformEventFn(const DeviceLayer::ChipDeviceEvent & event)
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

CHIP_ERROR TimeSynchronizationCluster::SetTrustedTimeSource(const DataModel::Nullable<Structs::TrustedTimeSourceStruct::Type> & tts)
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

CHIP_ERROR TimeSynchronizationCluster::SetDefaultNTP(const DataModel::Nullable<CharSpan> & dntp)
{
    if (!dntp.IsNull())
    {
        return mTimeSyncDataProvider.StoreDefaultNtp(dntp.Value());
    }
    return mTimeSyncDataProvider.ClearDefaultNtp();
}

CHIP_ERROR TimeSynchronizationCluster::SetTimeZone(const DataModel::DecodableList<Structs::TimeZoneStruct::Type> & tzL)
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

CHIP_ERROR TimeSynchronizationCluster::SetDSTOffset(const DataModel::DecodableList<Structs::DSTOffsetStruct::Type> & dstL)
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

CHIP_ERROR TimeSynchronizationCluster::ClearDSTOffset()
{
    InitDSTOffset();
    ReturnErrorOnFailure(mTimeSyncDataProvider.ClearDSTOffset());
    emitDSTTableEmptyEvent(GetDelegate()->GetEndpoint(), GetEventsGenerator());
    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeSynchronizationCluster::SetUTCTime(EndpointId ep, uint64_t utcTime, GranularityEnum granularity,
                                                  TimeSourceEnum source)
{
    CHIP_ERROR err = UpdateUTCTime(utcTime);
    if (err != CHIP_NO_ERROR && !RuntimeOptionsProvider::Instance().GetSimulateNoInternalTime())
    {
        ChipLogError(Zcl, "Error setting UTC time on the device: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    GetDelegate()->UTCTimeAvailabilityChanged(utcTime);
    mGranularity  = granularity;
    Status status = TimeSource::Set(ep, source);
    if (!(status == Status::Success || status == Status::UnsupportedAttribute))
    {
        ChipLogError(Zcl, "Writing TimeSource failed.");
        return CHIP_IM_GLOBAL_STATUS(Failure);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeSynchronizationCluster::GetLocalTime(EndpointId ep, DataModel::Nullable<uint64_t> & localTime)
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
    VerifyOrReturnError(UnixEpochToChipEpochMicros(utcTime.count(), chipEpochTime), CHIP_ERROR_INVALID_TIME);
    if (TimeState::kChanged == UpdateTimeZoneState())
    {
        emitTimeZoneStatusEvent(ep, GetEventsGenerator());
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
        emitDSTStatusEvent(0, dstOffset != 0, GetEventsGenerator());
    }
    return CHIP_NO_ERROR;
}

TimeState TimeSynchronizationCluster::UpdateTimeZoneState()
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
    VerifyOrReturnValue(UnixEpochToChipEpochMicros(utcTime.count(), chipEpochTime), TimeState::kInvalid);

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

TimeState TimeSynchronizationCluster::UpdateDSTOffsetState()
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
    VerifyOrReturnValue(UnixEpochToChipEpochMicros(utcTime.count(), chipEpochTime), TimeState::kInvalid);

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
        // inactive or an invalid and the caller should make the judgment about whether that has changed OR this function should
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

std::optional<DataModel::ActionReturnStatus> TimeSynchronizationCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                       chip::TLV::TLVReader & input_arguments,
                                                                                       CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::SetUTCTime::Id: {
        Commands::SetUTCTime::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return HandleSetUTCTime(handler, request.path, data);
    }
    case Commands::SetTrustedTimeSource::Id: {
        Commands::SetTrustedTimeSource::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments, handler->GetAccessingFabricIndex()));
        return HandleSetTrustedTimeSource(handler, request.path, data);
    }
    case Commands::SetTimeZone::Id: {
        Commands::SetTimeZone::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return HandleSetTimeZone(handler, request.path, data);
    }
    case Commands::SetDSTOffset::Id: {
        Commands::SetDSTOffset::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return HandleSetDSTOffset(handler, request.path, data);
    }
    case Commands::SetDefaultNTP::Id: {
        Commands::SetDefaultNTP::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return HandleSetDefaultNTP(handler, request.path, data);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR TimeSynchronizationCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mFeatures.Has(Feature::kTimeSyncClient))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            Commands::SetTrustedTimeSource::kMetadataEntry,
        }));
    }

    if (mFeatures.Has(Feature::kTimeZone))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            Commands::SetTimeZone::kMetadataEntry,
            Commands::SetDSTOffset::kMetadataEntry,
        }));
    }

    if (mFeatures.Has(Feature::kNTPClient))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            Commands::SetDefaultNTP::kMetadataEntry,
        }));
    }

    return builder.AppendElements({
        Commands::SetUTCTime::kMetadataEntry,
    });
}

CHIP_ERROR TimeSynchronizationCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                                         ReadOnlyBufferBuilder<CommandId> & builder)
{
    if (mFeatures.Has(Feature::kTimeZone))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            Commands::SetTimeZoneResponse::Id,
        }));
    }

    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus>
TimeSynchronizationCluster::HandleSetUTCTime(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                             const Commands::SetUTCTime::DecodableType & commandData)
{
    const auto & utcTime     = commandData.UTCTime;
    const auto & granularity = commandData.granularity;
    const auto & timeSource  = commandData.timeSource;

    auto currentGranularity = GetGranularity();
    if (granularity < GranularityEnum::kNoTimeGranularity || granularity > GranularityEnum::kMicrosecondsGranularity)
    {
        return Protocols::InteractionModel::Status::InvalidCommand;
    }
    if (timeSource.HasValue() && (timeSource.Value() < TimeSourceEnum::kNone || timeSource.Value() > TimeSourceEnum::kGnss))
    {
        return Protocols::InteractionModel::Status::InvalidCommand;
    }

    if (granularity != GranularityEnum::kNoTimeGranularity &&
        (currentGranularity == GranularityEnum::kNoTimeGranularity || granularity >= currentGranularity) &&
        CHIP_NO_ERROR == SetUTCTime(commandPath.mEndpointId, utcTime, granularity, TimeSourceEnum::kAdmin))
    {
        NotifyAttributeChanged(UTCTime::Id);
        return Protocols::InteractionModel::Status::Success;
    }

    commandObj->AddClusterSpecificFailure(commandPath, to_underlying(StatusCode::kTimeNotAccepted));
    return Protocols::InteractionModel::Status::Failure;
}

std::optional<DataModel::ActionReturnStatus>
TimeSynchronizationCluster::HandleSetTrustedTimeSource(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                       const Commands::SetTrustedTimeSource::DecodableType & commandData)
{
    const auto & timeSource = commandData.trustedTimeSource;
    DataModel::Nullable<Structs::TrustedTimeSourceStruct::Type> tts;

    if (!timeSource.IsNull())
    {
        Structs::TrustedTimeSourceStruct::Type ts = { commandObj->GetAccessingFabricIndex(), timeSource.Value().nodeID,
                                                      timeSource.Value().endpoint };
        tts.SetNonNull(ts);
        // TODO: schedule a utctime read from this time source and emit event only on failure to get time
        emitTimeFailureEvent(commandPath.mEndpointId, GetEventsGenerator());
    }
    else
    {
        tts.SetNull();
        emitMissingTrustedTimeSourceEvent(commandPath.mEndpointId, GetEventsGenerator());
    }

    SetTrustedTimeSource(tts);
    NotifyAttributeChanged(TrustedTimeSource::Id);
    return Protocols::InteractionModel::Status::Success;
}

std::optional<DataModel::ActionReturnStatus>
TimeSynchronizationCluster::HandleSetTimeZone(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                              const Commands::SetTimeZone::DecodableType & commandData)
{
    const auto & timeZone = commandData.timeZone;

    CHIP_ERROR err = SetTimeZone(timeZone);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            return Protocols::InteractionModel::Status::ResourceExhausted;
        }
        if (err == CHIP_IM_GLOBAL_STATUS(InvalidCommand))
        {
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    if (to_underlying(mEventFlag) & to_underlying(TimeSyncEventFlag::kTimeZoneStatus))
    {
        ClearEventFlag(TimeSyncEventFlag::kTimeZoneStatus);
        emitTimeZoneStatusEvent(commandPath.mEndpointId, GetEventsGenerator());
    }
    GetDelegate()->TimeZoneListChanged(GetTimeZone());

    Commands::SetTimeZoneResponse::Type response;
    response.DSTOffsetRequired = true;
    UpdateTimeZoneState();
    const auto & tzList = GetTimeZone();
    if (mFeatures.Has(TimeSynchronization::Feature::kTimeZone) && mTimeZoneDatabase != TimeZoneDatabaseEnum::kNone &&
        tzList.size() != 0)
    {
        auto & tz = tzList[0].timeZone;
        if (tz.name.HasValue() && GetDelegate()->HandleUpdateDSTOffset(tz.name.Value()))
        {
            response.DSTOffsetRequired = false;
            emitDSTStatusEvent(commandPath.mEndpointId, true, GetEventsGenerator());
        }
    }

    if (response.DSTOffsetRequired)
    {
        TimeState dstState = UpdateDSTOffsetState();
        ClearDSTOffset();
        if (dstState == TimeState::kActive || dstState == TimeState::kChanged)
        {
            emitDSTStatusEvent(commandPath.mEndpointId, false, GetEventsGenerator());
        }
    }

    commandObj->AddResponse(commandPath, response);
    NotifyAttributeChanged(TimeZone::Id);
    return Protocols::InteractionModel::Status::Success;
}

std::optional<DataModel::ActionReturnStatus>
TimeSynchronizationCluster::HandleSetDSTOffset(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                               const Commands::SetDSTOffset::DecodableType & commandData)
{
    const auto & dstOffset = commandData.DSTOffset;

    TimeState dstState = UpdateDSTOffsetState();

    CHIP_ERROR err = SetDSTOffset(dstOffset);
    if (err != CHIP_NO_ERROR)
    {
        if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            return Protocols::InteractionModel::Status::ResourceExhausted;
        }
        if (err == CHIP_IM_GLOBAL_STATUS(InvalidCommand))
        {
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    // if DST state changes, generate DSTStatus event
    if (dstState != UpdateDSTOffsetState())
    {
        emitDSTStatusEvent(commandPath.mEndpointId, TimeState::kActive == UpdateDSTOffsetState(), GetEventsGenerator());
    }

    NotifyAttributeChanged(DSTOffset::Id);
    return Protocols::InteractionModel::Status::Success;
}

std::optional<DataModel::ActionReturnStatus>
TimeSynchronizationCluster::HandleSetDefaultNTP(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                const Commands::SetDefaultNTP::DecodableType & commandData)
{
    const auto & dNtpChar = commandData.defaultNTP;

    if (!dNtpChar.IsNull() && dNtpChar.Value().size() > 0)
    {
        size_t len = dNtpChar.Value().size();
        if (len > DefaultNTP::TypeInfo::MaxLength())
        {
            return Protocols::InteractionModel::Status::ConstraintError;
        }
        bool isDomain = GetDelegate()->IsNTPAddressDomain(dNtpChar.Value());
        bool isIPv6   = GetDelegate()->IsNTPAddressValid(dNtpChar.Value());
        bool useable  = isIPv6 || (isDomain && mSupportsDNSResolve);
        if (!useable)
        {
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
    }

    if (CHIP_NO_ERROR != SetDefaultNTP(dNtpChar))
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    NotifyAttributeChanged(DefaultNTP::Id);
    return Protocols::InteractionModel::Status::Success;
}

} // namespace chip::app::Clusters
