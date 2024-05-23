/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
 *  Shared state for OpenThread implementations of TCPEndPoint and UDPEndPoint.
 */

#pragma once

#include <inet/EndPointBasis.h>

#include <inet/IPAddress.h>

namespace chip {
namespace Inet {

/**
 * Definitions shared by all OpenThread EndPoint classes.
 */
class DLL_EXPORT EndPointStateOpenThread
{
public:
    typedef void (*openThreadTaskSyncCb)(void);
    struct OpenThreadEndpointInitParam
    {
        openThreadTaskSyncCb lockCb        = nullptr;
        openThreadTaskSyncCb unlockCb      = nullptr;
        otInstance * openThreadInstancePtr = nullptr;
    };

protected:
    EndPointStateOpenThread() : mOpenThreadEndPointType(OpenThreadEndPointType::Unknown) {}

    enum class OpenThreadEndPointType : uint8_t
    {
        Unknown = 0,
        UDP     = 1,
        TCP     = 2
    } mOpenThreadEndPointType;

    void LockOpenThread(void)
    {
        if (lockOpenThread != nullptr)
            lockOpenThread();
    }
    void UnlockOpenThread(void)
    {
        if (unlockOpenThread != nullptr)
            unlockOpenThread();
    }

    otInstance * mOTInstance              = nullptr;
    openThreadTaskSyncCb lockOpenThread   = nullptr;
    openThreadTaskSyncCb unlockOpenThread = nullptr;
};

} // namespace Inet
} // namespace chip
