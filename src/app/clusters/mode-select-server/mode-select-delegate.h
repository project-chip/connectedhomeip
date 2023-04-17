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
// todo update the above?

#ifndef SOME_UNIQUE_NAME_HERE
#define SOME_UNIQUE_NAME_HERE
#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandlerInterface.h>
#include <app/util/util.h>
#include <utility>
#include <vector>

using chip::Protocols::InteractionModel::Status;
using ModeOptionStructType  = chip::app::Clusters::ModeSelect::Structs::ModeOptionStruct::Type;
using SemanticTagStructType = chip::app::Clusters::ModeSelect::Structs::SemanticTagStruct::Type;

template <typename T>
using List               = chip::app::DataModel::List<T>;

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

class Delegate {
private:
    const std::vector<ModeOptionStructType> modeOptions;

public:
    /** 
     * This is a helper function to build a mode option struture. It takes the label/name of the mode,
     * the value of the mode and a list of semantic tags that apply to this mode.
     */
    static ModeOptionStructType BuildModeOptionStruct(const char * label, uint8_t mode, const List<const SemanticTagStructType> & semanticTags)
    {
        Structs::ModeOptionStruct::Type option;
        option.label        = CharSpan::fromCharString(label);
        option.mode         = mode;
        option.semanticTags = semanticTags;
        return option;
    }

    explicit Delegate(std::vector<ModeOptionStructType> modes) : modeOptions(std::move(modes))
    {
        
    }

    virtual CHIP_ERROR Init() = 0;

    Status IsSupportedMode(uint8_t mode);

    Status GetMode(uint8_t mode, ModeOptionStructType &modeOption);

    virtual ~Delegate() = default;
};

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip

#endif