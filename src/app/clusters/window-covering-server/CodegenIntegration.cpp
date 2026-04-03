/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <app/clusters/window-covering-server/CodegenIntegration.h>
#include <app/clusters/window-covering-server/WindowCoveringClusterDelegate.h>
#include <app/clusters/window-covering-server/window-covering-server.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WindowCovering;
using namespace chip::app::Clusters::WindowCovering::Attributes;

namespace {

constexpr size_t kWindowCoveringFixedClusterCount = WindowCovering::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kWindowCoveringMaxClusterCount   = kWindowCoveringFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<WindowCoveringCluster> gServers[kWindowCoveringMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        WindowCoveringCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);
        using namespace chip::Protocols::InteractionModel;
        // attributes which come from ram
        Type type{};
        if (Type::Get(endpointId, type) != Status::Success)
        {
        }

        chip::BitMask<ConfigStatus> configStatus{};
        if (ConfigStatus::Get(endpointId, configStatus) != Success)
        {
        }

        chip::BitMask<OperationalStatus> operationalStatus{};
        if (OperationalStatus::Get(endpointId, operationalStatus) != Success)
        {
        }

        EndProductType endProductType() if (EndProductType::Get(endpointId, endProductType) != Success) {}

        chip::BitMask<Mode> mode{};
        if (Mode::Get(endpointId, operationalStatus) != Success)
        {
        }

        gServers[clusterInstanceIndex].Create(endpointId, featureMap, optionalAttributeSet,
                                              WindowCoveringCluster::StartupConfig{ .type              = type,
                                                                                    .configStatus      = configStatus,
                                                                                    .operationalStatus = operationalStatus,
                                                                                    .endProductType    = endProductType,
                                                                                    .mode              = mode });
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

void MatterWindowCoveringClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = WindowCovering::Id,
            .fixedClusterInstanceCount = kWindowCoveringFixedClusterCount,
            .maxClusterInstanceCount   = kWindowCoveringMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterWindowCoveringClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = WindowCovering::Id,
            .fixedClusterInstanceCount = kWindowCoveringFixedClusterCount,
            .maxClusterInstanceCount   = kWindowCoveringMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::WindowCovering {

WindowCoveringCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * WindowCovering = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = WindowCovering::Id,
            .fixedClusterInstanceCount = kWindowCoveringFixedClusterCount,
            .maxClusterInstanceCount   = kWindowCoveringMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);

    return static_cast<WindowCoveringCluster *>(WindowCovering);
}

void TypeSet(chip::EndpointId endpoint, Type type)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturn(cluster != nullptr);
    cluster->SetType(type);
}

Type TypeGet(chip::EndpointId endpoint)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Type::kUnknown);
    return cluster->GetType();
}

void ConfigStatusSet(chip::EndpointId endpoint, const chip::BitMask<ConfigStatus> & configStatus)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturn(cluster != nullptr);
    cluster->SetConfigStatus(type);
}

chip::BitMask<ConfigStatus> ConfigStatusGet(chip::EndpointId endpoint)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturn(cluster != nullptr);
    return cluster->GetConfigStatus();
}

void ConfigStatusUpdateFeatures(chip::EndpointId endpoint)
{
    chip::BitMask<ConfigStatus> configStatus = ConfigStatusGet(endpoint);

    configStatus.Set(ConfigStatus::kLiftPositionAware, HasFeaturePaLift());
    configStatus.Set(ConfigStatus::kTiltPositionAware, HasFeaturePaTilt());

    if (!HasFeaturePaLift())
        configStatus.Clear(ConfigStatus::kLiftEncoderControlled);

    if (!HasFeaturePaTilt())
        configStatus.Clear(ConfigStatus::kTiltEncoderControlled);

    ConfigStatusSet(endpoint, configStatus);
}

chip::BitMask<OperationalStatus> OperationalStatusGet(chip::EndpointId endpoint)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturn(cluster != nullptr);
    auto & prevStatus = cluster->GetOperationalStatus();
}

void OperationalStatusSet(chip::EndpointId endpoint, chip::BitMask<OperationalStatus> newStatus)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturn(cluster != nullptr);

    // Filter changes
    if (newStatus != prevStatus)
    {
        OperationalStatusPrint(newStatus);
        cluster->SetOperationalStatus(newStatus);
    }
}

void OperationalStateSet(chip::EndpointId endpoint, const chip::BitMask<OperationalStatus> field, OperationalState state)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturn(cluster != nullptr);
    chip::BitMask<OperationalStatus> status = cluster->GetOperationalStatus();

    /* Filter only Lift or Tilt action since we cannot allow global reflecting a state alone */
    if ((OperationalStatus::kLift == field) || (OperationalStatus::kTilt == field))
    {
        status.SetField(field, static_cast<uint8_t>(state));
        status.SetField(OperationalStatus::kGlobal, static_cast<uint8_t>(state));

        /* Global Always follow Lift by priority or therefore fallback to Tilt */
        chip::BitMask<OperationalStatus> opGlobal =
            status.HasAny(OperationalStatus::kLift) ? OperationalStatus::kLift : OperationalStatus::kTilt;
        status.SetField(OperationalStatus::kGlobal, status.GetField(opGlobal));

        OperationalStatusSet(endpoint, status);
    }
}

OperationalState OperationalStateGet(chip::EndpointId endpoint, const chip::BitMask<OperationalStatus> field)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturn(cluster != nullptr);
    chip::BitMask<OperationalStatus> status = cluster->GetOperationalStatus();

    return static_cast<OperationalState>(status.GetField(field));
}

void EndProductTypeSet(chip::EndpointId endpoint, EndProductType type)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturn(cluster != nullptr);
    cluster->SetEndProductType(type);
}

EndProductType EndProductTypeGet(chip::EndpointId endpoint)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturn(cluster != nullptr);
    return cluster->GetEndProductType();
}

void ModeSet(chip::EndpointId endpoint, chip::BitMask<Mode> & newMode)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturn(cluster != nullptr);
    chip::BitMask<ConfigStatus> newStatus;

    chip::BitMask<ConfigStatus> oldStatus = ConfigStatusGet(endpoint);
    chip::BitMask<Mode> oldMode           = ModeGet(endpoint);

    newStatus = oldStatus;

    // Attribute: ConfigStatus reflects the following current mode flags
    newStatus.Set(ConfigStatus::kOperational, !newMode.HasAny(Mode::kMaintenanceMode, Mode::kCalibrationMode));
    newStatus.Set(ConfigStatus::kLiftMovementReversed, newMode.Has(Mode::kMotorDirectionReversed));

    // Verify only one mode supported at once and maintenance lock goes over calibration
    if (newMode.HasAll(Mode::kMaintenanceMode, Mode::kCalibrationMode))
    {
        newMode.Clear(Mode::kCalibrationMode);
    }

    if (oldMode != newMode)
        cluster->SetMode(newMode);

    if (oldStatus != newStatus)
        ConfigStatusSet(endpoint, newStatus);
}

chip::BitMask<Mode> ModeGet(chip::EndpointId endpoint)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturn(cluster != nullptr);
    return cluster->GetMode();
}

void SafetyStatusSet(chip::EndpointId endpoint, chip::BitMask<SafetyStatus> & newSafetyStatus)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturn(cluster != nullptr);
    cluster->SetSafetyStatus(newSafetyStatus);
}

chip::BitMask<SafetyStatus> SafetyStatusGet(chip::EndpointId endpoint)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturn(cluster != nullptr);
    return cluster->GetSafetyStatus();
}

// LiftPositionSet() and
void LiftPositionSet(chip::EndpointId endpoint, NPercent100ths percent100ths)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturn(cluster != nullptr);
    NPercent percent;
    NAbsolute rawpos;

    if (percent100ths.IsNull())
    {
        percent.SetNull();
        rawpos.SetNull();
        ChipLogProgress(Zcl, "Lift[%u] Position Set to Null", endpoint);
    }
    else
    {
        percent.SetNonNull(static_cast<uint8_t>(percent100ths.Value() / 100));
        rawpos.SetNonNull(Percent100thsToLift(endpoint, percent100ths.Value()));
        ChipLogProgress(Zcl, "Lift[%u] Position Set: %u", endpoint, percent100ths.Value());
    }
    Attributes::CurrentPositionLift::Set(endpoint, rawpos);
    cluster->GetCurrentPositionLiftPercentage(percent);
    cluster->SetCurrentPositionLiftPercentage100ths(percent100ths);
}

void TiltPositionSet(chip::EndpointId endpoint, NPercent100ths percent100ths)
{
    NPercent percent;
    NAbsolute rawpos;

    if (percent100ths.IsNull())
    {
        percent.SetNull();
        rawpos.SetNull();
        ChipLogProgress(Zcl, "Tilt[%u] Position Set to Null", endpoint);
    }
    else
    {
        percent.SetNonNull(static_cast<uint8_t>(percent100ths.Value() / 100));
        rawpos.SetNonNull(Percent100thsToTilt(endpoint, percent100ths.Value()));
        ChipLogProgress(Zcl, "Tilt[%u] Position Set: %u", endpoint, percent100ths.Value());
    }
    Attributes::CurrentPositionTilt::Set(endpoint, rawpos);
    cluster->GetCurrentPositionTiltPercentage(percent);
    cluster->SetCurrentPositionTiltPercentage100ths(percent100ths);
}

} // namespace chip::app::Clusters::WindowCovering
