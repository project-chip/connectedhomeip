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

#include <app/clusters/time-synchronization-server/TimeSynchronizationCluster.h>
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

namespace chip::app::Clusters {

namespace {

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

CHIP_ERROR UpdateUTCTime(uint64_t UTCTimeInChipEpochUs)
{
    uint64_t UTCTimeInUnixEpochUs;

    VerifyOrReturnError(ChipEpochToUnixEpochMicros(UTCTimeInChipEpochUs, UTCTimeInUnixEpochUs), CHIP_ERROR_INVALID_TIME);
    uint64_t secs = UTCTimeInChipEpochUs / kMicrosecondsPerSecond;
    // https://github.com/project-chip/connectedhomeip/issues/27501
    VerifyOrReturnError(secs <= UINT32_MAX, CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
    ReturnErrorOnFailure(Server::GetInstance().GetFabricTable().SetLastKnownGoodChipEpochTime(
        System::Clock::Seconds32(static_cast<uint32_t>(secs))));
    ReturnErrorOnFailure(System::SystemClock().SetClock_RealTime(System::Clock::Microseconds64(UTCTimeInUnixEpochUs)));

    return CHIP_NO_ERROR;
}

// -----------------------------------------------------------------------------
// Event generation functions

bool VerifyAndLogIfNoEventGenerator(DataModel::EventsGenerator * eventsGenerator)
{
    if (eventsGenerator == nullptr)
    {
        ChipLogError(Zcl, "Unable to emit event [ep=%d]. Event generator is null.", kRootEndpointId);
        return false;
    }
    return true;
}

void EmitDSTTableEmptyEvent(DataModel::EventsGenerator * eventsGenerator)
{
    VerifyOrReturn(VerifyAndLogIfNoEventGenerator(eventsGenerator));

    Events::DSTTableEmpty::Type event;
    eventsGenerator->GenerateEvent(event, kRootEndpointId);

    // TODO: re-schedule event for after min 1hr https://github.com/project-chip/connectedhomeip/issues/27200
    // delegate->scheduleDSTTableEmptyEvent()
}

void EmitDSTStatusEvent(bool dstOffsetActive, DataModel::EventsGenerator * eventsGenerator)
{
    VerifyOrReturn(VerifyAndLogIfNoEventGenerator(eventsGenerator));

    Events::DSTStatus::Type event;
    event.DSTOffsetActive = dstOffsetActive;
    eventsGenerator->GenerateEvent(event, kRootEndpointId);
}

void EmitTimeZoneStatusEvent(const Span<TimeSyncDataProvider::TimeZoneStore> & timeZone,
                             DataModel::EventsGenerator * eventsGenerator)
{
    VerifyOrReturn(VerifyAndLogIfNoEventGenerator(eventsGenerator));

    const auto & tzList = timeZone;
    VerifyOrReturn(tzList.size() != 0);
    const auto & tz = tzList[0].timeZone;

    Events::TimeZoneStatus::Type event;
    event.offset = tz.offset;
    if (tz.name.HasValue())
    {
        event.name.SetValue(tz.name.Value());
    }
    eventsGenerator->GenerateEvent(event, kRootEndpointId);
}

void EmitTimeFailureEvent(TimeSynchronization::Delegate * delegate, DataModel::EventsGenerator * eventsGenerator)
{
    VerifyOrReturn(VerifyAndLogIfNoEventGenerator(eventsGenerator));

    Events::TimeFailure::Type event;
    eventsGenerator->GenerateEvent(event, kRootEndpointId);
    delegate->NotifyTimeFailure();

    // TODO: re-schedule event for after min 1hr if no time is still available
    // https://github.com/project-chip/connectedhomeip/issues/27200
}

void EmitMissingTrustedTimeSourceEvent(DataModel::EventsGenerator * eventsGenerator)
{
    VerifyOrReturn(VerifyAndLogIfNoEventGenerator(eventsGenerator));

    Events::MissingTrustedTimeSource::Type event;
    eventsGenerator->GenerateEvent(event, kRootEndpointId);

    // TODO: re-schedule event for after min 1hr if TTS is null or cannot be reached
    // https://github.com/project-chip/connectedhomeip/issues/27200
}

} // namespace

TimeSynchronizationCluster::TimeSynchronizationCluster(EndpointId endpoint, const BitFlags<TimeSynchronization::Feature> features,
                                                       const OptionalAttributeSet & optionalAttributeSet,
                                                       const StartupConfiguration & config) :
    DefaultServerCluster({ endpoint, TimeSynchronization::Id }),
    mFeatures(features), mOptionalAttributeSet(optionalAttributeSet), mSupportsDNSResolve(config.supportsDNSResolve),
    mNTPServerAvailable(config.ntpServerAvailable), mTimeZoneDatabase(config.timeZoneDatabase), mTimeSource(config.timeSource),
    mDelegate(config.delegate),
#if TIME_SYNC_ENABLE_TSC_FEATURE
    mOnDeviceConnectedCallback(OnDeviceConnectedWrapper, this),
    mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureWrapper, this),
#endif
    mOnTimeSyncCompletion(OnTimeSyncCompletionWrapper, this), mOnFallbackNTPCompletion(OnFallbackNTPCompletionWrapper, this)
{
    VerifyOrDie(mDelegate != nullptr);
}

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
    app::OptionalAttributeSet<TrustedTimeSource::Id,    //
                              DefaultNTP::Id,           //
                              TimeZone::Id,             //
                              DSTOffset::Id,            //
                              LocalTime::Id,            //
                              TimeZoneDatabase::Id,     //
                              NTPServerAvailable::Id,   //
                              TimeZoneListMaxSize::Id,  //
                              DSTOffsetListMaxSize::Id, //
                              SupportsDNSResolve::Id,   //
                              TimeSource::Id            //
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
        return encoder.Encode(GetTrustedTimeSource());
    case DefaultNTP::Id: {
        char buffer[DefaultNTP::TypeInfo::MaxLength()];
        MutableCharSpan dntp(buffer);
        CHIP_ERROR err = GetDefaultNtp(dntp);
        // no storage is ok and gets translated to null. Anything else is a real error
        if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            return encoder.EncodeNull();
        }
        ReturnErrorOnFailure(err);
        return encoder.Encode(CharSpan(buffer, dntp.size()));
    }
    case TimeZone::Id:
        return encoder.EncodeList([this](const auto & encod) -> CHIP_ERROR {
            const auto & tzList = GetTimeZone();
            for (const auto & tzStore : tzList)
            {
                ReturnErrorOnFailure(encod.Encode(tzStore.timeZone));
            }
            return CHIP_NO_ERROR;
        });
    case DSTOffset::Id:
        return encoder.EncodeList([this](const auto & encod) -> CHIP_ERROR {
            const auto & dstList = GetDSTOffset();
            for (const auto & dstOffset : dstList)
            {
                ReturnErrorOnFailure(encod.Encode(dstOffset));
            }
            return CHIP_NO_ERROR;
        });
    case TimeZoneListMaxSize::Id:
        return encoder.Encode<uint8_t>(CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE);
    case DSTOffsetListMaxSize::Id:
        return encoder.Encode<uint8_t>(CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE);
    case LocalTime::Id: {
        DataModel::Nullable<uint64_t> localTime;
        CHIP_ERROR err = GetLocalTime(localTime);
        if (err == CHIP_ERROR_INVALID_TIME)
        {
            return encoder.EncodeNull();
        }
        ReturnErrorOnFailure(err);
        return encoder.Encode(localTime);
    }
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
    case NTPServerAvailable::Id:
        return encoder.Encode(mNTPServerAvailable);
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
        TEMPORARY_RETURN_IGNORED ClearTimeZone();
    }
    if (LoadDSTOffset() != CHIP_NO_ERROR)
    {
        TEMPORARY_RETURN_IGNORED ClearDSTOffset();
    }

    // Set the granularity to none for now - this will force us to go to the delegate so it can
    // properly report the time source
    mGranularity = GranularityEnum::kNoTimeGranularity;

    // This can error, but it's not clear what should happen in this case. For now, just ignore it because we still
    // want time sync even if we can't register the delegate here.
    CHIP_ERROR err = Server::GetInstance().GetFabricTable().AddFabricDelegate(this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Unable to register Fabric table delegate for time sync: %" CHIP_ERROR_FORMAT, err.Format());
    }
    err = PlatformMgr().AddEventHandler(OnPlatformEventWrapper, reinterpret_cast<intptr_t>(this));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Unable to add event handler for time sync: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return CHIP_NO_ERROR;
}

void TimeSynchronizationCluster::Shutdown(ClusterShutdownType shutdownType)
{
    PlatformMgr().RemoveEventHandler(OnPlatformEventWrapper, 0);
    Server::GetInstance().GetFabricTable().RemoveFabricDelegate(this);
    DefaultServerCluster::Shutdown(shutdownType);
}

void TimeSynchronizationCluster::AttemptToGetFallbackNTPTimeFromDelegate()
{
    // Sent as a char-string to the delegate so they can read it easily
    char defaultNTP[kMaxDefaultNTPSize];
    MutableCharSpan span(defaultNTP);
    if (GetDefaultNtp(span) != CHIP_NO_ERROR)
    {
        EmitTimeFailureEvent(GetDelegate(), GetEventsGenerator());
        return;
    }
    if (span.size() > kMaxDefaultNTPSize)
    {
        EmitTimeFailureEvent(GetDelegate(), GetEventsGenerator());
        return;
    }
    if (GetDelegate()->UpdateTimeUsingNTPFallback(span, &mOnFallbackNTPCompletion) != CHIP_NO_ERROR)
    {
        EmitTimeFailureEvent(GetDelegate(), GetEventsGenerator());
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

        CHIP_ERROR err = SetUTCTime(mTimeReadInfo->utcTime.Value(), ourGranularity, TimeSourceEnum::kNodeTimeCluster);
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
    SetAttributeValue(mGranularity, granularity, Granularity::Id);
    SetAttributeValue(mTimeSource, timeSource, TimeSource::Id);
}

void TimeSynchronizationCluster::OnFallbackNTPCompletionFn(bool timeSyncSuccessful)
{
    if (timeSyncSuccessful)
    {
        SetAttributeValue(mGranularity, GranularityEnum::kMillisecondsGranularity, Granularity::Id);

        // Non-matter SNTP because we know it's external and there's only one source
        SetAttributeValue(mTimeSource, TimeSourceEnum::kNonMatterSNTP, TimeSource::Id);
    }
    else
    {
        EmitTimeFailureEvent(GetDelegate(), GetEventsGenerator());
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
    auto setAttribute = [this](const DataModel::Nullable<Structs::TrustedTimeSourceStruct::Type> & trustedTimeSource) {
        CHIP_ERROR err     = CHIP_NO_ERROR;
        mTrustedTimeSource = trustedTimeSource;
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
    };

    return NotifyAttributeChangedIfSuccess(TrustedTimeSource::Id, setAttribute(tts)).GetUnderlyingError();
}

inline CHIP_ERROR TimeSynchronizationCluster::SetDefaultNTP(const DataModel::Nullable<CharSpan> & dntp)
{
    auto setAttribute = [this](const DataModel::Nullable<CharSpan> & defaultNtp) {
        if (!defaultNtp.IsNull())
        {
            return mTimeSyncDataProvider.StoreDefaultNtp(defaultNtp.Value());
        }
        return mTimeSyncDataProvider.ClearDefaultNtp();
    };

    return NotifyAttributeChangedIfSuccess(DefaultNTP::Id, setAttribute(dntp)).GetUnderlyingError();
}

void TimeSynchronizationCluster::InitTimeZone()
{
    mTimeZoneObj.validSize    = 1; // one default time zone item is needed
    mTimeZoneObj.timeZoneList = Span<TimeSyncDataProvider::TimeZoneStore>(mTz);
    for (auto & tzStore : mTimeZoneObj.timeZoneList)
    {
        memset(tzStore.name, 0, sizeof(tzStore.name));
        tzStore.timeZone = { .offset = 0, .validAt = 0, .name = NullOptional };
    }
}

CHIP_ERROR TimeSynchronizationCluster::SetTimeZone(const DataModel::DecodableList<Structs::TimeZoneStruct::Type> & tzL)
{
    auto setAttribute = [this](const DataModel::DecodableList<Structs::TimeZoneStruct::Type> & tzList) {
        size_t items;
        VerifyOrReturnError(CHIP_NO_ERROR == tzList.ComputeSize(&items), CHIP_IM_GLOBAL_STATUS(InvalidCommand));

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

        auto newTzL = tzList.begin();
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
                MutableCharSpan tempSpan(tzStore.name, len);
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
    };

    return NotifyAttributeChangedIfSuccess(TimeZone::Id, setAttribute(tzL)).GetUnderlyingError();
}

CHIP_ERROR TimeSynchronizationCluster::LoadTimeZone()
{
    InitTimeZone();
    return mTimeSyncDataProvider.LoadTimeZone(mTimeZoneObj);
}

CHIP_ERROR TimeSynchronizationCluster::ClearTimeZone()
{
    InitTimeZone();
    return mTimeSyncDataProvider.StoreTimeZone(GetTimeZone());
}

void TimeSynchronizationCluster::InitDSTOffset()
{
    mDstOffsetObj.validSize     = 0;
    mDstOffsetObj.dstOffsetList = DataModel::List<Structs::DSTOffsetStruct::Type>(mDst);
}

CHIP_ERROR TimeSynchronizationCluster::SetDSTOffset(const DataModel::DecodableList<Structs::DSTOffsetStruct::Type> & dstL)
{
    auto setAttribute = [this](const DataModel::DecodableList<Structs::DSTOffsetStruct::Type> & dstList) {
        size_t items;
        VerifyOrReturnError(CHIP_NO_ERROR == dstList.ComputeSize(&items), CHIP_IM_GLOBAL_STATUS(InvalidCommand));

        if (items > CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }

        if (items == 0)
        {
            return ClearDSTOffset();
        }

        auto newDstL = dstList.begin();
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
    };

    return NotifyAttributeChangedIfSuccess(DSTOffset::Id, setAttribute(dstL)).GetUnderlyingError();
}

CHIP_ERROR TimeSynchronizationCluster::LoadDSTOffset()
{
    InitDSTOffset();
    return mTimeSyncDataProvider.LoadDSTOffset(mDstOffsetObj);
}

CHIP_ERROR TimeSynchronizationCluster::ClearDSTOffset()
{
    InitDSTOffset();
    ReturnErrorOnFailure(mTimeSyncDataProvider.ClearDSTOffset());
    EmitDSTTableEmptyEvent(GetEventsGenerator());
    return CHIP_NO_ERROR;
}

inline const DataModel::Nullable<Structs::TrustedTimeSourceStruct::Type> & TimeSynchronizationCluster::GetTrustedTimeSource() const
{
    return mTrustedTimeSource;
}

inline CHIP_ERROR TimeSynchronizationCluster::GetDefaultNtp(MutableCharSpan & dntp)
{
    return mTimeSyncDataProvider.LoadDefaultNtp(dntp);
}

inline Span<TimeSyncDataProvider::TimeZoneStore> & TimeSynchronizationCluster::GetTimeZone()
{
    // We can't return a reference to a local temporary object, so we need this assignment
    mTimeZoneObj.timeZoneList = mTimeZoneObj.timeZoneList.SubSpan(0, mTimeZoneObj.validSize);
    return mTimeZoneObj.timeZoneList;
}

inline DataModel::List<Structs::DSTOffsetStruct::Type> & TimeSynchronizationCluster::GetDSTOffset()
{
    // We can't return a reference to a local temporary object, so we need this assignment
    mDstOffsetObj.dstOffsetList = mDstOffsetObj.dstOffsetList.SubSpan(0, mDstOffsetObj.validSize);
    return mDstOffsetObj.dstOffsetList;
}

CHIP_ERROR TimeSynchronizationCluster::SetUTCTime(uint64_t utcTime, GranularityEnum granularity, TimeSourceEnum source)
{
    auto setAttribute = [this](uint64_t utct, GranularityEnum gran, TimeSourceEnum tsrc) {
        CHIP_ERROR err = UpdateUTCTime(utct);
        if (err != CHIP_NO_ERROR && !RuntimeOptionsProvider::Instance().GetSimulateNoInternalTime())
        {
            ChipLogError(Zcl, "Error setting UTC time on the device: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }
        GetDelegate()->UTCTimeAvailabilityChanged(utct);
        SetAttributeValue(mGranularity, gran, Granularity::Id);
        SetAttributeValue(mTimeSource, tsrc, TimeSource::Id);
        return CHIP_NO_ERROR;
    };

    return NotifyAttributeChangedIfSuccess(UTCTime::Id, setAttribute(utcTime, granularity, source)).GetUnderlyingError();
}

CHIP_ERROR TimeSynchronizationCluster::GetLocalTime(DataModel::Nullable<uint64_t> & localTime)
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
        EmitTimeZoneStatusEvent(GetTimeZone(), GetEventsGenerator());
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

    uint64_t usRemainder = chipEpochTime % kMicrosecondsPerSecond;   // microseconds part of chipEpochTime
    chipEpochTime        = (chipEpochTime / kMicrosecondsPerSecond); // make it safe to cast to int64 by converting to seconds

    uint64_t localTimeSec = static_cast<uint64_t>(static_cast<int64_t>(chipEpochTime) + timeZoneOffset + dstOffset);
    localTime.SetNonNull((localTimeSec * kMicrosecondsPerSecond) + usRemainder);
    if (newState == TimeState::kChanged)
    {
        EmitDSTStatusEvent(dstOffset != 0, GetEventsGenerator());
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

inline TimeSynchronization::TimeSyncEventFlag TimeSynchronizationCluster::GetEventFlag() const
{
    return mEventFlag;
}

inline void TimeSynchronizationCluster::ClearEventFlag(TimeSynchronization::TimeSyncEventFlag flag)
{
    uint8_t eventFlag = to_underlying(mEventFlag) ^ to_underlying(flag);
    mEventFlag        = static_cast<TimeSynchronization::TimeSyncEventFlag>(eventFlag);
}

void TimeSynchronizationCluster::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    if (!mTrustedTimeSource.IsNull() && mTrustedTimeSource.Value().fabricIndex == fabricIndex)
    {
        DataModel::Nullable<Structs::TrustedTimeSourceStruct::Type> tts;
        TEMPORARY_RETURN_IGNORED SetTrustedTimeSource(tts);
        EmitMissingTrustedTimeSourceEvent(GetEventsGenerator());
    }
}
std::optional<DataModel::ActionReturnStatus> TimeSynchronizationCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                       TLV::TLVReader & input_arguments,
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
        CHIP_NO_ERROR == SetUTCTime(utcTime, granularity, TimeSourceEnum::kAdmin))
    {
        return Protocols::InteractionModel::Status::Success;
    }

    return Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificFailure(StatusCode::kTimeNotAccepted);
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
        EmitTimeFailureEvent(GetDelegate(), GetEventsGenerator());
    }
    else
    {
        tts.SetNull();
        EmitMissingTrustedTimeSourceEvent(GetEventsGenerator());
    }

    TEMPORARY_RETURN_IGNORED SetTrustedTimeSource(tts);
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
        EmitTimeZoneStatusEvent(GetTimeZone(), GetEventsGenerator());
    }
    GetDelegate()->TimeZoneListChanged(GetTimeZone());

    Commands::SetTimeZoneResponse::Type response;
    response.DSTOffsetRequired = true;
    UpdateTimeZoneState();
    const auto & tzList = GetTimeZone();
    if (mFeatures.Has(Feature::kTimeZone) && mTimeZoneDatabase != TimeZoneDatabaseEnum::kNone && tzList.size() != 0)
    {
        auto & tz = tzList[0].timeZone;
        if (tz.name.HasValue() && GetDelegate()->HandleUpdateDSTOffset(tz.name.Value()))
        {
            response.DSTOffsetRequired = false;
            EmitDSTStatusEvent(true, GetEventsGenerator());
        }
    }

    if (response.DSTOffsetRequired)
    {
        TimeState dstState = UpdateDSTOffsetState();
        TEMPORARY_RETURN_IGNORED ClearDSTOffset();
        if (dstState == TimeState::kActive || dstState == TimeState::kChanged)
        {
            EmitDSTStatusEvent(false, GetEventsGenerator());
        }
    }

    commandObj->AddResponse(commandPath, response);
    return std::nullopt;
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
        EmitDSTStatusEvent(TimeState::kActive == UpdateDSTOffsetState(), GetEventsGenerator());
    }

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

    return Protocols::InteractionModel::Status::Success;
}

} // namespace chip::app::Clusters
