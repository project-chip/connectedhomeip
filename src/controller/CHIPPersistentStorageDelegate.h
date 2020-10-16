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

class DLL_EXPORT PersistentStorageDelegate
{
public:
    virtual ~PersistentStorageDelegate() {}

    /**
     * @brief
     *   Lookup the key and return it's stringified value
     *
     * @param[in] key Key to lookup
     * @return Value or nullptr if not found. Lifetime of the returned
     *         buffer is tied to the delegate object. If the delegate is
     *         freed, the returned value would be inaccessible.
     */
    virtual const char * GetKeyValue(const char * key) = 0;

    /**
     * @brief
     *   Set the value for the key
     *
     * @param[in] key Key to be set
     * @param[in] value Value to be set
     * @return returns corresponding error if unsuccessful
     */
    virtual CHIP_ERROR SetKeyValue(const char * key, const char * value) = 0;

    /**
     * @brief
     *   Deletes the value for the key
     *
     * @param[in] key Key to be deleted
     * @return returns corresponding error if unsuccessful
     */
    virtual CHIP_ERROR DeleteKeyValue(const char * key) = 0;
};

} // namespace DeviceController
} // namespace chip
