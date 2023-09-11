/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
