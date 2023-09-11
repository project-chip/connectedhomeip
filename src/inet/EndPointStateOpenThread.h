/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
