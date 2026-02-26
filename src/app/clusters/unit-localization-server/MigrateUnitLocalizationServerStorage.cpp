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
#include <app/clusters/unit-localization-server/UnitLocalizationCluster.h>
#include <app/persistence/AttributePersistenceMigration.h>

namespace chip {
namespace app {
namespace Clusters {
namespace UnitLocalization {

CHIP_ERROR MigrateUnitLocalizationClusterStorage(EndpointId endpointId, PersistentStorageDelegate & storageDelegate)
{
    static constexpr AttrMigrationData attributesToUpdate[] = { { Attributes::TemperatureUnit::Id,
                                                                  &DefaultMigrators::ScalarValue<uint8_t> } };
    return MigrateFromSafeToAttributePersistenceProvider<sizeof(uint8_t)>({ endpointId, UnitLocalization::Id },
                                                                          Span(attributesToUpdate), storageDelegate);
}

} // namespace UnitLocalization
} // namespace Clusters
} // namespace app
} // namespace chip
