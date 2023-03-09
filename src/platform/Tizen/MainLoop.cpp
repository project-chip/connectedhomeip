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

#include <iterator>
#include <memory>
#include <utility>

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

void MainLoop::Deinit()
{
    for (auto & loopData : mLoopData)
    {
        g_main_loop_quit(loopData->mMainLoop);
    }
    mLoopData.clear();
}

bool MainLoop::AsyncRequest(asyncFn_t asyncFn, gpointer asyncUserData)
{
    auto loopData = std::make_shared<LoopData>();
    bool result   = false;

    VerifyOrReturnError(loopData, false);

    g_main_context_push_thread_default(loopData->mMainContext);
    ChipLogDetail(DeviceLayer, "[PUSH] thread default context [%p]", loopData->mMainContext);

    result = asyncFn(loopData->mMainLoop, asyncUserData);

    g_main_context_pop_thread_default(loopData->mMainContext);
    ChipLogDetail(DeviceLayer, "[POP] thread default context [%p]", loopData->mMainContext);

    VerifyOrReturnError(result, false);

    loopData->mThread = std::thread(ThreadHandler, loopData);
    loopData->mThread.detach();

    return true;
}

MainLoop & MainLoop::Instance()
{
    static MainLoop sMainLoop;
    return sMainLoop;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
