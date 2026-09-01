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

#pragma once

#include <lib/core/CHIPCallback.h>
#include <lib/support/CodeUtils.h>

#include <type_traits>
#include <utility>

namespace chip {
namespace Callback {

/**
 * Base class for an asynchronous operation that accepts a Callback<T>, and enforces the contract
 * that the caller hears back exactly once.
 *
 * This class is primarily useful for cases where cancellation has to actually do some work to stop
 * an operation. In contrast, CallbackDeque or GroupedCallbackList are most suited to use cases
 * where several callers wait on some shared work, and cancellation simply unlinks an individual
 * Callback from that list.
 *
 * The contract a subclass has to keep is: an operation that has been started completes exactly
 * once, unless the caller cancels it, in which case it never completes. Cancellation is the only
 * way out without a completion, and it is the caller's to ask for, not the operation's to take.
 *
 * This is enforced via VerifyOrDie: it is illegal to start an operation that is already pending,
 * to complete one that isn't, or to destruct an operation that is still pending. Note that the
 * latter means that if an operation needs to be aborted by the callee, it must complete it with an
 * error; the obligation to call back to the caller can't be dropped.
 *
 * This class is deliberately storage-agnostic: it works as a member of whatever issues the
 * operations, or as an object in a pool, and it neither knows nor decides when it is destroyed.
 * In particular, an operation may be reused or destroyed from within its own completion callback.
 *
 * Completion signatures may take rvalue reference arguments, which is useful for handing back a
 * heavy payload without the operation having to move it first. Care must be taken with the
 * lifetime of objects passed by reference or non-owning types like ByteSpans, especially in cases
 * where destruction or reuse of the operation from within the callback is a possibility. The
 * recommended recipe is to move such objects into a local in the function that completes the
 * operation, so that its lifetime is the completion call rather than the operation's:
 *
 *     void Finish(CHIP_ERROR status)
 *     {
 *         System::PacketBufferHandle response = std::move(mResponse);
 *         Complete(status, ByteSpan(response->Start(), response->DataLength()));
 *     }
 */
class CancelableOperationBase
{
public:
    /**
     * True while an operation is in flight, i.e. from being started until it is completed or the
     * caller cancels it.
     */
    bool IsPending() const { return mCancelable != nullptr; }

    // Not copyable. Not movable; moving an operation would be hazardous because it could happen
    // reentrantly during completion calls. Any subclass holding a Callback to track underlying
    // work is also immovable by virtue of the underlying Cancelable.
    CancelableOperationBase(const CancelableOperationBase &)             = delete;
    CancelableOperationBase & operator=(const CancelableOperationBase &) = delete;
    CancelableOperationBase(CancelableOperationBase &&)                  = delete;
    CancelableOperationBase & operator=(CancelableOperationBase &&)      = delete;

protected:
    CancelableOperationBase() = default;

    /**
     * Asserts that we no longer have ownership of a caller's Callback, since a Cancel() on that
     * callback would call back into our CancelFn, triggering a use-after-free.
     */
    virtual ~CancelableOperationBase() { VerifyOrDie(!IsPending()); }

    /**
     * Takes over the given Cancelable, obtained from Callback::Cancel(), and registers this
     * operation as the callee holding it.
     *
     * An intermediate subclass that needs parameters of its own simply declares them ahead of the
     * completion; TypedOperation reads them off this declaration and forwards to it from its
     * type-safe Start() that takes a typed Callback<Args...>::Owned instead of an untyped
     * Cancelable::Owned. Note that Start() must not be overloaded.
     */
    void Start(Cancelable::Owned onCompletion)
    {
        VerifyOrDie(!IsPending());
        mCancelable = onCompletion.Take();

        // Taking the Cancelable and installing mCancel have to happen together, here: mCancel is
        // the only thing that makes our borrow visible to the Callback's owner, and keeping the two
        // in one place is what guarantees that whatever is in mCancel is ours, which in turn is
        // what makes TakeCompletion()'s unconditional clear of it sound.
        mCancelable->mContextA = this;
        mCancelable->mCancel   = [](Cancelable * aCancelable) {
            auto * self = static_cast<CancelableOperationBase *>(aCancelable->mContextA);

            // Cancelable::Cancel() has already cleared mCancel. Give up our reference before
            // telling the subclass, so that IsPending() is already false during OnFinished():
            // The subclass shouldn't attempt to call the callback in response to cancellation.
            self->mCancelable = nullptr;
            self->OnFinished(/* cancelled = */ true);
        };
    }

    /**
     * Unregisters the Cancelable from the operation and returns it in preparation for recovering
     * the typed Callback and invoking it. (The latter is usually handled via a subclass created
     * via the TypedOperation mixin.)
     *
     * Calls OnFinished(false) once the operation is no longer pending, i.e. subclasses have
     * released whatever they had in flight by the time the caller's callback runs.
     *
     * The caller of this method takes on the operation's debt of invoking the callback: the
     * operation is no longer pending, so nothing else will assert if the completion is dropped.
     */
    [[nodiscard]] Cancelable * TakeCompletion()
    {
        VerifyOrDie(IsPending());
        Cancelable * cancelable = mCancelable;

        // Unregister the cancelable, but don't call Cancel(), since that would invoke our own
        // cancellation handler and OnFinished(true). Releasing ownership of the Cancelable
        // *before* calling the completion is important, because the caller may well reuse its
        // Callback and register it with another callee immediately, resulting in a call to
        // Cancel(). Clearing mCancelable at the same time also ensures that the operation itself
        // is no longer pending, and therefore able to be reused synchronously from the callback.
        cancelable->mCancel = nullptr;
        cancelable->Invalidate();
        mCancelable = nullptr;

        OnFinished(/* cancelled = */ false);
        return cancelable;
    }

    /**
     * Called when the operation stops being pending, to release whatever resources or operations
     * it has in flight. When this method is called, IsPending() is already false, so an
     * implementation must not try to complete the operation.
     *
     * Called with cancelled = true when the *caller* withdrew its Callback, in which case no
     * completion will be delivered and everything the operation holds can be released. Called with
     * cancelled = false from TakeCompletion(), i.e. just *before* the caller's callback runs, so
     * that resources are cleaned up down before the callback can reuse or destroy the operation.
     *
     * The distinction matters only for state the operation intends to hand to the caller: releasing
     * it here would pull it out from under the completion. Resources or underlying work like nested
     * Callbacks, timers, etc. should be released unconditionally.
     *
     * A subclass that overrides this MUST call the inherited implementation.
     */
    virtual void OnFinished(bool cancelled) {}

private:
    Cancelable * mCancelable = nullptr;
};

namespace detail {

// Re-publishes OperationBase's protected Start(), so that a pointer to it can be formed:
// [class.protected] does not allow naming a protected member via OperationBase itself, and
// TypedOperation cannot name it via its own scope either, since its Start() hides the base one.
template <typename OperationBase>
struct StartProbe : OperationBase
{
    using OperationBase::Start;
};

// A list of types, carrying a parameter list as a single type argument.
template <typename... T>
struct ParameterList
{
};

// The return type and parameters of a pointer-to-member-function type. The class the member belongs
// to is deliberately dropped: a layer that adds no parameters of its own does not redeclare Start(),
// so the pointer is in terms of whichever ancestor did declare it.
template <typename T>
struct MemberFunctionSignature;
template <typename C, typename R, typename... A>
struct MemberFunctionSignature<R (C::*)(A...)>
{
    using Return     = R;
    using Parameters = ParameterList<A...>;
};

// The signature of OperationBase::Start(), i.e. what TypedOperation::Start() has to forward.
template <typename OperationBase>
using StartSignature = MemberFunctionSignature<decltype(&StartProbe<OperationBase>::Start)>;

// Drops the trailing Cancelable::Owned from a Start() parameter list, leaving the parameters the
// layer declares for itself. Recurses so long as there is more than one parameter left.
template <typename Kept, typename Remaining>
struct DropCompletionParameter
{
    // Both specializations below match a non-empty list, so this primary template is only ever
    // selected for a Start() that declares no parameters at all. Report the error with the same
    // static_assert message, rather than as an undefined-template error. Type is still defined so
    // that StartParameters below resolves, leaving just the one diagnostic. Note the condition is
    // always false, but has to be phrased in a way that depends on a template parameter.
    static_assert(std::is_same_v<Remaining, ParameterList<Cancelable::Owned>>,
                  "Start() must take the completion as a Cancelable::Owned in its last parameter");
    using Type = ParameterList<>;
};
template <typename... Kept, typename Last>
struct DropCompletionParameter<ParameterList<Kept...>, ParameterList<Last>>
{
    static_assert(std::is_same_v<Last, Cancelable::Owned>,
                  "Start() must take the completion as a Cancelable::Owned in its last parameter");
    using Type = ParameterList<Kept...>;
};
template <typename... Kept, typename First, typename Second, typename... Rest>
struct DropCompletionParameter<ParameterList<Kept...>, ParameterList<First, Second, Rest...>>
    : DropCompletionParameter<ParameterList<Kept..., First>, ParameterList<Second, Rest...>>
{
};

// The parameters OperationBase::Start() takes ahead of the completion.
template <typename OperationBase>
using StartParameters = typename DropCompletionParameter<ParameterList<>, typename StartSignature<OperationBase>::Parameters>::Type;

// Helper template implementing the TypedOperation alias below. It is necessary because a parameter
// pack can only be introduced by a template parameter list: there is no way to expand
// StartParameters<OperationBase> in place.
template <typename OperationBase, typename Parameters, typename... CompletionArgs>
class TypedOperation;
template <typename OperationBase, typename... StartArgs, typename... CompletionArgs>
class TypedOperation<OperationBase, ParameterList<StartArgs...>, CompletionArgs...> : public OperationBase
{
    static_assert(std::is_base_of_v<CancelableOperationBase, OperationBase>);

public:
    using Completion = Callback<void (*)(void * context, CompletionArgs... args)>;

protected:
    // Forwards the base class constructors, so that operation bases taking arguments can be used.
    // (Inherited constructors keep the access of the base constructor they name, not the access
    // in effect here.)
    using OperationBase::OperationBase;

    /**
     * Takes ownership of the provided callback and starts the operation, which must be completed
     * via Complete() unless cancelled by the caller.
     *
     * This hides OperationBase::Start(), so the parameters OperationBase declares are the only way
     * in: a subclass cannot start an operation without supplying them. They are forwarded, so an
     * rvalue reference among them hands a payload to the operation without a copy, mirroring what
     * the completion arguments do in the other direction.
     *
     * The return value of OperationBase::Start() is passed straight through. An OperationBase
     * that reports a synchronous rejection via the return value MUST decide before taking
     * ownership of the completion, i.e. before calling CancelableOperationBase::Start(): returning
     * an error while holding the caller's callback would claim no completion is coming and then
     * owe one anyway.
     */
    typename StartSignature<OperationBase>::Return Start(StartArgs... startArgs, typename Completion::Owned onCompletion)
    {
        return OperationBase::Start(std::forward<StartArgs>(startArgs)..., std::move(onCompletion));
    }

    /**
     * Completes the operation (which must be pending) by releasing ownership of the caller's
     * callback and invoking it with the provided arguments.
     *
     * Note that the operation is no longer pending by the time the caller's callback executes, so
     * if operation objects can be reused (e.g. via a pool), such reuse may occur reentrantly from
     * within the completion callback. Completion may also result in reentrant destruction of the
     * operation.
     */
    void Complete(CompletionArgs... args)
    {
        Completion::FromCancelable(OperationBase::TakeCompletion())->Invoke(std::forward<CompletionArgs>(args)...);
    }
};

} // namespace detail

/**
 * Adds a typed Start() and Complete() to a CancelableOperationBase subclass: Start() takes whatever
 * parameters OperationBase::Start() declares ahead of the completion, followed by a typed completion
 * callback, and Complete() invokes that callback. Whatever OperationBase::Start() returns is
 * forwarded, so a layer whose Start() is fallible keeps its CHIP_ERROR.
 *
 * This is the only part of the CancelableOperation type hierarchy that depends on the completion
 * signature. Separating it out in this way allows reusable operation subclasses to be compiled
 * once rather than expanded per signature.
 *
 * The completion signature is a single decision made by the concrete operation, whereas Start()
 * parameters accumulate down the hierarchy, each layer declaring the full list it takes; hence the
 * former is a trailing pack here and the latter is read off OperationBase::Start() itself.
 */
template <typename OperationBase, typename... CompletionArgs>
using TypedOperation = detail::TypedOperation<OperationBase, detail::StartParameters<OperationBase>, CompletionArgs...>;

/**
 * Convenience type for a type-safe cancelable operation. Subclasses of this class should be
 * concrete non-template operation implementations. Reusable base classes should instead subclass
 * CancelableOperationBase, declare a Start() taking their own parameters ahead of a
 * Cancelable::Owned, and offer their own convenience type alias over TypedOperation.
 */
template <typename... CompletionArgs>
using CancelableOperation = TypedOperation<CancelableOperationBase, CompletionArgs...>;

} // namespace Callback
} // namespace chip
