/*
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2021 SmartThings
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

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/StateMachine.h>
#include <lib/support/Variant.h>

namespace {

struct Event1
{
};
struct Event2
{
};
struct Event3
{
};
struct Event4
{
};
struct Event5
{
};

using Event   = chip::Variant<Event1, Event2, Event3, Event4, Event5>;
using Context = chip::StateMachine::Context<Event>;

struct MockState
{
    unsigned mEntered;
    unsigned mExited;
    unsigned mLogged;
    const char * mPrevious;

    void Enter() { ++mEntered; }
    void Exit() { ++mExited; }
    void LogTransition(const char * previous)
    {
        ++mLogged;
        mPrevious = previous;
    }
};

struct BaseState
{
    void Enter() { mMock.Enter(); }
    void Exit() { mMock.Exit(); }
    void LogTransition(const char * previous) { mMock.LogTransition(previous); }
    const char * GetName() { return mName; }

    Context * mCtx;
    const char * mName;
    MockState & mMock;
};

struct State1 : public BaseState
{
    State1(Context * ctx, MockState & mock) : BaseState{ ctx, "State1", mock } {}
};

struct State2 : public BaseState
{
    State2(Context * ctx, MockState & mock) : BaseState{ ctx, "State2", mock } {}
};

struct State3 : public BaseState
{
    State3(Context * ctx, MockState & mock) : BaseState{ ctx, "State3", mock } {}
    void Enter()
    {
        BaseState::Enter();
        if (this->mCtx)
        {
            this->mCtx->Dispatch(Event::Create<Event5>());
        }
    }
};

// Place State3 first in the variant.  This can evoke the behavior that
// TestNestedDispatch is looking for.
using State = chip::StateMachine::VariantState<State3, State2, State1>;

struct StateFactory
{
    Context * mCtx;
    MockState ms1{ 0, 0, 0, nullptr };
    MockState ms2{ 0, 0, 0, nullptr };
    MockState ms3{ 0, 0, 0, nullptr };

    StateFactory(Context * ctx) : mCtx(ctx) {}

    auto CreateState1() { return State::Create<State1>(mCtx, ms1); }
    auto CreateState2() { return State::Create<State2>(mCtx, ms2); }
    auto CreateState3() { return State::Create<State3>(mCtx, ms3); }
};

struct Transitions
{
    Context * mCtx;
    StateFactory mFactory;
    Transitions(Context * ctx) : mCtx(ctx), mFactory(ctx) {}

    using OptState = chip::StateMachine::Optional<State>;
    State GetInitState() { return mFactory.CreateState1(); }
    OptState operator()(const State & state, const Event & event)
    {
        if (state.Is<State1>() && event.Is<Event2>())
        {
            return mFactory.CreateState2();
        }
        if (state.Is<State2>() && event.Is<Event1>())
        {
            return mFactory.CreateState1();
        }
        if (state.Is<State1>() && event.Is<Event4>())
        {
            // legal - Dispatches event without transition
            if (mCtx)
            {
                mCtx->Dispatch(Event::Create<Event2>());
            }
            return {};
        }
        if (state.Is<State2>() && event.Is<Event4>())
        {
            // mCtx.Dispatch(Event::Create<Event2>()); // dispatching an event and returning a transition would be illegal
            return mFactory.CreateState1();
        }
        if (state.Is<State1>() && event.Is<Event5>())
        {
            return mFactory.CreateState3();
        }
        if (state.Is<State3>() && event.Is<Event5>())
        {
            return mFactory.CreateState2();
        }

        return {};
    }
};

class SimpleStateMachine
{
public:
    Transitions mTransitions;
    chip::StateMachine::StateMachine<State, Event, Transitions> mStateMachine;

    SimpleStateMachine() : mTransitions(&mStateMachine), mStateMachine(mTransitions) {}
    ~SimpleStateMachine() {}
};

TEST(TestStateMachine, TestInit)
{
    // state machine initializes to State1
    SimpleStateMachine fsm;
    EXPECT_TRUE(fsm.mStateMachine.GetState().Is<State1>());
}

TEST(TestStateMachine, TestIgnoredEvents)
{
    // in State1 - ignore Event1 and Event3
    SimpleStateMachine fsm;
    fsm.mStateMachine.Dispatch(Event::Create<Event1>());
    EXPECT_TRUE(fsm.mStateMachine.GetState().Is<State1>());
    fsm.mStateMachine.Dispatch(Event::Create<Event3>());
    EXPECT_TRUE(fsm.mStateMachine.GetState().Is<State1>());
    // transition to State2
    fsm.mStateMachine.Dispatch(Event::Create<Event2>());
    EXPECT_TRUE(fsm.mStateMachine.GetState().Is<State2>());
    // in State2 - ignore Event2 and Event3
    fsm.mStateMachine.Dispatch(Event::Create<Event2>());
    EXPECT_TRUE(fsm.mStateMachine.GetState().Is<State2>());
    fsm.mStateMachine.Dispatch(Event::Create<Event3>());
    EXPECT_TRUE(fsm.mStateMachine.GetState().Is<State2>());
}

TEST(TestStateMachine, TestTransitions)
{
    // in State1
    SimpleStateMachine fsm;
    // dispatch Event2 to transition to State2
    fsm.mStateMachine.Dispatch(Event::Create<Event2>());
    EXPECT_TRUE(fsm.mStateMachine.GetState().Is<State2>());
    // dispatch Event1 to transition back to State1
    fsm.mStateMachine.Dispatch(Event::Create<Event1>());
    EXPECT_TRUE(fsm.mStateMachine.GetState().Is<State1>());
    // dispatch Event2 to transition to State2
    fsm.mStateMachine.Dispatch(Event::Create<Event2>());
    EXPECT_TRUE(fsm.mStateMachine.GetState().Is<State2>());
    // dispatch Event4 to transitions to State1.
    fsm.mStateMachine.Dispatch(Event::Create<Event4>());
    EXPECT_TRUE(fsm.mStateMachine.GetState().Is<State1>());
}

TEST(TestStateMachine, TestTransitionsDispatch)
{
    // in State1
    SimpleStateMachine fsm;
    // Dispatch Event4, which in turn dispatches Event2 from the transitions
    // table and ultimately places us in State2.
    fsm.mStateMachine.Dispatch(Event::Create<Event4>());
    EXPECT_TRUE(fsm.mStateMachine.GetState().Is<State2>());
}

TEST(TestStateMachine, TestNestedDispatch)
{
    // in State1
    SimpleStateMachine fsm;
    // Dispatch Event5, which places us into State3, which will dispatch
    // Event5 again from its Enter method to place us into State2.
    fsm.mStateMachine.Dispatch(Event::Create<Event5>());
    EXPECT_TRUE(fsm.mStateMachine.GetState().Is<State2>());
    // Make sure that Enter methods execute the correct number of times.
    // This helps verify that pattern matching is working correctly.
    // Specifically, we need to verify this case: State3 creates State2
    // by dispatching Event5 from its Enter method.  This means that the
    // Dispatch call from State3 also destructs State3.  If the State3
    // Enter method pattern matching triggers Enter more than once, this
    // is use-after-destruction.  What can appear to happen is that the
    // State2 Enter method will execute twice, as State2 will already have
    // been constructed when the State3 Enter method executes a second
    // time.  The state machine pattern matching has code to explicitly
    // prevent this double-execution.  This is testing that.
    EXPECT_EQ(fsm.mTransitions.mFactory.ms1.mEntered, 0u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms1.mExited, 1u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms1.mLogged, 0u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms2.mEntered, 1u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms2.mExited, 0u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms2.mLogged, 1u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms3.mEntered, 1u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms3.mExited, 1u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms3.mLogged, 1u);
}

TEST(TestStateMachine, TestMethodExec)
{
    // in State1
    SimpleStateMachine fsm;
    // transition to State2
    fsm.mStateMachine.Dispatch(Event::Create<Event2>());
    EXPECT_TRUE(fsm.mStateMachine.GetState().Is<State2>());
    // verify expected method calls
    EXPECT_EQ(fsm.mTransitions.mFactory.ms1.mEntered, 0u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms1.mExited, 1u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms1.mLogged, 0u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms1.mPrevious, nullptr);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms2.mEntered, 1u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms2.mExited, 0u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms2.mLogged, 1u);
    EXPECT_STREQ(fsm.mTransitions.mFactory.ms2.mPrevious, "State1");
    // transition back to State1
    fsm.mStateMachine.Dispatch(Event::Create<Event1>());
    EXPECT_TRUE(fsm.mStateMachine.GetState().Is<State1>());
    // verify expected method calls
    EXPECT_EQ(fsm.mTransitions.mFactory.ms1.mEntered, 1u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms1.mExited, 1u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms1.mLogged, 1u);
    EXPECT_STREQ(fsm.mTransitions.mFactory.ms1.mPrevious, "State2");
    EXPECT_EQ(fsm.mTransitions.mFactory.ms2.mEntered, 1u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms2.mExited, 1u);
    EXPECT_EQ(fsm.mTransitions.mFactory.ms2.mLogged, 1u);
    EXPECT_STREQ(fsm.mTransitions.mFactory.ms2.mPrevious, "State1");
}

} // namespace
