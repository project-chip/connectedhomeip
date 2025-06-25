/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <atomic>
#include <lib/core/CHIPError.h>

class DFUSync
{
public:
    /**
     * @brief Tries to take a mutex allowing to perform the DFU process.
     *
     * @param id reference to the mutex owner id that is assigned with random id, if the mutex was taken successfully
     *
     * @return CHIP_NO_ERROR on success, the other error code on failure.
     */
    CHIP_ERROR Take(uint32_t & id);

    /**
     * @brief Tries to free a mutex allowing to perform the DFU process.
     *
     * @param id mutex owner id that has to be equal to the id assigned by Take method to prevent free attempts from the other
     * threads that do not own the mutex.
     *
     * @return CHIP_NO_ERROR on success, the other error code on failure.
     */
    CHIP_ERROR Free(uint32_t id);

    /**
     * @brief Get the DFUSync instance
     *
     * @return DFUSync object
     */
    static inline DFUSync & GetInstance()
    {
        static DFUSync sInstance;
        return sInstance;
    }

private:
    /* Mutex to synchronize the DFU operations. */
    std::atomic<bool> mIsTaken = false;
    uint32_t mOwnerId          = 0;
};
