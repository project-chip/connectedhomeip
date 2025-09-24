/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/time-synchronization-server/CodegenIntegration.h>
#include <app/clusters/time-synchronization-server/DefaultTimeSyncDelegate.h>
#include <app/static-cluster-config/TimeSynchronization.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TimeSynchronization;
using namespace chip::app::Clusters::TimeSynchronization::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace {

constexpr size_t kTimeSynchronizationFixedClusterCount = TimeSynchronization::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kTimeSynchronizationMaxClusterCount =
    kTimeSynchronizationFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<TimeSynchronizationCluster> gServers[kTimeSynchronizationMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t rawFeatureMap) override
    {
        const BitFlags<TimeSynchronization::Feature> featureMap(rawFeatureMap);
        gServers[clusterInstanceIndex].Create(endpointId, featureMap);
        return gServers[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gServers[clusterInstanceIndex].Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServers[clusterInstanceIndex].Destroy(); }
};

} // namespace

void emberAfTimeSynchronizationClusterServerInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    // register a singleton server (root endpoint only)
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = TimeSynchronization::Id,
            .fixedClusterInstanceCount = kTimeSynchronizationFixedClusterCount,
            .maxClusterInstanceCount   = kTimeSynchronizationMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterTimeSynchronizationClusterServerShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    // register a singleton server (root endpoint only)
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = TimeSynchronization::Id,
            .fixedClusterInstanceCount = kTimeSynchronizationFixedClusterCount,
            .maxClusterInstanceCount   = kTimeSynchronizationMaxClusterCount,
        },
        integrationDelegate);
}

void MatterTimeSynchronizationPluginServerInitCallback() {}

void MatterTimeSynchronizationPluginServerShutdownCallback() {}

namespace chip::app::Clusters::TimeSynchronization {

TimeSynchronizationCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * timeSynchronization = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = TimeSynchronization::Id,
            .fixedClusterInstanceCount = kTimeSynchronizationFixedClusterCount,
            .maxClusterInstanceCount   = kTimeSynchronizationMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<TimeSynchronizationCluster *>(timeSynchronization);
}

} // namespace chip::app::Clusters::TimeSynchronization

bool emberAfTimeSynchronizationClusterSetUTCTimeCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::TimeSynchronization::Commands::SetUTCTime::DecodableType & commandData)
{
    const auto & utcTime     = commandData.UTCTime;
    const auto & granularity = commandData.granularity;
    const auto & timeSource  = commandData.timeSource;

    auto timeSynchronization = FindClusterOnEndpoint(commandPath.mEndpointId);
    VerifyOrReturnValue(timeSynchronization != nullptr, false);

    auto currentGranularity = timeSynchronization->GetGranularity();
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
        CHIP_NO_ERROR == timeSynchronization->SetUTCTime(commandPath.mEndpointId, utcTime, granularity, TimeSourceEnum::kAdmin))
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

    auto timeSynchronization = FindClusterOnEndpoint(commandPath.mEndpointId);
    VerifyOrReturnValue(timeSynchronization != nullptr, false);

    if (!timeSource.IsNull())
    {
        Structs::TrustedTimeSourceStruct::Type ts = { commandObj->GetAccessingFabricIndex(), timeSource.Value().nodeID,
                                                      timeSource.Value().endpoint };
        tts.SetNonNull(ts);
        // TODO: schedule a utctime read from this time source and emit event only on failure to get time
        emitTimeFailureEvent(commandPath.mEndpointId, timeSynchronization->GetEventsGenerator());
    }
    else
    {
        tts.SetNull();
        emitMissingTrustedTimeSourceEvent(commandPath.mEndpointId, timeSynchronization->GetEventsGenerator());
    }

    timeSynchronization->SetTrustedTimeSource(tts);
    commandObj->AddStatus(commandPath, Status::Success);
    return true;
}

bool emberAfTimeSynchronizationClusterSetTimeZoneCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::TimeSynchronization::Commands::SetTimeZone::DecodableType & commandData)
{
    const auto & timeZone = commandData.timeZone;

    auto timeSynchronization = FindClusterOnEndpoint(commandPath.mEndpointId);
    VerifyOrReturnValue(timeSynchronization != nullptr, false);

    CHIP_ERROR err = timeSynchronization->SetTimeZone(timeZone);
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

    if (to_underlying(timeSynchronization->GetEventFlag()) & to_underlying(TimeSyncEventFlag::kTimeZoneStatus))
    {
        timeSynchronization->ClearEventFlag(TimeSyncEventFlag::kTimeZoneStatus);
        emitTimeZoneStatusEvent(commandPath.mEndpointId, timeSynchronization->GetEventsGenerator());
    }
    GetDelegate()->TimeZoneListChanged(timeSynchronization->GetTimeZone());

    TimeZoneDatabaseEnum tzDb;
    TimeZoneDatabase::Get(commandPath.mEndpointId, &tzDb);
    Commands::SetTimeZoneResponse::Type response;
    timeSynchronization->UpdateTimeZoneState();
    const auto & tzList = timeSynchronization->GetTimeZone();
    if (timeSynchronization->HasFeature(TimeSynchronization::Feature::kTimeZone) && tzDb != TimeZoneDatabaseEnum::kNone &&
        tzList.size() != 0)
    {
        auto & tz = tzList[0].timeZone;
        if (tz.name.HasValue() && GetDelegate()->HandleUpdateDSTOffset(tz.name.Value()))
        {
            response.DSTOffsetRequired = false;
            emitDSTStatusEvent(commandPath.mEndpointId, true, timeSynchronization->GetEventsGenerator());
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
        TimeState dstState = timeSynchronization->UpdateDSTOffsetState();
        timeSynchronization->ClearDSTOffset();
        if (dstState == TimeState::kActive || dstState == TimeState::kChanged)
        {
            emitDSTStatusEvent(commandPath.mEndpointId, false, timeSynchronization->GetEventsGenerator());
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

    auto timeSynchronization = FindClusterOnEndpoint(commandPath.mEndpointId);
    VerifyOrReturnValue(timeSynchronization != nullptr, false);

    TimeState dstState = timeSynchronization->UpdateDSTOffsetState();

    CHIP_ERROR err = timeSynchronization->SetDSTOffset(dstOffset);
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
    if (dstState != timeSynchronization->UpdateDSTOffsetState())
    {
        emitDSTStatusEvent(commandPath.mEndpointId, TimeState::kActive == timeSynchronization->UpdateDSTOffsetState(),
                           timeSynchronization->GetEventsGenerator());
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

    auto timeSynchronization = FindClusterOnEndpoint(commandPath.mEndpointId);
    VerifyOrReturnValue(timeSynchronization != nullptr, false);

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

    status = (CHIP_NO_ERROR == timeSynchronization->SetDefaultNTP(dNtpChar)) ? Status::Success : Status::Failure;

    commandObj->AddStatus(commandPath, status);
    return true;
}
