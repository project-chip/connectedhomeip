/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include <core/CHIPError.h>

namespace example {
struct Timer
{
    typedef void (*Callback)(Timer & timer);

    virtual ~Timer() = default;
    Timer(const char * name, uint32_t timeoutInMs, Callback callback, void * context) :
        mName(name), mTimeoutInMs(timeoutInMs), mCallback(callback), mContext(context)
    {}

    const char * mName;
    uint32_t mTimeoutInMs;
    Callback mCallback = nullptr;
    void * mContext    = nullptr;
    bool mIsActive     = false;

    virtual void Start() = 0;
    virtual void Stop()  = 0;
    virtual void Timeout()
    {
        mIsActive = false;
        if (mCallback)
        {
            mCallback(*this);
        }
    }
};

template <typename EventType>
class App
{

public:
    virtual ~App() {}
    virtual CHIP_ERROR Init()                       = 0;
    virtual void PostEvent(const EventType & event) = 0;

protected:
    virtual void ProcessEvents()                         = 0;
    virtual void DispatchEvent(const EventType & event)  = 0;
    virtual example::Timer * CreateTimer(const char * name, uint32_t timeoutInMs, example::Timer::Callback callback,
                                         void * context) = 0;
    virtual void DestroyTimer(example::Timer * timer)    = 0;
};

} // namespace example
