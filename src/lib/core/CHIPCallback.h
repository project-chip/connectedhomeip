/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *  @file
 *    This file contains definitions for Callback objects for registering with
 *     Clusters and the Device
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

namespace chip {

namespace Callback {

/**
 *  @class Cancelable
 *
 *    Private members of a Callback for use by subsystems that accept
 *     Callbacks for event registration/notification.
 *
 */
class Cancelable
{
    typedef void (*CancelFn)(Cancelable *);

public:
    /**
     *  @brief for use by Callback callees, i.e. those that accept callbacks for
     *   event registration.  The names suggest how to use these members, but
     *   implementations can choose.
     */
    Cancelable * mNext;
    Cancelable * mPrev;
    alignas(uint64_t) char mInfo[24];

    /**
     * @brief when non-null, indicates the Callback is registered with
     *   a subsystem and that Cancelable members belong to
     *   that subsystem
     */
    CancelFn mCancel;

    Cancelable()
    {
        mNext = mPrev = this;
        mCancel       = nullptr;
    }

    /**
     * @brief run whatever function the callee/registrar has specified in order
     *  to clean up any resource allocation associated with the registration,
     *  and surrender ownership of the Cancelable's fields
     */
    Cancelable * Cancel()
    {
        if (mCancel != nullptr)
        {
            CancelFn cancel = mCancel;
            mCancel         = nullptr;
            cancel(this);
        }
        return this;
    }
    ~Cancelable() { Cancel(); }

    Cancelable(const Cancelable &) = delete;
};

typedef void (*CallFn)(void *);

/**
 *  @class Callback
 *
 *    Base struct used for registration of items of interest, includes
 *     memory for list management and storing information about the registration's
 *     meaning.  Callback also defines cancellation.
 *    Callbacks can be registered with exactly one callee at a time. While
 *     registered (as indicated by a non-null mCancel function), all fields of
 *     the Callback save usercontext are "owned" by the callee, and should not
 *     be touched unless Cancel() has first been called.
 *    When a callee accepts a Callback for registration, step one is always Cancel(),
 *     in order to take ownership of Cancelable members next, prev, info_ptr, and info_scalar.
 *    This template class also defines a default notification function prototype.
 *
 *   One-shot semantics can be accomplished by calling Cancel() before calling mCall.
 *     Persistent registration semantics would skip that.
 *
 *   There is no provision for queueing data passed as arguments to a Callback's mCall
 *     function.  If such a thing is required, the normal pattern is to take an output
 *     parameter at Callback registration time.
 *
 */
template <class T = CallFn>
class Callback : private Cancelable
{
public:
    /**
     * pointer to owner context, normally passed to the run function
     */
    void * mContext;

    /**
     * where to call when the event of interest has occurred
     */
    T mCall;

    /**
     * Indication that the Callback is registered with a notifier
     */
    bool IsRegistered() { return (mCancel != nullptr); }

    /**
     * Cancel, i.e. de-register interest in the event,
     * This is the only way to get access to the Cancelable, to enqueue,
     *  store any per-registration state.
     * There are 3 primary use cases for this API:
     *   1. For the owner of the Callback, Cancel() means "where-ever this Callback
     *       was put in a list or registered for an event, gimme back, remove interest".
     *   2. To a new registrar, during a registration call, it means "hey cleanup any
     *       current registrations, let me use the internal fields of Cancelable
     *       to keep track of what the owner is interested in.
     *   3. To any current registrar (i.e. when mCancel is non-null), Cancel() means:
     *       "remove this Callback from any internal lists and free any resources
     *        you've allocated to track the interest".
     *
     *  For example: a sockets library with an API like Socket::Readable(Callback<> *cb)
     *    using an underlying persistent registration API with the OS (like epoll())
     *    might store the file descriptor and interest mask in the scalar, put the
     *    Callback in a list.  Cancel() would dequeue the callback and remove
     *    the socket from the interest set
     *
     */
    Cancelable * Cancel() { return Cancelable::Cancel(); }

    /**
     * public constructor
     */
    Callback(T call, void * context) : mContext(context), mCall(call) { Cancelable(); }

    /**
     * TODO: type-safety? It'd be nice if Cancelables that aren't Callbacks returned null
     *    here.  https://github.com/project-chip/connectedhomeip/issues/1350
     */
    static Callback * FromCancelable(Cancelable * ca) { return static_cast<Callback *>(ca); }
};

/**
 * @brief core of a simple doubly-linked list Callback keeper-tracker-of
 *
 */
class CallbackDeque : public Cancelable
{
public:
    /**
     * @brief appends with overridden cancel function, in case the
     *   list change requires some other state update.
     */
    void Enqueue(Cancelable * ca, void (*cancel)(Cancelable *))
    {
        // add to a doubly-linked list, set cancel function
        InsertBefore(ca, this, cancel);
    }
    /**
     * @brief appends
     */
    void Enqueue(Cancelable * ca) { Enqueue(ca, Dequeue); }

    /**
     * @brief dequeue, but don't cancel, all cas that match the by()
     */
    void DequeueBy(bool (*by)(uint64_t, const Cancelable *), uint64_t p, Cancelable & dequeued)
    {
        for (Cancelable * ca = mNext; ca != this;)
        {
            Cancelable * next = ca->mNext;
            if (by(p, ca))
            {
                _Dequeue(ca);
                _InsertBefore(ca, &dequeued);
            }
            ca = next;
        }
    }

    /**
     * @brief insert the node in a queue in order, sorted by "sortby(a, b)"
     *   sortby(a, b) should return 1 if a > b, -1 if a < b and 0 if a == b
     */
    void InsertBy(Cancelable * ca, int (*sortby)(void *, const Cancelable *, const Cancelable *), void * p,
                  void (*cancel)(Cancelable *))
    {
        Cancelable * where; // node before which we need to insert
        for (where = mNext; where != this; where = where->mNext)
        {
            if (sortby(p, ca, where) <= 0)
            {
                break;
            }
        }
        InsertBefore(ca, where, cancel);
    }

    void InsertBy(Cancelable * ca, int (*sortby)(void *, const Cancelable *, const Cancelable *), void * p)
    {
        InsertBy(ca, sortby, p, Dequeue);
    }

    /**
     * @brief insert the node in a the list at a specific point
     */
    void InsertBefore(Cancelable * ca, Cancelable * where, void (*cancel)(Cancelable *))
    {
        ca->Cancel(); // make doubly-sure we're not corrupting another list somewhere
        ca->mCancel = cancel;
        _InsertBefore(ca, where);
    }
    void InsertBefore(Cancelable * ca, Cancelable * where) { InsertBefore(ca, where, Dequeue); }

    /**
     * @brief returns first item unless list is empty, otherwise returns NULL
     */
    Cancelable * First() { return (mNext != this) ? mNext : nullptr; }

    /**
     * @brief Dequeue all, return in a stub. does not cancel the cas, as the list
     *   members are still in use
     */
    void DequeueAll(Cancelable & ready)
    {
        if (mNext != this)
        {
            ready.mNext        = mNext;
            ready.mPrev        = mPrev;
            ready.mPrev->mNext = &ready;
            ready.mNext->mPrev = &ready;

            mNext = mPrev = this;
        }
    }

    /**
     * @brief dequeue but don't cancel, useful if
     *     immediately putting on another list
     */
    static void Dequeue(Cancelable * ca)
    {
        _Dequeue(ca);
        ca->mCancel = nullptr;
    }

    /**
     * @brief empty?
     */
    bool IsEmpty() { return mNext == this; }

private:
    static void _Dequeue(Cancelable * ca)
    {
        ca->mNext->mPrev = ca->mPrev;
        ca->mPrev->mNext = ca->mNext;
        ca->mNext = ca->mPrev = ca;
    }
    void _InsertBefore(Cancelable * ca, Cancelable * where)
    {
        ca->mPrev           = where->mPrev;
        where->mPrev->mNext = ca;
        where->mPrev        = ca;
        ca->mNext           = where;
    }
};

} // namespace Callback
} // namespace chip
