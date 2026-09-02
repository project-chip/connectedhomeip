/*
 *
 *    Copyright (c) 2020-2026 Project CHIP Authors
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
 *
 * Cancelable Callbacks
 * ====================
 *
 * Callback<T>, Cancelable, and related classes facilitate a family of asynchronous interaction
 * patterns, in which one component (the "caller") makes a function call to another component
 * (referred to as the "callee" or "registrar") to request some asynchronous operation with a
 * completion callback, or to register interest in some kind of event or notification.
 *
 * In contrast to passing a conventional function pointer or std::function, which only provides a
 * "return" channel from the callee to the caller, a cancelable callback additionally gives the
 * caller a way to send a cancellation signal to the callee/registrar when it wishes to cancel
 * the asynchronous operation or notification registration.
 *
 * The specific contract around when or how the callee/registrar calls the callback or delivers
 * events varies between use cases, but cancellation has a very strict contract that MUST be
 * adhered to by correct implementations of this pattern: Once the caller has called Cancel() on
 * a callback that it had passed to a callee/registrar, there will be no further invocations of
 * the callback. This means that the callback (and possibly the caller itself) can safely be
 * deallocated at that point without creating dangling pointers / callbacks.
 *
 * The flipside of this contract is that a callee/registrar that has accepted a callback MUST
 * install a CancelFn on the underlying Cancelable. When this CancelFn is called, the callee
 * removes the Cancelable and/or the associated callback from its internal data structures; the
 * callee MUST NOT access the Cancelable or Callback in any way after the CancelFn returns.
 *
 * Refer to the documentation of Callback and Cancelable below for details.
 *
 * While the concrete interaction patterns vary, the following are strong recommendations,
 * especially for asynchronous operations that have a single outcome:
 *
 * - Callees should take a single Callback as the last parameter to the function, with a callback
 *   signature that covers all success and failure cases. This is generally preferable to accepting
 *   e.g. separate OnSuccess and OnFailure callbacks, which would entail additional bookkeeping and
 *   memory usage for both the caller and callee, and would create separate cancellation signals.
 *
 * - If the callee has synchronously rejected the call outright (e.g. by returning a CHIP_ERROR)
 *   the callback will not be called at all. If the caller cancels the callback prior to the
 *   operation completing, the callback will not be called. Otherwise, the callback will be called
 *   exactly once.
 *
 * - Callback parameters should be passed by value as Callback<T>::Owned tokens (though various
 *   existing APIs accept callbacks as plain pointers). The Owned token implicitly constructs from
 *   a Callback<T> reference or pointer (an OwnedOrNull variant is available where a callback is
 *   optional). This guarantees that any callbacks are always cancelled at the caller-to-callee
 *   boundary, resulting in predictable cancellation with respect to any previous callee/registrar
 *   that a reused callback object may have been registered with. Note that because a token takes
 *   ownership when it is constructed, any previous callee's CancelFn runs during argument
 *   evaluation, with unspecified order relative to sibling arguments. Owned tokens are only
 *   intended to live for the duration of the call they are passed to. A callee that returns with
 *   the operation still in flight must Take() the Cancelable out of the token and install a
 *   CancelFn on it, as outlined in the general contract above; one that rejects the call outright
 *   simply drops the token, and one that completes synchronously may bypass registration and
 *   call Owned::Invoke().
 *
 * - Callers must be prepared for their callback to be invoked synchronously from within the
 *   function call initiating the operation. This avoids the callee having to dispatch the callback
 *   invocation via a work queue in case of a cached or other immediate outcome.
 *
 * - CallFn types should always carry a `void * context` as the initial parameter, which the callee
 *   populates from Callback.mContext when invoking the callback function (Callback.mCall).
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include <lib/core/CHIPConfig.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace Callback {

/**
 * Represents the "callee" side of a cancelable callback (see Callback below).
 *
 * The callee receives ownership of a Cancelable by calling Cancel() on a Callback provided by the
 * caller; usually it is convenient to store the pointer to the Cancelable, and to only recover the
 * Callback itself using Callback::FromCancelable when the time comes to invoke it.
 *
 * The callee "registers" the callback/cancelable by setting mCancel to a CancelFn, and may then
 * use the mNext/mPrev or mContextA/mContextB pointers to keep track of it internally as needed.
 * (Note that the CancelFn receives only the Cancelable pointer itself, i.e. has to rely on those
 * members to recover any context necessary to perform the cancellation and relinquish the callback.)
 */
class Cancelable
{
    typedef void (*CancelFn)(Cancelable *);

public:
    /**
     *  @brief for use by Callback callees, i.e. those that accept callbacks for
     *  event registration. When callbacks are enqueued in a CallbackDeque or
     *  GroupedCallbackList, the mPrev/mNext pointers are used to manage the
     *  list data structure. Callees that manage callbacks directly are free to
     *  use either mNext/mPrev or the generic mContextA/mContextB as needed.
     *
     *  Unless a callee has ownership (via Cancel() on the Callback, see
     *  below for details), these fields are indeterminate. The callee must
     *  explicitly initialize the fields it is going to use.
     */
    union
    {
        Cancelable * mNext;
        void * mContextA;
    };
    union
    {
        Cancelable * mPrev;
        void * mContextB;
    };

    // CHIP_CONFIG_CANCELABLE_HAS_INFO_STRING_FIELD allows consumers that were
    // using this field to opt into having it (and the resulting memory bloat)
    // while allowing everyone else to save the memory.
#if CHIP_CONFIG_CANCELABLE_HAS_INFO_STRING_FIELD
    alignas(uint64_t) char mInfo[24];
#endif // CHIP_CONFIG_CANCELABLE_HAS_INFO_STRING_FIELD

    /**
     * When non-null, indicates that the Callback associated with this Cancelable is registered
     * with a callee, and that the Cancelable members (mPrev, mNext, mContextA, mContextB, mInfo)
     * belong to that callee.
     *
     * When this function is called (generally by Cancel()), the callee must relinquish the
     * cancelable, and MUST NOT access the cancelable or associated callback in any way after
     * the function returns. Note that the CancelFn does not need to clear mCancel or any other
     * Cancelable members; this is handled automatically by Cancel().
     *
     * Cancellation also happens automatically during destruction.
     */
    CancelFn mCancel = nullptr;

    Cancelable() = default;
    ~Cancelable() { Cancel(); }

    /**
     * Runs the CancelFn (if any), and takes ownership of the Cancelable.
     *
     * Usually this method is called through the Callback sub-class, refer to
     * Callback::Cancel() for more details.
     */
    Cancelable * Cancel()
    {
        if (mCancel != nullptr)
        {
            CancelFn cancel = mCancel;
            mCancel         = nullptr;
            cancel(this);
            Invalidate();
        }
        return this;
    }

    /**
     * Invalidate the callee state after ownership has been relinquished.
     *
     * This is semantically a no-op in the sense that it clears state that by the contract of this
     * class shouldn't be read after that point anyway, but avoids dangling pointers.
     *
     * This is called automatically by Cancel() after the CancelFn has been invoked, but can also
     * be called manually as part of removing the cancelable from a data structure.
     */
    void Invalidate() { mPrev = mNext = nullptr; }

    // Not copyable
    Cancelable(const Cancelable &)             = delete;
    Cancelable & operator=(const Cancelable &) = delete;

    class Owned;

    /**
     * Transient move-only ownership of a Cancelable taken from a Callback, which may be null.
     * Usually obtained via an implicit Callback<T>::OwnedOrNull constructor.
     * A null instance can be constructed directly from a nullptr.
     * A token is consumed at most once, by Take() or by being moved from.
     */
    class OwnedOrNull
    {
    public:
        OwnedOrNull(std::nullptr_t) : mCancelable(nullptr) {}
        OwnedOrNull(OwnedOrNull && other) : mCancelable(std::exchange(other.mCancelable, nullptr)) {}

        /**
         * Widens an Owned, which is a non-null Cancelable, into a nullable one. Goes through
         * Owned::Take(), so the result of widening a live Owned is always non-null.
         */
        OwnedOrNull(Owned && owned);

        /**
         * Whether there is a Cancelable for Take() to return, i.e. false if no callback was
         * supplied or the token has been consumed. This is useful for callees that treat a missing
         * callback as a runtime error, rather than as a hard contract violation like Owned does.
         */
        [[nodiscard]] bool HasValue() const { return mCancelable != nullptr; }

        /**
         * Transfers ownership of the Cancelable out of the token, which may be null. This is how a
         * callee takes on the callback it was handed.
         *
         * Null means no callback was supplied. Taking from a consumed token is a programming error;
         * it currently yields null, but callers must not rely on that.
         */
        [[nodiscard]] Cancelable * Take() { return std::exchange(mCancelable, nullptr); }

    protected:
        explicit OwnedOrNull(Cancelable * cancelable) : mCancelable(cancelable) {}

        [[nodiscard]] Cancelable * TakeNonNull()
        {
            Cancelable * cancelable = OwnedOrNull::Take();
            VerifyOrDie(cancelable != nullptr);
            return cancelable;
        }

    private:
        // Consumed and empty tokens are indistinguishable, which is what keeps this pointer-sized.
        // Take() is documented as single-use rather than as returning null, so that consuming twice
        // can be made fatal later (e.g. via a sentinel) without breaking callers.
        Cancelable * mCancelable;
    };

    /**
     * Transient move-only ownership of a Cancelable taken from a Callback. Never null.
     * Usually obtained via an implicit Callback<T>::Owned constructor.
     * A token is consumed at most once, by Take() or by being moved from.
     */
    class Owned : private OwnedOrNull // private so the checked Take() can't be bypassed
    {
    public:
        /**
         * Transfers ownership of the Cancelable out of the token. Valid only if the token has not
         * been consumed.
         */
        [[nodiscard]] Cancelable * Take() { return TakeNonNull(); }

    protected:
        explicit Owned(Cancelable * cancelable) : OwnedOrNull(cancelable) { VerifyOrDie(cancelable != nullptr); }
    };
};

inline Cancelable::OwnedOrNull::OwnedOrNull(Owned && owned) : mCancelable(owned.Take()) {}

typedef void (*CallFn)(void *);

/**
 * A cancelable callback, refer to the introduction above for an overview.
 */
template <class T = CallFn>
class Callback : private Cancelable
{
public:
    /**
     * Constructs a Callback with the provided function pointer and context parameter.
     */
    Callback(T call, void * context) : mContext(context), mCall(call) {}

    /**
     * Caller context, generally passed as the first argument to mCall.
     */
    void * mContext;

    /**
     * The function to call when the event of interest has occurred.
     * Generally expected to be non-null.
     */
    T mCall;

    /**
     * Returns true if a callee/registrar has ownership of the Cancelable underlying this callback.
     */
    bool IsRegistered() { return (mCancel != nullptr); }

    /**
     * Cancel interest with any caller/registrar that this callback may be registered with,
     * and take ownership of the underlying Cancelable (this method is the only way to get
     * access to it).
     *
     * The "caller" and "callee" both make use of this API, but for different use cases:
     * 1) When a caller starts an interaction with a callee, the callee calls Cancel() to gain
     *    access to the Cancelable, and then registers its CancelFn on it in accordance with the
     *    contract outlined in the introduction above.
     * 2) At a later time, the caller may call Cancel() to notify the registered callee that it
     *    is no longer interested in being called back, and to make that registered callee
     *    relinquish the callback.
     * Combining these use cases into the same function guarantees that if caller (accidentally?)
     * passes a Callback that is already registered with some registrar A to a second registrar B,
     * then the Cancel() call performed by B to take ownership also cancels interest with A.
     *
     * Note that ownership of the Callback itself (and the mCall and mContext members specifically)
     * remains with the "caller" at all times; the "callee" only reads those members when invoking
     * mCall.
     */
    Cancelable * Cancel() { return Cancelable::Cancel(); }

    /**
     * Allows a callee to recover the Callback associated with a Cancelable.
     * This method is not type safe, the callee is responsible for recovering the Callback as the
     * type that the Cancelable was obtained from.
     */
    static Callback * FromCancelable(Cancelable * ca) { return static_cast<Callback *>(ca); }

    /**
     * Invokes the callback, passing mContext as the first argument.
     */
    template <typename... Args>
    void Invoke(Args &&... args)
    {
        mCall(mContext, std::forward<Args>(args)...);
    }

    /**
     * Implicitly accepts a Callback (or pointer to one) and takes ownership of it.
     *
     * A null pointer aborts, since an Owned is by definition non-null. When converting an existing
     * pointer-based API whose callers may legitimately pass null, take an OwnedOrNull instead and
     * reject a missing callback via HasValue(); prefer a Callback reference for new APIs, so that
     * the case cannot arise in the first place.
     *
     * Note that constructing a second Owned from the same Callback while the first is still alive
     * is not diagnosed. Only a callee accepting two callbacks of the same type can realistically
     * run into this, and diagnosing it would require marking the Cancelable for as long as a token
     * holds it, and make Owned no longer trivially destructible. This extra cost at every call site
     * of an API taking an Owned callback is not warranted for such a narrow corner case.
     */
    class Owned : public Cancelable::Owned
    {
    public:
        Owned(Callback & callback) : Cancelable::Owned(callback.Cancel()) {}
        Owned(Callback * callback) : Cancelable::Owned((callback != nullptr) ? callback->Cancel() : nullptr) {}
        Owned(std::nullptr_t) = delete;

        /**
         * Takes the callback out of the token and invokes it, for a callee that completes an
         * operation synchronously. Valid only if the token has not been consumed.
         *
         * A callee that completes asynchronously cannot use this: it must Take() the Cancelable and
         * register it synchronously, and later unregister from its bookkeeping before recovering
         * the Callback via FromCancelable().
         */
        template <typename... Args>
        void Invoke(Args &&... args)
        {
            FromCancelable(Take())->Invoke(std::forward<Args>(args)...);
        }
    };

    /**
     * Implicitly accepts a Callback (or pointer to one, which may be null) and takes ownership of it.
     */
    class OwnedOrNull : public Cancelable::OwnedOrNull
    {
    public:
        OwnedOrNull(Callback & callback) : Cancelable::OwnedOrNull(callback.Cancel()) {}
        OwnedOrNull(Callback * callback) : Cancelable::OwnedOrNull((callback != nullptr) ? callback->Cancel() : nullptr) {}
        OwnedOrNull(std::nullptr_t) : Cancelable::OwnedOrNull(nullptr) {}
        OwnedOrNull(Owned && owned) : Cancelable::OwnedOrNull(std::move(owned)) {}

        /**
         * Takes the callback out of the token and invokes it, for a callee that completes an
         * operation synchronously. Valid only if the token is not null and has not been consumed,
         * i.e. if HasValue() is true.
         *
         * A callee that completes asynchronously cannot use this: it must Take() the Cancelable and
         * register it synchronously, and later unregister from its bookkeeping before recovering
         * the Callback via FromCancelable().
         */
        template <typename... Args>
        void Invoke(Args &&... args)
        {
            FromCancelable(TakeNonNull())->Invoke(std::forward<Args>(args)...);
        }
    };
};

/**
 * A doubly-linked list of Cancelables.
 *
 * By default the CancelFn used by this container is CallbackDeque::Dequeue, i.e. cancellation
 * only removes a Cancelable from the list, but performs no other cleanup.
 *
 * Enqueue() optionally accepts a custom CancelFn, which must delegate to CallbackDeque::Dequeue.
 * However because both Cancelable members (mPrev and mNext) are used to represent the list structure
 * itself, such a custom CancelFn is unable to associate any additional context with each Cancelable.
 */
class CallbackDeque : protected Cancelable
{
public:
    CallbackDeque() { mNext = mPrev = this; }

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
    void DequeueBy(bool (*by)(uint64_t, const Cancelable *), uint64_t p, CallbackDeque & dequeued)
    {
        VerifyOrDie(&dequeued != this);
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
    Cancelable * First() { return (!IsEmpty()) ? mNext : nullptr; }

    /**
     * @brief Moves all callbacks from this deque to the end of the ready deque, without cancelling them.
     */
    void DequeueAll(CallbackDeque & ready)
    {
        VerifyOrReturn(!IsEmpty() && &ready != this);

        mNext->mPrev       = ready.mPrev;
        ready.mPrev->mNext = mNext;
        mPrev->mNext       = &ready;
        ready.mPrev        = mPrev;

        mNext = mPrev = this;
    }

    /**
     * Removes a Cancelable from the CallbackDeque containing it.
     * MUST NOT be called on a Cancelable that is not an element in a CallbackDeque.
     */
    static void Dequeue(Cancelable * ca)
    {
        _Dequeue(ca);
        ca->mCancel = nullptr;
        ca->Invalidate(); // Cancel() already does this, but this function is also a public API
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
