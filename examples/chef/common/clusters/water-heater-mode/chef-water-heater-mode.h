/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/util/config.h>
#include <cstring>
#include <utility>

#include <lib/support/Span.h>

#pragma once

#if MATTER_DM_WATER_HEATER_MODE_CLUSTER_SERVER_ENDPOINT_COUNT > 0

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterMode {
namespace Chef {

const uint8_t ModeOff    = 0;
const uint8_t ModeManual = 1;

static const detail::Structs::ModeTagStruct::Type modeTagsOff[]    = { { .value = to_underlying(ModeTag::kOff) } };
static const detail::Structs::ModeTagStruct::Type modeTagsManual[] = { { .value = to_underlying(ModeTag::kManual) } };

static const detail::Structs::ModeOptionStruct::Type kModeOptions[] = {
    detail::Structs::ModeOptionStruct::Type{ .label    = "Off"_span,
                                             .mode     = ModeOff,
                                             .modeTags = DataModel::List<const detail::Structs::ModeTagStruct::Type>(modeTagsOff) },
    detail::Structs::ModeOptionStruct::Type{ .label = "Manual"_span,
                                             .mode  = ModeManual,
                                             .modeTags =
                                                 DataModel::List<const detail::Structs::ModeTagStruct::Type>(modeTagsManual) },
};

static const Span<const detail::Structs::ModeOptionStruct::Type> kSupportedModes(kModeOptions);

} // namespace Chef
} // namespace WaterHeaterMode
} // namespace Clusters
} // namespace app
} // namespace chip

#endif // MATTER_DM_WATER_HEATER_MODE_CLUSTER_SERVER_ENDPOINT_COUNT > 0
