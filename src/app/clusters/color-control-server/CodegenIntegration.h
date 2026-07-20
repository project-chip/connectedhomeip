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

#pragma once

#include <app/clusters/color-control-server/ColorControlCluster.h>
#include <app/clusters/color-control-server/ColorControlDelegate.h>
#include <lib/core/DataModelTypes.h>

namespace chip::app::Clusters::ColorControl {

/**
 * Returns the code-driven Color Control server cluster instance registered for `endpointId` when using
 * the codegen data model, or `nullptr` if the endpoint has no Color Control server or the cluster is not
 * registered. This is the replacement for the legacy `ColorControlServer::Instance()` singleton: instead of
 * a node-global object keyed by endpoint internally, callers resolve the per-endpoint cluster instance here
 * and invoke its command handlers directly (e.g. `FindClusterOnEndpoint(ep)->moveToColorTemp(...)`).
 */
ColorControlCluster * FindClusterOnEndpoint(EndpointId endpointId);

/**
 * Registers the application `ColorControlDelegate` for `endpoint`, used by the codegen integration layer.
 *
 * The integration keeps one delegate pointer per Color Control server endpoint. The code-driven cluster is
 * always wired to an internal wrapper (see CodegenIntegration.cpp) that holds this pointer and forwards the
 * color-science `Convert*` conversions and the hardware `On*Changed` callbacks; when no application delegate
 * is registered the `Convert*` calls fall back to the base no-ops (correct for single-feature devices, which
 * never switch into a mode they do not advertise) and the `On*Changed` notifications are dropped.
 *
 * Call this from application init before or after the cluster is constructed.
 *
 * @param endpoint  Color Control server endpoint.
 * @param delegate  Application delegate, or `nullptr` to unbind.
 */
void SetDefaultDelegate(EndpointId endpoint, ColorControlDelegate * delegate);

/** Returns the delegate registered with `SetDefaultDelegate` for `endpoint`, or `nullptr` if none/invalid. */
ColorControlDelegate * GetDelegate(EndpointId endpoint);

} // namespace chip::app::Clusters::ColorControl

/**
 * Backward-compatibility facade for the legacy `ColorControlServer::Instance().<command>(endpoint, ...)`
 * command API (global namespace, matching the pre-migration `color-control-server.h`).
 *
 * This is the "prove backward compat" bridge: every method resolves the code-driven ColorControlCluster
 * registered on `endpoint` (via ColorControl::FindClusterOnEndpoint) and forwards to that cluster's public
 * command handler, so application code that drove color through this singleton keeps compiling and working
 * without change. There is NO transition state or color math here anymore — it all lives in the cluster;
 * this class only unpacks the legacy argument shapes and dispatches. When no Color Control server cluster is
 * registered on the endpoint, the forwarders return Status::UnsupportedEndpoint.
 */
class ColorControlServer
{
public:
    using StepModeEnum    = chip::app::Clusters::ColorControl::StepModeEnum;
    using MoveModeEnum    = chip::app::Clusters::ColorControl::MoveModeEnum;
    using DirectionEnum   = chip::app::Clusters::ColorControl::DirectionEnum;
    using Feature         = chip::app::Clusters::ColorControl::Feature;
    using OptionsBitmap   = chip::app::Clusters::ColorControl::OptionsBitmap;
    using Status          = chip::Protocols::InteractionModel::Status;

    static ColorControlServer & Instance();

    bool HasFeature(chip::EndpointId endpoint, Feature feature);

    Status stopAllColorTransitions(chip::EndpointId endpoint);
    Status stopMoveStepCommand(chip::EndpointId endpoint,
                               const chip::app::Clusters::ColorControl::Commands::StopMoveStep::DecodableType & commandData);

    Status moveHueCommand(chip::EndpointId endpoint, MoveModeEnum moveMode, uint16_t rate,
                          chip::BitMask<OptionsBitmap> optionsMask, chip::BitMask<OptionsBitmap> optionsOverride, bool isEnhanced);
    Status moveToHueCommand(chip::EndpointId endpoint, uint16_t hue, DirectionEnum moveDirection, uint16_t transitionTime,
                            chip::BitMask<OptionsBitmap> optionsMask, chip::BitMask<OptionsBitmap> optionsOverride, bool isEnhanced);
    Status moveToHueAndSaturationCommand(chip::EndpointId endpoint, uint16_t hue, uint8_t saturation, uint16_t transitionTime,
                                         chip::BitMask<OptionsBitmap> optionsMask, chip::BitMask<OptionsBitmap> optionsOverride,
                                         bool isEnhanced);
    Status stepHueCommand(chip::EndpointId endpoint, StepModeEnum stepMode, uint16_t stepSize, uint16_t transitionTime,
                          chip::BitMask<OptionsBitmap> optionsMask, chip::BitMask<OptionsBitmap> optionsOverride, bool isEnhanced);

    Status moveSaturationCommand(chip::EndpointId endpoint,
                                 const chip::app::Clusters::ColorControl::Commands::MoveSaturation::DecodableType & commandData);
    Status moveToSaturationCommand(chip::EndpointId endpoint,
                                   const chip::app::Clusters::ColorControl::Commands::MoveToSaturation::DecodableType & commandData);
    Status stepSaturationCommand(chip::EndpointId endpoint,
                                 const chip::app::Clusters::ColorControl::Commands::StepSaturation::DecodableType & commandData);
    Status colorLoopCommand(chip::EndpointId endpoint,
                            const chip::app::Clusters::ColorControl::Commands::ColorLoopSet::DecodableType & commandData);

    Status moveToColorCommand(chip::EndpointId endpoint,
                              const chip::app::Clusters::ColorControl::Commands::MoveToColor::DecodableType & commandData);
    Status moveColorCommand(chip::EndpointId endpoint,
                            const chip::app::Clusters::ColorControl::Commands::MoveColor::DecodableType & commandData);
    Status stepColorCommand(chip::EndpointId endpoint,
                            const chip::app::Clusters::ColorControl::Commands::StepColor::DecodableType & commandData);

    Status moveColorTempCommand(chip::EndpointId endpoint,
                                const chip::app::Clusters::ColorControl::Commands::MoveColorTemperature::DecodableType & commandData);
    Status
    moveToColorTempCommand(chip::EndpointId endpoint,
                           const chip::app::Clusters::ColorControl::Commands::MoveToColorTemperature::DecodableType & commandData);
    Status stepColorTempCommand(chip::EndpointId endpoint,
                                const chip::app::Clusters::ColorControl::Commands::StepColorTemperature::DecodableType & commandData);

private:
    ColorControlServer() = default;
};
