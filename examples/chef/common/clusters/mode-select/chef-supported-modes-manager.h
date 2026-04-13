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

#pragma once

#include <app/clusters/mode-select-server/supported-modes-manager.h>
#include <app/data-model/List.h>
#include <app/util/config.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <map>

#if MATTER_DM_MODE_SELECT_CLUSTER_SERVER_ENDPOINT_COUNT > 0

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {
namespace Chef {

using chip::operator""_span;

static const Structs::SemanticTagStruct::Type kModeOneTags[]   = { { .mfgCode = static_cast<chip::VendorId>(0xFFF1), .value = 1 } };
static const Structs::SemanticTagStruct::Type kModeTwoTags[]   = { { .mfgCode = static_cast<chip::VendorId>(0xFFF1), .value = 2 } };
static const Structs::SemanticTagStruct::Type kModeThreeTags[] = { { .mfgCode = static_cast<chip::VendorId>(0xFFF1), .value = 3 } };

static const Structs::ModeOptionStruct::Type kDefaultModeOptions[] = {
    {
        .label        = "One"_span,
        .mode         = 1,
        .semanticTags = chip::app::DataModel::List<const Structs::SemanticTagStruct::Type>(kModeOneTags),
    },
    {
        .label        = "Two"_span,
        .mode         = 2,
        .semanticTags = chip::app::DataModel::List<const Structs::SemanticTagStruct::Type>(kModeTwoTags),
    },
    {
        .label        = "Three"_span,
        .mode         = 3,
        .semanticTags = chip::app::DataModel::List<const Structs::SemanticTagStruct::Type>(kModeThreeTags),
    }
};

class ChefSupportedModesManager : public SupportedModesManager
{
    using ModeOptionStructType = Structs::ModeOptionStruct::Type;

public:
    CHIP_ERROR AddModeOptionsProvider(EndpointId endpoint, ModeOptionsProvider optionsProvider);

    ModeOptionsProvider getModeOptionsProvider(EndpointId endpointId) const override;

    Protocols::InteractionModel::Status getModeOptionByMode(EndpointId endpointId, uint8_t mode,
                                                            const ModeOptionStructType ** dataPtr) const override;

    ~ChefSupportedModesManager() override {}

private:
    std::map<EndpointId, ModeOptionsProvider> mModeOptionsPerEndpoint;
};

} // namespace Chef
} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip

#endif // MATTER_DM_MODE_SELECT_CLUSTER_SERVER_ENDPOINT_COUNT
