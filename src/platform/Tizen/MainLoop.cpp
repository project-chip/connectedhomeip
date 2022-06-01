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

#include "MainLoop.h"

#include <errno.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <pthread.h>
#include <system/SystemTimer.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

gboolean MainLoop::ThreadTimeout(gpointer userData)
{
    LoopData * loopData = reinterpret_cast<LoopData *>(userData);

    VerifyOrReturnError(loopData != nullptr, G_SOURCE_REMOVE);

    if (loopData->mTimeoutFn)
    {
        ChipLogProgress(DeviceLayer, "[Timeout] thread timeout function");
        loopData->mTimeoutFn(loopData->mTimeoutUserData);
    }

    if (loopData->mMainLoop)
    {
        ChipLogProgress(DeviceLayer, "[Timeout] main loop %p", loopData->mMainLoop);
        g_main_loop_quit(loopData->mMainLoop);
    }

    return G_SOURCE_REMOVE;
}

void MainLoop::ThreadMainHandler(LoopData * loopData)
{
    g_main_context_push_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[PUSH] main context %p", loopData->mMainContext);

    ChipLogProgress(DeviceLayer, "[RUN] main loop %p", loopData->mMainLoop);
    g_main_loop_run(loopData->mMainLoop);
    ChipLogProgress(DeviceLayer, "[QUIT] main loop %p", loopData->mMainLoop);
    g_main_loop_unref(loopData->mMainLoop);
    loopData->mMainLoop = nullptr;

    g_main_context_pop_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[POP] main context %p", loopData->mMainContext);
    g_main_context_unref(loopData->mMainContext);
    loopData->mMainContext = nullptr;
}

void MainLoop::ThreadAsyncHandler(LoopData * loopData)
{
    g_main_context_push_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[PUSH] async context %p", loopData->mMainContext);

    ChipLogProgress(DeviceLayer, "[RUN] async loop %p", loopData->mMainLoop);
    g_main_loop_run(loopData->mMainLoop);
    ChipLogProgress(DeviceLayer, "[QUIT] async loop %p", loopData->mMainLoop);
    g_main_loop_unref(loopData->mMainLoop);
    loopData->mMainLoop = nullptr;

    g_main_context_pop_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[POP] async context %p", loopData->mMainContext);
    g_main_context_unref(loopData->mMainContext);
    loopData->mMainContext = nullptr;

    chip::Platform::Delete(loopData);
}

bool MainLoop::Init(initFn_t initFn, gpointer userData)
{
    bool result;
    LoopData * loopData = chip::Platform::New<LoopData>();

    VerifyOrReturnError(loopData != nullptr, false);

    loopData->mMainContext = g_main_context_new();
    loopData->mMainLoop    = g_main_loop_new(loopData->mMainContext, FALSE);

    g_main_context_push_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[PUSH] main context %p", loopData->mMainContext);

    result = initFn(userData);

    g_main_context_pop_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[POP] main context %p", loopData->mMainContext);

    VerifyOrReturnError(result, false);

    loopData->mThread = std::thread(ThreadMainHandler, loopData);
    mLoopData.push_back(loopData);

    return true;
}

void MainLoop::DeleteData(LoopData * loopData)
{
    if (loopData->mMainLoop)
    {
        g_main_loop_quit(loopData->mMainLoop);
        loopData->mThread.join();
    }

    chip::Platform::Delete(loopData);
}

void MainLoop::Deinit(void)
{
    std::vector<LoopData *>::const_iterator iter = mLoopData.cbegin();
    while (iter != mLoopData.cend())
    {
        DeleteData(*iter);
        mLoopData.erase(iter);
        iter++;
    }
}

bool MainLoop::AsyncRequest(asyncFn_t asyncFn, gpointer asyncUserData, guint timeoutInterval, timeoutFn_t timeoutFn,
                            gpointer timeoutUserData)
{
    bool result         = false;
    LoopData * loopData = chip::Platform::New<LoopData>();

    VerifyOrReturnError(loopData != nullptr, false);

    loopData->mMainContext     = g_main_context_new();
    loopData->mMainLoop        = g_main_loop_new(loopData->mMainContext, FALSE);
    loopData->mTimeoutFn       = timeoutFn;
    loopData->mTimeoutUserData = timeoutUserData;

    g_main_context_push_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[PUSH] async context %p", loopData->mMainContext);

    result = asyncFn(loopData->mMainLoop, asyncUserData);

    g_main_context_pop_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[POP] async context %p", loopData->mMainContext);

    VerifyOrReturnError(result, false);

    loopData->mThread = std::thread(ThreadAsyncHandler, loopData);
    loopData->mThread.detach();

    if (timeoutInterval)
    {
        GSource * source = g_timeout_source_new_seconds(timeoutInterval);
        g_source_set_callback(source, ThreadTimeout, loopData, nullptr);
        g_source_attach(source, loopData->mMainContext);
        g_source_unref(source);
    }

    return true;
}

MainLoop & MainLoop::Instance(void)
{
    static MainLoop sMainLoop;
    return sMainLoop;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
