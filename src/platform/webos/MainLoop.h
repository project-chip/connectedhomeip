/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>

#include <glib.h>
#include <luna-service2/lunaservice.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

typedef gboolean (*initFn_t)(gpointer userData);
typedef gboolean (*asyncFn_t)(GMainLoop * mainLoop, gpointer userData);
typedef void (*timeoutFn_t)(gpointer userData);

class LoopData
{
public:
    LoopData() : mMainContext(NULL), mMainLoop(NULL), mThread(NULL), mTimeoutFn(NULL), mTimeoutUserData(NULL) {}

    GMainContext * mMainContext;
    GMainLoop * mMainLoop;
    GThread * mThread;
    timeoutFn_t mTimeoutFn;
    gpointer mTimeoutUserData;
};

class MainLoop
{
public:
    bool Init(initFn_t initFn, gpointer userData = NULL);
    void Deinit(void);
    bool AsyncRequest(asyncFn_t asyncFn, gpointer asyncUserData = NULL, guint interval = 0, timeoutFn_t timeoutFn = NULL,
                      gpointer timeoutUserData = NULL);

    bool StartLSMainLoop(void);

    static MainLoop & Instance(void);

    LSHandle * mLSHandle;

private:
    MainLoop() {}
    void DeleteData(LoopData * loopData);
    static gboolean ThreadTimeout(gpointer userData);
    void SetThreadTimeout(LoopData * loopData, guint interval);
    static gpointer ThreadMainHandler(gpointer data);
    static gpointer ThreadAsyncHandler(gpointer data);
    static gpointer ThreadStartLSMainLoopHandler(gpointer data);

    std::vector<LoopData *> mLoopData;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
