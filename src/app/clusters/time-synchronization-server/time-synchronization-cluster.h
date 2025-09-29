/*
 *    Copyright (c) 2023-2025 Project CHIP Authors
 *    All rights reserved.
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
#pragma once

#include "TimeSyncDataProvider.h"
#include "time-synchronization-delegate.h"
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/TimeSynchronization/Metadata.h>
#include <platform/DeviceInfoProvider.h>

#include <app/server/Server.h>

namespace chip::app::Clusters {

namespace TimeSynchronization {

/**
 * @brief Describes the state of time zone and DSTOffset in use.
 */
enum class TimeState : uint8_t
{
    kInvalid = 0, // No valid offset available
    kActive  = 1, // An offset is currently being used
    kChanged = 2, // An offset expired or changed to a new value
    kStopped = 3, // Permanent item in use
};

/**
 * @brief Flags for tracking event types to emit.
 */
enum class TimeSyncEventFlag : uint8_t
{
    kNone            = 0,
    kDSTTableEmpty   = 1,
    kDSTStatus       = 2,
    kTimeZoneStatus  = 4,
    kTimeFailure     = 8,
    kMissingTTSource = 16,
};

// Delegate functions
Delegate * GetDelegate();
void SetDefaultDelegate(Delegate * delegate);
Delegate * GetDefaultDelegate();

} // namespace TimeSynchronization

class TimeSynchronizationCluster : public DefaultServerCluster,
                                   public chip::FabricTable::Delegate
#if TIME_SYNC_ENABLE_TSC_FEATURE
    ,
                                   public ReadClient::Callback
#endif
{
public:
    TimeSynchronizationCluster(EndpointId endpoint, const BitFlags<TimeSynchronization::Feature> features,
                               TimeSynchronization::Attributes::SupportsDNSResolve::TypeInfo::Type supportsDNSResolve,
                               TimeSynchronization::TimeZoneDatabaseEnum timeZoneDatabase,
                               TimeSynchronization::TimeSourceEnum timeSource);

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown() override;

    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

    bool HasFeature(TimeSynchronization::Feature feature) const { return mFeatures.Has(feature); }

#if TIME_SYNC_ENABLE_TSC_FEATURE
    // CASE connection functions
    void OnDeviceConnectedFn(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    void OnDeviceConnectionFailureFn();

    // ReadClient::Callback functions
    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override;
    void OnDone(ReadClient * apReadClient) override;
#endif

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR SetTrustedTimeSource(const DataModel::Nullable<TimeSynchronization::Structs::TrustedTimeSourceStruct::Type> & tts);
    CHIP_ERROR SetDefaultNTP(const DataModel::Nullable<chip::CharSpan> & dntp);
    void InitTimeZone();
    CHIP_ERROR SetTimeZone(const DataModel::DecodableList<TimeSynchronization::Structs::TimeZoneStruct::Type> & tzL);
    CHIP_ERROR LoadTimeZone();
    CHIP_ERROR ClearTimeZone();
    void InitDSTOffset();
    CHIP_ERROR SetDSTOffset(const DataModel::DecodableList<TimeSynchronization::Structs::DSTOffsetStruct::Type> & dstL);
    CHIP_ERROR LoadDSTOffset();
    CHIP_ERROR ClearDSTOffset();
    DataModel::Nullable<TimeSynchronization::Structs::TrustedTimeSourceStruct::Type> & GetTrustedTimeSource();
    Span<TimeSyncDataProvider::TimeZoneStore> & GetTimeZone();
    DataModel::List<TimeSynchronization::Structs::DSTOffsetStruct::Type> & GetDSTOffset();
    CHIP_ERROR GetDefaultNtp(MutableCharSpan & dntp);

    CHIP_ERROR SetUTCTime(chip::EndpointId ep, uint64_t utcTime, TimeSynchronization::GranularityEnum granularity,
                          TimeSynchronization::TimeSourceEnum source);
    CHIP_ERROR GetLocalTime(chip::EndpointId ep, DataModel::Nullable<uint64_t> & localTime);
    TimeSynchronization::GranularityEnum & GetGranularity() { return mGranularity; }

    void ScheduleDelayedAction(System::Clock::Seconds32 delay, System::TimerCompleteCallback action, void * aAppState);

    TimeSynchronization::TimeSyncEventFlag GetEventFlag();
    void ClearEventFlag(TimeSynchronization::TimeSyncEventFlag flag);

    CHIP_ERROR AttemptToGetTimeFromTrustedNode();

    void OnPlatformEventFn(const DeviceLayer::ChipDeviceEvent & event);
    void OnTimeSyncCompletionFn(TimeSynchronization::TimeSourceEnum timeSource, TimeSynchronization::GranularityEnum granularity);
    void OnFallbackNTPCompletionFn(bool timeSyncSuccessful);

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

private:
    static constexpr size_t kMaxDefaultNTPSize = 128;
    DataModel::Nullable<TimeSynchronization::Structs::TrustedTimeSourceStruct::Type> mTrustedTimeSource;
    TimeSyncDataProvider::TimeZoneObj mTimeZoneObj{ Span<TimeSyncDataProvider::TimeZoneStore>(mTz), 0 };
    TimeSyncDataProvider::DSTOffsetObj mDstOffsetObj{ DataModel::List<TimeSynchronization::Structs::DSTOffsetStruct::Type>(mDst),
                                                      0 };
    TimeSynchronization::GranularityEnum mGranularity = TimeSynchronization::GranularityEnum::kNoTimeGranularity;

    TimeSyncDataProvider::TimeZoneStore mTz[CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE];
    TimeSynchronization::Structs::DSTOffsetStruct::Type mDst[CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE];

    const BitFlags<TimeSynchronization::Feature> mFeatures;

    TimeSynchronization::Attributes::SupportsDNSResolve::TypeInfo::Type mSupportsDNSResolve;
    TimeSynchronization::TimeZoneDatabaseEnum mTimeZoneDatabase;
    TimeSynchronization::TimeSourceEnum mTimeSource;

    TimeSyncDataProvider mTimeSyncDataProvider;
    TimeSynchronization::TimeSyncEventFlag mEventFlag = TimeSynchronization::TimeSyncEventFlag::kNone;

#if TIME_SYNC_ENABLE_TSC_FEATURE
    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
    struct TimeReadInfo
    {
        TimeReadInfo(InteractionModelEngine * apImEngine, Messaging::ExchangeManager * apExchangeMgr,
                     ReadClient::Callback & apCallback, ReadClient::InteractionType aInteractionType) :
            readClient(apImEngine, apExchangeMgr, apCallback, aInteractionType)
        {
            utcTime.SetNull();
        }
        TimeSynchronization::Attributes::UTCTime::TypeInfo::DecodableType utcTime;
        TimeSynchronization::Attributes::Granularity::TypeInfo::DecodableType granularity =
            TimeSynchronization::GranularityEnum::kNoTimeGranularity;
        ReadClient readClient;
    };
    Platform::UniquePtr<TimeReadInfo> mTimeReadInfo;
#endif

    chip::Callback::Callback<TimeSynchronization::OnTimeSyncCompletion> mOnTimeSyncCompletion;
    chip::Callback::Callback<TimeSynchronization::OnFallbackNTPCompletion> mOnFallbackNTPCompletion;

    // Called when the platform is set up - attempts to get time using the recommended source list in the spec.
    void AttemptToGetTime();
    // Attempts to get fallback NTP from the delegate (last available source)
    // If successful, the function will set mGranulatiry and the time source
    // If unsuccessful, it will emit a TimeFailure event.
    void AttemptToGetFallbackNTPTimeFromDelegate();

    CHIP_ERROR ReadTrustedTimeSource(AttributeValueEncoder & encoder);
    CHIP_ERROR ReadDefaultNtp(AttributeValueEncoder & encoder);
    CHIP_ERROR ReadTimeZone(AttributeValueEncoder & encoder);
    CHIP_ERROR ReadDSTOffset(AttributeValueEncoder & encoder);
    CHIP_ERROR ReadLocalTime(AttributeValueEncoder & encoder);

    std::optional<DataModel::ActionReturnStatus>
    HandleSetUTCTime(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                     const TimeSynchronization::Commands::SetUTCTime::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleSetTrustedTimeSource(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                               const TimeSynchronization::Commands::SetTrustedTimeSource::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleSetTimeZone(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                      const TimeSynchronization::Commands::SetTimeZone::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleSetDSTOffset(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                       const TimeSynchronization::Commands::SetDSTOffset::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleSetDefaultNTP(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                        const TimeSynchronization::Commands::SetDefaultNTP::DecodableType & commandData);

    TimeSyncDataProvider & GetDataProvider() { return mTimeSyncDataProvider; }

    DataModel::EventsGenerator * GetEventsGenerator() const
    {
        return mContext != nullptr ? &mContext->interactionContext.eventsGenerator : nullptr;
    }

    TimeSynchronization::TimeState UpdateTimeZoneState();
    TimeSynchronization::TimeState UpdateDSTOffsetState();
};

} // namespace chip::app::Clusters
