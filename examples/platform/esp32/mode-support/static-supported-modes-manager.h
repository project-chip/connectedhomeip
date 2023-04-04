/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <src/app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

class StaticSupportedModesManager : public chip::app::Clusters::ModeSelect::SupportedModesManager
{
private:
    using ModeOptionStructType = Structs::ModeOptionStruct::Type;
    using SemanticTag          = Structs::SemanticTagStruct::Type;

    struct ModeLabel
    {
        char supportedModeLabel[64];
    };

    static ModeLabel * modeLabelList;
    static ModeOptionStructType * modeOptionStruct;
    static SemanticTag * semanticTags;
    // TODO : We need to decide wheather the endpointArray shoule be static or dynamic.
    static ModeOptionStructType * endpointArray[FIXED_ENDPOINT_COUNT][2];

    void InitEndpointArray();

    void FreeSupportedModes();

public:
    static const StaticSupportedModesManager instance;

    SupportedModesManager::ModeOptionsProvider getModeOptionsProvider(EndpointId endpointId) const override;

    Protocols::InteractionModel::Status getModeOptionByMode(EndpointId endpointId, uint8_t mode,
                                                            const ModeOptionStructType ** dataPtr) const override;

    StaticSupportedModesManager() { InitEndpointArray(); }

    ~StaticSupportedModesManager() { FreeSupportedModes(); }

    static inline const StaticSupportedModesManager & getStaticSupportedModesManagerInstance() { return instance; }
};

const SupportedModesManager * getSupportedModesManager();

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip
