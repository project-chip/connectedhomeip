/*
 *    Copyright (c) 2020 Project CHIP Authors
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
 *     @file
 *          Implementation for the CHIP Manager class.
 *
 *          This class provides various facilities through CHIP's inner
 *          implementation
 *
 */

#include <errno.h>

#include "CHIPManager.h"

#include <support/CodeUtils.h>
#include <support/ErrorStr.h>

CHIP_ERROR ChipManager::InitLayers()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initialize the CHIP System Layer.
    err = mSystem->Init(NULL);
    if (err != CHIP_SYSTEM_NO_ERROR)
    {
        ChipLogError(DeviceManager, "SystemLayer initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Initialize the CHIP Inet layer.
    err = mInet->Init(*mSystem, NULL);
    if (err != INET_NO_ERROR)
    {
        ChipLogError(DeviceManager, "InetLayer initialization failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

exit:
    return err;
}

System::Layer * ChipManager::SystemLayer()
{
    return this->mSystem;
}

Inet::InetLayer * ChipManager::InetLayer()
{
    return this->mInet;
}

void ChipManager::ServiceEvents()
{

    // Set the select timeout to 100ms
    struct timeval aSleepTime;
    aSleepTime.tv_sec  = 0;
    aSleepTime.tv_usec = 100 * 1000;

    static bool printed = false;

    if (!printed)
    {
        {
            ChipLogProgress(DeviceManager, "CHIP node ready to service events; PID: %d; PPID: %d\n", getpid(), getppid());
            printed = true;
        }
    }
    fd_set readFDs, writeFDs, exceptFDs;
    int numFDs = 0;

    FD_ZERO(&readFDs);
    FD_ZERO(&writeFDs);
    FD_ZERO(&exceptFDs);

    if (mSystem->State() == System::kLayerState_Initialized)
        mSystem->PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);

    if (mInet->State == Inet::InetLayer::kState_Initialized)
        mInet->PrepareSelect(numFDs, &readFDs, &writeFDs, &exceptFDs, aSleepTime);

    int selectRes = select(numFDs, &readFDs, &writeFDs, &exceptFDs, &aSleepTime);
    if (selectRes < 0)
    {
        ChipLogError(DeviceManager, "select failed: %s\n", ErrorStr(System::MapErrorPOSIX(errno)));
        return;
    }

    if (mSystem->State() == System::kLayerState_Initialized)
    {
        mSystem->HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }

    if (mInet->State == Inet::InetLayer::kState_Initialized)
    {
        mInet->HandleSelectResult(selectRes, &readFDs, &writeFDs, &exceptFDs);
    }
}
