/*
 *
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

#include <app-common/zap-generated/enums.h>
#include <app/util/af-types.h>
#include <lib/core/CHIPError.h>

namespace example {

struct Task
{
    struct Runner
    {
        virtual ~Runner()                               = default;
        virtual CHIP_ERROR OnTaskRun(const Task & task) = 0;
    };
    Task(const char * name, Runner & run) : mName(name), mRunner(run) {}
    virtual ~Task()            = default;
    virtual CHIP_ERROR Start() = 0;
    virtual void Finish()      = 0;
    CHIP_ERROR Run() { return mRunner.OnTaskRun(*this); }
    const char * mName = nullptr;
    Runner & mRunner;
};

struct Event
{
    enum class Id
    {
        None = 0,
        Reset,
        Button0Pressed,
        Button0Released,
        Button1Pressed,
        Button1Released,
    };

    Event(Id id) : mId(id), mEndpoint(0) {}
    Event(Id id, chip::EndpointId endpoint) : mId(id), mEndpoint(endpoint) {}

    Id mId;
    chip::EndpointId mEndpoint;
};

struct Queue
{
    struct Dispatcher
    {
        virtual ~Dispatcher()                           = default;
        virtual CHIP_ERROR OnEvent(const Event & event) = 0;
    };
    Queue(Dispatcher & dispatcher) : mDispatcher(dispatcher) {}
    virtual ~Queue()                             = default;
    virtual CHIP_ERROR Init()                    = 0;
    virtual void Finish()                        = 0;
    virtual CHIP_ERROR Post(const Event & event) = 0;
    virtual void DispatchPending()               = 0;
    CHIP_ERROR Dispatch(const Event & event) { return mDispatcher.OnEvent(event); }
    Dispatcher & mDispatcher;
};

// struct Button
// {
//     enum class Id
//     {
//         Up   = 0,
//         Down = 1
//     };

//     struct Listener
//     {
//         virtual ~Listener()                           = default;
//         virtual CHIP_ERROR OnPressed(const Button & button) = 0;
//         virtual CHIP_ERROR OnReleased(const Button & button) = 0;
//     };

//     Button(Id id, const char * name, Listener &listen) : mId(id), mName(name), mListener(listen) {}
//     virtual ~Button() = default;
//     void Press() { mListener.OnPressed(*this); }
//     void Release() { mListener.OnReleased(*this); }

//     Id mId;
//     const char * mName = nullptr;
//     Listener &mListener;
// };

class MiniApp : public Task::Runner, Queue::Dispatcher
{
public:
    static MiniApp & Instance();

    virtual ~MiniApp() = default;
    virtual CHIP_ERROR Init();
    virtual void Finish();
    virtual CHIP_ERROR Start();

    CHIP_ERROR PostEvent(const Event & event);

protected:
    virtual Task * CreateTask(const char * name, Task::Runner & runner) = 0;
    virtual Queue * CreateQueue(Queue::Dispatcher & dispatcher)         = 0;
    virtual CHIP_ERROR Main();

private:
    CHIP_ERROR OnTaskRun(const Task & task) override;
    CHIP_ERROR OnEvent(const Event & event) override;
    Task * mTask   = nullptr;
    Queue * mQueue = nullptr;
};

} // namespace example
