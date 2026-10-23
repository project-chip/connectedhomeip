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

#include "DFUSync.h"

CHIP_ERROR DFUSync::Take(uint32_t & id)
{
    if (mIsTaken)
    {
        if (id == mOwnerId)
        {
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_BUSY;
    }

    mIsTaken = true;
    /* Increment owner id to make sure that every allocation is unique. */
    mOwnerId++;
    id = mOwnerId;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DFUSync::Free(uint32_t id)
{
    /* Prevent free operation from the threads that do not own mutex. */
    if (id != mOwnerId)
    {
        return CHIP_ERROR_ACCESS_DENIED;
    }

    mIsTaken = false;

    return CHIP_NO_ERROR;
}
