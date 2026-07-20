/*
 *
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/color-control-server/CodegenIntegration.h>
#include <app/clusters/color-control-server/ColorControlCluster.h>
#include <app/clusters/color-control-server/ColorControlDelegate.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <lib/support/BitFlags.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ColorControl;
using namespace chip::app::Clusters::ColorControl::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace {

// Fixed-endpoint count comes straight from the ZAP server endpoint count. We deliberately do NOT use
// ColorControl::StaticApplicationConfig::kFixedClusterConfig here: this integration reads the feature map
// from ember at runtime (fetchFeatureMap = true) and never consumes the static config's feature list, and
// that list currently fails to compile for acronym features (Feature::kXY vs the enum's kXy — a ZAP vs
// py_matter_idl codegen naming mismatch tracked separately).
constexpr size_t kColorControlFixedClusterCount = MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT;
constexpr size_t kColorControlMaxClusterCount   = kColorControlFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kColorControlMaxClusterCount <= kEmberInvalidEndpointIndex, "ColorControl cluster table size error");

// A single shared no-op delegate for endpoints that have no application delegate registered.
//
// ColorControlCluster requires a non-null ColorControlDelegate&, and every base-class method is a no-op: the
// color-science Convert* are never reached on a single-feature device (it cannot switch into a mode it does
// not advertise), and hardware On*Changed are simply not delivered. So one stateless instance safely backs
// every delegate-less endpoint. Applications that need conversions or hardware output register their own
// delegate via SetDefaultDelegate BEFORE the cluster is constructed; the binding is fixed at construction and
// is not repointable afterwards (no consumer needs that).
ColorControlDelegate gDefaultColorControlDelegate;

struct ClusterWithDelegate
{
    ColorControlDelegate * userDelegate = nullptr;
    LazyRegisteredServerCluster<ColorControlCluster> server;
};

ClusterWithDelegate gClusters[kColorControlMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        // Bind the application delegate registered for this endpoint, or the shared no-op default if none.
        ColorControlDelegate & delegate = (gClusters[clusterInstanceIndex].userDelegate != nullptr)
            ? *gClusters[clusterInstanceIndex].userDelegate
            : gDefaultColorControlDelegate;

        ColorControlCluster::Config config(delegate);
        config.mFeatures.SetRaw(featureMap);

        // Color-temperature physical limits + couple-to-level minimum are Fixed (F) attributes whose ZAP
        // defaults live in ember storage; seed the cluster's runtime CTConfig from them when CT is advertised.
        // StartUpColorTemperatureMireds and the live color/color-loop values are restored by the cluster's
        // Startup() through the persistence provider, so they are not fetched here.
        if (config.mFeatures.Has(Feature::kColorTemperature))
        {
            uint16_t value = 0;
            // Code-driven cluster: ember does not manage these attributes at runtime, so the generated
            // Accessors expose only GetDefault() (the ZAP-configured fixed default), not Get(). Seed the
            // cluster's CTConfig from those defaults; a missing/disabled attribute leaves the CTConfig default.
            if (ColorTempPhysicalMinMireds::GetDefault(endpointId, &value) == Status::Success)
            {
                config.ctConfig.colorTempPhysicalMinMireds = value;
            }
            if (ColorTempPhysicalMaxMireds::GetDefault(endpointId, &value) == Status::Success)
            {
                config.ctConfig.colorTempPhysicalMaxMireds = value;
            }
            if (CoupleColorTempToLevelMinMireds::GetDefault(endpointId, &value) == Status::Success)
            {
                config.ctConfig.coupleColorTempToLevelMinMireds = value;
            }
        }

        gClusters[clusterInstanceIndex].server.Create(endpointId, config);
        return gClusters[clusterInstanceIndex].server.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gClusters[clusterInstanceIndex].server.IsConstructed(), nullptr);
        return &gClusters[clusterInstanceIndex].server.Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gClusters[clusterInstanceIndex].server.Destroy(); }
};

} // namespace

// Plugin-level server init: the generated ember glue (emberAfInit) calls this once per cluster type.
// ColorControl is code-driven, so there is nothing to do here — per-endpoint construction happens in
// MatterColorControlClusterInitCallback below. Defined here (not in app/util/util.cpp) to keep all of
// ColorControl's callback symbols in the cluster's own codegen integration.
void MatterColorControlPluginServerInitCallback() {}

void MatterColorControlClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ColorControl::Id,
            .fixedClusterInstanceCount = kColorControlFixedClusterCount,
            .maxClusterInstanceCount   = kColorControlMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterColorControlClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ColorControl::Id,
            .fixedClusterInstanceCount = kColorControlFixedClusterCount,
            .maxClusterInstanceCount   = kColorControlMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::ColorControl {

ColorControlCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = ColorControl::Id,
            .fixedClusterInstanceCount = kColorControlFixedClusterCount,
            .maxClusterInstanceCount   = kColorControlMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<ColorControlCluster *>(cluster);
}

ColorControlDelegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, ColorControl::Id, MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kColorControlMaxClusterCount ? nullptr : gClusters[ep].userDelegate);
}

void SetDefaultDelegate(EndpointId endpoint, ColorControlDelegate * delegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, ColorControl::Id, MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kColorControlMaxClusterCount)
    {
        gClusters[ep].userDelegate = delegate;
    }
}

} // namespace chip::app::Clusters::ColorControl

// ─────────────────────────────────────────────────────────────────────────────────────────────────
// Legacy ColorControlServer command facade — forwards every legacy command entry point to the
// code-driven ColorControlCluster registered on the endpoint. See CodegenIntegration.h for rationale.
// ─────────────────────────────────────────────────────────────────────────────────────────────────

using chip::app::Clusters::ColorControl::FindClusterOnEndpoint;

ColorControlServer & ColorControlServer::Instance()
{
    static ColorControlServer instance;
    return instance;
}

bool ColorControlServer::HasFeature(EndpointId endpoint, Feature feature)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    return cluster != nullptr && cluster->HasFeature(feature);
}

ColorControlServer::Status ColorControlServer::stopAllColorTransitions(EndpointId endpoint)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->stopMoveStep();
}

ColorControlServer::Status
ColorControlServer::stopMoveStepCommand(EndpointId endpoint,
                                        const ColorControl::Commands::StopMoveStep::DecodableType & commandData)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->stopMoveStep(commandData.optionsMask, commandData.optionsOverride);
}

ColorControlServer::Status ColorControlServer::moveHueCommand(EndpointId endpoint, MoveModeEnum moveMode, uint16_t rate,
                                                              chip::BitMask<OptionsBitmap> optionsMask,
                                                              chip::BitMask<OptionsBitmap> optionsOverride, bool isEnhanced)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->moveHue(moveMode, rate, isEnhanced, optionsMask, optionsOverride);
}

ColorControlServer::Status ColorControlServer::moveToHueCommand(EndpointId endpoint, uint16_t hue, DirectionEnum moveDirection,
                                                                uint16_t transitionTime, chip::BitMask<OptionsBitmap> optionsMask,
                                                                chip::BitMask<OptionsBitmap> optionsOverride, bool isEnhanced)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->moveToHue(hue, moveDirection, transitionTime, isEnhanced, optionsMask, optionsOverride);
}

ColorControlServer::Status ColorControlServer::moveToHueAndSaturationCommand(EndpointId endpoint, uint16_t hue, uint8_t saturation,
                                                                             uint16_t transitionTime,
                                                                             chip::BitMask<OptionsBitmap> optionsMask,
                                                                             chip::BitMask<OptionsBitmap> optionsOverride,
                                                                             bool isEnhanced)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->moveToHueAndSaturation(hue, saturation, transitionTime, isEnhanced, optionsMask, optionsOverride);
}

ColorControlServer::Status ColorControlServer::stepHueCommand(EndpointId endpoint, StepModeEnum stepMode, uint16_t stepSize,
                                                              uint16_t transitionTime, chip::BitMask<OptionsBitmap> optionsMask,
                                                              chip::BitMask<OptionsBitmap> optionsOverride, bool isEnhanced)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->stepHue(stepMode, stepSize, transitionTime, isEnhanced, optionsMask, optionsOverride);
}

ColorControlServer::Status
ColorControlServer::moveSaturationCommand(EndpointId endpoint,
                                          const ColorControl::Commands::MoveSaturation::DecodableType & commandData)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->moveSaturation(commandData.moveMode, commandData.rate, commandData.optionsMask, commandData.optionsOverride);
}

ColorControlServer::Status
ColorControlServer::moveToSaturationCommand(EndpointId endpoint,
                                            const ColorControl::Commands::MoveToSaturation::DecodableType & commandData)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->moveToSaturation(commandData.saturation, commandData.transitionTime, commandData.optionsMask,
                                     commandData.optionsOverride);
}

ColorControlServer::Status
ColorControlServer::stepSaturationCommand(EndpointId endpoint,
                                          const ColorControl::Commands::StepSaturation::DecodableType & commandData)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->stepSaturation(commandData.stepMode, commandData.stepSize, commandData.transitionTime, commandData.optionsMask,
                                   commandData.optionsOverride);
}

ColorControlServer::Status
ColorControlServer::colorLoopCommand(EndpointId endpoint, const ColorControl::Commands::ColorLoopSet::DecodableType & commandData)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->ColorLoopSet(commandData.updateFlags, commandData.action, commandData.direction, commandData.time,
                                 commandData.startHue, commandData.optionsMask, commandData.optionsOverride);
}

ColorControlServer::Status
ColorControlServer::moveToColorCommand(EndpointId endpoint, const ColorControl::Commands::MoveToColor::DecodableType & commandData)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->moveToColor(commandData.colorX, commandData.colorY, commandData.transitionTime, commandData.optionsMask,
                                commandData.optionsOverride);
}

ColorControlServer::Status
ColorControlServer::moveColorCommand(EndpointId endpoint, const ColorControl::Commands::MoveColor::DecodableType & commandData)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->moveColor(commandData.rateX, commandData.rateY, commandData.optionsMask, commandData.optionsOverride);
}

ColorControlServer::Status
ColorControlServer::stepColorCommand(EndpointId endpoint, const ColorControl::Commands::StepColor::DecodableType & commandData)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->stepColor(commandData.stepX, commandData.stepY, commandData.transitionTime, commandData.optionsMask,
                              commandData.optionsOverride);
}

ColorControlServer::Status
ColorControlServer::moveColorTempCommand(EndpointId endpoint,
                                         const ColorControl::Commands::MoveColorTemperature::DecodableType & commandData)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->moveColorTemp(commandData.moveMode, commandData.rate, commandData.colorTemperatureMinimumMireds,
                                  commandData.colorTemperatureMaximumMireds, commandData.optionsMask, commandData.optionsOverride);
}

ColorControlServer::Status
ColorControlServer::moveToColorTempCommand(EndpointId endpoint,
                                           const ColorControl::Commands::MoveToColorTemperature::DecodableType & commandData)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->moveToColorTemp(commandData.colorTemperatureMireds, commandData.transitionTime, commandData.optionsMask,
                                    commandData.optionsOverride);
}

ColorControlServer::Status
ColorControlServer::stepColorTempCommand(EndpointId endpoint,
                                         const ColorControl::Commands::StepColorTemperature::DecodableType & commandData)
{
    auto * cluster = FindClusterOnEndpoint(endpoint);
    VerifyOrReturnValue(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->stepColorTemp(commandData.stepMode, commandData.stepSize, commandData.transitionTime,
                                  commandData.colorTemperatureMinimumMireds, commandData.colorTemperatureMaximumMireds,
                                  commandData.optionsMask, commandData.optionsOverride);
}
