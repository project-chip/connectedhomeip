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

    VerifyOrReturnError(loopData != NULL, G_SOURCE_REMOVE);

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

void MainLoop::SetThreadTimeout(LoopData * loopData, guint interval)
{
    VerifyOrReturn(loopData != NULL);

    GSource * source = g_timeout_source_new_seconds(interval);
    g_source_set_callback(source, ThreadTimeout, reinterpret_cast<gpointer>(loopData), NULL);
    g_source_attach(source, loopData->mMainContext);
    g_source_unref(source);
}

gpointer MainLoop::ThreadMainHandler(gpointer data)
{
    LoopData * loopData = reinterpret_cast<LoopData *>(data);

    g_main_context_push_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[PUSH] main context %p", loopData->mMainContext);

    ChipLogProgress(DeviceLayer, "[RUN] main loop %p", loopData->mMainLoop);
    g_main_loop_run(loopData->mMainLoop);
    ChipLogProgress(DeviceLayer, "[QUIT] main loop %p", loopData->mMainLoop);
    g_main_loop_unref(loopData->mMainLoop);
    loopData->mMainLoop = NULL;

    g_main_context_pop_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[POP] main context %p", loopData->mMainContext);
    g_main_context_unref(loopData->mMainContext);
    loopData->mMainContext = NULL;

    return NULL;
}

gpointer MainLoop::ThreadAsyncHandler(gpointer data)
{
    LoopData * loopData = reinterpret_cast<LoopData *>(data);

    g_main_context_push_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[PUSH] async context %p", loopData->mMainContext);

    ChipLogProgress(DeviceLayer, "[RUN] async loop %p", loopData->mMainLoop);
    g_main_loop_run(loopData->mMainLoop);
    ChipLogProgress(DeviceLayer, "[QUIT] async loop %p", loopData->mMainLoop);
    g_main_loop_unref(loopData->mMainLoop);
    loopData->mMainLoop = NULL;

    g_main_context_pop_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[POP] async context %p", loopData->mMainContext);
    g_main_context_unref(loopData->mMainContext);
    loopData->mMainContext = NULL;

    chip::Platform::Delete(loopData);

    return NULL;
}

bool MainLoop::Init(initFn_t initFn, gpointer userData)
{
    bool result;
    LoopData * loopData = chip::Platform::New<LoopData>();

    VerifyOrReturnError(loopData != NULL, false);

    loopData->mMainContext = g_main_context_new();
    loopData->mMainLoop    = g_main_loop_new(loopData->mMainContext, FALSE);

    g_main_context_push_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[PUSH] main context %p", loopData->mMainContext);

    result = initFn(userData);

    g_main_context_pop_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[POP] main context %p", loopData->mMainContext);

    if (result != false)
    {
        loopData->mThread = g_thread_new("ThreadMainHandler", ThreadMainHandler, reinterpret_cast<gpointer>(loopData));
        mLoopData.push_back(loopData);
    }

    return result;
}

void MainLoop::DeleteData(LoopData * loopData)
{
    if (loopData->mMainLoop)
    {
        g_main_loop_quit(loopData->mMainLoop);
        g_thread_join(loopData->mThread);
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

bool MainLoop::AsyncRequest(asyncFn_t asyncFn, gpointer asyncUserData, guint interval, timeoutFn_t timeoutFn,
                            gpointer timeoutUserData)
{
    bool result         = false;
    LoopData * loopData = chip::Platform::New<LoopData>();

    VerifyOrReturnError(loopData != NULL, false);

    loopData->mMainContext = g_main_context_new();
    loopData->mMainLoop    = g_main_loop_new(loopData->mMainContext, FALSE);

    g_main_context_push_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[PUSH] async context %p", loopData->mMainContext);

    result = asyncFn(loopData->mMainLoop, asyncUserData);

    g_main_context_pop_thread_default(loopData->mMainContext);
    ChipLogProgress(DeviceLayer, "[POP] async context %p", loopData->mMainContext);

    if (result != false)
    {
        loopData->mThread = g_thread_new("ThreadAsyncHandler", ThreadAsyncHandler, reinterpret_cast<gpointer>(loopData));

        if (interval)
        {
            loopData->mTimeoutFn       = timeoutFn;
            loopData->mTimeoutUserData = timeoutUserData;
            SetThreadTimeout(loopData, interval);
        }
    }

    return result;
}

gpointer MainLoop::ThreadStartLSMainLoopHandler(gpointer data)
{
    LoopData * loopData = reinterpret_cast<LoopData *>(data);

    ChipLogProgress(DeviceLayer, "[RUN] LS2 loop %p", loopData->mMainLoop);
    g_main_loop_run(loopData->mMainLoop);
    ChipLogProgress(DeviceLayer, "[QUIT] LS2 loop %p", loopData->mMainLoop);
    g_main_loop_unref(loopData->mMainLoop);
    loopData->mMainLoop = NULL;

    g_main_context_unref(loopData->mMainContext);
    loopData->mMainContext = NULL;

    chip::Platform::Delete(loopData);

    return NULL;
}

bool MainLoop::StartLSMainLoop(void)
{
    bool result = true;
    LSError lserror;
    LSErrorInit(&lserror);

    LoopData * loopData = chip::Platform::New<LoopData>();

    VerifyOrReturnError(loopData != NULL, false);

    loopData->mMainContext = g_main_context_new();
    loopData->mMainLoop    = g_main_loop_new(loopData->mMainContext, FALSE);

    if (!LSRegister("com.webos.service.matter-1234", &mLSHandle, &lserror))
    {
        g_print("Unable to register to luna-bus\n");
        LSErrorFree(&lserror);
        result = false;
    }

    if (!LSGmainAttach(mLSHandle, loopData->mMainLoop, &lserror))
    {
        g_print("Unable to attach service\n");
        result = false;
    }

    if (result != false)
    {
        loopData->mThread =
            g_thread_new("ThreadStartLSMainLoopHandler", ThreadStartLSMainLoopHandler, reinterpret_cast<gpointer>(loopData));
    }

    return result;
}

MainLoop & MainLoop::Instance(void)
{
    static MainLoop sMainLoop;
    return sMainLoop;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
