/*
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

#include <memory>
#include <thread>

#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>

#include <glib.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

typedef gboolean (*initFn_t)(gpointer userData);
typedef gboolean (*asyncFn_t)(GMainLoop * mainLoop, gpointer userData);
typedef void (*timeoutFn_t)(gpointer userData);

class LoopData
{
public:
    LoopData();
    ~LoopData();

    // Thread which governs glib main event loop
    std::thread mThread;
    // Objects for running glib main event loop
    GMainContext * mMainContext = nullptr;
    GMainLoop * mMainLoop       = nullptr;
    // Optional timeout function
    timeoutFn_t mTimeoutFn    = nullptr;
    gpointer mTimeoutUserData = nullptr;
};

class MainLoop
{
public:
    bool Init(initFn_t initFn, gpointer userData = nullptr);
    void Deinit(void);
    bool AsyncRequest(asyncFn_t asyncFn, gpointer asyncUserData = nullptr, guint timeoutInterval = 0,
                      timeoutFn_t timeoutFn = nullptr, gpointer timeoutUserData = nullptr);
    static MainLoop & Instance(void);

private:
    MainLoop() = default;

    static gboolean ThreadTimeout(gpointer userData);
    static void ThreadHandler(std::shared_ptr<LoopData> loopData);

    std::vector<std::shared_ptr<LoopData>> mLoopData;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
