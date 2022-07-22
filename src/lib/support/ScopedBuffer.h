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
    ScopedMemoryBufferBase(const ScopedMemoryBufferBase &) = delete;
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
     * Releases the undelying buffer. Buffer stops being managed and will not be
     * auto-freed.
     */
    void * Release()
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

    static_assert(std::is_trivial<T>::value, "Constructors won't get run");

    inline T * Get() { return static_cast<T *>(Base::Ptr()); }
    inline T & operator[](size_t index) { return Get()[index]; }

    inline const T * Get() const { return static_cast<const T *>(Base::Ptr()); }
    inline const T & operator[](size_t index) const { return Get()[index]; }

    inline T * Release() { return static_cast<T *>(Base::Release()); }

    ScopedMemoryBuffer & Calloc(size_t elementCount)
    {
        Base::Calloc(elementCount, sizeof(T));
        return *this;
    }

    ScopedMemoryBuffer & Alloc(size_t size)
    {
        Base::Alloc(size * sizeof(T));
        return *this;
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
            mSize       = other.mSize;
            other.mSize = 0;
        }
        ScopedMemoryBuffer<T>::operator=(std::move(other));
        return *this;
    }

    ~ScopedMemoryBufferWithSize() { mSize = 0; }

    // return the size in bytes
    inline size_t AllocatedSize() const { return mSize; }

    void Free()
    {
        mSize = 0;
        ScopedMemoryBuffer<T>::Free();
    }

    T * Release()
    {
        T * buffer = ScopedMemoryBuffer<T>::Release();
        mSize      = 0;
        return buffer;
    }

    ScopedMemoryBufferWithSize & Calloc(size_t elementCount)
    {
        ScopedMemoryBuffer<T>::Calloc(elementCount);
        if (this->Get() != nullptr)
        {
            mSize = elementCount * sizeof(T);
        }
        return *this;
    }

    ScopedMemoryBufferWithSize & Alloc(size_t size)
    {
        ScopedMemoryBuffer<T>::Alloc(size);
        if (this->Get() != nullptr)
        {
            mSize = size * sizeof(T);
        }
        return *this;
    }

private:
    size_t mSize = 0;
};

} // namespace Platform
} // namespace chip
