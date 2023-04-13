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

#pragma once

#include <app/CommandHandlerInterface.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

class Delegate {
public:
    virtual CHIP_ERROR Init() = 0;
    // virtual getSupportedModes();
    // Protocols::InteractionModel::Status getModeOptionByMode(EndpointId, uint8_t, const **ModeSelect::Structs::ModeOptionStruct::Type);
    virtual ~Delegate() = default;
};


} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip
