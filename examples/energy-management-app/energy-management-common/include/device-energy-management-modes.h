/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <cstring>
#include <utility>

namespace chip {
namespace app {
namespace Clusters {

namespace DeviceEnergyManagementMode {

constexpr uint8_t kModeNoOptimization             = 0;
constexpr uint8_t kModeDeviceOnlyOptimization     = 1;
constexpr uint8_t kModeDeviceAndLocalOptimization = 2;
constexpr uint8_t kModeDeviceAndGridOptimization  = 3;
constexpr uint8_t kModeAllOptimization            = 4;

/// This is an application level delegate to handle DeviceEnergyManagement modes.
class DeviceEnergyManagementModeDelegate : public ModeBase::Delegate
{
private:
    using ModeTagStructType                                 = detail::Structs::ModeTagStruct::Type;
    ModeTagStructType ModeTagsNoOptimization[1]             = { { .value = to_underlying(ModeTag::kNoOptimization) } };
    ModeTagStructType ModeTagsDeviceOnlyOptimization[1]     = { { .value = to_underlying(ModeTag::kDeviceOptimization) } };
    ModeTagStructType ModeTagsDeviceAndLocalOptimization[2] = { { .value = to_underlying(ModeTag::kLocalOptimization) },
                                                                { .value = to_underlying(ModeTag::kDeviceOptimization) } };
    ModeTagStructType ModeTagsDeviceAndGridOptimization[2]  = { { .value = to_underlying(ModeTag::kDeviceOptimization) },
                                                                { .value = to_underlying(ModeTag::kGridOptimization) } };
    ModeTagStructType ModeTagsAllOptimization[3]            = { { .value = to_underlying(ModeTag::kLocalOptimization) },
                                                                { .value = to_underlying(ModeTag::kDeviceOptimization) },
                                                                { .value = to_underlying(ModeTag::kGridOptimization) } };

    const detail::Structs::ModeOptionStruct::Type kModeOptions[5] = {
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("No energy management (forecast only)"),
                                                 .mode     = kModeNoOptimization,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsNoOptimization) },
        detail::Structs::ModeOptionStruct::Type{ .label = CharSpan::fromCharString("Device optimizes (no local or grid control)"),
                                                 .mode  = kModeDeviceOnlyOptimization,
                                                 .modeTags =
                                                     DataModel::List<const ModeTagStructType>(ModeTagsDeviceOnlyOptimization) },
        detail::Structs::ModeOptionStruct::Type{ .label = CharSpan::fromCharString("Optimized within building"),
                                                 .mode  = kModeDeviceAndLocalOptimization,
                                                 .modeTags =
                                                     DataModel::List<const ModeTagStructType>(ModeTagsDeviceAndLocalOptimization) },
        detail::Structs::ModeOptionStruct::Type{ .label = CharSpan::fromCharString("Optimized for grid"),
                                                 .mode  = kModeDeviceAndGridOptimization,
                                                 .modeTags =
                                                     DataModel::List<const ModeTagStructType>(ModeTagsDeviceAndGridOptimization) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Optimized for grid and building"),
                                                 .mode     = kModeAllOptimization,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(ModeTagsAllOptimization) },

    };

    CHIP_ERROR Init() override;
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags) override;

public:
    ~DeviceEnergyManagementModeDelegate() override = default;
};

ModeBase::Instance * Instance();

void Shutdown();

} // namespace DeviceEnergyManagementMode

} // namespace Clusters
} // namespace app
} // namespace chip
