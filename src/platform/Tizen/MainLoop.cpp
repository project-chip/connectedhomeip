/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "MainLoop.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

LoopData::LoopData()
{
    mMainContext = g_main_context_new();
    mMainLoop    = g_main_loop_new(mMainContext, FALSE);
}

LoopData::~LoopData()
{
    if (mThread.joinable())
    {
        mThread.join();
    }
    if (mMainContext != nullptr)
    {
        g_main_context_unref(mMainContext);
    }
    if (mMainLoop != nullptr)
    {
        g_main_loop_unref(mMainLoop);
    }
}

gboolean MainLoop::ThreadTimeout(gpointer userData)
{
    LoopData * loopData = reinterpret_cast<LoopData *>(userData);

    VerifyOrReturnError(loopData != nullptr, G_SOURCE_REMOVE);

    ChipLogDetail(DeviceLayer, "[TIMEOUT] glib main loop [%p]", loopData->mMainLoop);

    if (loopData->mTimeoutFn)
    {
        ChipLogDetail(DeviceLayer, "Running thread timeout function");
        loopData->mTimeoutFn(loopData->mTimeoutUserData);
    }

    g_main_loop_quit(loopData->mMainLoop);

    return G_SOURCE_REMOVE;
}

void MainLoop::ThreadHandler(std::shared_ptr<LoopData> loopData)
{
    g_main_context_push_thread_default(loopData->mMainContext);
    ChipLogDetail(DeviceLayer, "[PUSH] thread default context [%p]", loopData->mMainContext);

    ChipLogDetail(DeviceLayer, "[RUN] glib main loop [%p]", loopData->mMainLoop);
    g_main_loop_run(loopData->mMainLoop);
    ChipLogDetail(DeviceLayer, "[QUIT] glib main loop [%p]", loopData->mMainLoop);

    g_main_context_pop_thread_default(loopData->mMainContext);
    ChipLogDetail(DeviceLayer, "[POP] thread default context [%p]", loopData->mMainContext);
}

bool MainLoop::Init(initFn_t initFn, gpointer userData)
{
    auto loopData = std::make_shared<LoopData>();
    bool result   = false;

    VerifyOrReturnError(loopData, false);

    g_main_context_push_thread_default(loopData->mMainContext);
    ChipLogDetail(DeviceLayer, "[PUSH] thread default context [%p]", loopData->mMainContext);

    result = initFn(userData);

    g_main_context_pop_thread_default(loopData->mMainContext);
    ChipLogDetail(DeviceLayer, "[POP] thread default context [%p]", loopData->mMainContext);

    VerifyOrReturnError(result, false);

    loopData->mThread = std::thread(ThreadHandler, loopData);
    mLoopData.push_back(loopData);

    return true;
}

void MainLoop::Deinit(void)
{
    for (auto & loopData : mLoopData)
    {
        g_main_loop_quit(loopData->mMainLoop);
    }
    mLoopData.clear();
}

bool MainLoop::AsyncRequest(asyncFn_t asyncFn, gpointer asyncUserData, guint timeoutInterval, timeoutFn_t timeoutFn,
                            gpointer timeoutUserData)
{
    auto loopData = std::make_shared<LoopData>();
    bool result   = false;

    VerifyOrReturnError(loopData, false);

    loopData->mTimeoutFn       = timeoutFn;
    loopData->mTimeoutUserData = timeoutUserData;

    g_main_context_push_thread_default(loopData->mMainContext);
    ChipLogDetail(DeviceLayer, "[PUSH] thread default context [%p]", loopData->mMainContext);

    result = asyncFn(loopData->mMainLoop, asyncUserData);

    g_main_context_pop_thread_default(loopData->mMainContext);
    ChipLogDetail(DeviceLayer, "[POP] thread default context [%p]", loopData->mMainContext);

    VerifyOrReturnError(result, false);

    loopData->mThread = std::thread(ThreadHandler, loopData);
    loopData->mThread.detach();

    if (timeoutInterval)
    {
        GSource * source = g_timeout_source_new_seconds(timeoutInterval);
        g_source_set_callback(source, ThreadTimeout, loopData.get(), nullptr);
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
