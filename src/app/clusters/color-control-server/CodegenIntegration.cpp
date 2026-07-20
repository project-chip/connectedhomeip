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

// Integration-owned delegate used only by the codegen integration layer.
//
// ColorControlCluster is constructed with a mandatory ColorControlDelegate& (no nullptr). Ember/ZAP apps
// register a real application delegate later (or never) via SetDefaultDelegate, so the cluster cannot assume
// an application delegate exists at construction time.
//
// This wrapper is the single object passed into ColorControlCluster::Config for the endpoint slot: it always
// lives for the slot and holds an optional pointer to the application delegate. Color-science Convert* calls
// forward when that pointer is non-null; otherwise they fall through to the base no-ops. A single-feature
// device never switches into a mode it does not advertise, so those no-ops are never reached there; a
// multi-feature device MUST register a delegate that implements the relevant conversions. Hardware On*Changed
// notifications forward when a delegate is present and are dropped otherwise.
class ColorControlIntegrationDelegateWrapper final : public ColorControlDelegate
{
public:
    void Init(EndpointId ep, ColorControlDelegate * wrapped)
    {
        mEndpoint = ep;
        mWrapped  = wrapped;
    }

    // ---- Color-science conversion (§3.2.8.2) ----
    void ConvertXYToHueSat(EndpointId ep, uint16_t x, uint16_t y, uint8_t & outHue, uint8_t & outSat) override
    {
        if (mWrapped)
            mWrapped->ConvertXYToHueSat(ep, x, y, outHue, outSat);
    }
    void ConvertHueSatToXY(EndpointId ep, uint8_t hue, uint8_t sat, uint16_t & outX, uint16_t & outY) override
    {
        if (mWrapped)
            mWrapped->ConvertHueSatToXY(ep, hue, sat, outX, outY);
    }
    void ConvertXYToMireds(EndpointId ep, uint16_t x, uint16_t y, uint16_t & outMireds) override
    {
        if (mWrapped)
            mWrapped->ConvertXYToMireds(ep, x, y, outMireds);
    }
    void ConvertMiredsToXY(EndpointId ep, uint16_t mireds, uint16_t & outX, uint16_t & outY) override
    {
        if (mWrapped)
            mWrapped->ConvertMiredsToXY(ep, mireds, outX, outY);
    }
    void ConvertHueSatToMireds(EndpointId ep, uint8_t hue, uint8_t sat, uint16_t & outMireds) override
    {
        if (mWrapped)
            mWrapped->ConvertHueSatToMireds(ep, hue, sat, outMireds);
    }
    void ConvertMiredsToHueSat(EndpointId ep, uint16_t mireds, uint8_t & outHue, uint8_t & outSat) override
    {
        if (mWrapped)
            mWrapped->ConvertMiredsToHueSat(ep, mireds, outHue, outSat);
    }

    // ---- Hardware output ----
    void OnColorXYChanged(EndpointId ep, uint16_t x, uint16_t y) override
    {
        if (mWrapped)
            mWrapped->OnColorXYChanged(ep, x, y);
    }
    void OnColorHSChanged(EndpointId ep, uint8_t hue, uint8_t sat) override
    {
        if (mWrapped)
            mWrapped->OnColorHSChanged(ep, hue, sat);
    }
    void OnColorCTChanged(EndpointId ep, uint16_t mireds) override
    {
        if (mWrapped)
            mWrapped->OnColorCTChanged(ep, mireds);
    }
    void OnEnhancedHueChanged(EndpointId ep, uint16_t enhancedHue) override
    {
        if (mWrapped)
            mWrapped->OnEnhancedHueChanged(ep, enhancedHue);
    }

private:
    EndpointId mEndpoint             = kInvalidEndpointId;
    ColorControlDelegate * mWrapped  = nullptr;
};

struct ClusterWithDelegate
{
    ColorControlDelegate * userDelegate = nullptr;
    ColorControlIntegrationDelegateWrapper integrationDelegateWrapper;
    LazyRegisteredServerCluster<ColorControlCluster> server;
};

ClusterWithDelegate gClusters[kColorControlMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        gClusters[clusterInstanceIndex].integrationDelegateWrapper.Init(endpointId,
                                                                        gClusters[clusterInstanceIndex].userDelegate);

        ColorControlCluster::Config config(gClusters[clusterInstanceIndex].integrationDelegateWrapper);
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
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, ColorControl::Id,
                                                       MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kColorControlMaxClusterCount ? nullptr : gClusters[ep].userDelegate);
}

void SetDefaultDelegate(EndpointId endpoint, ColorControlDelegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, ColorControl::Id,
                                                       MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kColorControlMaxClusterCount)
    {
        gClusters[ep].userDelegate = delegate;
        gClusters[ep].integrationDelegateWrapper.Init(endpoint, delegate);
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
    return cluster->stepSaturation(commandData.stepMode, commandData.stepSize, commandData.transitionTime,
                                   commandData.optionsMask, commandData.optionsOverride);
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
                                  commandData.colorTemperatureMaximumMireds, commandData.optionsMask,
                                  commandData.optionsOverride);
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
