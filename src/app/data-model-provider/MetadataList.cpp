/*
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
#include <app/data-model-provider/MetadataList.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>

namespace chip {
namespace app {
namespace DataModel {
namespace detail {

GenericAppendOnlyBuffer::~GenericAppendOnlyBuffer()
{
    if (mBufferIsAllocated && (mBuffer != nullptr))
    {
        Platform::MemoryFree(mBuffer);
    }
}

GenericAppendOnlyBuffer::GenericAppendOnlyBuffer(GenericAppendOnlyBuffer && other) : mElementSize(other.mElementSize)
{
    // take over the data
    mBuffer            = other.mBuffer;
    mElementCount      = other.mElementCount;
    mCapacity          = other.mCapacity;
    mBufferIsAllocated = other.mBufferIsAllocated;

    // clear other
    other.mBuffer            = nullptr;
    other.mElementCount      = 0;
    other.mCapacity          = 0;
    other.mBufferIsAllocated = false;
}

GenericAppendOnlyBuffer & GenericAppendOnlyBuffer::operator=(GenericAppendOnlyBuffer && other)
{
    VerifyOrDie(mElementSize == other.mElementSize);

    if (mBufferIsAllocated && (mBuffer != nullptr))
    {
        Platform::Impl::PlatformMemoryManagement::MemoryFree(mBuffer);
    }

    // take over the data
    mBuffer            = other.mBuffer;
    mElementCount      = other.mElementCount;
    mCapacity          = other.mCapacity;
    mBufferIsAllocated = other.mBufferIsAllocated;

    // clear other
    other.mBuffer            = nullptr;
    other.mElementCount      = 0;
    other.mCapacity          = 0;
    other.mBufferIsAllocated = false;

    return *this;
}

CHIP_ERROR GenericAppendOnlyBuffer::EnsureAppendCapacity(size_t numElements)
{
    if (mCapacity >= mElementCount + numElements)
    {
        // Sufficient capacity already exists
        return CHIP_NO_ERROR;
    }

    if (mBuffer == nullptr)
    {
        mBuffer = static_cast<uint8_t *>(Platform::MemoryCalloc(numElements, mElementSize));
        VerifyOrReturnError(mBuffer != nullptr, CHIP_ERROR_NO_MEMORY);
        mCapacity          = numElements;
        mBufferIsAllocated = true;
        return CHIP_NO_ERROR;
    }

    // we already have the data in buffer. we have two choices:
    //   - allocated buffer needs to be extended
    //   - re-used const buffer needs to be copied over
    if (mBufferIsAllocated)
    {
        auto new_buffer = static_cast<uint8_t *>(Platform::MemoryRealloc(mBuffer, (mElementCount + numElements) * mElementSize));
        VerifyOrReturnError(new_buffer != nullptr, CHIP_ERROR_NO_MEMORY);
        mBuffer = new_buffer;
    }
    else
    {
        // this is NOT an allocated buffer, but it should become one
        auto new_buffer = static_cast<uint8_t *>(Platform::MemoryCalloc(mElementCount + numElements, mElementSize));
        VerifyOrReturnError(new_buffer != nullptr, CHIP_ERROR_NO_MEMORY);
        mBufferIsAllocated = true;
        memcpy(new_buffer, mBuffer, mElementCount * mElementSize);
        mBuffer = new_buffer;
    }
    mCapacity = mElementCount + numElements;

    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericAppendOnlyBuffer::AppendSingleElementRaw(const void * buffer)
{
    VerifyOrReturnError(mElementCount < mCapacity, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(mBuffer + mElementCount * mElementSize, buffer, mElementSize);
    mElementCount++;
    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericAppendOnlyBuffer::AppendElementArrayRaw(const void * __restrict__ buffer, size_t numElements)
{
    ReturnErrorOnFailure(EnsureAppendCapacity(numElements));

    memcpy(mBuffer + mElementCount * mElementSize, buffer, numElements * mElementSize);
    mElementCount += numElements;

    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericAppendOnlyBuffer::ReferenceExistingElementArrayRaw(const void * buffer, size_t numElements)
{
    if (mBuffer == nullptr)
    {
        // we can NEVER append with 0 capacity, so const cast is safe
        mBuffer       = const_cast<uint8_t *>(static_cast<const uint8_t *>(buffer));
        mElementCount = numElements;
        // The assertions below are because we know the buffer is null/not allocated yet
        VerifyOrDie(mCapacity == 0);
        VerifyOrDie(!mBufferIsAllocated);
        return CHIP_NO_ERROR;
    }

    return AppendElementArrayRaw(buffer, numElements);
}

void GenericAppendOnlyBuffer::ReleaseBuffer(void *& buffer, size_t & size, bool & allocated)
{
    buffer    = mBuffer;
    size      = mElementCount;
    allocated = mBufferIsAllocated;

    // we release the ownership
    mBuffer            = nullptr;
    mCapacity          = 0;
    mElementCount      = 0;
    mBufferIsAllocated = false;
}

ScopedBuffer::~ScopedBuffer()
{
    if (mBuffer != nullptr)
    {
        Platform::MemoryFree(mBuffer);
    }
}

ScopedBuffer & ScopedBuffer::operator=(ScopedBuffer && other)
{
    if (mBuffer != nullptr)
    {
        Platform::MemoryFree(mBuffer);
    }

    mBuffer       = other.mBuffer;
    other.mBuffer = nullptr;
    return *this;
}

} // namespace detail
} // namespace DataModel
} // namespace app
} // namespace chip
