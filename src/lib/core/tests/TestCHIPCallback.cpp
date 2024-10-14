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

using namespace chip::Callback;

/**
 * An example Callback registrar. Resumer::Resume() accepts Callbacks
 *   to be run during the next call to Resumer::Dispatch().  In an environment
 *   completely driven by callbacks, an application's main() would just call
 *   something like Resumer::Dispatch() in a loop.
 */
class Resumer : private CallbackDeque
{
public:
    /**
     * @brief run this callback next Dispatch
     */
    void Resume(Callback<> * cb)
    {
        // always first thing: cancel to take ownership of
        //  cb members
        Enqueue(cb->Cancel());
    }

    void Dispatch()
    {
        Cancelable ready;

        DequeueAll(ready);

        // runs the ready list
        while (ready.mNext != &ready)
        {
            Callback<> * cb = Callback<>::FromCancelable(ready.mNext);

            // one-shot semantics
            cb->Cancel();
            cb->mCall(cb->mContext);
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

            Callback<NotifyFn> * cb = Callback<NotifyFn>::FromCancelable(ca);
            cb->mCall(cb->mContext, v);
        }
    }

    /**
     * @brief example
     */
    static void Cancel(Cancelable * cb)
    {
        Dequeue(cb); // take off ready list
    }

    /**
     * @brief illustrate a case where this needs notification of cancellation
     */
    void Register(Callback<NotifyFn> * cb) { Enqueue(cb->Cancel(), Cancel); }
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
