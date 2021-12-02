/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 * Provides access to UDP (and optionally TCP) EndPointManager.
 */

#include <inet/InetLayer.h>

#include <inet/InetError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/LockTracker.h>

namespace chip {
namespace Inet {

CHIP_ERROR InetLayer::Init(System::Layer & aSystemLayer, EndPointManager<UDPEndPoint> * udpEndPointManager)
{
    Inet::RegisterLayerErrorFormatter();
    VerifyOrReturnError(mLayerState.SetInitializing(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(udpEndPointManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mSystemLayer = &aSystemLayer;
    ReturnErrorOnFailure(udpEndPointManager->Init(aSystemLayer));
    mUDPEndPointManager = udpEndPointManager;

    mLayerState.SetInitialized();
    return CHIP_NO_ERROR;
}

CHIP_ERROR InetLayer::Shutdown()
{
    VerifyOrReturnError(mLayerState.SetShuttingDown(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mTCPEndPointManager == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mUDPEndPointManager != nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mUDPEndPointManager->Shutdown());
    mUDPEndPointManager = nullptr;
    mSystemLayer        = nullptr;
    mLayerState.ResetFromShuttingDown(); // Return to uninitialized state to permit re-initialization.
    return CHIP_NO_ERROR;
}

CHIP_ERROR InetLayer::InitTCP(EndPointManager<TCPEndPoint> * tcpEndPointManager)
{
    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    VerifyOrReturnError(tcpEndPointManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(tcpEndPointManager->Init(*mSystemLayer));
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
    mTCPEndPointManager = tcpEndPointManager;
    return CHIP_NO_ERROR;
}

CHIP_ERROR InetLayer::ShutdownTCP()
{
    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if (mTCPEndPointManager != nullptr)
    {
        ReturnErrorOnFailure(mTCPEndPointManager->Shutdown());
        mTCPEndPointManager = nullptr;
    }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
    return CHIP_NO_ERROR;
}

} // namespace Inet
} // namespace chip
