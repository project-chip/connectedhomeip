/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <protocols/secure_channel/SessionIDAllocator.h>

#include <lib/support/CodeUtils.h>

namespace chip {

uint16_t SessionIDAllocator::sNextAvailable = 1;

CHIP_ERROR SessionIDAllocator::Allocate(uint16_t & id)
{
    VerifyOrReturnError(sNextAvailable < kMaxSessionID, CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(sNextAvailable > kUnsecuredSessionId, CHIP_ERROR_INTERNAL);
    id = sNextAvailable;

    // TODO - Update SessionID allocator to use freed session IDs
    sNextAvailable++;

    return CHIP_NO_ERROR;
}

void SessionIDAllocator::Free(uint16_t id)
{
    // As per spec 4.4.1.3 Session ID of 0 is reserved for Unsecure communication
    if (sNextAvailable > (kUnsecuredSessionId + 1) && (sNextAvailable - 1) == id)
    {
        sNextAvailable--;
    }
}

CHIP_ERROR SessionIDAllocator::Reserve(uint16_t id)
{
    VerifyOrReturnError(id < kMaxSessionID, CHIP_ERROR_NO_MEMORY);
    if (id >= sNextAvailable)
    {
        sNextAvailable = id;
        sNextAvailable++;
    }

    // TODO - Check if ID is already allocated in SessionIDAllocator::Reserve()

    return CHIP_NO_ERROR;
}

CHIP_ERROR SessionIDAllocator::ReserveUpTo(uint16_t id)
{
    VerifyOrReturnError(id < kMaxSessionID, CHIP_ERROR_NO_MEMORY);
    if (id >= sNextAvailable)
    {
        sNextAvailable = id;
        sNextAvailable++;
    }

    // TODO - Update ReserveUpTo to mark all IDs in use
    // Current SessionIDAllocator only tracks the smallest unused session ID.
    // If/when we change it to track all in use IDs, we should also update ReserveUpTo
    // to reserve all individual session IDs, instead of just setting the sNextAvailable.

    return CHIP_NO_ERROR;
}

uint16_t SessionIDAllocator::Peek()
{
    return sNextAvailable;
}

} // namespace chip
