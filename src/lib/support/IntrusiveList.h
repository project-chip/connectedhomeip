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

enum class IntrusiveMode
{
    // Strict mode features
    //
    //   Node constructor puts the hook in a well-known default state.
    //
    //   Node destructor checks if the hook is in the well-known default state.
    //   If not, an assertion is raised.
    //
    //   Every time an object is inserted in the intrusive container, the
    //   container checks if the hook is in the well-known default state. If
    //   not, an assertion is raised.
    //
    //   Every time an object is being erased from the intrusive container, the
    //   container puts the erased object in the well-known default state.
    //
    // Note: The strict mode works similar as safe mode for Boost.Intrusive
    //
    Strict,

    // Auto-unlink mode features
    //
    //   When the destructor of the node is called, the hook checks if the node
    //   is inserted in a container. If so, the hook removes the node from the
    //   container.
    //
    //   The hook has a member function called unlink() that can be used to
    //   unlink the node from the container at any time, without having any
    //   reference to the container, if the user wants to do so.
    //
    // This auto-unlink feature is useful in certain applications but it must
    // be used very carefully:
    //
    //   If several threads are using the same container the destructor of the
    //   auto-unlink hook will be called without any thread synchronization so
    //   removing the object is thread-unsafe. Container contents change
    //   silently without modifying the container directly. This can lead to
    //   surprising effects.
    //
    // These auto-unlink hooks have also strict-mode properties:
    //
    //   Node constructors put the hook in a well-known default state.
    //
    //   Every time an object is inserted in the intrusive container, the
    //   container checks if the hook is in the well-known default state. If
    //   not, an assertion is raised.
    //
    //   Every time an object is erased from an intrusive container, the
    //   container puts the erased object in the well-known default state.
    //
    // Note: The strict mode works similar as auto-unlink mode for
    // Boost.Intrusive
    //
    AutoUnlink,
};

class IntrusiveListNodePrivateBase
{
public:
    IntrusiveListNodePrivateBase() : mPrev(nullptr), mNext(nullptr) {}
    ~IntrusiveListNodePrivateBase() { VerifyOrDie(!IsInList()); }

    // Note: The copy construct/assignment is not provided because the list node state is not copyable.
    //       The move construct/assignment is not provided because all modifications to the list shall go through the list object.
    IntrusiveListNodePrivateBase(const IntrusiveListNodePrivateBase &) = delete;
    IntrusiveListNodePrivateBase & operator=(const IntrusiveListNodePrivateBase &) = delete;
    IntrusiveListNodePrivateBase(IntrusiveListNodePrivateBase &&)                  = delete;
    IntrusiveListNodePrivateBase & operator=(IntrusiveListNodePrivateBase &&) = delete;

    bool IsInList() const { return (mPrev != nullptr && mNext != nullptr); }

private:
    friend class IntrusiveListBase;
    IntrusiveListNodePrivateBase(IntrusiveListNodePrivateBase * prev, IntrusiveListNodePrivateBase * next) :
        mPrev(prev), mNext(next)
    {}

    void TakePlaceOf(const IntrusiveListNodePrivateBase * that)
    {
        // prerequisite `that` is in a list
        // `this` will take place of the position of `that`.
        // `that` will be emptied by the caller after this function
        mPrev        = that->mPrev;
        mNext        = that->mNext;
        mPrev->mNext = this;
        mNext->mPrev = this;
    }

    void Prepend(IntrusiveListNodePrivateBase * node)
    {
        VerifyOrDie(IsInList());
        VerifyOrDie(!node->IsInList());
        node->mPrev  = mPrev;
        node->mNext  = this;
        mPrev->mNext = node;
        mPrev        = node;
    }

    void Append(IntrusiveListNodePrivateBase * node)
    {
        VerifyOrDie(IsInList());
        VerifyOrDie(!node->IsInList());
        node->mPrev  = this;
        node->mNext  = mNext;
        mNext->mPrev = node;
        mNext        = node;
    }

protected:
    void Remove()
    {
        VerifyOrDie(IsInList());
        mPrev->mNext = mNext;
        mNext->mPrev = mPrev;
        mPrev        = nullptr;
        mNext        = nullptr;
    }

private:
    IntrusiveListNodePrivateBase * mPrev;
    IntrusiveListNodePrivateBase * mNext;
};

// Strict mode implementation
template <IntrusiveMode Mode = IntrusiveMode::Strict>
class IntrusiveListNodeBase : public IntrusiveListNodePrivateBase
{
};

// Partial specialization implementation for auto-unlink mode.
template <>
class IntrusiveListNodeBase<IntrusiveMode::AutoUnlink> : public IntrusiveListNodePrivateBase
{
public:
    ~IntrusiveListNodeBase() { Unlink(); }
    void Unlink()
    {
        if (IsInList())
            Remove();
    }
};

// Non-template part of IntrusiveList. It appears that for list structure, both mode can share same implementation.
class IntrusiveListBase
{
public:
    class ConstIteratorBase
    {
    private:
        friend class IntrusiveListBase;
        ConstIteratorBase(const IntrusiveListNodePrivateBase * cur) : mCurrent(cur) {}

        const IntrusiveListNodePrivateBase & operator*() { return *mCurrent; }

    public:
        using difference_type   = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        ConstIteratorBase(const ConstIteratorBase &) = default;
        ConstIteratorBase(ConstIteratorBase &&)      = default;
        ConstIteratorBase & operator=(const ConstIteratorBase &) = default;
        ConstIteratorBase & operator=(ConstIteratorBase &&) = default;

        bool operator==(const ConstIteratorBase & that) const { return mCurrent == that.mCurrent; }
        bool operator!=(const ConstIteratorBase & that) const { return !(*this == that); }

        ConstIteratorBase & operator++()
        {
            mCurrent = mCurrent->mNext;
            return *this;
        }

        ConstIteratorBase operator++(int)
        {
            ConstIteratorBase res(mCurrent);
            mCurrent = mCurrent->mNext;
            return res;
        }

        ConstIteratorBase & operator--()
        {
            mCurrent = mCurrent->mPrev;
            return *this;
        }

        ConstIteratorBase operator--(int)
        {
            ConstIteratorBase res(mCurrent);
            mCurrent = mCurrent->mPrev;
            return res;
        }

    protected:
        const IntrusiveListNodePrivateBase * mCurrent;
    };

    class IteratorBase
    {
    private:
        friend class IntrusiveListBase;
        IteratorBase(IntrusiveListNodePrivateBase * cur) : mCurrent(cur) {}

        IntrusiveListNodePrivateBase & operator*() { return *mCurrent; }

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
        IntrusiveListNodePrivateBase * mCurrent;
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
    ~IntrusiveListBase()
    {
        VerifyOrDie(Empty());
        /* clear mNode such that the destructor checking mNode.IsInList doesn't fail */
        mNode.Remove();
    }

    IntrusiveListBase(const IntrusiveListBase &) = delete;
    IntrusiveListBase & operator=(const IntrusiveListBase &) = delete;

    IntrusiveListBase(IntrusiveListBase && that) : mNode(&mNode, &mNode) { *this = std::move(that); }

    IntrusiveListBase & operator=(IntrusiveListBase && that)
    {
        VerifyOrDie(Empty());
        if (!that.Empty())
        {
            mNode.TakePlaceOf(&that.mNode);
            that.mNode.mNext = &that.mNode;
            that.mNode.mPrev = &that.mNode;
        }
        else
        {
            // Do nothing here if that is empty, because there is a prerequisite that `this` is empty.
        }
        return *this;
    }

    ConstIteratorBase begin() const { return ConstIteratorBase(mNode.mNext); }
    ConstIteratorBase end() const { return ConstIteratorBase(&mNode); }
    IteratorBase begin() { return IteratorBase(mNode.mNext); }
    IteratorBase end() { return IteratorBase(&mNode); }

    void PushFront(IntrusiveListNodePrivateBase * node) { mNode.Append(node); }
    void PushBack(IntrusiveListNodePrivateBase * node) { mNode.Prepend(node); }

    void InsertBefore(IteratorBase pos, IntrusiveListNodePrivateBase * node) { pos.mCurrent->Prepend(node); }
    void InsertAfter(IteratorBase pos, IntrusiveListNodePrivateBase * node) { pos.mCurrent->Append(node); }
    void Remove(IntrusiveListNodePrivateBase * node) { node->Remove(); }

    /// @brief Replace an original node in list with a new node.
    void Replace(IntrusiveListNodePrivateBase * original, IntrusiveListNodePrivateBase * replacement)
    {
        // VerifyOrDie(Contains(original)); This check is too heavy to do, but it shall hold
        VerifyOrDie(!replacement->IsInList());
        replacement->TakePlaceOf(original);
        original->mPrev = nullptr;
        original->mNext = nullptr;
    }

    bool Contains(const IntrusiveListNodePrivateBase * node) const
    {
        for (auto & iter : *this)
        {
            if (&iter == node)
                return true;
        }
        return false;
    }

private:
    IntrusiveListNodePrivateBase mNode;
};

/// The hook converts between node object T and IntrusiveListNodeBase
///
/// When using this hook, the node type (T) MUST inherit from IntrusiveListNodeBase.
///
template <typename T, IntrusiveMode Mode>
class IntrusiveListBaseHook
{
public:
    static_assert(std::is_base_of<IntrusiveListNodeBase<Mode>, T>::value, "T must be derived from IntrusiveListNodeBase");

    static T * ToObject(IntrusiveListNodePrivateBase * node) { return static_cast<T *>(node); }
    static const T * ToObject(const IntrusiveListNodePrivateBase * node) { return static_cast<T *>(node); }

    static T * ToObject(IntrusiveListNodeBase<Mode> * node) { return static_cast<T *>(node); }
    static const T * ToObject(const IntrusiveListNodeBase<Mode> * node) { return static_cast<T *>(node); }

    static IntrusiveListNodeBase<Mode> * ToNode(T * object) { return static_cast<IntrusiveListNodeBase<Mode> *>(object); }
    static const IntrusiveListNodeBase<Mode> * ToNode(const T * object)
    {
        return static_cast<const IntrusiveListNodeBase<Mode> *>(object);
    }
};

/// A double-linked list where the data is stored together with the previous/next pointers for cache efficiency / and compactness.
///
/// The default hook (IntrusiveListBaseHook<T>) requires T to inherit from IntrusiveListNodeBase.
///
/// Consumers must ensure that the IntrusiveListNodeBase object associated with
/// a node is removed from any list it might belong to before it is destroyed.
///
/// Consumers must ensure that a list is empty before it is destroyed.
///
/// A node may only belong to a single list. The code will assert (via VerifyOrDie) on this invariant.
///
/// Example usage:
///
///     class ListNode : public IntrusiveListNodeBase  {};
///     // ...
///     ListNode a,b,c;
///     IntrusiveList<ListNode> list;    // NOTE: node lifetime >= list lifetime
///
///     list.PushBack(&a);
///     list.PushFront(&b);
///     assert(list.Contains(&a)  && list.Contains(&b) && !list.Contains(&c));
///     list.Remove(&a);
///     list.Remove(&b);
template <typename T, IntrusiveMode Mode = IntrusiveMode::Strict, typename Hook = IntrusiveListBaseHook<T, Mode>>
class IntrusiveList : public IntrusiveListBase
{
public:
    IntrusiveList() : IntrusiveListBase() {}

    IntrusiveList(IntrusiveList &&) = default;
    IntrusiveList & operator=(IntrusiveList &&) = default;

    class ConstIterator : public IntrusiveListBase::ConstIteratorBase
    {
    public:
        using value_type = const T;
        using pointer    = const T *;
        using reference  = const T &;

        ConstIterator(IntrusiveListBase::ConstIteratorBase && base) : IntrusiveListBase::ConstIteratorBase(std::move(base)) {}
        const T * operator->() { return Hook::ToObject(mCurrent); }
        const T & operator*() { return *Hook::ToObject(mCurrent); }
    };

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

    ConstIterator begin() const { return IntrusiveListBase::begin(); }
    ConstIterator end() const { return IntrusiveListBase::end(); }
    Iterator begin() { return IntrusiveListBase::begin(); }
    Iterator end() { return IntrusiveListBase::end(); }
    void PushFront(T * value) { IntrusiveListBase::PushFront(Hook::ToNode(value)); }
    void PushBack(T * value) { IntrusiveListBase::PushBack(Hook::ToNode(value)); }
    void InsertBefore(Iterator pos, T * value) { IntrusiveListBase::InsertBefore(pos, Hook::ToNode(value)); }
    void InsertAfter(Iterator pos, T * value) { IntrusiveListBase::InsertAfter(pos, Hook::ToNode(value)); }
    void Remove(T * value) { IntrusiveListBase::Remove(Hook::ToNode(value)); }
    void Replace(T * original, T * replacement) { IntrusiveListBase::Replace(Hook::ToNode(original), Hook::ToNode(replacement)); }
    bool Contains(const T * value) const { return IntrusiveListBase::Contains(Hook::ToNode(value)); }
};

} // namespace chip
