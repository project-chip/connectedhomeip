/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "migration_manager.h"

#include <crypto/OperationalKeystore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>

#ifdef CONFIG_CHIP_MIGRATE_OPERATIONAL_KEYS_TO_ITS
CHIP_ERROR MoveOperationalKeysFromKvsToIts(chip::PersistentStorageDelegate * storage, chip::Crypto::OperationalKeystore * keystore)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(keystore && storage, CHIP_ERROR_INVALID_ARGUMENT);

    /* Initialize the obsolete Operational Keystore*/
    chip::PersistentStorageOperationalKeystore obsoleteKeystore;
    err = obsoleteKeystore.Init(storage);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);

    /* Migrate all obsolete Operational Keys to PSA ITS */
    for (const chip::FabricInfo & fabric : chip::Server::GetInstance().GetFabricTable())
    {
        err = keystore->MigrateOpKeypairForFabric(fabric.GetFabricIndex(), obsoleteKeystore);
        if (CHIP_NO_ERROR != err)
        {
            break;
        }
    }

#ifdef CONFIG_CHIP_FACTORY_RESET_ON_KEY_MIGRATION_FAILURE
    if (CHIP_NO_ERROR != err)
    {
        chip::Server::GetInstance().ScheduleFactoryReset();
        /* Return a success to not block the Matter event Loop and allow to call scheduled factory
         * reset. */
        err = CHIP_NO_ERROR;
    }
#endif /* CONFIG_CHIP_FACTORY_RESET_ON_KEY_MIGRATION_FAILURE */

    return err;
}
#endif /* CONFIG_CHIP_MIGRATE_OPERATIONAL_KEYS_TO_ITS */
