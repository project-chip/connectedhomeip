/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <lib/support/DLLUtil.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {

class DLL_EXPORT PersistentStorageDelegate
{
public:
    /**
     *  Maximum length of a key required by implementations. Any implementer of PersistentStorageDelegate
     *  must support keys AT LEAST this long.
     */
    static constexpr size_t kKeyLengthMax = 32;

    virtual ~PersistentStorageDelegate() {}

    /**
     * @brief
     *   This is a synchronous Get API, where the value is returned via the output
     *   buffer.
     *
     *   This API can be used to retrieve a byte buffer value from the storage.
     *   There is no implied data format and and data will be stored/fetched binary.
     *   Caller is responsible to take care of any special formatting needs (e.g. byte
     *   order, null terminators, consistency checks or versioning).
     *
     *   If a key is found and the `buffer`'s `size` is large enough, then the value will
     *   be copied to `buffer` and `size` will be updated to the actual size used.
     *
     *   Whenever the passed `size` is smaller than the size of the stored value for the given key,
     *   CHIP_ERROR_BUFFER_TOO_SMALL will be returned, but the `buffer` will still be filled with the
     *   first `size` bytes of the stored value.
     *
     *   In the case where `size` of 0 is given, and the value stored is of size 0, CHIP_NO_ERROR is
     *   returned. It is recommended to use helper method SyncDoesKeyExist(key) to determine if key
     *   exists.
     *
     *   It is legal to use `nullptr` for `buffer` if `size` is 0.
     *
     * @param[in]      key Key to lookup
     * @param[out]     buffer Pointer to a buffer where the place the read value.
     * @param[in, out] size Input is maximum buffer size, output updated to number of bytes written into `buffer`.
     *
     * @return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND the key is not found in storage.
     * @return CHIP_ERROR_BUFFER_TOO_SMALL the provided buffer is not big enough.  In this case
     *                                     the first `size` bytes of the value will be placed in `buffer`.
     */
    virtual CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) = 0;

    /**
     * @brief
     *   Set the value for the key to a byte buffer. Empty values can be stored
     *   with size == 0, in which case `value` may be nullptr.
     *
     * @param[in] key Key to set
     * @param[in] value Pointer to bytes of value to be set. `value` can only be `nullptr` if size == 0.
     * @param[in] size Size of the `value` to store.
     *
     * @return CHIP_NO_ERROR on success, CHIP_INVALID_ARGUMENT on `value` being `nullptr` with size > 0,
     *         or another CHIP_ERROR value from implementation on failure.
     */
    virtual CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) = 0;

    /**
     * @brief
     *   Deletes the value for the key
     *
     * @param[in] key Key to be deleted
     *
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND if the key is not found in storage,
     *         or another CHIP_ERROR value from implementation on failure.
     */
    virtual CHIP_ERROR SyncDeleteKeyValue(const char * key) = 0;

    /**
     * @brief
     *   Helper function that identifies if a key exists.
     *
     *   This may be overridden to provide an implementation that is simpler or more direct.
     *
     * @param[in] key Key to check if it exist
     *
     * @return true if key exists in storage. It returns false if key does not exist in storage or an internal error arises.
     */
    virtual bool SyncDoesKeyExist(const char * key)
    {
        uint16_t size  = 0;
        CHIP_ERROR err = SyncGetKeyValue(key, nullptr, size);
        return (err == CHIP_ERROR_BUFFER_TOO_SMALL) || (err == CHIP_NO_ERROR);
    }
};

} // namespace chip
