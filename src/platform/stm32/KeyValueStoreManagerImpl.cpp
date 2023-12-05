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

#include "flash_wb.h"
#include <lib/support/CodeUtils.h>
#include <platform/KeyValueStoreManager.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MATTER_KEY_NAME_MAX_LENGTH (15 * 2) // ADD Max key name string size is 30 "keyType...;KeyName..."
namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {
/** Singleton instance of the KeyValueStoreManager implementation object.
 */
KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if ((key != NULL) && (value != NULL) && (read_bytes_size != NULL))
    {
        return this->_PrintError(NM_GetKeyValue(value, key, (uint32_t) value_size, read_bytes_size, SECTOR_SECURE));
    }
    else
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{

    ChipLogDetail(DataManagement, "DELETE=> %s", key);
    if (key != NULL)
    {
        return this->_PrintError(NM_DeleteKey(key, SECTOR_SECURE));
    }
    return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{

    if ((value_size != 0) && (key != NULL) && (value != NULL))
    {

        return this->_PrintError(NM_SetKeyValue((char *) value, (char *) key, (uint32_t) value_size, SECTOR_SECURE));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::_PrintError(NVM_StatusTypeDef err)
{
    switch (err)
    {
    case NVM_OK:
        ChipLogDetail(DataManagement, "NVM_OK");
        return CHIP_NO_ERROR;

    case NVM_KEY_NOT_FOUND:
        ChipLogDetail(DataManagement, "CHIP_ERROR_PERSISTED_STORAGE_NOT_FOUND");
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;

    case NVM_WRITE_FAILED:
        ChipLogDetail(DataManagement, "NVM_WRITE_FAILED");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    case NVM_READ_FAILED:
        ChipLogDetail(DataManagement, "NVM_READ_FAILED");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    case NVM_DELETE_FAILED:
        ChipLogDetail(DataManagement, "NVM_DELETE_FAILED");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    case NVM_SIZE_FULL:
        ChipLogDetail(DataManagement, "NVM_SIZE_FULL");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    case NVM_BLOCK_SIZE_OVERFLOW:
        ChipLogDetail(DataManagement, "NVM_BLOCK_SIZE_OVERFLOW");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    case NVM_ERROR_BLOCK_ALIGN:
        ChipLogDetail(DataManagement, "NVM_ERROR_BLOCK_ALIGN");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    case NVM_BUFFER_TOO_SMALL:
        ChipLogDetail(DataManagement, "NVM_BUFFER_TOO_SMALL");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    default:
        ChipLogDetail(DataManagement, "NVM_UNKNOWN_ERROR ");
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
