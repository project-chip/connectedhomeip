/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <lib/core/CHIPCore.h>
#include <lib/core/PeerId.h>
#include <lib/support/DLLUtil.h>

namespace chip {

/**
 * @brief The Persistent Storage Delegate provides a Fabric scoped storage
 *        interface for the CHIP stack to store items.
 *
 *        The FabricId used here allows for storage to reuse keys within each fabric's scope
 *        These APIs might be called with `kUndefinedCompressedFabricId` in which case
 *        The delegate is expected to access a "global" storage scope.
 *
 */
class DLL_EXPORT PersistentStorageDelegate
{
public:
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
     * @param[in]      fabricId The compressed Fabric Id where this key is stored
     * @param[in]      key Key to lookup
     * @param[out]     buffer Value for the key
     * @param[in, out] size Input value buffer size, output length of value.
     *                 The output length could be larger than input value. In
     *                 such cases, the user should allocate the buffer large
     *                 enough (>= output length), and call the API again.
     */
    virtual CHIP_ERROR SyncGetKeyValue(const CompressedFabricId fabricId, const char * key, void * buffer, uint16_t & size) = 0;

    /**
     * @brief
     *   Set the value for the key to a byte buffer.
     *
     * @param[in] fabricId The compressed Fabric Id where this key should be stored
     * @param[in] key Key to be set
     * @param[in] value Value to be set
     * @param[in] size Size of the Value
     */
    virtual CHIP_ERROR SyncSetKeyValue(const CompressedFabricId fabricId, const char * key, const void * value, uint16_t size) = 0;

    /**
     * @brief
     *   Deletes the value for the key
     *
     * @param[in] fabricId The compressed Fabric Id where this key is stored
     * @param[in] key Key to be deleted
     */
    virtual CHIP_ERROR SyncDeleteKeyValue(const CompressedFabricId fabricId, const char * key) = 0;
};

} // namespace chip
