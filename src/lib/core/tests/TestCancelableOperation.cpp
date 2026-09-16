/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <lib/core/CancelableOperation.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

using namespace chip;
using namespace chip::Callback;

using TestOperationBase = CancelableOperation<CHIP_ERROR, int>;

class TestCaller;

/**
 * A concrete operation, standing in for the sort of thing a real subclass would be. It performs no
 * work of its own; the test drives the inherited Start() / Complete() directly, which are exposed
 * publicly here.
 */
class TestOperation : public TestOperationBase
{
public:
    using TestOperationBase::Complete;
    using TestOperationBase::Start;

    int mCancelledCount = 0;
    int mFinishedCount  = 0;

    // The `cancelled` argument of, and what IsPending() returned during, the most recent
    // OnFinished() call.
    std::optional<bool> mLastFinishedCancelled;
    std::optional<bool> mPendingDuringFinished;

protected:
    void OnFinished(bool cancelled) override
    {
        mFinishedCount++;
        mCancelledCount += cancelled ? 1 : 0;
        mLastFinishedCancelled = cancelled;
        mPendingDuringFinished = IsPending();
        TestOperationBase::OnFinished(cancelled);
    }
};

/**
 * The "caller" side: owns the completion callback and records what it hears back.
 */
class TestCaller
{
public:
    TestOperationBase::Completion mCallback{ OnCompletion, this };

    int mCompletionCount = 0;
    CHIP_ERROR mStatus   = CHIP_NO_ERROR;
    int mValue           = 0;

    // Observed from within the completion handler.
    std::optional<bool> mPendingDuringCompletion;
    std::optional<bool> mRegisteredDuringCompletion;
    std::optional<int> mFinishedCountDuringCompletion;

    // The operation the callback was handed to, and an optional action to run reentrantly from
    // within the completion handler (cleared before it runs, so it only fires once).
    TestOperation * mOperation             = nullptr;
    void (*mReentrantAction)(TestCaller &) = nullptr;

    // Optionally holds the operation, so that a reentrant action can destroy it.
    std::unique_ptr<TestOperation> mOwnedOperation;

private:
    static void OnCompletion(void * context, CHIP_ERROR status, int value)
    {
        auto & self = *static_cast<TestCaller *>(context);
        self.mCompletionCount++;
        self.mStatus = status;
        self.mValue  = value;

        if (self.mOperation != nullptr)
        {
            self.mPendingDuringCompletion       = self.mOperation->IsPending();
            self.mRegisteredDuringCompletion    = self.mCallback.IsRegistered();
            self.mFinishedCountDuringCompletion = self.mOperation->mFinishedCount;
        }

        if (auto * action = self.mReentrantAction)
        {
            self.mReentrantAction = nullptr;
            action(self);
        }
    }
};

TEST(CancelableOperationTest, IdleBeforeBeingStarted)
{
    TestOperation operation;
    TestCaller caller;

    EXPECT_FALSE(operation.IsPending());
    EXPECT_FALSE(caller.mCallback.IsRegistered());
}

TEST(CancelableOperationTest, StartAndComplete)
{
    TestOperation operation;
    TestCaller caller;
    caller.mOperation = &operation;

    operation.Start(caller.mCallback);
    EXPECT_TRUE(operation.IsPending());
    EXPECT_TRUE(caller.mCallback.IsRegistered());
    EXPECT_EQ(caller.mCompletionCount, 0);

    CHIP_ERROR status = CHIP_ERROR_BUSY;
    int value         = 42;
    operation.Complete(status, value);

    EXPECT_EQ(caller.mCompletionCount, 1);
    EXPECT_EQ(caller.mStatus, CHIP_ERROR_BUSY);
    EXPECT_EQ(caller.mValue, 42);
    EXPECT_EQ(operation.mCancelledCount, 0);
    EXPECT_FALSE(operation.IsPending());
    EXPECT_FALSE(caller.mCallback.IsRegistered());
}

// The operation must relinquish the callback *before* invoking it, so that the caller is free to
// hand the callback straight back to another callee (or to this operation) from the completion.
TEST(CancelableOperationTest, NeitherPendingNorRegisteredDuringCompletion)
{
    TestOperation operation;
    TestCaller caller;
    caller.mOperation = &operation;

    operation.Start(caller.mCallback);
    operation.Complete(CHIP_NO_ERROR, 0);

    ASSERT_TRUE(caller.mPendingDuringCompletion.has_value());
    EXPECT_FALSE(*caller.mPendingDuringCompletion);
    ASSERT_TRUE(caller.mRegisteredDuringCompletion.has_value());
    EXPECT_FALSE(*caller.mRegisteredDuringCompletion);
}

TEST(CancelableOperationTest, RestartedFromWithinCompletion)
{
    TestOperation operation;
    TestCaller caller;
    caller.mOperation       = &operation;
    caller.mReentrantAction = [](TestCaller & self) { self.mOperation->Start(self.mCallback); };

    operation.Start(caller.mCallback);
    operation.Complete(CHIP_NO_ERROR, 1);

    // The reentrant Start() left a second operation in flight.
    EXPECT_EQ(caller.mCompletionCount, 1);
    EXPECT_TRUE(operation.IsPending());
    EXPECT_TRUE(caller.mCallback.IsRegistered());

    operation.Complete(CHIP_NO_ERROR, 2);
    EXPECT_EQ(caller.mCompletionCount, 2);
    EXPECT_EQ(caller.mValue, 2);
    EXPECT_FALSE(operation.IsPending());
    EXPECT_FALSE(caller.mCallback.IsRegistered());
}

// Complete() touches no operation state once it has handed the callback back, so a completion
// handler is free to destroy the operation (e.g. to return it to a pool).
TEST(CancelableOperationTest, DestroyedFromWithinCompletion)
{
    TestCaller caller;
    caller.mOwnedOperation  = std::make_unique<TestOperation>();
    caller.mOperation       = caller.mOwnedOperation.get();
    caller.mReentrantAction = [](TestCaller & self) {
        self.mOperation = nullptr;
        self.mOwnedOperation.reset();
    };

    TestOperation & operation = *caller.mOwnedOperation;
    operation.Start(caller.mCallback);
    operation.Complete(CHIP_NO_ERROR, 5);

    EXPECT_EQ(caller.mCompletionCount, 1);
    EXPECT_EQ(caller.mValue, 5);
    EXPECT_EQ(caller.mOwnedOperation, nullptr);
    EXPECT_FALSE(caller.mCallback.IsRegistered());
}

TEST(CancelableOperationTest, CallerCancels)
{
    TestOperation operation;
    TestCaller caller;
    caller.mOperation = &operation;

    operation.Start(caller.mCallback);
    caller.mCallback.Cancel();

    EXPECT_EQ(operation.mCancelledCount, 1);
    EXPECT_EQ(caller.mCompletionCount, 0);
    EXPECT_FALSE(operation.IsPending());
    EXPECT_FALSE(caller.mCallback.IsRegistered());

    // The operation is told about the cancellation only once it is no longer pending, so that it
    // cannot mistakenly try to complete in response.
    ASSERT_TRUE(operation.mPendingDuringFinished.has_value());
    EXPECT_FALSE(*operation.mPendingDuringFinished);
    EXPECT_EQ(operation.mLastFinishedCancelled, std::optional(true));
}

// A completed operation must not be reachable from the callback any more: a caller that recycles
// its callback (which starts with a Cancel()) must not be mistaken for one giving up on the work.
TEST(CancelableOperationTest, CancellingAfterCompletionIsANoOp)
{
    TestOperation operation;
    TestCaller caller;
    caller.mOperation = &operation;

    operation.Start(caller.mCallback);
    operation.Complete(CHIP_NO_ERROR, 0);
    caller.mCallback.Cancel();

    EXPECT_EQ(operation.mCancelledCount, 0);
    EXPECT_EQ(caller.mCompletionCount, 1);
}

TEST(CancelableOperationTest, DestroyingTheCallbackCancels)
{
    TestOperation operation;
    {
        TestCaller caller;
        caller.mOperation = &operation;
        operation.Start(caller.mCallback);
        EXPECT_TRUE(operation.IsPending());
    }

    EXPECT_EQ(operation.mCancelledCount, 1);
    EXPECT_FALSE(operation.IsPending());
}

// A pooled operation may well free itself once it is no longer pending. Cancel() only touches the
// Cancelable (which belongs to the caller) after the CancelFn returns, so this is safe.
TEST(CancelableOperationTest, DestroyedFromWithinOnFinished)
{
    bool destroyed = false;

    class SelfFreeingOperation : public TestOperationBase
    {
    public:
        using TestOperationBase::Start;

        explicit SelfFreeingOperation(bool * destroyed) : mDestroyed(destroyed) {}
        ~SelfFreeingOperation() override { *mDestroyed = true; }

    protected:
        void OnFinished(bool cancelled) override
        {
            TestOperationBase::OnFinished(cancelled);
            delete this;
        }

    private:
        bool * mDestroyed;
    };

    TestCaller caller;
    (new SelfFreeingOperation(&destroyed))->Start(caller.mCallback);
    ASSERT_FALSE(destroyed);

    caller.mCallback.Cancel();
    EXPECT_TRUE(destroyed);
    EXPECT_FALSE(caller.mCallback.IsRegistered());
    EXPECT_EQ(caller.mCompletionCount, 0);
}

// Callback::Cancel() cancels interest with any previous callee, so handing an already-registered
// callback to a second operation withdraws it from the first.
TEST(CancelableOperationTest, StartingASecondOperationCancelsTheFirst)
{
    TestOperation first;
    TestOperation second;
    TestCaller caller;

    first.Start(caller.mCallback);
    second.Start(caller.mCallback);

    EXPECT_EQ(first.mCancelledCount, 1);
    EXPECT_FALSE(first.IsPending());
    EXPECT_EQ(second.mCancelledCount, 0);
    EXPECT_TRUE(second.IsPending());
    EXPECT_TRUE(caller.mCallback.IsRegistered());

    caller.mOperation = &second;
    second.Complete(CHIP_NO_ERROR, 7);
    EXPECT_EQ(caller.mCompletionCount, 1);
    EXPECT_EQ(caller.mValue, 7);
}

// Handing a callback back to the operation it is already registered with does not trip Start()'s
// not-already-pending assertion: the Owned token cancels at the call boundary, so the in-flight
// operation is cancelled before Start() ever runs.
TEST(CancelableOperationTest, RestartingWithTheSameCallbackCancelsTheInFlightOperation)
{
    TestOperation operation;
    TestCaller caller;
    caller.mOperation = &operation;

    operation.Start(caller.mCallback);
    operation.Start(caller.mCallback);

    EXPECT_EQ(operation.mCancelledCount, 1);
    EXPECT_EQ(caller.mCompletionCount, 0);
    EXPECT_TRUE(operation.IsPending());
    EXPECT_TRUE(caller.mCallback.IsRegistered());

    operation.Complete(CHIP_NO_ERROR, 9);
    EXPECT_EQ(caller.mCompletionCount, 1);
    EXPECT_EQ(caller.mValue, 9);
    EXPECT_EQ(operation.mCancelledCount, 1);
}

// A callee that rejects the call outright drops the Owned token instead of starting. The callback
// has still been cancelled at the boundary, so a previously pending operation hears about it.
TEST(CancelableOperationTest, RejectingWithoutStarting)
{
    TestOperation operation;
    TestCaller caller;
    caller.mOperation = &operation;

    auto reject = [](TestOperationBase::Completion::Owned) { return CHIP_ERROR_INCORRECT_STATE; };

    operation.Start(caller.mCallback);
    EXPECT_EQ(reject(caller.mCallback), CHIP_ERROR_INCORRECT_STATE);

    EXPECT_EQ(operation.mCancelledCount, 1);
    EXPECT_EQ(caller.mCompletionCount, 0);
    EXPECT_FALSE(operation.IsPending());
    EXPECT_FALSE(caller.mCallback.IsRegistered());
}

TEST(CancelableOperationTest, SequentialOperationsReuseTheSameInstance)
{
    TestOperation operation;
    TestCaller caller;
    caller.mOperation = &operation;

    operation.Start(caller.mCallback);
    operation.Complete(CHIP_NO_ERROR, 1);
    operation.Start(caller.mCallback);
    caller.mCallback.Cancel();
    operation.Start(caller.mCallback);
    operation.Complete(CHIP_ERROR_TIMEOUT, 3);

    EXPECT_EQ(caller.mCompletionCount, 2);
    EXPECT_EQ(caller.mStatus, CHIP_ERROR_TIMEOUT);
    EXPECT_EQ(caller.mValue, 3);
    EXPECT_EQ(operation.mCancelledCount, 1);
    EXPECT_FALSE(operation.IsPending());
    EXPECT_FALSE(caller.mCallback.IsRegistered());
}

/**
 * A move-only payload, to pin that Start() can take ownership of something and Complete() can hand
 * ownership back to the caller. This is what makes the std::forward<>() in both load-bearing rather
 * than a no-op: without it neither the Start() below nor either Complete() spelling compiles.
 */
using Payload = std::unique_ptr<int>;

/**
 * A reusable, signature-independent operation layer, in the shape that real users of the mixin are
 * expected to take: the shared mechanics subclass CancelableOperationBase (and are compiled once),
 * and the completion signature is applied on top per concrete operation.
 *
 * It takes parameters of its own in Start(); the mixin reads them off that declaration and
 * re-declares Start() with them ahead of the typed completion.
 */
class UnderlyingWorkOperationBase : public CancelableOperationBase
{
public:
    bool mWorkReleased  = false;
    const char * mLabel = nullptr;
    Payload mWork;

protected:
    void Start(const char * label, Payload work, Cancelable::Owned onCompletion)
    {
        CancelableOperationBase::Start(std::move(onCompletion));
        mLabel = label;
        mWork  = std::move(work);
    }

    void OnFinished(bool cancelled) override
    {
        if (cancelled)
        {
            mWorkReleased = true;
            mWork.reset();
        }
        CancelableOperationBase::OnFinished(cancelled);
    }
};

template <typename... Args>
using UnderlyingWorkOperation = TypedOperation<UnderlyingWorkOperationBase, Args...>;

using StatusOnlyOperationBase = UnderlyingWorkOperation<CHIP_ERROR>;

class StatusOnlyOperation : public StatusOnlyOperationBase
{
public:
    using StatusOnlyOperationBase::Complete;
    using StatusOnlyOperationBase::Start;
};

TEST(CancelableOperationTest, TypedCompletionOverACustomOperationBase)
{
    struct Recorder
    {
        int count         = 0;
        CHIP_ERROR status = CHIP_NO_ERROR;

        static void OnCompletion(void * context, CHIP_ERROR status)
        {
            auto & self = *static_cast<Recorder *>(context);
            self.count++;
            self.status = status;
        }
    } recorder;

    StatusOnlyOperation::Completion callback(Recorder::OnCompletion, &recorder);
    StatusOnlyOperation operation;

    // The layer's own parameters come ahead of the completion, and are forwarded to it.
    operation.Start("first", std::make_unique<int>(1), callback);
    EXPECT_TRUE(operation.IsPending());
    EXPECT_FALSE(operation.mWorkReleased);
    EXPECT_STREQ(operation.mLabel, "first");
    ASSERT_NE(operation.mWork, nullptr);
    EXPECT_EQ(*operation.mWork, 1);

    operation.Complete(CHIP_ERROR_CANCELLED);
    EXPECT_EQ(recorder.count, 1);
    EXPECT_EQ(recorder.status, CHIP_ERROR_CANCELLED);
    EXPECT_FALSE(operation.mWorkReleased);
    EXPECT_FALSE(operation.IsPending());

    operation.Start("second", std::make_unique<int>(2), callback);
    EXPECT_STREQ(operation.mLabel, "second");
    callback.Cancel();
    EXPECT_TRUE(operation.mWorkReleased);
    EXPECT_EQ(operation.mWork, nullptr); // the layer released what it was started with
    EXPECT_EQ(recorder.count, 1);
}

/**
 * An operation that takes ownership of a payload when it starts, and hands it back on completion.
 * PayloadArg selects how the completion signature carries it: `Payload` by value, or `Payload &&`.
 * Note the latter is baked into the class type parameter, i.e. is not a forwarding reference.
 */
template <typename PayloadArg>
class PayloadOperation : public CancelableOperation<CHIP_ERROR, PayloadArg>
{
public:
    using Base = CancelableOperation<CHIP_ERROR, PayloadArg>;
    using typename Base::Completion;

    void Begin(typename Completion::Owned onCompletion, Payload payload)
    {
        mPayload = std::move(payload);
        Base::Start(std::move(onCompletion));
    }

    void Finish() { Base::Complete(CHIP_NO_ERROR, std::move(mPayload)); }

protected:
    void OnFinished(bool cancelled) override
    {
        if (cancelled)
        {
            mPayload.reset();
        }
        Base::OnFinished(cancelled);
    }

private:
    Payload mPayload;
};

class PayloadRecorder
{
public:
    int mCount         = 0;
    CHIP_ERROR mStatus = CHIP_ERROR_INTERNAL;
    Payload mPayload;

    // When set, the completion handler restarts this operation with a payload holding
    // mRestartValue, but only after having moved away the payload it was called with.
    PayloadOperation<Payload &&> * mRestartOperation            = nullptr;
    PayloadOperation<Payload &&>::Completion * mRestartCallback = nullptr;
    int mRestartValue                                           = 0;

    static void OnCompletionByValue(void * context, CHIP_ERROR status, Payload payload)
    {
        static_cast<PayloadRecorder *>(context)->Record(status, std::move(payload));
    }

    static void OnCompletionByRvalueRef(void * context, CHIP_ERROR status, Payload && payload)
    {
        static_cast<PayloadRecorder *>(context)->Record(status, std::move(payload));
    }

private:
    void Record(CHIP_ERROR status, Payload payload)
    {
        mCount++;
        mStatus  = status;
        mPayload = std::move(payload);

        if (auto * operation = mRestartOperation)
        {
            mRestartOperation = nullptr;
            operation->Begin(*mRestartCallback, std::make_unique<int>(mRestartValue));
        }
    }
};

TEST(CancelableOperationTest, MoveOnlyCompletionArgumentPassedByValue)
{
    using Operation = PayloadOperation<Payload>;

    PayloadRecorder recorder;
    Operation::Completion callback(PayloadRecorder::OnCompletionByValue, &recorder);
    Operation operation;

    operation.Begin(callback, std::make_unique<int>(11));
    EXPECT_TRUE(operation.IsPending());

    operation.Finish();
    EXPECT_EQ(recorder.mCount, 1);
    EXPECT_EQ(recorder.mStatus, CHIP_NO_ERROR);
    ASSERT_NE(recorder.mPayload, nullptr);
    EXPECT_EQ(*recorder.mPayload, 11);
    EXPECT_FALSE(operation.IsPending());
    EXPECT_FALSE(callback.IsRegistered());
}

TEST(CancelableOperationTest, MoveOnlyCompletionArgumentPassedByRvalueReference)
{
    using Operation = PayloadOperation<Payload &&>;

    PayloadRecorder recorder;
    Operation::Completion callback(PayloadRecorder::OnCompletionByRvalueRef, &recorder);
    Operation operation;

    operation.Begin(callback, std::make_unique<int>(22));
    operation.Finish();
    EXPECT_EQ(recorder.mCount, 1);
    ASSERT_NE(recorder.mPayload, nullptr);
    EXPECT_EQ(*recorder.mPayload, 22);
    EXPECT_FALSE(operation.IsPending());
    EXPECT_FALSE(callback.IsRegistered());
}

// The operation hands back a reference to storage it owns, so a caller accepting an rvalue
// reference has to move the payload away before reusing the operation. Done in that order, both
// the payload it was given and the restarted operation come out intact.
TEST(CancelableOperationTest, RvalueReferencePayloadConsumedBeforeReuse)
{
    using Operation = PayloadOperation<Payload &&>;

    PayloadRecorder recorder;
    Operation::Completion callback(PayloadRecorder::OnCompletionByRvalueRef, &recorder);
    Operation operation;
    recorder.mRestartOperation = &operation;
    recorder.mRestartCallback  = &callback;
    recorder.mRestartValue     = 44;

    operation.Begin(callback, std::make_unique<int>(22));
    operation.Finish();

    EXPECT_EQ(recorder.mCount, 1);
    ASSERT_NE(recorder.mPayload, nullptr);
    EXPECT_EQ(*recorder.mPayload, 22);
    EXPECT_TRUE(operation.IsPending());

    operation.Finish();
    EXPECT_EQ(recorder.mCount, 2);
    ASSERT_NE(recorder.mPayload, nullptr);
    EXPECT_EQ(*recorder.mPayload, 44);
    EXPECT_FALSE(operation.IsPending());
}

// The payload an operation is holding is released when the caller withdraws its callback, rather
// than being handed back through a completion that must not happen.
TEST(CancelableOperationTest, MoveOnlyPayloadIsReleasedOnCancellation)
{
    using Operation = PayloadOperation<Payload>;

    PayloadRecorder recorder;
    Operation::Completion callback(PayloadRecorder::OnCompletionByValue, &recorder);
    Operation operation;

    operation.Begin(callback, std::make_unique<int>(33));
    callback.Cancel();

    EXPECT_EQ(recorder.mCount, 0);
    EXPECT_EQ(recorder.mPayload, nullptr);
    EXPECT_FALSE(operation.IsPending());
}

// The hook fires on the completion path too, with cancelled = false, and it does so *before* the
// caller's callback runs: an intermediate layer's in-flight work is torn down by the time the
// callback can reuse or destroy the operation.
TEST(CancelableOperationTest, FinishedBeforeCompletionCallbackRuns)
{
    TestOperation operation;
    TestCaller caller;
    caller.mOperation = &operation;

    operation.Start(caller.mCallback);
    EXPECT_EQ(operation.mFinishedCount, 0);

    operation.Complete(CHIP_NO_ERROR, 3);

    EXPECT_EQ(operation.mFinishedCount, 1);
    EXPECT_EQ(operation.mCancelledCount, 0);
    EXPECT_EQ(operation.mLastFinishedCancelled, std::optional(false));

    // Already finished by the time the callback observed it, and no longer pending either.
    EXPECT_EQ(caller.mFinishedCountDuringCompletion, std::optional<int>(1));
    ASSERT_TRUE(operation.mPendingDuringFinished.has_value());
    EXPECT_FALSE(*operation.mPendingDuringFinished);
}

// Exactly one OnFinished() per started operation, whichever way it ends.
TEST(CancelableOperationTest, FinishedExactlyOncePerOperation)
{
    TestOperation operation;
    TestCaller caller;
    caller.mOperation = &operation;

    operation.Start(caller.mCallback);
    operation.Complete(CHIP_NO_ERROR, 1);
    EXPECT_EQ(operation.mFinishedCount, 1);

    operation.Start(caller.mCallback);
    caller.mCallback.Cancel();
    EXPECT_EQ(operation.mFinishedCount, 2);
    EXPECT_EQ(operation.mCancelledCount, 1);

    // Cancelling again after the operation has already finished is a no-op.
    caller.mCallback.Cancel();
    EXPECT_EQ(operation.mFinishedCount, 2);
}

// A restart from within the completion is a fresh operation, so it gets its own OnFinished().
TEST(CancelableOperationTest, FinishedAgainForAnOperationRestartedFromTheCompletion)
{
    TestOperation operation;
    TestCaller caller;
    caller.mOperation       = &operation;
    caller.mReentrantAction = [](TestCaller & self) { self.mOperation->Start(self.mCallback); };

    operation.Start(caller.mCallback);
    operation.Complete(CHIP_NO_ERROR, 1);
    EXPECT_EQ(operation.mFinishedCount, 1);
    EXPECT_TRUE(operation.IsPending());

    operation.Complete(CHIP_NO_ERROR, 2);
    EXPECT_EQ(operation.mFinishedCount, 2);
    EXPECT_EQ(operation.mCancelledCount, 0);
}

// An operation may free itself from the completion path of the hook as well, which is what lets a
// pooled operation return itself to the pool without the leaf having to arrange it.
TEST(CancelableOperationTest, DestroyedFromWithinOnFinishedWhileCompleting)
{
    bool destroyed = false;

    class SelfFreeingOnCompletion : public TestOperationBase
    {
    public:
        using TestOperationBase::Complete;
        using TestOperationBase::Start;

        explicit SelfFreeingOnCompletion(bool * destroyed) : mDestroyed(destroyed) {}
        ~SelfFreeingOnCompletion() override { *mDestroyed = true; }

    protected:
        void OnFinished(bool cancelled) override
        {
            TestOperationBase::OnFinished(cancelled);
            delete this;
        }

    private:
        bool * mDestroyed;
    };

    TestCaller caller;
    auto * operation = new SelfFreeingOnCompletion(&destroyed);
    operation->Start(caller.mCallback);
    ASSERT_FALSE(destroyed);

    // Complete() must not touch the operation after TakeCompletion() returns, so the callback is
    // still delivered even though the operation is already gone.
    operation->Complete(CHIP_ERROR_TIMEOUT, 8);
    EXPECT_TRUE(destroyed);
    EXPECT_EQ(caller.mCompletionCount, 1);
    EXPECT_EQ(caller.mStatus, CHIP_ERROR_TIMEOUT);
    EXPECT_EQ(caller.mValue, 8);
    EXPECT_FALSE(caller.mCallback.IsRegistered());
}

/**
 * A layer whose Start() is fallible, to pin that TypedOperation forwards the result. It decides
 * before taking ownership of the completion, so a rejection leaves nothing pending and owes no
 * callback -- the Owned token is simply dropped, having already cancelled at the call boundary.
 */
class FallibleOperationBase : public CancelableOperationBase
{
public:
    CHIP_ERROR mAcceptedWith = CHIP_ERROR_INTERNAL;

protected:
    CHIP_ERROR Start(bool accept, Cancelable::Owned onCompletion)
    {
        VerifyOrReturnError(accept, CHIP_ERROR_INVALID_ARGUMENT);
        CancelableOperationBase::Start(std::move(onCompletion));
        mAcceptedWith = CHIP_NO_ERROR;
        return CHIP_NO_ERROR;
    }
};

using FallibleLeafBase = TypedOperation<FallibleOperationBase, CHIP_ERROR>;

class FallibleOperation : public FallibleLeafBase
{
public:
    using FallibleLeafBase::Complete;
    using FallibleLeafBase::Start;

    int mFinishedCount = 0;

protected:
    void OnFinished(bool cancelled) override
    {
        mFinishedCount++;
        FallibleOperationBase::OnFinished(cancelled);
    }
};

// TypedOperation must also be nameable unqualified from inside a subclass body, where the base
// contributes the name of the implementation template it resolves to as an injected-class-name.
// Compile-time only, so there is no test of its own below.
class UnqualifiedTypedOperationSubclass : public TypedOperation<FallibleOperationBase, CHIP_ERROR>
{
public:
    using Base = TypedOperation<FallibleOperationBase, CHIP_ERROR>;
    using Base::Complete;
    using Base::Start;
};

// The CHIP_ERROR from the layer's Start() reaches the caller through the typed Start().
TEST(CancelableOperationTest, FallibleStartForwardsItsResult)
{
    struct Recorder
    {
        int count = 0;
        static void OnCompletion(void * context, CHIP_ERROR) { static_cast<Recorder *>(context)->count++; }
    } recorder;

    FallibleOperation::Completion callback(Recorder::OnCompletion, &recorder);
    FallibleOperation operation;

    // Rejected: no completion is owed, nothing is pending, and the callback was still cancelled at
    // the call boundary, so it is left registered with nobody.
    EXPECT_EQ(operation.Start(false, callback), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(operation.IsPending());
    EXPECT_FALSE(callback.IsRegistered());
    EXPECT_EQ(operation.mFinishedCount, 0);
    EXPECT_EQ(recorder.count, 0);

    // Accepted: the usual contract applies from here on.
    EXPECT_EQ(operation.Start(true, callback), CHIP_NO_ERROR);
    EXPECT_TRUE(operation.IsPending());
    EXPECT_TRUE(callback.IsRegistered());

    operation.Complete(CHIP_NO_ERROR);
    EXPECT_EQ(recorder.count, 1);
    EXPECT_EQ(operation.mFinishedCount, 1);
    EXPECT_FALSE(operation.IsPending());
}

// A rejection must not disturb an operation that is already in flight elsewhere -- except via the
// Owned token's boundary Cancel(), which withdraws the callback from whoever held it.
TEST(CancelableOperationTest, FallibleStartRejectionCancelsAPendingOperation)
{
    FallibleOperation::Completion callback([](void *, CHIP_ERROR) {}, nullptr);
    FallibleOperation first, second;

    EXPECT_EQ(first.Start(true, callback), CHIP_NO_ERROR);
    EXPECT_TRUE(first.IsPending());

    EXPECT_EQ(second.Start(false, callback), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(first.IsPending());
    EXPECT_EQ(first.mFinishedCount, 1);
    EXPECT_FALSE(second.IsPending());
    EXPECT_EQ(second.mFinishedCount, 0);
    EXPECT_FALSE(callback.IsRegistered());
}

// void Start() still works: the forwarding `return` of a void expression is well-formed.
static_assert(
    std::is_same_v<decltype(std::declval<TestOperation &>().Start(std::declval<TestOperationBase::Completion &>())), void>,
    "a void Start() forwards as void");
static_assert(
    std::is_same_v<decltype(std::declval<FallibleOperation &>().Start(true, std::declval<FallibleOperation::Completion &>())),
                   CHIP_ERROR>,
    "a fallible Start() forwards its CHIP_ERROR");

/**
 * A layer between the root and the leaf, to pin that OnFinished() chains all the way up. Records
 * into a trace so that the order of the layers is observable, not just the fact that both ran.
 */
std::vector<std::string> gFinishedTrace;

class ChainMiddleBase : public CancelableOperationBase
{
protected:
    void OnFinished(bool cancelled) override
    {
        gFinishedTrace.push_back(cancelled ? "middle:cancelled" : "middle:completed");
        CancelableOperationBase::OnFinished(cancelled);
    }
};

using ChainLeafBase = TypedOperation<ChainMiddleBase, CHIP_ERROR>;

class ChainLeafOperation : public ChainLeafBase
{
public:
    using ChainLeafBase::Complete;
    using ChainLeafBase::Start;

protected:
    void OnFinished(bool cancelled) override
    {
        gFinishedTrace.push_back(cancelled ? "leaf:cancelled" : "leaf:completed");
        ChainMiddleBase::OnFinished(cancelled);
    }
};

TEST(CancelableOperationTest, HookChainsThroughIntermediateLayers)
{
    gFinishedTrace.clear();

    ChainLeafOperation::Completion callback([](void *, CHIP_ERROR) {}, nullptr);
    ChainLeafOperation operation;

    operation.Start(callback);
    operation.Complete(CHIP_NO_ERROR);
    operation.Start(callback);
    callback.Cancel();

    EXPECT_EQ(gFinishedTrace,
              (std::vector<std::string>{ "leaf:completed", "middle:completed", "leaf:cancelled", "middle:cancelled" }));
}

} // namespace
