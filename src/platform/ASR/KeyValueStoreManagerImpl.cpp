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

#include <platform/KeyValueStoreManager.h>
#ifdef CFG_PLF_RV32
#include "alto_flash_kv.h"
#include "asr_flash.h"
#include "asr_rv32.h"
#define duet_flash_kv_set alto_flash_kv_set
#define duet_flash_kv_get alto_flash_kv_get
#define duet_flash_kv_del alto_flash_kv_del
#define duet_flash_erase asr_flash_erase
#elif defined CFG_PLF_DUET
#include "duet_cm4.h"
#include "duet_flash.h"
#include "duet_flash_kv.h"
#else
#include "lega_cm4.h"
#include "lega_flash.h"
#include "lega_flash_kv.h"
#define duet_flash_kv_set lega_flash_kv_set
#define duet_flash_kv_get lega_flash_kv_get
#define duet_flash_kv_del lega_flash_kv_del
#define duet_flash_erase lega_flash_erase
#endif
#include "string.h"

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::Init()
{
    // KV has already been initialized at init_asrPlatform(), so return success
    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::ConvertKVResultToChip(int32_t err) const
{
    switch (err)
    {
    case 0: /* Successed */
        return CHIP_NO_ERROR;
    case -10002: /* The parameter is invalid */
        return CHIP_ERROR_INVALID_ARGUMENT;
    case -10001: /* The space is out of range */
    case -10003: /* The os memory malloc error */
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    case -10005: /* The flash read operation error */
    case -10006: /* The flash write operation error */
    case -10007: /* The flash erase operation error */
        return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
    case -10004: /* Could not found the item */
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    default:
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes) const
{
    int32_t ret        = 0;
    int32_t buffer_len = value_size;
    ret                = duet_flash_kv_get(key, value, (int32_t *) &buffer_len);
    if (ret == 0)
    {
        if (read_bytes_size)
        {
            *read_bytes_size = buffer_len;
        }
    }

    return ConvertKVResultToChip(ret);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    int32_t ret = 0;
    ret         = duet_flash_kv_set(key, value, value_size, 1);

    return ConvertKVResultToChip(ret);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    int32_t ret = 0;
    ret         = duet_flash_kv_del(key);
    return ConvertKVResultToChip(ret);
}

CHIP_ERROR KeyValueStoreManagerImpl::Erase(void)
{
    int32_t ret = 0;
    lega_rtos_declare_critical();
    lega_rtos_enter_critical();
    ret = duet_flash_erase(PARTITION_PARAMETER_2, 0, KV_MAX_SIZE);
    lega_rtos_exit_critical();

    return ConvertKVResultToChip(ret);
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
