/*
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

#include <stdint.h>
#include <stdio.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {

class MigrationManager
{
public:
    /**
     * The Silabs migration manager is implemented as a singleton
     * User should get the object from this getter.
     */
    static MigrationManager & GetMigrationInstance();
    static void applyMigrations();
    static void MigrateUint16(uint32_t old_key, uint32_t new_key);
    static void MigrateUint32(uint32_t old_key, uint32_t new_key);

private:
    MigrationManager(){};
    ~MigrationManager(){};
};

/**
 * Migration functions. These definitions allow the MigrationManager
 * to be agnostic of the specifics of each individual migration.
 */
void MigrateKvsMap(void);
void MigrateDacProvider(void);
void MigrateCounterConfigs(void);
void MigrateHardwareVersion(void);

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
