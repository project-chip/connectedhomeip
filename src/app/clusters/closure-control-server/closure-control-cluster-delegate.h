/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

/** @brief Defines methods for implementing application-specific logic for the Closure Control Cluster.
 */
class DelegateBase
{
public:
    DelegateBase()          = default;
    virtual ~DelegateBase() = default;

    // TODO: Delagte only has the default three APIs for now. This will be refined when the updated cluster structure is integrated
    // in the Closure Sample app
    virtual Protocols::InteractionModel::Status HandleStopCommand()      = 0;
    virtual Protocols::InteractionModel::Status HandleMoveToCommand()    = 0;
    virtual Protocols::InteractionModel::Status HandleCalibrateCommand() = 0;
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
