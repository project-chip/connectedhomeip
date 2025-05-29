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

/**
 *    @file
 *      Defines scoped auto-free buffers for CHIP.
 *
 */

#pragma once

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

#include <type_traits>
#include <utility>

namespace chip {
namespace Platform {

namespace Impl {

/**
 * Represents a memory buffer that is auto-freed in the destructor.
 *
 * This class uses void* underneath on purpose (rather than a unique_ptr like
 * 'Type') and uses  templated type on Ptr(). This is to avoid template explosion
 * when the buffers are used for different types - only one implementation of
 * the class will be stored in flash.
 */
template <class Impl>
class ScopedMemoryBufferBase
{
public:
    ScopedMemoryBufferBase() {}
    ScopedMemoryBufferBase(const ScopedMemoryBufferBase &)                   = delete;
    ScopedMemoryBufferBase & operator=(const ScopedMemoryBufferBase & other) = delete;

    ScopedMemoryBufferBase(ScopedMemoryBufferBase && other) { *this = std::move(other); }

    ScopedMemoryBufferBase & operator=(ScopedMemoryBufferBase && other)
    {
        if (this != &other)
        {
            mBuffer       = other.mBuffer;
            other.mBuffer = nullptr;
        }
        return *this;
    }

    ~ScopedMemoryBufferBase() { Free(); }

    /** Check if a buffer is valid */
    explicit operator bool() const { return mBuffer != nullptr; }
    bool operator!() const { return mBuffer == nullptr; }

    /** Release memory used */
    void Free()
    {
        if (mBuffer == nullptr)
        {
            return;
        }
        Impl::MemoryFree(mBuffer);
        mBuffer = nullptr;
    }

protected:
    void * Ptr() { return mBuffer; }
    const void * Ptr() const { return mBuffer; }

    /**
     * Releases the underlying buffer.
     *
     * The buffer stops being managed and will not be auto-freed.
     */
    CHECK_RETURN_VALUE void * Release()
    {
        void * buffer = mBuffer;
        mBuffer       = nullptr;
        return buffer;
    }

    void Alloc(size_t size)
    {
        Free();
        mBuffer = Impl::MemoryAlloc(size);
    }

    void Calloc(size_t elementCount, size_t elementSize)
    {
        Free();
        mBuffer = Impl::MemoryCalloc(elementCount, elementSize);
    }

private:
    void * mBuffer = nullptr;
};

/**
 * Helper class that forwards memory management tasks to Platform::Memory* calls.
 */
class PlatformMemoryManagement
{
public:
    static void MemoryFree(void * p) { chip::Platform::MemoryFree(p); }
    static void * MemoryAlloc(size_t size) { return chip::Platform::MemoryAlloc(size); }
    static void * MemoryCalloc(size_t num, size_t size) { return chip::Platform::MemoryCalloc(num, size); }
};

} // namespace Impl

/**
 * Represents a memory buffer allocated using chip::Platform::Memory*Alloc
 * methods.
 *
 * Use for RAII to auto-free after use.
 */
template <typename T, class MemoryManagement = Impl::PlatformMemoryManagement>
class ScopedMemoryBuffer : public Impl::ScopedMemoryBufferBase<MemoryManagement>
{
    friend class Impl::ScopedMemoryBufferBase<MemoryManagement>;

public:
    using Base = Impl::ScopedMemoryBufferBase<MemoryManagement>;

    static_assert(std::is_trivially_destructible<T>::value, "Destructors won't get run");

    T * Get() { return static_cast<T *>(Base::Ptr()); }
    T & operator[](size_t index) { return Get()[index]; }

    const T * Get() const { return static_cast<const T *>(Base::Ptr()); }
    const T & operator[](size_t index) const { return Get()[index]; }

    /**
     * Releases the underlying buffer.
     *
     * The buffer stops being managed and will not be auto-freed.
     */
    CHECK_RETURN_VALUE T * Release() { return static_cast<T *>(Base::Release()); }

    ScopedMemoryBuffer & Calloc(size_t elementCount)
    {
        Base::Calloc(elementCount, sizeof(T));
        ExecuteConstructors(elementCount);
        return *this;
    }

    ScopedMemoryBuffer & Alloc(size_t elementCount)
    {
        Base::Alloc(elementCount * sizeof(T));
        ExecuteConstructors(elementCount);
        return *this;
    }

private:
    template <typename U = T, std::enable_if_t<std::is_trivial<U>::value, int> = 0>
    void ExecuteConstructors(size_t elementCount)
    {
        // Do nothing if our type is trivial.  In particular, if we are a buffer
        // of integers, we should not go zero-initializing them here: either
        // caller wants that and called Calloc(), or it doesn't and we shouldn't
        // do it.
    }

    template <typename U = T, std::enable_if_t<!std::is_trivial<U>::value, int> = 0>
    void ExecuteConstructors(size_t elementCount)
    {
        T * elementPtr = Get();
        if (elementPtr == nullptr)
        {
            // Alloc failed, don't bother.
            return;
        }
        for (size_t i = 0; i < elementCount; ++i)
        {
            new (&elementPtr[i]) T();
        }
    }
};

/**
 * Represents a memory buffer with buffer size allocated using chip::Platform::Memory*Alloc
 * methods.
 *
 * Use for RAII to auto-free after use.
 */
template <typename T>
class ScopedMemoryBufferWithSize : public ScopedMemoryBuffer<T>
{
public:
    ScopedMemoryBufferWithSize() {}
    ScopedMemoryBufferWithSize(ScopedMemoryBufferWithSize && other) { *this = std::move(other); }

    ScopedMemoryBufferWithSize & operator=(ScopedMemoryBufferWithSize && other)
    {
        if (this != &other)
        {
            mCount       = other.mCount;
            other.mCount = 0;
        }
        ScopedMemoryBuffer<T>::operator=(std::move(other));
        return *this;
    }

    // return the size as count of elements
    inline size_t AllocatedSize() const { return mCount; }

    chip::Span<T> Span() { return chip::Span<T>(this->Get(), AllocatedSize()); }
    chip::Span<const T> Span() const { return chip::Span<T>(this->Get(), AllocatedSize()); }

    void Free()
    {
        mCount = 0;
        ScopedMemoryBuffer<T>::Free();
    }

    /**
     * Releases the underlying buffer.
     *
     * The buffer stops being managed and will not be auto-freed.
     */
    CHECK_RETURN_VALUE T * Release()
    {
        T * buffer = ScopedMemoryBuffer<T>::Release();
        mCount     = 0;
        return buffer;
    }

    ScopedMemoryBufferWithSize & Calloc(size_t elementCount)
    {
        ScopedMemoryBuffer<T>::Calloc(elementCount);
        if (this->Get() != nullptr)
        {
            mCount = elementCount;
        }
        return *this;
    }

    ScopedMemoryBufferWithSize & Alloc(size_t elementCount)
    {
        ScopedMemoryBuffer<T>::Alloc(elementCount);
        if (this->Get() != nullptr)
        {
            mCount = elementCount;
        }
        return *this;
    }

private:
    size_t mCount = 0;
};

} // namespace Platform
} // namespace chip
