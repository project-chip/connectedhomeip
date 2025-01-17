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

GenericMetadataList::~GenericMetadataList()
{
    Invalidate();
}

GenericMetadataList & GenericMetadataList::operator=(GenericMetadataList && other)
{
    if (this != &other)
    {
        // Generic metadata lists should not be used directly except for same-sized data
        VerifyOrDie(this->mElementSize == other.mElementSize);

        if (mAllocated && (mBuffer != nullptr))
        {
            chip::Platform::MemoryFree(mBuffer);
        }

        this->mAllocated    = other.mAllocated;
        this->mBuffer       = other.mBuffer;
        this->mElementCount = other.mElementCount;
        this->mCapacity     = other.mCapacity;
        this->mIsImmutable  = other.mIsImmutable;

        other.mAllocated    = false;
        other.mBuffer       = nullptr;
        other.mElementCount = 0;
        other.mCapacity     = 0;
        other.mIsImmutable  = true;
    }
    return *this;
}

const void * GenericMetadataList::operator[](size_t index) const
{
    VerifyOrDie(index < mElementCount);
    return mBuffer + index * mElementSize;
}

CHIP_ERROR GenericMetadataList::reserve(size_t numElements)
{
    VerifyOrReturnError(!mIsImmutable, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mElementCount == 0, CHIP_ERROR_INCORRECT_STATE);

    if ((mBuffer != nullptr) && mAllocated)
    {
        chip::Platform::MemoryFree(mBuffer);
    }

    mBuffer = static_cast<uint8_t *>(chip::Platform::MemoryCalloc(numElements, mElementSize));

    VerifyOrReturnError(mBuffer != nullptr, CHIP_ERROR_NO_MEMORY);

    mAllocated = true;
    mCapacity  = numElements;
    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericMetadataList::AppendRaw(const void * buffer)
{
    VerifyOrReturnError(!mIsImmutable, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mElementCount < mCapacity, CHIP_ERROR_NO_MEMORY);
    memcpy(mBuffer + mElementCount * mElementSize, buffer, mElementSize);
    mElementCount++;
    return CHIP_NO_ERROR;
}

void GenericMetadataList::Invalidate()
{
    if ((mBuffer != nullptr) && mAllocated)
    {
        chip::Platform::MemoryFree(mBuffer);
        mBuffer    = nullptr;
        mAllocated = false;
    }
    mCapacity     = 0;
    mElementCount = 0;
    mCapacity     = 0;
    mIsImmutable  = true;
}

CHIP_ERROR GenericMetadataList::CopyExistingBuffer(const void * buffer, size_t elements)
{
    ReturnErrorOnFailure(reserve(elements));
    memcpy(mBuffer, buffer, mElementSize * elements);
    mIsImmutable  = true;
    mElementCount = elements;
    mCapacity     = elements;
    return CHIP_NO_ERROR;
}

void GenericMetadataList::AcquireExistingBuffer(const void * buffer, size_t elements)
{
    Invalidate();
    mAllocated    = false;
    mElementCount = elements;
    mCapacity     = elements;
    mIsImmutable  = true;
    // NOTE: const cast, however we are marked as immutable and not allocated,
    // so will never perform any writes on mBuffer's contents or try to deallocate it.
    mBuffer = static_cast<uint8_t *>(const_cast<void *>(buffer));
}

} // namespace detail
} // namespace DataModel
} // namespace app
} // namespace chip
