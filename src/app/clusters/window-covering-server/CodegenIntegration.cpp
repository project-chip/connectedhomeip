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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app/clusters/window-covering-server/CodegenIntegration.h>
#include <app/clusters/window-covering-server/WindowCoveringCluster.h>
#include <app/clusters/window-covering-server/WindowCoveringDelegate.h>
#include <app/data-model-provider/AttributeChangeListener.h>
#include <app/static-cluster-config/WindowCovering.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WindowCovering;
using namespace chip::app::Clusters::WindowCovering::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr size_t kWindowCoveringFixedClusterCount = WindowCovering::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kWindowCoveringMaxClusterCount   = kWindowCoveringFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

// Helper subclass where Setters for fixed attributes Type and EndProductType are enabled
// This is done only for keeping the backwards compatibility with example apps
class CodegenWindowCoveringCluster : public WindowCoveringCluster
{
public:
    using WindowCoveringCluster::SetEndProductType;
    using WindowCoveringCluster::SetType;
    using WindowCoveringCluster::WindowCoveringCluster;
};

LazyRegisteredServerCluster<CodegenWindowCoveringCluster> gServers[kWindowCoveringMaxClusterCount];

class WindowCoveringIntegrationDelegateWrapper final : public WindowCoveringDelegate
{
public:
    void Init(EndpointId ep, WindowCoveringDelegate * wrapped)
    {
        mEndpoint = ep;
        mWrapped  = wrapped;
    }

    CHIP_ERROR HandleMovement(WindowCoveringType type) override
    {
        if (mWrapped)
        {
            return mWrapped->HandleMovement(type);
        }
        ChipLogProgress(Zcl, "WindowCovering has no delegate set for endpoint:%u", mEndpoint);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR HandleStopMotion() override
    {
        if (mWrapped)
        {
            return mWrapped->HandleStopMotion();
        }
        ChipLogProgress(Zcl, "WindowCovering has no delegate set for endpoint:%u", mEndpoint);
        return CHIP_NO_ERROR;
    }

private:
    WindowCoveringDelegate * mWrapped = nullptr;
};

WindowCoveringIntegrationDelegateWrapper gDelegateWrappers[kWindowCoveringMaxClusterCount];

// Helper for the derived class
CodegenWindowCoveringCluster * FindCodegenCluster(chip::EndpointId endpoint)
{
    WindowCoveringCluster * baseCluster = WindowCovering::FindClusterOnEndpoint(endpoint);

    return static_cast<CodegenWindowCoveringCluster *>(baseCluster);
}

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        BitFlags<WindowCovering::Feature> features;
        features.SetRaw(featureMap);

        // Build OptionalAttributeSet: only set feature-dependent attributes when the feature is present.
        WindowCovering::OptionalAttributeSet optionalAttributes;
        app::AttributeSet emberOptionals(optionalAttributeBits);

        if (features.Has(Feature::kLift) && emberOptionals.IsSet(Attributes::NumberOfActuationsLift::Id))
        {
            optionalAttributes.Set<Attributes::NumberOfActuationsLift::Id>();
        }
        if (features.Has(Feature::kTilt) && emberOptionals.IsSet(Attributes::NumberOfActuationsTilt::Id))
        {
            optionalAttributes.Set<Attributes::NumberOfActuationsTilt::Id>();
        }
        if (features.HasAll(Feature::kLift, Feature::kPositionAwareLift) &&
            emberOptionals.IsSet(Attributes::CurrentPositionLiftPercentage::Id))
        {
            optionalAttributes.Set<Attributes::CurrentPositionLiftPercentage::Id>();
        }
        if (features.HasAll(Feature::kTilt, Feature::kPositionAwareTilt) &&
            emberOptionals.IsSet(Attributes::CurrentPositionTiltPercentage::Id))
        {
            optionalAttributes.Set<Attributes::CurrentPositionTiltPercentage::Id>();
        }
        if (emberOptionals.IsSet(Attributes::SafetyStatus::Id))
        {
            optionalAttributes.Set<Attributes::SafetyStatus::Id>();
        }

        gDelegateWrappers[clusterInstanceIndex].SetEndpoint(endpointId);
        WindowCoveringCluster::Config config(gDelegateWrappers[clusterInstanceIndex]);
        config.WithFeatures(features).WithOptionalAttributes(optionalAttributes);

        // RAM attributes whose default is defined per-endpoint in the application Matter files
        // are passed to the constructor via Config (per the Default Value Rule). If a default is
        // not present in ZAP, the Config's own default (spec conformance value) is used.
        WindowCovering::Type type{};
        if (Attributes::Type::GetDefault(endpointId, &type) == Status::Success)
        {
            config.WithType(type);
        }

        WindowCovering::EndProductType endProductType{};
        if (Attributes::EndProductType::GetDefault(endpointId, &endProductType) == Status::Success)
        {
            config.WithEndProductType(endProductType);
        }

        gServers[clusterInstanceIndex].Create(endpointId, config);

        auto & cluster = gServers[clusterInstanceIndex].Cluster();

        chip::BitMask<WindowCovering::ConfigStatus> configStatus;
        if (Attributes::ConfigStatus::GetDefault(endpointId, &configStatus) == Status::Success)
        {
            cluster.SetConfigStatus(configStatus);
        }

        chip::BitMask<WindowCovering::Mode> mode;
        if (Attributes::Mode::GetDefault(endpointId, &mode) == Status::Success)
        {
            cluster.SetMode(mode);
        }

        // Load feature-gated position attributes from ZAP defaults.
        if (features.HasAll(Feature::kLift, Feature::kPositionAwareLift))
        {
            DataModel::Nullable<Percent100ths> percent100ths;
            if (Attributes::CurrentPositionLiftPercent100ths::GetDefault(endpointId, percent100ths) == Status::Success)
            {
                cluster.SetCurrentPositionLiftPercent100ths(percent100ths);
            }
            if (Attributes::TargetPositionLiftPercent100ths::GetDefault(endpointId, percent100ths) == Status::Success)
            {
                cluster.SetTargetPositionLiftPercent100ths(percent100ths);
            }
        }

        if (features.HasAll(Feature::kTilt, Feature::kPositionAwareTilt))
        {
            DataModel::Nullable<Percent100ths> percent100ths;
            if (Attributes::CurrentPositionTiltPercent100ths::GetDefault(endpointId, percent100ths) == Status::Success)
            {
                cluster.SetCurrentPositionTiltPercent100ths(percent100ths);
            }
            if (Attributes::TargetPositionTiltPercent100ths::GetDefault(endpointId, percent100ths) == Status::Success)
            {
                cluster.SetTargetPositionTiltPercent100ths(percent100ths);
            }
        }

        return gServers[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gServers[clusterInstanceIndex].Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServers[clusterInstanceIndex].Destroy(); }
};

class WindowCoveringAttributeChangeListener final : public chip::app::DataModel::AttributeChangeListener
{
public:
    void OnAttributeChanged(const chip::app::ConcreteAttributePath & path, chip::app::DataModel::AttributeChangeType type) override
    {
        if (path.mClusterId == chip::app::Clusters::WindowCovering::Id)
        {
            MatterWindowCoveringClusterServerAttributeChangedCallback(path);
        }
    }
};

WindowCoveringAttributeChangeListener gAttributeChangeListener;
uint16_t gActiveWindowCoveringEndpoints = 0;

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

    if (gActiveWindowCoveringEndpoints == 0)
    {
        CodegenDataModelProvider::Instance().RegisterAttributeChangeListener(gAttributeChangeListener);
    }
    gActiveWindowCoveringEndpoints++;
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
        },
        integrationDelegate, shutdownType);

    if (gActiveWindowCoveringEndpoints > 0)
    {
        gActiveWindowCoveringEndpoints--;
        if (gActiveWindowCoveringEndpoints == 0)
        {
            CodegenDataModelProvider::Instance().UnregisterAttributeChangeListener(gAttributeChangeListener);
        }
    }
}

namespace chip::app::Clusters::WindowCovering {

WindowCoveringCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = WindowCovering::Id,
            .fixedClusterInstanceCount = kWindowCoveringFixedClusterCount,
            .maxClusterInstanceCount   = kWindowCoveringMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<WindowCoveringCluster *>(cluster);
}

void SetDefaultDelegate(EndpointId endpointId, WindowCoveringDelegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpointId, WindowCovering::Id,
                                                       MATTER_DM_WINDOW_COVERING_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kWindowCoveringMaxClusterCount)
    {
        gDelegateWrappers[ep].Init(endpointId, delegate);
    }
    else
    {
        ChipLogProgress(Zcl, "Failed to set WindowCovering delegate for endpoint:%u", endpointId);
    }
}

void TypeSet(chip::EndpointId endpoint, Type type)
{
    auto cluster = FindCodegenCluster(endpoint);
    VerifyOrDie(cluster != nullptr);
    cluster->SetType(type);
}

Type TypeGet(chip::EndpointId endpoint)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrDie(cluster != nullptr);
    return cluster->GetType();
}

void ConfigStatusPrint(const chip::BitMask<ConfigStatus> & configStatus)
{
    ChipLogProgress(Zcl, "ConfigStatus 0x%02X Operational=%u OnlineReserved=%u", configStatus.Raw(),
                    configStatus.Has(ConfigStatus::kOperational), configStatus.Has(ConfigStatus::kOnlineReserved));

    ChipLogProgress(Zcl, "Lift(PA=%u Encoder=%u Reversed=%u) Tilt(PA=%u Encoder=%u)",
                    configStatus.Has(ConfigStatus::kLiftPositionAware), configStatus.Has(ConfigStatus::kLiftEncoderControlled),
                    configStatus.Has(ConfigStatus::kLiftMovementReversed), configStatus.Has(ConfigStatus::kTiltPositionAware),
                    configStatus.Has(ConfigStatus::kTiltEncoderControlled));
}

void ConfigStatusSet(chip::EndpointId endpoint, const chip::BitMask<ConfigStatus> & configStatus)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrDie(cluster != nullptr);
    cluster->SetConfigStatus(configStatus);
}

chip::BitMask<ConfigStatus> ConfigStatusGet(chip::EndpointId endpoint)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrDie(cluster != nullptr);
    return cluster->GetConfigStatus();
}

void ConfigStatusUpdateFeatures(chip::EndpointId endpoint)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrDie(cluster != nullptr);
    chip::BitMask<ConfigStatus> configStatus = ConfigStatusGet(endpoint);

    configStatus.Set(ConfigStatus::kLiftPositionAware, cluster->GetFeatureMap().Has(Feature::kPositionAwareLift));
    configStatus.Set(ConfigStatus::kTiltPositionAware, cluster->GetFeatureMap().Has(Feature::kPositionAwareTilt));

    if (!cluster->GetFeatureMap().Has(Feature::kPositionAwareLift))
    {
        configStatus.Clear(ConfigStatus::kLiftEncoderControlled);
    }

    if (!cluster->GetFeatureMap().Has(Feature::kPositionAwareTilt))
    {
        configStatus.Clear(ConfigStatus::kTiltEncoderControlled);
    }

    ConfigStatusSet(endpoint, configStatus);
}

void OperationalStatusPrint(const chip::BitMask<OperationalStatus> & opStatus)
{
    ChipLogProgress(Zcl, "OperationalStatus raw=0x%02X global=%u lift=%u tilt=%u", opStatus.Raw(),
                    opStatus.GetField(OperationalStatus::kGlobal), opStatus.GetField(OperationalStatus::kLift),
                    opStatus.GetField(OperationalStatus::kTilt));
}

chip::BitMask<OperationalStatus> OperationalStatusGet(chip::EndpointId endpoint)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrDie(cluster != nullptr);
    return cluster->GetOperationalStatus();
}

void OperationalStatusSet(chip::EndpointId endpoint, chip::BitMask<OperationalStatus> newStatus)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrDie(cluster != nullptr);
    chip::BitMask<OperationalStatus> prevStatus = cluster->GetOperationalStatus();

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
    VerifyOrDie(cluster != nullptr);
    chip::BitMask<OperationalStatus> status = cluster->GetOperationalStatus();

    /* Filter only Lift or Tilt action since we cannot allow global reflecting a state alone */
    if ((OperationalStatus::kLift == field) || (OperationalStatus::kTilt == field))
    {
        status.SetField(field, static_cast<uint8_t>(state));

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
    VerifyOrDie(cluster != nullptr);
    chip::BitMask<OperationalStatus> status = cluster->GetOperationalStatus();

    return static_cast<OperationalState>(status.GetField(field));
}

void EndProductTypeSet(chip::EndpointId endpoint, EndProductType type)
{
    auto cluster = FindCodegenCluster(endpoint);
    VerifyOrDie(cluster != nullptr);
    cluster->SetEndProductType(type);
}

EndProductType EndProductTypeGet(chip::EndpointId endpoint)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrDie(cluster != nullptr);
    return cluster->GetEndProductType();
}

void ModeSet(chip::EndpointId endpoint, chip::BitMask<Mode> & newMode)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrDie(cluster != nullptr);
    cluster->SetMode(newMode);
}

chip::BitMask<Mode> ModeGet(chip::EndpointId endpoint)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrDie(cluster != nullptr);
    return cluster->GetMode();
}

void ModePrint(const chip::BitMask<Mode> & mode)
{
    ChipLogProgress(Zcl, "Mode 0x%02X MotorDirReversed=%u LedFeedback=%u Maintenance=%u Calibration=%u", mode.Raw(),
                    mode.Has(Mode::kMotorDirectionReversed), mode.Has(Mode::kLedFeedback), mode.Has(Mode::kMaintenanceMode),
                    mode.Has(Mode::kCalibrationMode));
}

void LiftPositionSet(chip::EndpointId endpoint, NPercent100ths percent100ths)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrDie(cluster != nullptr);

    if (percent100ths.IsNull())
    {
        ChipLogProgress(Zcl, "Lift[%u] Position Set to Null", endpoint);
    }
    else
    {
        ChipLogProgress(Zcl, "Lift[%u] Position Set: %u", endpoint, percent100ths.Value());
    }
    cluster->SetCurrentPositionLiftPercent100ths(percent100ths);
}

void TiltPositionSet(chip::EndpointId endpoint, NPercent100ths percent100ths)
{
    auto cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrDie(cluster != nullptr);

    if (percent100ths.IsNull())
    {
        ChipLogProgress(Zcl, "Tilt[%u] Position Set to Null", endpoint);
    }
    else
    {
        ChipLogProgress(Zcl, "Tilt[%u] Position Set: %u", endpoint, percent100ths.Value());
    }
    cluster->SetCurrentPositionTiltPercent100ths(percent100ths);
}

OperationalState ComputeOperationalState(uint16_t target, uint16_t current)
{
    OperationalState opState = OperationalState::Stall;

    if (current != target)
    {
        opState = (current < target) ? OperationalState::MovingDownOrClose : OperationalState::MovingUpOrOpen;
    }
    return opState;
}

OperationalState ComputeOperationalState(NPercent100ths target, NPercent100ths current)
{
    if (!current.IsNull() && !target.IsNull())
    {
        return ComputeOperationalState(target.Value(), current.Value());
    }
    return OperationalState::Stall;
}

LimitStatus CheckLimitState(uint16_t position, AbsoluteLimits limits)
{
    if (limits.open > limits.closed)
    {
        return LimitStatus::Inverted;
    }

    if (position == limits.open)
    {
        return LimitStatus::IsUpOrOpen;
    }

    if (position == limits.closed)
    {
        return LimitStatus::IsDownOrClose;
    }

    if ((limits.open > 0) && (position < limits.open))
    {
        return LimitStatus::IsPastUpOrOpen;
    }

    if ((limits.closed > 0) && (position > limits.closed))
    {
        return LimitStatus::IsPastDownOrClose;
    }

    return LimitStatus::Intermediate;
}

/*
 * ConvertValue: Converts values from one range to another
 * Range In  -> from  inputLowValue to   inputHighValue
 * Range Out -> from outputLowValue to outputHighValue
 */
uint16_t ConvertValue(uint16_t inputLowValue, uint16_t inputHighValue, uint16_t outputLowValue, uint16_t outputHighValue,
                      uint16_t value)
{
    uint16_t inputMin = inputLowValue, inputMax = inputHighValue, inputRange = UINT16_MAX;
    uint16_t outputMin = outputLowValue, outputMax = outputHighValue, outputRange = UINT16_MAX;

    if (inputLowValue > inputHighValue)
    {
        inputMin = inputHighValue;
        inputMax = inputLowValue;
    }

    if (outputLowValue > outputHighValue)
    {
        outputMin = outputHighValue;
        outputMax = outputLowValue;
    }

    inputRange  = static_cast<uint16_t>(inputMax - inputMin);
    outputRange = static_cast<uint16_t>(outputMax - outputMin);

    if (value < inputMin)
    {
        return outputMin;
    }

    if (value > inputMax)
    {
        return outputMax;
    }

    if (inputRange > 0)
    {
        return static_cast<uint16_t>(outputMin + ((outputRange * (value - inputMin) / inputRange)));
    }

    return outputMax;
}

uint16_t Percent100thsToValue(AbsoluteLimits limits, Percent100ths relative)
{
    return ConvertValue(kWcPercent100thsMinOpen, kWcPercent100thsMaxClosed, limits.open, limits.closed, relative);
}

Percent100ths ComputePercent100thsStep(OperationalState direction, Percent100ths previous, Percent100ths delta)
{
    Percent100ths percent100ths = previous;

    switch (direction)
    {
    case OperationalState::MovingDownOrClose:
        if (percent100ths < (kWcPercent100thsMaxClosed - delta))
        {
            percent100ths = static_cast<Percent100ths>(percent100ths + delta);
        }
        else
        {
            percent100ths = kWcPercent100thsMaxClosed;
        }
        break;
    case OperationalState::MovingUpOrOpen:
        if (percent100ths > (kWcPercent100thsMinOpen + delta))
        {
            percent100ths = static_cast<Percent100ths>(percent100ths - delta);
        }
        else
        {
            percent100ths = kWcPercent100thsMinOpen;
        }
        break;
    default:
        // nothing to do we keep previous value, simple passthrough
        break;
    }

    if (percent100ths > kWcPercent100thsMaxClosed)
    {
        return kWcPercent100thsMaxClosed;
    }

    return percent100ths;
}

void PostAttributeChange(chip::EndpointId endpoint, chip::AttributeId attributeId)
{
    BitMask<Mode> mode;
    BitMask<ConfigStatus> configStatus;

    ChipLogProgress(Zcl, "WC POST ATTRIBUTE=%u", (unsigned int) attributeId);

    switch (attributeId)
    {
    case Attributes::Mode::Id:
        mode = ModeGet(endpoint);
        ModePrint(mode);
        ModeSet(endpoint, mode);
        break;
    case Attributes::ConfigStatus::Id:
        configStatus = ConfigStatusGet(endpoint);
        ConfigStatusPrint(configStatus);
        break;
    default:
        break;
    }
}

} // namespace chip::app::Clusters::WindowCovering

/**
 * @brief Cluster Plugin Init Callback
 */
void MatterWindowCoveringPluginServerInitCallback() {}
void MatterWindowCoveringPluginServerShutdownCallback() {}

void __attribute__((weak))
MatterWindowCoveringClusterServerAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath)
{
    chip::app::Clusters::WindowCovering::PostAttributeChange(attributePath.mEndpointId, attributePath.mAttributeId);
}
