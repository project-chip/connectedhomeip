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

#include <app/clusters/time-synchronization-server/DefaultTimeSyncDelegate.h>
#include <app/clusters/time-synchronization-server/TimeSyncDataProvider.h>
#include <app/clusters/time-synchronization-server/time-synchronization-delegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/TimeSynchronization/Metadata.h>
#include <platform/DeviceInfoProvider.h>

#include <app/server/Server.h>

// NOTE: this is part of AppConfig, so this has to be checked for AFTER the inclusion
//       of that header
#if TIME_SYNC_ENABLE_TSC_FEATURE
#include <app/ReadClient.h>
#endif

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

} // namespace TimeSynchronization

class TimeSynchronizationCluster : public DefaultServerCluster,
                                   public FabricTable::Delegate
#if TIME_SYNC_ENABLE_TSC_FEATURE
    ,
                                   public ReadClient::Callback
#endif
{
public:
    // NOTE: this set is smaller than the full optional attributes supported by time synchronization
    //       as other attributes are controlled by feature flags
    using OptionalAttributeSet = app::OptionalAttributeSet<TimeSynchronization::Attributes::TimeSource::Id>;

    TimeSynchronizationCluster(EndpointId endpoint, const OptionalAttributeSet & optionalAttributeSet,
                               const BitFlags<TimeSynchronization::Feature> features,
                               TimeSynchronization::Attributes::SupportsDNSResolve::TypeInfo::Type supportsDNSResolve,
                               TimeSynchronization::TimeZoneDatabaseEnum timeZoneDatabase,
                               TimeSynchronization::TimeSourceEnum timeSource,
                               TimeSynchronization::Attributes::NTPServerAvailable::TypeInfo::Type ntpServerAvailable);

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown() override;

    // Fabric Table delegate functions
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

#if TIME_SYNC_ENABLE_TSC_FEATURE
    // CASE connection functions
    void OnDeviceConnectedFn(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    void OnDeviceConnectionFailureFn();

    // ReadClient::Callback functions
    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override;
    void OnDone(ReadClient * apReadClient) override;
#endif

    // Platform event handler functions
    void OnPlatformEventFn(const DeviceLayer::ChipDeviceEvent & event);
    void OnTimeSyncCompletionFn(TimeSynchronization::TimeSourceEnum timeSource, TimeSynchronization::GranularityEnum granularity);
    void OnFallbackNTPCompletionFn(bool timeSyncSuccessful);

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    const TimeSynchronization::GranularityEnum & GetGranularity() const { return mGranularity; }

    TimeSynchronization::Delegate * GetDelegate();
    void SetDefaultDelegate(TimeSynchronization::Delegate * delegate) { mDelegate = delegate; }
    TimeSynchronization::Delegate * GetDefaultDelegate() { return GetDelegate(); }

private:
    CHIP_ERROR SetTrustedTimeSource(const DataModel::Nullable<TimeSynchronization::Structs::TrustedTimeSourceStruct::Type> & tts);
    CHIP_ERROR SetDefaultNTP(const DataModel::Nullable<CharSpan> & dntp);
    void InitTimeZone();
    CHIP_ERROR SetTimeZone(const DataModel::DecodableList<TimeSynchronization::Structs::TimeZoneStruct::Type> & tzL);
    CHIP_ERROR LoadTimeZone();
    CHIP_ERROR ClearTimeZone();
    void InitDSTOffset();
    CHIP_ERROR SetDSTOffset(const DataModel::DecodableList<TimeSynchronization::Structs::DSTOffsetStruct::Type> & dstL);
    CHIP_ERROR LoadDSTOffset();
    CHIP_ERROR ClearDSTOffset();

    const DataModel::Nullable<TimeSynchronization::Structs::TrustedTimeSourceStruct::Type> & GetTrustedTimeSource() const;
    Span<TimeSyncDataProvider::TimeZoneStore> & GetTimeZone();
    DataModel::List<TimeSynchronization::Structs::DSTOffsetStruct::Type> & GetDSTOffset();
    CHIP_ERROR GetDefaultNtp(MutableCharSpan & dntp);
    CHIP_ERROR SetUTCTime(EndpointId ep, uint64_t utcTime, TimeSynchronization::GranularityEnum granularity,
                          TimeSynchronization::TimeSourceEnum source);
    CHIP_ERROR GetLocalTime(EndpointId ep, DataModel::Nullable<uint64_t> & localTime);

    TimeSyncDataProvider & GetDataProvider() { return mTimeSyncDataProvider; }

    TimeSynchronization::TimeState UpdateTimeZoneState();
    TimeSynchronization::TimeState UpdateDSTOffsetState();
    TimeSynchronization::TimeSyncEventFlag GetEventFlag() const;
    void ClearEventFlag(TimeSynchronization::TimeSyncEventFlag flag);

    static constexpr size_t kMaxDefaultNTPSize = 128;
    DataModel::Nullable<TimeSynchronization::Structs::TrustedTimeSourceStruct::Type> mTrustedTimeSource;
    TimeSyncDataProvider::TimeZoneObj mTimeZoneObj{ Span<TimeSyncDataProvider::TimeZoneStore>(mTz), 0 };
    TimeSyncDataProvider::DSTOffsetObj mDstOffsetObj{ DataModel::List<TimeSynchronization::Structs::DSTOffsetStruct::Type>(mDst),
                                                      0 };
    TimeSynchronization::GranularityEnum mGranularity = TimeSynchronization::GranularityEnum::kNoTimeGranularity;

    TimeSyncDataProvider::TimeZoneStore mTz[CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE];
    TimeSynchronization::Structs::DSTOffsetStruct::Type mDst[CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE];

    const OptionalAttributeSet mOptionalAttributeSet;
    const BitFlags<TimeSynchronization::Feature> mFeatures;

    TimeSynchronization::Attributes::SupportsDNSResolve::TypeInfo::Type mSupportsDNSResolve;
    TimeSynchronization::TimeZoneDatabaseEnum mTimeZoneDatabase;
    TimeSynchronization::TimeSourceEnum mTimeSource;
    TimeSynchronization::Attributes::NTPServerAvailable::TypeInfo::Type mNTPServerAvailable;
    TimeSynchronization::Delegate * mDelegate = nullptr;

    TimeSyncDataProvider mTimeSyncDataProvider;
    TimeSynchronization::TimeSyncEventFlag mEventFlag = TimeSynchronization::TimeSyncEventFlag::kNone;

    CHIP_ERROR AttemptToGetTimeFromTrustedNode();

#if TIME_SYNC_ENABLE_TSC_FEATURE
    chip::Callback::Callback<OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
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

    Protocols::InteractionModel::Status
    HandleSetUTCTime(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                     const TimeSynchronization::Commands::SetUTCTime::DecodableType & commandData);

    Protocols::InteractionModel::Status
    HandleSetTrustedTimeSource(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                               const TimeSynchronization::Commands::SetTrustedTimeSource::DecodableType & commandData);

    Protocols::InteractionModel::Status
    HandleSetTimeZone(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                      const TimeSynchronization::Commands::SetTimeZone::DecodableType & commandData);

    Protocols::InteractionModel::Status
    HandleSetDSTOffset(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                       const TimeSynchronization::Commands::SetDSTOffset::DecodableType & commandData);

    Protocols::InteractionModel::Status
    HandleSetDefaultNTP(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                        const TimeSynchronization::Commands::SetDefaultNTP::DecodableType & commandData);

    DataModel::EventsGenerator * GetEventsGenerator() const
    {
        return mContext != nullptr ? &mContext->interactionContext.eventsGenerator : nullptr;
    }
};

} // namespace chip::app::Clusters
