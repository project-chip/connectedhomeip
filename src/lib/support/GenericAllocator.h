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

/**
 *    @file
 *        Dual static / dynamic allocator. This allocator can be configured
 *        to use either a static pool of size N or dynamic allocation
 *        if N is set to 0. This class can be used to manage non-reference-counted
 *        pools (ex. for private data members) where some nodes may wish to use
 *        static allocation of variable sizes and others may wish to use dynamic.
 *
 *        To use this class, declare the class with the desired pool size, then
 *        use allocate / free / etc. as required. For example, to use an allocator
 *        for ints
 *        #ifdef USE_STATIC
 *        #define POOL_SIZE 5
 *        #else
 *        #define POOL_SIZE 0
 *        GenericAllocator<int, POOL_SIZE> allocator;
 *        int* ptr = allocator.Allocate();
 *        *ptr = 3;
 *        ptr = allocator.Allocate();
 *        *ptr = 2;
 *        for (auto & a: allocator) {
 *            printf("ranged for: %d\n", a);
 *        }
 *        for (auto it = allocator.begin(); it != allocator.end();++it)
 *        {
 *            printf("iterator: %d\n", *it);
 *        }
 *        for (auto it = allocator.begin(); it != allocator.end();)
 *        {
 *            it = allocator.Free(it);
 *        }
 *
 */

#pragma once

#include <lib/support/CHIPMem.h>
#include <list>
#include <optional>
#include <stdint.h>
#include <stdio.h>

namespace chip {
namespace Internal {

template <typename T>
struct ListWrapper
{
    T data;
    ListWrapper<T> * next = nullptr;
};
template <typename T>
class AllocatorDataBase
{
public:
    virtual ~AllocatorDataBase() = default;
    virtual T * Allocate()       = 0;
    virtual void Free(T * data)  = 0;
    virtual size_t GetN() const  = 0;
    virtual ListWrapper<T> * GetHead() { return nullptr; }
    virtual T * GetArray() { return nullptr; }
    virtual bool * GetInUse() { return nullptr; }
};

template <typename T, size_t N>
class AllocatorData : public AllocatorDataBase<T>
{
public:
    AllocatorData()
    {
        for (auto & inUse : mInUse)
        {
            inUse = false;
        }
    }
    T * Allocate() override
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (!mInUse[i])
            {
                mInUse[i] = true;
                return &mData[i];
            }
        }
        return nullptr;
    }
    void Free(T * data) override
    {
        for (size_t i = 0; i < N; ++i)
        {
            {
                if (&mData[i] == data && mInUse[i])
                {
                    mInUse[i] = false;
                    return;
                }
            }
        }
    }
    size_t GetN() const override { return N; }
    T * GetArray() override { return mData; }
    bool * GetInUse() override { return mInUse; }

private:
    T mData[N];
    bool mInUse[N];
};

template <typename T>
class AllocatorData<T, 0> : public AllocatorDataBase<T>
{
public:
    ~AllocatorData()
    {
        while (head != nullptr)
        {
            Free(&head->data);
        }
    }
    T * Allocate() override
    {
        auto * newLW = Platform::New<ListWrapper<T>>();
        if (newLW != nullptr)
        {
            if (head == nullptr)
            {
                head = newLW;
            }
            else
            {
                tail->next = newLW;
            }
            tail = newLW;
        }
        return &newLW->data;
    }
    void Free(T * data) override
    {
        ListWrapper<T> * prev = nullptr;
        ListWrapper<T> * curr = head;
        while (curr != nullptr)
        {
            if (data == &curr->data)
            {
                if (curr == head)
                {
                    head = curr->next;
                }
                else
                {
                    prev->next = curr->next;
                    if (curr == tail)
                    {
                        tail = prev;
                    }
                }
                Platform::MemoryFree(curr);
                return;
            }
            prev = curr;
            curr = curr->next;
        }
    }
    size_t GetN() const override { return 0; }
    ListWrapper<T> * GetHead() override { return head; }

private:
    ListWrapper<T> * head = nullptr;
    ListWrapper<T> * tail = nullptr;
};

} // namespace Internal

template <typename T, size_t N>
class GenericAllocator;

template <typename T>
class GenericAllocatorIterator
{
public:
    GenericAllocatorIterator(Internal::AllocatorDataBase<T> * data) : mData(data)
    {
        mIdx    = 0;
        listPtr = mData->GetHead();
        if (mData->GetN() != 0 && mData->GetInUse() != nullptr)
        {
            while (mIdx < mData->GetN() && mData->GetInUse()[mIdx] == false)
            {
                ++mIdx;
            }
        }
    }
    GenericAllocatorIterator<T> & SetEnd()
    {
        mIdx    = mData->GetN();
        listPtr = nullptr;
        return *this;
    }
    bool IsEnd() { return listPtr == nullptr && mIdx == mData->GetN(); }

    GenericAllocatorIterator<T> & operator++()
    {
        if (listPtr != nullptr)
        {
            listPtr = listPtr->next;
        }
        if (mData->GetInUse() != nullptr)
        {
            mIdx++;
            while (mIdx < mData->GetN() && mData->GetInUse()[mIdx] == false)
            {
                ++mIdx;
            }
        }
        return *this;
    }

    bool operator==(const GenericAllocatorIterator<T> & rhs) const
    {
        return rhs.mData == mData && rhs.mIdx == mIdx && rhs.listPtr == listPtr;
    }
    bool operator!=(const GenericAllocatorIterator<T> & rhs) const { return !(*this == rhs); }
    T & operator*()
    {
        if (listPtr != nullptr)
        {
            return listPtr->data;
        }
        else
        {
            return mData->GetArray()[mIdx];
        }
    }
    T * operator->()
    {
        if (listPtr != nullptr)
        {
            return &listPtr->data;
        }
        else
        {
            return &(mData->GetArray()[mIdx]);
        }
    }

    GenericAllocatorIterator<T> & Free()
    {
        if (listPtr != nullptr)
        {
            Internal::ListWrapper<T> * temp = listPtr;
            listPtr                         = listPtr->next;
            mData->Free(&temp->data);
        }
        else if (mData->GetInUse() != nullptr && mIdx < mData->GetN())
        {
            mData->Free(&(mData->GetArray()[mIdx]));
            operator++();
        }
        return *this;
    }

private:
    Internal::AllocatorDataBase<T> * mData;
    size_t mIdx                        = 0;
    Internal::ListWrapper<T> * listPtr = nullptr;
};

template <typename T>
class GenericAllocatorBase
{
public:
    GenericAllocatorBase(Internal::AllocatorDataBase<T> * data) : mData(data) {}
    virtual ~GenericAllocatorBase()                                              = default;
    virtual bool IsDynamic()                                                     = 0;
    virtual T * Allocate()                                                       = 0;
    virtual GenericAllocatorIterator<T> & Free(GenericAllocatorIterator<T> & it) = 0;
    virtual void Free(T * data)                                                  = 0;
    virtual GenericAllocatorIterator<T> begin()                                  = 0;
    virtual GenericAllocatorIterator<T> end()                                    = 0;

private:
    Internal::AllocatorDataBase<T> * mData;
};

/**
 * Allocates from either a static pool or from the heap depending on configuration.
 *
 * @tparam T is the class that is being allocated
 * @tparam N is the size of the pool, or 0 for dynamic allocation from the heap.
 */
template <typename T, size_t N>
class GenericAllocator : public GenericAllocatorBase<T>
{
public:
    GenericAllocator() : GenericAllocatorBase<T>(&mData) {}
    ~GenericAllocator() {}
    bool IsDynamic() override { return N == 0; }
    T * Allocate() override { return mData.Allocate(); }
    GenericAllocatorIterator<T> & Free(GenericAllocatorIterator<T> & it) override { return it.Free(); }
    void Free(T * data) override { mData.Free(data); }
    GenericAllocatorIterator<T> begin() override { return GenericAllocatorIterator<T>(&mData); }
    GenericAllocatorIterator<T> end() override { return GenericAllocatorIterator<T>(&mData).SetEnd(); }

private:
    Internal::AllocatorData<T, N> mData;
};
} // namespace chip
