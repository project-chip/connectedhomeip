/*
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

#pragma once

#include <iterator>
#include <utility>

#include <lib/support/CodeUtils.h>

namespace chip {

class IntrusiveListBase;

class IntrusiveListNodeBase
{
public:
    IntrusiveListNodeBase() : mPrev(nullptr), mNext(nullptr) {}
    ~IntrusiveListNodeBase() { VerifyOrDie(!IsInList()); }

    bool IsInList() const { return (mPrev != nullptr && mNext != nullptr); }

private:
    friend class IntrusiveListBase;
    IntrusiveListNodeBase(IntrusiveListNodeBase * prev, IntrusiveListNodeBase * next) : mPrev(prev), mNext(next) {}

    void Prepend(IntrusiveListNodeBase * node)
    {
        VerifyOrDie(IsInList());
        VerifyOrDie(!node->IsInList());
        node->mPrev  = mPrev;
        node->mNext  = this;
        mPrev->mNext = node;
        mPrev        = node;
    }

    void Append(IntrusiveListNodeBase * node)
    {
        VerifyOrDie(IsInList());
        VerifyOrDie(!node->IsInList());
        node->mPrev  = this;
        node->mNext  = mNext;
        mNext->mPrev = node;
        mNext        = node;
    }

    void Remove()
    {
        VerifyOrDie(IsInList());
        mPrev->mNext = mNext;
        mNext->mPrev = mPrev;
        mPrev        = nullptr;
        mNext        = nullptr;
    }

    IntrusiveListNodeBase * mPrev;
    IntrusiveListNodeBase * mNext;
};

// non template part of IntrusiveList
class IntrusiveListBase
{
public:
    class IteratorBase
    {
    private:
        friend class IntrusiveListBase;
        IteratorBase(IntrusiveListNodeBase * cur) : mCurrent(cur) {}

        IntrusiveListNodeBase & operator*() { return *mCurrent; }

    public:
        using difference_type   = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        IteratorBase(const IteratorBase &) = default;
        IteratorBase(IteratorBase &&)      = default;
        IteratorBase & operator=(const IteratorBase &) = default;
        IteratorBase & operator=(IteratorBase &&) = default;

        bool operator==(const IteratorBase & that) const { return mCurrent == that.mCurrent; }
        bool operator!=(const IteratorBase & that) const { return !(*this == that); }

        IteratorBase & operator++()
        {
            mCurrent = mCurrent->mNext;
            return *this;
        }

        IteratorBase operator++(int)
        {
            IteratorBase res(mCurrent);
            mCurrent = mCurrent->mNext;
            return res;
        }

        IteratorBase & operator--()
        {
            mCurrent = mCurrent->mPrev;
            return *this;
        }

        IteratorBase operator--(int)
        {
            IteratorBase res(mCurrent);
            mCurrent = mCurrent->mPrev;
            return res;
        }

    protected:
        IntrusiveListNodeBase * mCurrent;
    };

    bool Empty() const { return mNode.mNext == &mNode; }

    void Erase(IteratorBase pos) { pos.mCurrent->Remove(); }

protected:
    // The list is formed as a ring with mNode being the end.
    //
    // begin                                        end
    //   v                                           v
    // item(first) -> item -> ... -> item(last) -> mNode
    //   ^                                           |
    //    \------------------------------------------/
    //
    IntrusiveListBase() : mNode(&mNode, &mNode) {}
    ~IntrusiveListBase() { mNode.Remove(); /* clear mNode such that the destructor checking mNode.IsInList doesn't fail */ }

    IteratorBase begin() { return IteratorBase(mNode.mNext); }
    IteratorBase end() { return IteratorBase(&mNode); }

    void PushFront(IntrusiveListNodeBase * node) { mNode.Append(node); }
    void PushBack(IntrusiveListNodeBase * node) { mNode.Prepend(node); }

    void InsertBefore(IteratorBase pos, IntrusiveListNodeBase * node) { pos.mCurrent->Prepend(node); }
    void InsertAfter(IteratorBase pos, IntrusiveListNodeBase * node) { pos.mCurrent->Append(node); }
    void Remove(IntrusiveListNodeBase * node) { node->Remove(); }

    bool Contains(IntrusiveListNodeBase * node)
    {
        for (auto & iter : *this)
        {
            if (&iter == node)
                return true;
        }
        return false;
    }

private:
    IntrusiveListNodeBase mNode;
};

template <typename T>
class IntrusiveListBaseHook
{
public:
    static_assert(std::is_base_of<IntrusiveListNodeBase, T>::value, "T must be derived from IntrusiveListNodeBase");
    static T * ToObject(IntrusiveListNodeBase * node) { return static_cast<T *>(node); }
    static IntrusiveListNodeBase * ToNode(T * object) { return static_cast<IntrusiveListNodeBase *>(object); }
};

/**
 * @brief An intrusive double linked list.
 *
 * @tparam T    Type of element in the list.
 * @tparam Hook A hook to convert between object T and the IntrusiveListNodeBase
 */
template <typename T, typename Hook = IntrusiveListBaseHook<T>>
class IntrusiveList : public IntrusiveListBase
{
public:
    static_assert(std::is_base_of<IntrusiveListNodeBase, T>::value, "T must derive from IntrusiveListNodeBase");

    IntrusiveList() : IntrusiveListBase() {}

    class Iterator : public IntrusiveListBase::IteratorBase
    {
    public:
        using value_type = T;
        using pointer    = T *;
        using reference  = T &;

        Iterator(IntrusiveListBase::IteratorBase && base) : IntrusiveListBase::IteratorBase(std::move(base)) {}
        T * operator->() { return Hook::ToObject(mCurrent); }
        T & operator*() { return *Hook::ToObject(mCurrent); }
    };

    Iterator begin() { return IntrusiveListBase::begin(); }
    Iterator end() { return IntrusiveListBase::end(); }
    void PushFront(T * value) { IntrusiveListBase::PushFront(Hook::ToNode(value)); }
    void PushBack(T * value) { IntrusiveListBase::PushBack(Hook::ToNode(value)); }
    void InsertBefore(Iterator pos, T * value) { IntrusiveListBase::InsertBefore(pos, Hook::ToNode(value)); }
    void InsertAfter(Iterator pos, T * value) { IntrusiveListBase::InsertAfter(pos, Hook::ToNode(value)); }
    void Remove(T * value) { IntrusiveListBase::Remove(Hook::ToNode(value)); }
    bool Contains(T * value) { return IntrusiveListBase::Contains(Hook::ToNode(value)); }
};

} // namespace chip
