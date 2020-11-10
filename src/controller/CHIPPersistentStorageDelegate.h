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

#include <core/CHIPCore.h>
#include <support/DLLUtil.h>

namespace chip {
namespace Controller {

class DLL_EXPORT PersistentStorageResultDelegate
{
public:
    enum class Operation : uint8_t
    {
        kGET = 0,
        kSET,
        kDELETE,
    };

    virtual ~PersistentStorageResultDelegate() {}

    /**
     * @brief
     *   Called when a value is returned from the storage.
     *   This is useful for GetKeyValue() API call.
     *
     * @param[in] key Key for which the value is being returned
     * @param[in] value Value or nullptr if not found.
     */
    virtual void OnValue(const char * key, const char * value) = 0;

    /**
     * @brief
     *   Called on completion of an operation in PersistentStorageDelegate API
     *
     * @param[in] key Key for which the status is being returned
     * @param[in] op Operation that was being performed on the key
     * @param[in] result CHIP_NO_ERROR or corresponding error code
     */
    virtual void OnStatus(const char * key, Operation op, CHIP_ERROR result) = 0;
};

class DLL_EXPORT PersistentStorageDelegate
{
public:
    virtual ~PersistentStorageDelegate() {}

    /**
     * @brief
     *   Set the callback object with methods that are called on completion
     *   of the operation.
     *
     * @param[in] delegate The callback object
     */
    virtual void SetDelegate(PersistentStorageResultDelegate * delegate) = 0;

    /**
     * @brief
     *   Lookup the key and call delegate object with it's stringified value
     *
     * @param[in] key Key to lookup
     */
    virtual void GetKeyValue(const char * key) = 0;

    /**
     * @brief
     *   This is a synchronous Get API, where the value is returned via the output
     *   buffer. This API should be used sparingly, since it may block for
     *   some duration.
     *
     * @param[in]      key Key to lookup
     * @param[out]     value Value for the key
     * @param[in, out] size Input value buffer size, output length of value.
     *                 The output length could be larger than input value. In
     *                 such cases, the user should allocate the buffer large
     *                 enough (>= output length), and call the API again.
     */
    virtual CHIP_ERROR GetKeyValue(const char * key, char * value, uint16_t & size) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    /**
     * @brief
     *   Set the value for the key
     *
     * @param[in] key Key to be set
     * @param[in] value Value to be set
     */
    virtual void SetKeyValue(const char * key, const char * value) = 0;

    /**
     * @brief
     *   Deletes the value for the key
     *
     * @param[in] key Key to be deleted
     */
    virtual void DeleteKeyValue(const char * key) = 0;
};

} // namespace Controller
} // namespace chip
