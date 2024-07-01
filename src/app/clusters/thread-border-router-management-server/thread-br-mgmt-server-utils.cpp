/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "app/server/Server.h"
#include "lib/core/DataModelTypes.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadBorderRouterManagement {

bool IsFailSafeArmed(FabricIndex accessingFabricIndex)
{
    auto & failSafeContext = Server::GetInstance().GetFailSafeContext();
    return failSafeContext.IsFailSafeArmed(accessingFabricIndex);
}

void DisarmFailSafeTimer()
{
    auto & failSafeContext = Server::GetInstance().GetFailSafeContext();
    failSafeContext.DisarmFailSafe();
}

} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
