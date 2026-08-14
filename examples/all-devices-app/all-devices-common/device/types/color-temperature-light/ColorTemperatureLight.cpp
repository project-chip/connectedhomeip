/*
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

#include <device/types/color-temperature-light/ColorTemperatureLight.h>
#include <devices/Types.h>

//   1: the feature maps below are forced onto the clusters this device registers.
//   0: each cluster keeps its own default features.
// Guarded so a build can override it with -DDEVICE_TYPE_COLOR_TEMP_LIGHT=0 without editing here.
#ifndef DEVICE_TYPE_COLOR_TEMP_LIGHT
#define DEVICE_TYPE_COLOR_TEMP_LIGHT 1
#endif

namespace {

// The full feature table of each cluster is listed so the maps below can be edited in place;
// [[maybe_unused]] because the bits this device type does not select are otherwise flagged.

// ---- Cluster 0x0006: On/Off ----
namespace OnOffFeatureBits {
[[maybe_unused]] constexpr uint32_t Lighting          = 1 << 0; // = 0x01 — M
[[maybe_unused]] constexpr uint32_t DeadFrontBehavior = 1 << 1; // = 0x02 (not required by this device type)
} // namespace OnOffFeatureBits

// ---- Cluster 0x0008: Level Control ----
namespace LevelControlFeatureBits {
[[maybe_unused]] constexpr uint32_t OnOff     = 1 << 0; // = 0x01 — M
[[maybe_unused]] constexpr uint32_t Lighting  = 1 << 1; // = 0x02 — M
[[maybe_unused]] constexpr uint32_t Frequency = 1 << 2; // = 0x04 (not required by this device type)
} // namespace LevelControlFeatureBits

// ---- Cluster 0x0062: Scenes Management ----
namespace ScenesManagementRequirements {
constexpr bool CopySceneSupported = true; // M — Command, not a feature bit
} // namespace ScenesManagementRequirements

// ---- Cluster 0x0300: Color Control ----
namespace ColorControlFeatureBits {
[[maybe_unused]] constexpr uint32_t HueSaturation    = 1 << 0; // = 0x01 (not required by this device type)
[[maybe_unused]] constexpr uint32_t EnhancedHue      = 1 << 1; // = 0x02 (not required by this device type)
[[maybe_unused]] constexpr uint32_t ColorLoop        = 1 << 2; // = 0x04 (not required by this device type)
[[maybe_unused]] constexpr uint32_t Xy               = 1 << 3; // = 0x08 (not required by this device type)
[[maybe_unused]] constexpr uint32_t ColorTemperature = 1 << 4; // = 0x10 — M
} // namespace ColorControlFeatureBits

[[maybe_unused]] uint32_t onoffFeatureMap = OnOffFeatureBits::Lighting;
// = 0x01

[[maybe_unused]] uint32_t levelFeatureMap = LevelControlFeatureBits::OnOff | LevelControlFeatureBits::Lighting;
// = 0x03

[[maybe_unused]] uint32_t colorFeatureMap = ColorControlFeatureBits::ColorTemperature;
// = 0x10

} // namespace

namespace chip {
namespace app {

ColorTemperatureLight::ColorTemperatureLight(const Context & context) :
    ColorLight(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kColorTemperatureLight, 1), context)
{}

CHIP_ERROR ColorTemperatureLight::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                           EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    ReturnErrorOnFailure(RegisterIdentify(endpoint, provider));
    ReturnErrorOnFailure(RegisterScenesManagement(endpoint, provider, ScenesManagementRequirements::CopySceneSupported));

#if DEVICE_TYPE_COLOR_TEMP_LIGHT
    ReturnErrorOnFailure(RegisterOnOff(endpoint, provider, BitMask<Clusters::OnOff::Feature>(onoffFeatureMap)));
    ReturnErrorOnFailure(RegisterLevelControl(endpoint, provider, BitMask<Clusters::LevelControl::Feature>(levelFeatureMap)));
    ReturnErrorOnFailure(RegisterGroups(endpoint, provider));
    ReturnErrorOnFailure(RegisterColorControl(endpoint, provider, BitMask<Clusters::ColorControl::Feature>(colorFeatureMap),
                                              Clusters::ColorControl::CTColor{}));
#else
    ReturnErrorOnFailure(RegisterOnOff(endpoint, provider));
    ReturnErrorOnFailure(RegisterLevelControl(endpoint, provider));
    ReturnErrorOnFailure(RegisterGroups(endpoint, provider));
    ReturnErrorOnFailure(RegisterColorControl(endpoint, provider, std::nullopt, Clusters::ColorControl::CTColor{}));
#endif

    ReturnErrorOnFailure(RegisterDynamicLighting(endpoint, provider));

    // Scenes are enabled, so the scenable clusters must be registered as handlers to be able to
    // save and recall scenes.
    RegisterSceneHandlers();

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void ColorTemperatureLight::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterAll(provider);
}

} // namespace app
} // namespace chip
