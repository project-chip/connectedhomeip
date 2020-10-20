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
namespace DeviceController {

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
    virtual void OnStatus(const char * key, Operation op, CHIP_ERROR err) = 0;
};

class DLL_EXPORT PersistentStorageDelegate
{
public:
    virtual ~PersistentStorageDelegate() {}

    /**
     * @brief
     *   Lookup the key and return it's stringified value
     *
     * @param[in] key Key to lookup
     * @param[in] delegate Callback handler using which the value wiil be returned.
     */
    virtual void SetDelegate(PersistentStorageResultDelegate * delegate) = 0;

    /**
     * @brief
     *   Lookup the key and return it's stringified value
     *
     * @param[in] key Key to lookup
     */
    virtual void GetKeyValue(const char * key) = 0;

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

} // namespace DeviceController
} // namespace chip
