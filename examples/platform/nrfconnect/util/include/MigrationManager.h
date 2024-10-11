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

#pragma once

#include <app/server/Server.h>

#ifdef CONFIG_CHIP_MIGRATE_OPERATIONAL_KEYS_TO_ITS
/**
 * @brief Migrate all stored Operational Keys from the persistent storage (KVS) to secure PSA ITS.
 *
 * This function will schedule a factory reset automatically if the
 * CONFIG_CHIP_FACTORY_RESET_ON_KEY_MIGRATION_FAILURE
 * Kconfig option is set to 'y'. In this case, the function returns CHIP_NO_ERROR to not block any further
 * operations until the scheduled factory reset is done.
 *
 * @note This function should be called just after Matter Server Init to avoid problems with further CASE
 * session re-establishments.
 * @param storage
 * @param keystore
 * @retval CHIP_NO_ERROR if all keys have been migrated properly to PSA ITS or if the error occurs, but
 * 		   the CONFIG_CHIP_FACTORY_RESET_ON_KEY_MIGRATION_FAILURE kconfig is set to 'y'.
 * @retval CHIP_ERROR_INVALID_ARGUMENT when keystore or storage are not defined.
 * @retval Other CHIP_ERROR codes related to internal Migration operations.
 */
CHIP_ERROR MoveOperationalKeysFromKvsToIts(chip::PersistentStorageDelegate * storage, chip::Crypto::OperationalKeystore * keystore);
#endif
