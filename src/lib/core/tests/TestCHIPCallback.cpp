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
 *      This file implements a test for  CHIP Callback
 *
 */

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPCallback.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

#include <cstddef>
#include <type_traits>
#include <utility>

using namespace chip::Callback;

// Runs a statement that uses a deprecated CallbackDeque API without a deprecation warning.
#define ALLOW_DEPRECATED(...)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"") __VA_ARGS__;                \
        _Pragma("GCC diagnostic pop")                                                                                              \
    } while (0)

/**
 * An example Callback registrar. Resumer::Resume() accepts Callbacks
 *   to be run during the next call to Resumer::Dispatch().  In an environment
 *   completely driven by callbacks, an application's main() would just call
 *   something like Resumer::Dispatch() in a loop.
 */
class Resumer : private TypedCallbackDeque<CallFn>
{
public:
    /**
     * @brief run this callback next Dispatch
     */
    void Resume(Callback<>::Owned cb) { Enqueue(std::move(cb)); }

    void Dispatch()
    {
        TypedCallbackDeque<CallFn> ready;

        DequeueAll(ready);

        // runs the ready list, with one-shot semantics
        while (auto * cb = ready.Take())
        {
            cb->Invoke();
        }
    }
};

static void increment(int * v)
{
    (*v)++;
}

struct Resume
{
    Callback<> * cb;
    Resumer * resumer;
};

static void resume(struct Resume * me)
{
    me->resumer->Resume(me->cb);
}

static void canceler(Cancelable * ca)
{
    ca->Cancel();
}

class TestCHIPCallback : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestCHIPCallback, ResumerTest)
{
    int n = 1;
    Callback<> cb(reinterpret_cast<CallFn>(increment), &n);
    Callback<> cancelcb(reinterpret_cast<CallFn>(canceler), cb.Cancel());
    Resumer resumer;

    // Resume() works
    resumer.Resume(&cb);
    resumer.Dispatch();
    resumer.Resume(&cb);
    resumer.Dispatch();
    EXPECT_EQ(n, 3);

    n = 1;
    // test cb->Cancel() cancels
    resumer.Resume(&cb);
    cb.Cancel();
    resumer.Dispatch();
    EXPECT_EQ(n, 1);

    n = 1;
    // Cancel cb before Dispatch() gets around to us (tests FIFO *and* cancel() from readylist)
    resumer.Resume(&cancelcb);
    resumer.Resume(&cb);
    resumer.Dispatch();
    EXPECT_EQ(n, 1);

    n = 1;
    // 2nd Resume() cancels first registration
    resumer.Resume(&cb);
    resumer.Resume(&cb); // cancels previous registration
    resumer.Dispatch();  // runs the list
    resumer.Dispatch();  // runs an empty list
    EXPECT_EQ(n, 2);

    n = 1;
    // Resume() during Dispatch() runs only once, but enqueues for next dispatch
    struct Resume res = { .cb = &cb, .resumer = &resumer };
    Callback<> resumecb(reinterpret_cast<CallFn>(resume), &res);
    resumer.Resume(&cb);
    resumer.Resume(&resumecb);
    resumer.Dispatch();
    EXPECT_EQ(n, 2);
    resumer.Dispatch();
    EXPECT_EQ(n, 3);

    Callback<> * pcb = chip::Platform::New<Callback<>>(reinterpret_cast<CallFn>(increment), &n);

    n = 1;
    // cancel on destruct
    resumer.Resume(pcb);
    resumer.Dispatch();
    EXPECT_EQ(n, 2);

    resumer.Resume(pcb);
    chip::Platform::Delete(pcb);
    resumer.Dispatch();
    EXPECT_EQ(n, 2);
}

/**
 * An example Callback registrar. Notifier implements persistently-registered
 *  semantics, and uses Callbacks with a non-default signature.
 *  Notifier covers the legacy custom-CancelFn path of the untyped CallbackDeque API.
 */
class Notifier : private CallbackDeque
{
public:
    typedef void (*NotifyFn)(void *, int);

    /**
     * run all the callers
     */
    void Notify(int v)
    {
        for (Cancelable * ca = mNext; ca != this; ca = ca->mNext)
        {
            // persistent registration semantics, with data
            Callback<NotifyFn>::FromCancelable(ca)->Invoke(v);
        }
    }

    static void Cancel(Cancelable * cb)
    {
        sCancelCount++;
        ALLOW_DEPRECATED(Dequeue(cb)); // take off ready list
    }

    /**
     * @brief illustrate a case where this needs notification of cancellation
     */
    void Register(Callback<NotifyFn> * cb) { ALLOW_DEPRECATED(Enqueue(cb->Cancel(), Cancel)); }

    using CallbackDeque::Clear;
    using CallbackDeque::Take;

    static inline int sCancelCount = 0;
};

static void increment_by(int * n, int by)
{
    *n += by;
}

TEST_F(TestCHIPCallback, NotifierTest)
{
    int n = 1;
    Callback<Notifier::NotifyFn> cb(reinterpret_cast<Notifier::NotifyFn>(increment_by), &n);
    Callback<Notifier::NotifyFn> cancelcb([](void * call, int) { canceler(reinterpret_cast<Cancelable *>(call)); }, cb.Cancel());

    // safe to call anytime
    cb.Cancel();

    Notifier notifier;

    // Simple stuff works, e.g. and there's persistent registration
    notifier.Register(&cb);
    notifier.Notify(1);
    notifier.Notify(8);
    EXPECT_EQ(n, 10);

    n = 1;
    // Cancel cb before Dispatch() gets around to us (tests FIFO *and* cancel() from readylist)
    notifier.Register(&cancelcb);
    notifier.Register(&cb);
    notifier.Notify(8);
    EXPECT_EQ(n, 1);

    cb.Cancel();
    cancelcb.Cancel();
}

// A custom CancelFn runs only when the element is cancelled; internal removal unregisters without it.
TEST_F(TestCHIPCallback, NotifierCancelFnRunsOnlyOnCancel)
{
    int n = 0;
    Callback<Notifier::NotifyFn> a(reinterpret_cast<Notifier::NotifyFn>(increment_by), &n);
    Callback<Notifier::NotifyFn> b(reinterpret_cast<Notifier::NotifyFn>(increment_by), &n);
    Notifier::sCancelCount = 0;

    {
        Notifier notifier;
        notifier.Register(&a);
        notifier.Register(&b);
        a.Cancel();
        EXPECT_EQ(Notifier::sCancelCount, 1);

        EXPECT_NE(notifier.Take(), nullptr);
        EXPECT_EQ(Notifier::sCancelCount, 1);

        notifier.Register(&a);
        notifier.Clear();
        EXPECT_EQ(Notifier::sCancelCount, 1);

        notifier.Register(&a);
    }
    EXPECT_EQ(Notifier::sCancelCount, 1);
}

// The legacy raw Enqueue() takes ownership itself, so it accepts a Callback that is still
// registered elsewhere.
TEST_F(TestCHIPCallback, LegacyRawEnqueue)
{
    int n = 0;
    Callback<> cb(reinterpret_cast<CallFn>(increment), &n);
    CallbackDeque first, second;

    Cancelable * ca = cb.Cancel();
    ALLOW_DEPRECATED(first.Enqueue(ca));
    EXPECT_TRUE(cb.IsRegistered());
    ALLOW_DEPRECATED(second.Enqueue(ca)); // still registered with first

    EXPECT_TRUE(first.IsEmpty());
    EXPECT_EQ(Callback<>::FromCancelable(second.Take()), &cb);
    EXPECT_FALSE(cb.IsRegistered());
    EXPECT_TRUE(second.IsEmpty());
}

TEST_F(TestCHIPCallback, TypedEnqueueAcceptsReferencePointerAndToken)
{
    int n = 0;
    Callback<> byRef(reinterpret_cast<CallFn>(increment), &n);
    Callback<> byPtr(reinterpret_cast<CallFn>(increment), &n);
    Callback<> byToken(reinterpret_cast<CallFn>(increment), &n);
    TypedCallbackDeque<CallFn> deque;

    deque.Enqueue(byRef);
    deque.Enqueue(&byPtr);
    Callback<>::Owned token(byToken);
    deque.Enqueue(std::move(token));
    EXPECT_TRUE(byRef.IsRegistered());
    EXPECT_TRUE(byPtr.IsRegistered());
    EXPECT_TRUE(byToken.IsRegistered());

    EXPECT_EQ(deque.First(), &byRef);
    EXPECT_EQ(deque.Take(), &byRef);
    EXPECT_EQ(deque.Take(), &byPtr);
    EXPECT_EQ(deque.Take(), &byToken);
    EXPECT_FALSE(byRef.IsRegistered());
    EXPECT_FALSE(byPtr.IsRegistered());
    EXPECT_FALSE(byToken.IsRegistered());
    EXPECT_EQ(n, 0);
}

// The untyped deque only accepts tokens, so ownership is still taken at the call boundary.
TEST_F(TestCHIPCallback, UntypedEnqueueTakesAToken)
{
    int n = 0;
    Callback<> a(reinterpret_cast<CallFn>(increment), &n);
    Callback<> b(reinterpret_cast<CallFn>(increment), &n);
    CallbackDeque deque;

    deque.Enqueue(Callback<>::Owned(a));
    deque.Enqueue(Callback<>::Owned(b));
    deque.Enqueue(Callback<>::Owned(a));
    EXPECT_TRUE(a.IsRegistered());
    EXPECT_TRUE(b.IsRegistered());

    EXPECT_EQ(Callback<>::FromCancelable(deque.Take()), &b);
    EXPECT_EQ(Callback<>::FromCancelable(deque.Take()), &a);
    EXPECT_EQ(deque.Take(), nullptr);
    EXPECT_FALSE(a.IsRegistered());
    EXPECT_FALSE(b.IsRegistered());
    EXPECT_EQ(n, 0);
}

TEST_F(TestCHIPCallback, TypedTakeReturnsNullWhenEmpty)
{
    TypedCallbackDeque<CallFn> deque;
    EXPECT_EQ(deque.First(), nullptr);
    EXPECT_EQ(deque.Take(), nullptr);
}

// Enqueueing a Callback that is already in a deque moves it to the end, rather than linking it twice.
TEST_F(TestCHIPCallback, TypedEnqueueOfAnElementMovesItToTheEnd)
{
    int n = 0;
    Callback<> a(reinterpret_cast<CallFn>(increment), &n);
    Callback<> b(reinterpret_cast<CallFn>(increment), &n);
    TypedCallbackDeque<CallFn> deque;

    deque.Enqueue(a);
    deque.Enqueue(b);
    deque.Enqueue(a);
    EXPECT_EQ(deque.Take(), &b);
    EXPECT_EQ(deque.Take(), &a);
    EXPECT_TRUE(deque.IsEmpty());
}

TEST_F(TestCHIPCallback, TypedClearUnregistersAllElements)
{
    int n = 0;
    Callback<> a(reinterpret_cast<CallFn>(increment), &n);
    Callback<> b(reinterpret_cast<CallFn>(increment), &n);
    TypedCallbackDeque<CallFn> deque;

    deque.Enqueue(a);
    deque.Enqueue(b);
    deque.Clear();
    EXPECT_TRUE(deque.IsEmpty());
    EXPECT_FALSE(a.IsRegistered());
    EXPECT_FALSE(b.IsRegistered());
}

// A deque that goes away first must not leave its elements pointing at it: Cancel() on an element
// would otherwise write into the destroyed deque.
TEST_F(TestCHIPCallback, DestroyingANonEmptyDequeUnregistersItsElements)
{
    int n = 0;
    Callback<> a(reinterpret_cast<CallFn>(increment), &n);
    Callback<> b(reinterpret_cast<CallFn>(increment), &n);

    {
        TypedCallbackDeque<CallFn> deque;
        deque.Enqueue(a);
        deque.Enqueue(b);
    }
    EXPECT_FALSE(a.IsRegistered());
    EXPECT_FALSE(b.IsRegistered());
    a.Cancel();
    b.Cancel();

    // Same for elements that were moved to another deque, i.e. are still registered.
    TypedCallbackDeque<CallFn> outer;
    {
        TypedCallbackDeque<CallFn> inner;
        inner.Enqueue(a);
        inner.DequeueAll(outer);
        EXPECT_TRUE(inner.IsEmpty());
        EXPECT_TRUE(a.IsRegistered());
        inner.Enqueue(b);
    }
    EXPECT_FALSE(b.IsRegistered());
    EXPECT_EQ(outer.Take(), &a);
    EXPECT_TRUE(outer.IsEmpty());
    EXPECT_EQ(n, 0);
}

// Verifies the mechanics of OperationalSessionSetup::NotifyRetryHandlers
TEST_F(TestCHIPCallback, PersistentCallbacksThatDeallocateThemselves)
{
    TypedCallbackDeque<CallFn> retryHandlers;

    int stackCallbackCalled = 0;
    Callback<> stackCallback{ [](void * context) { ++*static_cast<int *>(context); }, &stackCallbackCalled };
    retryHandlers.Enqueue(stackCallback);

    int heapCallbackCalled = 0;
    struct HeapCallback : public Callback<>
    {
        int * mCounter;
        explicit HeapCallback(int * counter) : Callback(&run, this), mCounter(counter) {}
        static void run(void * context)
        {
            HeapCallback * self = static_cast<HeapCallback *>(context);
            ++*self->mCounter;
            chip::Platform::Delete(self);
        }
    };
    retryHandlers.Enqueue(chip::Platform::New<HeapCallback>(&heapCallbackCalled));

    TypedCallbackDeque<CallFn> retryHandlerListSnapshot;
    retryHandlers.DequeueAll(retryHandlerListSnapshot);
    while (auto * cb = retryHandlerListSnapshot.First())
    {
        TypedCallbackDeque<CallFn> currentCallbackHolder;
        currentCallbackHolder.Enqueue(cb);

        cb->mCall(cb->mContext);

        if (!currentCallbackHolder.IsEmpty()) // cb->IsRegistered() would be UAF for HeapCallback
        {
            // Callback has not been canceled as part of the call,
            // so is still supposed to be registered with us.
            retryHandlers.Enqueue(cb);
        }
    }

    EXPECT_TRUE(retryHandlerListSnapshot.IsEmpty());
    EXPECT_EQ(stackCallbackCalled, 1);
    EXPECT_EQ(heapCallbackCalled, 1);
    EXPECT_FALSE(retryHandlers.IsEmpty()); // stack callback remains
    stackCallback.Cancel();
    EXPECT_TRUE(retryHandlers.IsEmpty());
}

/*
 * Owned / OwnedOrNull tokens
 */

// The properties the Owned tokens exist to enforce, none of which are observable at runtime.
static_assert(!std::is_copy_constructible_v<Callback<>::Owned>, "Owned is move-only");
static_assert(std::is_move_constructible_v<Callback<>::Owned>, "Owned is move-only");
static_assert(!std::is_copy_constructible_v<Callback<>::OwnedOrNull>, "OwnedOrNull is move-only");
static_assert(std::is_move_constructible_v<Callback<>::OwnedOrNull>, "OwnedOrNull is move-only");

// Owned implicitly accepts a Callback by reference or by pointer, but rejects a literal nullptr.
static_assert(std::is_convertible_v<Callback<> &, Callback<>::Owned>, "Owned accepts a reference");
static_assert(std::is_convertible_v<Callback<> *, Callback<>::Owned>, "Owned accepts a pointer");
static_assert(!std::is_constructible_v<Callback<>::Owned, std::nullptr_t>, "Owned rejects nullptr");

// OwnedOrNull additionally accepts a literal nullptr, as well as an Owned.
static_assert(std::is_convertible_v<std::nullptr_t, Callback<>::OwnedOrNull>, "OwnedOrNull accepts nullptr");
static_assert(std::is_convertible_v<Callback<>::Owned, Callback<>::OwnedOrNull>, "Owned converts to OwnedOrNull");

// A signature-agnostic callee's "no callback" case does not go through a typed token either, since
// a caller with nothing to pass has no signature to name.
static_assert(std::is_convertible_v<std::nullptr_t, Cancelable::OwnedOrNull>, "Cancelable::OwnedOrNull accepts nullptr");
static_assert(!std::is_constructible_v<Cancelable::Owned, std::nullptr_t>, "Cancelable::Owned rejects nullptr");

// Callbacks with a different signature are not interchangeable.
static_assert(!std::is_constructible_v<Callback<>::Owned, Callback<Notifier::NotifyFn> &>, "signatures must match");
static_assert(!std::is_constructible_v<Callback<Notifier::NotifyFn>::Owned, Callback<> &>, "signatures must match");

// A callee may take a signature-agnostic Cancelable::Owned, or widen to a Cancelable::OwnedOrNull ...
static_assert(std::is_convertible_v<Callback<>::Owned, Cancelable::Owned>, "Owned converts to Cancelable::Owned");
static_assert(std::is_convertible_v<Cancelable::Owned, Cancelable::OwnedOrNull>, "Owned widens to OwnedOrNull");
static_assert(std::is_convertible_v<Callback<>::Owned, Cancelable::OwnedOrNull>, "Owned widens to OwnedOrNull");
// ... but only by value: Owned::Take() hides rather than overrides OwnedOrNull::Take(), so binding
// an Owned to an OwnedOrNull reference would sidestep its non-null check.
static_assert(!std::is_convertible_v<Callback<>::Owned &, Cancelable::OwnedOrNull &>, "Owned must not slice");
static_assert(!std::is_convertible_v<Cancelable::Owned &, Cancelable::OwnedOrNull &>, "Owned must not slice");

// Narrowing an OwnedOrNull back to an Owned aborts on null, so it has to be explicit.
static_assert(std::is_constructible_v<Cancelable::Owned, Cancelable::OwnedOrNull>, "OwnedOrNull narrows to Owned");
static_assert(!std::is_convertible_v<Cancelable::OwnedOrNull, Cancelable::Owned>, "narrowing is explicit");
static_assert(std::is_constructible_v<Callback<>::Owned, Callback<>::OwnedOrNull>, "OwnedOrNull narrows to Owned");
static_assert(!std::is_convertible_v<Callback<>::OwnedOrNull, Callback<>::Owned>, "narrowing is explicit");
static_assert(!std::is_constructible_v<Callback<>::Owned, Cancelable::OwnedOrNull>, "narrowing keeps the signature");

/**
 * A registrar in the shape CHIPCallback.h recommends for new APIs: it accepts callbacks by value
 * as Owned / OwnedOrNull tokens rather than as plain pointers.
 */
class Acceptor
{
public:
    void Accept(Callback<>::Owned cb) { mCallbacks.Enqueue(std::move(cb)); }

    // Returns whether a callback was actually provided.
    bool AcceptOptional(Callback<>::OwnedOrNull cb)
    {
        VerifyOrReturnValue(cb.HasValue(), false);
        mCallbacks.Enqueue(Callback<>::Owned(std::move(cb)));
        return true;
    }

    // Models a signature-agnostic callee, e.g. a shared layer underneath several typed APIs.
    // Returns whether a callback was actually provided.
    bool AcceptAgnostic(Cancelable::OwnedOrNull cb)
    {
        VerifyOrReturnValue(cb.HasValue(), false);
        mCallbacks.Enqueue(Cancelable::Owned(std::move(cb)));
        return true;
    }

    // Models a callee that rejects the call outright, dropping the token without taking it.
    static CHIP_ERROR Reject(Callback<>::Owned) { return CHIP_ERROR_INCORRECT_STATE; }

    // Models a callee that completes the operation synchronously, without ever registering the
    // callback with itself.
    static void AcceptAndComplete(Callback<>::Owned cb) { cb.Invoke(); }

    // As above, but for an optional callback: Invoke() requires one, so HasValue() is a precondition.
    // Returns whether a callback was actually provided.
    static bool AcceptOptionalAndComplete(Callback<>::OwnedOrNull cb)
    {
        VerifyOrReturnValue(cb.HasValue(), false);
        cb.Invoke();
        return true;
    }

    // Models synchronous completion of an operation whose callback signature carries an outcome.
    static void CompleteWith(Callback<Notifier::NotifyFn>::Owned cb, int result) { cb.Invoke(result); }

    // Returns whether a callback was actually provided.
    static bool CompleteOptionalWith(Callback<Notifier::NotifyFn>::OwnedOrNull cb, int result)
    {
        VerifyOrReturnValue(cb.HasValue(), false);
        cb.Invoke(result);
        return true;
    }

    // All entry points store type-erased; this is where the CallFn signature is recovered.
    void InvokeAll()
    {
        CallbackDeque ready;
        mCallbacks.DequeueAll(ready);
        while (Cancelable * ca = ready.Take())
        {
            Callback<>::FromCancelable(ca)->Invoke();
        }
    }

    bool IsEmpty() { return mCallbacks.IsEmpty(); }

private:
    CallbackDeque mCallbacks;
};

TEST_F(TestCHIPCallback, OwnedTakesOwnershipAtTheCallBoundary)
{
    int n = 0;
    Callback<> cb(reinterpret_cast<CallFn>(increment), &n);
    Acceptor first, second;

    first.Accept(cb);
    EXPECT_TRUE(cb.IsRegistered());
    EXPECT_FALSE(first.IsEmpty());

    // Handing the same callback to a second callee withdraws it from the first.
    second.Accept(cb);
    EXPECT_TRUE(first.IsEmpty());
    EXPECT_FALSE(second.IsEmpty());

    first.InvokeAll();
    EXPECT_EQ(n, 0);
    second.InvokeAll();
    EXPECT_EQ(n, 1);
    EXPECT_FALSE(cb.IsRegistered());
}

TEST_F(TestCHIPCallback, OwnedAcceptsAPointer)
{
    int n = 0;
    Callback<> cb(reinterpret_cast<CallFn>(increment), &n);
    Acceptor acceptor;

    acceptor.Accept(&cb);
    EXPECT_TRUE(cb.IsRegistered());
    acceptor.InvokeAll();
    EXPECT_EQ(n, 1);
}

TEST_F(TestCHIPCallback, OwnedIsMovable)
{
    int n = 0;
    Callback<> cb(reinterpret_cast<CallFn>(increment), &n);
    Acceptor acceptor;

    Callback<>::Owned owned(cb);
    EXPECT_FALSE(cb.IsRegistered()); // the token holds the cancelable now, nobody is registered

    acceptor.Accept(std::move(owned));
    EXPECT_TRUE(cb.IsRegistered());
    acceptor.InvokeAll();
    EXPECT_EQ(n, 1);
}

// A callee that rejects the call outright still cancels the callback at the boundary: dropping the
// token leaves the callback registered with nobody, and no invocation ever happens.
TEST_F(TestCHIPCallback, DroppingAnOwnedTokenLeavesTheCallbackUnregistered)
{
    int n = 0;
    Callback<> cb(reinterpret_cast<CallFn>(increment), &n);
    Acceptor acceptor;

    acceptor.Accept(cb);
    EXPECT_TRUE(cb.IsRegistered());

    EXPECT_EQ(Acceptor::Reject(cb), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(cb.IsRegistered());
    EXPECT_TRUE(acceptor.IsEmpty());

    acceptor.InvokeAll();
    EXPECT_EQ(n, 0);
}

// A callee completing synchronously never registers the callback, so it invokes it straight off the
// token: there is no window in which the caller could cancel, and nothing to unregister.
TEST_F(TestCHIPCallback, OwnedInvokesTheCallbackDirectly)
{
    int n = 0;
    Callback<> cb(reinterpret_cast<CallFn>(increment), &n);
    Acceptor acceptor;

    // Whoever held the callback still gives it up at the boundary, as for any other call.
    acceptor.Accept(cb);
    EXPECT_TRUE(cb.IsRegistered());

    Acceptor::AcceptAndComplete(cb);
    EXPECT_EQ(n, 1);
    EXPECT_FALSE(cb.IsRegistered());
    EXPECT_TRUE(acceptor.IsEmpty());
}

TEST_F(TestCHIPCallback, OwnedOrNullAcceptsAMissingCallback)
{
    int n = 0;
    Callback<> cb(reinterpret_cast<CallFn>(increment), &n);
    Acceptor acceptor;

    EXPECT_FALSE(acceptor.AcceptOptional(nullptr));
    EXPECT_TRUE(acceptor.IsEmpty());

    Callback<> * missing = nullptr;
    EXPECT_FALSE(acceptor.AcceptOptional(missing));
    EXPECT_TRUE(acceptor.IsEmpty());

    EXPECT_TRUE(acceptor.AcceptOptional(cb));
    EXPECT_TRUE(cb.IsRegistered());
    acceptor.InvokeAll();
    EXPECT_EQ(n, 1);
}

// An Owned converts to an OwnedOrNull, so a callee holding a mandatory callback can forward it to
// an API that treats it as optional.
TEST_F(TestCHIPCallback, OwnedOrNullAcceptsAnOwned)
{
    int n = 0;
    Callback<> cb(reinterpret_cast<CallFn>(increment), &n);
    Acceptor acceptor;

    EXPECT_TRUE(acceptor.AcceptOptional(Callback<>::Owned(cb)));
    EXPECT_TRUE(cb.IsRegistered());
    acceptor.InvokeAll();
    EXPECT_EQ(n, 1);
}

// The optional-callback counterpart of OwnedInvokesTheCallbackDirectly. Invoke() requires a
// callback, so a callee that may not have been given one checks HasValue() first.
TEST_F(TestCHIPCallback, OwnedOrNullInvokesTheCallbackDirectly)
{
    int n = 0;
    Callback<> cb(reinterpret_cast<CallFn>(increment), &n);
    Acceptor acceptor;

    EXPECT_FALSE(Acceptor::AcceptOptionalAndComplete(nullptr));
    EXPECT_EQ(n, 0);

    acceptor.Accept(cb);
    EXPECT_TRUE(cb.IsRegistered());

    EXPECT_TRUE(Acceptor::AcceptOptionalAndComplete(cb));
    EXPECT_EQ(n, 1);
    EXPECT_FALSE(cb.IsRegistered());
    EXPECT_TRUE(acceptor.IsEmpty());
}

// A callee that has checked HasValue() can narrow an OwnedOrNull to an Owned, consuming the source.
TEST_F(TestCHIPCallback, OwnedOrNullNarrowsToOwned)
{
    int n = 0;
    Callback<> cb(reinterpret_cast<CallFn>(increment), &n);

    Callback<>::OwnedOrNull optional(cb);
    Callback<>::Owned owned(std::move(optional));
    EXPECT_FALSE(optional.HasValue()); // NOLINT(bugprone-use-after-move)
    EXPECT_EQ(Callback<>::FromCancelable(owned.Take()), &cb);

    Cancelable::OwnedOrNull agnosticOptional(Callback<>::Owned{ cb });
    Cancelable::Owned agnostic(std::move(agnosticOptional));
    EXPECT_FALSE(agnosticOptional.HasValue()); // NOLINT(bugprone-use-after-move)
    EXPECT_EQ(Callback<>::FromCancelable(agnostic.Take()), &cb);
    EXPECT_EQ(n, 0);
}

// Invoke() forwards its arguments to the callback, so a callee completing synchronously reports an
// outcome exactly as it would have from an asynchronous completion.
TEST_F(TestCHIPCallback, InvokeForwardsArgumentsToTheCallback)
{
    int n = 0;
    Callback<Notifier::NotifyFn> cb(reinterpret_cast<Notifier::NotifyFn>(increment_by), &n);

    Acceptor::CompleteWith(cb, 3);
    EXPECT_EQ(n, 3);
    EXPECT_FALSE(cb.IsRegistered());

    EXPECT_TRUE(Acceptor::CompleteOptionalWith(cb, 4));
    EXPECT_EQ(n, 7);
    EXPECT_FALSE(cb.IsRegistered());

    EXPECT_FALSE(Acceptor::CompleteOptionalWith(nullptr, 5));
    EXPECT_EQ(n, 7);
}

// A typed Owned widens all the way to a signature-agnostic Cancelable::OwnedOrNull, so a shared
// layer underneath several typed APIs can accept one without being templated over the signature.
TEST_F(TestCHIPCallback, SignatureAgnosticOwnedOrNull)
{
    int n = 0;
    Callback<> cb(reinterpret_cast<CallFn>(increment), &n);
    Acceptor acceptor;

    EXPECT_FALSE(acceptor.AcceptAgnostic(Callback<>::OwnedOrNull(nullptr)));
    EXPECT_TRUE(acceptor.IsEmpty());

    // A caller with no callback to pass need not name a signature just to say so.
    EXPECT_FALSE(acceptor.AcceptAgnostic(nullptr));
    EXPECT_TRUE(acceptor.IsEmpty());

    EXPECT_TRUE(acceptor.AcceptAgnostic(Callback<>::Owned(cb)));
    EXPECT_TRUE(cb.IsRegistered());
    acceptor.InvokeAll();
    EXPECT_EQ(n, 1);
}

// HasValue() answers what Take() would hand over, so that a callee rejecting a missing callback can
// check it as a precondition rather than having to take first and test the pointer.
TEST_F(TestCHIPCallback, OwnedOrNullHasValue)
{
    int n = 0;
    Callback<> cb(reinterpret_cast<CallFn>(increment), &n);

    EXPECT_TRUE(Callback<>::OwnedOrNull(cb).HasValue());
    EXPECT_FALSE(Callback<>::OwnedOrNull(nullptr).HasValue());
    EXPECT_FALSE(Cancelable::OwnedOrNull(nullptr).HasValue());
    EXPECT_TRUE(Cancelable::OwnedOrNull(Callback<>::Owned(cb)).HasValue());

    // Reports false once the token has been consumed. This stays true of HasValue() even though
    // Take() does not promise a value for a consumed token, so that a callee holding one it may
    // already have passed on can still ask.
    Callback<>::OwnedOrNull token(cb);
    EXPECT_NE(token.Take(), nullptr);
    EXPECT_FALSE(token.HasValue());
}
