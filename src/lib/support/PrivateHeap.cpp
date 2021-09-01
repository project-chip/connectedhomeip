/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright 2019 Google Inc. All Rights Reserved.
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

#include "PrivateHeap.h"

#include <string.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace {

constexpr uint32_t kInvalidHeapBlockSize = 0xFFFFFFFF;
constexpr int32_t kHeapBlockInUse        = 0x01;
constexpr int32_t kHeapBlockFree         = 0x10;
constexpr int32_t kInvalidHeaderState    = 0xff;

using internal::PrivateHeapBlockHeader;

// this makes life easier, no need to add align offsets.
static_assert(sizeof(PrivateHeapBlockHeader) % kPrivateHeapAllocationAlignment == 0, "Invalid block size.");

uint32_t ComputeHeapBlockChecksum(const PrivateHeapBlockHeader * header)
{
    uint32_t checksum = header->prevBytes;
    checksum *= 31;
    checksum += header->nextBytes;
    checksum *= 31;
    checksum += header->state;
    return checksum;
}

// Advances the heap block to the next value
PrivateHeapBlockHeader * NextHeader(PrivateHeapBlockHeader * start)
{
    if (start->nextBytes == kInvalidHeapBlockSize)
    {
        return nullptr;
    }

    if (start->checksum != ComputeHeapBlockChecksum(start))
    {
        ChipLogError(Support, "Corrupted heap: checksum is invalid");
        chipDie();
    }

    return reinterpret_cast<PrivateHeapBlockHeader *>(reinterpret_cast<char *>(start) + sizeof(PrivateHeapBlockHeader) +
                                                      start->nextBytes);
}

// Advances the heap block to the previous value
PrivateHeapBlockHeader * PreviousHeader(PrivateHeapBlockHeader * start)
{
    if (start->prevBytes == kInvalidHeapBlockSize)
    {
        return nullptr;
    }

    if (start->checksum != ComputeHeapBlockChecksum(start))
    {
        ChipLogError(Support, "Corrupted heap: checksum is invalid");
        chipDie();
    }

    return reinterpret_cast<PrivateHeapBlockHeader *>(reinterpret_cast<char *>(start) - sizeof(PrivateHeapBlockHeader) -
                                                      start->prevBytes);
}

void ValidateHeader(const PrivateHeapBlockHeader * header)
{
    if (header->state != kHeapBlockFree && header->state != kHeapBlockInUse)
    {
        ChipLogError(Support, "Invalid header state (neither free nor in use) at %p", header);
        chipDie();
    }

    if (header->checksum != ComputeHeapBlockChecksum(header))
    {
        ChipLogError(Support, "Corrupted heap: checksum is invalid at %p", header);
        chipDie();
    }
}

} // namespace

extern "C" void PrivateHeapInit(void * heap, size_t size)
{
    if (heap == nullptr)
    {
        ChipLogError(Support, "Cannot initialize null heap");
        chipDie();
    }

    if (size < 2 * sizeof(PrivateHeapBlockHeader))
    {
        ChipLogError(Support, "Insufficient space in private heap");
        chipDie();
    }

    if (reinterpret_cast<uintptr_t>(heap) % kPrivateHeapAllocationAlignment != 0)
    {
        ChipLogError(Support, "Invalid alignment for private heap initialization");
        chipDie();
    }

    PrivateHeapBlockHeader * header = reinterpret_cast<PrivateHeapBlockHeader *>(heap);

    header->prevBytes = kInvalidHeapBlockSize;
    header->nextBytes = static_cast<uint32_t>(size - 2 * sizeof(PrivateHeapBlockHeader));
    header->state     = kHeapBlockFree;
    header->checksum  = ComputeHeapBlockChecksum(header);

    header            = NextHeader(header);
    header->nextBytes = kInvalidHeapBlockSize;
    header->prevBytes = static_cast<uint32_t>(size - 2 * sizeof(PrivateHeapBlockHeader));
    header->state     = kHeapBlockFree; // does not matter really
    header->checksum  = ComputeHeapBlockChecksum(header);
}

extern "C" void * PrivateHeapAlloc(void * heap, size_t size)
{
    PrivateHeapBlockHeader * header = reinterpret_cast<PrivateHeapBlockHeader *>(heap);

    // we allocate aligned, no matter what
    if (size % kPrivateHeapAllocationAlignment != 0)
    {
        size += kPrivateHeapAllocationAlignment - (size % kPrivateHeapAllocationAlignment);
    }

    for (; header != nullptr; header = NextHeader(header))
    {
        ValidateHeader(header);

        if (header->nextBytes == kInvalidHeapBlockSize)
        {
            continue;
        }

        if (header->state != kHeapBlockFree)
        {
            continue; // not free
        }

        if (header->nextBytes < size)
        {
            continue; // insufficient space
        }

        if (header->nextBytes - size < sizeof(PrivateHeapBlockHeader) + kPrivateHeapAllocationAlignment)
        {
            // allocate the entire block
            header->state    = kHeapBlockInUse;
            header->checksum = ComputeHeapBlockChecksum(header);
        }
        else
        {
            // splits the block into two
            //
            // +--------+       +--------+       +------+
            // | header |  ---> | middle |  ---> | next |
            // +--------+       +--------+       +------+
            //
            PrivateHeapBlockHeader * next = NextHeader(header);
            PrivateHeapBlockHeader * middle =
                reinterpret_cast<PrivateHeapBlockHeader *>(reinterpret_cast<char *>(header + 1) + size);

            // middle is a new block
            middle->nextBytes = static_cast<uint32_t>(header->nextBytes - size - sizeof(PrivateHeapBlockHeader));
            middle->prevBytes = static_cast<uint32_t>(size);
            middle->state     = kHeapBlockFree;
            middle->checksum  = ComputeHeapBlockChecksum(middle);

            // fix up the header
            header->nextBytes = static_cast<uint32_t>(size);
            header->state     = kHeapBlockInUse;
            header->checksum  = ComputeHeapBlockChecksum(header);

            // fix up the final block
            if (next != nullptr)
            {
                next->prevBytes = middle->nextBytes;
                next->checksum  = ComputeHeapBlockChecksum(next);
            }
        }

        // we can now use the header
        return header + 1; // data right after the header
    }

    // no space found
    return nullptr;
}

extern "C" void PrivateHeapFree(void * ptr)
{
    if (ptr == nullptr)
    {
        // freeing NULL pointers is always acceptable and a noop
        return;
    }

    PrivateHeapBlockHeader * header =
        reinterpret_cast<PrivateHeapBlockHeader *>(static_cast<char *>(ptr) - sizeof(PrivateHeapBlockHeader));

    ValidateHeader(header);
    header->state    = kHeapBlockFree;
    header->checksum = ComputeHeapBlockChecksum(header);

    // Merge with previous
    //
    // +-------+                          +--------+
    // | other |  ----- nextBytes ----->  | header |
    // +-------+                          +--------+
    //
    PrivateHeapBlockHeader * other = PreviousHeader(header);
    if (other != nullptr && other->state == kHeapBlockFree && other->nextBytes != kInvalidHeapBlockSize)
    {
        // includes the free bytes in this block in the previous
        other->nextBytes += static_cast<uint32_t>(header->nextBytes + sizeof(PrivateHeapBlockHeader));
        other->checksum = ComputeHeapBlockChecksum(other);
        header->state   = kInvalidHeaderState;
        header          = other;

        // fixes up the next block
        other = NextHeader(header);
        if (other != nullptr)
        {
            other->prevBytes = header->nextBytes;
            other->checksum  = ComputeHeapBlockChecksum(other);
        }
    }

    // Merge with next
    //
    // +--------+                          +-------+
    // | header |  ----- nextBytes ----->  | other |
    // +--------+                          +-------+
    //
    other = NextHeader(header);
    if (other != nullptr && other->state == kHeapBlockFree && other->nextBytes != kInvalidHeapBlockSize)
    {
        // includes the free bytes in the next block
        other->state = kInvalidHeaderState;
        header->nextBytes += static_cast<uint32_t>(other->nextBytes + sizeof(PrivateHeapBlockHeader));
        header->checksum = ComputeHeapBlockChecksum(header);

        // fixes up the next block
        other = NextHeader(header);
        if (other != nullptr)
        {
            other->prevBytes = header->nextBytes;
            other->checksum  = ComputeHeapBlockChecksum(other);
        }
    }
}

void * PrivateHeapRealloc(void * heap, void * ptr, size_t size)
{
    if (ptr == nullptr)
    {
        return PrivateHeapAlloc(heap, size);
    }

    if (size == 0)
    {
        PrivateHeapFree(ptr);
        return nullptr;
    }

    PrivateHeapBlockHeader * header =
        reinterpret_cast<PrivateHeapBlockHeader *>(static_cast<char *>(ptr) - sizeof(PrivateHeapBlockHeader));

    ValidateHeader(header);

    if (header->nextBytes >= size)
    {
        return ptr; // no reallocation needed
    }

    void * largerCopy = PrivateHeapAlloc(heap, size);
    if (largerCopy == nullptr)
    {
        // NOTE: original is left untouched (not freed) to match realloc() libc
        // functionality
        return nullptr;
    }

    memcpy(largerCopy, ptr, header->nextBytes);
    PrivateHeapFree(ptr);

    return largerCopy;
}

extern "C" void PrivateHeapDump(void * top)
{
    PrivateHeapBlockHeader * header = reinterpret_cast<PrivateHeapBlockHeader *>(top);

    ChipLogProgress(Support, "========= HEAP ===========");
    while (header->nextBytes != kInvalidHeapBlockSize)
    {
        ChipLogProgress(Support, "    %td: size: %d, state: %d", reinterpret_cast<char *>(header) - reinterpret_cast<char *>(top),
                        static_cast<int>(header->nextBytes), static_cast<int>(header->state));

        header = NextHeader(header);
    }
}
