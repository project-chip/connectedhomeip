/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Provides an implementation of the PlatformManager object
 *          for Tizen platforms.
 */

/**
 * Note: Use public include for PlatformManager which includes our local
 *       platform/<PLATFORM>/PlatformManager.h after defining interface
 *       class. */
#include <platform/PlatformManager.h>

#include <condition_variable>
#include <mutex>

#include <glib.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/Tizen/DeviceInstanceInfoProviderImpl.h>

#include "PosixConfig.h"
#include "SystemInfo.h"
#include "platform/internal/GenericPlatformManagerImpl.h"
#include "platform/internal/GenericPlatformManagerImpl.ipp"
#include "platform/internal/GenericPlatformManagerImpl_POSIX.h"
#include "platform/internal/GenericPlatformManagerImpl_POSIX.ipp"

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

namespace {

struct GLibMatterContextInvokeData
{
    CHIP_ERROR (*mFunc)(void *);
    void * mFuncUserData;
    CHIP_ERROR mFuncResult;
    // Sync primitives to wait for the function to be executed
    std::mutex mDoneMutex;
    std::condition_variable mDoneCond;
    bool mDone = false;
};

void * GLibMainLoopThread(void * userData)
{
    GMainLoop * loop       = static_cast<GMainLoop *>(userData);
    GMainContext * context = g_main_loop_get_context(loop);

    g_main_context_push_thread_default(context);
    g_main_loop_run(loop);

    return nullptr;
}

} // namespace

CHIP_ERROR PlatformManagerImpl::_InitChipStack()
{
    auto * context      = g_main_context_new();
    mGLibMainLoop       = g_main_loop_new(context, FALSE);
    mGLibMainLoopThread = g_thread_new("gmain-matter", GLibMainLoopThread, mGLibMainLoop);
    g_main_context_unref(context);

    {
        // Wait for the GLib main loop to start. It is required that the GLib Matter
        // context is acquired by the g_main_loop_run() before any other GLib function
        // is called. Otherwise, the GLibMatterContextInvokeSync() might run callback
        // functions on the wrong thread.

        GLibMatterContextInvokeData invokeData{};

        GAutoPtr<GSource> idleSource(g_idle_source_new());
        g_source_set_callback(
            idleSource.get(),
            [](void * userData_) {
                auto * data = reinterpret_cast<GLibMatterContextInvokeData *>(userData_);
                std::unique_lock<std::mutex> lock(data->mDoneMutex);
                data->mDone = true;
                data->mDoneCond.notify_one();
                return G_SOURCE_REMOVE;
            },
            &invokeData, nullptr);
        g_source_attach(idleSource.get(), g_main_loop_get_context(mGLibMainLoop));

        std::unique_lock<std::mutex> lock(invokeData.mDoneMutex);
        invokeData.mDoneCond.wait(lock, [&invokeData]() { return invokeData.mDone; });
    }

    ReturnErrorOnFailure(Internal::PosixConfig::Init());

    ReturnErrorOnFailure(Internal::GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>::_InitChipStack());

    // Now set up our device instance info provider.  We couldn't do that
    // earlier, because the generic implementation sets a generic one.
    SetDeviceInstanceInfoProvider(&DeviceInstanceInfoProviderMgrImpl());

    mStartTime = System::SystemClock().GetMonotonicTimestamp();

    Internal::PlatformVersion version;
    ReturnErrorOnFailure(Internal::SystemInfo::GetPlatformVersion(version));
    ChipLogProgress(DeviceLayer, "Tizen Version: %d.%d", version.mMajor, version.mMinor);

    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::_Shutdown()
{
    if (mGLibMainLoop == nullptr)
    {
        ChipLogError(DeviceLayer, "System Layer is already shutdown.");
        return;
    }

    Internal::GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>::_Shutdown();

    g_main_loop_quit(mGLibMainLoop);
    g_main_loop_unref(mGLibMainLoop);
    g_thread_join(mGLibMainLoopThread);

    mGLibMainLoop = nullptr;
}

CHIP_ERROR PlatformManagerImpl::_GLibMatterContextInvokeSync(CHIP_ERROR (*func)(void *), void * userData)
{
    GLibMatterContextInvokeData invokeData{ func, userData };

    g_main_context_invoke_full(
        g_main_loop_get_context(mGLibMainLoop), G_PRIORITY_HIGH_IDLE,
        [](void * userData_) {
            auto * data = reinterpret_cast<GLibMatterContextInvokeData *>(userData_);
            VerifyOrExit(g_main_context_get_thread_default() != nullptr,
                         ChipLogError(DeviceLayer, "GLib thread default main context is not set");
                         data->mFuncResult = CHIP_ERROR_INCORRECT_STATE);
            data->mFuncResult = data->mFunc(data->mFuncUserData);
        exit:
            data->mDoneMutex.lock();
            data->mDone = true;
            data->mDoneMutex.unlock();
            data->mDoneCond.notify_one();
            return G_SOURCE_REMOVE;
        },
        &invokeData, nullptr);

    std::unique_lock<std::mutex> lock(invokeData.mDoneMutex);
    invokeData.mDoneCond.wait(lock, [&invokeData]() { return invokeData.mDone; });

    return invokeData.mFuncResult;
}

} // namespace DeviceLayer
} // namespace chip
