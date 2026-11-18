/*
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <app/clusters/pump-configuration-and-control-server/CodegenIntegration.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/static-cluster-config/PumpConfigurationAndControl.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PumpConfigurationAndControl;
using namespace chip::app::Clusters::PumpConfigurationAndControl::Attributes;

namespace {

using OptionalAttributes = PumpConfigurationAndControlCluster::OptionalAttributes;

constexpr size_t kPumpConfigurationAndControlFixedClusterCount =
    PumpConfigurationAndControl::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kPumpConfigurationAndControlMaxClusterCount =
    kPumpConfigurationAndControlFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

/// Delegate of each cluster created from the ZAP configuration.
///
/// It forwards the remote sensor query and the attribute change notifications to the
/// ApplicationDelegate, if one is installed. While OperationMode is Minimum or Maximum it sets the
/// Level Control CurrentLevel on the same endpoint to MinLevel (at least 1) or MaxLevel.
class CodegenDelegate : public PumpConfigurationAndControl::Delegate
{
public:
    void SetApplicationDelegate(ApplicationDelegate * delegate) { mApplicationDelegate = delegate; }

    RemoteSensorType GetRemoteSensorType(EndpointId endpointId) override
    {
        VerifyOrReturnValue(mApplicationDelegate != nullptr, RemoteSensorType::kNoSensor);
        return mApplicationDelegate->GetRemoteSensorType(endpointId);
    }

    void OnOperationModeMinMax(EndpointId endpointId, OperationModeEnum operationMode) override
    {
#ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL
        uint8_t level = 0;
        if (operationMode == OperationModeEnum::kMaximum)
        {
            VerifyOrReturn(LevelControl::Attributes::MaxLevel::Get(endpointId, &level) ==
                           Protocols::InteractionModel::Status::Success);
        }
        else
        {
            VerifyOrReturn(LevelControl::Attributes::MinLevel::Get(endpointId, &level) ==
                           Protocols::InteractionModel::Status::Success);
            // A level of 0 stops the pump.
            if (level == 0)
            {
                level = 1;
            }
        }
        LevelControl::Attributes::CurrentLevel::Set(endpointId, level);
#endif // MATTER_DM_PLUGIN_LEVEL_CONTROL
    }

    void OnAttributeChanged(EndpointId endpointId, AttributeId attributeId) override
    {
        VerifyOrReturn(mApplicationDelegate != nullptr);
        mApplicationDelegate->OnAttributeChanged(endpointId, attributeId);
    }

private:
    ApplicationDelegate * mApplicationDelegate = nullptr;
};

LazyRegisteredServerCluster<PumpConfigurationAndControlCluster> gServers[kPumpConfigurationAndControlMaxClusterCount];
CodegenDelegate gDelegates[kPumpConfigurationAndControlMaxClusterCount];

PumpConfigurationAndControlCluster::FixedAttributes ReadFixedAttributeDefaults(EndpointId endpointId)
{
    PumpConfigurationAndControlCluster::FixedAttributes fixed;
    MaxPressure::GetDefaultOr(endpointId, fixed.maxPressure, DataModel::NullNullable);
    MaxSpeed::GetDefaultOr(endpointId, fixed.maxSpeed, DataModel::NullNullable);
    MaxFlow::GetDefaultOr(endpointId, fixed.maxFlow, DataModel::NullNullable);
    MinConstPressure::GetDefaultOr(endpointId, fixed.minConstPressure, DataModel::NullNullable);
    MaxConstPressure::GetDefaultOr(endpointId, fixed.maxConstPressure, DataModel::NullNullable);
    MinCompPressure::GetDefaultOr(endpointId, fixed.minCompPressure, DataModel::NullNullable);
    MaxCompPressure::GetDefaultOr(endpointId, fixed.maxCompPressure, DataModel::NullNullable);
    MinConstSpeed::GetDefaultOr(endpointId, fixed.minConstSpeed, DataModel::NullNullable);
    MaxConstSpeed::GetDefaultOr(endpointId, fixed.maxConstSpeed, DataModel::NullNullable);
    MinConstFlow::GetDefaultOr(endpointId, fixed.minConstFlow, DataModel::NullNullable);
    MaxConstFlow::GetDefaultOr(endpointId, fixed.maxConstFlow, DataModel::NullNullable);
    MinConstTemp::GetDefaultOr(endpointId, fixed.minConstTemp, DataModel::NullNullable);
    MaxConstTemp::GetDefaultOr(endpointId, fixed.maxConstTemp, DataModel::NullNullable);
    return fixed;
}

BitMask<OptionalAttributes> ReadOptionalAttributes(EndpointId endpointId, uint32_t optionalAttributeBits)
{
    const AttributeSet enabled(optionalAttributeBits);
    BitMask<OptionalAttributes> attributes;
    attributes.Set(OptionalAttributes::kPumpStatus, enabled.IsSet(PumpStatus::Id));
    attributes.Set(OptionalAttributes::kSpeed, enabled.IsSet(Speed::Id));
    attributes.Set(OptionalAttributes::kLifetimeRunningHours, enabled.IsSet(LifetimeRunningHours::Id));
    attributes.Set(OptionalAttributes::kPower, enabled.IsSet(Power::Id));
    attributes.Set(OptionalAttributes::kLifetimeEnergyConsumed, enabled.IsSet(LifetimeEnergyConsumed::Id));
    attributes.Set(OptionalAttributes::kMinConstPressure, enabled.IsSet(MinConstPressure::Id));
    attributes.Set(OptionalAttributes::kMaxConstPressure, enabled.IsSet(MaxConstPressure::Id));
    attributes.Set(OptionalAttributes::kMinCompPressure, enabled.IsSet(MinCompPressure::Id));
    attributes.Set(OptionalAttributes::kMaxCompPressure, enabled.IsSet(MaxCompPressure::Id));
    attributes.Set(OptionalAttributes::kMinConstSpeed, enabled.IsSet(MinConstSpeed::Id));
    attributes.Set(OptionalAttributes::kMaxConstSpeed, enabled.IsSet(MaxConstSpeed::Id));
    attributes.Set(OptionalAttributes::kMinConstFlow, enabled.IsSet(MinConstFlow::Id));
    attributes.Set(OptionalAttributes::kMaxConstFlow, enabled.IsSet(MaxConstFlow::Id));
    attributes.Set(OptionalAttributes::kMinConstTemp, enabled.IsSet(MinConstTemp::Id));
    attributes.Set(OptionalAttributes::kMaxConstTemp, enabled.IsSet(MaxConstTemp::Id));
    // ControlMode (0x21) is above the 31 attribute ids covered by optionalAttributeBits.
    attributes.Set(OptionalAttributes::kControlMode,
                   emberAfContainsAttribute(endpointId, PumpConfigurationAndControl::Id, ControlMode::Id));
    return attributes;
}

std::optional<ControlModeEnum> ReadControlModeDefault(EndpointId endpointId, bool controlModeEnabled)
{
    ControlModeEnum controlMode;
    // Without a ControlMode attribute, the configured EffectiveControlMode default is the control
    // mode of the pump.
    const Protocols::InteractionModel::Status status = controlModeEnabled
        ? ControlMode::GetDefault(endpointId, controlMode)
        : EffectiveControlMode::GetDefault(endpointId, controlMode);
    VerifyOrReturnValue(status == Protocols::InteractionModel::Status::Success, std::nullopt);
    return controlMode;
}

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        PumpConfigurationAndControlCluster::Config config(gDelegates[clusterInstanceIndex]);
        config.features           = BitFlags<Feature>(featureMap);
        config.optionalAttributes = ReadOptionalAttributes(endpointId, optionalAttributeBits);
        config.fixedAttributes    = ReadFixedAttributeDefaults(endpointId);
        config.controlMode = ReadControlModeDefault(endpointId, config.optionalAttributes.Has(OptionalAttributes::kControlMode));
        OperationMode::GetDefaultOr(endpointId, config.operationMode, OperationModeEnum::kNormal);

        gServers[clusterInstanceIndex].Create(endpointId, config);
        return gServers[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gServers[clusterInstanceIndex].Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override
    {
        gServers[clusterInstanceIndex].Destroy();
        gDelegates[clusterInstanceIndex].SetApplicationDelegate(nullptr);
    }
};

} // namespace

void MatterPumpConfigurationAndControlClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = PumpConfigurationAndControl::Id,
            .fixedClusterInstanceCount = kPumpConfigurationAndControlFixedClusterCount,
            .maxClusterInstanceCount   = kPumpConfigurationAndControlMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterPumpConfigurationAndControlClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = PumpConfigurationAndControl::Id,
            .fixedClusterInstanceCount = kPumpConfigurationAndControlFixedClusterCount,
            .maxClusterInstanceCount   = kPumpConfigurationAndControlMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

void MatterPumpConfigurationAndControlPluginServerInitCallback() {}
void MatterPumpConfigurationAndControlPluginServerShutdownCallback() {}

namespace chip::app::Clusters::PumpConfigurationAndControl {

PumpConfigurationAndControlCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = PumpConfigurationAndControl::Id,
            .fixedClusterInstanceCount = kPumpConfigurationAndControlFixedClusterCount,
            .maxClusterInstanceCount   = kPumpConfigurationAndControlMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<PumpConfigurationAndControlCluster *>(cluster);
}

CHIP_ERROR SetApplicationDelegate(EndpointId endpointId, ApplicationDelegate * delegate)
{
    const uint16_t index = emberAfGetClusterServerEndpointIndex(endpointId, PumpConfigurationAndControl::Id,
                                                                kPumpConfigurationAndControlFixedClusterCount);
    VerifyOrReturnError(index < kPumpConfigurationAndControlMaxClusterCount, CHIP_ERROR_NOT_FOUND);

    gDelegates[index].SetApplicationDelegate(delegate);
    if (PumpConfigurationAndControlCluster * cluster = FindClusterOnEndpoint(endpointId); cluster != nullptr)
    {
        cluster->OnRemoteSensorChanged();
    }
    return CHIP_NO_ERROR;
}

namespace {

using Protocols::InteractionModel::Status;

template <typename T, typename Getter>
Status GetFromCluster(EndpointId endpoint, AttributeId attributeId, T & value, Getter && getter)
{
    PumpConfigurationAndControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    VerifyOrReturnValue(cluster->IsAttributeEnabled(attributeId), Status::UnsupportedAttribute);
    value = getter(*cluster);
    return Status::Success;
}

template <typename T, typename Getter>
Status GetFromCluster(EndpointId endpoint, AttributeId attributeId, T * value, Getter && getter)
{
    VerifyOrReturnValue(value != nullptr, Status::Failure);
    return GetFromCluster(endpoint, attributeId, *value, getter);
}

template <typename Setter>
Status SetOnCluster(EndpointId endpoint, Setter && setter)
{
    PumpConfigurationAndControlCluster * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return Protocols::InteractionModel::ClusterStatusCode(setter(*cluster)).GetStatus();
}

} // namespace

namespace Attributes {

namespace MaxPressure {
Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetFixedAttributes().maxPressure; });
}
} // namespace MaxPressure

namespace MaxSpeed {
Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetFixedAttributes().maxSpeed; });
}
} // namespace MaxSpeed

namespace MaxFlow {
Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetFixedAttributes().maxFlow; });
}
} // namespace MaxFlow

namespace MinConstPressure {
Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetFixedAttributes().minConstPressure; });
}
} // namespace MinConstPressure

namespace MaxConstPressure {
Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetFixedAttributes().maxConstPressure; });
}
} // namespace MaxConstPressure

namespace MinCompPressure {
Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetFixedAttributes().minCompPressure; });
}
} // namespace MinCompPressure

namespace MaxCompPressure {
Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetFixedAttributes().maxCompPressure; });
}
} // namespace MaxCompPressure

namespace MinConstSpeed {
Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetFixedAttributes().minConstSpeed; });
}
} // namespace MinConstSpeed

namespace MaxConstSpeed {
Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetFixedAttributes().maxConstSpeed; });
}
} // namespace MaxConstSpeed

namespace MinConstFlow {
Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetFixedAttributes().minConstFlow; });
}
} // namespace MinConstFlow

namespace MaxConstFlow {
Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetFixedAttributes().maxConstFlow; });
}
} // namespace MaxConstFlow

namespace MinConstTemp {
Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetFixedAttributes().minConstTemp; });
}
} // namespace MinConstTemp

namespace MaxConstTemp {
Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetFixedAttributes().maxConstTemp; });
}
} // namespace MaxConstTemp

namespace Capacity {
Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetCapacity(); });
}
Status Set(EndpointId endpoint, int16_t value)
{
    return Set(endpoint, DataModel::MakeNullable(value));
}
Status SetNull(EndpointId endpoint)
{
    return Set(endpoint, DataModel::Nullable<int16_t>());
}
Status Set(EndpointId endpoint, const DataModel::Nullable<int16_t> & value)
{
    return SetOnCluster(endpoint, [&value](auto & cluster) { return cluster.SetCapacity(value); });
}
} // namespace Capacity

namespace Speed {
Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetSpeed(); });
}
Status Set(EndpointId endpoint, uint16_t value)
{
    return Set(endpoint, DataModel::MakeNullable(value));
}
Status SetNull(EndpointId endpoint)
{
    return Set(endpoint, DataModel::Nullable<uint16_t>());
}
Status Set(EndpointId endpoint, const DataModel::Nullable<uint16_t> & value)
{
    return SetOnCluster(endpoint, [&value](auto & cluster) { return cluster.SetSpeed(value); });
}
} // namespace Speed

namespace LifetimeRunningHours {
Status Get(EndpointId endpoint, DataModel::Nullable<uint32_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetLifetimeRunningHours(); });
}
Status Set(EndpointId endpoint, uint32_t value)
{
    return Set(endpoint, DataModel::MakeNullable(value));
}
Status SetNull(EndpointId endpoint)
{
    return Set(endpoint, DataModel::Nullable<uint32_t>());
}
Status Set(EndpointId endpoint, const DataModel::Nullable<uint32_t> & value)
{
    return SetOnCluster(endpoint, [&value](auto & cluster) { return cluster.SetLifetimeRunningHours(value); });
}
} // namespace LifetimeRunningHours

namespace Power {
Status Get(EndpointId endpoint, DataModel::Nullable<uint32_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetPower(); });
}
Status Set(EndpointId endpoint, uint32_t value)
{
    return Set(endpoint, DataModel::MakeNullable(value));
}
Status SetNull(EndpointId endpoint)
{
    return Set(endpoint, DataModel::Nullable<uint32_t>());
}
Status Set(EndpointId endpoint, const DataModel::Nullable<uint32_t> & value)
{
    return SetOnCluster(endpoint, [&value](auto & cluster) { return cluster.SetPower(value); });
}
} // namespace Power

namespace LifetimeEnergyConsumed {
Status Get(EndpointId endpoint, DataModel::Nullable<uint32_t> & value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetLifetimeEnergyConsumed(); });
}
Status Set(EndpointId endpoint, uint32_t value)
{
    return Set(endpoint, DataModel::MakeNullable(value));
}
Status SetNull(EndpointId endpoint)
{
    return Set(endpoint, DataModel::Nullable<uint32_t>());
}
Status Set(EndpointId endpoint, const DataModel::Nullable<uint32_t> & value)
{
    return SetOnCluster(endpoint, [&value](auto & cluster) { return cluster.SetLifetimeEnergyConsumed(value); });
}
} // namespace LifetimeEnergyConsumed

namespace PumpStatus {
Status Get(EndpointId endpoint, BitMask<PumpStatusBitmap> * value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetPumpStatus(); });
}
Status Set(EndpointId endpoint, BitMask<PumpStatusBitmap> value)
{
    return SetOnCluster(endpoint, [value](auto & cluster) { return cluster.SetPumpStatus(value); });
}
} // namespace PumpStatus

namespace EffectiveOperationMode {
Status Get(EndpointId endpoint, OperationModeEnum * value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetEffectiveOperationMode(); });
}
} // namespace EffectiveOperationMode

namespace EffectiveControlMode {
Status Get(EndpointId endpoint, ControlModeEnum * value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetEffectiveControlMode(); });
}
Status Set(EndpointId endpoint, ControlModeEnum value)
{
    return SetOnCluster(endpoint, [value](auto & cluster) { return cluster.SetEffectiveControlMode(value); });
}
} // namespace EffectiveControlMode

namespace OperationMode {
Status Get(EndpointId endpoint, OperationModeEnum * value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetOperationMode(); });
}
Status Set(EndpointId endpoint, OperationModeEnum value)
{
    return SetOnCluster(endpoint, [value](auto & cluster) { return cluster.SetOperationMode(value); });
}
} // namespace OperationMode

namespace ControlMode {
Status Get(EndpointId endpoint, ControlModeEnum * value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetControlMode(); });
}
Status Set(EndpointId endpoint, ControlModeEnum value)
{
    return SetOnCluster(endpoint, [value](auto & cluster) { return cluster.SetControlMode(value); });
}
} // namespace ControlMode

namespace FeatureMap {
Status Get(EndpointId endpoint, uint32_t * value)
{
    return GetFromCluster(endpoint, Id, value, [](auto & cluster) { return cluster.GetFeatures().Raw(); });
}
} // namespace FeatureMap

} // namespace Attributes

} // namespace chip::app::Clusters::PumpConfigurationAndControl
