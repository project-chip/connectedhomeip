
/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/clusters/UnitLocalization/AttributeIds.h>
#include <app/clusters/unit-localization-server/unit-localization-server.h>
#include <app/persistence/AttributePersistenceMigration.h>
#include <app/persistence/AttributePersistenceProvider.h>
namespace chip {
namespace app {
namespace Clusters {

namespace UnitLocalization {
void SafeAttributePersistanceMigration(const ConcreteClusterPath & clusterPath, ServerClusterContext & context)
{
    static constexpr AttributeId attributesToUpdate[] = { UnitLocalization::Attributes::TemperatureUnit::Id };
    MigrateFromSafeAttributePersistenceProvider(clusterPath, Span(attributesToUpdate), context.storage);
}
} // namespace UnitLocalization
} // namespace Clusters
} // namespace app
} // namespace chip
