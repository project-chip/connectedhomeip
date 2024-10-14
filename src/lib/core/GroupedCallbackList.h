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

#pragma once

#include <lib/core/CHIPCallback.h>
#include <lib/support/CodeUtils.h>

#include <utility>

namespace chip {
namespace Callback {

namespace detail {
// Internal helper functions
template <size_t Index>
void TaggedDequeueGroup(Cancelable * cancelable);
void EnqueueWithGroup(Cancelable * cancelable, Cancelable *& group, Cancelable * anchor, void (*cancelFn)(Cancelable *));
void LinkGroup(Cancelable * prev, Cancelable * next);
inline Cancelable * ClearCancelable(Cancelable * cancelable);
} // namespace detail

/**
 * A GroupedCallbackList manages a list of Callback objects (see CHIPCallback.h).
 * The state of the list is maintained using the prev/next pointers of each Callback.
 * Unlike a normal linked list where entries are managed individually, this class
 * manages a number of related callbacks as a group, with the callback function types
 * given as template parameters.
 *
 * For example, a `GroupedCallbackList<SuccessFn, FailureFn>` manages groups of a
 * `Callback<SuccessFn>` and a `Callback<FailureFn>`.
 *
 * Groups of callbacks are enqueued and dequeued (or cancelled) as a unit.
 * Within a group each callback is optional (i.e. can be null), however attempting
 * to enqueue a group where all callbacks are null has no effect.
 */
template <typename... T>
class GroupedCallbackList : protected Cancelable
{
public:
    GroupedCallbackList() = default;
    ~GroupedCallbackList() { Clear(); }

    GroupedCallbackList(GroupedCallbackList const &)             = delete;
    GroupedCallbackList & operator=(GroupedCallbackList const &) = delete;

    bool IsEmpty() { return mNext == this; }

    /**
     * Enqueues the specified group of callbacks, any of which may be null.
     */
    void Enqueue(Callback<T> *... callback) { Enqueue(std::index_sequence_for<T...>{}, callback...); }

    /**
     * If the list is non-empty, populates the reference arguments with the first
     * group of callbacks and returns true. Returns false if the list is empty.
     */
    bool Peek(Callback<T> *&... callback) const { return Peek(std::index_sequence_for<T...>{}, callback...); }

    /**
     * Like Peek(), but additionally removes the first group of callbacks from the list.
     */
    bool Take(Callback<T> *&... callback)
    {
        VerifyOrReturnValue(Peek(callback...), false);
        mNext->Cancel();
        return true;
    }

    /**
     * Moves all elements of the source list into this list, leaving the source list empty.
     */
    void EnqueueTakeAll(GroupedCallbackList & source)
    {
        VerifyOrReturn(!source.IsEmpty() && this != &source);
        detail::LinkGroup(mPrev, source.mNext);
        source.mPrev->mNext = this;
        mPrev               = source.mPrev;

        source.mPrev = source.mNext = &source;
    }

    void Clear()
    {
        Cancelable * next = mNext;
        while (next != this)
        {
            next = detail::ClearCancelable(next);
        }
        mPrev = mNext = this;
    }

private:
    /*
     *  The grouped list structure is similar to a normal doubly linked list,
     *  with the list object itself (via inheriting from Cancelable) acting as
     *  an external "anchor" node that is both the head and tail of the list.
     *
     *  However we have the additional requirement of representing node grouping.
     *  Due to the requirement so support sparse groups (one or more callbacks may
     *  not be present in a particular group) we cannot rely on a fixed group size.
     *  This problem is solved by having the "prev" pointer for all nodes in a group
     *  point to the node before the group, as illustrated below:
     *
     *     |Anchor|    |Grp 1|    |====== Group 2 ======|
     *     _______________________________________________
     *    /                                               \
     *    \  +---+      +---+      +---+   +---+   +---+  /
     *     ->|###|----->|   |----->|   |-->|   |-->|   |--
     *       |###|      |   |      |   |   |   |   |   |
     *     --|###|<-----|   |<-----|   |  -|   |  -|   |<-
     *    /  +---+      +---+  \   +---+ / +---+ / +---+  \
     *   |                      \_______/       /          |
     *   |                       \_____________/           |
     *    \_______________________________________________/
     *
     *  This allows the start of a group to be reached from any group member via
     *  ->prev->next. Nodes in a group can be enumerated by via the "next" chain,
     *  inspecting the "prev" pointers to detect the end of the group. The price
     *  for encoding grouping in this way is that upon removal of a group we have
     *  to update not just the "prev" pointer of the following node, but of all
     *  nodes in the following group.
     *
     *  When retrieving a (sparse) group from the list, we also need to be able
     *  to tell which callbacks are present: In a grouped list with types (A, B)
     *  both (a, nullptr) and (nullptr, b) are by necessity represented by only
     *  a single node in the list. To be able to recover this information, we use
     *  distinct trampolines that tag the "cancel" function pointer stored in each
     *  node with the index of the callback type within the argument type tuple.
     */

    template <std::size_t... Index>
    void Enqueue(std::index_sequence<Index...>, Callback<T> *... callback)
    {
        Cancelable * group = nullptr;
        (
            [&] {
                VerifyOrReturn(callback != nullptr);
                detail::EnqueueWithGroup(callback->Cancel(), group, this, &detail::TaggedDequeueGroup<Index>);
            }(),
            ...);
    }

    template <std::size_t... Index>
    bool Peek(std::index_sequence<Index...>, Callback<T> *&... callback) const
    {
        Cancelable * cancelable = mNext;
        VerifyOrReturnValue(cancelable != this, false);
        Cancelable * groupPrev = cancelable->mPrev;
        (
            [&] {
                if (cancelable->mPrev == groupPrev && cancelable->mCancel == &detail::TaggedDequeueGroup<Index>)
                {
                    callback   = Callback<decltype(callback->mCall)>::FromCancelable(cancelable);
                    cancelable = cancelable->mNext;
                }
                else
                {
                    callback = nullptr;
                }
            }(),
            ...);
        return true;
    }
};

namespace detail {

// Inserts `cancelable` before `anchor`, either starting a new `group`
// (populating the passed pointer if it is null) or adding to it.
inline void EnqueueWithGroup(Cancelable * cancelable, Cancelable *& group, Cancelable * anchor, void (*cancelFn)(Cancelable *))
{
    cancelable->mCancel = cancelFn;
    cancelable->mNext   = anchor;
    if (!group)
    {
        group             = cancelable;
        cancelable->mPrev = anchor->mPrev;
    }
    else
    {
        cancelable->mPrev = group->mPrev;
    }
    anchor->mPrev->mNext = cancelable;
    anchor->mPrev        = cancelable;
}

// Establish prev/next links between `prev` and the group starting at `cancelable`.
inline void LinkGroup(Cancelable * prev, Cancelable * cancelable)
{
    prev->mNext = cancelable;

    Cancelable * groupPrev = cancelable->mPrev;
    do
    {
        cancelable->mPrev = prev;
        cancelable        = cancelable->mNext;
    } while (cancelable->mPrev == groupPrev);
}

// Clears the state of a cancelable and returns the following one.
// Does NOT touch the state of adjacent nodes.
inline Cancelable * ClearCancelable(Cancelable * cancelable)
{
    auto * next       = cancelable->mNext;
    cancelable->mPrev = cancelable->mNext = cancelable;
    cancelable->mCancel                   = nullptr;
    return next;
}

// Dequeues `cancelable` and all otehr nodes in the same group.
inline void DequeueGroup(Cancelable * cancelable)
{
    Cancelable * prev = cancelable->mPrev;
    Cancelable * next = prev->mNext;
    do
    {
        next = ClearCancelable(next);
    } while (next->mPrev == prev);
    LinkGroup(prev, next);
}

template <size_t Index>
void TaggedDequeueGroup(Cancelable * cancelable)
{
    (void) Index; // not used, we only care that instantiations have unique addresses
    DequeueGroup(cancelable);
}

} // namespace detail
} // namespace Callback
} // namespace chip
