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

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <errno.h>
#include <pthread.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

MainLoop & MainLoop::Instance()
{
    static MainLoop sMainLoop;
    return sMainLoop;
}

void * MainLoop::Thread(void * self)
{
    MainLoop * loop = reinterpret_cast<MainLoop *>(self);

    ChipLogDetail(DeviceLayer, "TRACE: Bluez mainloop starting %s", __func__);
    g_main_loop_run(loop->mBluezMainLoop);
    ChipLogDetail(DeviceLayer, "TRACE: Bluez mainloop stopping %s", __func__);

    if (loop->mCleanup != nullptr)
    {
        ChipLogDetail(DeviceLayer, "TRACE: Executing cleanup %s", __func__);
        loop->mCleanup(loop->mCleanupArgument);
    }

    return nullptr;
}

CHIP_ERROR MainLoop::EnsureStarted()
{
    if (mBluezMainLoop != nullptr)
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;
    int pthreadErr = 0;
    int tmpErrno;

    mBluezMainLoop = g_main_loop_new(nullptr, FALSE);
    VerifyOrExit(mBluezMainLoop != nullptr, ChipLogError(DeviceLayer, "FAIL: memory alloc in %s", __func__));

    pthreadErr = pthread_create(&mThread, nullptr, &MainLoop::Thread, reinterpret_cast<void *>(this));
    tmpErrno   = errno;
    VerifyOrExit(pthreadErr == 0, ChipLogError(DeviceLayer, "FAIL: pthread_create (%s) in %s", strerror(tmpErrno), __func__));

    while (!g_main_loop_is_running(mBluezMainLoop))
    {
        pthread_yield();
    }

exit:

    if (err != CHIP_NO_ERROR)
    {
        if (mBluezMainLoop != nullptr)
        {
            g_free(mBluezMainLoop);
            mBluezMainLoop = nullptr;
        }
    }

    return err;
}

bool MainLoop::RunOnBluezThread(GSourceFunc callback, void * argument)
{
    GMainContext * context = nullptr;
    const char * msg       = nullptr;

    VerifyOrExit(mBluezMainLoop != nullptr, msg = "FAIL: NULL sBluezMainLoop");
    VerifyOrExit(g_main_loop_is_running(mBluezMainLoop), msg = "FAIL: sBluezMainLoop not running");

    context = g_main_loop_get_context(mBluezMainLoop);
    VerifyOrExit(context != nullptr, msg = "FAIL: NULL main context");
    g_main_context_invoke(context, callback, argument);

exit:
    if (msg != nullptr)
    {
        ChipLogDetail(DeviceLayer, "%s in %s", msg, __func__);
    }

    return msg == nullptr;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
