/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <luna-service2/lunaservice.h>

#include <glib.h>

#include <exception>

namespace LS {

class Error : public std::exception
{
public:
    const char * what() const noexcept override { return "luna-service2 fuzzer stub"; }
};

class Message
{
public:
    explicit Message(LSMessage * message) : mMessage(message) {}

    const char * getPayload() const { return LSMessageGetPayload(mMessage); }

private:
    LSMessage * mMessage = nullptr;
};

class Call
{
public:
    Call() = default;
    explicit Call(bool active) : mActive(active) {}

    bool isActive() const { return mActive; }
    void cancel() { mActive = false; }

    void continueWith(LSFilterFunc callback, void * ctx)
    {
        mCallback = callback;
        mContext  = ctx;
    }

private:
    bool mActive           = false;
    LSFilterFunc mCallback = nullptr;
    void * mContext        = nullptr;
};

class Handle
{
public:
    Handle() = default;
    explicit Handle(const char * name) { mHandle.name = name; }
    Handle(const char * name, const char * appId)
    {
        (void) appId;
        mHandle.name = name;
    }

    LSHandle * get() { return &mHandle; }
    const LSHandle * get() const { return &mHandle; }

    void attachToLoop(GMainLoop * loop) { mLoop = loop; }
    void detach() { mLoop = nullptr; }

    Call callOneReply(const char * uri, const char * payload, LSFilterFunc callback, void * ctx)
    {
        (void) uri;
        (void) payload;

        if (callback != nullptr)
        {
            LSMessage reply{ "{\"returnValue\":true}" };
            callback(&mHandle, &reply, ctx);
        }

        return Call(false);
    }

    Call callMultiReply(const char * uri, const char * payload)
    {
        (void) uri;
        (void) payload;

        return Call(true);
    }

private:
    LSHandle mHandle  = {};
    GMainLoop * mLoop = nullptr;
};

inline Handle registerService(const char * name)
{
    return Handle(name);
}

} // namespace LS
